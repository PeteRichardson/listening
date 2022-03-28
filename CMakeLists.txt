cmake_minimum_required(VERSION 3.18)


IF(APPLE)
  SET(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build architectures for Mac OS X" FORCE)
ENDIF(APPLE)

project(listening)

add_executable(listening
    listening.cc)

target_compile_features(listening PUBLIC cxx_std_20)
# Disable building tests and examples in libfort project
set(FORT_ENABLE_TESTING OFF CACHE INTERNAL "")
add_subdirectory(third-party/libfort)
target_link_libraries(listening PRIVATE fort)