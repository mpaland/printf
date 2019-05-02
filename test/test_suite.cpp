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

using Catch::Matchers::Equals;

// store pointers to library functions before they are masked by macro
int (*lib_sprintf)(char * buffer, const char* fmt, ...) = sprintf;
int (*lib_snprintf)(char * buffer, size_t count, const char* fmt, ...) = snprintf;

#include "../printf.h"

int (*our_sprintf)(char * buffer, const char* fmt, ...) = sprintf;
int (*our_snprintf)(char * buffer, size_t count, const char* fmt, ...) = snprintf;


// pointers to tested versions
// allows running tests against standard library implementation
int (*tested_sprintf)(char * buffer, const char* fmt, ...) = our_sprintf;
int (*tested_snprintf)(char * buffer, size_t count, const char* fmt, ...) = our_snprintf;

// dummy putchar
static char   printf_buffer[100];
static size_t printf_idx = 0U;

void _putchar(char character)
{
  printf_buffer[printf_idx++] = character;
}

void _out_fct(char character, void* arg)
{
  (void)arg;
  printf_buffer[printf_idx++] = character;
}

// run basic sanity tests

TEST_CASE("printf", "[]" ) {
  printf_idx = 0U;
  memset(printf_buffer, 0xCC, 100U);
  REQUIRE(printf("% d", 4232) == 5);
  REQUIRE(printf_buffer[5] == (char)0xCC);
  printf_buffer[5] = 0;
  REQUIRE(!strcmp(printf_buffer, " 4232"));
}


TEST_CASE("fctprintf", "[]" ) {
  printf_idx = 0U;
  memset(printf_buffer, 0xCC,  sizeof(printf_buffer));
  int ret = fctprintf(&_out_fct, nullptr, "This is a test of %X", 0x12EFU);
  REQUIRE(ret == 22U);
  REQUIRE(!strncmp(printf_buffer, "This is a test of 12EF", 22U));
  REQUIRE(printf_buffer[22] == (char)0xCC);
}


TEST_CASE("snprintf", "[]" ) {
  char buffer[100];
  memset(buffer, 0xCC, sizeof(buffer));
  snprintf(buffer, 100U, "%d", -1000);
  REQUIRE_THAT(buffer, Equals("-1000"));

  snprintf(buffer, 3U, "%d", -1000);
  REQUIRE_THAT(buffer, Equals("-1"));
}

static void vprintf_builder_1(char* buffer, ...)
{
  va_list args;
  va_start(args, buffer);
  vprintf("%d", args);
  va_end(args);
}

static void vsnprintf_builder_1(char* buffer, ...)
{
  va_list args;
  va_start(args, buffer);
  vsnprintf(buffer, 100U, "%d", args);
  va_end(args);
}

