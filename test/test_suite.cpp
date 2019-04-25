///////////////////////////////////////////////////////////////////////////////
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
//
///////////////////////////////////////////////////////////////////////////////

// use the 'catch' test framework
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <string.h>
#include <sstream>
#include <math.h>
#include <limits.h>
#include <inttypes.h>

using Catch::Matchers::Equals;

// store pointers to library functions before they are masked by macro
int (*lib_sprintf)(char * buffer, const char* fmt, ...) = sprintf;
int (*lib_snprintf)(char * buffer, size_t count, const char* fmt, ...) = snprintf;

#if 0
namespace test {
  // use functions in own test namespace to avoid stdio conflicts
  #include "../printf.h"
  #include "../printf.c"
} // namespace test
#else
  #include "../printf.h"
#endif

// dummy putchar
static char   printf_buffer[100];
static size_t printf_idx = 0U;

void /*test::*/_putchar(char character)
{
  printf_buffer[printf_idx++] = character;
}

void _out_fct(char character, void* arg)
{
  (void)arg;
  printf_buffer[printf_idx++] = character;
}


TEST_CASE("printf", "[]" ) {
  printf_idx = 0U;
  memset(printf_buffer, 0xCC, sizeof(printf_buffer));
  REQUIRE(/*test::*/printf("% d", 4232) == 5);
  REQUIRE(printf_buffer[5] == (char)0xCC);
  printf_buffer[5] = 0;
  REQUIRE(!strcmp(printf_buffer, " 4232"));
}


TEST_CASE("fctprintf", "[]" ) {
  printf_idx = 0U;
  memset(printf_buffer, 0xCC,  sizeof(printf_buffer));
  /*test::*/fctprintf(&_out_fct, nullptr, "This is a test of %X", 0x12EFU);
  REQUIRE(!strncmp(printf_buffer, "This is a test of 12EF", 22U));
  REQUIRE(printf_buffer[22] == (char)0xCC);
}

static void vprintf_builder_1(char* buffer, ...)
{
  va_list args;
  va_start(args, buffer);
  /*test::*/vprintf("%d", args);
  va_end(args);
}

static void vsnprintf_builder_1(char* buffer, ...)
{
  va_list args;
  va_start(args, buffer);
  /*test::*/vsnprintf(buffer, 100U, "%d", args);
  va_end(args);
}

static void vsnprintf_builder_3(char* buffer, ...)
{
  va_list args;
  va_start(args, buffer);
  /*test::*/vsnprintf(buffer, 100U, "%d %d %s", args);
  va_end(args);
}


TEST_CASE("vprintf", "[]" ) {
  char buffer[100];
  printf_idx = 0U;
  memset(printf_buffer, 0xCC, 100U);
  vprintf_builder_1(buffer, 2345);
  REQUIRE(printf_buffer[4] == (char)0xCC);
  printf_buffer[4] = 0;
  REQUIRE(!strcmp(printf_buffer, "2345"));
}

TEST_CASE("vsnprintf", "[]" ) {
  char buffer[100];

  vsnprintf_builder_1(buffer, -1);
  REQUIRE_THAT(buffer, Equals("-1"));

  vsnprintf_builder_3(buffer, 3, -1000, "test");
  REQUIRE_THAT(buffer, Equals("3 -1000 test"));
}


// wrapper macros to compare our implementation with library one
// variables with result are available for further test cases
#define TEST_DEF                                        \
  char buffer[100];                                     \
  int ret;                                              \
  struct dummy                                          \
  /**/

#define CMP_DEF                                         \
  TEST_DEF;                                             \
  char buffer_ref[100];                                 \
  int ret_ref;                                          \
  struct dummy                                          \
  /**/

#define CMP_SPRINTF(...)                                \
  do {                                                  \
    CAPTURE(__VA_ARGS__);                               \
    ret = /*test::*/sprintf(buffer, __VA_ARGS__);       \
    ret_ref = lib_sprintf(buffer_ref, __VA_ARGS__);     \
    CHECK_THAT(buffer, Equals(buffer_ref));             \
    CHECK(ret == ret_ref);                              \
  } while (0)                                           \
    /**/

#define CMP_SNPRINTF(...)                               \
  do {                                                  \
    CAPTURE(__VA_ARGS__);                               \
    ret = /*test::*/snprintf(buffer, __VA_ARGS__);      \
    ret_ref = lib_snprintf(buffer_ref, __VA_ARGS__);    \
    CHECK_THAT(buffer, Equals(buffer_ref));             \
    CHECK(ret == ret_ref);                              \
  } while (0)                                           \
    /**/

#define TEST_SPRINTF(matcher, ...)                      \
  do {                                                  \
  CAPTURE(__VA_ARGS__);                                 \
    ret = /*test::*/sprintf(buffer, __VA_ARGS__);       \
    CHECK_THAT(buffer, matcher);                        \
    (void)ret;                                          \
  } while (0)                                           \
    /**/

#define TEST(str, retval, ...)                          \
  do {                                                  \
    CAPTURE(__VA_ARGS__);                               \
    ret = /*test::*/sprintf(buffer, __VA_ARGS__);       \
    CHECK_THAT(buffer, Equals(str));                    \
    CHECK(retval == ret);                               \
  } while (0)                                           \
    /**/

TEST_CASE("snprintf", "[]" ) {
  CMP_DEF;

  CMP_SNPRINTF(100U, "%d", -1000);
  CMP_SNPRINTF(3U, "%d", -1000);
}


TEST_CASE("space flag", "[]" ) {
  CMP_DEF;

  CMP_SPRINTF("% d", 42);
  CMP_SPRINTF("% d", -42);
  CMP_SPRINTF("% 5d", 42);
  CMP_SPRINTF("% 5d", -42);
  CMP_SPRINTF("% 15d", 42);
  CMP_SPRINTF("% 15d", -42);
  CMP_SPRINTF("% 15d", -42);
  CMP_SPRINTF("% 15.3f", -42.987);
  CMP_SPRINTF("% 15.3f", 42.987);
  CMP_SPRINTF("% s", "Hello testing");
  CMP_SPRINTF("% d", 1024);
  CMP_SPRINTF("% d", -1024);
  CMP_SPRINTF("% i", 1024);
  CMP_SPRINTF("% i", -1024);
  CMP_SPRINTF("% u", 1024);
  CMP_SPRINTF("% u", 4294966272U);
  CMP_SPRINTF("% o", 511);
  CMP_SPRINTF("% o", 4294966785U);
  CMP_SPRINTF("% x", 305441741);
  CMP_SPRINTF("% x", 3989525555U);
  CMP_SPRINTF("% X", 305441741);
  CMP_SPRINTF("% X", 3989525555U);
  CMP_SPRINTF("% c", 'x');
}


TEST_CASE("+ flag", "[]" ) {
  CMP_DEF;

  CMP_SPRINTF("%+d", 42);
  CMP_SPRINTF("%+d", -42);
  CMP_SPRINTF("%+5d", 42);
  CMP_SPRINTF("%+5d", -42);
  CMP_SPRINTF("%+15d", 42);
  CMP_SPRINTF("%+15d", -42);
  CMP_SPRINTF("%+s", "Hello testing");
  CMP_SPRINTF("%+d", 1024);
  CMP_SPRINTF("%+d", -1024);
  CMP_SPRINTF("%+i", 1024);
  CMP_SPRINTF("%+i", -1024);
  CMP_SPRINTF("%+u", 1024);
  CMP_SPRINTF("%+u", 4294966272U);
  CMP_SPRINTF("%+o", 511);
  CMP_SPRINTF("%+o", 4294966785U);
  CMP_SPRINTF("%+x", 305441741);
  CMP_SPRINTF("%+x", 3989525555U);
  CMP_SPRINTF("%+X", 305441741);
  CMP_SPRINTF("%+X", 3989525555U);
  CMP_SPRINTF("%+c", 'x');
  CMP_SPRINTF("%+.0d", 0);
}


TEST_CASE("0 flag", "[]" ) {
  CMP_DEF;

  CMP_SPRINTF("%0d", 42);
  CMP_SPRINTF("%0ld", 42L);
  CMP_SPRINTF("%0d", -42);
  CMP_SPRINTF("%05d", 42);
  CMP_SPRINTF("%05d", -42);
  CMP_SPRINTF("%015d", 42);
  CMP_SPRINTF("%015d", -42);
  CMP_SPRINTF("%015.2f", 42.1234);
  CMP_SPRINTF("%015.3f", 42.9876);
  CMP_SPRINTF("%015.5f", -42.9876);
}


TEST_CASE("- flag", "[]" ) {
  CMP_DEF;

  CMP_SPRINTF("%-d", 42);
  CMP_SPRINTF("%-d", -42);
  CMP_SPRINTF("%-5d", 42);
  CMP_SPRINTF("%-5d", -42);
  CMP_SPRINTF("%-15d", 42);
  CMP_SPRINTF("%-15d", -42);
  CMP_SPRINTF("%-0d", 42);
  CMP_SPRINTF("%-0d", -42);
  CMP_SPRINTF("%-05d", 42);
  CMP_SPRINTF("%-05d", -42);
  CMP_SPRINTF("%-015d", 42);
  CMP_SPRINTF("%-015d", -42);
  CMP_SPRINTF("%0-d", 42);
  CMP_SPRINTF("%0-d", -42);
  CMP_SPRINTF("%0-5d", 42);
  CMP_SPRINTF("%0-5d", -42);
  CMP_SPRINTF("%0-15d", 42);
  CMP_SPRINTF("%0-15d", -42);
#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  CMP_SPRINTF("%0-15.3e", -42.);
#else
  TEST_SPRINTF(Equals("e"), "%0-15.3e", -42.);
#endif

#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  TEST_SPRINTF(Equals("-42.0          "), "%0-15.3g", -42.);
#else
  TEST_SPRINTF(Equals("g"), "%0-15.3g", -42.);
#endif
}


TEST_CASE("# flag", "[]" ) {
  CMP_DEF;

  CMP_SPRINTF("%#.0x", 0);
  CMP_SPRINTF("%#.1x", 0);
  CMP_SPRINTF("%#.0llx", (long long)0);
  CMP_SPRINTF("%#.8x", 0x614e);
  TEST_SPRINTF(Equals("0b110"), "%#b", 6);
  TEST_SPRINTF(Equals("0b110"), "%#5b", 6);
}


TEST_CASE("specifier", "[]" ) {
  CMP_DEF;

  CMP_SPRINTF("Hello testing");
  CMP_SPRINTF("%s", "Hello testing");
  CMP_SPRINTF("%d", 1024);
  CMP_SPRINTF("%d", -1024);
  CMP_SPRINTF("%i", 1024);
  CMP_SPRINTF("%i", -1024);
  CMP_SPRINTF("%u", 1024);
  CMP_SPRINTF("%u", 4294966272U);
  CMP_SPRINTF("%o", 511);
  CMP_SPRINTF("%o", 4294966785U);
  CMP_SPRINTF("%x", 305441741);
  CMP_SPRINTF("%x", 3989525555U);
  CMP_SPRINTF("%X", 305441741);
  CMP_SPRINTF("%X", 3989525555U);
  CMP_SPRINTF("%%");
}


