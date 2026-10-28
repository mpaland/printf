///////////////////////////////////////////////////////////////////////////////
// @author (c) Eyal Rozenberg <eyalroz1@gmx.com>
//             2021-2022, Haifa, Palestine/Israel
// \author (c) Marco Paland (info@paland.com)
//             2017-2019, PALANDesign Hannover, Germany
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
// \brief printf unit tests
//        The test cases are taken from the eyalroz/printf fork
//        (https://github.com/eyalroz/printf) and adapted to this implementation.
//
///////////////////////////////////////////////////////////////////////////////

#include "catch.hpp"

#include <cstring>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cfloat>
#include <sstream>
#include <cmath>
#include <limits>
#include <climits>
#include <iostream>
#include <iomanip>
#include <stdbool.h>

// compile the implementation into its own namespace to avoid conflicts with the standard library
namespace test {
  #include "../printf.h"
  #include "../printf.c"
} // namespace test

// printf.h maps the standard function names to this implementation, but the
// tests need the standard library functions as reference
#undef printf
#undef sprintf
#undef snprintf
#undef vsnprintf
#undef vprintf

using test::printf_;
using test::sprintf_;
using test::snprintf_;
using test::vprintf_;
using test::vsnprintf_;
using test::fctprintf;
using test::vfctprintf;

// vsprintf_() is not provided by this implementation
static int vsprintf_(char* buffer, const char* format, va_list va)
{
  return test::vsnprintf_(buffer, (size_t)-1, format, va);
}

// map the compiler switches of this implementation to the ones used by the test cases
#if defined(PRINTF_SUPPORT_FLOAT)
#define PRINTF_SUPPORT_DECIMAL_SPECIFIERS 1
#else
#define PRINTF_SUPPORT_DECIMAL_SPECIFIERS 0
#endif
#if defined(PRINTF_SUPPORT_FLOAT) && defined(PRINTF_SUPPORT_EXPONENTIAL)
#define PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS 1
#else
#define PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS 0
#endif
#if defined(PRINTF_SUPPORT_LONG_LONG)
#undef  PRINTF_SUPPORT_LONG_LONG
#define PRINTF_SUPPORT_LONG_LONG 1
#else
#define PRINTF_SUPPORT_LONG_LONG 0
#endif
#define PRINTF_MAX_INTEGRAL_DIGITS_FOR_DECIMAL 9   // default PRINTF_MAX_FLOAT is 1e9
#define PRINTF_USE_DOUBLE_INTERNALLY           1
#define TEST_WITH_NON_STANDARD_FORMAT_STRINGS

#if defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)) || defined(__MINGW32__)
#include <sys/types.h>
#elif defined(_WIN32)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else
// Let's just cross our fingers and hope `ssize_t` is defined.
#endif


#ifndef STRINGIFY
#define STRINGIFY(_x) #_x
#endif
#ifndef EXPAND_AND_STRINGIFY
#define EXPAND_AND_STRINGIFY(_x) STRINGIFY(_x)
#endif

#define CAPTURE_AND_PRINT(printer_, ...)                  \
do {                                                      \
  INFO( #printer_  <<                                     \
     " arguments (ignore the equations; interpret \"expr" \
     "\" := expr\" as just \"expr\"): ");                 \
  CAPTURE( __VA_ARGS__);                                  \
  printer_(__VA_ARGS__);                                  \
} while(0)

#define CAPTURE_AND_PRINT_WITH_RETVAL(retval, printer_, ...) \
do {                                                      \
  INFO( #printer_  <<                                     \
     " arguments (ignore the equations; interpret \"expr" \
     "\" := expr\" as just \"expr\"): ");                 \
  CAPTURE( __VA_ARGS__);                                  \
  retval = printer_(__VA_ARGS__);                         \
} while(0)


#define PRINTING_CHECK_WITH_BUF_SIZE(expected_, dummy, printer_, buffer_, buffer_size, ...) \
do {                                                             \
  INFO( #printer_ << " arguments, replicated ( \"arg := arg\" "  \
  "):\n----"); \
  CAPTURE( __VA_ARGS__);                                         \
  std::memset(buffer_, 0xCC, base_buffer_size);                  \
  printer_(buffer_, buffer_size, __VA_ARGS__);                   \
  if (!strcmp(buffer_, expected_)) {                             \
    buffer_[strlen(expected_) + 1] = '\0';                       \
  }                                                              \
  INFO( "----");                                                 \
  INFO( "Resulting buffer contents: " << '"' << buffer_ << '"'); \
  CHECK(!strcmp(buffer_, expected_));                            \
} while(0)