static void vsnprintf_builder_3(char* buffer, ...)
{
  va_list args;
  va_start(args, buffer);
  vsnprintf(buffer, 100U, "%d %d %s", args);
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

// macros to wrap tests
// variables with result are available for further test cases
#define TEST_DEF                                        \
  char buffer[100];                                     \
  int ret;                                              \
  struct dummy                                          \
  /**/

#define TEST_SPRINTF(expected, ...)                     \
  do {                                                  \
    CAPTURE(__VA_ARGS__);                               \
    ret = tested_sprintf(buffer, __VA_ARGS__);          \
    CHECK_THAT(buffer, Equals(expected));               \
    (void)ret;                                          \
  } while (0)                                           \
    /**/

#define TEST_SNPRINTF(expected, retval, ...)            \
  do {                                                  \
    CAPTURE(__VA_ARGS__);                               \
    ret = tested_snprintf(buffer, __VA_ARGS__);         \
    if (expected)                                       \
      CHECK_THAT(buffer, Equals(expected));             \
    CHECK(ret == retval);                               \
  } while (0)                                           \
    /**/



TEST_CASE("space flag", "[]" ) {
  TEST_DEF;

  TEST_SPRINTF(" 42",
               "% d", 42);

  TEST_SPRINTF("-42",
               "% d", -42);
  TEST_SPRINTF("   42",
               "% 5d", 42);
  TEST_SPRINTF("  -42",
               "% 5d", -42);
  TEST_SPRINTF("             42",
               "% 15d", 42);
  TEST_SPRINTF("            -42",
               "% 15d", -42);
  TEST_SPRINTF("            -42",
               "% 15d", -42);
  TEST_SPRINTF("        -42.987",
               "% 15.3f", -42.987);
  TEST_SPRINTF("         42.987",
               "% 15.3f", 42.987);
  TEST_SPRINTF("Hello testing",
               "% s", "Hello testing");
  TEST_SPRINTF(" 1024",
               "% d", 1024);
  TEST_SPRINTF("-1024",
               "% d", -1024);
  TEST_SPRINTF(" 1024",
               "% i", 1024);
  TEST_SPRINTF("-1024",
               "% i", -1024);
  TEST_SPRINTF("1024",
               "% u", 1024);
  TEST_SPRINTF("4294966272",
               "% u", 4294966272U);
  TEST_SPRINTF("777",
               "% o", 511);
  TEST_SPRINTF("37777777001",
               "% o", 4294966785U);
  TEST_SPRINTF("1234abcd",
               "% x", 305441741);
  TEST_SPRINTF("edcb5433",
               "% x", 3989525555U);
  TEST_SPRINTF("1234ABCD",
               "% X", 305441741);
  TEST_SPRINTF("EDCB5433",
               "% X", 3989525555U);
  TEST_SPRINTF("x",
               "% c", 'x');
}


TEST_CASE("+ flag", "[]" ) {
  TEST_DEF;

  TEST_SPRINTF("+42",
               "%+d", 42);
  TEST_SPRINTF("-42",
               "%+d", -42);
  TEST_SPRINTF("  +42",
               "%+5d", 42);
  TEST_SPRINTF("  -42",
               "%+5d", -42);
  TEST_SPRINTF("            +42",
               "%+15d", 42);
  TEST_SPRINTF("            -42",
               "%+15d", -42);
  TEST_SPRINTF("Hello testing",
               "%+s", "Hello testing");
  TEST_SPRINTF("+1024",
               "%+d", 1024);
  TEST_SPRINTF("-1024",
               "%+d", -1024);
  TEST_SPRINTF("+1024",
               "%+i", 1024);
  TEST_SPRINTF("-1024",
               "%+i", -1024);
  TEST_SPRINTF("1024",
               "%+u", 1024);
  TEST_SPRINTF("4294966272",
               "%+u", 4294966272U);
  TEST_SPRINTF("777",
               "%+o", 511);
  TEST_SPRINTF("37777777001",
               "%+o", 4294966785U);
  TEST_SPRINTF("1234abcd",
               "%+x", 305441741);
  TEST_SPRINTF("edcb5433",
               "%+x", 3989525555U);
  TEST_SPRINTF("1234ABCD",
               "%+X", 305441741);
  TEST_SPRINTF("EDCB5433",
               "%+X", 3989525555U);
  TEST_SPRINTF("x",
               "%+c", 'x');
  TEST_SPRINTF("+",
               "%+.0d", 0);
}


TEST_CASE("0 flag", "[]" ) {
  TEST_DEF;

  TEST_SPRINTF("42",
               "%0d", 42);
  TEST_SPRINTF("42",
               "%0ld", 42L);
  TEST_SPRINTF("-42",
               "%0d", -42);
  TEST_SPRINTF("00042",
               "%05d", 42);
  TEST_SPRINTF("-0042",
               "%05d", -42);
  TEST_SPRINTF("000000000000042",
               "%015d", 42);
  TEST_SPRINTF("-00000000000042",
               "%015d", -42);
  TEST_SPRINTF("000000000042.12",
               "%015.2f", 42.1234);
  TEST_SPRINTF("00000000042.988",
               "%015.3f", 42.9876);
  TEST_SPRINTF("-00000042.98760",
               "%015.5f", -42.9876);}


TEST_CASE("- flag", "[]" ) {
  TEST_DEF;

  TEST_SPRINTF("42",
               "%-d", 42);
  TEST_SPRINTF("-42",
               "%-d", -42);
  TEST_SPRINTF("42   ",
               "%-5d", 42);
  TEST_SPRINTF("-42  ",
               "%-5d", -42);
  TEST_SPRINTF("42             ",
               "%-15d", 42);
  TEST_SPRINTF("-42            ",
               "%-15d", -42);
  TEST_SPRINTF("42",
               "%-0d", 42);
  TEST_SPRINTF("-42",
               "%-0d", -42);
  TEST_SPRINTF("42   ",
               "%-05d", 42);
  TEST_SPRINTF("-42  ",
               "%-05d", -42);
  TEST_SPRINTF("42             ",
               "%-015d", 42);
  TEST_SPRINTF("-42            ",
               "%-015d", -42);
  TEST_SPRINTF("42",
               "%0-d", 42);
  TEST_SPRINTF("-42",
               "%0-d", -42);
  TEST_SPRINTF("42   ",
               "%0-5d", 42);
  TEST_SPRINTF("-42  ",
               "%0-5d", -42);
  TEST_SPRINTF("42             ",
               "%0-15d", 42);
  TEST_SPRINTF("-42            ",
               "%0-15d", -42);
#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  TEST_SPRINTF("-4.200e+01     ",
               "%0-15.3e", -42.);
#else
  TEST_SPRINTF("-4.200e+01     ",
               "e", -42.);
#endif

#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  TEST_SPRINTF("-42.0          ",
               "%0-15.3g", -42.);
#else
  TEST_SPRINTF("g",
               "%0-15.3g", -42.);
#endif
}


