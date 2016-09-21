exec_program(${CMAKE_COMMAND} ${CMAKE_BINARY_DIR}/glfw
  ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/glfw ${DEPENDENCIES_DIR}/glfw
)

exec_program(${CMAKE_COMMAND} ${CMAKE_BINARY_DIR}/glfw
  ARGS --build . --target install
)

set(glfw3_DIR "${CMAKE_BINARY_DIR}/glfw/lib/cmake/glfw3")
find_package(glfw3 3.2 REQUIRED)

