#include "device-wrapper.h"

Napi::Object DeviceWrapper::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    auto obj = Napi::Object::New(env);
    obj.Set("getAllAvailableDevices", Napi::Function::New(env, &DeviceWrapper::getAllAvailableDevices));
    obj.Set("getAllConnectedDevices", Napi::Function::New(env, &DeviceWrapper::getAllConnectedDevices));
    exports.Set("Device", obj);

    return exports;
}

Napi::Value DeviceWrapper::getAllAvailableDevices(const Napi::CallbackInfo& info) 
{
    return DeviceWrapper::deviceInfosToNode(info.Env(), dai::Device::getAllAvailableDevices());
}

Napi::Value DeviceWrapper::getAllConnectedDevices(const Napi::CallbackInfo& info) 
{
    return DeviceWrapper::deviceInfosToNode(info.Env(), dai::Device::getAllConnectedDevices());
}

Napi::Array DeviceWrapper::deviceInfosToNode(Napi::Env env, std::vector<dai::DeviceInfo> infos)
{
    auto nodeArray = Napi::Array::New(env, infos.size());
    for(int i = 0;i < infos.size();i++) {
        auto deviceInfo = Napi::Object::New(env);
        deviceInfo.Set("name", Napi::String::New(env, infos[i].name));
        deviceInfo.Set("mxid", Napi::String::New(env, infos[i].mxid));
        deviceInfo.Set("state", DeviceWrapper::nodeEnumFromState(env, infos[i].state));
        deviceInfo.Set("protocol", DeviceWrapper::nodeEnumFromProtocol(env, infos[i].protocol));
        deviceInfo.Set("platform", DeviceWrapper::nodeEnumFromPlatform(env, infos[i].platform));
        deviceInfo.Set("status", DeviceWrapper::nodeEnumFromStatus(env, infos[i].status));
        nodeArray[i] = deviceInfo;
    }
    return nodeArray;
} 

Napi::String DeviceWrapper::nodeEnumFromState(Napi::Env env, XLinkDeviceState_t state)
{
    switch (state) {
        case X_LINK_ANY_STATE:
            return Napi::String::New(env, "unknown");
        case X_LINK_BOOTED:
            return Napi::String::New(env, "booted");
        case X_LINK_UNBOOTED:
            return Napi::String::New(env, "unbooted");
        case X_LINK_BOOTLOADER:
            return Napi::String::New(env, "bootloader");
        case X_LINK_FLASH_BOOTED:
            return Napi::String::New(env, "flash-booted");
    }
}

Napi::String DeviceWrapper::nodeEnumFromProtocol(Napi::Env env, XLinkProtocol_t protocol)
{
    switch (protocol) {
        case X_LINK_USB_VSC:
            return Napi::String::New(env, "usb-vsc");
        case X_LINK_USB_CDC:
            return Napi::String::New(env, "usb-cdc");
        case X_LINK_PCIE:
            return Napi::String::New(env, "pcie");
        case X_LINK_IPC:
            return Napi::String::New(env, "ipc");
        case X_LINK_TCP_IP:
            return Napi::String::New(env, "tcp-ip");
        case X_LINK_NMB_OF_PROTOCOLS:
            return Napi::String::New(env, "nmb-of-protocols");
        case X_LINK_ANY_PROTOCOL:
            return Napi::String::New(env, "unknown");
    }
}

Napi::String DeviceWrapper::nodeEnumFromPlatform(Napi::Env env, XLinkPlatform_t platform)
{
    switch (platform) {
        case X_LINK_ANY_PLATFORM:
            return Napi::String::New(env, "unknown");
        case X_LINK_MYRIAD_2:
            return Napi::String::New(env, "myriad-2");
        case X_LINK_MYRIAD_X:
            return Napi::String::New(env, "myriad-x");
    }
}

Napi::String DeviceWrapper::nodeEnumFromStatus(Napi::Env env, XLinkError_t status)
{
    switch (status) {
        case X_LINK_SUCCESS:
            return Napi::String::New(env, "success");
        case X_LINK_ALREADY_OPEN:
            return Napi::String::New(env, "already-open");
        case X_LINK_COMMUNICATION_NOT_OPEN:
            return Napi::String::New(env, "communication-not-open");
        case X_LINK_COMMUNICATION_FAIL:
            return Napi::String::New(env, "communication-fail");
        case X_LINK_COMMUNICATION_UNKNOWN_ERROR:
            return Napi::String::New(env, "communication-unknown-error");
        case X_LINK_DEVICE_NOT_FOUND:
            return Napi::String::New(env, "device-not-found");
        case X_LINK_TIMEOUT:
            return Napi::String::New(env, "timeout");
        case X_LINK_ERROR:
            return Napi::String::New(env, "error");
        case X_LINK_OUT_OF_MEMORY:
            return Napi::String::New(env, "out-of-memory");
        case X_LINK_INSUFFICIENT_PERMISSIONS:
            return Napi::String::New(env, "insufficient-permissions");
        case X_LINK_DEVICE_ALREADY_IN_USE:
            return Napi::String::New(env, "device-already-in-use");
        case X_LINK_NOT_IMPLEMENTED:
            return Napi::String::New(env, "not-implemented");
        case X_LINK_INIT_USB_ERROR:
            return Napi::String::New(env, "usb-error");
        case X_LINK_INIT_TCP_IP_ERROR:
            return Napi::String::New(env, "tcp-ip-error");
        case X_LINK_INIT_PCIE_ERROR:
            return Napi::String::New(env, "pcie-error");
    }
}
