cmake_minimum_required(VERSION 3.18)

SET(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build architectures for Mac OS X" FORCE)

include(FetchContent)
FetchContent_Declare(
    libfort
    GIT_REPOSITORY https://github.com/seleznevae/libfort
    GIT_TAG        v0.4.2
)
FetchContent_MakeAvailable(libfort)

project(listening)

add_executable(listening
    listening.cc
    listener.cc)

target_compile_features(listening PUBLIC cxx_std_20)
target_link_libraries(listening PRIVATE fort)
