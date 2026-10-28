cmake_minimum_required(VERSION 3.10)

project(printf LANGUAGES C)

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

  # the test suite includes printf.c directly, so it must not link the library
  add_executable(test_suite test/test_main.cpp test/test_suite.cpp)
  target_compile_definitions(test_suite PRIVATE ${PRINTF_DEFINES})
  target_compile_options(test_suite PRIVATE ${PRINTF_WARNINGS} $<$<CXX_COMPILER_ID:MSVC>:/permissive->)
  set_target_properties(test_suite PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
  add_test(NAME test_suite COMMAND test_suite)

  # check that a C program can use the library via the standard function names
  add_executable(aliasing test/aliasing.c)
  target_link_libraries(aliasing PRIVATE printf)
  target_compile_options(aliasing PRIVATE ${PRINTF_WARNINGS})
  set_target_properties(aliasing PROPERTIES C_STANDARD 99 C_STANDARD_REQUIRED ON C_EXTENSIONS OFF)
  add_test(NAME aliasing COMMAND aliasing)

  # random comparison against the standard library, runs until stopped (not part of CTest)
  if(PRINTF_BUILD_AUTOTEST)
    add_executable(autotest test/autotest.cpp)
    target_link_libraries(autotest PRIVATE printf)
    target_compile_definitions(autotest PRIVATE ${PRINTF_DEFINES})
    set_target_properties(autotest PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF
                                              COMPILE_WARNING_AS_ERROR OFF)
  endif()
endif()
