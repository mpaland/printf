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
#include <climits>

// PRIi32 etc. macros
// printf should provide its own version
#include <inttypes.h>

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


// some assumptions about implementation
#define PRINTF_FLOAT_PREC 9
#define PRINTF_FLOAT_DECIMALS 9
#define PRINTF_POINTER_0XHEX 0

// enable failing but valid tests
#define ENABLE_FAILING 0

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
  (void)buffer; (void)ret;                              \
  struct dummy                                          \
  /**/

// use prefix T_ so that lininsanity macros do not collide
#define T_SPRINTF(expected, ...)                     \
  do {                                                  \
    CAPTURE(__VA_ARGS__);                               \
    ret = tested_sprintf(buffer, __VA_ARGS__);          \
    CHECK_THAT(buffer, Equals(expected));               \
    (void)ret;                                          \
  } while (0)                                           \
    /**/

#define T_SNPRINTF(expected, retval, ...)            \
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

  T_SPRINTF(" 42",
            "% d", 42);

  T_SPRINTF("-42",
            "% d", -42);
  T_SPRINTF("   42",
            "% 5d", 42);
  T_SPRINTF("  -42",
            "% 5d", -42);
  T_SPRINTF("             42",
            "% 15d", 42);
  T_SPRINTF("            -42",
            "% 15d", -42);
  T_SPRINTF("            -42",
            "% 15d", -42);
  T_SPRINTF("        -42.987",
            "% 15.3f", -42.987);
  T_SPRINTF("         42.987",
            "% 15.3f", 42.987);
  T_SPRINTF("Hello testing",
            "% s", "Hello testing");
  T_SPRINTF(" 1024",
            "% d", 1024);
  T_SPRINTF("-1024",
            "% d", -1024);
  T_SPRINTF(" 1024",
            "% i", 1024);
  T_SPRINTF("-1024",
            "% i", -1024);
  T_SPRINTF("1024",
            "% u", 1024);
  T_SPRINTF("4294966272",
            "% u", 4294966272U);
  T_SPRINTF("777",
            "% o", 511);
  T_SPRINTF("37777777001",
            "% o", 4294966785U);
  T_SPRINTF("1234abcd",
            "% x", 305441741);
  T_SPRINTF("edcb5433",
            "% x", 3989525555U);
  T_SPRINTF("1234ABCD",
            "% X", 305441741);
  T_SPRINTF("EDCB5433",
            "% X", 3989525555U);
  T_SPRINTF("x",
            "% c", 'x');
}


TEST_CASE("+ flag", "[]" ) {
  TEST_DEF;

  T_SPRINTF("+42",
            "%+d", 42);
  T_SPRINTF("-42",
            "%+d", -42);
  T_SPRINTF("  +42",
            "%+5d", 42);
  T_SPRINTF("  -42",
            "%+5d", -42);
  T_SPRINTF("            +42",
            "%+15d", 42);
  T_SPRINTF("            -42",
            "%+15d", -42);
  T_SPRINTF("Hello testing",
            "%+s", "Hello testing");
  T_SPRINTF("+1024",
            "%+d", 1024);
  T_SPRINTF("-1024",
            "%+d", -1024);
  T_SPRINTF("+1024",
            "%+i", 1024);
  T_SPRINTF("-1024",
            "%+i", -1024);
  T_SPRINTF("1024",
            "%+u", 1024);
  T_SPRINTF("4294966272",
            "%+u", 4294966272U);
  T_SPRINTF("777",
            "%+o", 511);
  T_SPRINTF("37777777001",
            "%+o", 4294966785U);
  T_SPRINTF("1234abcd",
            "%+x", 305441741);
  T_SPRINTF("edcb5433",
            "%+x", 3989525555U);
  T_SPRINTF("1234ABCD",
            "%+X", 305441741);
  T_SPRINTF("EDCB5433",
            "%+X", 3989525555U);
  T_SPRINTF("x",
            "%+c", 'x');
  T_SPRINTF("+",
            "%+.0d", 0);
}


TEST_CASE("0 flag", "[]" ) {
  TEST_DEF;

  T_SPRINTF("42",
            "%0d", 42);
  T_SPRINTF("42",
            "%0ld", 42L);
  T_SPRINTF("-42",
            "%0d", -42);
  T_SPRINTF("00042",
            "%05d", 42);
  T_SPRINTF("-0042",
            "%05d", -42);
  T_SPRINTF("000000000000042",
            "%015d", 42);
  T_SPRINTF("-00000000000042",
            "%015d", -42);
  T_SPRINTF("000000000042.12",
            "%015.2f", 42.1234);
  T_SPRINTF("00000000042.988",
            "%015.3f", 42.9876);
  T_SPRINTF("-00000042.98760",
            "%015.5f", -42.9876);}


TEST_CASE("- flag", "[]" ) {
  TEST_DEF;

  T_SPRINTF("42",
            "%-d", 42);
  T_SPRINTF("-42",
            "%-d", -42);
  T_SPRINTF("42   ",
            "%-5d", 42);
  T_SPRINTF("-42  ",
            "%-5d", -42);
  T_SPRINTF("42             ",
            "%-15d", 42);
  T_SPRINTF("-42            ",
            "%-15d", -42);
  T_SPRINTF("42",
            "%-0d", 42);
  T_SPRINTF("-42",
            "%-0d", -42);
  T_SPRINTF("42   ",
            "%-05d", 42);
  T_SPRINTF("-42  ",
            "%-05d", -42);
  T_SPRINTF("42             ",
            "%-015d", 42);
  T_SPRINTF("-42            ",
            "%-015d", -42);
  T_SPRINTF("42",
            "%0-d", 42);
  T_SPRINTF("-42",
            "%0-d", -42);
  T_SPRINTF("42   ",
            "%0-5d", 42);
  T_SPRINTF("-42  ",
            "%0-5d", -42);
  T_SPRINTF("42             ",
            "%0-15d", 42);
  T_SPRINTF("-42            ",
            "%0-15d", -42);
#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  T_SPRINTF("-4.200e+01     ",
            "%0-15.3e", -42.);
#else
  T_SPRINTF("-4.200e+01     ",
            "e", -42.);
#endif

#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  T_SPRINTF("-42.0          ",
            "%0-15.3g", -42.);
#else
  T_SPRINTF("g",
            "%0-15.3g", -42.);
#endif
}


