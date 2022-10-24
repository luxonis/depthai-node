#pragma once
#include <napi.h>

#include "device.hpp"

struct Discovery;

class StopMonitorWorker : public Napi::AsyncWorker {
 public:
  explicit StopMonitorWorker(const Napi::Function& callback, std::string mxId);

  ~StopMonitorWorker() override = default;

  void Execute() override;

  void OnOK() override;

 private:
  Discovery* discovery;
  std::string mxId;
};

class MonitorWorker : public Napi::AsyncWorker {
 public:
  explicit MonitorWorker(const Napi::Function& callback, const Napi::Function& monitorCallback, std::string mxId);

  ~MonitorWorker() override = default;

  void Execute() override;

  void OnError(const Napi::Error& e) override;

  void OnOK() override;

 private:
  Discovery* discovery;
  std::string mxId;
  MonitorCallback safeMonitorCallback;
};

class ListDevicesWorker : public Napi::AsyncWorker {
 public:
  explicit ListDevicesWorker(const Napi::Function& callback);

  ~ListDevicesWorker() override = default;

  void Execute() override;

  void OnOK() override;

 private:
  Discovery* discovery;
  std::vector<dai::DeviceInfo> devices;
};

Napi::Value monitor(const Napi::CallbackInfo& info);
Napi::Value stopMonitor(const Napi::CallbackInfo& info);
Napi::Value listDevices(const Napi::CallbackInfo& info);