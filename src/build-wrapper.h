#ifndef BUILD_WRAPPER_H
#define BUILD_WRAPPER_H

#include <iostream>
#include <string>

#include <napi.h>

class BuildWrapper
{
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
};

#endif // BUILD_WRAPPER_H