TEST_CASE("# flag", "[]" ) {
  TEST_DEF;

  T_SPRINTF("",
            "%#.0x", 0);
  T_SPRINTF("0",
            "%#.1x", 0);
  T_SPRINTF("",
            "%#.0llx", (long long)0);
  T_SPRINTF("0x0000614e",
            "%#.8x", 0x614e);
  T_SPRINTF("0b110",
            "%#b", 6);
}


TEST_CASE("specifier", "[]" ) {
  TEST_DEF;

  T_SPRINTF("Hello testing",
            "Hello testing");
  T_SPRINTF("Hello testing",
            "%s", "Hello testing");
  T_SPRINTF("1024",
            "%d", 1024);
  T_SPRINTF("-1024",
            "%d", -1024);
  T_SPRINTF("1024",
            "%i", 1024);
  T_SPRINTF("-1024",
            "%i", -1024);
  T_SPRINTF("1024",
            "%u", 1024);
  T_SPRINTF("4294966272",
            "%u", 4294966272U);
  T_SPRINTF("777",
            "%o", 511);
  T_SPRINTF("37777777001",
            "%o", 4294966785U);
  T_SPRINTF("1234abcd",
            "%x", 305441741);
  T_SPRINTF("edcb5433",
            "%x", 3989525555U);
  T_SPRINTF("1234ABCD",
            "%X", 305441741);
  T_SPRINTF("EDCB5433",
            "%X", 3989525555U);
  T_SPRINTF("%",
            "%%");
}


TEST_CASE("width", "[]" ) {
  TEST_DEF;

  T_SPRINTF("Hello testing",
            "%1s", "Hello testing");
  T_SPRINTF("1024",
            "%1d", 1024);
  T_SPRINTF("-1024",
            "%1d", -1024);
  T_SPRINTF("1024",
            "%1i", 1024);
  T_SPRINTF("-1024",
            "%1i", -1024);
  T_SPRINTF("1024",
            "%1u", 1024);
  T_SPRINTF("4294966272",
            "%1u", 4294966272U);
  T_SPRINTF("777",
            "%1o", 511);
  T_SPRINTF("37777777001",
            "%1o", 4294966785U);
  T_SPRINTF("1234abcd",
            "%1x", 305441741);
  T_SPRINTF("edcb5433",
            "%1x", 3989525555U);
  T_SPRINTF("1234ABCD",
            "%1X", 305441741);
  T_SPRINTF("EDCB5433",
            "%1X", 3989525555U);
  T_SPRINTF("x",
            "%1c", 'x');
}


TEST_CASE("width 20", "[]" ) {
  TEST_DEF;

  T_SPRINTF("               Hello",
            "%20s", "Hello");
  T_SPRINTF("                1024",
            "%20d", 1024);
  T_SPRINTF("               -1024",
            "%20d", -1024);
  T_SPRINTF("                1024",
            "%20i", 1024);
  T_SPRINTF("               -1024",
            "%20i", -1024);
  T_SPRINTF("                1024",
            "%20u", 1024);
  T_SPRINTF("          4294966272",
            "%20u", 4294966272U);
  T_SPRINTF("                 777",
            "%20o", 511);
  T_SPRINTF("         37777777001",
            "%20o", 4294966785U);
  T_SPRINTF("            1234abcd",
            "%20x", 305441741);
  T_SPRINTF("            edcb5433",
            "%20x", 3989525555U);
  T_SPRINTF("            1234ABCD",
            "%20X", 305441741);
  T_SPRINTF("            EDCB5433",
            "%20X", 3989525555U);
  T_SPRINTF("                   x",
            "%20c", 'x');
}


TEST_CASE("width *20", "[]" ) {
  TEST_DEF;

  T_SPRINTF("               Hello",
            "%*s", 20, "Hello");
  T_SPRINTF("                1024",
            "%*d", 20, 1024);
  T_SPRINTF("               -1024",
            "%*d", 20, -1024);
  T_SPRINTF("                1024",
            "%*i", 20, 1024);
  T_SPRINTF("               -1024",
            "%*i", 20, -1024);
  T_SPRINTF("                1024",
            "%*u", 20, 1024);
  T_SPRINTF("          4294966272",
            "%*u", 20, 4294966272U);
  T_SPRINTF("                 777",
            "%*o", 20, 511);
  T_SPRINTF("         37777777001",
            "%*o", 20, 4294966785U);
  T_SPRINTF("            1234abcd",
            "%*x", 20, 305441741);
  T_SPRINTF("            edcb5433",
            "%*x", 20, 3989525555U);
  T_SPRINTF("            1234ABCD",
            "%*X", 20, 305441741);
  T_SPRINTF("            EDCB5433",
            "%*X", 20, 3989525555U);
  T_SPRINTF("                   x",
            "%*c", 20,'x');
}


TEST_CASE("width -20", "[]" ) {
  TEST_DEF;

  T_SPRINTF("Hello               ",
            "%-20s", "Hello");
  T_SPRINTF("1024                ",
            "%-20d", 1024);
  T_SPRINTF("-1024               ",
            "%-20d", -1024);
  T_SPRINTF("1024                ",
            "%-20i", 1024);
  T_SPRINTF("-1024               ",
            "%-20i", -1024);
  T_SPRINTF("1024                ",
            "%-20u", 1024);
  T_SPRINTF("1024.1234           ",
            "%-20.4f", 1024.1234);
  T_SPRINTF("4294966272          ",
            "%-20u", 4294966272U);
  T_SPRINTF("777                 ",
            "%-20o", 511);
  T_SPRINTF("37777777001         ",
            "%-20o", 4294966785U);
  T_SPRINTF("1234abcd            ",
            "%-20x", 305441741);
  T_SPRINTF("edcb5433            ",
            "%-20x", 3989525555U);
  T_SPRINTF("1234ABCD            ",
            "%-20X", 305441741);
  T_SPRINTF("EDCB5433            ",
            "%-20X", 3989525555U);
  T_SPRINTF("x                   ",
            "%-20c", 'x');
  T_SPRINTF("|    9| |9 | |    9|",
            "|%5d| |%-2d| |%5d|", 9, 9, 9);
  T_SPRINTF("|   10| |10| |   10|",
            "|%5d| |%-2d| |%5d|", 10, 10, 10);
  T_SPRINTF("|    9| |9           | |    9|",
            "|%5d| |%-12d| |%5d|", 9, 9, 9);
  T_SPRINTF("|   10| |10          | |   10|",
            "|%5d| |%-12d| |%5d|", 10, 10, 10);
}


