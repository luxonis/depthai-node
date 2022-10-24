#pragma once
#include "device.hpp"
#include "debug.hpp"
#include "deviceMonitor.hpp"

const auto MIN_BOOTLOADER_VERSION = dai::DeviceBootloader::Version(0, 0, 17);

class Pipeline {
public:
  explicit Pipeline(dai::DeviceInfo deviceInfo, MonitorCallback emitFn);

  ~Pipeline();

  [[nodiscard]] bool needsStart() const;

  [[nodiscard]] bool needsDestroy() const;

  void update();

  void setStage(DeviceBootProgress::Stage newStage);

  void emitData(EmitData* data) const;

  void destroy() const;

  void start();

  void stop();

  std::string mxId;
  std::atomic<DeviceBootProgress::Stage> stage = DeviceBootProgress::Stage::Queued;
  dai::DeviceInfo deviceInfo;
  std::unique_ptr<dai::Device> device;
  std::atomic<bool> running = {true};
  bool streamEnabled = false;
  bool lastConfig = false;
  std::mutex guard;
  MonitorCallback emitFn;
};