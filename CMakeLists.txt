cmake_minimum_required(VERSION 3.18)

project(listening)

add_executable(${PROJECT_NAME}
    listening.cc)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
