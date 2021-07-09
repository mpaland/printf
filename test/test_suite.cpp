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


namespace test {
  // use functions in own test namespace to avoid stdio conflicts
  #include "../printf.h"
  #include "../printf.c"
} // namespace test


// Multi-compiler-compatible local warning suppression

#if defined(_MSC_VER)
    #define DISABLE_WARNING_PUSH           __pragma(warning( push ))
    #define DISABLE_WARNING_POP            __pragma(warning( pop ))
    #define DISABLE_WARNING(warningNumber) __pragma(warning( disable : warningNumber ))

    // TODO: find the right warning number for this
    #define DISABLE_WARNING_PRINTF_FORMAT
    #define DISABLE_WARNING_PRINTF_FORMAT_EXTRA_ARGS

#elif defined(__GNUC__) || defined(__clang__)
    #define DO_PRAGMA(X) _Pragma(#X)
    #define DISABLE_WARNING_PUSH           DO_PRAGMA(GCC diagnostic push)
    #define DISABLE_WARNING_POP            DO_PRAGMA(GCC diagnostic pop)
    #define DISABLE_WARNING(warningName)   DO_PRAGMA(GCC diagnostic ignored #warningName)

    #define DISABLE_WARNING_PRINTF_FORMAT    DISABLE_WARNING(-Wformat)
    #define DISABLE_WARNING_PRINTF_FORMAT_EXTRA_ARGS DISABLE_WARNING(-Wformat-extra-args)
    #define DISABLE_WARNING_PRINTF_FORMAT_OVERFLOW DISABLE_WARNING(-Wformat-overflow)

#else
    #define DISABLE_WARNING_PUSH
    #define DISABLE_WARNING_POP
    #define DISABLE_WARNING_PRINTF_FORMAT
    #define DISABLE_WARNING_PRINTF_FORMAT_EXTRA_ARGS
#endif

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS
DISABLE_WARNING_PUSH
DISABLE_WARNING_PRINTF_FORMAT
DISABLE_WARNING_PRINTF_FORMAT_EXTRA_ARGS
#endif

// dummy putchar
static char   printf_buffer[100];
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


TEST_CASE("printf", "[]" ) {
  printf_idx = 0U;
  memset(printf_buffer, 0xCC, 100U);
  CHECK(test::printf_("% d", 4232) == 5);
  CHECK(printf_buffer[5] == (char)0xCC);
  printf_buffer[5] = 0;
  CHECK(!strcmp(printf_buffer, " 4232"));
}


TEST_CASE("fctprintf", "[]" ) {
  printf_idx = 0U;
  memset(printf_buffer, 0xCC, 100U);
  test::fctprintf(&_out_fct, nullptr, "This is a test of %X", 0x12EFU);
  CHECK(!strncmp(printf_buffer, "This is a test of 12EF", 22U));
  CHECK(printf_buffer[22] == (char)0xCC);
}

// output function type
typedef void (*out_fct_type)(char character, void* arg);


static void vfctprintf_builder_1(out_fct_type f, char* buffer, ...)
{
  va_list args;
  va_start(args, buffer);
  test::vfctprintf(f, nullptr, "This is a test of %X", args);
  va_end(args);
}

TEST_CASE("vfctprintf", "[]" ) {
  printf_idx = 0U;
  memset(printf_buffer, 0xCC, 100U);
  vfctprintf_builder_1(&_out_fct, nullptr, 0x12EFU);
  CHECK(!strncmp(printf_buffer, "This is a test of 12EF", 22U));
  CHECK(printf_buffer[22] == (char)0xCC);
}

TEST_CASE("snprintf_", "[]" ) {
  char buffer[100];

  test::snprintf_(buffer, 100U, "%d", -1000);
  CHECK(!strcmp(buffer, "-1000"));

  test::snprintf_(buffer, 3U, "%d", -1000);
  CHECK(!strcmp(buffer, "-1"));
}

static void vprintf_builder_1(char* buffer, ...)
{
  va_list args;
  va_start(args, buffer);
  test::vprintf_("%d", args);
  va_end(args);
}

static void vsprintf_builder_1(char* buffer, ...)
{
  va_list args;
  va_start(args, buffer);
  test::vsprintf_(buffer, "%d", args);
  va_end(args);
}

static void vsnprintf_builder_1(char* buffer, ...)
{
  va_list args;
  va_start(args, buffer);
  test::vsnprintf_(buffer, 100U, "%d", args);
  va_end(args);
}

static void vsprintf_builder_3(char* buffer, ...)
{
  va_list args;
  va_start(args, buffer);
  test::vsprintf_(buffer, "%d %d %s", args);
  va_end(args);
}

static void vsnprintf_builder_3(char* buffer, ...)
{
  va_list args;
  va_start(args, buffer);
  test::vsnprintf_(buffer, 100U, "%d %d %s", args);
  va_end(args);
}


TEST_CASE("vprintf", "[]" ) {
  char buffer[100];
  printf_idx = 0U;
  memset(printf_buffer, 0xCC, 100U);
  vprintf_builder_1(buffer, 2345);
  CHECK(printf_buffer[4] == (char)0xCC);
  printf_buffer[4] = 0;
  CHECK(!strcmp(printf_buffer, "2345"));
}


TEST_CASE("vsprintf", "[]" ) {
  char buffer[100];

  vsprintf_builder_1(buffer, -1);
  CHECK(!strcmp(buffer, "-1"));

  vsprintf_builder_3(buffer, 3, -1000, "test");
  CHECK(!strcmp(buffer, "3 -1000 test"));
}


TEST_CASE("vsnprintf_", "[]" ) {
  char buffer[100];

  vsnprintf_builder_1(buffer, -1);
  CHECK(!strcmp(buffer, "-1"));

  vsnprintf_builder_3(buffer, 3, -1000, "test");
  CHECK(!strcmp(buffer, "3 -1000 test"));
}


