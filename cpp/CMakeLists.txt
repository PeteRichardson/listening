cmake_minimum_required(VERSION 3.18)

project(listening)

SET(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build architectures for Mac OS X" FORCE)

# Make a static libfort library with only fort.c and fort.hpp
# This bypasses libfort 4.2's use of cmake_minimum_required(VERSION 3.0) which is
# no longer supported by recent cmake.  (lowest acceptable VERSION is 3.5 now).
# This assumes that libfort source actually compiles with newer cmake.
include(FetchContent)
FetchContent_Declare(
    libfort_src
    GIT_REPOSITORY https://github.com/seleznevae/libfort
    GIT_TAG        v0.4.2
)
FetchContent_GetProperties(libfort_src)
if(NOT libfort_src_POPULATED)
  FetchContent_Populate(libfort_src)
endif()
set(LIBFORT_DIR "${libfort_src_SOURCE_DIR}/lib")
add_library(libfort STATIC
  "${LIBFORT_DIR}/fort.c"
)

add_executable(listening
    listening.cc
    listener.cc
)

target_compile_features(listening PUBLIC cxx_std_20)
target_include_directories(listening PUBLIC "${LIBFORT_DIR}")
target_link_libraries(listening PRIVATE libfort)

add_executable(listener_tests
    test/listener_test.cc
    listener.cc
)
target_compile_features(listener_tests PUBLIC cxx_std_20)
target_include_directories(listener_tests PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
