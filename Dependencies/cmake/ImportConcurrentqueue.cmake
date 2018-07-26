cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

if(NOT TARGET ConcurrentQueue)

  include(ExternalProject)

  ExternalProject_Add(ConcurrentQueue_EP
    URL https://github.com/cameron314/concurrentqueue/archive/v1.0.0-beta.tar.gz
    URL_MD5 761446e2392942aa342f437697ddb72e
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
  )

  ExternalProject_Get_Property(ConcurrentQueue_EP SOURCE_DIR)
  message("CQ SOURCE_DIR=${SOURCE_DIR}")

  add_library(ConcurrentQueue INTERFACE)
  if("${CMAKE_GENERATOR}" STREQUAL "Xcode")
    set_property(TARGET ConcurrentQueue PROPERTY INTERFACE_COMPILE_OPTIONS
      $<BUILD_INTERFACE:-isystem ${SOURCE_DIR}>
    )
  else()
    set_property(TARGET ConcurrentQueue PROPERTY INTERFACE_INCLUDE_DIRECTORIES
      $<BUILD_INTERFACE:${SOURCE_DIR}>
    )
    set_property(TARGET ConcurrentQueue PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
      $<BUILD_INTERFACE:${SOURCE_DIR}>
    )
  endif()

  add_dependencies(ConcurrentQueue INTERFACE ConcurrentQueue_EP)

  install(TARGETS ConcurrentQueue EXPORT VolVizExport)

endif()

