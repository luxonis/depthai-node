#ifndef PIPELINE_WRAPPER_H
#define PIPELINE_WRAPPER_H

#include <iostream>
#include <string>

#include <napi.h>
#include <depthai/depthai.hpp>

class PipelineWrapper : public Napi::ObjectWrap<PipelineWrapper>
{
public:
  static Napi::FunctionReference constructor;
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  PipelineWrapper(const Napi::CallbackInfo &info);
  ~PipelineWrapper();
private:
    static std::unordered_set<PipelineWrapper*> instances;

    std::shared_ptr<dai::Pipeline> mPipelinePtr = nullptr;

    void doClose();
}

#endif // PIPELINE_WRAPPER_H