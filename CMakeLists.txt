cmake_minimum_required(VERSION 3.16)
if(POLICY CMP0079)
    cmake_policy(SET CMP0079 NEW)
endif()
cmake_policy(SET CMP0077 NEW)

set(CMAKE_FIND_LIBRARY_SUFFIXES .a .lib)
if (NOT CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/cmake/toolchain/pic.cmake")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb3")

# For IDE
if (NOT CMAKE_JS_VERSION)
    set(CMAKE_JS_VERSION "7.0.0")
    set(NODE_RUNTIMEVERSION "18.5.0")
    set(NODE_RUNTIME "node")
    set(NODE_ARCH "x64")
    set(CMAKE_JS_INC "~/.cmake-js/node-x64/v18.5.0/include/node")
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

project(depthai_node VERSION "0.0.1")

# Add module paths
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/depthai-core/cmake")

set(DEPTHAI_OPENCV_SUPPORT OFF CACHE BOOL "" FORCE)
add_subdirectory(depthai-core)

#find_package(PkgConfig REQUIRED)
find_library(libusb-1.0 NAMES usb-1.0 PATHS
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    /sw/lib
)
#pkg_check_modules(libusb REQUIRED libusb-1.0)

include_directories(${CMAKE_JS_INC})
add_library(${PROJECT_NAME} SHARED
    src/main.cpp
    src/build-wrapper.cpp
    src/device-wrapper.cpp
    ${CMAKE_JS_SRC}
)

add_definitions(-DNAPI_VERSION=3)

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/node_modules/node-addon-api
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/node_modules/napi-thread-safe-callback-cancellable
)

# Set compiler features (c++14), and disables extensions
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_EXTENSIONS OFF)

set(LINK_LIBRARIES
    ${CMAKE_JS_LIB}
    depthai::core
    usb-1.0
)

if(APPLE)
    #
elseif(UNIX)
    list(APPEND LINK_LIBRARIES -static-libgcc -static-libstdc++)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE ${LINK_LIBRARIES})