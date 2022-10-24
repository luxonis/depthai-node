#pragma once
#include <atomic>
#include <depthai/build/version.hpp>
#include <depthai/depthai.hpp>
#include <shared_mutex>

#include "pipeline.hpp"

class Discovery {
 public:
  Discovery();

  ~Discovery();

  bool hasPipeline(const std::string& mxId);

  bool addPipeline(const dai::DeviceInfo& device, const MonitorCallback& callback);

  std::weak_ptr<Pipeline> removePipeline(const std::string& mxId);

  std::vector<dai::DeviceInfo> getAllDevices();

  std::tuple<bool, dai::DeviceInfo> getDeviceByMxId(const std::string& mxId);

 private:
  void runMonitoring();

  void runPipelines();

  void startPipelines();

  void updatePipelines();

  void refreshDevices();

  std::shared_mutex pipelinesMutex;
  std::shared_mutex devicesMutex;

  std::thread pipelineThread;
  std::thread monitoringThread;

  std::atomic_bool running = {true};

  std::unordered_map<std::string, std::shared_ptr<Pipeline>> pipelines;

  std::chrono::steady_clock::time_point nextDevicesUpdate;
  std::vector<dai::DeviceInfo> devices;
};
