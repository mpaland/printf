cmake_minimum_required(VERSION 3.10)

project(printf VERSION 4.1.0 LANGUAGES C)   # x-release-please-version

# build the tests by default only if this is the top level project
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  set(PRINTF_IS_TOP_LEVEL ON)
else()
  set(PRINTF_IS_TOP_LEVEL OFF)
endif()

option(PRINTF_BUILD_TESTS                  "Build the test suite"                       ${PRINTF_IS_TOP_LEVEL})
option(PRINTF_BUILD_AUTOTEST               "Build the random comparison test (autotest)" OFF)
option(PRINTF_DISABLE_SUPPORT_FLOAT        "Disable support for %f"                     OFF)
option(PRINTF_DISABLE_SUPPORT_EXPONENTIAL  "Disable support for %e and %g"              OFF)
option(PRINTF_DISABLE_SUPPORT_LONG_LONG    "Disable support for long long types"        OFF)
option(PRINTF_DISABLE_SUPPORT_PTRDIFF_T    "Disable support for the ptrdiff_t type"     OFF)

# collect the compiler switches selected above
set(PRINTF_DEFINES "")
foreach(feature FLOAT EXPONENTIAL LONG_LONG PTRDIFF_T)
  if(PRINTF_DISABLE_SUPPORT_${feature})
    list(APPEND PRINTF_DEFINES PRINTF_DISABLE_SUPPORT_${feature})
  endif()
endforeach()

if(MSVC)
  set(PRINTF_WARNINGS /W4)
else()
  set(PRINTF_WARNINGS -Wall -Wextra -pedantic)
endif()


# the library
add_library(printf printf.c printf.h)
target_include_directories(printf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(printf PRIVATE ${PRINTF_DEFINES})
target_compile_options(printf PRIVATE ${PRINTF_WARNINGS})
set_target_properties(printf PROPERTIES C_STANDARD 99 C_STANDARD_REQUIRED ON C_EXTENSIONS OFF)


# the tests
if(PRINTF_BUILD_TESTS)
  enable_language(CXX)
  enable_testing()

  # the Catch2 v3 test framework (amalgamated files, provides main()), compiled without our warning flags
  add_library(catch2 STATIC test/catch_amalgamated.cpp)
  target_include_directories(catch2 SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/test)
  target_compile_options(catch2 PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)
  set_target_properties(catch2 PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF
                                          COMPILE_WARNING_AS_ERROR OFF)

  # the test suite includes printf.c directly, so it must not link the library
  add_executable(test_suite test/test_suite.cpp)
  target_link_libraries(test_suite PRIVATE catch2)
  target_compile_definitions(test_suite PRIVATE ${PRINTF_DEFINES})
  target_compile_options(test_suite PRIVATE ${PRINTF_WARNINGS} $<$<CXX_COMPILER_ID:MSVC>:/permissive->)
  set_target_properties(test_suite PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
  add_test(NAME test_suite COMMAND test_suite)

  # check that a C program can use the library via the standard function names
  add_executable(aliasing test/aliasing.c)
  target_link_libraries(aliasing PRIVATE printf)
  target_compile_options(aliasing PRIVATE ${PRINTF_WARNINGS})
  set_target_properties(aliasing PROPERTIES C_STANDARD 99 C_STANDARD_REQUIRED ON C_EXTENSIONS OFF)
  add_test(NAME aliasing COMMAND aliasing)

  # check that the library links without _putchar() when printf_() and vprintf_() are disabled
  add_executable(no_putchar test/no_putchar.c printf.c)
  target_compile_definitions(no_putchar PRIVATE PRINTF_DISABLE_SUPPORT_PUTCHAR)
  target_compile_options(no_putchar PRIVATE ${PRINTF_WARNINGS})
  set_target_properties(no_putchar PROPERTIES C_STANDARD 99 C_STANDARD_REQUIRED ON C_EXTENSIONS OFF)
  add_test(NAME no_putchar COMMAND no_putchar)

  # random comparison against the standard library, runs until stopped (not part of CTest)
  if(PRINTF_BUILD_AUTOTEST)
    add_executable(autotest test/autotest.cpp)
    target_link_libraries(autotest PRIVATE printf)
    target_compile_definitions(autotest PRIVATE ${PRINTF_DEFINES})
    set_target_properties(autotest PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF
                                              COMPILE_WARNING_AS_ERROR OFF)
  endif()
endif()