TEST_CASE("# flag", "[]" ) {
  TEST_DEF;

  TEST_SPRINTF("",
               "%#.0x", 0);
  TEST_SPRINTF("0",
               "%#.1x", 0);
  TEST_SPRINTF("",
               "%#.0llx", (long long)0);
  TEST_SPRINTF("0x0000614e",
               "%#.8x", 0x614e);
  TEST_SPRINTF("0b110",
               "%#b", 6);
}


TEST_CASE("specifier", "[]" ) {
  TEST_DEF;

  TEST_SPRINTF("Hello testing",
               "Hello testing");
  TEST_SPRINTF("Hello testing",
               "%s", "Hello testing");
  TEST_SPRINTF("1024",
               "%d", 1024);
  TEST_SPRINTF("-1024",
               "%d", -1024);
  TEST_SPRINTF("1024",
               "%i", 1024);
  TEST_SPRINTF("-1024",
               "%i", -1024);
  TEST_SPRINTF("1024",
               "%u", 1024);
  TEST_SPRINTF("4294966272",
               "%u", 4294966272U);
  TEST_SPRINTF("777",
               "%o", 511);
  TEST_SPRINTF("37777777001",
               "%o", 4294966785U);
  TEST_SPRINTF("1234abcd",
               "%x", 305441741);
  TEST_SPRINTF("edcb5433",
               "%x", 3989525555U);
  TEST_SPRINTF("1234ABCD",
               "%X", 305441741);
  TEST_SPRINTF("EDCB5433",
               "%X", 3989525555U);
  TEST_SPRINTF("%",
               "%%");
}


TEST_CASE("width", "[]" ) {
  TEST_DEF;

  TEST_SPRINTF("Hello testing",
               "%1s", "Hello testing");
  TEST_SPRINTF("1024",
               "%1d", 1024);
  TEST_SPRINTF("-1024",
               "%1d", -1024);
  TEST_SPRINTF("1024",
               "%1i", 1024);
  TEST_SPRINTF("-1024",
               "%1i", -1024);
  TEST_SPRINTF("1024",
               "%1u", 1024);
  TEST_SPRINTF("4294966272",
               "%1u", 4294966272U);
  TEST_SPRINTF("777",
               "%1o", 511);
  TEST_SPRINTF("37777777001",
               "%1o", 4294966785U);
  TEST_SPRINTF("1234abcd",
               "%1x", 305441741);
  TEST_SPRINTF("edcb5433",
               "%1x", 3989525555U);
  TEST_SPRINTF("1234ABCD",
               "%1X", 305441741);
  TEST_SPRINTF("EDCB5433",
               "%1X", 3989525555U);
  TEST_SPRINTF("x",
               "%1c", 'x');
}


TEST_CASE("width 20", "[]" ) {
  TEST_DEF;

  TEST_SPRINTF("               Hello",
               "%20s", "Hello");
  TEST_SPRINTF("                1024",
               "%20d", 1024);
  TEST_SPRINTF("               -1024",
               "%20d", -1024);
  TEST_SPRINTF("                1024",
               "%20i", 1024);
  TEST_SPRINTF("               -1024",
               "%20i", -1024);
  TEST_SPRINTF("                1024",
               "%20u", 1024);
  TEST_SPRINTF("          4294966272",
               "%20u", 4294966272U);
  TEST_SPRINTF("                 777",
               "%20o", 511);
  TEST_SPRINTF("         37777777001",
               "%20o", 4294966785U);
  TEST_SPRINTF("            1234abcd",
               "%20x", 305441741);
  TEST_SPRINTF("            edcb5433",
               "%20x", 3989525555U);
  TEST_SPRINTF("            1234ABCD",
               "%20X", 305441741);
  TEST_SPRINTF("            EDCB5433",
               "%20X", 3989525555U);
  TEST_SPRINTF("                   x",
               "%20c", 'x');
}


TEST_CASE("width *20", "[]" ) {
  TEST_DEF;

  TEST_SPRINTF("               Hello",
               "%*s", 20, "Hello");
  TEST_SPRINTF("                1024",
               "%*d", 20, 1024);
  TEST_SPRINTF("               -1024",
               "%*d", 20, -1024);
  TEST_SPRINTF("                1024",
               "%*i", 20, 1024);
  TEST_SPRINTF("               -1024",
               "%*i", 20, -1024);
  TEST_SPRINTF("                1024",
               "%*u", 20, 1024);
  TEST_SPRINTF("          4294966272",
               "%*u", 20, 4294966272U);
  TEST_SPRINTF("                 777",
               "%*o", 20, 511);
  TEST_SPRINTF("         37777777001",
               "%*o", 20, 4294966785U);
  TEST_SPRINTF("            1234abcd",
               "%*x", 20, 305441741);
  TEST_SPRINTF("            edcb5433",
               "%*x", 20, 3989525555U);
  TEST_SPRINTF("            1234ABCD",
               "%*X", 20, 305441741);
  TEST_SPRINTF("            EDCB5433",
               "%*X", 20, 3989525555U);
  TEST_SPRINTF("                   x",
               "%*c", 20,'x');
}