TEST_CASE("width", "[]" ) {
  CMP_DEF;

  CMP_SPRINTF("%1s", "Hello testing");
  CMP_SPRINTF("%1d", 1024);
  CMP_SPRINTF("%1d", -1024);
  CMP_SPRINTF("%1i", 1024);
  CMP_SPRINTF("%1i", -1024);
  CMP_SPRINTF("%1u", 1024);
  CMP_SPRINTF("%1u", 4294966272U);
  CMP_SPRINTF("%1o", 511);
  CMP_SPRINTF("%1o", 4294966785U);
  CMP_SPRINTF("%1x", 305441741);
  CMP_SPRINTF("%1x", 3989525555U);
  CMP_SPRINTF("%1X", 305441741);
  CMP_SPRINTF("%1X", 3989525555U);
  CMP_SPRINTF("%1c", 'x');
}


TEST_CASE("width 20", "[]" ) {
  CMP_DEF;

  CMP_SPRINTF("%20s", "Hello");
  CMP_SPRINTF("%20d", 1024);
  CMP_SPRINTF("%20d", -1024);
  CMP_SPRINTF("%20i", 1024);
  CMP_SPRINTF("%20i", -1024);
  CMP_SPRINTF("%20u", 1024);
  CMP_SPRINTF("%20u", 4294966272U);
  CMP_SPRINTF("%20o", 511);
  CMP_SPRINTF("%20o", 4294966785U);
  CMP_SPRINTF("%20x", 305441741);
  CMP_SPRINTF("%20x", 3989525555U);
  CMP_SPRINTF("%20X", 305441741);
  CMP_SPRINTF("%20X", 3989525555U);
  CMP_SPRINTF("%20c", 'x');
}


TEST_CASE("width *20", "[]" ) {
  CMP_DEF;

  CMP_SPRINTF("%*s", 20, "Hello");
  CMP_SPRINTF("%*d", 20, 1024);
  CMP_SPRINTF("%*d", 20, -1024);
  CMP_SPRINTF("%*i", 20, 1024);
  CMP_SPRINTF("%*i", 20, -1024);
  CMP_SPRINTF("%*u", 20, 1024);
  CMP_SPRINTF("%*u", 20, 4294966272U);
  CMP_SPRINTF("%*o", 20, 511);
  CMP_SPRINTF("%*o", 20, 4294966785U);
  CMP_SPRINTF("%*x", 20, 305441741);
  CMP_SPRINTF("%*x", 20, 3989525555U);
  CMP_SPRINTF("%*X", 20, 305441741);
  CMP_SPRINTF("%*X", 20, 3989525555U);
  CMP_SPRINTF("%*c", 20,'x');
}


TEST_CASE("width -20", "[]" ) {
  CMP_DEF;

  CMP_SPRINTF("%-20s", "Hello");
  CMP_SPRINTF("%-20d", 1024);
  CMP_SPRINTF("%-20d", -1024);
  CMP_SPRINTF("%-20i", 1024);
  CMP_SPRINTF("%-20i", -1024);
  CMP_SPRINTF("%-20u", 1024);
  CMP_SPRINTF("%-20.4f", 1024.1234);
  CMP_SPRINTF("%-20u", 4294966272U);
  CMP_SPRINTF("%-20o", 511);
  CMP_SPRINTF("%-20o", 4294966785U);
  CMP_SPRINTF("%-20x", 305441741);
  CMP_SPRINTF("%-20x", 3989525555U);
  CMP_SPRINTF("%-20X", 305441741);
  CMP_SPRINTF("%-20X", 3989525555U);
  CMP_SPRINTF("%-20c", 'x');
  CMP_SPRINTF("|%5d| |%-2d| |%5d|", 9, 9, 9);
  CMP_SPRINTF("|%5d| |%-2d| |%5d|", 10, 10, 10);
  CMP_SPRINTF("|%5d| |%-12d| |%5d|", 9, 9, 9);
  CMP_SPRINTF("|%5d| |%-12d| |%5d|", 10, 10, 10);
}


TEST_CASE("width 0-20", "[]" ) {
  CMP_DEF;

  CMP_SPRINTF("%0-20s", "Hello");
  CMP_SPRINTF("%0-20d", 1024);
  CMP_SPRINTF("%0-20d", -1024);
  CMP_SPRINTF("%0-20i", 1024);
  CMP_SPRINTF("%0-20i", -1024);
  CMP_SPRINTF("%0-20u", 1024);
  CMP_SPRINTF("%0-20u", 4294966272U);
  CMP_SPRINTF("%0-20o", 511);
  CMP_SPRINTF("%0-20o", 4294966785U);
  CMP_SPRINTF("%0-20x", 305441741);
  CMP_SPRINTF("%0-20x", 3989525555U);
  CMP_SPRINTF("%0-20X", 305441741);
  CMP_SPRINTF("%0-20X", 3989525555U);
  CMP_SPRINTF("%0-20c", 'x');
}


TEST_CASE("padding 20", "[]" ) {
  CMP_DEF;

  CMP_SPRINTF("%020d", 1024);
  CMP_SPRINTF("%020d", -1024);
  CMP_SPRINTF("%020i", 1024);
  CMP_SPRINTF("%020i", -1024);
  CMP_SPRINTF("%020u", 1024);
  CMP_SPRINTF("%020u", 4294966272U);
  CMP_SPRINTF("%020o", 511);
  CMP_SPRINTF("%020o", 4294966785U);
  CMP_SPRINTF("%020x", 305441741);
  CMP_SPRINTF("%020x", 3989525555U);
  CMP_SPRINTF("%020X", 305441741);
  CMP_SPRINTF("%020X", 3989525555U);
}


TEST_CASE("padding .20", "[]" ) {
  CMP_DEF;

  CMP_SPRINTF("%.20d", 1024);
  CMP_SPRINTF("%.20d", -1024);
  CMP_SPRINTF("%.20i", 1024);
  CMP_SPRINTF("%.20i", -1024);
  CMP_SPRINTF("%.20u", 1024);
  CMP_SPRINTF("%.20u", 4294966272U);
  CMP_SPRINTF("%.20o", 511);
  CMP_SPRINTF("%.20o", 4294966785U);
  CMP_SPRINTF("%.20x", 305441741);
  CMP_SPRINTF("%.20x", 3989525555U);
  CMP_SPRINTF("%.20X", 305441741);
  CMP_SPRINTF("%.20X", 3989525555U);
}


TEST_CASE("padding #020", "[]" ) {
  CMP_DEF;

  CMP_SPRINTF("%#020d", 1024);
  CMP_SPRINTF("%#020d", -1024);
  CMP_SPRINTF("%#020i", 1024);
  CMP_SPRINTF("%#020i", -1024);
  CMP_SPRINTF("%#020u", 1024);
  CMP_SPRINTF("%#020u", 4294966272U);
  CMP_SPRINTF("%#020o", 511);
  CMP_SPRINTF("%#020o", 4294966785U);
  CMP_SPRINTF("%#020x", 305441741);
  CMP_SPRINTF("%#020x", 3989525555U);
  CMP_SPRINTF("%#020X", 305441741);
  CMP_SPRINTF("%#020X", 3989525555U);
}

TEST_CASE("variable length padding", "[]") {
  CMP_DEF;

  for (int pad = 0; pad <= 20; pad++)
    CMP_SPRINTF("%#*x", pad, 1234);
}

TEST_CASE("padding #20", "[]" ) {
  CMP_DEF;

  CMP_SPRINTF("%#20d", 1024);
  CMP_SPRINTF("%#20d", -1024);
  CMP_SPRINTF("%#20i", 1024);
  CMP_SPRINTF("%#20i", -1024);
  CMP_SPRINTF("%#20u", 1024);
  CMP_SPRINTF("%#20u", 4294966272U);
  CMP_SPRINTF("%#20o", 511);
  CMP_SPRINTF("%#20o", 4294966785U);
  CMP_SPRINTF("%#20x", 305441741);
  CMP_SPRINTF("%#20x", 3989525555U);
  CMP_SPRINTF("%#20X", 305441741);
  CMP_SPRINTF("%#20X", 3989525555U);
}


TEST_CASE("padding 20.5", "[]" ) {
  CMP_DEF;

  CMP_SPRINTF("%20.5d", 1024);
  CMP_SPRINTF("%20.5d", -1024);
  CMP_SPRINTF("%20.5i", 1024);
  CMP_SPRINTF("%20.5i", -1024);
  CMP_SPRINTF("%20.5u", 1024);
  CMP_SPRINTF("%20.5u", 4294966272U);
  CMP_SPRINTF("%20.5o", 511);
  CMP_SPRINTF("%20.5o", 4294966785U);
  CMP_SPRINTF("%20.5x", 305441741);
  CMP_SPRINTF("%20.10x", 3989525555U);
  CMP_SPRINTF("%20.5X", 305441741);
  CMP_SPRINTF("%20.10X", 3989525555U);
}


TEST_CASE("padding neg numbers", "[]" ) {
  CMP_DEF;

  // space padding
  CMP_SPRINTF("% 1d", -5);
  CMP_SPRINTF("% 2d", -5);
  CMP_SPRINTF("% 3d", -5);
  CMP_SPRINTF("% 4d", -5);
  // zero padding
  CMP_SPRINTF("%01d", -5);
  CMP_SPRINTF("%02d", -5);
  CMP_SPRINTF("%03d", -5);
  CMP_SPRINTF("%04d", -5);
}


TEST_CASE("float padding neg numbers", "[]" ) {
  CMP_DEF;

  // space padding
  CMP_SPRINTF("% 3.1f", -5.);
  CMP_SPRINTF("% 4.1f", -5.);
  CMP_SPRINTF("% 5.1f", -5.);
#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  CMP_SPRINTF("% 6.1g", -5.);
  CMP_SPRINTF("% 6.1e", -5.);
  CMP_SPRINTF("% 10.1e", -5.);
#endif

  // zero padding
  CMP_SPRINTF("%03.1f", -5.);
  CMP_SPRINTF("%04.1f", -5.);
  CMP_SPRINTF("%05.1f", -5.);
  // zero padding no decimal point
  CMP_SPRINTF("%01.0f", -5.);
  CMP_SPRINTF("%02.0f", -5.);
  CMP_SPRINTF("%03.0f", -5.);
#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  CMP_SPRINTF("%010.1e", -5.);
  CMP_SPRINTF("%07.0E", -5.);
  CMP_SPRINTF("%03.0g", -5.);
#endif
}

TEST_CASE("length", "[]" ) {
  CMP_DEF;

  CMP_SPRINTF("%.0s", "Hello testing");
  CMP_SPRINTF("%20.0s", "Hello testing");
  CMP_SPRINTF("%.s", "Hello testing");
  CMP_SPRINTF("%20.s", "Hello testing");
  CMP_SPRINTF("%20.0d", 1024);
  CMP_SPRINTF("%20.0d", -1024);
  CMP_SPRINTF("%20.d", 0);
  CMP_SPRINTF("%20.0i", 1024);
  CMP_SPRINTF("%20.i", -1024);
  CMP_SPRINTF("%20.i", 0);
  CMP_SPRINTF("%20.u", 1024);
  CMP_SPRINTF("%20.0u", 4294966272U);
  CMP_SPRINTF("%20.u", 0U);
  CMP_SPRINTF("%20.o", 511);
  CMP_SPRINTF("%20.0o", 4294966785U);
  CMP_SPRINTF("%20.o", 0U);
  CMP_SPRINTF("%20.x", 305441741);
  CMP_SPRINTF("%50.x", 305441741);
  CMP_SPRINTF("%50.x%10.u", 305441741, 12345);
  CMP_SPRINTF("%20.0x", 3989525555U);
  CMP_SPRINTF("%20.x", 0U);
  CMP_SPRINTF("%20.X", 305441741);
  CMP_SPRINTF("%20.0X", 3989525555U);
  CMP_SPRINTF("%20.X", 0U);
  CMP_SPRINTF("%02.0u", 0U);
  CMP_SPRINTF("%02.0d", 0);
}