TEST_CASE("width 0-20", "[]" ) {
  TEST_DEF;

  T_SPRINTF("Hello               ",
            "%0-20s", "Hello");
  T_SPRINTF("1024                ",
            "%0-20d", 1024);
  T_SPRINTF("-1024               ",
            "%0-20d", -1024);
  T_SPRINTF("1024                ",
            "%0-20i", 1024);
  T_SPRINTF("-1024               ",
            "%0-20i", -1024);
  T_SPRINTF("1024                ",
            "%0-20u", 1024);
  T_SPRINTF("4294966272          ",
            "%0-20u", 4294966272U);
  T_SPRINTF("777                 ",
            "%0-20o", 511);
  T_SPRINTF("37777777001         ",
            "%0-20o", 4294966785U);
  T_SPRINTF("1234abcd            ",
            "%0-20x", 305441741);
  T_SPRINTF("edcb5433            ",
            "%0-20x", 3989525555U);
  T_SPRINTF("1234ABCD            ",
            "%0-20X", 305441741);
  T_SPRINTF("EDCB5433            ",
            "%0-20X", 3989525555U);
  T_SPRINTF("x                   ",
            "%0-20c", 'x');
}


TEST_CASE("padding 20", "[]" ) {
  TEST_DEF;

  T_SPRINTF("00000000000000001024",
            "%020d", 1024);
  T_SPRINTF("-0000000000000001024",
            "%020d", -1024);
  T_SPRINTF("00000000000000001024",
            "%020i", 1024);
  T_SPRINTF("-0000000000000001024",
            "%020i", -1024);
  T_SPRINTF("00000000000000001024",
            "%020u", 1024);
  T_SPRINTF("00000000004294966272",
            "%020u", 4294966272U);
  T_SPRINTF("00000000000000000777",
            "%020o", 511);
  T_SPRINTF("00000000037777777001",
            "%020o", 4294966785U);
  T_SPRINTF("0000000000001234abcd",
            "%020x", 305441741);
  T_SPRINTF("000000000000edcb5433",
            "%020x", 3989525555U);
  T_SPRINTF("0000000000001234ABCD",
            "%020X", 305441741);
  T_SPRINTF("000000000000EDCB5433",
            "%020X", 3989525555U);
}


TEST_CASE("padding .20", "[]" ) {
  TEST_DEF;

  T_SPRINTF("00000000000000001024",
            "%.20d", 1024);
  T_SPRINTF("-00000000000000001024",
            "%.20d", -1024);
  T_SPRINTF("00000000000000001024",
            "%.20i", 1024);
  T_SPRINTF("-00000000000000001024",
            "%.20i", -1024);
  T_SPRINTF("00000000000000001024",
            "%.20u", 1024);
  T_SPRINTF("00000000004294966272",
            "%.20u", 4294966272U);
  T_SPRINTF("00000000000000000777",
            "%.20o", 511);
  T_SPRINTF("00000000037777777001",
            "%.20o", 4294966785U);
  T_SPRINTF("0000000000001234abcd",
            "%.20x", 305441741);
  T_SPRINTF("000000000000edcb5433",
            "%.20x", 3989525555U);
  T_SPRINTF("0000000000001234ABCD",
            "%.20X", 305441741);
  T_SPRINTF("000000000000EDCB5433",
            "%.20X", 3989525555U);
}


TEST_CASE("padding #020", "[]" ) {
  TEST_DEF;

  T_SPRINTF("00000000000000001024",
            "%#020d", 1024);
  T_SPRINTF("-0000000000000001024",
            "%#020d", -1024);
  T_SPRINTF("00000000000000001024",
            "%#020i", 1024);
  T_SPRINTF("-0000000000000001024",
            "%#020i", -1024);
  T_SPRINTF("00000000000000001024",
            "%#020u", 1024);
  T_SPRINTF("00000000004294966272",
            "%#020u", 4294966272U);
  T_SPRINTF("00000000000000000777",
            "%#020o", 511);
  T_SPRINTF("00000000037777777001",
            "%#020o", 4294966785U);
  T_SPRINTF("0x00000000001234abcd",
            "%#020x", 305441741);
  T_SPRINTF("0x0000000000edcb5433",
            "%#020x", 3989525555U);
  T_SPRINTF("0X00000000001234ABCD",
            "%#020X", 305441741);
  T_SPRINTF("0X0000000000EDCB5433",
            "%#020X", 3989525555U);
}


TEST_CASE("padding #20", "[]" ) {
  TEST_DEF;

  T_SPRINTF("                1024",
            "%#20d", 1024);
  T_SPRINTF("               -1024",
            "%#20d", -1024);
  T_SPRINTF("                1024",
            "%#20i", 1024);
  T_SPRINTF("               -1024",
            "%#20i", -1024);
  T_SPRINTF("                1024",
            "%#20u", 1024);
  T_SPRINTF("          4294966272",
            "%#20u", 4294966272U);
  T_SPRINTF("                0777",
            "%#20o", 511);
  T_SPRINTF("        037777777001",
            "%#20o", 4294966785U);
  T_SPRINTF("          0x1234abcd",
            "%#20x", 305441741);
  T_SPRINTF("          0xedcb5433",
            "%#20x", 3989525555U);
  T_SPRINTF("          0X1234ABCD",
            "%#20X", 305441741);
  T_SPRINTF("          0XEDCB5433",
            "%#20X", 3989525555U);
}


TEST_CASE("padding 20.5", "[]" ) {
  TEST_DEF;

  T_SPRINTF("               01024",
            "%20.5d", 1024);
  T_SPRINTF("              -01024",
            "%20.5d", -1024);
  T_SPRINTF("               01024",
            "%20.5i", 1024);
  T_SPRINTF("              -01024",
            "%20.5i", -1024);
  T_SPRINTF("               01024",
            "%20.5u", 1024);
  T_SPRINTF("          4294966272",
            "%20.5u", 4294966272U);
  T_SPRINTF("               00777",
            "%20.5o", 511);
  T_SPRINTF("         37777777001",
            "%20.5o", 4294966785U);
  T_SPRINTF("            1234abcd",
            "%20.5x", 305441741);
  T_SPRINTF("          00edcb5433",
            "%20.10x", 3989525555U);
  T_SPRINTF("            1234ABCD",
            "%20.5X", 305441741);
  T_SPRINTF("          00EDCB5433",
            "%20.10X", 3989525555U);
}