TEST_CASE("width -20", "[]" ) {
  TEST_DEF;

  TEST_SPRINTF("Hello               ",
               "%-20s", "Hello");
  TEST_SPRINTF("1024                ",
               "%-20d", 1024);
  TEST_SPRINTF("-1024               ",
               "%-20d", -1024);
  TEST_SPRINTF("1024                ",
               "%-20i", 1024);
  TEST_SPRINTF("-1024               ",
               "%-20i", -1024);
  TEST_SPRINTF("1024                ",
               "%-20u", 1024);
  TEST_SPRINTF("1024.1234           ",
               "%-20.4f", 1024.1234);
  TEST_SPRINTF("4294966272          ",
               "%-20u", 4294966272U);
  TEST_SPRINTF("777                 ",
               "%-20o", 511);
  TEST_SPRINTF("37777777001         ",
               "%-20o", 4294966785U);
  TEST_SPRINTF("1234abcd            ",
               "%-20x", 305441741);
  TEST_SPRINTF("edcb5433            ",
               "%-20x", 3989525555U);
  TEST_SPRINTF("1234ABCD            ",
               "%-20X", 305441741);
  TEST_SPRINTF("EDCB5433            ",
               "%-20X", 3989525555U);
  TEST_SPRINTF("x                   ",
               "%-20c", 'x');
  TEST_SPRINTF("|    9| |9 | |    9|",
               "|%5d| |%-2d| |%5d|", 9, 9, 9);
  TEST_SPRINTF("|   10| |10| |   10|",
               "|%5d| |%-2d| |%5d|", 10, 10, 10);
  TEST_SPRINTF("|    9| |9           | |    9|",
               "|%5d| |%-12d| |%5d|", 9, 9, 9);
  TEST_SPRINTF("|   10| |10          | |   10|",
               "|%5d| |%-12d| |%5d|", 10, 10, 10);
}


TEST_CASE("width 0-20", "[]" ) {
  TEST_DEF;

  TEST_SPRINTF("Hello               ",
               "%0-20s", "Hello");
  TEST_SPRINTF("1024                ",
               "%0-20d", 1024);
  TEST_SPRINTF("-1024               ",
               "%0-20d", -1024);
  TEST_SPRINTF("1024                ",
               "%0-20i", 1024);
  TEST_SPRINTF("-1024               ",
               "%0-20i", -1024);
  TEST_SPRINTF("1024                ",
               "%0-20u", 1024);
  TEST_SPRINTF("4294966272          ",
               "%0-20u", 4294966272U);
  TEST_SPRINTF("777                 ",
               "%0-20o", 511);
  TEST_SPRINTF("37777777001         ",
               "%0-20o", 4294966785U);
  TEST_SPRINTF("1234abcd            ",
               "%0-20x", 305441741);
  TEST_SPRINTF("edcb5433            ",
               "%0-20x", 3989525555U);
  TEST_SPRINTF("1234ABCD            ",
               "%0-20X", 305441741);
  TEST_SPRINTF("EDCB5433            ",
               "%0-20X", 3989525555U);
  TEST_SPRINTF("x                   ",
               "%0-20c", 'x');
}


TEST_CASE("padding 20", "[]" ) {
  TEST_DEF;

  TEST_SPRINTF("00000000000000001024",
               "%020d", 1024);
  TEST_SPRINTF("-0000000000000001024",
               "%020d", -1024);
  TEST_SPRINTF("00000000000000001024",
               "%020i", 1024);
  TEST_SPRINTF("-0000000000000001024",
               "%020i", -1024);
  TEST_SPRINTF("00000000000000001024",
               "%020u", 1024);
  TEST_SPRINTF("00000000004294966272",
               "%020u", 4294966272U);
  TEST_SPRINTF("00000000000000000777",
               "%020o", 511);
  TEST_SPRINTF("00000000037777777001",
               "%020o", 4294966785U);
  TEST_SPRINTF("0000000000001234abcd",
               "%020x", 305441741);
  TEST_SPRINTF("000000000000edcb5433",
               "%020x", 3989525555U);
  TEST_SPRINTF("0000000000001234ABCD",
               "%020X", 305441741);
  TEST_SPRINTF("000000000000EDCB5433",
               "%020X", 3989525555U);
}


TEST_CASE("padding .20", "[]" ) {
  TEST_DEF;

  TEST_SPRINTF("00000000000000001024",
               "%.20d", 1024);
  TEST_SPRINTF("-00000000000000001024",
               "%.20d", -1024);
  TEST_SPRINTF("00000000000000001024",
               "%.20i", 1024);
  TEST_SPRINTF("-00000000000000001024",
               "%.20i", -1024);
  TEST_SPRINTF("00000000000000001024",
               "%.20u", 1024);
  TEST_SPRINTF("00000000004294966272",
               "%.20u", 4294966272U);
  TEST_SPRINTF("00000000000000000777",
               "%.20o", 511);
  TEST_SPRINTF("00000000037777777001",
               "%.20o", 4294966785U);
  TEST_SPRINTF("0000000000001234abcd",
               "%.20x", 305441741);
  TEST_SPRINTF("000000000000edcb5433",
               "%.20x", 3989525555U);
  TEST_SPRINTF("0000000000001234ABCD",
               "%.20X", 305441741);
  TEST_SPRINTF("000000000000EDCB5433",
               "%.20X", 3989525555U);
}


