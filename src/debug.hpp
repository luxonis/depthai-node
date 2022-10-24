#pragma once

#define DISCOVERY_RELEASE
#ifndef DISCOVERY_RELEASE
#define LOG(x) std::cerr << x << std::endl;
#else
#define LOG(x)
#endif // DEBUG