TEST_CASE("padding neg numbers", "[]" ) {
  TEST_DEF;

  // space padding
  T_SPRINTF("-5",
            "% 1d", -5);
  T_SPRINTF("-5",
            "% 2d", -5);
  T_SPRINTF(" -5",
            "% 3d", -5);
  T_SPRINTF("  -5",
            "% 4d", -5);
  // zero padding
  T_SPRINTF("-5",
            "%01d", -5);
  T_SPRINTF("-5",
            "%02d", -5);
  T_SPRINTF("-05",
            "%03d", -5);
  T_SPRINTF("-005",
            "%04d", -5);
}


TEST_CASE("float padding neg numbers", "[]" ) {
  TEST_DEF;

  // space padding
  T_SPRINTF("-5.0",
            "% 3.1f", -5.);
  T_SPRINTF("-5.0",
            "% 4.1f", -5.);
  T_SPRINTF(" -5.0",
            "% 5.1f", -5.);
#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  T_SPRINTF("    -5",
            "% 6.1g", -5.);
  T_SPRINTF("-5.0e+00",
            "% 6.1e", -5.);
  T_SPRINTF("  -5.0e+00",
            "% 10.1e", -5.);
#endif

  // zero padding
  T_SPRINTF("-5.0",
            "%03.1f", -5.);
  T_SPRINTF("-5.0",
            "%04.1f", -5.);
  T_SPRINTF("-05.0",
            "%05.1f", -5.);
  // zero padding no decimal point
  T_SPRINTF("-5",
            "%01.0f", -5.);
  T_SPRINTF("-5",
            "%02.0f", -5.);
  T_SPRINTF("-05",
            "%03.0f", -5.);
#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  T_SPRINTF("-005.0e+00",
            "%010.1e", -5.);
  T_SPRINTF("-05E+00",
            "%07.0E", -5.);
  T_SPRINTF("-05",
            "%03.0g", -5.);
#endif
}

TEST_CASE("length", "[]" ) {
  TEST_DEF;

  T_SPRINTF("",
            "%.0s", "Hello testing");
  T_SPRINTF("                    ",
            "%20.0s", "Hello testing");
  T_SPRINTF("",
            "%.s", "Hello testing");
  T_SPRINTF("                    ",
            "%20.s", "Hello testing");
  T_SPRINTF("                1024",
            "%20.0d", 1024);
  T_SPRINTF("               -1024",
            "%20.0d", -1024);
  T_SPRINTF("                    ",
            "%20.d", 0);
  T_SPRINTF("                1024",
            "%20.0i", 1024);
  T_SPRINTF("               -1024",
            "%20.i", -1024);
  T_SPRINTF("                    ",
            "%20.i", 0);
  T_SPRINTF("                1024",
            "%20.u", 1024);
  T_SPRINTF("          4294966272",
            "%20.0u", 4294966272U);
  T_SPRINTF("                    ",
            "%20.u", 0U);
  T_SPRINTF("                 777",
            "%20.o", 511);
  T_SPRINTF("         37777777001",
            "%20.0o", 4294966785U);
  T_SPRINTF("                    ",
            "%20.o", 0U);
  T_SPRINTF("            1234abcd",
            "%20.x", 305441741);
  T_SPRINTF("                                          1234abcd",
            "%50.x", 305441741);
  T_SPRINTF("                                          1234abcd     12345",
            "%50.x%10.u", 305441741, 12345);
  T_SPRINTF("            edcb5433",
            "%20.0x", 3989525555U);
  T_SPRINTF("                    ",
            "%20.x", 0U);
  T_SPRINTF("            1234ABCD",
            "%20.X", 305441741);
  T_SPRINTF("            EDCB5433",
            "%20.0X", 3989525555U);
  T_SPRINTF("                    ",
            "%20.X", 0U);
  T_SPRINTF("  ",
            "%02.0u", 0U);
  T_SPRINTF("  ",
            "%02.0d", 0);
}


TEST_CASE("float", "[]" ) {
  TEST_DEF;

  // test special-case floats using math.h macros
  T_SPRINTF("     nan",
            "%8f", NAN);
  T_SPRINTF("     inf",
            "%8f", (double)INFINITY);
  T_SPRINTF("-inf    ",
            "%-8f", (double)-INFINITY);
#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  T_SPRINTF("    +inf",
            "%+8e", (double)INFINITY);
#endif

  T_SPRINTF("3.1415",
            "%.4f", 3.1415354);
  T_SPRINTF("30343.142",
            "%.3f", 30343.1415354);
  T_SPRINTF("34",
            "%.0f", 34.1415354);
  T_SPRINTF("1",
            "%.0f", 1.3);
  T_SPRINTF("2",
            "%.0f", 1.55);
  T_SPRINTF("1.6",
            "%.1f", 1.64);
  T_SPRINTF("42.90",
            "%.2f", 42.8952);
  T_SPRINTF("42.895200000",
            "%.9f", 42.8952);
  T_SPRINTF("42.8952230000",
            "%.10f", 42.895223);
  // this testcase checks, that the precision is truncated to 9 digits.
  // a perfect working float should return the whole number
  T_SPRINTF("42.895223123000",
            "%.12f", 42.89522312345678);
  // this testcase checks, that the precision is truncated AND rounded to 9 digits.
  // a perfect working float should return the whole number
  T_SPRINTF("42.895223877000",
            "%.12f", 42.89522387654321);
  T_SPRINTF(" 42.90",
            "%6.2f", 42.8952);
  T_SPRINTF("+42.90",
            "%+6.2f", 42.8952);
  T_SPRINTF("+42.9",
            "%+5.1f", 42.9252);
  T_SPRINTF("42.500000",
            "%f", 42.5);
  T_SPRINTF("42.5",
            "%.1f", 42.5);
  T_SPRINTF("42167.000000",
            "%f", 42167.0);
  T_SPRINTF("-12345.987654321",
            "%.9f", -12345.987654321);
  T_SPRINTF("4.0",
            "%.1f", 3.999);
  T_SPRINTF("4",
            "%.0f", 3.5);
  T_SPRINTF("4",
            "%.0f", 4.5);
  T_SPRINTF("3",
            "%.0f", 3.49);
  T_SPRINTF("3.5",
            "%.1f", 3.49);
  T_SPRINTF("a0.5  ",
            "a%-5.1f", 0.5);
  T_SPRINTF("a0.5  end",
            "a%-5.1fend", 0.5);
