cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(TARGET_NAME "direct_input_hook_test")

project(${TARGET_NAME}
  VERSION 0.1.0.0
  LANGUAGES CXX
)

add_subdirectory("hook")
add_subdirectory("hook_injector")
