#include <depthai/build/version.hpp>
#include "build-wrapper.h"

Napi::Object BuildWrapper::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    auto obj = Napi::Object::New(env);
    obj.Set("VERSION", dai::build::VERSION);
    obj.Set("BUILD_DATETIME", dai::build::BUILD_DATETIME);
    obj.Set("COMMIT", dai::build::COMMIT);
    obj.Set("COMMIT_DATETIME", dai::build::COMMIT_DATETIME);
    exports.Set("Build", obj);

    return exports;
}