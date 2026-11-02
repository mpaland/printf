///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2017-2026, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// \brief Common part of the printf unit tests: the test framework, the
//        feature switches, the output capture and the check macros.
//        The test cases were originally taken from the eyalroz/printf fork
//        (https://github.com/eyalroz/printf).
//
///////////////////////////////////////////////////////////////////////////////

#ifndef TEST_COMMON_HPP
#define TEST_COMMON_HPP

#include "catch_amalgamated.hpp"

#include <cstring>
#include <cstddef>
#include <cstdint>
#include <climits>
#include <cfloat>
#include <cmath>
#include <string>

#include "../printf.h"

// printf.h maps the standard function names to this implementation, but the
// tests need the standard library functions as reference
#undef printf
#undef sprintf
#undef snprintf
#undef vsnprintf
#undef vprintf


///////////////////////////////////////////////////////////////////////////////
// feature switches of the implementation, derived from the compile switches

#if defined(PRINTF_DISABLE_SUPPORT_FLOAT)
#define TEST_FLOAT        0
#else
#define TEST_FLOAT        1
#endif

#if TEST_FLOAT && !defined(PRINTF_DISABLE_SUPPORT_EXPONENTIAL)
#define TEST_EXPONENTIAL  1
#else
#define TEST_EXPONENTIAL  0
#endif

#if defined(PRINTF_DISABLE_SUPPORT_LONG_LONG)
#define TEST_LONG_LONG    0
#else
#define TEST_LONG_LONG    1
#endif

#if defined(PRINTF_DISABLE_SUPPORT_PTRDIFF_T)
#define TEST_PTRDIFF_T    0
#else
#define TEST_PTRDIFF_T    1
#endif


///////////////////////////////////////////////////////////////////////////////
// local warning suppression for the checks with non-standard format strings

#if defined(_MSC_VER)
#define DISABLE_WARNING_PUSH                    __pragma(warning(push))
#define DISABLE_WARNING_POP                     __pragma(warning(pop))
#define DISABLE_WARNING(number)                 __pragma(warning(disable : number))
#define DISABLE_WARNING_FORMAT
#define DISABLE_WARNING_FORMAT_EXTRA_ARGS
#define DISABLE_WARNING_FORMAT_OVERFLOW
#define DISABLE_WARNING_FORMAT_INVALID_SPECIFIER
#elif defined(__GNUC__) || defined(__clang__)
#define DO_PRAGMA(x)                            _Pragma(#x)
#define DISABLE_WARNING_PUSH                    DO_PRAGMA(GCC diagnostic push)
#define DISABLE_WARNING_POP                     DO_PRAGMA(GCC diagnostic pop)
#define DISABLE_WARNING(name)                   DO_PRAGMA(GCC diagnostic ignored #name)
#define DISABLE_WARNING_FORMAT                  DISABLE_WARNING(-Wformat)
#define DISABLE_WARNING_FORMAT_EXTRA_ARGS       DISABLE_WARNING(-Wformat-extra-args)
#if defined(__clang__)
#define DISABLE_WARNING_FORMAT_OVERFLOW
#define DISABLE_WARNING_FORMAT_INVALID_SPECIFIER DISABLE_WARNING(-Wformat-invalid-specifier)
#else
#define DISABLE_WARNING_FORMAT_OVERFLOW         DISABLE_WARNING(-Wformat-overflow)
#define DISABLE_WARNING_FORMAT_INVALID_SPECIFIER
#endif
#else
#define DISABLE_WARNING_PUSH
#define DISABLE_WARNING_POP
#define DISABLE_WARNING_FORMAT
#define DISABLE_WARNING_FORMAT_EXTRA_ARGS
#define DISABLE_WARNING_FORMAT_OVERFLOW
#define DISABLE_WARNING_FORMAT_INVALID_SPECIFIER
#endif

// all format warnings at once, for the checks with non-standard format strings
#define DISABLE_WARNING_NON_STANDARD_FORMAT     \
  DISABLE_WARNING_FORMAT                        \
  DISABLE_WARNING_FORMAT_EXTRA_ARGS             \
  DISABLE_WARNING_FORMAT_INVALID_SPECIFIER

#if defined(_MSC_VER)
DISABLE_WARNING(4996)   // discouragement of use of std::sprintf()
DISABLE_WARNING(4310)   // cast truncates constant value
DISABLE_WARNING(4127)   // conditional expression is constant
#endif


///////////////////////////////////////////////////////////////////////////////
// output capture of printf_(), vprintf_() and fctprintf()

constexpr std::size_t test_buffer_size = 100U;

// the characters passed to _putchar() or test_out_fct(), NOT null terminated
extern char        test_output[test_buffer_size];
extern std::size_t test_output_len;

// fill test_output with 0xCC and reset test_output_len
void test_output_reset();

// output function for fctprintf() and vfctprintf(), appends to test_output
void test_out_fct(char character, void* arg);

// test_output as string, with any 0xCC fill bytes not included
std::string test_output_str();


///////////////////////////////////////////////////////////////////////////////
// check macros

// the string in a buffer that may lack the terminating null
inline std::string test_str(const char* buffer, std::size_t size)
{
  std::size_t len = 0U;
  while ((len < size) && (buffer[len] != '\0')) {
    len++;
  }
  return std::string(buffer, len);
}

// checks that sprintf_() writes exactly the expected string, returns its
// length and does not touch the buffer beyond the terminating null
#define CHECK_SPRINTF(expected, ...)                                          \
  do {                                                                        \
    char buffer_[test_buffer_size];                                           \
    std::memset(buffer_, 0xCC, sizeof(buffer_));                              \
    CAPTURE(__VA_ARGS__);                                                     \
    const int ret_ = sprintf_(buffer_, __VA_ARGS__);                          \
    const std::string expected_(expected);                                    \
    CHECK(test_str(buffer_, sizeof(buffer_)) == expected_);                   \
    CHECK(ret_ == static_cast<int>(expected_.size()));                        \
    CHECK(buffer_[expected_.size() + 1U] == static_cast<char>(0xCC));         \
  } while (0)

// checks that snprintf_() with the given buffer size writes exactly the
// expected (truncated) string, returns the length of the untruncated output
// and does not touch the buffer at and beyond the given size
#define CHECK_SNPRINTF(expected, ret, size, ...)                              \
  do {                                                                        \
    char buffer_[test_buffer_size];                                           \
    std::memset(buffer_, 0xCC, sizeof(buffer_));                              \
    CAPTURE(size, __VA_ARGS__);                                               \
    const int ret_ = snprintf_(buffer_, (size), __VA_ARGS__);                 \
    const std::string expected_(expected);                                    \
    if ((size) > 0U) {                                                        \
      CHECK(test_str(buffer_, sizeof(buffer_)) == expected_);                 \
    }                                                                         \
    CHECK(ret_ == (ret));                                                     \
    if ((size) < sizeof(buffer_)) {                                           \
      CHECK(buffer_[(size)] == static_cast<char>(0xCC));                      \
    }                                                                         \
  } while (0)

#endif  // TEST_COMMON_HPP