#define PRINTING_CHECK(expected_, dummy, printer_, buffer_, ...) \
do {                                                             \
  INFO( #printer_ << " arguments, replicated ( \"arg := arg\" "  \
  "):\n----"); \
  CAPTURE( __VA_ARGS__);                                         \
  std::memset(buffer_, 0xCC, base_buffer_size);                  \
  printer_(buffer_, __VA_ARGS__);                                \
  if (!strcmp(buffer_, expected_)) {                             \
    buffer_[strlen(expected_) + 1] = '\0';                       \
  }                                                              \
  INFO( "----");                                                 \
  INFO( "Resulting buffer contents: " << '"' << buffer_ << '"'); \
  CHECK(!strcmp(buffer_, expected_));                            \
} while(0)

// Multi-compiler-compatible local warning suppression

#if defined(_MSC_VER)
    #define DISABLE_WARNING_PUSH           __pragma(warning( push ))
    #define DISABLE_WARNING_POP            __pragma(warning( pop ))
    #define DISABLE_WARNING(warningNumber) __pragma(warning( disable : warningNumber ))

    // TODO: find the right warning number for this
    #define DISABLE_WARNING_PRINTF_FORMAT
    #define DISABLE_WARNING_PRINTF_FORMAT_EXTRA_ARGS
    #define DISABLE_WARNING_PRINTF_FORMAT_OVERFLOW
    #define DISABLE_WARNING_PRINTF_FORMAT_INVALID_SPECIFIER

#elif defined(__GNUC__) || defined(__clang__)
    #define DO_PRAGMA(X) _Pragma(#X)
    #define DISABLE_WARNING_PUSH           DO_PRAGMA(GCC diagnostic push)
    #define DISABLE_WARNING_POP            DO_PRAGMA(GCC diagnostic pop)
    #define DISABLE_WARNING(warningName)   DO_PRAGMA(GCC diagnostic ignored #warningName)

    #define DISABLE_WARNING_PRINTF_FORMAT    DISABLE_WARNING(-Wformat)
    #define DISABLE_WARNING_PRINTF_FORMAT_EXTRA_ARGS DISABLE_WARNING(-Wformat-extra-args)
#if defined(__clang__)
    #define DISABLE_WARNING_PRINTF_FORMAT_OVERFLOW
    #define DISABLE_WARNING_PRINTF_FORMAT_INVALID_SPECIFIER DISABLE_WARNING(-Wformat-invalid-specifier)
#else
    #define DISABLE_WARNING_PRINTF_FORMAT_OVERFLOW DISABLE_WARNING(-Wformat-overflow)
    #define DISABLE_WARNING_PRINTF_FORMAT_INVALID_SPECIFIER
#endif
#else
    #define DISABLE_WARNING_PUSH
    #define DISABLE_WARNING_POP
    #define DISABLE_WARNING_PRINTF_FORMAT
    #define DISABLE_WARNING_PRINTF_FORMAT_EXTRA_ARGS
    #define DISABLE_WARNING_PRINTF_FORMAT_INVALID_SPECIFIER
#endif

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS
DISABLE_WARNING_PUSH
DISABLE_WARNING_PRINTF_FORMAT
DISABLE_WARNING_PRINTF_FORMAT_EXTRA_ARGS
DISABLE_WARNING_PRINTF_FORMAT_INVALID_SPECIFIER
#endif

#if defined(_MSC_VER)
DISABLE_WARNING(4996) // Discouragement of use of std::sprintf()
DISABLE_WARNING(4310) // Casting to smaller type
DISABLE_WARNING(4127) // Constant conditional expression
#endif

constexpr const size_t base_buffer_size { 100 };

// This macro is idempotent here, but for other platforms may
// be defined differently
#define mkstr(_str) _str

// dummy putchar
static char   printf_buffer[base_buffer_size];
static size_t printf_idx = 0U;

