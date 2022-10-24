#pragma once
#include <napi.h>

#include <depthai/build/version.hpp>
#include <depthai/depthai.hpp>
#include <functional>
#include <variant>

using Context = Napi::Reference<Napi::Value>;
struct EmitData;
extern void emit(Napi::Env env, Napi::Function callback, Context* context, EmitData* rawData);
using MonitorCallback = Napi::TypedThreadSafeFunction<Context, EmitData, emit>;

struct DeviceStats {
  int cssUsage;
  int mssUsage;
  int64_t ddrMemFree;
  int64_t ddrMemTotal;
  int64_t cmxMemFree;
  int64_t cmxMemTotal;
  int cssTemperature;
  int mssTemperature;
  int upaTemperature;
  int dssTemperature;
  int temperature;
};

struct DeviceInfo {
  XLinkDeviceState_t state = X_LINK_ANY_STATE;
  XLinkProtocol_t protocol = X_LINK_ANY_PROTOCOL;
  XLinkPlatform_t platform = X_LINK_ANY_PLATFORM;
  std::string productName;
  std::string boardName;
  std::string boardRev;
  std::string bootloaderVersion;
};

struct DeviceBootProgress {
  enum class Stage { Queued = 1, Bootloader, Creating, Running };

  static std::string stageToString(Stage stage) {
    switch(stage) {
      case Stage::Queued:
        return "QUEUED";
      case Stage::Bootloader:
        return "BOOTLOADER";
      case Stage::Creating:
        return "CREATING";
      case Stage::Running:
      default:
        return "RUNNING";
    }
  }

  Stage stage;
};

struct DeviceError {
  std::string message;
};

struct EmitData {
  explicit EmitData(std::string serialNumber, const dai::SystemInformation* sys);

  explicit EmitData(const dai::DeviceInfo& deviceInfo, const dai::EepromData& eeprom, const dai::DeviceBootloader::Version& blVersion);

  explicit EmitData(std::string serialNumber, const char* message);

  explicit EmitData(std::string serialNumber, DeviceBootProgress::Stage stage);

  std::string serialNumber;
  enum EmitDataType { STATS = 1, INFO, BOOT, ERROR } type;

  std::variant<DeviceStats, DeviceInfo, DeviceBootProgress, DeviceError> payload;
};