TEST_CASE("padding #020", "[]" ) {
  TEST_DEF;

  TEST_SPRINTF("00000000000000001024",
               "%#020d", 1024);
  TEST_SPRINTF("-0000000000000001024",
               "%#020d", -1024);
  TEST_SPRINTF("00000000000000001024",
               "%#020i", 1024);
  TEST_SPRINTF("-0000000000000001024",
               "%#020i", -1024);
  TEST_SPRINTF("00000000000000001024",
               "%#020u", 1024);
  TEST_SPRINTF("00000000004294966272",
               "%#020u", 4294966272U);
  TEST_SPRINTF("00000000000000000777",
               "%#020o", 511);
  TEST_SPRINTF("00000000037777777001",
               "%#020o", 4294966785U);
  TEST_SPRINTF("0x00000000001234abcd",
               "%#020x", 305441741);
  TEST_SPRINTF("0x0000000000edcb5433",
               "%#020x", 3989525555U);
  TEST_SPRINTF("0X00000000001234ABCD",
               "%#020X", 305441741);
  TEST_SPRINTF("0X0000000000EDCB5433",
               "%#020X", 3989525555U);
}


TEST_CASE("padding #20", "[]" ) {
  TEST_DEF;

  TEST_SPRINTF("                1024",
               "%#20d", 1024);
  TEST_SPRINTF("               -1024",
               "%#20d", -1024);
  TEST_SPRINTF("                1024",
               "%#20i", 1024);
  TEST_SPRINTF("               -1024",
               "%#20i", -1024);
  TEST_SPRINTF("                1024",
               "%#20u", 1024);
  TEST_SPRINTF("          4294966272",
               "%#20u", 4294966272U);
  TEST_SPRINTF("                0777",
               "%#20o", 511);
  TEST_SPRINTF("        037777777001",
               "%#20o", 4294966785U);
  TEST_SPRINTF("          0x1234abcd",
               "%#20x", 305441741);
  TEST_SPRINTF("          0xedcb5433",
               "%#20x", 3989525555U);
  TEST_SPRINTF("          0X1234ABCD",
               "%#20X", 305441741);
  TEST_SPRINTF("          0XEDCB5433",
               "%#20X", 3989525555U);
}


TEST_CASE("padding 20.5", "[]" ) {
  TEST_DEF;

  TEST_SPRINTF("               01024",
               "%20.5d", 1024);
  TEST_SPRINTF("              -01024",
               "%20.5d", -1024);
  TEST_SPRINTF("               01024",
               "%20.5i", 1024);
  TEST_SPRINTF("              -01024",
               "%20.5i", -1024);
  TEST_SPRINTF("               01024",
               "%20.5u", 1024);
  TEST_SPRINTF("          4294966272",
               "%20.5u", 4294966272U);
  TEST_SPRINTF("               00777",
               "%20.5o", 511);
  TEST_SPRINTF("         37777777001",
               "%20.5o", 4294966785U);
  TEST_SPRINTF("            1234abcd",
               "%20.5x", 305441741);
  TEST_SPRINTF("          00edcb5433",
               "%20.10x", 3989525555U);
  TEST_SPRINTF("            1234ABCD",
               "%20.5X", 305441741);
  TEST_SPRINTF("          00EDCB5433",
               "%20.10X", 3989525555U);
}


TEST_CASE("padding neg numbers", "[]" ) {
  TEST_DEF;

  // space padding
  TEST_SPRINTF("-5",
               "% 1d", -5);
  TEST_SPRINTF("-5",
               "% 2d", -5);
  TEST_SPRINTF(" -5",
               "% 3d", -5);
  TEST_SPRINTF("  -5",
               "% 4d", -5);
  // zero padding
  TEST_SPRINTF("-5",
               "%01d", -5);
  TEST_SPRINTF("-5",
               "%02d", -5);
  TEST_SPRINTF("-05",
               "%03d", -5);
  TEST_SPRINTF("-005",
               "%04d", -5);
}


TEST_CASE("float padding neg numbers", "[]" ) {
  TEST_DEF;

  // space padding
  TEST_SPRINTF("-5.0",
               "% 3.1f", -5.);
  TEST_SPRINTF("-5.0",
               "% 4.1f", -5.);
  TEST_SPRINTF(" -5.0",
               "% 5.1f", -5.);
#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  TEST_SPRINTF("    -5",
               "% 6.1g", -5.);
  TEST_SPRINTF("-5.0e+00",
               "% 6.1e", -5.);
  TEST_SPRINTF("  -5.0e+00",
               "% 10.1e", -5.);
