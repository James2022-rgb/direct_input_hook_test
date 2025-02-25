cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(TARGET_NAME "hook")

project(${TARGET_NAME}
  VERSION 0.1.0.0
  LANGUAGES CXX
)

option(RENAME_DEVICES "Change the name of the devices as seen by the hooked EXE." ON)
option(MAX_AXIS_VALUES "Always report the maximum value for the axes. Only for the c_dfDIJoystick2 data format." ON)

# --------------------------------------------------------------------------------
# Library
#

add_library(${TARGET_NAME} SHARED hook.def)

set(SOURCE_DIR ".")
set(SOURCES
  ${SOURCE_DIR}/dllmain.cpp
  ${SOURCE_DIR}/wrapped_directinput8.h
  ${SOURCE_DIR}/wrapped_directinputdevice8.h
)

target_sources(${TARGET_NAME} PRIVATE ${SOURCES})

if(RENAME_DEVICES)
  target_compile_definitions(${TARGET_NAME} PRIVATE CONFIG_RENAME_DEVICES=1)
else()
  target_compile_definitions(${TARGET_NAME} PRIVATE CONFIG_RENAME_DEVICES=0)
endif()

if(MAX_AXIS_VALUES)
  target_compile_definitions(${TARGET_NAME} PRIVATE CONFIG_MAX_AXIS_VALUES=1)
else()
  target_compile_definitions(${TARGET_NAME} PRIVATE CONFIG_MAX_AXIS_VALUES=0)
endif()

target_link_libraries(${TARGET_NAME} PRIVATE detours)
