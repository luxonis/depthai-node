#include "discovery.hpp"
#include "debug.hpp"
#include "rtcService.hpp"
#include <XLink/XLink.h>

extern RtcService rtcService;

Discovery::Discovery() {
  pipelineThread = std::thread(&Discovery::runPipelines, this);
  monitoringThread = std::thread(&Discovery::runMonitoring, this);
}

Discovery::~Discovery() {
  running = false;
  if(monitoringThread.joinable()) monitoringThread.join();
  if(pipelineThread.joinable()) pipelineThread.join();
}

void Discovery::runMonitoring() {
  while(running) {
    {
      std::shared_lock lock(pipelinesMutex);
      for(const auto& item : pipelines) {
        auto stage = item.second->stage.load();
        if(item.second->running && stage != DeviceBootProgress::Stage::Running) {
          item.second->emitData(new EmitData(item.first, stage));
        }
      }
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

void Discovery::runPipelines() {
  while(running) {
    refreshDevices();
    startPipelines();
    updatePipelines();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
}

void Discovery::startPipelines() {
  std::vector<std::shared_ptr<Pipeline>> pipelinesToStart;
  {
    std::shared_lock lock(pipelinesMutex);
    for(const auto& item : pipelines) {
      if(item.second->needsStart()) {
        pipelinesToStart.push_back(item.second);
      }
    }
  }

  for(const auto& pipeline : pipelinesToStart) {
    if(pipeline->needsStart()) {
      pipeline->start();
    }
  }
}

void Discovery::updatePipelines() {
  std::vector<std::shared_ptr<Pipeline>> pipelinesToPrune;
  LOG("maintainPipelines start");
  {
    std::shared_lock lock(pipelinesMutex);
    auto tracks = rtcService.getTracks();
    for(const auto& pipeline : pipelines) {
      if(pipeline.second->needsDestroy()) {
        pipelinesToPrune.push_back(pipeline.second);
      } else {
        pipeline.second->streamEnabled = tracks.find(pipeline.first) != tracks.end();
        pipeline.second->update();
      }
    }
  }

  LOG("Pruning " << pipelinesToPrune.size() << " pipelines");

  for(const auto& pipeline : pipelinesToPrune) {
    // NOTE(michal): may be a long blocking operation
    pipeline->destroy();
  }

  {
    std::unique_lock lock(pipelinesMutex);
    for(const auto& pipeline : pipelinesToPrune) {
      auto search = pipelines.find(pipeline->mxId);
      if(search != pipelines.end()) {
        pipelines.erase(search);
      }
    }
  }

  LOG("Pruning done");
}

bool Discovery::hasPipeline(const std::string& mxId) {
  std::shared_lock lock(pipelinesMutex);
  auto search = pipelines.find(mxId);
  return search != pipelines.end();
}

bool Discovery::addPipeline(const dai::DeviceInfo& device, const MonitorCallback& callback) {
  std::unique_lock lock(pipelinesMutex);
  auto search = pipelines.find(device.mxid);
  if(search == pipelines.end()) {
    pipelines.emplace(device.mxid, std::make_shared<Pipeline>(device, callback));
    return true;
  }
  return false;
}

std::weak_ptr<Pipeline> Discovery::removePipeline(const std::string& mxId) {
  std::weak_ptr<Pipeline> weakPipeline;

  LOG("[removePipeline] " << mxId);

  {
    std::unique_lock lock(pipelinesMutex);
    auto search = pipelines.find(mxId);
    if(search != pipelines.end()) {
      weakPipeline = search->second;
    } else {
      LOG("[removePipeline] Pipeline " << mxId << " not found");
    }
  }

  if(auto pipeline = weakPipeline.lock()) {
    LOG("[removePipeline] " << mxId << " stop");
    pipeline->stop();
  }

  return weakPipeline;
}

std::vector<dai::DeviceInfo> Discovery::getAllDevices() {
  std::shared_lock lock(devicesMutex);
  return devices;
}

std::tuple<bool, dai::DeviceInfo> Discovery::getDeviceByMxId(const std::string& mxId) {
  std::shared_lock lock(devicesMutex);
  for(const auto& device : devices) {
    if(device.mxid == mxId) {
      return {true, device};
    }
  }

  return {false, {}};
}

void Discovery::refreshDevices() {
  std::unique_lock lock(devicesMutex);
  if(nextDevicesUpdate > std::chrono::steady_clock::now()) {
    return;
  }

  unsigned int numdev = 0;
  std::array<deviceDesc_t, 32> deviceDescAll = {};
  deviceDesc_t suitableDevice = {};
  suitableDevice.protocol = X_LINK_ANY_PROTOCOL;
  suitableDevice.platform = X_LINK_ANY_PLATFORM;
  suitableDevice.state = X_LINK_ANY_STATE;

  auto status = XLinkFindAllSuitableDevices(suitableDevice, deviceDescAll.data(), static_cast<unsigned int>(deviceDescAll.size()), &numdev);
  if(status != X_LINK_SUCCESS) throw std::runtime_error("Couldn't retrieve connected devices");

  devices.clear();
  for(unsigned i = 0; i < numdev; i++) {
    dai::DeviceInfo info(deviceDescAll.at(i));
    if(!info.mxid.empty()) {
      devices.push_back(info);
    }
  }

  nextDevicesUpdate = std::chrono::steady_clock::now() + std::chrono::seconds(5);
}