void test::_putchar(char character)
{
  printf_buffer[printf_idx++] = character;
}

void _out_fct(char character, void* arg)
{
  (void)arg;
  printf_buffer[printf_idx++] = character;
}

#ifndef STRINGIFY
#define STRINGIFY(_x) #_x
#endif
#define PRINTF_TEST_CASE(unstringified_name)  TEST_CASE(STRINGIFY(unstringified_name), "[]")

PRINTF_TEST_CASE(printf) {
  printf_idx = 0U;
  memset(printf_buffer, 0xCC, base_buffer_size);
  INFO("printf_ format string and arguments: ");
  CAPTURE("% d", 4232);
  CHECK(printf_("% d", 4232) == 5);
  INFO("printf_ format string and arguments: ");
  CAPTURE("% d", 4232);
  CHECK(printf_buffer[5] == (char)0xCC);
  printf_buffer[5] = 0;
  INFO("printf_ format string and arguments: ");
  CAPTURE("% d", 4232);
  CHECK(!strcmp(printf_buffer, " 4232"));
}


PRINTF_TEST_CASE(fctprintf) {
  printf_idx = 0U;
  memset(printf_buffer, 0xCC, base_buffer_size);
  fctprintf(&_out_fct, nullptr, "This is a test of %X", 0x12EFU);
  INFO("fctprintf format string and arguments: ");
  CAPTURE("This is a test of %X", 0x12EFU);
  CHECK(!strncmp(printf_buffer, "This is a test of 12EF", 22U));
  CHECK(printf_buffer[22] == (char)0xCC);
}

// output function type
typedef void (*out_fct_type_)(char character, void* arg);


static void vfctprintf_builder_1(out_fct_type_ f, char* buffer, ...)
{
  va_list args;
  va_start(args, buffer);
  CAPTURE_AND_PRINT(vfctprintf, f, nullptr, "This is a test of %X", args);
  va_end(args);
}

PRINTF_TEST_CASE(vfctprintf) {
  printf_idx = 0U;
  memset(printf_buffer, 0xCC, base_buffer_size);
  vfctprintf_builder_1(&_out_fct, nullptr, 0x12EFU);
  CHECK(!strncmp(printf_buffer, "This is a test of 12EF", 22U));
  CHECK(printf_buffer[22] == (char)0xCC);
}

PRINTF_TEST_CASE(snprintf_) {
  char buffer[base_buffer_size];
  PRINTING_CHECK_WITH_BUF_SIZE("-1000", ==, snprintf_, buffer, base_buffer_size, "%d", -1000);
  PRINTING_CHECK_WITH_BUF_SIZE("-1",    ==, snprintf_, buffer, 3U, "%d", -1000);
}

static void vprintf_builder_1(char* buffer, ...)
{
  va_list args;
  va_start(args, buffer);
  vprintf_("%d", args);
  va_end(args);
}

static void vsprintf_builder_1(char* buffer, ...)
{
  va_list args;
  va_start(args, buffer);
  CAPTURE_AND_PRINT(vsprintf_, buffer, "%d", args);
  va_end(args);
}

static void vsnprintf_builder_1(char* buffer, ...)
{
  va_list args;
  va_start(args, buffer);
  CAPTURE_AND_PRINT(vsnprintf_, buffer, 100U, "%d", args);
  va_end(args);
}

static void vsprintf_builder_3(char* buffer, ...)
{
  va_list args;
  va_start(args, buffer);
  CAPTURE_AND_PRINT(vsprintf_, buffer, "%d %d %s", args);
  va_end(args);
}

static void vsnprintf_builder_3(char* buffer, ...)
{
  va_list args;
  va_start(args, buffer);
  CAPTURE_AND_PRINT(vsnprintf_, buffer, 100U, "%d %d %s", args);
  va_end(args);
}


PRINTF_TEST_CASE(vprintf) {
  char buffer[base_buffer_size];
  printf_idx = 0U;
  memset(printf_buffer, 0xCC, base_buffer_size);
  vprintf_builder_1(buffer, 2345);
  CHECK(printf_buffer[4] == (char)0xCC);
  printf_buffer[4] = 0;
  CHECK(!strcmp(printf_buffer, "2345"));
}