TEST_CASE("float", "[]" ) {
  CMP_DEF;

  // test special-case floats using math.h macros
  CMP_SPRINTF("%8f", NAN);
  CMP_SPRINTF("%8f", (double)INFINITY);
  CMP_SPRINTF("%-8f", (double)-INFINITY);
#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  CMP_SPRINTF("%+8e", INFINITY);
#endif

  CMP_SPRINTF("%.4f", 3.1415354);
  CMP_SPRINTF("%.3f", 30343.1415354);
  CMP_SPRINTF("%.0f", 34.1415354);
  CMP_SPRINTF("%.0f", 1.3);
  CMP_SPRINTF("%.0f", 1.55);
  CMP_SPRINTF("%.1f", 1.64);
  CMP_SPRINTF("%.2f", 42.8952);
  CMP_SPRINTF("%.9f", 42.8952);
  CMP_SPRINTF("%.10f", 42.895223);
  CMP_SPRINTF("%.1f", -.95);
  CMP_SPRINTF("%.1f", .95);
  CMP_SPRINTF("%.1f", .25);
  CMP_SPRINTF("%.1f", .75);
 // this testcase checks, that the precision is truncated to 9 digits.
  // a perfect working float should return the whole number
  TEST_SPRINTF(Equals("42.895223123000"), "%.12f", 42.89522312345678);
  // this testcase checks, that the precision is truncated AND rounded to 9 digits.
  // a perfect working float should return the whole number
  TEST_SPRINTF(Equals("42.895223877000"), "%.12f", 42.89522387654321);
  CMP_SPRINTF("%6.2f", 42.8952);
  CMP_SPRINTF("%+6.2f", 42.8952);
  CMP_SPRINTF("%+5.1f", 42.9252);
  CMP_SPRINTF("%f", 42.5);
  CMP_SPRINTF("%.1f", 42.5);
  CMP_SPRINTF("%f", 42167.0);
  CMP_SPRINTF("%.9f", -12345.987654321);
  CMP_SPRINTF("%.1f", 3.999);
  CMP_SPRINTF("%.0f", 3.5);
  CMP_SPRINTF("%.0f", 4.5);
  CMP_SPRINTF("%.0f", 3.49);
  CMP_SPRINTF("%.1f", 3.49);
  CMP_SPRINTF("a%-5.1f", 0.5);
  CMP_SPRINTF("a%-5.1fend", 0.5);
#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  CMP_SPRINTF("%G", 12345.678);
  CMP_SPRINTF("%.7G", 12345.678);
  CMP_SPRINTF("%.5G", 123456789.);
  TEST_SPRINTF(Equals("12345.0"), "%.6G", 12345.);
  CMP_SPRINTF("%+12.4g", 123456789.);
  CMP_SPRINTF("%.2G", 0.001234);
  CMP_SPRINTF("%+10.4G", 0.001234);
  CMP_SPRINTF("%+012.4g", 0.00001234);
  CMP_SPRINTF("%.3g", -1.2345e-308);
  CMP_SPRINTF("%+.3E", 1.23e+308);
#endif

  // out of range for float: should switch to exp notation if supported, else empty
#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  TEST_SPRINTF(Equals("1.0e+20"), "%.1f", 1E20);
#else
  TEST_SPRINTF(Equals(""), "%.1f", 1E20);
#endif

  // brute force float
  bool fail = false;
  std::stringstream str;
  str.precision(5);

  for (double i = -200000; i < 200000; i += 1) {
    CMP_SPRINTF("%.5f", i / 20000);
  }

  for (double i = -1; i <= 1; i += .05) {
    CMP_SPRINTF("%.1f", i);
  }

#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  // brute force exp
  str.setf(std::ios::scientific, std::ios::floatfield);
  for (float i = -1e20f; i < 1e20f; i += 1e15f) {
    /*test::*/sprintf(buffer, "%.5f", (double)i);
    str.str("");
    str << i;
    fail = fail || !!strcmp(buffer, str.str().c_str());
  }
  REQUIRE(!fail);
#endif
}


TEST_CASE("types", "[]" ) {
  CMP_DEF;

  CMP_SPRINTF("%i", 0);
  CMP_SPRINTF("%i", 1234);
  CMP_SPRINTF("%i", 32767);
  CMP_SPRINTF("%i", -32767);
  CMP_SPRINTF("%li", 30L);
  CMP_SPRINTF("%li", -2147483647L);
  CMP_SPRINTF("%li", 2147483647L);
  CMP_SPRINTF("%lli", 30LL);
  CMP_SPRINTF("%lli", -9223372036854775807LL);
  CMP_SPRINTF("%lli", 9223372036854775807LL);
  CMP_SPRINTF("%lu", 100000L);
  CMP_SPRINTF("%lu", 0xFFFFFFFFL);
  CMP_SPRINTF("%llu", 281474976710656LLU);
  CMP_SPRINTF("%llu", 18446744073709551615LLU);
  CMP_SPRINTF("%zu", 2147483647UL);
  CMP_SPRINTF("%zd", 2147483647UL);
  if (sizeof(size_t) == sizeof(long)) {
    CMP_SPRINTF("%zi", -2147483647L);
  }
  else {
    CMP_SPRINTF("%zi", -2147483647LL);
  }

  TEST_SPRINTF(Equals("1110101001100000"), "%b", 60000);
  TEST_SPRINTF(Equals("101111000110000101001110"), "%lb", 12345678L);    // size limit 
  CMP_SPRINTF("%o", 60000);
  CMP_SPRINTF("%lo", 12345678L);
  CMP_SPRINTF("%lx", 0x12345678L);
  CMP_SPRINTF("%llx", 0x1234567891234567LLU);
  CMP_SPRINTF("%lx", 0xabcdefabL);
  CMP_SPRINTF("%lX", 0xabcdefabL);
  CMP_SPRINTF("%c", 'v');
  CMP_SPRINTF("%cv", 'w');
  CMP_SPRINTF("%s", "A Test");
  CMP_SPRINTF("%hhu", 0xFFFFUL);
  CMP_SPRINTF("%hu", 0x123456UL);
  CMP_SPRINTF("%s%hhi %hu", "Test", 10000, 0xFFFFFFFF);
  CMP_SPRINTF("%tx", &buffer[10] - &buffer[0]);
// TBD
  if (sizeof(intmax_t) == sizeof(long)) {
    CMP_SPRINTF("%ji", -2147483647L);
  }
  else {
    CMP_SPRINTF("%ji", -2147483647LL);
  }
}


TEST_CASE("pointer", "[]" ) {
  TEST_DEF;

  // gcc %p implementation is different (and implementatino-specific)

  if (sizeof(void*) == 4U) {
    TEST_SPRINTF(Equals("00001234"), "%p", (void*)0x1234U);
  }
  else {
    TEST_SPRINTF(Equals("0000000000001234"), "%p", (void*)0x1234U);
  }

  if (sizeof(void*) == 4U) {
    TEST_SPRINTF(Equals("12345678"), "%p", (void*)0x12345678U);
  }
  else {
    TEST_SPRINTF(Equals("0000000012345678"), "%p", (void*)0x12345678U);
  }

  if (sizeof(void*) == 4U) {
    TEST_SPRINTF(Equals("12345678-7EDCBA98"), "%p-%p", (void*)0x12345678U, (void*)0x7EDCBA98U);
  }
  else {
    TEST_SPRINTF(Equals("0000000012345678-000000007EDCBA98"), "%p-%p", (void*)0x12345678U, (void*)0x7EDCBA98U);
  }

  if (sizeof(uintptr_t) == sizeof(uint64_t)) {
    TEST_SPRINTF(Equals("00000000FFFFFFFF"), "%p", (void*)(uintptr_t)0xFFFFFFFFU);
  }
  else {
    TEST_SPRINTF(Equals("FFFFFFFF"), "%p", (void*)(uintptr_t)0xFFFFFFFFU);
  }
}


TEST_CASE("unknown flag", "[]" ) {
  TEST_DEF;

  TEST_SPRINTF(Equals("kmarco"), "%kmarco", 42, 37);
}


TEST_CASE("string length", "[]" ) {
  CMP_DEF;

  CMP_SPRINTF("%.4s", "This is a test");
  CMP_SPRINTF("%.4s", "test");
  CMP_SPRINTF("%.7s", "123");
  CMP_SPRINTF("%.7s", "");
  CMP_SPRINTF("%.4s%.2s", "123456", "abcdef");
  TEST_SPRINTF(Equals(".2s"), "%.4.2s", "123456");
  CMP_SPRINTF("%.*s", 3, "123456");
}


TEST_CASE("buffer length", "[]" ) {
  TEST_DEF;

  ret = /*test::*/snprintf(nullptr, 10, "%s", "Test");
  REQUIRE(ret == 4);
  ret = /*test::*/snprintf(nullptr, 0, "%s", "Test");
  REQUIRE(ret == 4);

  buffer[0] = (char)0xA5;
  ret = /*test::*/snprintf(buffer, 0, "%s", "Test");
  REQUIRE(buffer[0] == (char)0xA5);
  REQUIRE(ret == 4);

  buffer[0] = (char)0xCC;
  /*test::*/snprintf(buffer, 1, "%s", "Test");
  REQUIRE(buffer[0] == '\0');

  /*test::*/snprintf(buffer, 2, "%s", "Hello");
  REQUIRE_THAT(buffer, Equals("H"));
}


TEST_CASE("ret value", "[]" ) {
  CMP_DEF;

  CMP_SNPRINTF(6, "0%s", "1234");

  CMP_SNPRINTF(6, "0%s", "12345");
  CMP_SNPRINTF(6, "0%s", "1234567"); // '567' are truncated
  
  CMP_SNPRINTF(10, ("hello, world"));  // ret is longer

  CMP_SNPRINTF(3, "%d", 10000);
}


TEST_CASE("misc", "[]" ) {
  CMP_DEF;

  CMP_SPRINTF("%u%u%ctest%d %s", 5, 3000, 'a', -20, "bit");
  CMP_SPRINTF("%.*f", 2, 0.33333333);
  CMP_SPRINTF("%.*d", -1, 1);
  CMP_SPRINTF("%.3s", "foobar");
  CMP_SPRINTF("% .0d", 0);
  CMP_SPRINTF("%10.5d", 4);
  CMP_SPRINTF("%*sx", -3, "hi");
#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  CMP_SPRINTF("%.*g", 2, 0.33333333);
  CMP_SPRINTF("%.*e", 2, 0.33333333);
#endif
}