#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  T_SPRINTF("12345.7",
            "%G", 12345.678);
  T_SPRINTF("12345.68",
            "%.7G", 12345.678);
  T_SPRINTF("1.2346E+08",
            "%.5G", 123456789.);
  T_SPRINTF("12345.0",
            "%.6G", 12345.);
  T_SPRINTF("  +1.235e+08",
            "%+12.4g", 123456789.);
  T_SPRINTF("0.0012",
            "%.2G", 0.001234);
  T_SPRINTF(" +0.001234",
            "%+10.4G", 0.001234);
  T_SPRINTF("+001.234e-05",
            "%+012.4g", 0.00001234);
  T_SPRINTF("-1.23e-308",
            "%.3g", -1.2345e-308);
  T_SPRINTF("+1.230E+308",
            "%+.3E", 1.23e+308);
#endif

  // out of range for float: should switch to exp notation if supported, else empty
#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  T_SPRINTF("1.0e+20",
            "%.1f", 1E20);
#else
  T_SPRINTF("",
            "%.1f", 1E20);
#endif

  // brute force float
  do {
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
  } while (0);

#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  do {
    bool fail = false;
    std::stringstream str;
  // brute force exp
    str.precision(5);
  str.setf(std::ios::scientific, std::ios::floatfield);
  for (float i = -1e20; i < 1e20f; i += 1e15f) {
    tested_sprintf(buffer, "%.5f", (double)i);
    str.str("");
    str << i;
    fail = fail || !!strcmp(buffer, str.str().c_str());
  }
  REQUIRE(!fail);
  } while (0);
#endif
}

TEST_CASE("float rounding", "[]") {
  TEST_DEF;

#if ENABLE_FAILING
  // improper handling of round when decimals is affected
  T_SPRINTF("1.0",
            "%.1f", 0.95);
#endif
}

TEST_CASE("types", "[]" ) {
  TEST_DEF;

  T_SPRINTF("0",
            "%i", 0);
  T_SPRINTF("1234",
            "%i", 1234);
  T_SPRINTF("32767",
            "%i", 32767);
  T_SPRINTF("-32767",
            "%i", -32767);
  T_SPRINTF("30",
            "%li", 30L);
  T_SPRINTF("-2147483647",
            "%li", -2147483647L);
  T_SPRINTF("2147483647",
            "%li", 2147483647L);
  T_SPRINTF("30",
            "%lli", 30LL);
  T_SPRINTF("-9223372036854775807",
            "%lli", -9223372036854775807LL);
  T_SPRINTF("9223372036854775807",
            "%lli", 9223372036854775807LL);
  T_SPRINTF("100000",
            "%lu", 100000L);
  T_SPRINTF("4294967295",
            "%lu", 0xFFFFFFFFL);
  T_SPRINTF("281474976710656",
            "%llu", 281474976710656LLU);
  T_SPRINTF("18446744073709551615",
            "%llu", 18446744073709551615LLU);
  T_SPRINTF("2147483647",
            "%zu", 2147483647UL);
  T_SPRINTF("2147483647",
            "%zd", 2147483647UL);
  if (sizeof(size_t) == sizeof(long)) {
    T_SPRINTF("-2147483647",
              "%zi", -2147483647L);
  }
  else {
    T_SPRINTF("-2147483647",
              "%zi", -2147483647LL);
  }

  T_SPRINTF("1110101001100000",
            "%b", 60000);
  T_SPRINTF("101111000110000101001110",
            "%lb", 12345678L);
  T_SPRINTF("165140",
            "%o", 60000);
  T_SPRINTF("57060516",
            "%lo", 12345678L);
  T_SPRINTF("12345678",
            "%lx", 0x12345678L);
  T_SPRINTF("1234567891234567",
            "%llx", 0x1234567891234567LLU);
  T_SPRINTF("abcdefab",
            "%lx", 0xabcdefabL);
  T_SPRINTF("ABCDEFAB",
            "%lX", 0xabcdefabL);
  T_SPRINTF("v",
            "%c", 'v');
  T_SPRINTF("wv",
            "%cv", 'w');
  T_SPRINTF("A Test",
            "%s", "A Test");
  T_SPRINTF("255",
            "%hhu", 0xFFFFUL);
  T_SPRINTF("13398",
            "%hu", 0x123456UL);
  T_SPRINTF("Test16 65535",
            "%s%hhi %hu", "Test", 10000, 0xFFFFFFFF);
  T_SPRINTF("a",
            "%tx", &buffer[10] - &buffer[0]);
// TBD
  if (sizeof(intmax_t) == sizeof(long)) {
    T_SPRINTF("-2147483647",
              "%ji", -2147483647L);
  }
  else {
    T_SPRINTF("-2147483647",
              "%ji", -2147483647LL);
  }
}


TEST_CASE("pointer", "[]" ) {
  TEST_DEF;

  if (sizeof(void*) == 4U) {
    T_SPRINTF("00001234",
              "%p", (void*)0x1234U);
  }
  else {
    T_SPRINTF("0000000000001234",
              "%p", (void*)0x1234U);
  }

  if (sizeof(void*) == 4U) {
    T_SPRINTF("12345678",
              "%p", (void*)0x12345678U);
  }
  else {
    T_SPRINTF("0000000012345678",
              "%p", (void*)0x12345678U);
  }

  if (sizeof(void*) == 4U) {
    T_SPRINTF("12345678-7EDCBA98",
              "%p-%p", (void*)0x12345678U, (void*)0x7EDCBA98U);
  }
  else {
    T_SPRINTF("0000000012345678-000000007EDCBA98",
              "%p-%p", (void*)0x12345678U, (void*)0x7EDCBA98U);
  }
  if (sizeof(uintptr_t) == sizeof(uint64_t)) {
    T_SPRINTF("00000000FFFFFFFF",
              "%p", (void*)(uintptr_t)0xFFFFFFFFU);
  }
  else {
    T_SPRINTF("FFFFFFFF",
              "%p", (void*)(uintptr_t)0xFFFFFFFFU);
  }
}