PRINTF_TEST_CASE(vsprintf) {
  char buffer[base_buffer_size];

  vsprintf_builder_1(buffer, -1);
  CHECK(!strcmp(buffer, "-1"));

  vsprintf_builder_3(buffer, 3, -1000, "test");
  CHECK(!strcmp(buffer, "3 -1000 test"));
}


PRINTF_TEST_CASE(vsnprintf_) {
  char buffer[base_buffer_size];

  vsnprintf_builder_1(buffer, -1);
  CHECK(!strcmp(buffer, "-1"));

  vsnprintf_builder_3(buffer, 3, -1000, "test");
  CHECK(!strcmp(buffer, "3 -1000 test"));
}

PRINTF_TEST_CASE(ret_value)
{
  char buffer[base_buffer_size];
  int ret;

  ret = snprintf_(buffer, 6, "0%s", "1234");
  CHECK(!strcmp(buffer, "01234"));
  CHECK(ret == 5);

  std::memset(buffer, 0xCC, sizeof(buffer));
  ret = snprintf_(buffer, 6, "0%s", "12345");
  CHECK(!strcmp(buffer, "01234"));
  CHECK(ret == 6);  // "5" is truncated

  std::memset(buffer, 0xCC, sizeof(buffer));
  ret = snprintf_(buffer, 6, "0%s", "1234567");
  CHECK(!strcmp(buffer, "01234"));
  CHECK(ret == 8);  // "567" are truncated

  std::memset(buffer, 0xCC, sizeof(buffer));
  DISABLE_WARNING_PUSH
  DISABLE_WARNING_PRINTF_FORMAT_OVERFLOW
  ret = snprintf_(buffer, 6, "0%s", (const char *) NULL);
  DISABLE_WARNING_POP
  CHECK(!strcmp(buffer, "0(nul"));
  CHECK(ret == 7);  // "l)" is truncated

  std::memset(buffer, 0xCC, sizeof(buffer));
  ret = snprintf_(buffer, 10, "hello, world");
  CHECK(ret == 12);

  std::memset(buffer, 0xCC, sizeof(buffer));
  ret = snprintf_(buffer, 3, "%d", 10000);
  CHECK(ret == 5);
  CHECK(strlen(buffer) == 2U);
  CHECK(buffer[0] == '1');
  CHECK(buffer[1] == '0');
  CHECK(buffer[2] == '\0');
}

#if PRINTF_SUPPORT_DECIMAL_SPECIFIERS || PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS
PRINTF_TEST_CASE(brute_force_float)
{
  char buffer[base_buffer_size];
#if PRINTF_SUPPORT_DECIMAL_SPECIFIERS
  // brute force float
  bool any_failed = false;
  std::stringstream sstr;
  sstr.precision(5);
  for (float i = -100000; i < 100000; i += (float) 1) {
    sprintf_(buffer, "%.5f", (double) (i / 10000));
    sstr.str("");
    sstr << std::fixed << i / 10000;
    auto expected = sstr.str();
    if (strcmp(buffer, expected.c_str()) != 0) {
      std::cerr
      << ": sprintf_(\"%.5f\", " << std::setw(6) << i << ") = " << std::setw(10) << buffer << " , "
      << "expected " << std::setw(10) << expected.c_str() << "\n";
      any_failed = true;
    }
  }
  CHECK(not any_failed);

#if PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS
  // This is tested when _both_ decimal and exponential specifiers are supported.
  // brute force exp
  sstr.setf(std::ios::scientific, std::ios::floatfield);
  any_failed = false;
  long n = 0;
  for (float i = (float) -1e20; i < (float) 1e20; i += (float) 1e15, n++) {
#if PRINTF_USE_DOUBLE_INTERNALLY
#define LOOP_PRECISION 4
#else
  // You're kind of screwed here...
#define LOOP_PRECISION 1
#endif
    sprintf_(buffer, "%." EXPAND_AND_STRINGIFY(LOOP_PRECISION) "f", (double) i);
    sstr.str("");
    sstr << std::setprecision(LOOP_PRECISION) << i;
    auto expected = sstr.str();
    if (strcmp(buffer, expected.c_str()) != 0) {
      std::cerr
      << n << ": sprintf_(\"%" EXPAND_AND_STRINGIFY(LOOP_PRECISION) "f\", " << std::setw(18) << std::setprecision(30) << i << ") = " << std::setw(15)
      << buffer << " , "
      << "expected " << std::setw(12) << expected.c_str() << "\n";
      any_failed = true;
    }
  }
  CHECK(not any_failed);
#undef LOOP_PRECISION
#endif
#endif
}