TEST_CASE("libc-sprintf", "[]") {
  CMP_DEF;
     /* Ein String ohne alles */
  TEST("Hallo heimur", 12, "Hallo heimur");

    /* Einfache Konvertierungen */
    TEST("Hallo heimur",   12, "%s",       "Hallo heimur");
    TEST("1024",            4, "%d",       1024);
    TEST("-1024",           5, "%d",       -1024);
    TEST("1024",            4, "%i",       1024);
    TEST("-1024",           5, "%i",       -1024);
    TEST("1024",            4, "%u",       1024u);
    TEST("4294966272",     10, "%u",       -1024u);
    TEST("777",             3, "%o",       0777u);
    TEST("37777777001",    11, "%o",       -0777u);
    TEST("1234abcd",        8, "%x",       0x1234abcdu);
    TEST("edcb5433",        8, "%x",       -0x1234abcdu);
    TEST("1234ABCD",        8, "%X",       0x1234abcdu);
    TEST("EDCB5433",        8, "%X",       -0x1234abcdu);
    TEST("x",               1, "%c",       'x');
    TEST("%",               1, "%%");

    /* Mit %c kann man auch Nullbytes ausgeben */
    TEST("\0",              1, "%c",       '\0');

    /* Vorzeichen erzwingen (Flag +) */
    TEST("Hallo heimur",   12, "%+s",      "Hallo heimur");
    TEST("+1024",           5, "%+d",      1024);
    TEST("-1024",           5, "%+d",      -1024);
    TEST("+1024",           5, "%+i",      1024);
    TEST("-1024",           5, "%+i",      -1024);
    TEST("1024",            4, "%+u",      1024u);
    TEST("4294966272",     10, "%+u",      -1024u);
    TEST("777",             3, "%+o",      0777u);
    TEST("37777777001",    11, "%+o",      -0777u);
    TEST("1234abcd",        8, "%+x",      0x1234abcdu);
    TEST("edcb5433",        8, "%+x",      -0x1234abcdu);
    TEST("1234ABCD",        8, "%+X",      0x1234abcdu);
    TEST("EDCB5433",        8, "%+X",      -0x1234abcdu);
    TEST("x",               1, "%+c",      'x');

    /* Vorzeichenplatzhalter erzwingen (Flag <space>) */
    TEST("Hallo heimur",   12, "% s",      "Hallo heimur");
    TEST(" 1024",           5, "% d",      1024);
    TEST("-1024",           5, "% d",      -1024);
    TEST(" 1024",           5, "% i",      1024);
    TEST("-1024",           5, "% i",      -1024);
    TEST("1024",            4, "% u",      1024u);
    TEST("4294966272",     10, "% u",      -1024u);
    TEST("777",             3, "% o",      0777u);
    TEST("37777777001",    11, "% o",      -0777u);
    TEST("1234abcd",        8, "% x",      0x1234abcdu);
    TEST("edcb5433",        8, "% x",      -0x1234abcdu);
    TEST("1234ABCD",        8, "% X",      0x1234abcdu);
    TEST("EDCB5433",        8, "% X",      -0x1234abcdu);
    TEST("x",               1, "% c",      'x');

    /* Flag + hat Vorrang über <space> */
    TEST("Hallo heimur",   12, "%+ s",      "Hallo heimur");
    TEST("+1024",           5, "%+ d",      1024);
    TEST("-1024",           5, "%+ d",      -1024);
    TEST("+1024",           5, "%+ i",      1024);
    TEST("-1024",           5, "%+ i",      -1024);
    TEST("1024",            4, "%+ u",      1024u);
    TEST("4294966272",     10, "%+ u",      -1024u);
    TEST("777",             3, "%+ o",      0777u);
    TEST("37777777001",    11, "%+ o",      -0777u);
    TEST("1234abcd",        8, "%+ x",      0x1234abcdu);
    TEST("edcb5433",        8, "%+ x",      -0x1234abcdu);
    TEST("1234ABCD",        8, "%+ X",      0x1234abcdu);
    TEST("EDCB5433",        8, "%+ X",      -0x1234abcdu);
    TEST("x",               1, "%+ c",      'x');

    /* Alternative Form */
    TEST("0777",            4, "%#o",      0777u);
    TEST("037777777001",   12, "%#o",      -0777u);
    TEST("0x1234abcd",     10, "%#x",      0x1234abcdu);
    TEST("0xedcb5433",     10, "%#x",      -0x1234abcdu);
    TEST("0X1234ABCD",     10, "%#X",      0x1234abcdu);
    TEST("0XEDCB5433",     10, "%#X",      -0x1234abcdu);
    TEST("0",               1, "%#o",      0u);
    TEST("0",               1, "%#x",      0u);
    TEST("0",               1, "%#X",      0u);

    /* Feldbreite: Kleiner als Ausgabe */
    TEST("Hallo heimur",   12, "%1s",      "Hallo heimur");
    TEST("1024",            4, "%1d",      1024);
    TEST("-1024",           5, "%1d",      -1024);
    TEST("1024",            4, "%1i",      1024);
    TEST("-1024",           5, "%1i",      -1024);
    TEST("1024",            4, "%1u",      1024u);
    TEST("4294966272",     10, "%1u",      -1024u);
    TEST("777",             3, "%1o",      0777u);
    TEST("37777777001",    11, "%1o",      -0777u);
    TEST("1234abcd",        8, "%1x",      0x1234abcdu);
    TEST("edcb5433",        8, "%1x",      -0x1234abcdu);
    TEST("1234ABCD",        8, "%1X",      0x1234abcdu);
    TEST("EDCB5433",        8, "%1X",      -0x1234abcdu);
    TEST("x",               1, "%1c",      'x');

    /* Feldbreite: Größer als Ausgabe */
    TEST("               Hallo",  20, "%20s",      "Hallo");
    TEST("                1024",  20, "%20d",      1024);
    TEST("               -1024",  20, "%20d",      -1024);
    TEST("                1024",  20, "%20i",      1024);
    TEST("               -1024",  20, "%20i",      -1024);
    TEST("                1024",  20, "%20u",      1024u);
    TEST("          4294966272",  20, "%20u",      -1024u);
    TEST("                 777",  20, "%20o",      0777u);
    TEST("         37777777001",  20, "%20o",      -0777u);
    TEST("            1234abcd",  20, "%20x",      0x1234abcdu);
    TEST("            edcb5433",  20, "%20x",      -0x1234abcdu);
    TEST("            1234ABCD",  20, "%20X",      0x1234abcdu);
    TEST("            EDCB5433",  20, "%20X",      -0x1234abcdu);
    TEST("                   x",  20, "%20c",      'x');

    /* Feldbreite: Linksbündig */
    TEST("Hallo               ",  20, "%-20s",      "Hallo");
    TEST("1024                ",  20, "%-20d",      1024);
    TEST("-1024               ",  20, "%-20d",      -1024);
    TEST("1024                ",  20, "%-20i",      1024);
    TEST("-1024               ",  20, "%-20i",      -1024);
    TEST("1024                ",  20, "%-20u",      1024u);
    TEST("4294966272          ",  20, "%-20u",      -1024u);
    TEST("777                 ",  20, "%-20o",      0777u);
    TEST("37777777001         ",  20, "%-20o",      -0777u);
    TEST("1234abcd            ",  20, "%-20x",      0x1234abcdu);
    TEST("edcb5433            ",  20, "%-20x",      -0x1234abcdu);
    TEST("1234ABCD            ",  20, "%-20X",      0x1234abcdu);
    TEST("EDCB5433            ",  20, "%-20X",      -0x1234abcdu);
    TEST("x                   ",  20, "%-20c",      'x');

    /* Feldbreite: Padding mit 0 */
    TEST("00000000000000001024",  20, "%020d",      1024);
    TEST("-0000000000000001024",  20, "%020d",      -1024);
    TEST("00000000000000001024",  20, "%020i",      1024);
    TEST("-0000000000000001024",  20, "%020i",      -1024);
    TEST("00000000000000001024",  20, "%020u",      1024u);
    TEST("00000000004294966272",  20, "%020u",      -1024u);
    TEST("00000000000000000777",  20, "%020o",      0777u);
    TEST("00000000037777777001",  20, "%020o",      -0777u);
    TEST("0000000000001234abcd",  20, "%020x",      0x1234abcdu);
    TEST("000000000000edcb5433",  20, "%020x",      -0x1234abcdu);
    TEST("0000000000001234ABCD",  20, "%020X",      0x1234abcdu);
    TEST("000000000000EDCB5433",  20, "%020X",      -0x1234abcdu);

    /* Feldbreite: Padding und alternative Form */
    TEST("                0777",  20, "%#20o",      0777u);
    TEST("        037777777001",  20, "%#20o",      -0777u);
    TEST("          0x1234abcd",  20, "%#20x",      0x1234abcdu);
    TEST("          0xedcb5433",  20, "%#20x",      -0x1234abcdu);
    TEST("          0X1234ABCD",  20, "%#20X",      0x1234abcdu);
    TEST("          0XEDCB5433",  20, "%#20X",      -0x1234abcdu);

    TEST("00000000000000000777",  20, "%#020o",     0777u);
    TEST("00000000037777777001",  20, "%#020o",     -0777u);
    TEST("0x00000000001234abcd",  20, "%#020x",     0x1234abcdu);
    TEST("0x0000000000edcb5433",  20, "%#020x",     -0x1234abcdu);
    TEST("0X00000000001234ABCD",  20, "%#020X",     0x1234abcdu);
    TEST("0X0000000000EDCB5433",  20, "%#020X",     -0x1234abcdu);

    /* Feldbreite: - hat Vorrang vor 0 */
    TEST("Hallo               ",  20, "%0-20s",      "Hallo");
    TEST("1024                ",  20, "%0-20d",      1024);
    TEST("-1024               ",  20, "%0-20d",      -1024);
    TEST("1024                ",  20, "%0-20i",      1024);
    TEST("-1024               ",  20, "%0-20i",      -1024);
    TEST("1024                ",  20, "%0-20u",      1024u);
    TEST("4294966272          ",  20, "%0-20u",      -1024u);
    TEST("777                 ",  20, "%-020o",      0777u);
    TEST("37777777001         ",  20, "%-020o",      -0777u);
    TEST("1234abcd            ",  20, "%-020x",      0x1234abcdu);
    TEST("edcb5433            ",  20, "%-020x",      -0x1234abcdu);
    TEST("1234ABCD            ",  20, "%-020X",      0x1234abcdu);
    TEST("EDCB5433            ",  20, "%-020X",      -0x1234abcdu);
    TEST("x                   ",  20, "%-020c",      'x');

    /* Feldbreite: Aus Parameter */
    TEST("               Hallo",  20, "%*s",      20, "Hallo");
    TEST("                1024",  20, "%*d",      20, 1024);
    TEST("               -1024",  20, "%*d",      20, -1024);
    TEST("                1024",  20, "%*i",      20, 1024);
    TEST("               -1024",  20, "%*i",      20, -1024);
    TEST("                1024",  20, "%*u",      20, 1024u);
    TEST("          4294966272",  20, "%*u",      20, -1024u);
    TEST("                 777",  20, "%*o",      20, 0777u);
    TEST("         37777777001",  20, "%*o",      20, -0777u);
    TEST("            1234abcd",  20, "%*x",      20, 0x1234abcdu);
    TEST("            edcb5433",  20, "%*x",      20, -0x1234abcdu);
    TEST("            1234ABCD",  20, "%*X",      20, 0x1234abcdu);
    TEST("            EDCB5433",  20, "%*X",      20, -0x1234abcdu);
    TEST("                   x",  20, "%*c",      20, 'x');

    /* Präzision / Mindestanzahl von Ziffern */
    TEST("Hallo heimur",           12, "%.20s",      "Hallo heimur");
    TEST("00000000000000001024",   20, "%.20d",      1024);
    TEST("-00000000000000001024",  21, "%.20d",      -1024);
    TEST("00000000000000001024",   20, "%.20i",      1024);
    TEST("-00000000000000001024",  21, "%.20i",      -1024);
    TEST("00000000000000001024",   20, "%.20u",      1024u);
    TEST("00000000004294966272",   20, "%.20u",      -1024u);
    TEST("00000000000000000777",   20, "%.20o",      0777u);
    TEST("00000000037777777001",   20, "%.20o",      -0777u);
    TEST("0000000000001234abcd",   20, "%.20x",      0x1234abcdu);
    TEST("000000000000edcb5433",   20, "%.20x",      -0x1234abcdu);
    TEST("0000000000001234ABCD",   20, "%.20X",      0x1234abcdu);
    TEST("000000000000EDCB5433",   20, "%.20X",      -0x1234abcdu);

    /* Feldbreite und Präzision */
    TEST("               Hallo",   20, "%20.5s",     "Hallo heimur");
    TEST("               01024",   20, "%20.5d",      1024);
    TEST("              -01024",   20, "%20.5d",      -1024);
    TEST("               01024",   20, "%20.5i",      1024);
    TEST("              -01024",   20, "%20.5i",      -1024);
    TEST("               01024",   20, "%20.5u",      1024u);
    TEST("          4294966272",   20, "%20.5u",      -1024u);
    TEST("               00777",   20, "%20.5o",      0777u);
    TEST("         37777777001",   20, "%20.5o",      -0777u);
    TEST("            1234abcd",   20, "%20.5x",      0x1234abcdu);
    TEST("          00edcb5433",   20, "%20.10x",     -0x1234abcdu);
    TEST("            1234ABCD",   20, "%20.5X",      0x1234abcdu);
    TEST("          00EDCB5433",   20, "%20.10X",     -0x1234abcdu);

    /* Präzision: 0 wird ignoriert */
    TEST("               Hallo",   20, "%020.5s",    "Hallo heimur");
    TEST("               01024",   20, "%020.5d",     1024);
    TEST("              -01024",   20, "%020.5d",     -1024);
    TEST("               01024",   20, "%020.5i",     1024);
    TEST("              -01024",   20, "%020.5i",     -1024);
    TEST("               01024",   20, "%020.5u",     1024u);
    TEST("          4294966272",   20, "%020.5u",     -1024u);
    TEST("               00777",   20, "%020.5o",     0777u);
    TEST("         37777777001",   20, "%020.5o",     -0777u);
    TEST("            1234abcd",   20, "%020.5x",     0x1234abcdu);
    TEST("          00edcb5433",   20, "%020.10x",    -0x1234abcdu);
    TEST("            1234ABCD",   20, "%020.5X",     0x1234abcdu);
    TEST("          00EDCB5433",   20, "%020.10X",    -0x1234abcdu);

    /* Präzision 0 */
    TEST("",                        0, "%.0s",        "Hallo heimur");
    TEST("                    ",   20, "%20.0s",      "Hallo heimur");
    TEST("",                        0, "%.s",         "Hallo heimur");
    TEST("                    ",   20, "%20.s",       "Hallo heimur");
    TEST("                1024",   20, "%20.0d",      1024);
    TEST("               -1024",   20, "%20.d",       -1024);
    TEST("                    ",   20, "%20.d",       0);
    TEST("                1024",   20, "%20.0i",      1024);
    TEST("               -1024",   20, "%20.i",       -1024);
    TEST("                    ",   20, "%20.i",       0);
    TEST("                1024",   20, "%20.u",       1024u);
    TEST("          4294966272",   20, "%20.0u",      -1024u);
    TEST("                    ",   20, "%20.u",       0u);
    TEST("                 777",   20, "%20.o",       0777u);
    TEST("         37777777001",   20, "%20.0o",      -0777u);
    TEST("                    ",   20, "%20.o",       0u);
    TEST("            1234abcd",   20, "%20.x",       0x1234abcdu);
    TEST("            edcb5433",   20, "%20.0x",      -0x1234abcdu);
    TEST("                    ",   20, "%20.x",       0u);
    TEST("            1234ABCD",   20, "%20.X",       0x1234abcdu);
    TEST("            EDCB5433",   20, "%20.0X",      -0x1234abcdu);
    TEST("                    ",   20, "%20.X",       0u);

    /* Negative Präzision wird ignoriert */
    /* XXX glibc tut nicht, was ich erwartet habe, vorerst deaktiviert... */
#if 0
    TEST("Hallo heimur",   12, "%.-42s",       "Hallo heimur");
    TEST("1024",            4, "%.-42d",       1024);
    TEST("-1024",           5, "%.-42d",       -1024);
    TEST("1024",            4, "%.-42i",       1024);
    TEST("-1024",           5, "%.-42i",       -1024);
    TEST("1024",            4, "%.-42u",       1024u);
    TEST("4294966272",     10, "%.-42u",       -1024u);
    TEST("777",             3, "%.-42o",       0777u);
    TEST("37777777001",    11, "%.-42o",       -0777u);
    TEST("1234abcd",        8, "%.-42x",       0x1234abcdu);
    TEST("edcb5433",        8, "%.-42x",       -0x1234abcdu);
    TEST("1234ABCD",        8, "%.-42X",       0x1234abcdu);
    TEST("EDCB5433",        8, "%.-42X",       -0x1234abcdu);
#endif

    /*
     * Präzision und Feldbreite aus Parameter.
     * + hat Vorrang vor <space>, - hat Vorrang vor 0 (das eh ignoriert wird,
     * weil eine Präzision angegeben ist);
     */
    TEST("Hallo               ",   20, "% -0+*.*s",    20,  5, "Hallo heimur");
    CMP_SPRINTF("%.*d",   -6,  0);
#if 1
    TEST("+01024              ",   20, "% -0+*.*d",    20,  5,  1024);
    TEST("-01024              ",   20, "% -0+*.*d",    20,  5,  -1024);
    TEST("+01024              ",   20, "% -0+*.*i",    20,  5,  1024);
    TEST("-01024              ",   20, "% 0-+*.*i",    20,  5,  -1024);
    TEST("01024               ",   20, "% 0-+*.*u",    20,  5,  1024u);
    TEST("4294966272          ",   20, "% 0-+*.*u",    20,  5,  -1024u);
    TEST("00777               ",   20, "%+ -0*.*o",    20,  5,  0777u);
    TEST("37777777001         ",   20, "%+ -0*.*o",    20,  5,  -0777u);
    TEST("1234abcd            ",   20, "%+ -0*.*x",    20,  5,  0x1234abcdu);
    TEST("00edcb5433          ",   20, "%+ -0*.*x",    20, 10,  -0x1234abcdu);
    TEST("1234ABCD            ",   20, "% -+0*.*X",    20,  5,  0x1234abcdu);
    TEST("00EDCB5433          ",   20, "% -+0*.*X",    20, 10,  -0x1234abcdu);
#endif
}