#endif

  // zero padding
  TEST_SPRINTF("-5.0",
               "%03.1f", -5.);
  TEST_SPRINTF("-5.0",
               "%04.1f", -5.);
  TEST_SPRINTF("-05.0",
               "%05.1f", -5.);
  // zero padding no decimal point
  TEST_SPRINTF("-5",
               "%01.0f", -5.);
  TEST_SPRINTF("-5",
               "%02.0f", -5.);
  TEST_SPRINTF("-05",
               "%03.0f", -5.);
#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  TEST_SPRINTF("-005.0e+00",
               "%010.1e", -5.);
  TEST_SPRINTF("-05E+00",
               "%07.0E", -5.);
  TEST_SPRINTF("-05",
               "%03.0g", -5.);
#endif
}

TEST_CASE("length", "[]" ) {
  TEST_DEF;

  TEST_SPRINTF("",
               "%.0s", "Hello testing");
  TEST_SPRINTF("                    ",
               "%20.0s", "Hello testing");
  TEST_SPRINTF("",
               "%.s", "Hello testing");
  TEST_SPRINTF("                    ",
               "%20.s", "Hello testing");
  TEST_SPRINTF("                1024",
               "%20.0d", 1024);
  TEST_SPRINTF("               -1024",
               "%20.0d", -1024);
  TEST_SPRINTF("                    ",
               "%20.d", 0);
  TEST_SPRINTF("                1024",
               "%20.0i", 1024);
  TEST_SPRINTF("               -1024",
               "%20.i", -1024);
  TEST_SPRINTF("                    ",
               "%20.i", 0);
  TEST_SPRINTF("                1024",
               "%20.u", 1024);
  TEST_SPRINTF("          4294966272",
               "%20.0u", 4294966272U);
  TEST_SPRINTF("                    ",
               "%20.u", 0U);
  TEST_SPRINTF("                 777",
               "%20.o", 511);
  TEST_SPRINTF("         37777777001",
               "%20.0o", 4294966785U);
  TEST_SPRINTF("                    ",
               "%20.o", 0U);
  TEST_SPRINTF("            1234abcd",
               "%20.x", 305441741);
  TEST_SPRINTF("                                          1234abcd",
               "%50.x", 305441741);
  TEST_SPRINTF("                                          1234abcd     12345",
               "%50.x%10.u", 305441741, 12345);
  TEST_SPRINTF("            edcb5433",
               "%20.0x", 3989525555U);
  TEST_SPRINTF("                    ",
               "%20.x", 0U);
  TEST_SPRINTF("            1234ABCD",
               "%20.X", 305441741);
  TEST_SPRINTF("            EDCB5433",
               "%20.0X", 3989525555U);
  TEST_SPRINTF("                    ",
               "%20.X", 0U);
  TEST_SPRINTF("  ",
               "%02.0u", 0U);
  TEST_SPRINTF("  ",
               "%02.0d", 0);
}


TEST_CASE("float", "[]" ) {
  TEST_DEF;

  // test special-case floats using math.h macros
  TEST_SPRINTF("     nan",
               "%8f", NAN);
  TEST_SPRINTF("     inf",
               "%8f", (double)INFINITY);
  TEST_SPRINTF("-inf    ",
               "%-8f", (double)-INFINITY);
#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  TEST_SPRINTF("    +inf",
               "%+8e", (double)INFINITY);
#endif

  TEST_SPRINTF("3.1415",
               "%.4f", 3.1415354);
  TEST_SPRINTF("30343.142",
               "%.3f", 30343.1415354);
  TEST_SPRINTF("34",
               "%.0f", 34.1415354);
  TEST_SPRINTF("1",
               "%.0f", 1.3);
  TEST_SPRINTF("2",
               "%.0f", 1.55);
  TEST_SPRINTF("1.6",
               "%.1f", 1.64);
  TEST_SPRINTF("42.90",
               "%.2f", 42.8952);
  TEST_SPRINTF("42.895200000",
               "%.9f", 42.8952);
  TEST_SPRINTF("42.8952230000",
               "%.10f", 42.895223);
  // this testcase checks, that the precision is truncated to 9 digits.
  // a perfect working float should return the whole number
  TEST_SPRINTF("42.895223123000",
               "%.12f", 42.89522312345678);
  // this testcase checks, that the precision is truncated AND rounded to 9 digits.
  // a perfect working float should return the whole number
  TEST_SPRINTF("42.895223877000",
               "%.12f", 42.89522387654321);
  TEST_SPRINTF(" 42.90",
               "%6.2f", 42.8952);
  TEST_SPRINTF("+42.90",
               "%+6.2f", 42.8952);
  TEST_SPRINTF("+42.9",
               "%+5.1f", 42.9252);
  TEST_SPRINTF("42.500000",
               "%f", 42.5);
  TEST_SPRINTF("42.5",
               "%.1f", 42.5);
  TEST_SPRINTF("42167.000000",
               "%f", 42167.0);
  TEST_SPRINTF("-12345.987654321",
               "%.9f", -12345.987654321);
  TEST_SPRINTF("4.0",
               "%.1f", 3.999);
  TEST_SPRINTF("4",
               "%.0f", 3.5);
  TEST_SPRINTF("4",
               "%.0f", 4.5);
  TEST_SPRINTF("3",
               "%.0f", 3.49);
  TEST_SPRINTF("3.5",
               "%.1f", 3.49);
  TEST_SPRINTF("a0.5  ",
               "a%-5.1f", 0.5);
  TEST_SPRINTF("a0.5  end",
               "a%-5.1fend", 0.5);
