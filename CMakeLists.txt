cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})

set(DEPENDENCIES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${DEPENDENCIES_DIR}/cmake")

option(BUILD_SHARED_LIBS "Build shared instead of static libraries" OFF)
option(BUILD_TESTING "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)
option(FIX_MISSING_GL_IMPLEMENTATIONS "Fix missing GL function implementations" OFF)

if (BUILD_SHARED_LIBS)
  set(HUNTER_BUILD_SHARED_LIBS ON)
endif()

if (${BUILD_EXAMPLES})
  include(InitSubmodules)
endif()
include(HunterGate)

HunterGate(
  URL "https://github.com/ruslo/hunter/archive/v0.23.34.tar.gz"
  SHA1 "70287b1ffa810ee4e952052a9adff9b4856d0d54"
  FILEPATH ${DEPENDENCIES_DIR}/cmake/Hunter/config.cmake
)

project(VolViz VERSION 0.1.2 LANGUAGES C CXX)


# Set the default build type if none was specified
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Debug' as none was specified.")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the build type." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo")
endif()

# import dependencies
include_directories(SYSTEM ${DEPENDENCIES_DIR}/GLHeaders)

set(PATH_TO_GL_LIBS "/usr/lib" CACHE PATH "Path to OpenGL libraries")
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${PATH_TO_GL_LIBS})
if (APPLE)
  find_package(OpenGL REQUIRED)
else()
  find_package(OpenGL 4.5 REQUIRED COMPONENTS OpenGL)
endif()
message(STATUS "Found OpenGL libraries: ${OPENGL_LIBRARIES}")
include(ImportMKL)

hunter_add_package(Eigen)
find_package(Eigen3 CONFIG REQUIRED)

hunter_add_package(Microsoft.GSL)
find_package(Microsoft.GSL CONFIG REQUIRED)

hunter_add_package(glfw)
find_package(glfw3 REQUIRED)

hunter_add_package(PhysUnits)
find_package(PhysUnits CONFIG REQUIRED)

include(ImportConcurrentqueue)

find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY
  )
  if(NOT TARGET doc)
    add_custom_target(doc
      ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
  endif()
endif()


if(BUILD_TESTING)
  enable_testing()
endif()

configure_file(
  "${PROJECT_SOURCE_DIR}/Source/config.h.in" "${PROJECT_BINARY_DIR}/config.h"
)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(Source VolViz)
if (BUILD_EXAMPLES)
  add_subdirectory(Examples Examples)
endif()

