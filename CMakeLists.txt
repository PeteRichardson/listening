cmake_minimum_required(VERSION 3.18)

include(FetchContent)
FetchContent_Declare(
    libfort
    GIT_REPOSITORY https://github.com/seleznevae/libfort
    GIT_TAG        v0.4.2
)
FetchContent_MakeAvailable(libfort)

project(listening)

add_executable(${PROJECT_NAME}
    listening.cc)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_link_libraries(${PROJECT_NAME} libfort)