#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  TEST_SPRINTF("12345.7",
               "%G", 12345.678);
  TEST_SPRINTF("12345.68",
               "%.7G", 12345.678);
  TEST_SPRINTF("1.2346E+08",
               "%.5G", 123456789.);
  TEST_SPRINTF("12345.0",
               "%.6G", 12345.);
  TEST_SPRINTF("  +1.235e+08",
               "%+12.4g", 123456789.);
  TEST_SPRINTF("0.0012",
               "%.2G", 0.001234);
  TEST_SPRINTF(" +0.001234",
               "%+10.4G", 0.001234);
  TEST_SPRINTF("+001.234e-05",
               "%+012.4g", 0.00001234);
  TEST_SPRINTF("-1.23e-308",
               "%.3g", -1.2345e-308);
  TEST_SPRINTF("+1.230E+308",
               "%+.3E", 1.23e+308);
#endif

  // out of range for float: should switch to exp notation if supported, else empty
#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  TEST_SPRINTF("1.0e+20",
               "%.1f", 1E20);
#else
  TEST_SPRINTF("",
               "%.1f", 1E20);
#endif

  // brute force float
  bool fail = false;
  std::stringstream str;
  str.precision(5);
  for (float i = -100000; i < 100000; i += 1) {
    tested_sprintf(buffer, "%.5f", (double)i / 10000);
    str.str("");
    str << std::fixed << i / 10000;
    fail = fail || !!strcmp(buffer, str.str().c_str());
  }
  REQUIRE(!fail);


#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  // brute force exp
  str.setf(std::ios::scientific, std::ios::floatfield);
  for (float i = -1e20; i < 1e20f; i += 1e15f) {
    tested_sprintf(buffer, "%.5f", (double)i);
    str.str("");
    str << i;
    fail = fail || !!strcmp(buffer, str.str().c_str());
  }
  REQUIRE(!fail);
#endif
}


TEST_CASE("types", "[]" ) {
  TEST_DEF;

  TEST_SPRINTF("0",
               "%i", 0);
  TEST_SPRINTF("1234",
               "%i", 1234);
  TEST_SPRINTF("32767",
               "%i", 32767);
  TEST_SPRINTF("-32767",
               "%i", -32767);
  TEST_SPRINTF("30",
               "%li", 30L);
  TEST_SPRINTF("-2147483647",
               "%li", -2147483647L);
  TEST_SPRINTF("2147483647",
               "%li", 2147483647L);
  TEST_SPRINTF("30",
               "%lli", 30LL);
  TEST_SPRINTF("-9223372036854775807",
               "%lli", -9223372036854775807LL);
  TEST_SPRINTF("9223372036854775807",
               "%lli", 9223372036854775807LL);
  TEST_SPRINTF("100000",
               "%lu", 100000L);
  TEST_SPRINTF("4294967295",
               "%lu", 0xFFFFFFFFL);
  TEST_SPRINTF("281474976710656",
               "%llu", 281474976710656LLU);
  TEST_SPRINTF("18446744073709551615",
               "%llu", 18446744073709551615LLU);
  TEST_SPRINTF("2147483647",
               "%zu", 2147483647UL);
  TEST_SPRINTF("2147483647",
               "%zd", 2147483647UL);
  if (sizeof(size_t) == sizeof(long)) {
    TEST_SPRINTF("-2147483647",
                 "%zi", -2147483647L);
  }
  else {
    TEST_SPRINTF("-2147483647",
                 "%zi", -2147483647LL);
  }

  TEST_SPRINTF("1110101001100000",
               "%b", 60000);
  TEST_SPRINTF("101111000110000101001110",
               "%lb", 12345678L);
  TEST_SPRINTF("165140",
               "%o", 60000);
  TEST_SPRINTF("57060516",
               "%lo", 12345678L);
  TEST_SPRINTF("12345678",
               "%lx", 0x12345678L);
  TEST_SPRINTF("1234567891234567",
               "%llx", 0x1234567891234567LLU);
  TEST_SPRINTF("abcdefab",
               "%lx", 0xabcdefabL);
  TEST_SPRINTF("ABCDEFAB",
               "%lX", 0xabcdefabL);
  TEST_SPRINTF("v",
               "%c", 'v');
  TEST_SPRINTF("wv",
               "%cv", 'w');
  TEST_SPRINTF("A Test",
               "%s", "A Test");
  TEST_SPRINTF("255",
               "%hhu", 0xFFFFUL);
  TEST_SPRINTF("13398",
               "%hu", 0x123456UL);
  TEST_SPRINTF("Test16 65535",
               "%s%hhi %hu", "Test", 10000, 0xFFFFFFFF);
  TEST_SPRINTF("a",
               "%tx", &buffer[10] - &buffer[0]);
// TBD
  if (sizeof(intmax_t) == sizeof(long)) {
    TEST_SPRINTF("-2147483647",
                 "%ji", -2147483647L);
  }
  else {
    TEST_SPRINTF("-2147483647",
                 "%ji", -2147483647LL);
  }
}


