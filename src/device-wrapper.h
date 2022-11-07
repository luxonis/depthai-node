#ifndef DEVICE_WRAPPER_H
#define DEVICE_WRAPPER_H

#include <iostream>
#include <string>

#include <napi.h>
#include <depthai/depthai.hpp>

class DeviceWrapper : public Napi::ObjectWrap<DeviceWrapper>
{
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    static Napi::Value getAllAvailableDevices(const Napi::CallbackInfo &info);
    static Napi::Value getAllConnectedDevices(const Napi::CallbackInfo &info);
    static void CloseAll();

    DeviceWrapper(const Napi::CallbackInfo &info);
    ~DeviceWrapper();
private:
    static std::unordered_set<DeviceWrapper*> instances;

    static Napi::Array deviceInfosToNode(Napi::Env env, std::vector<dai::DeviceInfo> infos);
    static Napi::String nodeEnumFromState(Napi::Env env, XLinkDeviceState_t state);
    static Napi::String nodeEnumFromProtocol(Napi::Env env, XLinkProtocol_t protocol);
    static Napi::String nodeEnumFromPlatform(Napi::Env env, XLinkPlatform_t platform);
    static Napi::String nodeEnumFromStatus(Napi::Env env, XLinkError_t status);

    std::shared_ptr<dai::Device> mDevicePtr = nullptr;

    void doClose();
};

#endif // DEVICE_WRAPPER_H