#define TEST_REDUNDANT_FLAGS 1
#define TEST_NON_STANDARD 1
#define TEST_IMPL_DEFINED 1

#define TEST_SNPRINTF(par, exp) CMP_SPRINTF par
#define TEST_SNPRINTF_N(par, len) CMP_SPRINTF par

#define cur_snprintf snprintf

#define REQUIRE_STR_EQ(s1,s2) REQUIRE_THAT(s1, Equals(s2))

TEST_CASE("libsanity printf 1", "[]")
{
  CMP_DEF;

  TEST_SNPRINTF(("%d", -1000), "-1000");
  CMP_SNPRINTF(3U, "%d", -1000);

  TEST_SNPRINTF(("%d", -1), "-1");

  CMP_SNPRINTF(sizeof(buffer), "%d %d %s", 3, -1000, "test");

  TEST_SNPRINTF(("% d", 42), " 42");
  TEST_SNPRINTF(("% d", -42), "-42");
  TEST_SNPRINTF(("% 5d", 42), "   42");
  TEST_SNPRINTF(("% 5d", -42), "  -42");
  TEST_SNPRINTF(("% 15d", 42), "             42");
  TEST_SNPRINTF(("% 15d", -42), "            -42");
  TEST_SNPRINTF(("% 15d", -42), "            -42");
  TEST_SNPRINTF(("% 15.3f", -42.987), "        -42.987");
  TEST_SNPRINTF(("% 15.3f", 42.987), "         42.987");
#if TEST_REDUNDANT_FLAGS
  TEST_SNPRINTF(("% s", "Hello testing"), "Hello testing");
#endif

  TEST_SNPRINTF(("% d", 1024), " 1024");
  TEST_SNPRINTF(("% d", -1024), "-1024");
  TEST_SNPRINTF(("% i", 1024), " 1024");
  TEST_SNPRINTF(("% i", -1024), "-1024");
#if TEST_REDUNDANT_FLAGS
  TEST_SNPRINTF(("% u", 1024), "1024");
  TEST_SNPRINTF(("% u", 4294966272U), "4294966272");
  TEST_SNPRINTF(("% o", 511), "777");
  TEST_SNPRINTF(("% o", 4294966785U), "37777777001");
  TEST_SNPRINTF(("% x", 305441741), "1234abcd");
  TEST_SNPRINTF(("% x", 3989525555U), "edcb5433");
  TEST_SNPRINTF(("% X", 305441741), "1234ABCD");
  TEST_SNPRINTF(("% X", 3989525555U), "EDCB5433");
  TEST_SNPRINTF(("% c", 'x'), "x");
#endif

  TEST_SNPRINTF(("%+d", 42), "+42");
  TEST_SNPRINTF(("%+d", -42), "-42");
  TEST_SNPRINTF(("%+5d", 42), "  +42");
  TEST_SNPRINTF(("%+5d", -42), "  -42");
  TEST_SNPRINTF(("%+15d", 42), "            +42");
  TEST_SNPRINTF(("%+15d", -42), "            -42");
#if TEST_REDUNDANT_FLAGS
  TEST_SNPRINTF(("%+s", "Hello testing"), "Hello testing");
#endif

  TEST_SNPRINTF(("%+d", 1024), "+1024");
  TEST_SNPRINTF(("%+d", -1024), "-1024");
  TEST_SNPRINTF(("%+i", 1024), "+1024");
  TEST_SNPRINTF(("%+i", -1024), "-1024");
#if TEST_REDUNDANT_FLAGS
  TEST_SNPRINTF(("%+u", 1024), "1024");
  TEST_SNPRINTF(("%+u", 4294966272U), "4294966272");
  TEST_SNPRINTF(("%+o", 511), "777");
  TEST_SNPRINTF(("%+o", 4294966785U), "37777777001");
  TEST_SNPRINTF(("%+x", 305441741), "1234abcd");
  TEST_SNPRINTF(("%+x", 3989525555U), "edcb5433");
  TEST_SNPRINTF(("%+X", 305441741), "1234ABCD");
  TEST_SNPRINTF(("%+X", 3989525555U), "EDCB5433");
  TEST_SNPRINTF(("%+c", 'x'), "x");
#endif

  TEST_SNPRINTF(("%0d", 42), "42");
  TEST_SNPRINTF(("%0ld", 42L), "42");
  TEST_SNPRINTF(("%0d", -42), "-42");
  TEST_SNPRINTF(("%05d", 42), "00042");
  TEST_SNPRINTF(("%05d", -42), "-0042");
  TEST_SNPRINTF(("%015d", 42), "000000000000042");
  TEST_SNPRINTF(("%015d", -42), "-00000000000042");
  TEST_SNPRINTF(("%015.2f", 42.1234), "000000000042.12");
  TEST_SNPRINTF(("%015.3f", 42.9876), "00000000042.988");
  TEST_SNPRINTF(("%015.5f", -42.9876), "-00000042.98760");
  TEST_SNPRINTF(("%-d", 42), "42");
  TEST_SNPRINTF(("%-d", -42), "-42");
  TEST_SNPRINTF(("%-5d", 42), "42   ");
  TEST_SNPRINTF(("%-5d", -42), "-42  ");
  TEST_SNPRINTF(("%-15d", 42), "42             ");
  TEST_SNPRINTF(("%-15d", -42), "-42            ");
#if TEST_REDUNDANT_FLAGS
  TEST_SNPRINTF(("%-0d", 42), "42");
  TEST_SNPRINTF(("%-0d", -42), "-42");
  TEST_SNPRINTF(("%-05d", 42), "42   ");
  TEST_SNPRINTF(("%-05d", -42), "-42  ");
  TEST_SNPRINTF(("%-015d", 42), "42             ");
  TEST_SNPRINTF(("%-015d", -42), "-42            ");
  TEST_SNPRINTF(("%0-d", 42), "42");
  TEST_SNPRINTF(("%0-d", -42), "-42");
  TEST_SNPRINTF(("%0-5d", 42), "42   ");
  TEST_SNPRINTF(("%0-5d", -42), "-42  ");
  TEST_SNPRINTF(("%0-15d", 42), "42             ");
  TEST_SNPRINTF(("%0-15d", -42), "-42            ");
#endif
}