TEST_CASE("unknown flag", "[]" ) {
  TEST_DEF;

  T_SPRINTF("kmarco",
            "%kmarco", 42, 37);
}


TEST_CASE("string length", "[]" ) {
  TEST_DEF;

  T_SPRINTF("This",
            "%.4s", "This is a test");
  T_SPRINTF("test",
            "%.4s", "test");
  T_SPRINTF("123",
            "%.7s", "123");
  T_SPRINTF("",
            "%.7s", "");
  T_SPRINTF("1234ab",
            "%.4s%.2s", "123456", "abcdef");
  T_SPRINTF(".2s",
            "%.4.2s", "123456");
  T_SPRINTF("123",
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

  T_SNPRINTF("H", 5,
             2, "%s", "Hello");
}


TEST_CASE("ret value", "[]" ) {
  TEST_DEF;

  T_SNPRINTF("01234", 5,
             6, "0%s", "1234");

  T_SNPRINTF("01234", 6, // '5' is truncated
             6, "0%s", "12345");

  T_SNPRINTF("01234", 8, // '567' are truncated
             6, "0%s", "1234567");

  T_SNPRINTF(NULL, 12,
             10, "hello, world");

  T_SNPRINTF("10", 5,
             3, "%d", 10000);
}


TEST_CASE("misc", "[]" ) {
  TEST_DEF;

  T_SPRINTF("53000atest-20 bit",
            "%u%u%ctest%d %s", 5, 3000, 'a', -20, "bit");
  T_SPRINTF("0.33",
            "%.*f", 2, 0.33333333);
  T_SPRINTF("1",
            "%.*d", -1, 1);
  T_SPRINTF("foo",
            "%.3s", "foobar");
  T_SPRINTF(" ",
            "% .0d", 0);
  T_SPRINTF("     00004",
            "%10.5d", 4);
  T_SPRINTF("hi x",
            "%*sx", -3, "hi");
#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  T_SPRINTF("0.33",
            "%.*g", 2, 0.33333333);
  T_SPRINTF("3.33e-01",
            "%.*e", 2, 0.33333333);
#endif
}


// libinsanity printf tests
// from https://github.com/wm4/libinsanity/blob/master/tests/printf_test.c
// 4363908964d0d890fedf47b8d130bb10f800ef14

///////////////////////////////////////////////////////////////////////////////
// (c) Marco Paland (info@paland.com)
//     2017-2018, PALANDesign Hannover, Germany
//
// The MIT License (MIT)
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
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS INx
// THE SOFTWARE.
//
// Additional tests from libc-testsuite (musl)
// a9baddd7d07b9fe15e212985a808a79773ec72e4


#define REQUIRE_STR_EQ(got, expect) REQUIRE_THAT(got, Equals(expect))
#define REQUIRE_INT_EQ(got, expect) REQUIRE((got) == (expect))

// Some tests use redundant format specifier flags (against which gcc warns,
// and which OS libc will usually not support).
#define TEST_NON_STANDARD 1

// Tests redundant flags, against which gcc warns. It's possible that testing
// this would be worth anyway, because the standard does not forbid passing
// them. Normally they just cause noise with -Wformat (which you should always
// enable).
#define TEST_REDUNDANT_FLAGS 1

// Implementation defined formatting (%p in particular).
#define TEST_IMPL_DEFINED 1
// explicit argument size specifiers ("%I64d" etc)
#define TEST_IMPL_EXPLICITSIZE 0

#define EXP(...) __VA_ARGS__

// Require that printf(args) outputs res.
#define TEST_SNPRINTF(args, expected)                   \
  do {                                                  \
    CAPTURE(EXP args);                                  \
    ret = tested_sprintf(buffer, EXP args);             \
    CHECK_THAT(buffer, Equals(expected));               \
    (void)ret;                                          \
  } while (0)                                           \
    /**/

// Require that snprintf(..., 0, args) returns res.
#define TEST_SNPRINTF_N(args, res)                      \
  do {                                                  \
    CAPTURE(EXP args);                                  \
    ret = tested_sprintf(buffer, EXP args);             \
    CHECK(ret == res);                                  \
    (void)ret;                                          \
  } while (0)                                           \
    /**/

TEST_CASE("lin:basic sanity", "[libinsanity]" ) {
  TEST_DEF;
  TEST_SNPRINTF(("%d", -1000), "-1000");
  tested_snprintf(buffer, 3U, "%d", -1000);
  REQUIRE_STR_EQ(buffer, "-1");
  TEST_SNPRINTF(("%d", -1), "-1");

  tested_snprintf(buffer, sizeof(buffer), "%d %d %s", 3, -1000, "test");
  REQUIRE_STR_EQ(buffer, "3 -1000 test");
}

TEST_CASE("lin:space+width", "[libinsanity]" ) {
  TEST_DEF;

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
}

TEST_CASE("lin:space", "[libinsanity]" ) {
  TEST_DEF;

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
}

TEST_CASE("lin:plus", "[libinsanity]" ) {
  TEST_DEF;

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
}

TEST_CASE("lin:zero", "[libinsanity]" ) {
  TEST_DEF;
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
}

