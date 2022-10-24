#include <depthai/depthai.hpp>
#include "build-wrapper.h"
#include "device-wrapper.h"

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  dai::initialize();

  BuildWrapper::Init(env, exports);
  DeviceWrapper::Init(env, exports);
  
  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, InitAll)