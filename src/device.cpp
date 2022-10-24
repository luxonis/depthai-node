#include "device.hpp"

EmitData::EmitData(std::string serialNumber, const dai::SystemInformation* sys) : type(STATS), serialNumber(std::move(serialNumber)) {
  DeviceStats stats = {
      .cssUsage = int(sys->leonCssCpuUsage.average * 100),
      .mssUsage = int(sys->leonMssCpuUsage.average * 100),
      .ddrMemFree = sys->ddrMemoryUsage.remaining,
      .ddrMemTotal = sys->ddrMemoryUsage.total,
      .cmxMemFree = sys->cmxMemoryUsage.remaining,
      .cmxMemTotal = sys->cmxMemoryUsage.total,
      .cssTemperature = int(sys->chipTemperature.css),
      .mssTemperature = int(sys->chipTemperature.mss),
      .upaTemperature = int(sys->chipTemperature.upa),
      .dssTemperature = int(sys->chipTemperature.dss),
      .temperature = int(sys->chipTemperature.average),
  };
  payload.emplace<DeviceStats>(stats);
}

EmitData::EmitData(const dai::DeviceInfo& deviceInfo, const dai::EepromData& eeprom, const dai::DeviceBootloader::Version& blVersion)
    : type(INFO), serialNumber(deviceInfo.getMxId()) {
  DeviceInfo info = {
      .state = deviceInfo.state,
      .protocol = deviceInfo.protocol,
      .platform = deviceInfo.platform,
      .productName = eeprom.productName,
      .boardName = eeprom.boardName,
      .boardRev = eeprom.boardRev,
      .bootloaderVersion = blVersion.toString(),
  };
  payload.emplace<DeviceInfo>(info);
}

EmitData::EmitData(std::string serialNumber, const char* message) : type(ERROR), serialNumber(std::move(serialNumber)) {
  DeviceError error = {.message = message};
  payload.emplace<DeviceError>(error);
}

EmitData::EmitData(std::string serialNumber, DeviceBootProgress::Stage stage) : type(BOOT), serialNumber(std::move(serialNumber)) {
  DeviceBootProgress bootProgress = {.stage = stage};
  payload.emplace<DeviceBootProgress>(bootProgress);
}

void emit(Napi::Env env, Napi::Function callback, Context* context, EmitData* rawData) {
  if(rawData == nullptr) {
    return;
  }

  auto data = std::unique_ptr<EmitData>(rawData);
  if(env != nullptr) {
    // On Node-API 5+, the `callback` parameter is optional; however, this example
    // does ensure a callback is provided.
    if(callback != nullptr) {
      switch(data->type) {
        case EmitData::STATS: {
          auto stats = std::get<DeviceStats>(data->payload);
          auto obj = Napi::Object::New(env);
          obj.Set("type", "statistics");
          obj.Set("serialNumber", data->serialNumber);
          obj.Set("cssUsage", stats.cssUsage);
          obj.Set("mssUsage", stats.mssUsage);
          obj.Set("ddrMemFree", stats.ddrMemFree);
          obj.Set("ddrMemTotal", stats.ddrMemTotal);
          obj.Set("cmxMemFree", stats.cmxMemFree);
          obj.Set("cmxMemTotal", stats.cmxMemTotal);
          obj.Set("cssTemperature", stats.cssTemperature);
          obj.Set("mssTemperature", stats.mssTemperature);
          obj.Set("upaTemperature", stats.upaTemperature);
          obj.Set("dssTemperature", stats.dssTemperature);
          obj.Set("temperature", stats.temperature);
          callback.Call({obj});
          break;
        }
        case EmitData::INFO: {
          auto obj = Napi::Object::New(env);
          auto info = std::get<DeviceInfo>(data->payload);
          obj.Set("type", "info");
          obj.Set("serialNumber", data->serialNumber);
          obj.Set("state", int(info.state));
          obj.Set("protocol", int(info.protocol));
          obj.Set("platform", int(info.platform));
          obj.Set("productName", info.productName);
          obj.Set("boardName", info.boardName);
          obj.Set("boardRev", info.boardRev);
          obj.Set("bootloaderVersion", info.bootloaderVersion);
          callback.Call({obj});
          break;
        }
        case EmitData::BOOT: {
          auto obj = Napi::Object::New(env);
          auto bootProgress = std::get<DeviceBootProgress>(data->payload);
          obj.Set("type", "boot");
          obj.Set("serialNumber", data->serialNumber);
          obj.Set("stage", DeviceBootProgress::stageToString(bootProgress.stage));
          callback.Call({obj});
          break;
        }
        case EmitData::ERROR: {
          auto obj = Napi::Object::New(env);
          auto error = std::get<DeviceError>(data->payload);
          obj.Set("type", "error");
          obj.Set("serialNumber", data->serialNumber);
          obj.Set("message", error.message);
          callback.Call({obj});
          break;
        }
      }
    }
  }
}