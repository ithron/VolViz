cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

if (NOT HAS_IGL)
  set(HAS_IGL YES)

  add_library(IGL INTERFACE)
  if(XCODE)
    set_property(TARGET IGL PROPERTY INTERFACE_COMPILE_OPTIONS
      -isystem$<BUILD_INTERFACE:${DEPENDENCIES_DIR}/libigl/include>$<INSTALL_INTERFACE:include/VolViz/src/IGL>
    )
  else()
    set_property(TARGET IGL PROPERTY INTERFACE_INCLUDE_DIRECTORIES
      $<BUILD_INTERFACE:${DEPENDENCIES_DIR}/libigl/include>
      $<INSTALL_INTERFACE:include/VolViz/src/IGL>
    )
    set_property(TARGET IGL PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
      $<BUILD_INTERFACE:${DEPENDENCIES_DIR}/libigl/include>
      $<INSTALL_INTERFACE:include/VolViz/src/IGL>
    )
  endif()

  install(TARGETS IGL EXPORT VolVizExport)
endif()
