#include "deviceMonitor.hpp"
#include "instanceData.hpp"
#include "debug.hpp"

extern RtcService rtcService;

StopMonitorWorker::StopMonitorWorker(const Napi::Function& callback, std::string mxId)
    : Napi::AsyncWorker(callback, "discovery:StopMonitorWorker"), mxId(std::move(mxId)) {
  discovery = &Env().GetInstanceData<InstanceData>()->discovery;
}

void StopMonitorWorker::Execute() {
  auto removedPipeline = discovery->removePipeline(mxId);

  while(removedPipeline.use_count()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

void StopMonitorWorker::OnOK() {
  Callback().Call({Env().Undefined()});
}

MonitorWorker::MonitorWorker(const Napi::Function& callback, const Napi::Function& monitorCallback, std::string mxId)
    : Napi::AsyncWorker(callback, "discovery:MonitorWorker"), mxId(std::move(mxId)) {
  safeMonitorCallback = MonitorCallback::New(Env(), monitorCallback, "discovery:monitor-emit", 60, 1);
  discovery = &Env().GetInstanceData<InstanceData>()->discovery;
}

void MonitorWorker::Execute() {
  dai::DeviceInfo device;
  bool foundDevice;
  std::tie(foundDevice, device) = discovery->getDeviceByMxId(mxId);

  if(!foundDevice) {
    SetError("Device not found");
  } else if(!discovery->addPipeline(device, safeMonitorCallback)) {
    SetError("Device is already being monitored");
  }
}

void MonitorWorker::OnError(const Napi::Error& e) {
  safeMonitorCallback.Release();
  Callback().Call({e.Value(), Napi::Boolean::New(Env(), false)});
}

void MonitorWorker::OnOK() {
  Callback().Call({Env().Undefined(), Napi::Boolean::New(Env(), true)});
}

ListDevicesWorker::ListDevicesWorker(const Napi::Function& callback) : Napi::AsyncWorker(callback, "discovery:ScanWorker") {
  discovery = &Env().GetInstanceData<InstanceData>()->discovery;
}

void ListDevicesWorker::Execute() {
  devices = discovery->getAllDevices();
#if DEBUG
  for(const auto& device : devices) {
    LOG("Found device: " << device.getMxId() << " state: " << XLinkDeviceStateToStr(device.state) << " protocol: " << XLinkProtocolToStr(device.protocol));
  }
#endif
}

void ListDevicesWorker::OnOK() {
  Napi::HandleScope scope(Env());
  auto result = Napi::Array::New(Env(), devices.size());
  auto index = 0;
  for(const auto& device : devices) {
    auto obj = Napi::Object::New(Env());
    obj.Set("serialNumber", device.mxid);
    obj.Set("name", device.name);
    obj.Set("state", int(device.state));
    obj.Set("protocol", int(device.protocol));
    obj.Set("platform", int(device.platform));
    obj.Set("status", int(device.status));
    result.Set(index++, obj);
  }
  Callback().Call({Env().Undefined(), result});
}

Napi::Value monitor(const Napi::CallbackInfo& info) {
  if(info.Length() < 3) {
    throw Napi::TypeError::New(info.Env(), "Expected three arguments");
  }
  if(!info[0].IsString()) {
    throw Napi::TypeError::New(info.Env(), "Expected first argument to be string");
  }
  if(!info[1].IsFunction()) {
    throw Napi::TypeError::New(info.Env(), "Expected second argument to be function");
  }
  if(!info[2].IsFunction()) {
    throw Napi::TypeError::New(info.Env(), "Expected third argument to be function");
  }

  auto mxId = info[0].As<Napi::String>().Utf8Value();
  auto monitorCallback = info[1].As<Napi::Function>();
  auto callback = info[2].As<Napi::Function>();

  auto discovery = &info.Env().GetInstanceData<InstanceData>()->discovery;
  if(discovery->hasPipeline(mxId)) {
    callback.Call({info.Env().Undefined(), Napi::Boolean::New(info.Env(), true)});
    return info.Env().Undefined();
  }

  auto* monitorWorker = new MonitorWorker(callback, monitorCallback, mxId);
  monitorWorker->Queue();

  return info.Env().Undefined();
}

Napi::Value stopMonitor(const Napi::CallbackInfo& info) {
  if(info.Length() < 2) {
    throw Napi::TypeError::New(info.Env(), "Expected two arguments");
  }
  if(!info[0].IsString()) {
    throw Napi::TypeError::New(info.Env(), "Expected first argument to be string");
  }
  if(!info[1].IsFunction()) {
    throw Napi::TypeError::New(info.Env(), "Expected second argument to be function");
  }

  auto mxId = info[0].As<Napi::String>().Utf8Value();
  auto callback = info[1].As<Napi::Function>();

  auto discovery = &info.Env().GetInstanceData<InstanceData>()->discovery;
  if(!discovery->hasPipeline(mxId)) {
    callback.Call({info.Env().Undefined()});
    return info.Env().Undefined();
  }

  auto* worker = new StopMonitorWorker(callback, mxId);
  worker->Queue();

  return info.Env().Undefined();
}

Napi::Value listDevices(const Napi::CallbackInfo& info) {
  if(info.Length() < 1) {
    throw Napi::TypeError::New(info.Env(), "Expected one argument");
  }
  if(!info[0].IsFunction()) {
    throw Napi::TypeError::New(info.Env(), "Expected first argument to be function");
  }
  auto callback = info[0].As<Napi::Function>();

  auto* listDevicesWorker = new ListDevicesWorker(callback);
  listDevicesWorker->Queue();

  return info.Env().Undefined();
}