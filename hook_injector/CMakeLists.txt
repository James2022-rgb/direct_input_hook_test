cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(TARGET_NAME "hook_injector")

project(${TARGET_NAME}
  VERSION 0.1.0.0
  LANGUAGES CXX
)

# --------------------------------------------------------------------------------
# External Targets
#

set(EXTERNAL_DIR "external")

#
# Microsoft Detours
#

set(DETOURS_DIR "${EXTERNAL_DIR}/detours")
set(DETOURS_SOURCES
  ${DETOURS_DIR}/src/creatwth.cpp
  ${DETOURS_DIR}/src/detours.cpp
  ${DETOURS_DIR}/src/detours.h
  ${DETOURS_DIR}/src/detver.h
  ${DETOURS_DIR}/src/disasm.cpp
  ${DETOURS_DIR}/src/disolarm.cpp
  ${DETOURS_DIR}/src/disolarm64.cpp
  ${DETOURS_DIR}/src/disolia64.cpp
  ${DETOURS_DIR}/src/disolx64.cpp
  ${DETOURS_DIR}/src/disolx86.cpp
  ${DETOURS_DIR}/src/image.cpp
  ${DETOURS_DIR}/src/modules.cpp
  ${DETOURS_DIR}/src/uimports.cpp
)

set_property(SOURCE ${DETOURS_DIR}/src/uimports.cpp APPEND PROPERTY HEADER_FILE_ONLY true)

add_library(detours STATIC ${DETOURS_SOURCES})

target_include_directories(detours PUBLIC ${DETOURS_DIR}/src)

# --------------------------------------------------------------------------------
# Executable
#

add_executable(${TARGET_NAME})

set(SOURCE_DIR ".")
set(SOURCES
  ${SOURCE_DIR}/main.cpp
)

target_sources(${TARGET_NAME} PRIVATE ${SOURCES})

target_link_libraries(${TARGET_NAME} PRIVATE detours)

target_link_libraries(${TARGET_NAME} PRIVATE hook)
