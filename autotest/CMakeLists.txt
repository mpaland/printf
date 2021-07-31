cmake_minimum_required(VERSION 3.9)

enable_language(CXX)

add_executable(autotest "autotest.cpp")

set_target_properties(
	autotest
	PROPERTIES
	CXX_STANDARD 11
	CXX_STANDARD_REQUIRED YES
	CXX_EXTENSIONS NO
)

add_definitions(-DNO_OVERRIDE_STDIO)

option(TEST_BROKEN_FORMATS "Include tests using non-standard-compliant format strings?" ON)
# ... don't worry, we'll suppress the compiler warnings for those.
if (TEST_BROKEN_FORMATS)
	target_compile_definitions(autotest PRIVATE TEST_WITH_NON_STANDARD_FORMAT_STRINGS)
endif()

target_link_libraries(autotest PRIVATE mpaland-printf)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(autoteste PRIVATE /W4)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR 
        CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	# lots of warnings and all warnings as errors
	target_compile_options(autotest PRIVATE
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
		target_compile_options(autotest PRIVATE -ffat-lto-objects)
	endif()
endif()

add_test(
	NAME ${PROJECT_NAME}.autotest
	COMMAND autotest # ${TEST_RUNNER_PARAMS}
)


