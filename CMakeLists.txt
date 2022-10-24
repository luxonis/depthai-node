cmake_minimum_required(VERSION 3.16)
if(POLICY CMP0079)
    cmake_policy(SET CMP0079 NEW)
endif()
cmake_policy(SET CMP0077 NEW)

set(CMAKE_FIND_LIBRARY_SUFFIXES .a .lib)
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/cmake/toolchain/pic.cmake")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb3")

if (NOT CMAKE_JS_VERSION)
    set(CMAKE_JS_VERSION "6.3.0")
    set(NODE_RUNTIMEVERSION "17.3.1")
    set(NODE_RUNTIME "node")
    set(NODE_ARCH "x64")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}")
    set(CMAKE_JS_INC "~/.cmake-js/node-x64/v17.3.1/include/node")
    set(CMAKE_JS_SRC "")
endif()


## Build dependencies as 'Release' only by default (if not MSVC, as it requires same debug level for libraries to be linked against)
if(NOT WIN32)
    set(HUNTER_CONFIGURATION_TYPES "Release" CACHE STRING "Hunter dependencies list of build configurations")
endif()

include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/cpp-pm/hunter/archive/v0.23.320.tar.gz"
    SHA1 "9b4e732afd22f40482c11ad6342f7d336634226f"
    FILEPATH ${CMAKE_CURRENT_LIST_DIR}/depthai-core/cmake/Hunter/config.cmake
)

project(discovery VERSION "0")

# Add module paths
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/depthai-core/cmake")

set(DEPTHAI_OPENCV_SUPPORT OFF CACHE BOOL "" FORCE)
add_subdirectory(depthai-core)

set(NO_WEBSOCKET ON CACHE BOOL "" FORCE)
set(NO_EXAMPLES ON CACHE BOOL "" FORCE)
set(NO_TESTS ON CACHE BOOL "" FORCE)
set(USE_NICE OFF CACHE BOOL "" FORCE)
set(OPENSSL_USE_STATIC_LIBS ON CACHE BOOL "" FORCE)
add_subdirectory(libdatachannel)
target_compile_definitions(srtp2 PRIVATE PACKAGE_VERSION="2.4.2") # this is for some reason missing in the dependency

# Include node-addon-api wrappers
execute_process(COMMAND node -p "require('node-addon-api').include"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE NODE_ADDON_API_DIR
        )
string(REPLACE "\n" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})

include_directories(${CMAKE_JS_INC})
set(DISCOVERY_SRC src/main.cpp src/streamMonitor.cpp src/pipeline.cpp src/discovery.cpp src/rtcService.cpp src/rtcStream.cpp src/uuid.cpp src/deviceMonitor.cpp src/device.cpp)
add_library(${PROJECT_NAME} SHARED ${DISCOVERY_SRC} ${CMAKE_JS_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_link_libraries(${PROJECT_NAME} PRIVATE LibDataChannel::LibDataChannelStatic depthai::core)
target_include_directories(${PROJECT_NAME} PRIVATE ${NODE_ADDON_API_DIR})

# Set compiler features (c++14), and disables extensions
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_EXTENSIONS OFF)
