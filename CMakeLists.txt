cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})

set(DEPENDENCIES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${DEPENDENCIES_DIR}/cmake")

project(VolViz VERSION 0.1.1 LANGUAGES CXX)

# Set the default build type if none was specified
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Debug' as none was specified.")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the build type." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo")
endif()

# import dependencies
find_package(OpenGL REQUIRED 4.5)
include(ImportEigen)
include(ImportGLFW)
include(ImportMKL)


option(BUILD_TESTING "Build tests" ON)

if(BUILD_TESTING)
  enable_testing()
endif()

configure_file(
  "${PROJECT_SOURCE_DIR}/Source/config.h.in" "${PROJECT_BINARY_DIR}/config.h"
)

add_subdirectory(Source VolViz)


