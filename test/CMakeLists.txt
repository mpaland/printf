cmake_minimum_required(VERSION 3.9)


################
##  Packages  ##
################

# find_package(mplanad-printf REQUIRED)

#############
##  Tests  ##
#############

enable_language(CXX)

add_executable(test_suite "test_suite.cpp")

set_target_properties(
	test_suite
	PROPERTIES
	CXX_STANDARD 11
	CXX_STANDARD_REQUIRED YES
	CXX_EXTENSIONS NO
)

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
                -ffat-lto-objects
                -fdata-sections
                -fverbose-asm
                -Wunused-parameter
                -Wfloat-equal
	)
endif()

add_test(
	NAME ${PROJECT_NAME}.test_suite
	COMMAND test_suite # ${TEST_RUNNER_PARAMS}
)


