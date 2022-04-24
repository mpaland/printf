cmake_minimum_required(VERSION 3.16)
project(printf-4.0.0)

add_executable(printf printf.c)

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(FATAL_ERROR "Clang is required for libFuzzer!")
endif()
target_compile_options(printf PUBLIC -fsanitize=fuzzer)
target_link_options(printf PUBLIC -fsanitize=fuzzer)