TEST_CASE("libsanity printf 2", "[]")
{
  CMP_DEF;

  CMP_SNPRINTF(sizeof(buffer), "Hello testing");

  TEST_SNPRINTF(("%s", "Hello testing"), "Hello testing");
  TEST_SNPRINTF(("%d", 1024), "1024");
  TEST_SNPRINTF(("%d", -1024), "-1024");
  TEST_SNPRINTF(("%i", 1024), "1024");
  TEST_SNPRINTF(("%i", -1024), "-1024");
  TEST_SNPRINTF(("%u", 1024), "1024");
  TEST_SNPRINTF(("%u", 4294966272U), "4294966272");
  TEST_SNPRINTF(("%o", 511), "777");
  TEST_SNPRINTF(("%o", 4294966785U), "37777777001");
  TEST_SNPRINTF(("%x", 305441741), "1234abcd");
  TEST_SNPRINTF(("%x", 3989525555U), "edcb5433");
  TEST_SNPRINTF(("%X", 305441741), "1234ABCD");
  TEST_SNPRINTF(("%X", 3989525555U), "EDCB5433");
  TEST_SNPRINTF(("%%"), "%");

  TEST_SNPRINTF(("%1s", "Hello testing"), "Hello testing");
  TEST_SNPRINTF(("%1d", 1024), "1024");
  TEST_SNPRINTF(("%1d", -1024), "-1024");
  TEST_SNPRINTF(("%1i", 1024), "1024");
  TEST_SNPRINTF(("%1i", -1024), "-1024");
  TEST_SNPRINTF(("%1u", 1024), "1024");
  TEST_SNPRINTF(("%1u", 4294966272U), "4294966272");
  TEST_SNPRINTF(("%1o", 511), "777");
  TEST_SNPRINTF(("%1o", 4294966785U), "37777777001");
  TEST_SNPRINTF(("%1x", 305441741), "1234abcd");
  TEST_SNPRINTF(("%1x", 3989525555U), "edcb5433");
  TEST_SNPRINTF(("%1X", 305441741), "1234ABCD");
  TEST_SNPRINTF(("%1X", 3989525555U), "EDCB5433");
  TEST_SNPRINTF(("%1c", 'x'), "x");
  TEST_SNPRINTF(("%20s", "Hello"), "               Hello");
  TEST_SNPRINTF(("%20d", 1024), "                1024");
  TEST_SNPRINTF(("%20d", -1024), "               -1024");
  TEST_SNPRINTF(("%20i", 1024), "                1024");
  TEST_SNPRINTF(("%20i", -1024), "               -1024");
  TEST_SNPRINTF(("%20u", 1024), "                1024");
  TEST_SNPRINTF(("%20u", 4294966272U), "          4294966272");
  TEST_SNPRINTF(("%20o", 511), "                 777");
  TEST_SNPRINTF(("%20o", 4294966785U), "         37777777001");
  TEST_SNPRINTF(("%20x", 305441741), "            1234abcd");
  TEST_SNPRINTF(("%20x", 3989525555U), "            edcb5433");
  TEST_SNPRINTF(("%20X", 305441741), "            1234ABCD");
  TEST_SNPRINTF(("%20X", 3989525555U), "            EDCB5433");
  TEST_SNPRINTF(("%20c", 'x'), "                   x");
  TEST_SNPRINTF(("%*s", 20, "Hello"), "               Hello");
  TEST_SNPRINTF(("%*d", 20, 1024), "                1024");
  TEST_SNPRINTF(("%*d", 20, -1024), "               -1024");
  TEST_SNPRINTF(("%*i", 20, 1024), "                1024");
  TEST_SNPRINTF(("%*i", 20, -1024), "               -1024");
  TEST_SNPRINTF(("%*u", 20, 1024), "                1024");
  TEST_SNPRINTF(("%*u", 20, 4294966272U), "          4294966272");
  TEST_SNPRINTF(("%*o", 20, 511), "                 777");
  TEST_SNPRINTF(("%*o", 20, 4294966785U), "         37777777001");
  TEST_SNPRINTF(("%*x", 20, 305441741), "            1234abcd");
  TEST_SNPRINTF(("%*x", 20, 3989525555U), "            edcb5433");
  TEST_SNPRINTF(("%*X", 20, 305441741), "            1234ABCD");
  TEST_SNPRINTF(("%*X", 20, 3989525555U), "            EDCB5433");
  TEST_SNPRINTF(("%*c", 20, 'x'), "                   x");
  TEST_SNPRINTF(("%-20s", "Hello"), "Hello               ");
  TEST_SNPRINTF(("%-20d", 1024), "1024                ");
  TEST_SNPRINTF(("%-20d", -1024), "-1024               ");
  TEST_SNPRINTF(("%-20i", 1024), "1024                ");
  TEST_SNPRINTF(("%-20i", -1024), "-1024               ");
  TEST_SNPRINTF(("%-20u", 1024), "1024                ");
  TEST_SNPRINTF(("%-20.4f", 1024.1234), "1024.1234           ");
  TEST_SNPRINTF(("%-20u", 4294966272U), "4294966272          ");
  TEST_SNPRINTF(("%-20o", 511), "777                 ");
  TEST_SNPRINTF(("%-20o", 4294966785U), "37777777001         ");
  TEST_SNPRINTF(("%-20x", 305441741), "1234abcd            ");
  TEST_SNPRINTF(("%-20x", 3989525555U), "edcb5433            ");
  TEST_SNPRINTF(("%-20X", 305441741), "1234ABCD            ");
  TEST_SNPRINTF(("%-20X", 3989525555U), "EDCB5433            ");
  TEST_SNPRINTF(("%-20c", 'x'), "x                   ");
  TEST_SNPRINTF(("|%5d| |%-2d| |%5d|", 9, 9, 9), "|    9| |9 | |    9|");
  TEST_SNPRINTF(("|%5d| |%-2d| |%5d|", 10, 10, 10), "|   10| |10| |   10|");
  TEST_SNPRINTF(("|%5d| |%-12d| |%5d|", 9, 9, 9),
                "|    9| |9           | |    9|");
  TEST_SNPRINTF(("|%5d| |%-12d| |%5d|", 10, 10, 10),
                "|   10| |10          | |   10|");
#if TEST_REDUNDANT_FLAGS
  TEST_SNPRINTF(("%0-20s", "Hello"), "Hello               ");
  TEST_SNPRINTF(("%0-20d", 1024), "1024                ");
  TEST_SNPRINTF(("%0-20d", -1024), "-1024               ");
  TEST_SNPRINTF(("%0-20i", 1024), "1024                ");
  TEST_SNPRINTF(("%0-20i", -1024), "-1024               ");
  TEST_SNPRINTF(("%0-20u", 1024), "1024                ");
  TEST_SNPRINTF(("%0-20u", 4294966272U), "4294966272          ");
  TEST_SNPRINTF(("%0-20o", 511), "777                 ");
  TEST_SNPRINTF(("%0-20o", 4294966785U), "37777777001         ");
  TEST_SNPRINTF(("%0-20x", 305441741), "1234abcd            ");
  TEST_SNPRINTF(("%0-20x", 3989525555U), "edcb5433            ");
  TEST_SNPRINTF(("%0-20X", 305441741), "1234ABCD            ");
  TEST_SNPRINTF(("%0-20X", 3989525555U), "EDCB5433            ");
  TEST_SNPRINTF(("%0-20c", 'x'), "x                   ");
#endif
}

