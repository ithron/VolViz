cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

set(DEFAULT_COMPILE_FEATURES
  cxx_alias_templates
  cxx_alignas
  cxx_auto_type
  cxx_binary_literals
  cxx_constexpr
  cxx_decltype
  cxx_decltype_auto
  cxx_default_function_template_args
  cxx_defaulted_functions
  cxx_defaulted_move_initializers
  cxx_delegating_constructors
  cxx_deleted_functions
  cxx_explicit_conversions
  cxx_generalized_initializers
  cxx_generic_lambdas
  cxx_inheriting_constructors
  cxx_inline_namespaces
  cxx_lambdas
  cxx_lambda_init_captures
  cxx_long_long_type
  cxx_noexcept
  cxx_nullptr
  cxx_override
  cxx_range_for
  cxx_raw_string_literals
  cxx_reference_qualified_functions
  cxx_return_type_deduction
  cxx_right_angle_brackets
  cxx_rvalue_references
  cxx_sizeof_member
  cxx_static_assert
  cxx_strong_enums
  cxx_trailing_return_types
  cxx_uniform_initialization
  cxx_variadic_templates
  cxx_template_template_parameters
)

if (("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang") OR
  ("${CMAKE_CXX_COMPILER_ID}" MATCHES "AppleClang"))
  set(DEFAULT_COMPILER_OPTIONS
    -Werror
    -Weverything
#disable warnings
    -Qunused-arguments
    -Wno-unknown-warning-option
    -Wno-c++98-compat
    -Wno-c++98-compat-bind-to-temporary-copy
    -Wno-c++98-compat-pedantic
    -Wno-global-constructors
    -Wno-reserved-id-macro
    -Wno-documentation
    -Wno-documentation-unknown-command
    -Wno-old-style-cast
    -Wno-deprecated
    -Wno-missing-noreturn
    -Wno-used-but-marked-unused
    -Wno-padded
    -Wno-cast-align
    -Wno-double-promotion
    -Wno-sign-compare
  )
endif()
if (${CMAKE_COMPILER_IS_GNUCXX})
  set(DEFAULT_COMPILER_OPTIONS
    -Werror
    -Wall -Wextra -pedantic
#disable warnings
    -Wno-unknown-pragmas
    -Wno-comment
  )
endif()

add_library(VolViz
  AxisAlignedPlane.cpp
  Camera.cpp
  Cube.cpp
  Geometry.cpp
  GeometryDescriptor.cpp
  GeometryFactory.cpp
  Mesh.cpp
  Shaders.cpp
  Visualizer.cpp
  VisualizerImpl.cpp
  # GL related sources
  GL/GLFW.cpp
  GL/ShaderProgram.cpp
  GL/Shaders.cpp
  GL/glad.c
)
if (BUILD_SHARED_LIBS)
  target_link_libraries(VolViz
    PUBLIC
      Eigen3::Eigen
      Microsoft.GSL::GSL
      PhysUnits::PhysUnits
    PRIVATE
      glfw
      ${MKL_TARGET}
      ConcurrentQueue
  )
else()
  target_link_libraries(VolViz
    PUBLIC
      ${MKL_TARGET}
      Eigen3::Eigen
      Microsoft.GSL::GSL
      PhysUnits::PhysUnits
      glfw
    PRIVATE
      ConcurrentQueue
  )
endif()
target_include_directories(VolViz PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include/VolViz/src>
  $<INSTALL_INTERFACE:include/VolViz>
  $<INSTALL_INTERFACE:include/VolViz/src>
)
target_compile_features(VolViz PUBLIC ${DEFAULT_COMPILE_FEATURES})
target_compile_options(VolViz PRIVATE ${DEFAULT_COMPILER_OPTIONS})

if (NOT WIN32) 
  target_link_libraries(VolViz PUBLIC dl)
endif() 

set(INSTALL_TARGETS VolViz)

add_library(VolViz::VolViz ALIAS VolViz)

if (BUILD_TESTING)
#add test targets here
endif()


###############################
#Install and packaging
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(
  TARGETS
    ${INSTALL_TARGETS}
  EXPORT
    VolVizExport
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
export(
  TARGETS ${INSTALL_TARGETS}
  FILE VolVizExport.cmake
)
install(EXPORT VolVizExport NAMESPACE VolViz::
  DESTINATION ${CMAKE_INSTALL_DATADIR}/VolViz/cmake
  FILE VolVizExport.cmake
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../include/VolViz
  DESTINATION include
)

export(EXPORT VolVizExport NAMESPACE VolViz:: FILE ${CMAKE_BINARY_DIR}/VolVizExport.cmake)
configure_package_config_file(VolVizConfig.cmake.in
  ${CMAKE_BINARY_DIR}/VolVizConfig.cmake
  INSTALL_DESTINATION lib/cmake/VolViz
)
install(FILES
  ${CMAKE_BINARY_DIR}/VolVizExport.cmake
  ${CMAKE_BINARY_DIR}/VolVizConfig.cmake
  DESTINATION lib/cmake/VolViz
)

export(PACKAGE VolViz)

