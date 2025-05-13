cmake_minimum_required(VERSION 3.25)


if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build configuration specified, defaulting to Release")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build configuration" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

option(GRPC_LITE_ENABLE_LOG "Enable logging" ON)
if(GRPC_LITE_ENABLE_LOG)
    add_definitions(-DGRPC_LITE_ENABLE_LOG=1)
endif()

option(GRPC_LITE_VERBOSE_LOG "Enable verbose logging" ON)
if(GRPC_LITE_VERBOSE_LOG)
    add_definitions(-DGRPC_LITE_VERBOSE_LOG=1)
endif()


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(grpc-lite VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")


# detect OS
if(WIN32)
    set(GRPC_LITE_WINDOWS TRUE)
    add_definitions(-DGRPC_LITE_WINDOWS=1)
elseif(UNIX AND NOT APPLE)
    set(GRPC_LITE_LINUX TRUE)
    add_definitions(-DGRPC_LITE_LINUX=1)
else()
    message(FATAL_ERROR "Unsupported OS")
endif()

include(dependencies)

if(MSVC)
    add_compile_options("/utf-8")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(
    "${PROJECT_BINARY_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

set(GRPC_LITE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")


# Windows-specific settings
if(GRPC_LITE_WINDOWS)
    add_definitions(-D_WIN32_WINNT=0x0602)
endif()

add_subdirectory(src)
add_subdirectory(examples)