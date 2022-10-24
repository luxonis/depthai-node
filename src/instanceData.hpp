#pragma once
#include "discovery.hpp"

struct InstanceData {
  Discovery discovery;
};

static void DeleteInstanceData(Napi::Env env, InstanceData* instanceData) {
  delete instanceData;
}