TEST_CASE("libsanity printf 3", "[]")
{
  CMP_DEF;


  TEST_SNPRINTF(("%020d", 1024), "00000000000000001024");
  TEST_SNPRINTF(("%020d", -1024), "-0000000000000001024");
  TEST_SNPRINTF(("%020i", 1024), "00000000000000001024");
  TEST_SNPRINTF(("%020i", -1024), "-0000000000000001024");
  TEST_SNPRINTF(("%020u", 1024), "00000000000000001024");
  TEST_SNPRINTF(("%020u", 4294966272U), "00000000004294966272");
  TEST_SNPRINTF(("%020o", 511), "00000000000000000777");
  TEST_SNPRINTF(("%020o", 4294966785U), "00000000037777777001");
  TEST_SNPRINTF(("%020x", 305441741), "0000000000001234abcd");
  TEST_SNPRINTF(("%020x", 3989525555U), "000000000000edcb5433");
  TEST_SNPRINTF(("%020X", 305441741), "0000000000001234ABCD");
  TEST_SNPRINTF(("%020X", 3989525555U), "000000000000EDCB5433");
  TEST_SNPRINTF(("%.20d", 1024), "00000000000000001024");
  TEST_SNPRINTF(("%.20d", -1024), "-00000000000000001024");
  TEST_SNPRINTF(("%.20i", 1024), "00000000000000001024");
  TEST_SNPRINTF(("%.20i", -1024), "-00000000000000001024");
  TEST_SNPRINTF(("%.20u", 1024), "00000000000000001024");
  TEST_SNPRINTF(("%.20u", 4294966272U), "00000000004294966272");
  TEST_SNPRINTF(("%.20o", 511), "00000000000000000777");
  TEST_SNPRINTF(("%.20o", 4294966785U), "00000000037777777001");
  TEST_SNPRINTF(("%.20x", 305441741), "0000000000001234abcd");
  TEST_SNPRINTF(("%.20x", 3989525555U), "000000000000edcb5433");
  TEST_SNPRINTF(("%.20X", 305441741), "0000000000001234ABCD");
  TEST_SNPRINTF(("%.20X", 3989525555U), "000000000000EDCB5433");
#if TEST_REDUNDANT_FLAGS
  TEST_SNPRINTF(("%#020d", 1024), "00000000000000001024");
  TEST_SNPRINTF(("%#020d", -1024), "-0000000000000001024");
  TEST_SNPRINTF(("%#020i", 1024), "00000000000000001024");
  TEST_SNPRINTF(("%#020i", -1024), "-0000000000000001024");
  TEST_SNPRINTF(("%#020u", 1024), "00000000000000001024");
  TEST_SNPRINTF(("%#020u", 4294966272U), "00000000004294966272");
#endif

  TEST_SNPRINTF(("%#020o", 511), "00000000000000000777");
  TEST_SNPRINTF(("%#020o", 4294966785U), "00000000037777777001");
  TEST_SNPRINTF(("%#020x", 305441741), "0x00000000001234abcd");
  TEST_SNPRINTF(("%#020x", 3989525555U), "0x0000000000edcb5433");
  TEST_SNPRINTF(("%#020X", 305441741), "0X00000000001234ABCD");
  TEST_SNPRINTF(("%#020X", 3989525555U), "0X0000000000EDCB5433");
#if TEST_REDUNDANT_FLAGS
  TEST_SNPRINTF(("%#20d", 1024), "                1024");
  TEST_SNPRINTF(("%#20d", -1024), "               -1024");
  TEST_SNPRINTF(("%#20i", 1024), "                1024");
  TEST_SNPRINTF(("%#20i", -1024), "               -1024");
  TEST_SNPRINTF(("%#20u", 1024), "                1024");
  TEST_SNPRINTF(("%#20u", 4294966272U), "          4294966272");
#endif

  TEST_SNPRINTF(("%#20o", 511), "                0777");
  TEST_SNPRINTF(("%#20o", 4294966785U), "        037777777001");
  TEST_SNPRINTF(("%#20x", 305441741), "          0x1234abcd");
  TEST_SNPRINTF(("%#20x", 3989525555U), "          0xedcb5433");
  TEST_SNPRINTF(("%#20X", 305441741), "          0X1234ABCD");
  TEST_SNPRINTF(("%#20X", 3989525555U), "          0XEDCB5433");
  TEST_SNPRINTF(("%20.5d", 1024), "               01024");
  TEST_SNPRINTF(("%20.5d", -1024), "              -01024");
  TEST_SNPRINTF(("%20.5i", 1024), "               01024");
  TEST_SNPRINTF(("%20.5i", -1024), "              -01024");
  TEST_SNPRINTF(("%20.5u", 1024), "               01024");
  TEST_SNPRINTF(("%20.5u", 4294966272U), "          4294966272");
  TEST_SNPRINTF(("%20.5o", 511), "               00777");
  TEST_SNPRINTF(("%20.5o", 4294966785U), "         37777777001");
  TEST_SNPRINTF(("%20.5x", 305441741), "            1234abcd");
  TEST_SNPRINTF(("%20.10x", 3989525555U), "          00edcb5433");
  TEST_SNPRINTF(("%20.5X", 305441741), "            1234ABCD");
  TEST_SNPRINTF(("%20.10X", 3989525555U), "          00EDCB5433");
  TEST_SNPRINTF(("%.0s", "Hello testing"), "");
  TEST_SNPRINTF(("%20.0s", "Hello testing"), "                    ");
  TEST_SNPRINTF(("%.s", "Hello testing"), "");
  TEST_SNPRINTF(("%20.s", "Hello testing"), "                    ");
  TEST_SNPRINTF(("%20.0d", 1024), "                1024");
  TEST_SNPRINTF(("%20.0d", -1024), "               -1024");
  TEST_SNPRINTF(("%20.d", 0), "                    ");
  TEST_SNPRINTF(("%20.0i", 1024), "                1024");
  TEST_SNPRINTF(("%20.i", -1024), "               -1024");
  TEST_SNPRINTF(("%20.i", 0), "                    ");
  TEST_SNPRINTF(("%20.u", 1024), "                1024");
  TEST_SNPRINTF(("%20.0u", 4294966272U), "          4294966272");
  TEST_SNPRINTF(("%20.u", 0U), "                    ");
  TEST_SNPRINTF(("%20.o", 511), "                 777");
  TEST_SNPRINTF(("%20.0o", 4294966785U), "         37777777001");
  TEST_SNPRINTF(("%20.o", 0U), "                    ");
  TEST_SNPRINTF(("%20.x", 305441741), "            1234abcd");
  TEST_SNPRINTF(("%50.x", 305441741),
                "                                          1234abcd");
  TEST_SNPRINTF(("%50.x%10.u", 305441741, 12345),
                "                                          1234abcd     12345");
  TEST_SNPRINTF(("%20.0x", 3989525555U), "            edcb5433");
  TEST_SNPRINTF(("%20.x", 0U), "                    ");
  TEST_SNPRINTF(("%20.X", 305441741), "            1234ABCD");
  TEST_SNPRINTF(("%20.0X", 3989525555U), "            EDCB5433");
  TEST_SNPRINTF(("%20.X", 0U), "                    ");
  TEST_SNPRINTF(("%.4f", 3.1415354), "3.1415");
  TEST_SNPRINTF(("%.3f", 30343.1415354), "30343.142");
  TEST_SNPRINTF(("%.0f", 34.1415354), "34");
  TEST_SNPRINTF(("%.2f", 42.8952), "42.90");
  TEST_SNPRINTF(("%.9f", 42.8952), "42.895200000");
  TEST_SNPRINTF(("%.10f", 42.895223), "42.8952230000");
// TODO limited number of digits
//  TEST_SNPRINTF(("%.12f", 42.89522312345678), "42.895223123457");
//  TEST_SNPRINTF(("%.12f", 42.89522387654321), "42.895223876543");
  TEST_SNPRINTF(("%6.2f", 42.8952), " 42.90");
  TEST_SNPRINTF(("%+6.2f", 42.8952), "+42.90");
  TEST_SNPRINTF(("%+5.1f", 42.9252), "+42.9");
  TEST_SNPRINTF(("%f", 42.5), "42.500000");
  TEST_SNPRINTF(("%.1f", 42.5), "42.5");
  TEST_SNPRINTF(("%f", 42167.0), "42167.000000");
  TEST_SNPRINTF(("%.9f", -12345.987654321), "-12345.987654321");
  TEST_SNPRINTF(("%.1f", 3.999), "4.0");
  TEST_SNPRINTF(("%.0f", 3.5), "4");
  TEST_SNPRINTF(("%.0f", 3.49), "3");
  TEST_SNPRINTF(("%.1f", 3.49), "3.5");
// TODO - automatic switch to %g
//  TEST_SNPRINTF(("%.1f", 1E20), "100000000000000000000.0");
  TEST_SNPRINTF(("a%-5.1f", 0.5), "a0.5  ");
  TEST_SNPRINTF(("%i", 0), "0");
  TEST_SNPRINTF(("%i", 1234), "1234");
  TEST_SNPRINTF(("%i", 32767), "32767");
  TEST_SNPRINTF(("%i", -32767), "-32767");
  TEST_SNPRINTF(("%li", 30L), "30");
  TEST_SNPRINTF(("%li", -2147483647L), "-2147483647");
  TEST_SNPRINTF(("%li", 2147483647L), "2147483647");
  TEST_SNPRINTF(("%lli", 30LL), "30");
  TEST_SNPRINTF(("%lli", -9223372036854775807LL), "-9223372036854775807");
  TEST_SNPRINTF(("%lli", 9223372036854775807LL), "9223372036854775807");
  TEST_SNPRINTF(("%lu", 100000L), "100000");
  TEST_SNPRINTF(("%lu", 0xFFFFFFFFL), "4294967295");
  TEST_SNPRINTF(("%llu", 281474976710656LLU), "281474976710656");
  TEST_SNPRINTF(("%llu", 18446744073709551615LLU), "18446744073709551615");
  TEST_SNPRINTF(("%zu", (size_t)2147483647UL), "2147483647");
  TEST_SNPRINTF(("%zd", (ptrdiff_t)2147483647UL), "2147483647");
  TEST_SNPRINTF(("%tu", (size_t)2147483647UL), "2147483647");
  TEST_SNPRINTF(("%td", (ptrdiff_t)2147483647UL), "2147483647");
#if TEST_NON_STANDARD
  // Unportable extension in original printf implementation.
  TEST_SNPRINTF(("%b", 60000), "1110101001100000");
  TEST_SNPRINTF(("%lb", 12345678L), "101111000110000101001110");
  TEST_SNPRINTF(("%#b", 60000), "0b1110101001100000");
#endif
}

TEST_CASE("libsanity printf 4", "[]")
{
  CMP_DEF;


  TEST_SNPRINTF(("%o", 60000), "165140");
  TEST_SNPRINTF(("%lo", 12345678L), "57060516");
  TEST_SNPRINTF(("%lx", 0x12345678L), "12345678");
  TEST_SNPRINTF(("%llx", 0x1234567891234567LLU), "1234567891234567");
  TEST_SNPRINTF(("%lx", 0xabcdefabL), "abcdefab");
  TEST_SNPRINTF(("%lX", 0xabcdefabL), "ABCDEFAB");
  TEST_SNPRINTF(("%c", 'v'), "v");
  TEST_SNPRINTF(("%cv", 'w'), "wv");
  TEST_SNPRINTF(("%s", "A Test"), "A Test");
  TEST_SNPRINTF(("%hhu", (unsigned char)0xFFFFU), "255");
  TEST_SNPRINTF(("%hu", (unsigned short)0x123456U), "13398");
  TEST_SNPRINTF(("%s%hhi %hu", "Test", 10000, 0xFFFFFFFF), "Test16 65535");
  TEST_SNPRINTF(("%tx", &buffer[10] - &buffer[0]), "a");
  TEST_SNPRINTF(("%ji", (intmax_t)-2147483647L), "-2147483647");
  TEST_SNPRINTF(("%ju", (uintmax_t)2147483647UL), "2147483647");

  TEST_SNPRINTF(("%.*d", -1, 1), "1");
  TEST_SNPRINTF(("%.*d", -1, 0), "0");

  TEST_SNPRINTF(("%hhd", -1), "-1");

#if TEST_IMPL_DEFINED
  CMP_SNPRINTF(sizeof(buffer), "%p", (void *)(uintptr_t)0x1234U);

  CMP_SNPRINTF(sizeof(buffer), "%p", (void *)(uintptr_t)0x12345678U);

  CMP_SNPRINTF(sizeof(buffer), "%p-%p", (void *)0x12345678U,
               (void *)(uintptr_t)0x7EDCBA98U);

  CMP_SNPRINTF(sizeof(buffer), "%p",
               (void *)(uintptr_t)0xFFFFFFFFU);
#endif

  buffer[0] = (char)0xA5;
  ret = cur_snprintf(buffer, 0, "%s", "Test");
  REQUIRE(buffer[0] == (char)0xA5);
  REQUIRE(ret == 4);

  buffer[0] = (char)0xCC;
  cur_snprintf(buffer, 1, "%s", "Test");
  REQUIRE(buffer[0] == '\0');

  cur_snprintf(buffer, 2, "%s", "Hello");
  REQUIRE_STR_EQ(buffer, "H");

  ret = cur_snprintf(buffer, 6, "0%s", "1234");
  REQUIRE_STR_EQ(buffer, "01234");
  REQUIRE(ret == 5);

  ret = cur_snprintf(buffer, 6, "0%s", "12345");
  REQUIRE_STR_EQ(buffer, "01234");
  REQUIRE(ret == 6);  // '5' is truncated

  ret = cur_snprintf(buffer, 6, "0%s", "1234567");
  REQUIRE_STR_EQ(buffer, "01234");
  REQUIRE(ret == 8);  // '567' are truncated

  ret = cur_snprintf(buffer, 10, "hello, world");
  REQUIRE(ret == 12);

  ret = cur_snprintf(buffer, 3, "%d", 10000);
  REQUIRE(ret == 5);
  REQUIRE(strlen(buffer) == 2U);
  REQUIRE(buffer[0] == '1');
  REQUIRE(buffer[1] == '0');
  REQUIRE(buffer[2] == '\0');

  TEST_SNPRINTF(("%u%u%ctest%d %s", 5, 3000, 'a', -20, "bit"),
                "53000atest-20 bit");
  TEST_SNPRINTF(("%.*f", 2, 0.33333333), "0.33");
  TEST_SNPRINTF(("%.3s", "foobar"), "foo");
  TEST_SNPRINTF(("%10.5d", 4), "     00004");
  TEST_SNPRINTF(("%*sx", -3, "hi"), "hi x");

#if INT_MAX == 2147483647
  TEST_SNPRINTF(("%d", INT_MAX), "2147483647");
  TEST_SNPRINTF(("%d", INT_MIN), "-2147483648");
  TEST_SNPRINTF(("%u", UINT_MAX), "4294967295");
#endif
#if LONG_MAX == 2147483647L
  TEST_SNPRINTF(("%ld", LONG_MAX), "2147483647");
  TEST_SNPRINTF(("%ld", LONG_MIN), "-2147483648");
  TEST_SNPRINTF(("%lu", ULONG_MAX), "4294967295");
#endif
#if LONG_MAX == 9223372036854775807L
  TEST_SNPRINTF(("%ld", LONG_MAX), "9223372036854775807");
  TEST_SNPRINTF(("%ld", LONG_MIN), "-9223372036854775808");
  TEST_SNPRINTF(("%lu", ULONG_MAX), "18446744073709551615");
#endif
  TEST_SNPRINTF(("%" PRIi32, INT32_MAX), "2147483647");
  TEST_SNPRINTF(("%" PRIi32, INT32_MIN), "-2147483648");
  TEST_SNPRINTF(("%" PRIi64, INT64_MAX), "9223372036854775807");
  TEST_SNPRINTF(("%" PRIi64, INT64_MIN), "-9223372036854775808");
  TEST_SNPRINTF(("%" PRIu64, UINT64_MAX), "18446744073709551615");

#if TEST_IMPL_DEFINED
  // libinsanity/Microsoft extensions for explicitly sized integer types
  TEST_SNPRINTF(("%I64d", INT64_MIN), "-9223372036854775808");
  TEST_SNPRINTF(("%I32d", INT32_MIN), "-2147483648");
  TEST_SNPRINTF(("%Id", (ptrdiff_t)-123), "-123");
  TEST_SNPRINTF(("%Iu", (size_t)123), "123");
  // libinsanity only extensions
  TEST_SNPRINTF(("%I8u", 10000), "16");
  TEST_SNPRINTF(("%I8d", 255), "-1");
  TEST_SNPRINTF(("%I16u", 100000), "34464");
  TEST_SNPRINTF(("%I16d", 65535), "-1");
  TEST_SNPRINTF_N(("%I34d", 123), -1);
#endif

  TEST_SNPRINTF(("%x", 0), "0");
  TEST_SNPRINTF(("%#x", 0), "0");
  TEST_SNPRINTF(("%#04x", 0), "0000");
  TEST_SNPRINTF(("%#08x", 0x614e), "0x00614e");
  TEST_SNPRINTF(("%#.3x", 0x614e), "0x614e");
  TEST_SNPRINTF(("%#.4x", 0x614e), "0x614e");
  TEST_SNPRINTF(("%#.5x", 0x614e), "0x0614e");
  TEST_SNPRINTF(("%#.6x", 0x614e), "0x00614e");
  TEST_SNPRINTF(("%#.7x", 0x614e), "0x000614e");

  TEST_SNPRINTF(("%o", 00), "0");
  TEST_SNPRINTF(("%#o", 00), "0");
  TEST_SNPRINTF(("%#04o", 0), "0000");
  TEST_SNPRINTF(("%#08o", 06143), "00006143");
  TEST_SNPRINTF(("%#.3o", 06143), "06143");
  TEST_SNPRINTF(("%#.4o", 06143), "06143");
  TEST_SNPRINTF(("%#.5o", 06143), "06143");   // TODO!
  TEST_SNPRINTF(("%#.6o", 06143), "006143");
  TEST_SNPRINTF(("%#.7o", 06143), "0006143");
}