#endif // PRINTF_SUPPORT_DECIMAL_SPECIFIERS || PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS


#include "test_suite_main_testcases.hpp"


// test cases of the original test suite which are not part of the eyalroz/printf test suite
PRINTF_TEST_CASE(additional_cases)
{
  char buffer[base_buffer_size];
  (void) buffer;

  PRINTING_CHECK("0",                       ==, sprintf_, buffer, "%#.1x", 0);
#if PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS
  PRINTING_CHECK("    +inf",                ==, sprintf_, buffer, "%+8e", (double) INFINITY);
#endif
}

// the examples of the fixed GitHub issues of mpaland/printf
PRINTF_TEST_CASE(issues)
{
  char buffer[base_buffer_size];
  (void) buffer;

  // #49, #113: precision with left alignment
  PRINTING_CHECK("001024    ",              ==, sprintf_, buffer, "%-10.6d", 1024);
  PRINTING_CHECK("00123               ",    ==, sprintf_, buffer, "%-20.5i", 123);

  // #50, #99: the '0x' prefix must not replace digits
  PRINTING_CHECK("0x1234",                  ==, sprintf_, buffer, "%#4x", 0x1234);
  PRINTING_CHECK("0x1001",                  ==, sprintf_, buffer, "%#05x", 0x1001);
  PRINTING_CHECK("0x01001",                 ==, sprintf_, buffer, "%#07x", 0x1001);
  PRINTING_CHECK("0b00000101",              ==, sprintf_, buffer, "%#010b", 5);

  // #53, #109, #110, #114: octal prefix is a leading zero
  PRINTING_CHECK("06143",                   ==, sprintf_, buffer, "%#.5o", 06143);
  PRINTING_CHECK("006143",                  ==, sprintf_, buffer, "%#.6o", 06143);
  PRINTING_CHECK("0",                       ==, sprintf_, buffer, "%#.0o", 0);
  PRINTING_CHECK("0001",                    ==, sprintf_, buffer, "%#.4o", 1);
#if PRINTF_SUPPORT_LONG_LONG
  PRINTING_CHECK("0",                       ==, sprintf_, buffer, "%#.0llo", (long long) 0);
#endif

  // #118: no signed integer overflow for the most negative values
  PRINTING_CHECK("-2147483648",             ==, sprintf_, buffer, "%d", (int) INT32_MIN);
#if PRINTF_SUPPORT_LONG_LONG
  PRINTING_CHECK("-9223372036854775808",    ==, sprintf_, buffer, "%lld", (long long) INT64_MIN);
#endif

  // a negative precision is taken as if the precision were omitted
  PRINTING_CHECK("0",                       ==, sprintf_, buffer, "%.*d", -1, 0);
  PRINTING_CHECK("abc",                     ==, sprintf_, buffer, "%.*s", -1, "abc");
#if PRINTF_SUPPORT_DECIMAL_SPECIFIERS
  PRINTING_CHECK("1.500000",                ==, sprintf_, buffer, "%.*f", -1, 1.5);
#endif

  // #63: NULL string pointer
  PRINTING_CHECK("(null)",                  ==, sprintf_, buffer, "%s", (const char *) NULL);
  PRINTING_CHECK("  (null)",                ==, sprintf_, buffer, "%8s", (const char *) NULL);

  // #47: a '\0' printed by %c is output by printf_() and fctprintf() as well
  printf_idx = 0U;
  memset(printf_buffer, 0xCC, base_buffer_size);
  CHECK(printf_("X%cX", '\0') == 3);
  CHECK(printf_idx == 3U);
  CHECK(!memcmp(printf_buffer, "X\0X", 3U));
  printf_idx = 0U;
  CHECK(fctprintf(&_out_fct, nullptr, "X%cX", '\0') == 3);
  CHECK(printf_idx == 3U);
  CHECK(!memcmp(printf_buffer, "X\0X", 3U));
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS
DISABLE_WARNING_POP
#endif

