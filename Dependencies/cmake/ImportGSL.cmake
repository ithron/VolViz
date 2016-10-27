cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

if (NOT GSL)

  set(INSTALL_GSL ON)

  add_library(GSL INTERFACE)

  option(GSL_THROW_ON_CONTRACT_VALIDATION "Sould GSL throw on contract validation?" YES)

  if (XCODE)
    set_property(TARGET GSL PROPERTY INTERFACE_COMPILE_OPTIONS
      -isystem$<BUILD_INTERFACE:${DEPENDENCIES_DIR}/GSL/include>$<INSTALL_INTERFACE:include/VolViz/src/GSL>
    )
  else()
    set_property(TARGET GSL PROPERTY INTERFACE_INCLUDE_DIRECTORIES
      $<BUILD_INTERFACE:${DEPENDENCIES_DIR}/GSL/include>
      $<INSTALL_INTERFACE:include/VolViz/src/GSL>
    )
    set_property(TARGET GSL PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
      $<BUILD_INTERFACE:${DEPENDENCIES_DIR}/GSL/include>
      $<INSTALL_INTERFACE:include/VolViz/src/GSL>
    )
  endif()

  if(GSL_THROW_ON_CONTRACT_VALIDATION)
    set_property(TARGET GSL PROPERTY INTERFACE_COMPILE_DEFINITIONS
      GSL_THROW_ON_CONTRACT_VIOLATION
    )
  endif()
endif()

install(TARGETS GSL EXPORT VolVizExport)

if (INSTALL_GSL)
  install(DIRECTORY
    ${DEPENDENCIES_DIR}/GSL/include
    DESTINATION include/VolViz/src/GSL
  )
endif()
