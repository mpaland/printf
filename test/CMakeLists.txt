cmake_minimum_required(VERSION 3.9)

enable_language(CXX)

add_executable(test_suite "test_suite.cpp")

set_target_properties(
	test_suite
	PROPERTIES
	CXX_STANDARD 11
	CXX_STANDARD_REQUIRED YES
	CXX_EXTENSIONS NO
)

# These two lines are necessary, since the test suite does not actually use the
# compiled library - it includes the library's source .c file; and that means we
# need to include the generated config.h file.
target_compile_definitions(test_suite PRIVATE PRINTF_INCLUDE_CONFIG_H)
target_include_directories(test_suite PRIVATE "${GENERATED_INCLUDE_DIR}")

option(TEST_BROKEN_FORMATS "Include tests using non-standard-compliant format strings?" ON)
# ... don't worry, we'll suppress the compiler warnings for those.
if (TEST_BROKEN_FORMATS)
	target_compile_definitions(test_suite PRIVATE TEST_WITH_NON_STANDARD_FORMAT_STRINGS)
endif()

target_link_libraries(test_suite PRIVATE mpaland-printf)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(test_suite PRIVATE /W4)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR
        CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	# lots of warnings and all warnings as errors
	target_compile_options(test_suite PRIVATE
        -g
		-Wall
		-Wextra
		-pedantic
		-Wundef
		-Wsign-conversion
		-Wuninitialized
		-Wshadow
		-Wunreachable-code
		-Wswitch-default
		-Wswitch
		-Wcast-align
		-Wmissing-include-dirs
		-Winit-self
		-Wdouble-promotion
		-gdwarf-2
		-fno-exceptions
		-ffunction-sections
		-fdata-sections
		-fverbose-asm
		-Wunused-parameter
	)
	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		target_compile_options(test_suite PRIVATE -ffat-lto-objects)
	endif()
endif()

add_test(
	NAME ${PROJECT_NAME}.test_suite
	COMMAND test_suite # ${TEST_RUNNER_PARAMS}
)