TEST_CASE("libsanity printf 5", "[]")
{
  CMP_DEF;

  // libc-testsuite tests

  /* width, precision, alignment */
  TEST_SNPRINTF(("%04d", 12), "0012");
  TEST_SNPRINTF(("%.3d", 12), "012");
  TEST_SNPRINTF(("%3d", 12), " 12");
  TEST_SNPRINTF(("%-3d", 12), "12 ");
  TEST_SNPRINTF(("%+3d", 12), "+12");
  TEST_SNPRINTF(("%+-5d", 12), "+12  ");
#if TEST_REDUNDANT_FLAGS
  TEST_SNPRINTF(("%+- 5d", 12), "+12  ");
#endif
  TEST_SNPRINTF(("%- 5d", 12), " 12  ");
  TEST_SNPRINTF(("% d", 12), " 12");
#if TEST_REDUNDANT_FLAGS
  TEST_SNPRINTF(("%0-5d", 12), "12   ");
  TEST_SNPRINTF(("%-05d", 12), "12   ");
#endif

}

TEST_CASE("libsanity printf 6", "[]")
{
  CMP_DEF;


  /* ...explicit precision of 0 shall be no characters. */
  TEST_SNPRINTF(("%.0d", 0), "");
  TEST_SNPRINTF(("%.0o", 0), "");
#if TEST_REDUNDANT_FLAGS
  TEST_SNPRINTF(("%#.0d", 0), "");
#endif
  // Note: the original libc-testsuite specifies "" as expected.
  TEST_SNPRINTF(("%#.0o", 0), "0");
  TEST_SNPRINTF(("%#.0x", 0), "");
  TEST_SNPRINTF(("%#3.0x", 0), "   ");

  /* ...but it still has to honor width and flags. */
  TEST_SNPRINTF(("%2.0u", 0), "  ");
#if TEST_REDUNDANT_FLAGS
  TEST_SNPRINTF(("%02.0u", 0), "  ");
  TEST_SNPRINTF(("%02.0d", 0), "  ");
#endif
  TEST_SNPRINTF(("%2.0d", 0), "  ");
  TEST_SNPRINTF(("% .0d", 0), " ");
  TEST_SNPRINTF(("%+.0d", 0), "+");

  /* hex: test alt form and case */
  TEST_SNPRINTF(("%x", 63), "3f");
  TEST_SNPRINTF(("%#x", 63), "0x3f");
  TEST_SNPRINTF(("%X", 63), "3F");

  /* octal: test alt form */
  TEST_SNPRINTF(("%o", 15), "17");
  TEST_SNPRINTF(("%#o", 15), "017");

  /* basic form, handling of exponent/precision for 0 */
  TEST_SNPRINTF(("%e", 0.0), "0.000000e+00");
  TEST_SNPRINTF(("%f", 0.0), "0.000000");
  TEST_SNPRINTF(("%g", 0.0), "0");
  TEST_SNPRINTF(("%#g", 0.0), "0.00000");

  /* rounding */
  TEST_SNPRINTF(("%f", 1.1), "1.100000");
  TEST_SNPRINTF(("%f", 1.2), "1.200000");
  TEST_SNPRINTF(("%f", 1.3), "1.300000");
  TEST_SNPRINTF(("%f", 1.4), "1.400000");
  TEST_SNPRINTF(("%f", 1.5), "1.500000");
  // Note: the original libc-testsuite test specifies "1.0612" as expected.
  TEST_SNPRINTF(("%.4f", 1.06125), "1.0613");
  TEST_SNPRINTF(("%.2f", 1.375), "1.38");
  TEST_SNPRINTF(("%.1f", 1.375), "1.4");
  TEST_SNPRINTF(("%.15f", 1.1), "1.100000000000000");
  TEST_SNPRINTF(("%.16f", 1.1), "1.1000000000000001");
  TEST_SNPRINTF(("%.17f", 1.1), "1.10000000000000009");
  TEST_SNPRINTF(("%.2e", 1500001.0), "1.50e+06");
  TEST_SNPRINTF(("%.2e", 1505000.0), "1.50e+06");
  TEST_SNPRINTF(("%.2e", 1505000.00000095367431640625), "1.51e+06");
  TEST_SNPRINTF(("%.2e", 1505001.0), "1.51e+06");
  TEST_SNPRINTF(("%.2e", 1506000.0), "1.51e+06");

  /* correctness in DBL_DIG places */
  TEST_SNPRINTF(("%.15g", 1.23456789012345), "1.23456789012345");

  /* correct choice of notation for %g */
  TEST_SNPRINTF(("%g", 0.0001), "0.0001");
  TEST_SNPRINTF(("%g", 0.00001), "1e-05");
  TEST_SNPRINTF(("%g", 123456.0), "123456");
  TEST_SNPRINTF(("%g", 1234567.0), "1.23457e+06");
  TEST_SNPRINTF(("%.7g", 1234567.0), "1234567");
  TEST_SNPRINTF(("%.7g", 12345678.0), "1.234568e+07");
  TEST_SNPRINTF(("%.8g", 0.1), "0.1");
  TEST_SNPRINTF(("%.9g", 0.1), "0.1");
  TEST_SNPRINTF(("%.10g", 0.1), "0.1");
  TEST_SNPRINTF(("%.11g", 0.1), "0.1");

  /* pi in double precision, printed to a few extra places */
  TEST_SNPRINTF(("%.15f", M_PI), "3.141592653589793");
  TEST_SNPRINTF(("%.18f", M_PI), "3.141592653589793116");

  /* exact conversion of large integers */
  TEST_SNPRINTF(("%.0f", 340282366920938463463374607431768211456.0),
                "340282366920938463463374607431768211456");

  TEST_SNPRINTF_N(("%d", 123456), 6);
  TEST_SNPRINTF_N(("%.4s", "hello"), 4);
  TEST_SNPRINTF_N(("%.0s", "goodbye"), 0);

  {
    char b[] = "xxxxxxxx";
    const char *s = "%d";
    int res = cur_snprintf(b, 4, s, 123456);
    REQUIRE(res == 6);
    REQUIRE_STR_EQ(b, "123");
    REQUIRE(b[5] == 'x'); // buffer overrun
  }

  {
    char b[2000];
    /* Perform ascii arithmetic to test printing tiny doubles */
    int res = cur_snprintf(b, sizeof(b), "%.1022f", 0x1p0-1021);
    REQUIRE(res == 1024);
    b[1] = '0';
    int i, k, j;
    for (i = 0; i < 1021; i++) {
      for (k = 0, j = 1023; j > 0; j--) {
        if (b[j] < '5')
          b[j] += b[j] - '0' + k, k = 0;
        else
          b[j] += b[j] - '0' - 10 + k, k = 1;
      }
    }
    REQUIRE(b[1] == '1');
    for (j = 2; b[j] == '0'; j++);
    REQUIRE(j == 1024);
  }

  // Not all implementations handle this correctly (such as glibc). It also
  // can be slow.
#if TEST_IMPL_DEFINED
  TEST_SNPRINTF_N(("%.*u", 2147483647, 0), 2147483647);
  TEST_SNPRINTF_N(("%.*u ", 2147483647, 0), -1);
#endif

  TEST_SNPRINTF_N(("%.4a", 1.0), 11);

#if TEST_IMPL_DEFINED
//  int r = cur_snprintf(buffer, sizeof(buffer), "a%wb", &(int){0});
//  assert(r < 0);
//  REQUIRE_STR_EQ(buffer, "a%w<error>b");
#endif

#if TEST_IMPL_DEFINED && 0
  test_rprintf(cur_snprintf, buffer, sizeof(buffer), "%s_%s", "%s:%.3s",
               "hello", "world");
  REQUIRE_STR_EQ(buffer, "<start>hello_world<mid>hello:wor<end>");
#endif
}