TEST_CASE("space flag", "[]" ) {
  char buffer[100];

  test::sprintf_(buffer, "% d", 42);
  CHECK(!strcmp(buffer, " 42"));

  test::sprintf_(buffer, "% d", -42);
  CHECK(!strcmp(buffer, "-42"));

  test::sprintf_(buffer, "% 5d", 42);
  CHECK(!strcmp(buffer, "   42"));

  test::sprintf_(buffer, "% 5d", -42);
  CHECK(!strcmp(buffer, "  -42"));

  test::sprintf_(buffer, "% 15d", 42);
  CHECK(!strcmp(buffer, "             42"));

  test::sprintf_(buffer, "% 15d", -42);
  CHECK(!strcmp(buffer, "            -42"));

  test::sprintf_(buffer, "% 15d", -42);
  CHECK(!strcmp(buffer, "            -42"));

  test::sprintf_(buffer, "% 15.3f", -42.987);
  CHECK(!strcmp(buffer, "        -42.987"));

  test::sprintf_(buffer, "% 15.3f", 42.987);
  CHECK(!strcmp(buffer, "         42.987"));

  test::sprintf_(buffer, "% d", 1024);
  CHECK(!strcmp(buffer, " 1024"));

  test::sprintf_(buffer, "% d", -1024);
  CHECK(!strcmp(buffer, "-1024"));

  test::sprintf_(buffer, "% i", 1024);
  CHECK(!strcmp(buffer, " 1024"));

  test::sprintf_(buffer, "% i", -1024);
  CHECK(!strcmp(buffer, "-1024"));
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS
TEST_CASE("space flag - non-standard format", "[]" ) {
  char buffer[100];

  test::sprintf_(buffer, "% s", "Hello testing");
  CHECK(!strcmp(buffer, "Hello testing"));

  test::sprintf_(buffer, "% u", 1024);
  CHECK(!strcmp(buffer, "1024"));

  test::sprintf_(buffer, "% u", 4294966272U);
  CHECK(!strcmp(buffer, "4294966272"));

  test::sprintf_(buffer, "% o", 511);
  CHECK(!strcmp(buffer, "777"));

  test::sprintf_(buffer, "% o", 4294966785U);
  CHECK(!strcmp(buffer, "37777777001"));

  test::sprintf_(buffer, "% x", 305441741);
  CHECK(!strcmp(buffer, "1234abcd"));

  test::sprintf_(buffer, "% x", 3989525555U);
  CHECK(!strcmp(buffer, "edcb5433"));

  test::sprintf_(buffer, "% X", 305441741);
  CHECK(!strcmp(buffer, "1234ABCD"));

  test::sprintf_(buffer, "% X", 3989525555U);
  CHECK(!strcmp(buffer, "EDCB5433"));

  test::sprintf_(buffer, "% c", 'x');
  CHECK(!strcmp(buffer, "x"));
}
#endif


TEST_CASE("+ flag", "[]" ) {
  char buffer[100];

  test::sprintf_(buffer, "%+d", 42);
  CHECK(!strcmp(buffer, "+42"));

  test::sprintf_(buffer, "%+d", -42);
  CHECK(!strcmp(buffer, "-42"));

  test::sprintf_(buffer, "%+5d", 42);
  CHECK(!strcmp(buffer, "  +42"));

  test::sprintf_(buffer, "%+5d", -42);
  CHECK(!strcmp(buffer, "  -42"));

  test::sprintf_(buffer, "%+15d", 42);
  CHECK(!strcmp(buffer, "            +42"));

  test::sprintf_(buffer, "%+15d", -42);
  CHECK(!strcmp(buffer, "            -42"));

  test::sprintf_(buffer, "%+d", 1024);
  CHECK(!strcmp(buffer, "+1024"));

  test::sprintf_(buffer, "%+d", -1024);
  CHECK(!strcmp(buffer, "-1024"));

  test::sprintf_(buffer, "%+i", 1024);
  CHECK(!strcmp(buffer, "+1024"));

  test::sprintf_(buffer, "%+i", -1024);
  CHECK(!strcmp(buffer, "-1024"));

  test::sprintf_(buffer, "%+.0d", 0);
  CHECK(!strcmp(buffer, "+"));
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS
TEST_CASE("+ flag - non-standard format", "[]" ) {
  char buffer[100];

  test::sprintf_(buffer, "%+s", "Hello testing");
  CHECK(!strcmp(buffer, "Hello testing"));

  test::sprintf_(buffer, "%+u", 1024);
  CHECK(!strcmp(buffer, "1024"));

  test::sprintf_(buffer, "%+u", 4294966272U);
  CHECK(!strcmp(buffer, "4294966272"));

  test::sprintf_(buffer, "%+o", 511);
  CHECK(!strcmp(buffer, "777"));

  test::sprintf_(buffer, "%+o", 4294966785U);
  CHECK(!strcmp(buffer, "37777777001"));

  test::sprintf_(buffer, "%+x", 305441741);
  CHECK(!strcmp(buffer, "1234abcd"));

  test::sprintf_(buffer, "%+x", 3989525555U);
  CHECK(!strcmp(buffer, "edcb5433"));

  test::sprintf_(buffer, "%+X", 305441741);
  CHECK(!strcmp(buffer, "1234ABCD"));

  test::sprintf_(buffer, "%+X", 3989525555U);
  CHECK(!strcmp(buffer, "EDCB5433"));

  test::sprintf_(buffer, "%+c", 'x');
  CHECK(!strcmp(buffer, "x"));
}
#endif


TEST_CASE("0 flag", "[]" ) {
  char buffer[100];

  test::sprintf_(buffer, "%0d", 42);
  CHECK(!strcmp(buffer, "42"));

  test::sprintf_(buffer, "%0ld", 42L);
  CHECK(!strcmp(buffer, "42"));

  test::sprintf_(buffer, "%0d", -42);
  CHECK(!strcmp(buffer, "-42"));

  test::sprintf_(buffer, "%05d", 42);
  CHECK(!strcmp(buffer, "00042"));

  test::sprintf_(buffer, "%05d", -42);
  CHECK(!strcmp(buffer, "-0042"));

  test::sprintf_(buffer, "%015d", 42);
  CHECK(!strcmp(buffer, "000000000000042"));

  test::sprintf_(buffer, "%015d", -42);
  CHECK(!strcmp(buffer, "-00000000000042"));

  test::sprintf_(buffer, "%015.2f", 42.1234);
  CHECK(!strcmp(buffer, "000000000042.12"));

  test::sprintf_(buffer, "%015.3f", 42.9876);
  CHECK(!strcmp(buffer, "00000000042.988"));

  test::sprintf_(buffer, "%015.5f", -42.9876);
  CHECK(!strcmp(buffer, "-00000042.98760"));
}


TEST_CASE("- flag", "[]" ) {
  char buffer[100];

  test::sprintf_(buffer, "%-d", 42);
  CHECK(!strcmp(buffer, "42"));

  test::sprintf_(buffer, "%-d", -42);
  CHECK(!strcmp(buffer, "-42"));

  test::sprintf_(buffer, "%-5d", 42);
  CHECK(!strcmp(buffer, "42   "));

  test::sprintf_(buffer, "%-5d", -42);
  CHECK(!strcmp(buffer, "-42  "));

  test::sprintf_(buffer, "%-15d", 42);
  CHECK(!strcmp(buffer, "42             "));

  test::sprintf_(buffer, "%-15d", -42);
  CHECK(!strcmp(buffer, "-42            "));
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS
TEST_CASE("- flag - non-standard format", "[]" ) {
  char buffer[100];

  test::sprintf_(buffer, "%-0d", 42);
  CHECK(!strcmp(buffer, "42"));

  test::sprintf_(buffer, "%-0d", -42);
  CHECK(!strcmp(buffer, "-42"));

  test::sprintf_(buffer, "%-05d", 42);
  CHECK(!strcmp(buffer, "42   "));

  test::sprintf_(buffer, "%-05d", -42);
  CHECK(!strcmp(buffer, "-42  "));

  test::sprintf_(buffer, "%-015d", 42);
  CHECK(!strcmp(buffer, "42             "));

  test::sprintf_(buffer, "%-015d", -42);
  CHECK(!strcmp(buffer, "-42            "));

  test::sprintf_(buffer, "%0-d", 42);
  CHECK(!strcmp(buffer, "42"));

  test::sprintf_(buffer, "%0-d", -42);
  CHECK(!strcmp(buffer, "-42"));

  test::sprintf_(buffer, "%0-5d", 42);
  CHECK(!strcmp(buffer, "42   "));

  test::sprintf_(buffer, "%0-5d", -42);
  CHECK(!strcmp(buffer, "-42  "));

  test::sprintf_(buffer, "%0-15d", 42);
  CHECK(!strcmp(buffer, "42             "));

  test::sprintf_(buffer, "%0-15d", -42);
  CHECK(!strcmp(buffer, "-42            "));

  test::sprintf_(buffer, "%0-15.3e", -42.);
#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  CHECK(!strcmp(buffer, "-4.200e+01     "));
#else
  CHECK(!strcmp(buffer, "e"));
#endif

  test::sprintf_(buffer, "%0-15.3g", -42.);
#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  CHECK(!strcmp(buffer, "-42.0          "));
#else
  CHECK(!strcmp(buffer, "g"));
#endif
}
#endif


TEST_CASE("# flag", "[]" ) {
  char buffer[100];

  test::sprintf(buffer, "%#o", 0);
  CHECK(!strcmp(buffer, "0"));
  test::sprintf(buffer, "%#0o", 0);
  CHECK(!strcmp(buffer, "0"));
  test::sprintf(buffer, "%#.0o", 0);
  CHECK(!strcmp(buffer, "0"));
  test::sprintf(buffer, "%#.1o", 0);
  CHECK(!strcmp(buffer, "0"));
  test::sprintf(buffer, "%#4o", 0);
  CHECK(!strcmp(buffer, "   0"));
  test::sprintf(buffer, "%#.4o", 0);
  CHECK(!strcmp(buffer, "0000"));

  test::sprintf(buffer, "%#o", 1);
  CHECK(!strcmp(buffer, "01"));
  test::sprintf(buffer, "%#0o", 1);
  CHECK(!strcmp(buffer, "01"));
  test::sprintf(buffer, "%#.0o", 1);
  CHECK(!strcmp(buffer, "01"));
  test::sprintf(buffer, "%#.1o", 1);
  CHECK(!strcmp(buffer, "01"));
  test::sprintf(buffer, "%#4o", 1);
  CHECK(!strcmp(buffer, "  01"));
  test::sprintf(buffer, "%#.4o", 1);
  CHECK(!strcmp(buffer, "0001"));

  test::sprintf(buffer, "%#04x", 0x1001);
  CHECK(!strcmp(buffer, "0x1001"));
  test::sprintf(buffer, "%#04o", 01001);
  CHECK(!strcmp(buffer, "01001"));

  test::sprintf(buffer, "%#.0llx", (long long)0);
  CHECK(!strcmp(buffer, ""));
  test::sprintf_(buffer, "%#.8x", 0x614e);
  CHECK(!strcmp(buffer, "0x0000614e"));
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS
TEST_CASE("# flag - non-standard format", "[]" ) {
  char buffer[100];

  test::sprintf_(buffer,"%#b", 6);
  CHECK(!strcmp(buffer, "0b110"));
}
#endif

TEST_CASE("specifier", "[]" ) {
  char buffer[100];

  test::sprintf_(buffer, "Hello testing");
  CHECK(!strcmp(buffer, "Hello testing"));

  test::sprintf_(buffer, "%s", "Hello testing");
  CHECK(!strcmp(buffer, "Hello testing"));

DISABLE_WARNING_PUSH
DISABLE_WARNING_PRINTF_FORMAT_OVERFLOW
  test::sprintf_(buffer, "%s", NULL);
DISABLE_WARNING_POP
  CHECK(!strcmp(buffer, "(null)"));

  test::sprintf_(buffer, "%d", 1024);
  CHECK(!strcmp(buffer, "1024"));

  test::sprintf_(buffer, "%d", -1024);
  CHECK(!strcmp(buffer, "-1024"));

  test::sprintf_(buffer, "%i", 1024);
  CHECK(!strcmp(buffer, "1024"));

  test::sprintf_(buffer, "%i", -1024);
  CHECK(!strcmp(buffer, "-1024"));

  test::sprintf_(buffer, "%u", 1024);
  CHECK(!strcmp(buffer, "1024"));

  test::sprintf_(buffer, "%u", 4294966272U);
  CHECK(!strcmp(buffer, "4294966272"));

  test::sprintf_(buffer, "%o", 511);
  CHECK(!strcmp(buffer, "777"));

  test::sprintf_(buffer, "%o", 4294966785U);
  CHECK(!strcmp(buffer, "37777777001"));

  test::sprintf_(buffer, "%x", 305441741);
  CHECK(!strcmp(buffer, "1234abcd"));

  test::sprintf_(buffer, "%x", 3989525555U);
  CHECK(!strcmp(buffer, "edcb5433"));

  test::sprintf_(buffer, "%X", 305441741);
  CHECK(!strcmp(buffer, "1234ABCD"));

  test::sprintf_(buffer, "%X", 3989525555U);
  CHECK(!strcmp(buffer, "EDCB5433"));

  test::sprintf_(buffer, "%%");
  CHECK(!strcmp(buffer, "%"));
}


TEST_CASE("width", "[]" ) {
  char buffer[100];

  test::sprintf_(buffer, "%1s", "Hello testing");
  CHECK(!strcmp(buffer, "Hello testing"));

  test::sprintf_(buffer, "%1d", 1024);
  CHECK(!strcmp(buffer, "1024"));

  test::sprintf_(buffer, "%1d", -1024);
  CHECK(!strcmp(buffer, "-1024"));

  test::sprintf_(buffer, "%1i", 1024);
  CHECK(!strcmp(buffer, "1024"));

  test::sprintf_(buffer, "%1i", -1024);
  CHECK(!strcmp(buffer, "-1024"));

  test::sprintf_(buffer, "%1u", 1024);
  CHECK(!strcmp(buffer, "1024"));

  test::sprintf_(buffer, "%1u", 4294966272U);
  CHECK(!strcmp(buffer, "4294966272"));

  test::sprintf_(buffer, "%1o", 511);
  CHECK(!strcmp(buffer, "777"));

  test::sprintf_(buffer, "%1o", 4294966785U);
  CHECK(!strcmp(buffer, "37777777001"));

  test::sprintf_(buffer, "%1x", 305441741);
  CHECK(!strcmp(buffer, "1234abcd"));

  test::sprintf_(buffer, "%1x", 3989525555U);
  CHECK(!strcmp(buffer, "edcb5433"));

  test::sprintf_(buffer, "%1X", 305441741);
  CHECK(!strcmp(buffer, "1234ABCD"));

  test::sprintf_(buffer, "%1X", 3989525555U);
  CHECK(!strcmp(buffer, "EDCB5433"));

  test::sprintf_(buffer, "%1c", 'x');
  CHECK(!strcmp(buffer, "x"));
}


TEST_CASE("width 20", "[]" ) {
  char buffer[100];

  test::sprintf_(buffer, "%20s", "Hello");
  CHECK(!strcmp(buffer, "               Hello"));

  test::sprintf_(buffer, "%20d", 1024);
  CHECK(!strcmp(buffer, "                1024"));

  test::sprintf_(buffer, "%20d", -1024);
  CHECK(!strcmp(buffer, "               -1024"));

  test::sprintf_(buffer, "%20i", 1024);
  CHECK(!strcmp(buffer, "                1024"));

  test::sprintf_(buffer, "%20i", -1024);
  CHECK(!strcmp(buffer, "               -1024"));

  test::sprintf_(buffer, "%20u", 1024);
  CHECK(!strcmp(buffer, "                1024"));

  test::sprintf_(buffer, "%20u", 4294966272U);
  CHECK(!strcmp(buffer, "          4294966272"));

  test::sprintf_(buffer, "%20o", 511);
  CHECK(!strcmp(buffer, "                 777"));

  test::sprintf_(buffer, "%20o", 4294966785U);
  CHECK(!strcmp(buffer, "         37777777001"));

  test::sprintf_(buffer, "%20x", 305441741);
  CHECK(!strcmp(buffer, "            1234abcd"));

  test::sprintf_(buffer, "%20x", 3989525555U);
  CHECK(!strcmp(buffer, "            edcb5433"));

  test::sprintf_(buffer, "%20X", 305441741);
  CHECK(!strcmp(buffer, "            1234ABCD"));

  test::sprintf_(buffer, "%20X", 3989525555U);
  CHECK(!strcmp(buffer, "            EDCB5433"));

  test::sprintf_(buffer, "%20c", 'x');
  CHECK(!strcmp(buffer, "                   x"));
}


TEST_CASE("width *20", "[]" ) {
  char buffer[100];

  test::sprintf_(buffer, "%*s", 20, "Hello");
  CHECK(!strcmp(buffer, "               Hello"));

  test::sprintf_(buffer, "%*d", 20, 1024);
  CHECK(!strcmp(buffer, "                1024"));

  test::sprintf_(buffer, "%*d", 20, -1024);
  CHECK(!strcmp(buffer, "               -1024"));

  test::sprintf_(buffer, "%*i", 20, 1024);
  CHECK(!strcmp(buffer, "                1024"));

  test::sprintf_(buffer, "%*i", 20, -1024);
  CHECK(!strcmp(buffer, "               -1024"));

  test::sprintf_(buffer, "%*u", 20, 1024);
  CHECK(!strcmp(buffer, "                1024"));

  test::sprintf_(buffer, "%*u", 20, 4294966272U);
  CHECK(!strcmp(buffer, "          4294966272"));

  test::sprintf_(buffer, "%*o", 20, 511);
  CHECK(!strcmp(buffer, "                 777"));

  test::sprintf_(buffer, "%*o", 20, 4294966785U);
  CHECK(!strcmp(buffer, "         37777777001"));

  test::sprintf_(buffer, "%*x", 20, 305441741);
  CHECK(!strcmp(buffer, "            1234abcd"));

  test::sprintf_(buffer, "%*x", 20, 3989525555U);
  CHECK(!strcmp(buffer, "            edcb5433"));

  test::sprintf_(buffer, "%*X", 20, 305441741);
  CHECK(!strcmp(buffer, "            1234ABCD"));

  test::sprintf_(buffer, "%*X", 20, 3989525555U);
  CHECK(!strcmp(buffer, "            EDCB5433"));

  test::sprintf_(buffer, "%*c", 20,'x');
  CHECK(!strcmp(buffer, "                   x"));
}


TEST_CASE("width -20", "[]" ) {
  char buffer[100];

  test::sprintf_(buffer, "%-20s", "Hello");
  CHECK(!strcmp(buffer, "Hello               "));

  test::sprintf_(buffer, "%-20d", 1024);
  CHECK(!strcmp(buffer, "1024                "));

  test::sprintf_(buffer, "%-20d", -1024);
  CHECK(!strcmp(buffer, "-1024               "));

  test::sprintf_(buffer, "%-20i", 1024);
  CHECK(!strcmp(buffer, "1024                "));

  test::sprintf_(buffer, "%-20i", -1024);
  CHECK(!strcmp(buffer, "-1024               "));

  test::sprintf_(buffer, "%-20u", 1024);
  CHECK(!strcmp(buffer, "1024                "));

  test::sprintf_(buffer, "%-20.4f", 1024.1234);
  CHECK(!strcmp(buffer, "1024.1234           "));

  test::sprintf_(buffer, "%-20u", 4294966272U);
  CHECK(!strcmp(buffer, "4294966272          "));

  test::sprintf_(buffer, "%-20o", 511);
  CHECK(!strcmp(buffer, "777                 "));

  test::sprintf_(buffer, "%-20o", 4294966785U);
  CHECK(!strcmp(buffer, "37777777001         "));

  test::sprintf_(buffer, "%-20x", 305441741);
  CHECK(!strcmp(buffer, "1234abcd            "));

  test::sprintf_(buffer, "%-20x", 3989525555U);
  CHECK(!strcmp(buffer, "edcb5433            "));

  test::sprintf_(buffer, "%-20X", 305441741);
  CHECK(!strcmp(buffer, "1234ABCD            "));

  test::sprintf_(buffer, "%-20X", 3989525555U);
  CHECK(!strcmp(buffer, "EDCB5433            "));

  test::sprintf_(buffer, "%-20c", 'x');
  CHECK(!strcmp(buffer, "x                   "));

  test::sprintf_(buffer, "|%5d| |%-2d| |%5d|", 9, 9, 9);
  CHECK(!strcmp(buffer, "|    9| |9 | |    9|"));

  test::sprintf_(buffer, "|%5d| |%-2d| |%5d|", 10, 10, 10);
  CHECK(!strcmp(buffer, "|   10| |10| |   10|"));

  test::sprintf_(buffer, "|%5d| |%-12d| |%5d|", 9, 9, 9);
  CHECK(!strcmp(buffer, "|    9| |9           | |    9|"));

  test::sprintf_(buffer, "|%5d| |%-12d| |%5d|", 10, 10, 10);
  CHECK(!strcmp(buffer, "|   10| |10          | |   10|"));
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS
TEST_CASE("width 0-20", "[]" ) {
  char buffer[100];

  test::sprintf_(buffer, "%0-20s", "Hello");
  CHECK(!strcmp(buffer, "Hello               "));

  test::sprintf_(buffer, "%0-20d", 1024);
  CHECK(!strcmp(buffer, "1024                "));

  test::sprintf_(buffer, "%0-20d", -1024);
  CHECK(!strcmp(buffer, "-1024               "));

  test::sprintf_(buffer, "%0-20i", 1024);
  CHECK(!strcmp(buffer, "1024                "));

  test::sprintf_(buffer, "%0-20i", -1024);
  CHECK(!strcmp(buffer, "-1024               "));

  test::sprintf_(buffer, "%0-20u", 1024);
  CHECK(!strcmp(buffer, "1024                "));

  test::sprintf_(buffer, "%0-20u", 4294966272U);
  CHECK(!strcmp(buffer, "4294966272          "));

  test::sprintf_(buffer, "%0-20o", 511);
  CHECK(!strcmp(buffer, "777                 "));

  test::sprintf_(buffer, "%0-20o", 4294966785U);
  CHECK(!strcmp(buffer, "37777777001         "));

  test::sprintf_(buffer, "%0-20x", 305441741);
  CHECK(!strcmp(buffer, "1234abcd            "));

  test::sprintf_(buffer, "%0-20x", 3989525555U);
  CHECK(!strcmp(buffer, "edcb5433            "));

  test::sprintf_(buffer, "%0-20X", 305441741);
  CHECK(!strcmp(buffer, "1234ABCD            "));

  test::sprintf_(buffer, "%0-20X", 3989525555U);
  CHECK(!strcmp(buffer, "EDCB5433            "));

  test::sprintf_(buffer, "%0-20c", 'x');
  CHECK(!strcmp(buffer, "x                   "));
}
#endif

TEST_CASE("padding 20", "[]" ) {
  char buffer[100];

  test::sprintf_(buffer, "%020d", 1024);
  CHECK(!strcmp(buffer, "00000000000000001024"));

  test::sprintf_(buffer, "%020d", -1024);
  CHECK(!strcmp(buffer, "-0000000000000001024"));

  test::sprintf_(buffer, "%020i", 1024);
  CHECK(!strcmp(buffer, "00000000000000001024"));

  test::sprintf_(buffer, "%020i", -1024);
  CHECK(!strcmp(buffer, "-0000000000000001024"));

  test::sprintf_(buffer, "%020u", 1024);
  CHECK(!strcmp(buffer, "00000000000000001024"));

  test::sprintf_(buffer, "%020u", 4294966272U);
  CHECK(!strcmp(buffer, "00000000004294966272"));

  test::sprintf_(buffer, "%020o", 511);
  CHECK(!strcmp(buffer, "00000000000000000777"));

  test::sprintf_(buffer, "%020o", 4294966785U);
  CHECK(!strcmp(buffer, "00000000037777777001"));

  test::sprintf_(buffer, "%020x", 305441741);
  CHECK(!strcmp(buffer, "0000000000001234abcd"));

  test::sprintf_(buffer, "%020x", 3989525555U);
  CHECK(!strcmp(buffer, "000000000000edcb5433"));

  test::sprintf_(buffer, "%020X", 305441741);
  CHECK(!strcmp(buffer, "0000000000001234ABCD"));

  test::sprintf_(buffer, "%020X", 3989525555U);
  CHECK(!strcmp(buffer, "000000000000EDCB5433"));
}


TEST_CASE("padding .20", "[]" ) {
  char buffer[100];

  test::sprintf_(buffer, "%.20d", 1024);
  CHECK(!strcmp(buffer, "00000000000000001024"));

  test::sprintf_(buffer, "%.20d", -1024);
  CHECK(!strcmp(buffer, "-00000000000000001024"));

  test::sprintf_(buffer, "%.20i", 1024);
  CHECK(!strcmp(buffer, "00000000000000001024"));

  test::sprintf_(buffer, "%.20i", -1024);
  CHECK(!strcmp(buffer, "-00000000000000001024"));

  test::sprintf_(buffer, "%.20u", 1024);
  CHECK(!strcmp(buffer, "00000000000000001024"));

  test::sprintf_(buffer, "%.20u", 4294966272U);
  CHECK(!strcmp(buffer, "00000000004294966272"));

  test::sprintf_(buffer, "%.20o", 511);
  CHECK(!strcmp(buffer, "00000000000000000777"));

  test::sprintf_(buffer, "%.20o", 4294966785U);
  CHECK(!strcmp(buffer, "00000000037777777001"));

  test::sprintf_(buffer, "%.20x", 305441741);
  CHECK(!strcmp(buffer, "0000000000001234abcd"));

  test::sprintf_(buffer, "%.20x", 3989525555U);
  CHECK(!strcmp(buffer, "000000000000edcb5433"));

  test::sprintf_(buffer, "%.20X", 305441741);
  CHECK(!strcmp(buffer, "0000000000001234ABCD"));

  test::sprintf_(buffer, "%.20X", 3989525555U);
  CHECK(!strcmp(buffer, "000000000000EDCB5433"));
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS
TEST_CASE("padding #020 - non-standard format", "[]" ) {
  char buffer[100];

  test::sprintf_(buffer, "%#020d", 1024);
  CHECK(!strcmp(buffer, "00000000000000001024"));

  test::sprintf_(buffer, "%#020d", -1024);
  CHECK(!strcmp(buffer, "-0000000000000001024"));

  test::sprintf_(buffer, "%#020i", 1024);
  CHECK(!strcmp(buffer, "00000000000000001024"));

  test::sprintf_(buffer, "%#020i", -1024);
  CHECK(!strcmp(buffer, "-0000000000000001024"));

  test::sprintf_(buffer, "%#020u", 1024);
  CHECK(!strcmp(buffer, "00000000000000001024"));

  test::sprintf_(buffer, "%#020u", 4294966272U);
  CHECK(!strcmp(buffer, "00000000004294966272"));
}
#endif

TEST_CASE("padding #020", "[]" ) {
  char buffer[100];
  test::sprintf_(buffer, "%#020o", 511);
  CHECK(!strcmp(buffer, "00000000000000000777"));

  test::sprintf_(buffer, "%#020o", 4294966785U);
  CHECK(!strcmp(buffer, "00000000037777777001"));

  test::sprintf_(buffer, "%#020x", 305441741);
  CHECK(!strcmp(buffer, "0x00000000001234abcd"));

  test::sprintf_(buffer, "%#020x", 3989525555U);
  CHECK(!strcmp(buffer, "0x0000000000edcb5433"));

  test::sprintf_(buffer, "%#020X", 305441741);
  CHECK(!strcmp(buffer, "0X00000000001234ABCD"));

  test::sprintf_(buffer, "%#020X", 3989525555U);
  CHECK(!strcmp(buffer, "0X0000000000EDCB5433"));
}


#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS
TEST_CASE("padding #20 - non-standard format", "[]" ) {
  char buffer[100];

  test::sprintf_(buffer, "%#20d", 1024);
  CHECK(!strcmp(buffer, "                1024"));

  test::sprintf_(buffer, "%#20d", -1024);
  CHECK(!strcmp(buffer, "               -1024"));

  test::sprintf_(buffer, "%#20i", 1024);
  CHECK(!strcmp(buffer, "                1024"));

  test::sprintf_(buffer, "%#20i", -1024);
  CHECK(!strcmp(buffer, "               -1024"));

  test::sprintf_(buffer, "%#20u", 1024);
  CHECK(!strcmp(buffer, "                1024"));

  test::sprintf_(buffer, "%#20u", 4294966272U);
  CHECK(!strcmp(buffer, "          4294966272"));
}
#endif

TEST_CASE("padding #20", "[]" ) {
  char buffer[100];

  test::sprintf_(buffer, "%#20o", 511);
  CHECK(!strcmp(buffer, "                0777"));

  test::sprintf_(buffer, "%#20o", 4294966785U);
  CHECK(!strcmp(buffer, "        037777777001"));

  test::sprintf_(buffer, "%#20x", 305441741);
  CHECK(!strcmp(buffer, "          0x1234abcd"));

  test::sprintf_(buffer, "%#20x", 3989525555U);
  CHECK(!strcmp(buffer, "          0xedcb5433"));

  test::sprintf_(buffer, "%#20X", 305441741);
  CHECK(!strcmp(buffer, "          0X1234ABCD"));

  test::sprintf_(buffer, "%#20X", 3989525555U);
  CHECK(!strcmp(buffer, "          0XEDCB5433"));
}


TEST_CASE("padding 20.5", "[]" ) {
  char buffer[100];

  test::sprintf_(buffer, "%20.5d", 1024);
  CHECK(!strcmp(buffer, "               01024"));

  test::sprintf_(buffer, "%20.5d", -1024);
  CHECK(!strcmp(buffer, "              -01024"));

  test::sprintf_(buffer, "%20.5i", 1024);
  CHECK(!strcmp(buffer, "               01024"));

  test::sprintf_(buffer, "%20.5i", -1024);
  CHECK(!strcmp(buffer, "              -01024"));

  test::sprintf_(buffer, "%20.5u", 1024);
  CHECK(!strcmp(buffer, "               01024"));

  test::sprintf_(buffer, "%20.5u", 4294966272U);
  CHECK(!strcmp(buffer, "          4294966272"));

  test::sprintf_(buffer, "%20.5o", 511);
  CHECK(!strcmp(buffer, "               00777"));

  test::sprintf_(buffer, "%20.5o", 4294966785U);
  CHECK(!strcmp(buffer, "         37777777001"));

  test::sprintf_(buffer, "%20.5x", 305441741);
  CHECK(!strcmp(buffer, "            1234abcd"));

  test::sprintf_(buffer, "%20.10x", 3989525555U);
  CHECK(!strcmp(buffer, "          00edcb5433"));

  test::sprintf_(buffer, "%20.5X", 305441741);
  CHECK(!strcmp(buffer, "            1234ABCD"));

  test::sprintf_(buffer, "%20.10X", 3989525555U);
  CHECK(!strcmp(buffer, "          00EDCB5433"));
}


TEST_CASE("padding neg numbers", "[]" ) {
  char buffer[100];

  // space padding
  test::sprintf_(buffer, "% 1d", -5);
  CHECK(!strcmp(buffer, "-5"));

  test::sprintf_(buffer, "% 2d", -5);
  CHECK(!strcmp(buffer, "-5"));

  test::sprintf_(buffer, "% 3d", -5);
  CHECK(!strcmp(buffer, " -5"));

  test::sprintf_(buffer, "% 4d", -5);
  CHECK(!strcmp(buffer, "  -5"));

  // zero padding
  test::sprintf_(buffer, "%01d", -5);
  CHECK(!strcmp(buffer, "-5"));

  test::sprintf_(buffer, "%02d", -5);
  CHECK(!strcmp(buffer, "-5"));

  test::sprintf_(buffer, "%03d", -5);
  CHECK(!strcmp(buffer, "-05"));

  test::sprintf_(buffer, "%04d", -5);
  CHECK(!strcmp(buffer, "-005"));
}


TEST_CASE("float padding neg numbers", "[]" ) {
  char buffer[100];

  // space padding
  test::sprintf_(buffer, "% 3.1f", -5.);
  CHECK(!strcmp(buffer, "-5.0"));

  test::sprintf_(buffer, "% 4.1f", -5.);
  CHECK(!strcmp(buffer, "-5.0"));

  test::sprintf_(buffer, "% 5.1f", -5.);
  CHECK(!strcmp(buffer, " -5.0"));

#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  test::sprintf_(buffer, "% 6.1g", -5.);
  CHECK(!strcmp(buffer, "    -5"));

  test::sprintf_(buffer, "% 6.1e", -5.);
  CHECK(!strcmp(buffer, "-5.0e+00"));

  test::sprintf_(buffer, "% 10.1e", -5.);
  CHECK(!strcmp(buffer, "  -5.0e+00"));
#endif

  // zero padding
  test::sprintf_(buffer, "%03.1f", -5.);
  CHECK(!strcmp(buffer, "-5.0"));

  test::sprintf_(buffer, "%04.1f", -5.);
  CHECK(!strcmp(buffer, "-5.0"));

  test::sprintf_(buffer, "%05.1f", -5.);
  CHECK(!strcmp(buffer, "-05.0"));

  // zero padding no decimal point
  test::sprintf_(buffer, "%01.0f", -5.);
  CHECK(!strcmp(buffer, "-5"));

  test::sprintf_(buffer, "%02.0f", -5.);
  CHECK(!strcmp(buffer, "-5"));

  test::sprintf_(buffer, "%03.0f", -5.);
  CHECK(!strcmp(buffer, "-05"));

#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  test::sprintf_(buffer, "%010.1e", -5.);
  CHECK(!strcmp(buffer, "-005.0e+00"));

  test::sprintf_(buffer, "%07.0E", -5.);
  CHECK(!strcmp(buffer, "-05E+00"));

  test::sprintf_(buffer, "%03.0g", -5.);
  CHECK(!strcmp(buffer, "-05"));
#endif
}

TEST_CASE("length", "[]" ) {
  char buffer[100];

  test::sprintf_(buffer, "%.0s", "Hello testing");
  CHECK(!strcmp(buffer, ""));

  test::sprintf_(buffer, "%20.0s", "Hello testing");
  CHECK(!strcmp(buffer, "                    "));

  test::sprintf_(buffer, "%.s", "Hello testing");
  CHECK(!strcmp(buffer, ""));

  test::sprintf_(buffer, "%20.s", "Hello testing");
  CHECK(!strcmp(buffer, "                    "));

  test::sprintf_(buffer, "%20.0d", 1024);
  CHECK(!strcmp(buffer, "                1024"));

  test::sprintf_(buffer, "%20.0d", -1024);
  CHECK(!strcmp(buffer, "               -1024"));

  test::sprintf_(buffer, "%20.d", 0);
  CHECK(!strcmp(buffer, "                    "));

  test::sprintf_(buffer, "%20.0i", 1024);
  CHECK(!strcmp(buffer, "                1024"));

  test::sprintf_(buffer, "%20.i", -1024);
  CHECK(!strcmp(buffer, "               -1024"));

  test::sprintf_(buffer, "%20.i", 0);
  CHECK(!strcmp(buffer, "                    "));

  test::sprintf_(buffer, "%20.u", 1024);
  CHECK(!strcmp(buffer, "                1024"));

  test::sprintf_(buffer, "%20.0u", 4294966272U);
  CHECK(!strcmp(buffer, "          4294966272"));

  test::sprintf_(buffer, "%20.u", 0U);
  CHECK(!strcmp(buffer, "                    "));

  test::sprintf_(buffer, "%20.o", 511);
  CHECK(!strcmp(buffer, "                 777"));

  test::sprintf_(buffer, "%20.0o", 4294966785U);
  CHECK(!strcmp(buffer, "         37777777001"));

  test::sprintf_(buffer, "%20.o", 0U);
  CHECK(!strcmp(buffer, "                    "));

  test::sprintf_(buffer, "%20.x", 305441741);
  CHECK(!strcmp(buffer, "            1234abcd"));

  test::sprintf_(buffer, "%50.x", 305441741);
  CHECK(!strcmp(buffer, "                                          1234abcd"));

  test::sprintf_(buffer, "%50.x%10.u", 305441741, 12345);
  CHECK(!strcmp(buffer, "                                          1234abcd     12345"));

  test::sprintf_(buffer, "%20.0x", 3989525555U);
  CHECK(!strcmp(buffer, "            edcb5433"));

  test::sprintf_(buffer, "%20.x", 0U);
  CHECK(!strcmp(buffer, "                    "));

  test::sprintf_(buffer, "%20.X", 305441741);
  CHECK(!strcmp(buffer, "            1234ABCD"));

  test::sprintf_(buffer, "%20.0X", 3989525555U);
  CHECK(!strcmp(buffer, "            EDCB5433"));

  test::sprintf_(buffer, "%20.X", 0U);
  CHECK(!strcmp(buffer, "                    "));
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS
TEST_CASE("length - non-standard format", "[]" ) {
  char buffer[100];

  test::sprintf_(buffer, "%02.0u", 0U);
  CHECK(!strcmp(buffer, "  "));

  test::sprintf_(buffer, "%02.0d", 0);
  CHECK(!strcmp(buffer, "  "));
}
#endif


TEST_CASE("float", "[]" ) {
  char buffer[100];

  // test special-case floats using math.h macros
  test::sprintf_(buffer, "%8f", (double) NAN);
  CHECK(!strcmp(buffer, "     nan"));

  test::sprintf_(buffer, "%8f", (double) INFINITY);
  CHECK(!strcmp(buffer, "     inf"));

  test::sprintf_(buffer, "%-8f", (double) -INFINITY);
  CHECK(!strcmp(buffer, "-inf    "));

#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  test::sprintf_(buffer, "%+8e", (double) INFINITY);
  CHECK(!strcmp(buffer, "    +inf"));
#endif

  test::sprintf_(buffer, "%.4f", 3.1415354);
  CHECK(!strcmp(buffer, "3.1415"));

  test::sprintf_(buffer, "%.3f", 30343.1415354);
  CHECK(!strcmp(buffer, "30343.142"));

  // switch from decimal to exponential representation
  //
  test::sprintf_(buffer, "%.0f", (double) ((int64_t)1 * 1000 ) );
  if (PRINTF_MAX_FLOAT < 10e+2) {
    CHECK(!strcmp(buffer, "10e+2"));
  }
  else {
    CHECK(!strcmp(buffer, "1000"));
  }

  test::sprintf_(buffer, "%.0f", (double) ((int64_t)1 * 1000 * 1000 ) );
  if (PRINTF_MAX_FLOAT < 10e+5) {
    CHECK(!strcmp(buffer, "10e+5"));
  }
  else {
    CHECK(!strcmp(buffer, "1000000"));
  }

  test::sprintf_(buffer, "%.0f", (double) ((int64_t)1 * 1000 * 1000 * 1000 ) );
  if (PRINTF_MAX_FLOAT < 10e+8) {
    CHECK(!strcmp(buffer, "10e+8"));
  }
  else {
    CHECK(!strcmp(buffer, "1000000000"));
  }

  test::sprintf_(buffer, "%.0f", (double) ((int64_t)1 * 1000 * 1000 * 1000 * 1000) );
  if (PRINTF_MAX_FLOAT < 10e+11) {
    CHECK(!strcmp(buffer, "10e+11"));
  }
  else {
    CHECK(!strcmp(buffer, "1000000000000"));
  }

  test::sprintf_(buffer, "%.0f", (double) ((int64_t)1 * 1000 * 1000 * 1000 * 1000 * 1000) );
  if (PRINTF_MAX_FLOAT < 10e+14) {
    CHECK(!strcmp(buffer, "10e+14"));
  }
  else {
    CHECK(!strcmp(buffer, "1000000000000000"));
  }

  test::sprintf_(buffer, "%.0f", 34.1415354);
  CHECK(!strcmp(buffer, "34"));

  test::sprintf_(buffer, "%.0f", 1.3);
  CHECK(!strcmp(buffer, "1"));

  test::sprintf_(buffer, "%.0f", 1.55);
  CHECK(!strcmp(buffer, "2"));

  test::sprintf_(buffer, "%.1f", 1.64);
  CHECK(!strcmp(buffer, "1.6"));

  test::sprintf_(buffer, "%.2f", 42.8952);
  CHECK(!strcmp(buffer, "42.90"));

  test::sprintf_(buffer, "%.9f", 42.8952);
  CHECK(!strcmp(buffer, "42.895200000"));

  test::sprintf_(buffer, "%.10f", 42.895223);
  CHECK(!strcmp(buffer, "42.8952230000"));

  // this testcase checks, that the precision is truncated to 9 digits.
  // a perfect working float should return the whole number
  test::sprintf_(buffer, "%.12f", 42.89522312345678);
  CHECK(!strcmp(buffer, "42.895223123000"));

  // this testcase checks, that the precision is truncated AND rounded to 9 digits.
  // a perfect working float should return the whole number
  test::sprintf_(buffer, "%.12f", 42.89522387654321);
  CHECK(!strcmp(buffer, "42.895223877000"));

  test::sprintf_(buffer, "%6.2f", 42.8952);
  CHECK(!strcmp(buffer, " 42.90"));

  test::sprintf_(buffer, "%+6.2f", 42.8952);
  CHECK(!strcmp(buffer, "+42.90"));

  test::sprintf_(buffer, "%+5.1f", 42.9252);
  CHECK(!strcmp(buffer, "+42.9"));

  test::sprintf_(buffer, "%f", 42.5);
  CHECK(!strcmp(buffer, "42.500000"));

  test::sprintf_(buffer, "%.1f", 42.5);
  CHECK(!strcmp(buffer, "42.5"));

  test::sprintf_(buffer, "%f", 42167.0);
  CHECK(!strcmp(buffer, "42167.000000"));

  test::sprintf_(buffer, "%.9f", -12345.987654321);
  CHECK(!strcmp(buffer, "-12345.987654321"));

  test::sprintf_(buffer, "%.1f", 3.999);
  CHECK(!strcmp(buffer, "4.0"));

  test::sprintf_(buffer, "%.0f", 3.5);
  CHECK(!strcmp(buffer, "4"));

  test::sprintf_(buffer, "%.0f", 4.5);
  CHECK(!strcmp(buffer, "4"));

  test::sprintf_(buffer, "%.0f", 3.49);
  CHECK(!strcmp(buffer, "3"));

  test::sprintf_(buffer, "%.1f", 3.49);
  CHECK(!strcmp(buffer, "3.5"));

  test::sprintf_(buffer, "a%-5.1f", 0.5);
  CHECK(!strcmp(buffer, "a0.5  "));

  test::sprintf_(buffer, "a%-5.1fend", 0.5);
  CHECK(!strcmp(buffer, "a0.5  end"));

#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  test::sprintf(buffer, "%.4g", 1.0);
  CHECK(!strcmp(buffer, "1"));
  
  test::sprintf_(buffer, "%G", 12345.678);
  CHECK(!strcmp(buffer, "12345.7"));

  test::sprintf_(buffer, "%.7G", 12345.678);
  CHECK(!strcmp(buffer, "12345.68"));

  test::sprintf_(buffer, "%.5G", 123456789.);
  CHECK(!strcmp(buffer, "1.2346E+08"));

  test::sprintf_(buffer, "%.6G", 12345.);
  CHECK(!strcmp(buffer, "12345.0"));

  test::sprintf_(buffer, "%+12.4g", 123456789.);
  CHECK(!strcmp(buffer, "  +1.235e+08"));

  test::sprintf_(buffer, "%.2G", 0.001234);
  CHECK(!strcmp(buffer, "0.0012"));

  test::sprintf_(buffer, "%+10.4G", 0.001234);
  CHECK(!strcmp(buffer, " +0.001234"));

  test::sprintf_(buffer, "%+012.4g", 0.00001234);
  CHECK(!strcmp(buffer, "+001.234e-05"));

  test::sprintf_(buffer, "%.3g", -1.2345e-308);
  CHECK(!strcmp(buffer, "-1.23e-308"));

  test::sprintf_(buffer, "%+.3E", 1.23e+308);
  CHECK(!strcmp(buffer, "+1.230E+308"));
#endif

  // out of range for float: should switch to exp notation if supported, else empty
  test::sprintf_(buffer, "%.1f", 1E20);
#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  CHECK(!strcmp(buffer, "1.0e+20"));
#else
  CHECK(!strcmp(buffer, ""));
#endif

  // brute force float
  bool fail = false;
  std::stringstream str;
  str.precision(5);
  for (float i = -100000; i < 100000; i += 1) {
    test::sprintf_(buffer, "%.5f", (double)(i / 10000));
    str.str("");
    str << std::fixed << i / 10000;
    fail = fail || !!strcmp(buffer, str.str().c_str());
  }
  CHECK(!fail);


#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  // brute force exp
  str.setf(std::ios::scientific, std::ios::floatfield);
  for (float i = -1e20; i < (float) 1e20; i += (float) 1e15) {
    test::sprintf_(buffer, "%.5f", (double) i);
    str.str("");
    str << i;
    fail = fail || !!strcmp(buffer, str.str().c_str());
  }
  CHECK(!fail);
#endif
}


TEST_CASE("types", "[]" ) {
  char buffer[100];

  test::sprintf_(buffer, "%i", 0);
  CHECK(!strcmp(buffer, "0"));

  test::sprintf_(buffer, "%i", 1234);
  CHECK(!strcmp(buffer, "1234"));

  test::sprintf_(buffer, "%i", 32767);
  CHECK(!strcmp(buffer, "32767"));

  test::sprintf_(buffer, "%i", -32767);
  CHECK(!strcmp(buffer, "-32767"));

  test::sprintf_(buffer, "%li", 30L);
  CHECK(!strcmp(buffer, "30"));

  test::sprintf_(buffer, "%li", -2147483647L);
  CHECK(!strcmp(buffer, "-2147483647"));

  test::sprintf_(buffer, "%li", 2147483647L);
  CHECK(!strcmp(buffer, "2147483647"));

  test::sprintf_(buffer, "%lli", 30LL);
  CHECK(!strcmp(buffer, "30"));

  test::sprintf_(buffer, "%lli", -9223372036854775807LL);
  CHECK(!strcmp(buffer, "-9223372036854775807"));

  test::sprintf_(buffer, "%lli", 9223372036854775807LL);
  CHECK(!strcmp(buffer, "9223372036854775807"));

  test::sprintf_(buffer, "%lu", 100000L);
  CHECK(!strcmp(buffer, "100000"));

  test::sprintf_(buffer, "%lu", 0xFFFFFFFFL);
  CHECK(!strcmp(buffer, "4294967295"));

  test::sprintf_(buffer, "%llu", 281474976710656LLU);
  CHECK(!strcmp(buffer, "281474976710656"));

  test::sprintf_(buffer, "%llu", 18446744073709551615LLU);
  CHECK(!strcmp(buffer, "18446744073709551615"));

  test::sprintf_(buffer, "%zu", (size_t)2147483647UL);
  CHECK(!strcmp(buffer, "2147483647"));

  test::sprintf_(buffer, "%zd", (size_t)2147483647UL);
  CHECK(!strcmp(buffer, "2147483647"));

  test::sprintf_(buffer, "%zi", (ssize_t)-2147483647L);
  CHECK(!strcmp(buffer, "-2147483647"));

  test::sprintf_(buffer, "%o", 60000);
  CHECK(!strcmp(buffer, "165140"));

  test::sprintf_(buffer, "%lo", 12345678L);
  CHECK(!strcmp(buffer, "57060516"));

  test::sprintf_(buffer, "%lx", 0x12345678L);
  CHECK(!strcmp(buffer, "12345678"));

  test::sprintf_(buffer, "%llx", 0x1234567891234567LLU);
  CHECK(!strcmp(buffer, "1234567891234567"));

  test::sprintf_(buffer, "%lx", 0xabcdefabL);
  CHECK(!strcmp(buffer, "abcdefab"));

  test::sprintf_(buffer, "%lX", 0xabcdefabL);
  CHECK(!strcmp(buffer, "ABCDEFAB"));

  test::sprintf_(buffer, "%c", 'v');
  CHECK(!strcmp(buffer, "v"));

  test::sprintf_(buffer, "%cv", 'w');
  CHECK(!strcmp(buffer, "wv"));

  test::sprintf_(buffer, "%s", "A Test");
  CHECK(!strcmp(buffer, "A Test"));

  test::sprintf_(buffer, "%hhu", (unsigned char) 0xFFU);
  CHECK(!strcmp(buffer, "255"));

  test::sprintf_(buffer, "%hu", (unsigned short) 0x1234u);
  CHECK(!strcmp(buffer, "4660"));

  test::sprintf_(buffer, "%s%hhi %hu", "Test", (char) 100, (unsigned short) 0xFFFF);
  CHECK(!strcmp(buffer, "Test100 65535"));

  test::sprintf_(buffer, "%tx", &buffer[10] - &buffer[0]);
  CHECK(!strcmp(buffer, "a"));

  test::sprintf_(buffer, "%ji", (intmax_t)-2147483647L);
  CHECK(!strcmp(buffer, "-2147483647"));
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS
TEST_CASE("types - non-standard format", "[]" ) {
  char buffer[100];

  test::sprintf_(buffer, "%b", 60000);
  CHECK(!strcmp(buffer, "1110101001100000"));

  test::sprintf_(buffer, "%lb", 12345678L);
  CHECK(!strcmp(buffer, "101111000110000101001110"));
}
#endif

TEST_CASE("pointer", "[]" ) {
  char buffer[100];

  test::sprintf_(buffer, "%p", (void*)0x1234U);
  if (sizeof(void*) == 4U) {
    CHECK(!strcmp(buffer, "0x00001234"));
  }
  else {
    CHECK(!strcmp(buffer, "0x0000000000001234"));
  }

  test::sprintf_(buffer, "%p", (void*)0x12345678U);
  if (sizeof(void*) == 4U) {
    CHECK(!strcmp(buffer, "0x12345678"));
  }
  else {
    CHECK(!strcmp(buffer, "0x0000000012345678"));
  }

  test::sprintf_(buffer, "%p-%p", (void*)0x12345678U, (void*)0x7EDCBA98U);
  if (sizeof(void*) == 4U) {
    CHECK(!strcmp(buffer, "0x12345678-0x7edcba98"));
  }
  else {
    CHECK(!strcmp(buffer, "0x0000000012345678-0x000000007edcba98"));
  }

  if (sizeof(uintptr_t) == sizeof(uint64_t)) {
    test::sprintf_(buffer, "%p", (void*)(uintptr_t)0xFFFFFFFFU);
    CHECK(!strcmp(buffer, "0x00000000ffffffff"));
  }
  else {
    test::sprintf_(buffer, "%p", (void*)(uintptr_t)0xFFFFFFFFU);
    CHECK(!strcmp(buffer, "0xffffffff"));
  }

  test::sprintf_(buffer, "%p", NULL);
  CHECK(!strcmp(buffer, "(nil)"));
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS
TEST_CASE("unknown flag (non-standard format)", "[]" ) {
  char buffer[100];

  test::sprintf_(buffer, "%kmarco", 42, 37);
  CHECK(!strcmp(buffer, "kmarco"));
}
#endif

TEST_CASE("string length", "[]" ) {
  char buffer[100];

  test::sprintf_(buffer, "%.4s", "This is a test");
  CHECK(!strcmp(buffer, "This"));

  test::sprintf_(buffer, "%.4s", "test");
  CHECK(!strcmp(buffer, "test"));

  test::sprintf_(buffer, "%.7s", "123");
  CHECK(!strcmp(buffer, "123"));

  test::sprintf_(buffer, "%.7s", "");
  CHECK(!strcmp(buffer, ""));

  test::sprintf_(buffer, "%.4s%.2s", "123456", "abcdef");
  CHECK(!strcmp(buffer, "1234ab"));

  test::sprintf_(buffer, "%.*s", 3, "123456");
  CHECK(!strcmp(buffer, "123"));

DISABLE_WARNING_PUSH
DISABLE_WARNING_PRINTF_FORMAT_OVERFLOW
  test::sprintf_(buffer, "%.*s", 3, NULL);
DISABLE_WARNING_POP
  CHECK(!strcmp(buffer, "(null)"));
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS
TEST_CASE("string length (non-standard format)", "[]" ) {
  char buffer[100];

  test::sprintf_(buffer, "%.4.2s", "123456");
  CHECK(!strcmp(buffer, ".2s"));
}
#endif


TEST_CASE("buffer length", "[]" ) {
  char buffer[100];
  int ret;

  ret = test::snprintf_(nullptr, 10, "%s", "Test");
  CHECK(ret == 4);
  ret = test::snprintf_(nullptr, 0, "%s", "Test");
  CHECK(ret == 4);

  buffer[0] = (char)0xA5;
  ret = test::snprintf_(buffer, 0, "%s", "Test");
  CHECK(buffer[0] == (char)0xA5);
  CHECK(ret == 4);

  buffer[0] = (char)0xCC;
  test::snprintf_(buffer, 1, "%s", "Test");
  CHECK(buffer[0] == '\0');

  test::snprintf_(buffer, 2, "%s", "Hello");
  CHECK(!strcmp(buffer, "H"));

DISABLE_WARNING_PUSH
DISABLE_WARNING_PRINTF_FORMAT_OVERFLOW
  test::snprintf_(buffer, 2, "%s", NULL);
DISABLE_WARNING_POP
  CHECK(!strcmp(buffer, "("));
}


TEST_CASE("ret value", "[]" ) {
  char buffer[100] ;
  int ret;

  ret = test::snprintf_(buffer, 6, "0%s", "1234");
  CHECK(!strcmp(buffer, "01234"));
  CHECK(ret == 5);

  ret = test::snprintf_(buffer, 6, "0%s", "12345");
  CHECK(!strcmp(buffer, "01234"));
  CHECK(ret == 6);  // "5" is truncated

  ret = test::snprintf_(buffer, 6, "0%s", "1234567");
  CHECK(!strcmp(buffer, "01234"));
  CHECK(ret == 8);  // "567" are truncated

DISABLE_WARNING_PUSH
DISABLE_WARNING_PRINTF_FORMAT_OVERFLOW
  ret = test::snprintf_(buffer, 6, "0%s", NULL);
DISABLE_WARNING_POP
  CHECK(!strcmp(buffer, "0(nul"));
  CHECK(ret == 7);  // "l)" is truncated

  ret = test::snprintf_(buffer, 10, "hello, world");
  CHECK(ret == 12);

  ret = test::snprintf_(buffer, 3, "%d", 10000);
  CHECK(ret == 5);
  CHECK(strlen(buffer) == 2U);
  CHECK(buffer[0] == '1');
  CHECK(buffer[1] == '0');
  CHECK(buffer[2] == '\0');
}


TEST_CASE("misc", "[]" ) {
  char buffer[100];

  test::sprintf_(buffer, "%u%u%ctest%d %s", 5, 3000, 'a', -20, "bit");
  CHECK(!strcmp(buffer, "53000atest-20 bit"));

  test::sprintf_(buffer, "%.*f", 2, 0.33333333);
  CHECK(!strcmp(buffer, "0.33"));

  test::sprintf_(buffer, "%.*d", -1, 1);
  CHECK(!strcmp(buffer, "1"));

  test::sprintf_(buffer, "%.3s", "foobar");
  CHECK(!strcmp(buffer, "foo"));

  test::sprintf_(buffer, "% .0d", 0);
  CHECK(!strcmp(buffer, " "));

  test::sprintf_(buffer, "%10.5d", 4);
  CHECK(!strcmp(buffer, "     00004"));

  test::sprintf_(buffer, "%*sx", -3, "hi");
  CHECK(!strcmp(buffer, "hi x"));

#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  test::sprintf_(buffer, "%.*g", 2, 0.33333333);
  CHECK(!strcmp(buffer, "0.33"));

  test::sprintf_(buffer, "%.*e", 2, 0.33333333);
  CHECK(!strcmp(buffer, "3.33e-01"));
#endif
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS
DISABLE_WARNING_POP
#endif

