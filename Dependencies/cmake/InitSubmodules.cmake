cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

find_package(Git REQUIRED)

message("===== Init submodules =====")

execute_process(
  COMMAND "${GIT_EXECUTABLE}" submodule update --init
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

message("===== Submodules initialized =====")

