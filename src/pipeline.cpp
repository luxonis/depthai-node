#include "pipeline.hpp"
#include "debug.hpp"

extern RtcService rtcService;

Pipeline::Pipeline(dai::DeviceInfo deviceInfo, MonitorCallback emitFn) : mxId(deviceInfo.mxid), deviceInfo(std::move(deviceInfo)), emitFn(emitFn) {
  LOG("Device: '" << this->mxId << "' constructor");
};

Pipeline::~Pipeline() {
  LOG("Device: '" << mxId << "' destructor");
  emitFn.Release();
}

[[nodiscard]] bool Pipeline::needsStart() const {
  return running && (!device || device->isClosed());
}

[[nodiscard]] bool Pipeline::needsDestroy() const {
  return !running;
}

void Pipeline::update() {
  auto now = std::chrono::steady_clock::now();
  if(device && lastConfig != streamEnabled) {
    auto buff = dai::Buffer();
    uint8_t val = streamEnabled ? 1 : 0;
    buff.setData({val});
    device->getInputQueue("config")->send(buff);
    lastConfig = streamEnabled;
  }
}

void Pipeline::setStage(DeviceBootProgress::Stage newStage) {
  stage = newStage;
  emitData(new EmitData(mxId, newStage));
}

void Pipeline::emitData(EmitData* data) const {
  if(running) {
    emitFn.BlockingCall(data);
  }
}

void Pipeline::destroy() const {
  if(device) {
    device->close();
  }
}

void Pipeline::stop() {
  running = false;
}

void Pipeline::start() {
  bool allowFlashing = false;

  try {
    LOG("Starting pipeline for device: '" << mxId);
    constexpr auto fps = 30;
    constexpr auto sampleDuration = (1000 * 1000) / fps;

    LOG("Creating pipeline for device: '" << mxId);

    auto blVersion = dai::DeviceBootloader::Version(0, 0, 0);
    if(deviceInfo.state == X_LINK_BOOTLOADER) {
      setStage(DeviceBootProgress::Stage::Bootloader);

      auto bl = dai::DeviceBootloader(deviceInfo, allowFlashing);
      blVersion = bl.getVersion();
      LOG("Bootloader version: '" << blVersion);

      if(allowFlashing && blVersion < MIN_BOOTLOADER_VERSION) {
        std::cout << "Flashing start..." << std::endl;
        auto progress = [](float p) { std::cout << "Flashing Progress..." << p * 100 << "%" << std::endl; };
        auto ts = std::chrono::steady_clock::now();
        bool success = false;
        std::string message;
        std::tie(success, message) = bl.flashBootloader(dai::DeviceBootloader::Memory::FLASH, bl.getType(), progress);
        if(success) {
          std::cout << "Flashing successful. Took " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - ts).count()
                    << "ms" << std::endl;
        } else {
          std::cout << "Flashing failed: " << message << std::endl;
        }
      }
    }

    if(!running) {
      return;  // return early
    }

    setStage(DeviceBootProgress::Stage::Creating);

    dai::Pipeline pipeline;
    auto camRgb = pipeline.create<dai::node::ColorCamera>();
    camRgb->setBoardSocket(dai::CameraBoardSocket::RGB);
    camRgb->setResolution(dai::ColorCameraProperties::SensorResolution::THE_1080_P);
    camRgb->setIspScale(2, 3);
    camRgb->setFps(fps);

    auto controlScript = pipeline.create<dai::node::Script>();
    controlScript->setProcessor(dai::ProcessorType::LEON_CSS);
    controlScript->setScript(R"""(
enabled = False
while True:
  config_payload = node.io['config'].tryGet()
  if config_payload:
    data = config_payload.getData()
    enabled = bool(data[0])
  frame = node.io['video_in'].get()
  if enabled:
    node.io['video_out'].send(frame)
)""",
                             "video_control");

    auto configInput = controlScript->inputs["config"];
    configInput.setBlocking(false);
    configInput.setQueueSize(1);

    auto videoEncoder = pipeline.create<dai::node::VideoEncoder>();
    videoEncoder->setDefaultProfilePreset(fps, dai::VideoEncoderProperties::Profile::H264_MAIN);
    videoEncoder->setKeyframeFrequency(fps);
    videoEncoder->setBitrate(3500 * 1000);
    videoEncoder->setRateControlMode(dai::VideoEncoderProperties::RateControlMode::CBR);

    auto sysLog = pipeline.create<dai::node::SystemLogger>();
    sysLog->setRate(1.0f);

    auto configIn = pipeline.create<dai::node::XLinkIn>();
    configIn->setStreamName("config");

    auto logOut = pipeline.create<dai::node::XLinkOut>();
    logOut->setStreamName("system");
    auto videoOut = pipeline.create<dai::node::XLinkOut>();
    videoOut->setStreamName("video");

    configIn->out.link(configInput);
    sysLog->out.link(logOut->input);
    camRgb->video.link(controlScript->inputs["video_in"]);
    controlScript->outputs["video_out"].link(videoEncoder->input);
    videoEncoder->bitstream.link(videoOut->input);

    // NOTE(michal): This is blocking
    device = std::make_unique<dai::Device>(pipeline, deviceInfo);
    LOG("Pipeline for device: '" << mxId << " created");

    device->getOutputQueue(logOut->getStreamName(), 1, false)->addCallback([this](const std::string&, const std::shared_ptr<dai::ADatatype>& callback) {
      auto* sys = dynamic_cast<dai::SystemInformation*>(callback.get());
      if(sys == nullptr || !running) return;
      emitData(new EmitData(mxId, sys));
    });

    device->getOutputQueue(videoOut->getStreamName(), 1, false)->addCallback([this](const std::string&, const std::shared_ptr<dai::ADatatype>& callback) {
      auto* frame = dynamic_cast<dai::ImgFrame*>(callback.get());
      if(frame == nullptr || !running) return;
      rtcService.sendData(mxId, sampleDuration, reinterpret_cast<std::vector<std::byte>&>(frame->getData()));
    });

    dai::EepromData eeprom{};

    try {
      eeprom = device->readFactoryCalibration().getEepromData();
    } catch(const std::exception& ex) {
      try {
        eeprom = device->readCalibration2().getEepromData();
      } catch(const std::exception& ex) {
        eeprom = device->readCalibration().getEepromData();
      }
    }

    emitData(new EmitData(deviceInfo, eeprom, blVersion));

    LOG("Pipeline for device: '" << mxId << " running");

    setStage(DeviceBootProgress::Stage::Running);
  } catch(const std::exception& e) {
    running = false;
    emitData(new EmitData(mxId, e.what()));
    LOG("Device: '" << mxId << "' thrown an error: " << e.what());
  } catch(...) {
    running = false;
    emitData(new EmitData(mxId, "Unknown error"));
    LOG("Device: '" << mxId << "' thrown an unknown error");
  }
}
