
project(lock_free)
cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")

add_executable(lock_free_test tests.cpp)

target_compile_options(lock_free_test PUBLIC -O3 -march=native)