TEST_CASE("pointer", "[]" ) {
  TEST_DEF;

  if (sizeof(void*) == 4U) {
    TEST_SPRINTF("00001234",
                 "%p", (void*)0x1234U);
  }
  else {
    TEST_SPRINTF("0000000000001234",
                 "%p", (void*)0x1234U);
  }

  if (sizeof(void*) == 4U) {
    TEST_SPRINTF("12345678",
                 "%p", (void*)0x12345678U);
  }
  else {
    TEST_SPRINTF("0000000012345678",
                 "%p", (void*)0x12345678U);
  }

  if (sizeof(void*) == 4U) {
    TEST_SPRINTF("12345678-7EDCBA98",
                 "%p-%p", (void*)0x12345678U, (void*)0x7EDCBA98U);
  }
  else {
    TEST_SPRINTF("0000000012345678-000000007EDCBA98",
                 "%p-%p", (void*)0x12345678U, (void*)0x7EDCBA98U);
  }
  if (sizeof(uintptr_t) == sizeof(uint64_t)) {
    TEST_SPRINTF("00000000FFFFFFFF",
                 "%p", (void*)(uintptr_t)0xFFFFFFFFU);
  }
  else {
    TEST_SPRINTF("FFFFFFFF",
                 "%p", (void*)(uintptr_t)0xFFFFFFFFU);
  }
}


TEST_CASE("unknown flag", "[]" ) {
  TEST_DEF;

  TEST_SPRINTF("kmarco",
               "%kmarco", 42, 37);
}


TEST_CASE("string length", "[]" ) {
  TEST_DEF;

  TEST_SPRINTF("This",
               "%.4s", "This is a test");
  TEST_SPRINTF("test",
               "%.4s", "test");
  TEST_SPRINTF("123",
               "%.7s", "123");
  TEST_SPRINTF("",
               "%.7s", "");
  TEST_SPRINTF("1234ab",
               "%.4s%.2s", "123456", "abcdef");
  TEST_SPRINTF(".2s",
               "%.4.2s", "123456");
  TEST_SPRINTF("123",
               "%.*s", 3, "123456");
}


TEST_CASE("buffer length", "[]" ) {
  TEST_DEF;

  ret = tested_snprintf(nullptr, 10, "%s", "Test");
  REQUIRE(ret == 4);
  ret = tested_snprintf(nullptr, 0, "%s", "Test");
  REQUIRE(ret == 4);

  buffer[0] = (char)0xA5;
  ret = tested_snprintf(buffer, 0, "%s", "Test");
  REQUIRE(buffer[0] == (char)0xA5);
  REQUIRE(ret == 4);

  buffer[0] = (char)0xCC;
  tested_snprintf(buffer, 1, "%s", "Test");
  REQUIRE(buffer[0] == '\0');

  TEST_SNPRINTF("H", 5,
                2, "%s", "Hello");
}


TEST_CASE("ret value", "[]" ) {
  TEST_DEF;

  TEST_SNPRINTF("01234", 5,
                6, "0%s", "1234");

  TEST_SNPRINTF("01234", 6, // '5' is truncated
                6, "0%s", "12345");

  TEST_SNPRINTF("01234", 8, // '567' are truncated
                6, "0%s", "1234567");

  TEST_SNPRINTF(NULL, 12,
                10, "hello, world");

  TEST_SNPRINTF("10", 5,
                3, "%d", 10000);
}


TEST_CASE("misc", "[]" ) {
  TEST_DEF;

  TEST_SPRINTF("53000atest-20 bit",
               "%u%u%ctest%d %s", 5, 3000, 'a', -20, "bit");
  TEST_SPRINTF("0.33",
               "%.*f", 2, 0.33333333);
  TEST_SPRINTF("1",
               "%.*d", -1, 1);
  TEST_SPRINTF("foo",
               "%.3s", "foobar");
  TEST_SPRINTF(" ",
               "% .0d", 0);
  TEST_SPRINTF("     00004",
               "%10.5d", 4);
  TEST_SPRINTF("hi x",
               "%*sx", -3, "hi");
#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  TEST_SPRINTF("0.33",
               "%.*g", 2, 0.33333333);
  TEST_SPRINTF("3.33e-01",
               "%.*e", 2, 0.33333333);
#endif
}
