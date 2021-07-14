cmake_minimum_required (VERSION 3.13)
project(
	mpaland-printf
	LANGUAGES C
	DESCRIPTION "Mostly- stand-alone C implementation of sprintf and printf functions"
	HOMEPAGE_URL https://github.com/eyalroz/printf
	VERSION 0.2.0
)

option(BUILD_TESTS "Build test programs for the library" OFF)

option(SUPPORT_FLOAT_ARGUMENTS "Support floating-point arguments (%f)" ON)
if (NOT SUPPORT_FLOAT_ARGUMENTS)
	add_definitions(-DPRINTF_DISABLE_SUPPORT_FLOAT)
endif()

option(SUPPORT_EXPONENTIAL_FLOAT_ARGUMENTS "Support SUPPORT_EXPONENTIAL_FLOAT_ARGUMENTS (%e/%g)" ON)
if (NOT SUPPORT_EXPONENTIAL_FLOAT_ARGUMENTS)
	add_definitions(-DPRINTF_DISABLE_SUPPORT_EXPONENTIAL)
endif()

option(SUPPORT_LONG_LONG_INTS_AND_PTR "Support long long integers and pointers (%ll, %llu, %p)" ON)
if (NOT SUPPORT_LONG_LONG_INTS_AND_PTR)
	add_definitions(-DPRINTF_DISABLE_SUPPORT_LONG_LONG)
endif()

option(SUPPORT_PTRDIFF_ARGUMENTS "Support SUPPORT_PTRDIFF_ARGUMENTS (%t)" ON)
if (NOT SUPPORT_PTRDIFF_ARGUMENTS)
	add_definitions(-DPRINTF_DISABLE_SUPPORT_PTRDIFF_T)
endif()

# numeric defines

set(INTEGER_TO_STRING_BUFFER_SIZE "32" CACHE STRING "Integer to string (ntoa) conversion buffer size")
string(REGEX MATCH "^[0-9]+([eE][0-9]+)?$" PRINTF_NTOA_BUFFER_SIZE "${INTEGER_TO_STRING_BUFFER_SIZE}")
if ("${PRINTF_NTOA_BUFFER_SIZE}" STREQUAL "")
	message(FATAL_ERROR "An (integral) buffer size for converting integers to floating-point values must be specified." )
else()
	add_definitions(-DPRINTF_NTOA_BUFFER_SIZE=${PRINTF_NTOA_BUFFER_SIZE})
endif()

set(FLOAT_TO_STRING_BUFFER_SIZE "32" CACHE STRING "Float to string (ftoa) conversion buffer size")
string(REGEX MATCH "^[0-9]+([eE][0-9]+)?$" PRINTF_FTOA_BUFFER_SIZE "${FLOAT_TO_STRING_BUFFER_SIZE}")
if ("${PRINTF_FTOA_BUFFER_SIZE}" STREQUAL "")
	message(FATAL_ERROR "An (integral) buffer size for converting floating-point values to strings must be specified." )
else()
	add_definitions(-DPRINTF_FTOA_BUFFER_SIZE=${PRINTF_FTOA_BUFFER_SIZE})
endif()

set(DEFAULT_FLOAT_PRECISION "6" CACHE STRING "Default precision when printing floating-point values")
string(REGEX MATCH "^[0-9]+([eE][0-9]+)?$" DFP_PARSED "${DEFAULT_FLOAT_PRECISION}")
if ("${DFP_PARSED}" STREQUAL "")
	message(FATAL_ERROR "An (integral) default floating-point precision value must be specified." )
else()
	add_definitions(-DPRINTF_DEFAULT_FLOAT_PRECISION=${DFP_PARSED})
endif()

set(FLOAT_NOTATION_THRESHOLD "1e9" CACHE STRING "Largest floating-point value for which printing with %f uses non-exponential notation")
string(REGEX MATCH "^[0-9]+([eE][0-9]+)?$" PRINTF_MAX_FLOAT "${FLOAT_NOTATION_THRESHOLD}")
if ("${PRINTF_MAX_FLOAT}" STREQUAL "")
	message(FATAL_ERROR "The %f notation switch threshold must be an integer." )
else()
	add_definitions(-DPRINTF_MAX_FLOAT=${PRINTF_MAX_FLOAT})
endif()

option(BUILD_STATIC_LIBRARY "Build the library as static rather than shared" OFF)


if (BUILD_STATIC_LIBRARY)
	add_library(mpaland-printf STATIC printf.c)
else()
	add_library(mpaland-printf SHARED printf.c)
endif()

set_property(TARGET mpaland-printf PROPERTY C_STANDARD 99)
set_property(TARGET mpaland-printf PROPERTY C_STANDARD_REQUIRED ON)
set_property(TARGET mpaland-printf PROPERTY C_EXTENSIONS OFF)

target_include_directories(
	mpaland-printf
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	PUBLIC
	"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

set_target_properties(mpaland-printf PROPERTIES
         LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
         ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(mpaland-printf PRIVATE /W4)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR 
        CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_compile_options(mpaland-printf PRIVATE -Wall -Wextra -pedantic)
endif()

if (BUILD_TESTS)
	enable_testing()
	add_subdirectory(test)
endif()

if (UNIX)
	add_custom_target(mpaland-printf-sizes 
		COMMAND size -A -t $<TARGET_FILE:mpaland-printf>  > mpaland-printf_sizes.txt
		DEPENDS mpaland-printf
		BYPRODUCTS mpaland-printf_sizes.txt
		COMMENT Prints the sizes of the different sections of the ELF file: text, dat, vss etc.)

	add_custom_target(mpaland-printf-symbols 
		COMMAND nm --numeric-sort --print-size "$<TARGET_FILE:mpaland-printf>" > mpaland-printf_symbols.txt
		COMMAND bash -c "nm --numeric-sort --print-size $<TARGET_FILE:mpaland-printf> | c++filt > mpaland-printf_cpp_symbols.txt" 
		VERBATIM
		DEPENDS mpaland-printf
		BYPRODUCTS mpaland-printf_symbols.txt mpaland-printf_cpp_symbols.txt
		COMMENT Produces lists of the symbols, and C++demangled symbols, inside the library)

	add_custom_target(mpaland-printf-lst
		COMMAND objdump --disassemble --line-numbers -S "$<TARGET_FILE:mpaland-printf>" > mpaland-printf.list
		DEPENDS mpaland-printf
		BYPRODUCTS mpaland-printf.lst
		COMMENT Dissassembles the compiled library into an .lst file)

endif()

# -------------------------
# Installation
# -------------------------

include(GNUInstallDirs)

install(
	TARGETS mpaland-printf
	EXPORT mpaland-printf_export
	RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
	ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(
	FILES "printf.h"
	DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(
	EXPORT mpaland-printf_export
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/mpaland-printf"
	NAMESPACE "mpaland-printf::"
	FILE "mpaland-printf-config.cmake"
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
	"mpaland-printf-config-version.cmake"
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMinorVersion
)

install(
	FILES "${CMAKE_CURRENT_BINARY_DIR}/mpaland-printf-config-version.cmake"
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/mpaland-printf"
)



