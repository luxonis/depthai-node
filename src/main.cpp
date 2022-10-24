#include <napi.h>
#include <depthai/depthai.hpp>
#include <depthai/build/version.hpp>
#include "discovery.hpp"
#include "instanceData.hpp"

Napi::Value depthaiInfo(const Napi::CallbackInfo& info) {
  auto obj = Napi::Object::New(info.Env());
  obj.Set("version", dai::build::VERSION);
  obj.Set("buildDate", dai::build::BUILD_DATETIME);
  obj.Set("commit", dai::build::COMMIT);
  obj.Set("commitDate", dai::build::COMMIT_DATETIME);
  return obj;
}

Napi::Object init(Napi::Env env, Napi::Object exports) {
  dai::initialize();
  rtc::InitLogger(rtc::LogLevel::Debug);
  auto* instanceData = new InstanceData();
  env.SetInstanceData<InstanceData, DeleteInstanceData>(instanceData);

  exports.Set(Napi::String::New(env, "depthaiInfo"), Napi::Function::New(env, depthaiInfo, "depthaiInfo"));
  
  exports.Set(Napi::String::New(env, "listDevices"), Napi::Function::New(env, listDevices, "listDevices"));
  exports.Set(Napi::String::New(env, "monitor"), Napi::Function::New(env, monitor, "monitor"));
  exports.Set(Napi::String::New(env, "stopMonitor"), Napi::Function::New(env, stopMonitor, "stopMonitor"));

  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, init)