TEST_CASE("lin:minus", "[libinsanity]" ) {
  TEST_DEF;

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

TEST_CASE("lin:basic specifiers", "[libinsanity]" ) {
  TEST_DEF;

  tested_snprintf(buffer, sizeof(buffer), "Hello testing");
  REQUIRE_STR_EQ(buffer, "Hello testing");

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
}

TEST_CASE("lin:width", "[libinsanity]" ) {
  TEST_DEF;
  TEST_SNPRINTF(("%ls", "Hello testing"), "Hello testing");
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
}
TEST_CASE("lin:width *", "[libinsanity]" ) {
  TEST_DEF;

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
}
TEST_CASE("lin:minus+width", "[libinsanity]" ) {
  TEST_DEF;

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
}

TEST_CASE("lin:zero+minus", "[libinsanity]" ) {
  TEST_DEF;

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

TEST_CASE("lin:zero+width", "[libinsanity]" ) {
  TEST_DEF;

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
}

TEST_CASE("lin:prec", "[libinsanity]" ) {
  TEST_DEF;

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
}

TEST_CASE("lin:hash", "[libinsanity]" ) {
  TEST_DEF;

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
}

TEST_CASE("lin:width+prec", "[libinsanity]" ) {
  TEST_DEF;

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
}

TEST_CASE("lin:%f width/prec", "[libinsanity]" ) {
  TEST_DEF;

  TEST_SNPRINTF(("%.4f", 3.1415354), "3.1415");
  TEST_SNPRINTF(("%.3f", 30343.1415354), "30343.142");
  TEST_SNPRINTF(("%.0f", 34.1415354), "34");
  TEST_SNPRINTF(("%.2f", 42.8952), "42.90");
  TEST_SNPRINTF(("%.9f", 42.8952), "42.895200000");
  TEST_SNPRINTF(("%.10f", 42.895223), "42.8952230000");
#if PRINTF_FLOAT_PREC > 9
  TEST_SNPRINTF(("%.12f", 42.89522312345678), "42.895223123457");
  TEST_SNPRINTF(("%.12f", 42.89522387654321), "42.895223876543");
#else
  // precision is limited to 9
  TEST_SNPRINTF(("%.12f", 42.89522312345678), "42.895223123000");
  TEST_SNPRINTF(("%.12f", 42.89522387654321), "42.895223877000");
#endif
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
#if PRINTF_FLOAT_DECIMALS >= 20 
  TEST_SNPRINTF(("%.1f", 1E20), "100000000000000000000.0");
#else
  // out of range for float: should switch to exp notation if supported, else empty
# ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  TEST_SNPRINTF(("%.1f", 1E20), "1.0e+20");
# else
  TEST_SNPRINTF(("%.1f", 1E20), "");
# endif
#endif
  TEST_SNPRINTF(("a%-5.1f", 0.5), "a0.5  ");
}

TEST_CASE("lin:size", "[libinsanity]" ) {
  TEST_DEF;

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

  TEST_SNPRINTF(("%hhd", -1), "-1");
}

TEST_CASE("lin:prec *", "[libinsanity]" ) {
  TEST_DEF;

  TEST_SNPRINTF(("%.*d", -1, 1), "1");
#if ENABLE_FAILING
  // negative precision as if no precision is given
  TEST_SNPRINTF(("%.*d", -1, 0), "0");
#endif
}

TEST_CASE("lin:%p", "[libinsanity]" ) {
  TEST_DEF;

#if TEST_IMPL_DEFINED && PRINTF_POINTER_0XHEX
  tested_snprintf(buffer, sizeof(buffer), "%p", (void *)(uintptr_t)0x1234U);
  REQUIRE_STR_EQ(buffer, "0x1234");

  tested_snprintf(buffer, sizeof(buffer), "%p", (void *)(uintptr_t)0x12345678U);
  REQUIRE_STR_EQ(buffer, "0x12345678");

  tested_snprintf(buffer, sizeof(buffer), "%p-%p", (void *)0x12345678U,
               (void *)(uintptr_t)0x7EDCBA98U);
  REQUIRE_STR_EQ(buffer, "0x12345678-0x7edcba98");

  tested_snprintf(buffer, sizeof(buffer), "%p",
               (void *)(uintptr_t)0xFFFFFFFFU);
  REQUIRE_STR_EQ(buffer, "0xffffffff");
#endif
}

TEST_CASE("lin:snprintf maxlength", "[libinsanity]" ) {
  TEST_DEF;

  buffer[0] = (char)0xA5;
  ret = tested_snprintf(buffer, 0, "%s", "Test");
  REQUIRE(buffer[0] == (char)0xA5);
  REQUIRE(ret == 4);

  buffer[0] = (char)0xCC;
  tested_snprintf(buffer, 1, "%s", "Test");
  REQUIRE(buffer[0] == '\0');

  tested_snprintf(buffer, 2, "%s", "Hello");
  REQUIRE_STR_EQ(buffer, "H");

  ret = tested_snprintf(buffer, 6, "0%s", "1234");
  REQUIRE_STR_EQ(buffer, "01234");
  REQUIRE(ret == 5);

  ret = tested_snprintf(buffer, 6, "0%s", "12345");
  REQUIRE_STR_EQ(buffer, "01234");
  REQUIRE(ret == 6);  // '5' is truncated

  ret = tested_snprintf(buffer, 6, "0%s", "1234567");
  REQUIRE_STR_EQ(buffer, "01234");
  REQUIRE(ret == 8);  // '567' are truncated

  ret = tested_snprintf(buffer, 10, "hello, world");
  REQUIRE(ret == 12);

  ret = tested_snprintf(buffer, 3, "%d", 10000);
  REQUIRE(ret == 5);
  REQUIRE(strlen(buffer) == 2U);
  REQUIRE(buffer[0] == '1');
  REQUIRE(buffer[1] == '0');
  REQUIRE(buffer[2] == '\0');
}

TEST_CASE("lin:extreme values", "[libinsanity]" ) {
  TEST_DEF;

#if INT_MAX == 2147483647
  TEST_SNPRINTF(("%d", INT_MAX), "2147483647");
#if ENABLE_FAILING
  // undefined behavior of -INT_MIN
  TEST_SNPRINTF(("%d", INT_MIN), "-2147483648");
#endif
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
#if ENABLE_FAILING
  // undefined behavior of -INT_MIN
  TEST_SNPRINTF(("%" PRIi32, INT32_MIN), "-2147483648");
#endif
  TEST_SNPRINTF(("%" PRIi64, INT64_MAX), "9223372036854775807");
  TEST_SNPRINTF(("%" PRIi64, INT64_MIN), "-9223372036854775808");
  TEST_SNPRINTF(("%" PRIu64, UINT64_MAX), "18446744073709551615");

#if TEST_IMPL_DEFINED && TEST_IMPL_EXPLICITSIZE
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
  TEST_SNPRINTF_N(("%I34d", 123), -1);   // expect error return value
#endif
}

TEST_CASE("lin:%x", "[libinsanity]" ) {
  TEST_DEF;

  TEST_SNPRINTF(("%x", 0), "0");
  TEST_SNPRINTF(("%#x", 0), "0");
  TEST_SNPRINTF(("%#04x", 0), "0000");
  TEST_SNPRINTF(("%#08x", 0x614e), "0x00614e");
  TEST_SNPRINTF(("%#.3x", 0x614e), "0x614e");
  TEST_SNPRINTF(("%#.4x", 0x614e), "0x614e");
  TEST_SNPRINTF(("%#.5x", 0x614e), "0x0614e");
  TEST_SNPRINTF(("%#.6x", 0x614e), "0x00614e");
  TEST_SNPRINTF(("%#.7x", 0x614e), "0x000614e");
}

TEST_CASE("lin:%o", "[libinsanity]" ) {
  TEST_DEF;

  TEST_SNPRINTF(("%o", 00), "0");
  TEST_SNPRINTF(("%#o", 00), "0");
  TEST_SNPRINTF(("%#04o", 0), "0000");
  TEST_SNPRINTF(("%#08o", 06143), "00006143");
  TEST_SNPRINTF(("%#.3o", 06143), "06143");
  TEST_SNPRINTF(("%#.4o", 06143), "06143");
#if ENABLE_FAILING
  // # modified should not output '0' whenre width already supplied one
  TEST_SNPRINTF(("%#.5o", 06143), "06143");
  TEST_SNPRINTF(("%#.6o", 06143), "006143");
  TEST_SNPRINTF(("%#.7o", 06143), "0006143");
#endif
}
    // libc-testsuite tests

TEST_CASE("libc:width/prec/minus", "[libc]" ) {
  TEST_DEF;

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

TEST_CASE("libc:prec=0", "[libc]") {
  TEST_DEF;

  /* ...explicit precision of 0 shall be no characters. */
  TEST_SNPRINTF(("%.0d", 0), "");
  TEST_SNPRINTF(("%.0o", 0), "");
#if TEST_REDUNDANT_FLAGS
  TEST_SNPRINTF(("%#.0d", 0), "");
#endif
  // Note: the original libc-testsuite specifies "" as expected.
#if ENABLE_FAILING
  // octal prefix should be respected
  TEST_SNPRINTF(("%#.0o", 0), "0");
#endif
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
}

TEST_CASE("libc:%x", "[libc]") {
  TEST_DEF;

  /* hex: test alt form and case */
  TEST_SNPRINTF(("%x", 63), "3f");
  TEST_SNPRINTF(("%#x", 63), "0x3f");
  TEST_SNPRINTF(("%X", 63), "3F");
}

 TEST_CASE("libc:%o", "[libc]") {
   TEST_DEF;
   /* octal: test alt form */
   TEST_SNPRINTF(("%o", 15), "17");
   TEST_SNPRINTF(("%#o", 15), "017");
 }

TEST_CASE("libc:%e %f %g", "[libc]") {
  TEST_DEF;

  /* basic form, handling of exponent/precision for 0 */
#if ENABLE_FAILING
  // zero has exponent 0
  TEST_SNPRINTF(("%e", 0.0), "0.000000e+00");
#endif
  TEST_SNPRINTF(("%f", 0.0), "0.000000");
#if ENABLE_FAILING
  // zero has exponent 0
  TEST_SNPRINTF(("%g", 0.0), "0");
  TEST_SNPRINTF(("%#g", 0.0), "0.00000");
#endif
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
#if PRINTF_FLOAT_PREC >= 16
  TEST_SNPRINTF(("%.16f", 1.1), "1.1000000000000001");
#endif
#if PRINTF_FLOAT_PREC >= 17
  TEST_SNPRINTF(("%.17f", 1.1), "1.10000000000000009");
#endif
  TEST_SNPRINTF(("%.2e", 1500001.0), "1.50e+06");
  TEST_SNPRINTF(("%.2e", 1505000.0), "1.50e+06");
  TEST_SNPRINTF(("%.2e", 1505000.00000095367431640625), "1.51e+06");
  TEST_SNPRINTF(("%.2e", 1505001.0), "1.51e+06");
  TEST_SNPRINTF(("%.2e", 1506000.0), "1.51e+06");

  /* correctness in DBL_DIG places */
#if PRINTF_FLOAT_PREC >= 15
  TEST_SNPRINTF(("%.15g", 1.23456789012345), "1.23456789012345");
#endif
  /* correct choice of notation for %g */
#if ENABLE_FAILING
  // %g uses shortest representation, no trailing zeroes even when using %f notation
  TEST_SNPRINTF(("%g", 0.0001), "0.0001");
#endif
#if ENABLE_FAILING
  // %g uses shortest representation, no trailing zeroes
  TEST_SNPRINTF(("%g", 0.00001), "1e-05");
#endif
  TEST_SNPRINTF(("%g", 123456.0), "123456");
#if ENABLE_FAILING
  // %g - precision is number of significant digits
  TEST_SNPRINTF(("%g", 1234567.0), "1.23457e+06");
#endif
#if ENABLE_FAILING
  // %g - use %f when precision allows it
  TEST_SNPRINTF(("%.7g", 1234567.0), "1234567");
#endif
  TEST_SNPRINTF(("%.7g", 12345678.0), "1.234568e+07");
#if ENABLE_FAILING
  // %g uses shortest representation, no trailing zeroes 
  TEST_SNPRINTF(("%.8g", 0.1), "0.1");
  TEST_SNPRINTF(("%.9g", 0.1), "0.1");
  TEST_SNPRINTF(("%.10g", 0.1), "0.1");
  TEST_SNPRINTF(("%.11g", 0.1), "0.1");
#endif
  /* pi in double precision, printed to a few extra places */
#if PRINTF_FLOAT_PREC >= 15
  TEST_SNPRINTF(("%.15f", M_PI), "3.141592653589793");
#endif
#if PRINTF_FLOAT_PREC >= 18
  TEST_SNPRINTF(("%.18f", M_PI), "3.141592653589793116");
#endif
#if PRINTF_FLOAT_DECIMALS >= 39
  /* exact conversion of large integers */
  TEST_SNPRINTF(("%.0f", 340282366920938463463374607431768211456.0),
                "340282366920938463463374607431768211456");
#endif
}

TEST_CASE("libc:snprintf retval", "[libc]") {
  TEST_DEF;

  TEST_SNPRINTF_N(("%d", 123456), 6);
  TEST_SNPRINTF_N(("%.4s", "hello"), 4);
  TEST_SNPRINTF_N(("%.0s", "goodbye"), 0);

  {
    char b[] = "xxxxxxxx";
    const char *s = "%d";
    int res = tested_snprintf(b, 4, s, 123456);
    REQUIRE(res == 6);
    REQUIRE_STR_EQ(b, "123");
    REQUIRE(b[5] == 'x'); // buffer overrun
  }
}
