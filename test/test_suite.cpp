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
//
///////////////////////////////////////////////////////////////////////////////

#define PRINTF_VISIBILITY static
#include <printf_config.h>
#include <printf/printf.c>

// use the 'catch' test framework
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <cstring>
#include <sstream>
#include <cmath>
#include <limits>
#include <climits>

#if defined(_WIN32)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#elif defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
#include <sys/types.h>
#else
// Let's just cross our fingers and hope `ssize_t` is defined.
#endif

#define CAPTURE_AND_PRINT(printer_, ...)                  \
do {                                                      \
  INFO( #printer_  <<                                     \
     " arguments (ignore the equations; interpret \"expr" \
     "\" := expr\" as just \"expr\"): ");                 \
  CAPTURE( __VA_ARGS__);                                  \
  printer_(__VA_ARGS__);                                  \
} while(0)

#define PRINTING_CHECK(expected_, dummy, printer_, buffer_, ...) \
do {                                                             \
  INFO( #printer_ << " arguments, replicated ( \"arg := arg\" "  \
  "):\n----"); \
  CAPTURE( __VA_ARGS__);                                         \
  std::memset(buffer_, 0xCC, 100);                               \
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

// dummy putchar
static char   printf_buffer[100];
static size_t printf_idx = 0U;

void putchar_(char character)
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


TEST_CASE("fctprintf", "[]" ) {
  printf_idx = 0U;
  memset(printf_buffer, 0xCC, 100U);
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

TEST_CASE("vfctprintf", "[]" ) {
  printf_idx = 0U;
  memset(printf_buffer, 0xCC, 100U);
  vfctprintf_builder_1(&_out_fct, nullptr, 0x12EFU);
  CHECK(!strncmp(printf_buffer, "This is a test of 12EF", 22U));
  CHECK(printf_buffer[22] == (char)0xCC);
}

TEST_CASE("snprintf_", "[]" ) {
  char buffer[100];
  PRINTING_CHECK("-1000", ==, snprintf_, buffer, 100U, "%d", -1000);
  PRINTING_CHECK("-1",    ==, snprintf_, buffer, 3U, "%d", -1000);
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
  PRINTING_CHECK(" 42",             ==, sprintf_, buffer, "% d", 42);
  PRINTING_CHECK("-42",             ==, sprintf_, buffer, "% d", -42);
  PRINTING_CHECK("   42",           ==, sprintf_, buffer, "% 5d", 42);
  PRINTING_CHECK("  -42",           ==, sprintf_, buffer, "% 5d", -42);
  PRINTING_CHECK("             42", ==, sprintf_, buffer, "% 15d", 42);
  PRINTING_CHECK("            -42", ==, sprintf_, buffer, "% 15d", -42);
  PRINTING_CHECK("            -42", ==, sprintf_, buffer, "% 15d", -42);
#if PRINTF_SUPPORT_DECIMAL_SPECIFIERS
  PRINTING_CHECK("        -42.987", ==, sprintf_, buffer, "% 15.3f", -42.987);
  PRINTING_CHECK("         42.987", ==, sprintf_, buffer, "% 15.3f", 42.987);
#endif
  PRINTING_CHECK(" 1024",           ==, sprintf_, buffer, "% d", 1024);
  PRINTING_CHECK("-1024",           ==, sprintf_, buffer, "% d", -1024);
  PRINTING_CHECK(" 1024",           ==, sprintf_, buffer, "% i", 1024);
  PRINTING_CHECK("-1024",           ==, sprintf_, buffer, "% i", -1024);
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS
TEST_CASE("space flag - non-standard format", "[]" ) {
  char buffer[100];
  PRINTING_CHECK("Hello testing", ==, sprintf_, buffer, "% s", "Hello testing");
  PRINTING_CHECK("1024",          ==, sprintf_, buffer, "% u", 1024);
#ifdef PRINTF_SUPPORT_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("1024",          ==, sprintf_, buffer, "% I16u", (uint16_t) 1024);
  PRINTING_CHECK("1024",          ==, sprintf_, buffer, "% I32u", (uint32_t) 1024);
  PRINTING_CHECK("1024",          ==, sprintf_, buffer, "% I64u", (uint64_t) 1024);
#endif
  PRINTING_CHECK("4294966272",    ==, sprintf_, buffer, "% u", 4294966272U);
#ifdef PRINTF_SUPPORT_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("4294966272",    ==, sprintf_, buffer, "% I32u", (uint32_t) 4294966272U);
  PRINTING_CHECK("4294966272",    ==, sprintf_, buffer, "% I64u", (uint64_t) 4294966272U);
#endif
  PRINTING_CHECK("777",           ==, sprintf_, buffer, "% o", 511);
  PRINTING_CHECK("37777777001",   ==, sprintf_, buffer, "% o", 4294966785U);
  PRINTING_CHECK("1234abcd",      ==, sprintf_, buffer, "% x", 305441741);
  PRINTING_CHECK("edcb5433",      ==, sprintf_, buffer, "% x", 3989525555U);
  PRINTING_CHECK("1234ABCD",      ==, sprintf_, buffer, "% X", 305441741);
  PRINTING_CHECK("EDCB5433",      ==, sprintf_, buffer, "% X", 3989525555U);
  PRINTING_CHECK("x",             ==, sprintf_, buffer, "% c", 'x');
}
#endif


TEST_CASE("+ flag", "[]" ) {
  char buffer[100];
  PRINTING_CHECK("+42",             ==, sprintf_, buffer, "%+d", 42);
  PRINTING_CHECK("-42",             ==, sprintf_, buffer, "%+d", -42);
  PRINTING_CHECK("  +42",           ==, sprintf_, buffer, "%+5d", 42);
  PRINTING_CHECK("  -42",           ==, sprintf_, buffer, "%+5d", -42);
  PRINTING_CHECK("            +42", ==, sprintf_, buffer, "%+15d", 42);
  PRINTING_CHECK("            -42", ==, sprintf_, buffer, "%+15d", -42);
  PRINTING_CHECK("+1024",           ==, sprintf_, buffer, "%+d", 1024);
  PRINTING_CHECK("-1024",           ==, sprintf_, buffer, "%+d", -1024);
  PRINTING_CHECK("+1024",           ==, sprintf_, buffer, "%+i", 1024);
  PRINTING_CHECK("-1024",           ==, sprintf_, buffer, "%+i", -1024);
#ifdef PRINTF_SUPPORT_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("+1024",           ==, sprintf_, buffer, "%+I16d", (int16_t)  1024);
  PRINTING_CHECK("-1024",           ==, sprintf_, buffer, "%+I16d", (int16_t) -1024);
  PRINTING_CHECK("+1024",           ==, sprintf_, buffer, "%+I32d", (int32_t)  1024);
  PRINTING_CHECK("-1024",           ==, sprintf_, buffer, "%+I32d", (int32_t) -1024);
  PRINTING_CHECK("+1024",           ==, sprintf_, buffer, "%+I64d", (int64_t)  1024);
  PRINTING_CHECK("-1024",           ==, sprintf_, buffer, "%+I64d", (int64_t) -1024);
#endif
  PRINTING_CHECK("+",               ==, sprintf_, buffer, "%+.0d", 0);
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS
TEST_CASE("+ flag - non-standard format", "[]" ) {
  char buffer[100];
  PRINTING_CHECK("Hello testing", ==, sprintf_, buffer, "%+s", "Hello testing");
  PRINTING_CHECK("1024",          ==, sprintf_, buffer, "%+u", 1024);
#ifdef PRINTF_SUPPORT_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("1024",          ==, sprintf_, buffer, "%+I32u", (uint32_t) 1024);
#endif
  PRINTING_CHECK("4294966272",    ==, sprintf_, buffer, "%+u", 4294966272U);
#ifdef PRINTF_SUPPORT_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("4294966272",    ==, sprintf_, buffer, "%+I32u", (uint32_t) 4294966272U);
#endif
  PRINTING_CHECK("777",           ==, sprintf_, buffer, "%+o", 511);
  PRINTING_CHECK("37777777001",   ==, sprintf_, buffer, "%+o", 4294966785U);
  PRINTING_CHECK("1234abcd",      ==, sprintf_, buffer, "%+x", 305441741);
  PRINTING_CHECK("edcb5433",      ==, sprintf_, buffer, "%+x", 3989525555U);
  PRINTING_CHECK("1234ABCD",      ==, sprintf_, buffer, "%+X", 305441741);
  PRINTING_CHECK("EDCB5433",      ==, sprintf_, buffer, "%+X", 3989525555U);
  PRINTING_CHECK("x",             ==, sprintf_, buffer, "%+c", 'x');
}
#endif


TEST_CASE("0 flag", "[]" ) {
  char buffer[100];
  PRINTING_CHECK("42",              ==, sprintf_, buffer, "%0d", 42);
  PRINTING_CHECK("42",              ==, sprintf_, buffer, "%0ld", 42L);
  PRINTING_CHECK("-42",             ==, sprintf_, buffer, "%0d", -42);
  PRINTING_CHECK("00042",           ==, sprintf_, buffer, "%05d", 42);
  PRINTING_CHECK("-0042",           ==, sprintf_, buffer, "%05d", -42);
  PRINTING_CHECK("000000000000042", ==, sprintf_, buffer, "%015d", 42);
  PRINTING_CHECK("-00000000000042", ==, sprintf_, buffer, "%015d", -42);
#if PRINTF_SUPPORT_DECIMAL_SPECIFIERS
  PRINTING_CHECK("000000000042.12", ==, sprintf_, buffer, "%015.2f", 42.1234);
  PRINTING_CHECK("00000000042.988", ==, sprintf_, buffer, "%015.3f", 42.9876);
  PRINTING_CHECK("-00000042.98760", ==, sprintf_, buffer, "%015.5f", -42.9876);
#endif
}


TEST_CASE("- flag", "[]" ) {
  char buffer[100];
  PRINTING_CHECK("42",              ==, sprintf_, buffer, "%-d", 42);
  PRINTING_CHECK("-42",             ==, sprintf_, buffer, "%-d", -42);
  PRINTING_CHECK("42   ",           ==, sprintf_, buffer, "%-5d", 42);
  PRINTING_CHECK("-42  ",           ==, sprintf_, buffer, "%-5d", -42);
  PRINTING_CHECK("42             ", ==, sprintf_, buffer, "%-15d", 42);
  PRINTING_CHECK("-42            ", ==, sprintf_, buffer, "%-15d", -42);
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS
TEST_CASE("- flag and non-standard 0 modifier for integers", "[]" ) {
  char buffer[100];
  PRINTING_CHECK("42",              ==, sprintf_, buffer, "%-0d", 42);
  PRINTING_CHECK("-42",             ==, sprintf_, buffer, "%-0d", -42);
  PRINTING_CHECK("42   ",           ==, sprintf_, buffer, "%-05d", 42);
  PRINTING_CHECK("-42  ",           ==, sprintf_, buffer, "%-05d", -42);
  PRINTING_CHECK("42             ", ==, sprintf_, buffer, "%-015d", 42);
  PRINTING_CHECK("-42            ", ==, sprintf_, buffer, "%-015d", -42);
  PRINTING_CHECK("42",              ==, sprintf_, buffer, "%0-d", 42);
  PRINTING_CHECK("-42",             ==, sprintf_, buffer, "%0-d", -42);
  PRINTING_CHECK("42   ",           ==, sprintf_, buffer, "%0-5d", 42);
  PRINTING_CHECK("-42  ",           ==, sprintf_, buffer, "%0-5d", -42);
  PRINTING_CHECK("42             ", ==, sprintf_, buffer, "%0-15d", 42);
  PRINTING_CHECK("-42            ", ==, sprintf_, buffer, "%0-15d", -42);

#if PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS
  PRINTING_CHECK("-4.200e+01     ", ==, sprintf_, buffer, "%0-15.3e", -42.);
#else
  PRINTING_CHECK("e",               ==, sprintf_, buffer, "%0-15.3e", -42.);
#endif

#if PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS
  PRINTING_CHECK("-42            ", ==, sprintf_, buffer,  "%0-15.3g", -42.);
#else
  PRINTING_CHECK("g",               ==, sprintf_, buffer,  "%0-15.3g", -42.);
#endif
}
#endif


TEST_CASE("# flag", "[]" ) {
  char buffer[100];
  PRINTING_CHECK("0",          ==, sprintf_, buffer, "%#o",          0 );
  PRINTING_CHECK("0",          ==, sprintf_, buffer, "%#0o",         0 );
  PRINTING_CHECK("0",          ==, sprintf_, buffer, "%#.0o",        0 );
  PRINTING_CHECK("0",          ==, sprintf_, buffer, "%#.1o",        0 );
  PRINTING_CHECK("   0",       ==, sprintf_, buffer, "%#4o",         0 );
  PRINTING_CHECK("0000",       ==, sprintf_, buffer, "%#.4o",        0 );
  PRINTING_CHECK("01",         ==, sprintf_, buffer, "%#o",          1 );
  PRINTING_CHECK("01",         ==, sprintf_, buffer, "%#0o",         1 );
  PRINTING_CHECK("01",         ==, sprintf_, buffer, "%#.0o",        1 );
  PRINTING_CHECK("01",         ==, sprintf_, buffer, "%#.1o",        1 );
  PRINTING_CHECK("  01",       ==, sprintf_, buffer, "%#4o",         1 );
  PRINTING_CHECK("0001",       ==, sprintf_, buffer, "%#.4o",        1 );
  PRINTING_CHECK("0x1001",     ==, sprintf_, buffer, "%#04x",   0x1001 );
  PRINTING_CHECK("01001",      ==, sprintf_, buffer, "%#04o",    01001 );
  PRINTING_CHECK("",           ==, sprintf_, buffer, "%#.0x",        0 );
  PRINTING_CHECK("0x0000614e", ==, sprintf_, buffer, "%#.8x",   0x614e );
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS
TEST_CASE("# flag - non-standard format", "[]" ) {
  char buffer[100];
  PRINTING_CHECK("0b110", ==, sprintf_, buffer, "%#b", 6);
}
#endif

#if PRINTF_SUPPORT_LONG_LONG
TEST_CASE("# flag with long-long", "[]" ) {
  char buffer[100];
  PRINTING_CHECK("0",          ==, sprintf_, buffer, "%#llo",   (long long)        0 );
  PRINTING_CHECK("0",          ==, sprintf_, buffer, "%#0llo",  (long long)        0 );
  PRINTING_CHECK("0",          ==, sprintf_, buffer, "%#.0llo", (long long)        0 );
  PRINTING_CHECK("0",          ==, sprintf_, buffer, "%#.1llo", (long long)        0 );
  PRINTING_CHECK("   0",       ==, sprintf_, buffer, "%#4llo",  (long long)        0 );
  PRINTING_CHECK("0000",       ==, sprintf_, buffer, "%#.4llo", (long long)        0 );
  PRINTING_CHECK("01",         ==, sprintf_, buffer, "%#llo",   (long long)        1 );
  PRINTING_CHECK("01",         ==, sprintf_, buffer, "%#0llo",  (long long)        1 );
  PRINTING_CHECK("01",         ==, sprintf_, buffer, "%#.0llo", (long long)        1 );
  PRINTING_CHECK("01",         ==, sprintf_, buffer, "%#.1llo", (long long)        1 );
  PRINTING_CHECK("  01",       ==, sprintf_, buffer, "%#4llo",  (long long)        1 );
  PRINTING_CHECK("0001",       ==, sprintf_, buffer, "%#.4llo", (long long)        1 );
  PRINTING_CHECK("0x1001",     ==, sprintf_, buffer, "%#04llx", (long long)   0x1001 );
  PRINTING_CHECK("01001",      ==, sprintf_, buffer, "%#04llo", (long long)    01001 );
  PRINTING_CHECK("",           ==, sprintf_, buffer, "%#.0llx", (long long)        0 );
  PRINTING_CHECK("0x0000614e", ==, sprintf_, buffer, "%#.8llx", (long long)   0x614e );
}


#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS
TEST_CASE("# flag with long-long - non-standard format", "[]" ) {
  char buffer[100];
  PRINTING_CHECK("0b110", ==, sprintf_, buffer, "%#llb", (long long) 6);
}
#endif
#endif // PRINTF_SUPPORT_LONG_LONG

TEST_CASE("specifier", "[]" ) {
  char buffer[100];
  PRINTING_CHECK("Hello testing", ==, sprintf_, buffer, "Hello testing");
  PRINTING_CHECK("Hello testing", ==, sprintf_, buffer, "%s", "Hello testing");

DISABLE_WARNING_PUSH
DISABLE_WARNING_PRINTF_FORMAT_OVERFLOW
  PRINTING_CHECK( "(null)", ==, sprintf_, buffer, "%s", (const char*) nullptr);
DISABLE_WARNING_POP
  PRINTING_CHECK("1024",        ==, sprintf_, buffer, "%d", 1024);
#if INT_MAX >= 2147483647LL
  PRINTING_CHECK("2147483647",  ==, sprintf_, buffer, "%d", 2147483647);
  PRINTING_CHECK("4294966272",  ==, sprintf_, buffer, "%u", 4294966272U);
  PRINTING_CHECK("37777777001", ==, sprintf_, buffer, "%o", 4294966785U);
  PRINTING_CHECK("1234abcd",    ==, sprintf_, buffer, "%x", 305441741);
  PRINTING_CHECK("edcb5433",    ==, sprintf_, buffer, "%x", 3989525555U);
  PRINTING_CHECK("1234ABCD",    ==, sprintf_, buffer, "%X", 305441741);
  PRINTING_CHECK("EDCB5433",    ==, sprintf_, buffer, "%X", 3989525555U);
#endif
  PRINTING_CHECK("-1024",       ==, sprintf_, buffer, "%d", -1024);
  PRINTING_CHECK("1024",        ==, sprintf_, buffer, "%i", 1024);
  PRINTING_CHECK("-1024",       ==, sprintf_, buffer, "%i", -1024);
  PRINTING_CHECK("1024",        ==, sprintf_, buffer, "%u", 1024);
  PRINTING_CHECK("777",         ==, sprintf_, buffer, "%o", 511);
  PRINTING_CHECK("%",           ==, sprintf_, buffer, "%%");

#ifdef PRINTF_SUPPORT_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("127",         ==, sprintf_, buffer, "%I8d", (int8_t) 127LL);
#if (SHRT_MAX >= 32767)
  PRINTING_CHECK("32767",  ==, sprintf_, buffer, "%I16d", (int16_t) 32767LL);
#endif
#if (LLONG_MAX >= 2147483647)
  PRINTING_CHECK("2147483647",  ==, sprintf_, buffer, "%I32d", (int32_t)  2147483647LL);
#if (LLONG_MAX >= 9223372036854775807LL)
  PRINTING_CHECK("9223372036854775807",  ==, sprintf_, buffer, "%I64d", (int64_t) 9223372036854775807LL);
#endif
#endif
#endif // PRINTF_SUPPORT_MSVC_STYLE_INTEGER_SPECIFIERS
}


TEST_CASE("width", "[]" ) {
  char buffer[100];
  PRINTING_CHECK("Hello testing", ==, sprintf_, buffer, "%1s", "Hello testing");
  PRINTING_CHECK("1024",          ==, sprintf_, buffer, "%1d", 1024);
  PRINTING_CHECK("-1024",         ==, sprintf_, buffer, "%1d", -1024);
  PRINTING_CHECK("1024",          ==, sprintf_, buffer, "%1i", 1024);
  PRINTING_CHECK("-1024",         ==, sprintf_, buffer, "%1i", -1024);
  PRINTING_CHECK("1024",          ==, sprintf_, buffer, "%1u", 1024);
#ifdef PRINTF_SUPPORT_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("1024",          ==, sprintf_, buffer, "%1I16u", (uint16_t) 1024);
  PRINTING_CHECK("1024",          ==, sprintf_, buffer, "%1I32u", (uint32_t) 1024);
  PRINTING_CHECK("1024",          ==, sprintf_, buffer, "%1I64u", (uint64_t) 1024);
#endif
  PRINTING_CHECK("4294966272",    ==, sprintf_, buffer, "%1u", 4294966272U);
#ifdef PRINTF_SUPPORT_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("4294966272",    ==, sprintf_, buffer, "%1I32u", (uint32_t) 4294966272U);
  PRINTING_CHECK("4294966272",    ==, sprintf_, buffer, "%1I64u", (uint64_t) 4294966272U);
#endif
  PRINTING_CHECK("777",           ==, sprintf_, buffer, "%1o", 511);
  PRINTING_CHECK("37777777001",   ==, sprintf_, buffer, "%1o", 4294966785U);
  PRINTING_CHECK("1234abcd",      ==, sprintf_, buffer, "%1x", 305441741);
  PRINTING_CHECK("edcb5433",      ==, sprintf_, buffer, "%1x", 3989525555U);
  PRINTING_CHECK("1234ABCD",      ==, sprintf_, buffer, "%1X", 305441741);
  PRINTING_CHECK("EDCB5433",      ==, sprintf_, buffer, "%1X", 3989525555U);
  PRINTING_CHECK("x",             ==, sprintf_, buffer, "%1c", 'x');
}


TEST_CASE("width 20", "[]" ) {
  char buffer[100];
  PRINTING_CHECK("               Hello", ==, sprintf_, buffer, "%20s",   "Hello");
  PRINTING_CHECK("                1024", ==, sprintf_, buffer, "%20d",   1024);
  PRINTING_CHECK("               -1024", ==, sprintf_, buffer, "%20d",   -1024);
  PRINTING_CHECK("                1024", ==, sprintf_, buffer, "%20i",   1024);
  PRINTING_CHECK("               -1024", ==, sprintf_, buffer, "%20i",   -1024);
  PRINTING_CHECK("                   0", ==, sprintf_, buffer, "%20i",   0);
  PRINTING_CHECK("                1024", ==, sprintf_, buffer, "%20u",   1024);
#ifdef PRINTF_SUPPORT_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("                1024", ==, sprintf_, buffer, "%20I16u", (uint16_t) 1024);
  PRINTING_CHECK("                1024", ==, sprintf_, buffer, "%20I32u", (uint32_t) 1024);
  PRINTING_CHECK("                1024", ==, sprintf_, buffer, "%20I64u", (uint64_t) 1024);
#endif
  PRINTING_CHECK("          4294966272", ==, sprintf_, buffer, "%20u",   4294966272U);
#ifdef PRINTF_SUPPORT_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("          4294966272", ==, sprintf_, buffer, "%20I32u", (uint32_t) 4294966272U);
  PRINTING_CHECK("          4294966272", ==, sprintf_, buffer, "%20I64u", (uint64_t) 4294966272U);
#endif
  PRINTING_CHECK("                 777", ==, sprintf_, buffer, "%20o",   511);
  PRINTING_CHECK("         37777777001", ==, sprintf_, buffer, "%20o",   4294966785U);
  PRINTING_CHECK("            1234abcd", ==, sprintf_, buffer, "%20x",   305441741);
  PRINTING_CHECK("            edcb5433", ==, sprintf_, buffer, "%20x",   3989525555U);
  PRINTING_CHECK("            1234ABCD", ==, sprintf_, buffer, "%20X",   305441741);
  PRINTING_CHECK("            EDCB5433", ==, sprintf_, buffer, "%20X",   3989525555U);
  PRINTING_CHECK("                   0", ==, sprintf_, buffer, "%20X",   0);
  PRINTING_CHECK("                   0", ==, sprintf_, buffer, "%20X",   0U);
#if PRINTF_SUPPORT_LONG_LONG
  PRINTING_CHECK("                   0", ==, sprintf_, buffer, "%20llX", 0ULL);
#endif
  PRINTING_CHECK("                   x", ==, sprintf_, buffer, "%20c",   'x');
}


TEST_CASE("width *20", "[]" ) {
  char buffer[100];
  PRINTING_CHECK("               Hello", ==, sprintf_, buffer, "%*s", 20, "Hello");
  PRINTING_CHECK("                1024", ==, sprintf_, buffer, "%*d", 20, 1024);
  PRINTING_CHECK("               -1024", ==, sprintf_, buffer, "%*d", 20, -1024);
  PRINTING_CHECK("                1024", ==, sprintf_, buffer, "%*i", 20, 1024);
  PRINTING_CHECK("               -1024", ==, sprintf_, buffer, "%*i", 20, -1024);
  PRINTING_CHECK("                1024", ==, sprintf_, buffer, "%*u", 20, 1024);
#ifdef PRINTF_SUPPORT_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("                1024", ==, sprintf_, buffer, "%*I16u", 20, (uint16_t) 1024);
  PRINTING_CHECK("                1024", ==, sprintf_, buffer, "%*I32u", 20, (uint32_t) 1024);
  PRINTING_CHECK("                1024", ==, sprintf_, buffer, "%*I64u", 20, (uint64_t) 1024);
#endif
  PRINTING_CHECK("          4294966272", ==, sprintf_, buffer, "%*u", 20, 4294966272U);
#ifdef PRINTF_SUPPORT_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("          4294966272", ==, sprintf_, buffer, "%*I32u", 20, (uint32_t) 4294966272U);
  PRINTING_CHECK("          4294966272", ==, sprintf_, buffer, "%*I64u", 20, (uint64_t) 4294966272U);
#endif
  PRINTING_CHECK("                 777", ==, sprintf_, buffer, "%*o", 20, 511);
  PRINTING_CHECK("         37777777001", ==, sprintf_, buffer, "%*o", 20, 4294966785U);
  PRINTING_CHECK("            1234abcd", ==, sprintf_, buffer, "%*x", 20, 305441741);
  PRINTING_CHECK("            edcb5433", ==, sprintf_, buffer, "%*x", 20, 3989525555U);
  PRINTING_CHECK("            1234ABCD", ==, sprintf_, buffer, "%*X", 20, 305441741);
  PRINTING_CHECK("            EDCB5433", ==, sprintf_, buffer, "%*X", 20, 3989525555U);
  PRINTING_CHECK("                   x", ==, sprintf_, buffer, "%*c", 20,'x');
}


TEST_CASE("width -20", "[]" ) {
  char buffer[100];
  PRINTING_CHECK("Hello               ", ==, sprintf_, buffer, "%-20s", "Hello");
  PRINTING_CHECK("1024                ", ==, sprintf_, buffer, "%-20d", 1024);
  PRINTING_CHECK("-1024               ", ==, sprintf_, buffer, "%-20d", -1024);
  PRINTING_CHECK("1024                ", ==, sprintf_, buffer, "%-20i", 1024);
  PRINTING_CHECK("-1024               ", ==, sprintf_, buffer, "%-20i", -1024);
  PRINTING_CHECK("1024                ", ==, sprintf_, buffer, "%-20u", 1024);
#if PRINTF_SUPPORT_DECIMAL_SPECIFIERS
  PRINTING_CHECK("1024.1234           ", ==, sprintf_, buffer, "%-20.4f", 1024.1234);
#endif
  PRINTING_CHECK("4294966272          ", ==, sprintf_, buffer, "%-20u", 4294966272U);
  PRINTING_CHECK("777                 ", ==, sprintf_, buffer, "%-20o", 511);
  PRINTING_CHECK("37777777001         ", ==, sprintf_, buffer, "%-20o", 4294966785U);
  PRINTING_CHECK("1234abcd            ", ==, sprintf_, buffer, "%-20x", 305441741);
  PRINTING_CHECK("edcb5433            ", ==, sprintf_, buffer, "%-20x", 3989525555U);
  PRINTING_CHECK("1234ABCD            ", ==, sprintf_, buffer, "%-20X", 305441741);
  PRINTING_CHECK("EDCB5433            ", ==, sprintf_, buffer, "%-20X", 3989525555U);
  PRINTING_CHECK("x                   ", ==, sprintf_, buffer, "%-20c", 'x');
  PRINTING_CHECK("|    9| |9 | |    9|", ==, sprintf_, buffer, "|%5d| |%-2d| |%5d|", 9, 9, 9);
  PRINTING_CHECK("|   10| |10| |   10|", ==, sprintf_, buffer, "|%5d| |%-2d| |%5d|", 10, 10, 10);
  PRINTING_CHECK("|    9| |9           | |    9|", ==, sprintf_, buffer, "|%5d| |%-12d| |%5d|", 9, 9, 9);
  PRINTING_CHECK("|   10| |10          | |   10|", ==, sprintf_, buffer, "|%5d| |%-12d| |%5d|", 10, 10, 10);
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS
TEST_CASE("width 0-20", "[]" ) {
  char buffer[100];
  PRINTING_CHECK("Hello               ", ==, sprintf_, buffer, "%0-20s", "Hello");
  PRINTING_CHECK("1024                ", ==, sprintf_, buffer, "%0-20d", 1024);
  PRINTING_CHECK("-1024               ", ==, sprintf_, buffer, "%0-20d", -1024);
  PRINTING_CHECK("1024                ", ==, sprintf_, buffer, "%0-20i", 1024);
  PRINTING_CHECK("-1024               ", ==, sprintf_, buffer, "%0-20i", -1024);
  PRINTING_CHECK("1024                ", ==, sprintf_, buffer, "%0-20u", 1024);
  PRINTING_CHECK("4294966272          ", ==, sprintf_, buffer, "%0-20u", 4294966272U);
  PRINTING_CHECK("777                 ", ==, sprintf_, buffer, "%0-20o", 511);
  PRINTING_CHECK("37777777001         ", ==, sprintf_, buffer, "%0-20o", 4294966785U);
  PRINTING_CHECK("1234abcd            ", ==, sprintf_, buffer, "%0-20x", 305441741);
  PRINTING_CHECK("edcb5433            ", ==, sprintf_, buffer, "%0-20x", 3989525555U);
  PRINTING_CHECK("1234ABCD            ", ==, sprintf_, buffer, "%0-20X", 305441741);
  PRINTING_CHECK("EDCB5433            ", ==, sprintf_, buffer, "%0-20X", 3989525555U);
  PRINTING_CHECK("x                   ", ==, sprintf_, buffer, "%0-20c", 'x');
}
#endif

TEST_CASE("padding 20", "[]" ) {
  char buffer[100];
  PRINTING_CHECK("00000000000000001024", ==, sprintf_, buffer, "%020d", 1024);
  PRINTING_CHECK("-0000000000000001024", ==, sprintf_, buffer, "%020d", -1024);
  PRINTING_CHECK("00000000000000001024", ==, sprintf_, buffer, "%020i", 1024);
  PRINTING_CHECK("-0000000000000001024", ==, sprintf_, buffer, "%020i", -1024);
  PRINTING_CHECK("00000000000000001024", ==, sprintf_, buffer, "%020u", 1024);
  PRINTING_CHECK("00000000004294966272", ==, sprintf_, buffer, "%020u", 4294966272U);
  PRINTING_CHECK("00000000000000000777", ==, sprintf_, buffer, "%020o", 511);
  PRINTING_CHECK("00000000037777777001", ==, sprintf_, buffer, "%020o", 4294966785U);
  PRINTING_CHECK("0000000000001234abcd", ==, sprintf_, buffer, "%020x", 305441741);
  PRINTING_CHECK("000000000000edcb5433", ==, sprintf_, buffer, "%020x", 3989525555U);
  PRINTING_CHECK("0000000000001234ABCD", ==, sprintf_, buffer, "%020X", 305441741);
  PRINTING_CHECK("000000000000EDCB5433", ==, sprintf_, buffer, "%020X", 3989525555U);
}


TEST_CASE("padding .20", "[]" ) {
  char buffer[100];
  PRINTING_CHECK("00000000000000001024",  ==, sprintf_, buffer, "%.20d", 1024);
  PRINTING_CHECK("-00000000000000001024", ==, sprintf_, buffer, "%.20d", -1024);
  PRINTING_CHECK("00000000000000001024",  ==, sprintf_, buffer, "%.20i", 1024);
  PRINTING_CHECK("-00000000000000001024", ==, sprintf_, buffer, "%.20i", -1024);
  PRINTING_CHECK("00000000000000001024",  ==, sprintf_, buffer, "%.20u", 1024);
  PRINTING_CHECK("00000000004294966272",  ==, sprintf_, buffer, "%.20u", 4294966272U);
  PRINTING_CHECK("00000000000000000777",  ==, sprintf_, buffer, "%.20o", 511);
  PRINTING_CHECK("00000000037777777001",  ==, sprintf_, buffer, "%.20o", 4294966785U);
  PRINTING_CHECK("0000000000001234abcd",  ==, sprintf_, buffer, "%.20x", 305441741);
  PRINTING_CHECK("000000000000edcb5433",  ==, sprintf_, buffer, "%.20x", 3989525555U);
  PRINTING_CHECK("0000000000001234ABCD",  ==, sprintf_, buffer, "%.20X", 305441741);
  PRINTING_CHECK("000000000000EDCB5433",  ==, sprintf_, buffer, "%.20X", 3989525555U);
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS
TEST_CASE("padding #020 - non-standard format", "[]" ) {
  char buffer[100];
  PRINTING_CHECK("00000000000000001024", ==, sprintf_, buffer, "%#020d", 1024);
  PRINTING_CHECK("-0000000000000001024", ==, sprintf_, buffer, "%#020d", -1024);
  PRINTING_CHECK("00000000000000001024", ==, sprintf_, buffer, "%#020i", 1024);
  PRINTING_CHECK("-0000000000000001024", ==, sprintf_, buffer, "%#020i", -1024);
  PRINTING_CHECK("00000000000000001024", ==, sprintf_, buffer, "%#020u", 1024);
  PRINTING_CHECK("00000000004294966272", ==, sprintf_, buffer, "%#020u", 4294966272U);
}
#endif

TEST_CASE("padding #020", "[]" ) {
  char buffer[100];
  PRINTING_CHECK("00000000000000000777", ==, sprintf_, buffer, "%#020o", 511);
  PRINTING_CHECK("00000000037777777001", ==, sprintf_, buffer, "%#020o", 4294966785U);
  PRINTING_CHECK("0x00000000001234abcd", ==, sprintf_, buffer, "%#020x", 305441741);
  PRINTING_CHECK("0x0000000000edcb5433", ==, sprintf_, buffer, "%#020x", 3989525555U);
  PRINTING_CHECK("0X00000000001234ABCD", ==, sprintf_, buffer, "%#020X", 305441741);
  PRINTING_CHECK("0X0000000000EDCB5433", ==, sprintf_, buffer, "%#020X", 3989525555U);
}


#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS
TEST_CASE("padding #20 - non-standard format", "[]" ) {
  char buffer[100];
  PRINTING_CHECK("                1024", ==, sprintf_, buffer, "%#20d", 1024);
  PRINTING_CHECK("               -1024", ==, sprintf_, buffer, "%#20d", -1024);
  PRINTING_CHECK("                1024", ==, sprintf_, buffer, "%#20i", 1024);
  PRINTING_CHECK("               -1024", ==, sprintf_, buffer, "%#20i", -1024);
  PRINTING_CHECK("                1024", ==, sprintf_, buffer, "%#20u", 1024);
  PRINTING_CHECK("          4294966272", ==, sprintf_, buffer, "%#20u", 4294966272U);
}
#endif

TEST_CASE("padding #20", "[]" ) {
  char buffer[100];
  PRINTING_CHECK("                0777", ==, sprintf_, buffer, "%#20o", 511);
  PRINTING_CHECK("        037777777001", ==, sprintf_, buffer, "%#20o", 4294966785U);
  PRINTING_CHECK("          0x1234abcd", ==, sprintf_, buffer, "%#20x", 305441741);
  PRINTING_CHECK("          0xedcb5433", ==, sprintf_, buffer, "%#20x", 3989525555U);
  PRINTING_CHECK("          0X1234ABCD", ==, sprintf_, buffer, "%#20X", 305441741);
  PRINTING_CHECK("          0XEDCB5433", ==, sprintf_, buffer, "%#20X", 3989525555U);
}


TEST_CASE("padding 20.5", "[]" ) {
  char buffer[100];
  PRINTING_CHECK("               01024", ==, sprintf_, buffer, "%20.5d", 1024);
  PRINTING_CHECK("              -01024", ==, sprintf_, buffer, "%20.5d", -1024);
  PRINTING_CHECK("               01024", ==, sprintf_, buffer, "%20.5i", 1024);
  PRINTING_CHECK("              -01024", ==, sprintf_, buffer, "%20.5i", -1024);
  PRINTING_CHECK("               01024", ==, sprintf_, buffer, "%20.5u", 1024);
  PRINTING_CHECK("          4294966272", ==, sprintf_, buffer, "%20.5u", 4294966272U);
  PRINTING_CHECK("               00777", ==, sprintf_, buffer, "%20.5o", 511);
  PRINTING_CHECK("         37777777001", ==, sprintf_, buffer, "%20.5o", 4294966785U);
  PRINTING_CHECK("            1234abcd", ==, sprintf_, buffer, "%20.5x", 305441741);
  PRINTING_CHECK("          00edcb5433", ==, sprintf_, buffer, "%20.10x", 3989525555U);
  PRINTING_CHECK("            1234ABCD", ==, sprintf_, buffer, "%20.5X", 305441741);
  PRINTING_CHECK("          00EDCB5433", ==, sprintf_, buffer, "%20.10X", 3989525555U);
}


TEST_CASE("padding neg numbers", "[]" ) {
  char buffer[100];

  // space padding
  PRINTING_CHECK("-5",   ==, sprintf_, buffer, "% 1d", -5);
  PRINTING_CHECK("-5",   ==, sprintf_, buffer, "% 2d", -5);
  PRINTING_CHECK(" -5",  ==, sprintf_, buffer, "% 3d", -5);
  PRINTING_CHECK("  -5", ==, sprintf_, buffer, "% 4d", -5);

  // zero padding
  PRINTING_CHECK("-5",   ==, sprintf_, buffer, "%01d", -5);
  PRINTING_CHECK("-5",   ==, sprintf_, buffer, "%02d", -5);
  PRINTING_CHECK("-05",  ==, sprintf_, buffer, "%03d", -5);
  PRINTING_CHECK("-005", ==, sprintf_, buffer, "%04d", -5);
}


#if PRINTF_SUPPORT_DECIMAL_SPECIFIERS || PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS
TEST_CASE("float padding neg numbers", "[]" ) {
  char buffer[100];

  // space padding
#if PRINTF_SUPPORT_DECIMAL_SPECIFIERS
  PRINTING_CHECK("-5.0",       ==, sprintf_, buffer, "% 3.1f", -5.);
  PRINTING_CHECK("-5.0",       ==, sprintf_, buffer, "% 4.1f", -5.);
  PRINTING_CHECK(" -5.0",      ==, sprintf_, buffer, "% 5.1f", -5.);
#endif

#if PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS
  PRINTING_CHECK("    -5",     ==, sprintf_, buffer, "% 6.1g", -5.);
  PRINTING_CHECK("-5.0e+00",   ==, sprintf_, buffer, "% 6.1e", -5.);
  PRINTING_CHECK("  -5.0e+00", ==, sprintf_, buffer, "% 10.1e", -5.);
#endif

  // zero padding
#if PRINTF_SUPPORT_DECIMAL_SPECIFIERS
  PRINTING_CHECK("-5.0",       ==, sprintf_, buffer, "%03.1f", -5.);
  PRINTING_CHECK("-5.0",       ==, sprintf_, buffer, "%04.1f", -5.);
  PRINTING_CHECK("-05.0",      ==, sprintf_, buffer, "%05.1f", -5.);

  // zero padding no decimal point
  PRINTING_CHECK("-5",         ==, sprintf_, buffer, "%01.0f", -5.);
  PRINTING_CHECK("-5",         ==, sprintf_, buffer, "%02.0f", -5.);
  PRINTING_CHECK("-05",        ==, sprintf_, buffer, "%03.0f", -5.);
#endif

#if PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS
  PRINTING_CHECK("-005.0e+00", ==, sprintf_, buffer, "%010.1e", -5.);
  PRINTING_CHECK("-05E+00",    ==, sprintf_, buffer, "%07.0E", -5.);
  PRINTING_CHECK("-05",        ==, sprintf_, buffer, "%03.0g", -5.);
#endif
}
#endif // PRINTF_SUPPORT_DECIMAL_SPECIFIERS || PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS

TEST_CASE("length", "[]" ) {
  char buffer[100];
  PRINTING_CHECK("",                     ==, sprintf_, buffer, "%.0s", "Hello testing");
  PRINTING_CHECK("                    ", ==, sprintf_, buffer, "%20.0s", "Hello testing");
  PRINTING_CHECK("",                     ==, sprintf_, buffer, "%.s", "Hello testing");
  PRINTING_CHECK("                    ", ==, sprintf_, buffer, "%20.s", "Hello testing");
  PRINTING_CHECK("                1024", ==, sprintf_, buffer, "%20.0d", 1024);
  PRINTING_CHECK("               -1024", ==, sprintf_, buffer, "%20.0d", -1024);
  PRINTING_CHECK("                    ", ==, sprintf_, buffer, "%20.d", 0);
  PRINTING_CHECK("                1024", ==, sprintf_, buffer, "%20.0i", 1024);
  PRINTING_CHECK("               -1024", ==, sprintf_, buffer, "%20.i", -1024);
  PRINTING_CHECK("                    ", ==, sprintf_, buffer, "%20.i", 0);
  PRINTING_CHECK("                1024", ==, sprintf_, buffer, "%20.u", 1024);
  PRINTING_CHECK("          4294966272", ==, sprintf_, buffer, "%20.0u", 4294966272U);
  PRINTING_CHECK("                    ", ==, sprintf_, buffer, "%20.u", 0U);
  PRINTING_CHECK("                 777", ==, sprintf_, buffer, "%20.o", 511);
  PRINTING_CHECK("         37777777001", ==, sprintf_, buffer, "%20.0o", 4294966785U);
  PRINTING_CHECK("                    ", ==, sprintf_, buffer, "%20.o", 0U);
  PRINTING_CHECK("            1234abcd", ==, sprintf_, buffer, "%20.x", 305441741);
  PRINTING_CHECK("                                          1234abcd", ==, sprintf_, buffer, "%50.x", 305441741);
  PRINTING_CHECK("                                          1234abcd     12345", ==, sprintf_, buffer, "%50.x%10.u", 305441741, 12345);
  PRINTING_CHECK("            edcb5433", ==, sprintf_, buffer, "%20.0x", 3989525555U);
  PRINTING_CHECK("                    ", ==, sprintf_, buffer, "%20.x", 0U);
  PRINTING_CHECK("            1234ABCD", ==, sprintf_, buffer, "%20.X", 305441741);
  PRINTING_CHECK("            EDCB5433", ==, sprintf_, buffer, "%20.0X", 3989525555U);
  PRINTING_CHECK("                    ", ==, sprintf_, buffer, "%20.X", 0U);
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS
TEST_CASE("length - non-standard format", "[]" ) {
  char buffer[100];
  PRINTING_CHECK("  ", ==, sprintf_, buffer, "%02.0u", 0U);
  PRINTING_CHECK("  ", ==, sprintf_, buffer, "%02.0d", 0);
}
#endif


TEST_CASE("infinity and not-a-number values", "[]" ) {
  char buffer[100];

  // test special-case floats using math.h macros
#if PRINTF_SUPPORT_DECIMAL_SPECIFIERS
  PRINTING_CHECK("     nan",  ==, sprintf_, buffer, "%8f", (double) NAN);
  PRINTING_CHECK("     inf",  ==, sprintf_, buffer, "%8f", (double) INFINITY);
  PRINTING_CHECK("-inf    ",  ==, sprintf_, buffer, "%-8f", (double) -INFINITY);
#endif
#if PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS
  PRINTING_CHECK("     nan",  ==, sprintf_, buffer, "%8e", (double) NAN);
  PRINTING_CHECK("     inf",  ==, sprintf_, buffer, "%8e", (double) INFINITY);
  PRINTING_CHECK("-inf    ",  ==, sprintf_, buffer, "%-8e", (double) -INFINITY);
#endif
}

#if PRINTF_SUPPORT_DECIMAL_SPECIFIERS
TEST_CASE("floating-point specifiers with 31-32 bit integer values", "[]" ) {
  char buffer[100];
#if PRINTF_MAX_INTEGRAL_DIGITS_FOR_DECIMAL >= 10
  PRINTING_CHECK("2147483647", ==, sprintf_, buffer, "%.10f", 2147483647.0); // 2^31 - 1
  PRINTING_CHECK("2147483648", ==, sprintf_, buffer, "%.10f", 2147483648.0); // 2^31
  PRINTING_CHECK("4294967295", ==, sprintf_, buffer, "%.10f", 4294967295.0); // 2^32 - 1
  PRINTING_CHECK("4294967296", ==, sprintf_, buffer, "%.10f", 4294967296.0); // 2^32
#else
  PRINTING_CHECK("2.1474836470e+09", ==, sprintf_, buffer, "%.10f", 2147483647.0); // 2^31 - 1
  PRINTING_CHECK("2.1474836480e+09", ==, sprintf_, buffer, "%.10f", 2147483648.0); // 2^31
  PRINTING_CHECK("4.2949672950e+09", ==, sprintf_, buffer, "%.10f", 4294967295.0); // 2^32 - 1
  PRINTING_CHECK("4.2949672960e+09", ==, sprintf_, buffer, "%.10f", 4294967296.0); // 2^32
#endif
  PRINTING_CHECK("2147483647", ==, sprintf_, buffer, "%.10g", 2147483647.0); // 2^31 - 1
  PRINTING_CHECK("2147483648", ==, sprintf_, buffer, "%.10g", 2147483648.0); // 2^31
  PRINTING_CHECK("4294967295", ==, sprintf_, buffer, "%.10g", 4294967295.0); // 2^32 - 1
  PRINTING_CHECK("4294967296", ==, sprintf_, buffer, "%.10g", 4294967296.0); // 2^32
}
#endif

TEST_CASE("tiny floating-point values", "[]" ) {
  char buffer[100];
//  boltzman_s_constant = 1.380651569e-23;

#if PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS
  PRINTING_CHECK("1e-23",                  ==, sprintf_, buffer, "%.0e",  1.380651569e-23);
  PRINTING_CHECK("1.4e-23",                ==, sprintf_, buffer, "%.1e",  1.380651569e-23);
  PRINTING_CHECK("1.38e-23",               ==, sprintf_, buffer, "%.2e",  1.380651569e-23);
  PRINTING_CHECK("1.381e-23",              ==, sprintf_, buffer, "%.3e",  1.380651569e-23);
  PRINTING_CHECK("1.3807e-23",             ==, sprintf_, buffer, "%.4e",  1.380651569e-23);
  PRINTING_CHECK("1.38065e-23",            ==, sprintf_, buffer, "%.5e",  1.380651569e-23);
  PRINTING_CHECK("1.380652e-23",           ==, sprintf_, buffer, "%.6e",  1.380651569e-23);
  PRINTING_CHECK("1.3806516e-23",          ==, sprintf_, buffer, "%.7e",  1.380651569e-23);
  PRINTING_CHECK("1.38065157e-23",         ==, sprintf_, buffer, "%.8e",  1.380651569e-23);
  PRINTING_CHECK("1.380651569e-23",        ==, sprintf_, buffer, "%.9e",  1.380651569e-23);
  PRINTING_CHECK("1.3806515690e-23",       ==, sprintf_, buffer, "%.10e", 1.380651569e-23);
  PRINTING_CHECK("1.38065156900e-23",      ==, sprintf_, buffer, "%.11e", 1.380651569e-23);
  PRINTING_CHECK("1.380651569000e-23",     ==, sprintf_, buffer, "%.12e", 1.380651569e-23);
  PRINTING_CHECK("1.3806515690000e-23",    ==, sprintf_, buffer, "%.13e", 1.380651569e-23);
  PRINTING_CHECK("1.38065156900000e-23",   ==, sprintf_, buffer, "%.14e", 1.380651569e-23);
  PRINTING_CHECK("1.380651569000000e-23",  ==, sprintf_, buffer, "%.15e", 1.380651569e-23);
  PRINTING_CHECK("1.3806515690000000e-23", ==, sprintf_, buffer, "%.16e", 1.380651569e-23);
#endif
}

#if PRINTF_SUPPORT_DECIMAL_SPECIFIERS
TEST_CASE("fallback from decimal to exponential", "[]" ) {
  char buffer[100];
  CAPTURE_AND_PRINT(sprintf_, buffer, "%.0f", (double) ((int64_t)1 * 1000 ) );
  if (PRINTF_MAX_INTEGRAL_DIGITS_FOR_DECIMAL < 3) {
    CHECK(!strcmp(buffer, "1e+3"));
  }
  else {

    CHECK(!strcmp(buffer, "1000"));
  }

  CAPTURE_AND_PRINT(sprintf_, buffer, "%.0f", (double) ((int64_t)1 * 1000 * 1000 ) );
  if (PRINTF_MAX_INTEGRAL_DIGITS_FOR_DECIMAL < 6) {
    CHECK(!strcmp(buffer, "1e+6"));
  }
  else {
    CHECK(!strcmp(buffer, "1000000"));
  }

  CAPTURE_AND_PRINT(sprintf_, buffer, "%.0f", (double) ((int64_t)1 * 1000 * 1000 * 1000 ) );
  if (PRINTF_MAX_INTEGRAL_DIGITS_FOR_DECIMAL < 9) {
    CHECK(!strcmp(buffer, "1e+9"));
  }
  else {
    CHECK(!strcmp(buffer, "1000000000"));
  }

  CAPTURE_AND_PRINT(sprintf_, buffer, "%.0f", (double) ((int64_t)1 * 1000 * 1000 * 1000 * 1000) );
  if (PRINTF_MAX_INTEGRAL_DIGITS_FOR_DECIMAL < 12) {
#if PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS
    CHECK(!strcmp(buffer, "1e+12"));
#else
    CHECK(!strcmp(buffer, ""));
#endif
  }
  else {
    CHECK(!strcmp(buffer, "1000000000000"));
  }

  CAPTURE_AND_PRINT(sprintf_, buffer, "%.0f", (double) ((int64_t)1 * 1000 * 1000 * 1000 * 1000 * 1000) );
  if (PRINTF_MAX_INTEGRAL_DIGITS_FOR_DECIMAL < 15) {
#if PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS
    CHECK(!strcmp(buffer, "1e+15"));
#else
    CHECK(!strcmp(buffer, ""));
#endif
  }
  else {
    CHECK(!strcmp(buffer, "1000000000000000"));
  }
#endif

  // A value which should Should definitely be out of range for float
#if PRINTF_SUPPORT_DECIMAL_SPECIFIERS
  CAPTURE_AND_PRINT(sprintf_, buffer, "%.1f", 1E20);
#if PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS
  CHECK(!strcmp(buffer, "1.0e+20"));
#else
  CHECK(!strcmp(buffer, ""));
#endif
#endif

}

TEST_CASE("floating-point specifiers, precision and flags", "[]" ) {
  char buffer[100];
#if PRINTF_SUPPORT_DECIMAL_SPECIFIERS
  PRINTING_CHECK("3.1415",    ==, sprintf_, buffer, "%.4f", 3.1415354);
  PRINTING_CHECK("30343.142", ==, sprintf_, buffer, "%.3f", 30343.1415354);
  PRINTING_CHECK("34",               ==, sprintf_, buffer, "%.0f", 34.1415354);
  PRINTING_CHECK("1",                ==, sprintf_, buffer, "%.0f", 1.3);
  PRINTING_CHECK("2",                ==, sprintf_, buffer, "%.0f", 1.55);
  PRINTING_CHECK("1.6",              ==, sprintf_, buffer, "%.1f", 1.64);
  PRINTING_CHECK("42.90",            ==, sprintf_, buffer, "%.2f", 42.8952);
  PRINTING_CHECK("42.895200000",     ==, sprintf_, buffer, "%.9f", 42.8952);
  PRINTING_CHECK("42.8952230000",    ==, sprintf_, buffer, "%.10f", 42.895223);
  PRINTING_CHECK("42.895223123457",  ==, sprintf_, buffer, "%.12f", 42.89522312345678);
  PRINTING_CHECK("42477.371093750000000", ==, sprintf_, buffer, "%020.15f", 42477.37109375);
  PRINTING_CHECK("42.895223876543",  ==, sprintf_, buffer, "%.12f", 42.89522387654321);
  PRINTING_CHECK(" 42.90",           ==, sprintf_, buffer, "%6.2f", 42.8952);
  PRINTING_CHECK("+42.90",           ==, sprintf_, buffer, "%+6.2f", 42.8952);
  PRINTING_CHECK("+42.9",            ==, sprintf_, buffer, "%+5.1f", 42.9252);
  PRINTING_CHECK("42.500000",        ==, sprintf_, buffer, "%f", 42.5);
  PRINTING_CHECK("42.5",             ==, sprintf_, buffer, "%.1f", 42.5);
  PRINTING_CHECK("42167.000000",     ==, sprintf_, buffer, "%f", 42167.0);
  PRINTING_CHECK("-12345.987654321", ==, sprintf_, buffer, "%.9f", -12345.987654321);
  PRINTING_CHECK("4.0",              ==, sprintf_, buffer, "%.1f", 3.999);
  PRINTING_CHECK("4",                ==, sprintf_, buffer, "%.0f", 3.5);
  PRINTING_CHECK("4",                ==, sprintf_, buffer, "%.0f", 4.5);
  PRINTING_CHECK("3",                ==, sprintf_, buffer, "%.0f", 3.49);
  PRINTING_CHECK("3.5",              ==, sprintf_, buffer, "%.1f", 3.49);
  PRINTING_CHECK("a0.5  ",           ==, sprintf_, buffer, "a%-5.1f", 0.5);
  PRINTING_CHECK("a0.5  end",        ==, sprintf_, buffer, "a%-5.1fend", 0.5);
#endif
#if PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS
  PRINTING_CHECK("0.5",              ==, sprintf_, buffer, "%.4g", 0.5);
  PRINTING_CHECK("1",                ==, sprintf_, buffer, "%.4g", 1.0);
  PRINTING_CHECK("12345.7",          ==, sprintf_, buffer, "%G", 12345.678);
  PRINTING_CHECK("12345.68",         ==, sprintf_, buffer, "%.7G", 12345.678);
  PRINTING_CHECK("1.2346E+08",       ==, sprintf_, buffer, "%.5G", 123456789.);
  PRINTING_CHECK("12345",            ==, sprintf_, buffer, "%.6G", 12345.);
  PRINTING_CHECK("  +1.235e+08",     ==, sprintf_, buffer, "%+12.4g", 123456789.);
  PRINTING_CHECK("0.0012",           ==, sprintf_, buffer, "%.2G", 0.001234);
  PRINTING_CHECK(" +0.001234",       ==, sprintf_, buffer, "%+10.4G", 0.001234);
  PRINTING_CHECK("+001.234e-05",     ==, sprintf_, buffer, "%+012.4g", 0.00001234);
  PRINTING_CHECK("-1.23e-308",       ==, sprintf_, buffer, "%.3g", -1.2345e-308);
  PRINTING_CHECK("+1.230E+308",      ==, sprintf_, buffer, "%+.3E", 1.23e+308);
  PRINTING_CHECK("1.000e+01",        ==, sprintf_, buffer, "%.3e", 9.9996);
  PRINTING_CHECK("0",                ==, sprintf_, buffer, "%g", 0.);
  PRINTING_CHECK("-0",               ==, sprintf_, buffer, "%g", -0.);
  PRINTING_CHECK("+0",               ==, sprintf_, buffer, "%+g", 0.);
  PRINTING_CHECK("-0",               ==, sprintf_, buffer, "%+g", -0.);
  PRINTING_CHECK("-4e+04",           ==, sprintf_, buffer, "%.1g", -40661.5);
  PRINTING_CHECK("-4.e+04",          ==, sprintf_, buffer, "%#.1g", -40661.5);
  PRINTING_CHECK("100.",             ==, sprintf_, buffer, "%#.3g", 99.998580932617187500);
  // Rounding-focused checks
  PRINTING_CHECK("4.895512e+04",     ==, sprintf_, buffer, "%e", 48955.125);
  PRINTING_CHECK("9.2524e+04",       ==, sprintf_, buffer, "%.4e", 92523.5);
  PRINTING_CHECK("-8.380923438e+04", ==, sprintf_, buffer, "%.9e", -83809.234375);
#endif


#if PRINTF_SUPPORT_DECIMAL_SPECIFIERS
  // brute force float
  bool any_failed = false;
  std::stringstream sstr;
  sstr.precision(5);
  for (float i = -100000; i < 100000; i += (float) 1) {
    sprintf_(buffer, "%.5f", (double) (i / 10000));
    sstr.str("");
    sstr << std::fixed << i / 10000;
    if (strcmp(buffer, sstr.str().c_str()) != 0) {
      std::cerr
        << ": sprintf_(\"%.5f\", " << std::setw(6) << i << ") = " << std::setw(10) << buffer   << " , "
        << "expected " << std::setw(10) << sstr.str().c_str() << "\n";
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
    sprintf_(buffer, "%.5f", (double) i);
    sstr.str("");
    sstr << i;
    if (strcmp(buffer, sstr.str().c_str()) != 0) {
      std::cerr
        << n << ": sprintf_(\"%.5f\", " << std::setw(18) << std::setprecision(30) << i << ") = " << std::setw(15) << buffer << " , "
        << "expected " << std::setw(12) << sstr.str().c_str() << "\n";
      any_failed = true;
    }
  }
  CHECK(not any_failed);
#endif
#endif
}


TEST_CASE("integer types", "[]" ) {
  char buffer[100];
  PRINTING_CHECK("0",                    ==, sprintf_, buffer, "%i", 0);
  PRINTING_CHECK("1234",                 ==, sprintf_, buffer, "%i", 1234);
  PRINTING_CHECK("32767",                ==, sprintf_, buffer, "%i", 32767);
  PRINTING_CHECK("-32767",               ==, sprintf_, buffer, "%i", -32767);
  PRINTING_CHECK("30",                   ==, sprintf_, buffer, "%li", 30L);
  PRINTING_CHECK("-2147483647",          ==, sprintf_, buffer, "%li", -2147483647L);
  PRINTING_CHECK("2147483647",           ==, sprintf_, buffer, "%li", 2147483647L);
#if PRINTF_SUPPORT_LONG_LONG
  PRINTING_CHECK("30",                   ==, sprintf_, buffer, "%lli", 30LL);
  PRINTING_CHECK("-9223372036854775807", ==, sprintf_, buffer, "%lli", -9223372036854775807LL);
  PRINTING_CHECK("9223372036854775807",  ==, sprintf_, buffer, "%lli", 9223372036854775807LL);
#endif
  PRINTING_CHECK("100000",               ==, sprintf_, buffer, "%lu", 100000L);
  PRINTING_CHECK("4294967295",           ==, sprintf_, buffer, "%lu", 0xFFFFFFFFL);
#if PRINTF_SUPPORT_LONG_LONG
  PRINTING_CHECK("281474976710656",      ==, sprintf_, buffer, "%llu", 281474976710656LLU);
  PRINTING_CHECK("18446744073709551615", ==, sprintf_, buffer, "%llu", 18446744073709551615LLU);
#endif
  PRINTING_CHECK("2147483647",           ==, sprintf_, buffer, "%zu", (size_t)2147483647UL);
  PRINTING_CHECK("2147483647",           ==, sprintf_, buffer, "%zd", (size_t)2147483647UL);
  PRINTING_CHECK("-2147483647",          ==, sprintf_, buffer, "%zi", (ssize_t)-2147483647L);
  PRINTING_CHECK("165140",               ==, sprintf_, buffer, "%o", 60000);
  PRINTING_CHECK("57060516",             ==, sprintf_, buffer, "%lo", 12345678L);
  PRINTING_CHECK("12345678",             ==, sprintf_, buffer, "%lx", 0x12345678L);
#if PRINTF_SUPPORT_LONG_LONG
  PRINTING_CHECK("1234567891234567",     ==, sprintf_, buffer, "%llx", 0x1234567891234567LLU);
#endif
  PRINTING_CHECK("abcdefab",             ==, sprintf_, buffer, "%lx", 0xabcdefabL);
  PRINTING_CHECK("ABCDEFAB",             ==, sprintf_, buffer, "%lX", 0xabcdefabL);
  PRINTING_CHECK("v",                    ==, sprintf_, buffer, "%c", 'v');
  PRINTING_CHECK("wv",                   ==, sprintf_, buffer, "%cv", 'w');
  PRINTING_CHECK("A Test",               ==, sprintf_, buffer, "%s", "A Test");
  PRINTING_CHECK("255",                  ==, sprintf_, buffer, "%hhu", (unsigned char) 0xFFU);
  PRINTING_CHECK("4660",                 ==, sprintf_, buffer, "%hu", (unsigned short) 0x1234u);
  PRINTING_CHECK("Test100 65535",        ==, sprintf_, buffer, "%s%hhi %hu", "Test", (char) 100, (unsigned short) 0xFFFF);
  PRINTING_CHECK("a",                    ==, sprintf_, buffer, "%tx", &buffer[10] - &buffer[0]);
  PRINTING_CHECK("-2147483647",          ==, sprintf_, buffer, "%ji", (intmax_t)-2147483647L);
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS
TEST_CASE("types - non-standard format", "[]" ) {
  char buffer[100];
  PRINTING_CHECK("1110101001100000", ==, sprintf_, buffer, "%b", 60000);
  PRINTING_CHECK("101111000110000101001110", ==, sprintf_, buffer, "%lb", 12345678L);
}
#endif

TEST_CASE("pointer", "[]" ) {
  char buffer[100];

  CAPTURE_AND_PRINT(sprintf_, buffer, "%p", (void*)0x1234U);
  if (sizeof(void*) == 4U) {
    CHECK(!strcmp(buffer, "0x00001234"));
  }
  else {
    CHECK(!strcmp(buffer, "0x0000000000001234"));
  }

  CAPTURE_AND_PRINT(sprintf_, buffer, "%p", (void*)0x12345678U);
  if (sizeof(void*) == 4U) {
    CHECK(!strcmp(buffer, "0x12345678"));
  }
  else {
    CHECK(!strcmp(buffer, "0x0000000012345678"));
  }

  CAPTURE_AND_PRINT(sprintf_, buffer, "%p-%p", (void*)0x12345678U, (void*)0x7EDCBA98U);
  if (sizeof(void*) == 4U) {
    CHECK(!strcmp(buffer, "0x12345678-0x7edcba98"));
  }
  else {
    CHECK(!strcmp(buffer, "0x0000000012345678-0x000000007edcba98"));
  }

  if (sizeof(uintptr_t) == sizeof(uint64_t)) {
    sprintf_(buffer, "%p", (void*)(uintptr_t)0xFFFFFFFFU);
    CHECK(!strcmp(buffer, "0x00000000ffffffff"));
  }
  else {
    sprintf_(buffer, "%p", (void*)(uintptr_t)0xFFFFFFFFU);
    CHECK(!strcmp(buffer, "0xffffffff"));
  }
  PRINTING_CHECK("(nil)", ==, sprintf_, buffer, "%p", (const void*) NULL);
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS
TEST_CASE("unknown flag (non-standard format)", "[]" ) {
  char buffer[100];
  PRINTING_CHECK("kmarco", ==, sprintf_, buffer, "%kmarco", 42, 37);
}
#endif

TEST_CASE("string length", "[]" ) {
  char buffer[100];
  PRINTING_CHECK("This", ==, sprintf_, buffer, "%.4s", "This is a test");
  PRINTING_CHECK("test", ==, sprintf_, buffer, "%.4s", "test");
  PRINTING_CHECK("123", ==, sprintf_, buffer, "%.7s", "123");
  PRINTING_CHECK("", ==, sprintf_, buffer, "%.7s", "");
  PRINTING_CHECK("1234ab", ==, sprintf_, buffer, "%.4s%.2s", "123456", "abcdef");
  PRINTING_CHECK("123", ==, sprintf_, buffer, "%.*s", 3, "123456");

DISABLE_WARNING_PUSH
DISABLE_WARNING_PRINTF_FORMAT_OVERFLOW
  PRINTING_CHECK("(null)", ==, sprintf_, buffer, "%.*s", 3, (const char*) NULL);
DISABLE_WARNING_POP
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS
TEST_CASE("string length (non-standard format)", "[]" ) {
  char buffer[100];
DISABLE_WARNING_PUSH
DISABLE_WARNING_PRINTF_FORMAT
DISABLE_WARNING_PRINTF_FORMAT_EXTRA_ARGS
DISABLE_WARNING_PRINTF_FORMAT_INVALID_SPECIFIER
  PRINTING_CHECK(".2s", ==, sprintf_, buffer, "%.4.2s", "123456");
DISABLE_WARNING_POP
}
#endif


TEST_CASE("buffer length", "[]" ) {
  char buffer[100];
  int ret;

  ret = snprintf_(nullptr, 10, "%s", "Test");
  CHECK(ret == 4);
  ret = snprintf_(nullptr, 0, "%s", "Test");
  CHECK(ret == 4);

  buffer[0] = (char)0xA5;
  ret = snprintf_(buffer, 0, "%s", "Test");
  CHECK(buffer[0] == (char)0xA5);
  CHECK(ret == 4);

  buffer[0] = (char)0xCC;
  snprintf_(buffer, 1, "%s", "Test");
  CHECK(buffer[0] == '\0');
  PRINTING_CHECK("H", ==, snprintf_, buffer, 2, "%s", "Hello");

DISABLE_WARNING_PUSH
DISABLE_WARNING_PRINTF_FORMAT_OVERFLOW
  snprintf_(buffer, 2, "%s", (const char*) NULL);
  CHECK(!strcmp(buffer, "("));
DISABLE_WARNING_POP
}


TEST_CASE("ret value", "[]" ) {
  char buffer[100] ;
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
  ret = snprintf_(buffer, 6, "0%s", (const char*) NULL);
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


TEST_CASE("misc", "[]" ) {
  char buffer[100];
  PRINTING_CHECK("53000atest-20 bit",    ==, sprintf_, buffer, "%u%u%ctest%d %s", 5, 3000, 'a', -20, "bit");
#if PRINTF_SUPPORT_DECIMAL_SPECIFIERS
  PRINTING_CHECK("0.33",                 ==, sprintf_, buffer, "%.*f", 2, 0.33333333);
  PRINTING_CHECK("1",                    ==, sprintf_, buffer, "%.*d", -1, 1);
  PRINTING_CHECK("foo",                  ==, sprintf_, buffer, "%.3s", "foobar");
  PRINTING_CHECK(" ",                    ==, sprintf_, buffer, "% .0d", 0);
  PRINTING_CHECK("     00004",           ==, sprintf_, buffer, "%10.5d", 4);
  PRINTING_CHECK("hi x",                 ==, sprintf_, buffer, "%*sx", -3, "hi");
  PRINTING_CHECK("00123               ", ==, sprintf_, buffer, "%-20.5i", 123);
  PRINTING_CHECK("-67224.546875000000000000", ==, sprintf_, buffer, "%.18f", -67224.546875);
#endif
#if PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS
  PRINTING_CHECK("0.33",              ==, sprintf_, buffer, "%.*g", 2, 0.33333333);
  PRINTING_CHECK("3.33e-01",          ==, sprintf_, buffer, "%.*e", 2, 0.33333333);
  PRINTING_CHECK("0.000000e+00",      ==, sprintf_, buffer, "%e", 0.0);
  PRINTING_CHECK("-0.000000e+00",     ==, sprintf_, buffer, "%e", -0.0);
#endif
}

TEST_CASE("extremal signed integer values", "[]" ) {
  char buffer[100];
  char expected[100];

  std::sprintf(expected, "%hhd", std::numeric_limits<char>::max());
  PRINTING_CHECK(expected, ==, sprintf_, buffer, "%hhd", std::numeric_limits<char>::max());

  std::sprintf(expected, "%hd", std::numeric_limits<short int>::max());
  PRINTING_CHECK(expected, ==, sprintf_, buffer, "%hd", std::numeric_limits<short int>::max());

  std::sprintf(expected, "%hd", std::numeric_limits<short int>::max());
  PRINTING_CHECK(expected, ==, sprintf_, buffer, "%hd", std::numeric_limits<short int>::max());

  std::sprintf(expected, "%d", std::numeric_limits<int>::min());
  PRINTING_CHECK(expected, ==, sprintf_, buffer, "%d", std::numeric_limits<int>::min());

  std::sprintf(expected, "%d", std::numeric_limits<int>::max());
  PRINTING_CHECK(expected, ==, sprintf_, buffer, "%d", std::numeric_limits<int>::max());

  std::sprintf(expected, "%ld", std::numeric_limits<long int>::min());
  PRINTING_CHECK(expected, ==, sprintf_, buffer, "%ld", std::numeric_limits<long int>::min());

  std::sprintf(expected, "%ld", std::numeric_limits<long int>::max());
  PRINTING_CHECK(expected, ==, sprintf_, buffer, "%ld", std::numeric_limits<long int>::max());

#if PRINTF_SUPPORT_LONG_LONG
  std::sprintf(expected, "%lld", std::numeric_limits<long long int>::min());
  PRINTING_CHECK(expected, ==, sprintf_, buffer, "%lld", std::numeric_limits<long long int>::min());

  std::sprintf(expected, "%lld", std::numeric_limits<long long int>::max());
  PRINTING_CHECK(expected, ==, sprintf_, buffer, "%lld", std::numeric_limits<long long int>::max());
#endif
}

TEST_CASE("extremal unsigned integer values", "[]" ) {
  char buffer[100];
  char expected[100];

  std::sprintf(expected, "%hhu", std::numeric_limits<char unsigned>::max());
  PRINTING_CHECK(expected, ==, sprintf_, buffer, "%hhu", std::numeric_limits<char unsigned>::max());

  std::sprintf(expected, "%hu", std::numeric_limits<short unsigned>::max());
  PRINTING_CHECK(expected, ==, sprintf_, buffer, "%hu", std::numeric_limits<short unsigned>::max());

  std::sprintf(expected, "%u", std::numeric_limits<unsigned>::max());
  PRINTING_CHECK(expected, ==, sprintf_, buffer, "%u", std::numeric_limits<unsigned>::max());

  std::sprintf(expected, "%lu", std::numeric_limits<long unsigned>::max());
  PRINTING_CHECK(expected, ==, sprintf_, buffer, "%lu", std::numeric_limits<long unsigned>::max());

#if PRINTF_SUPPORT_LONG_LONG
  std::sprintf(expected, "%llu", std::numeric_limits<long long unsigned>::max());
  PRINTING_CHECK(expected, ==, sprintf_, buffer, "%llu", std::numeric_limits<long long unsigned>::max());
#endif
}

TEST_CASE("writeback specifier", "[]" ) {
  char buffer[100];

  struct {
    char char_;
    short short_;
    int int_;
    long long_;
    long long long_long_;
  } num_written;


  num_written.int_ = 1234;
  printf_("%n", &num_written.int_);
  CHECK(num_written.int_ == 0);
  num_written.int_ = 1234;
  printf_("foo%nbar", &num_written.int_);
  CHECK(num_written.int_ == 3);

  num_written.int_ = 1234;
  PRINTING_CHECK("", ==, sprintf_, buffer, "%n", &num_written.int_);
  CHECK(num_written.int_ == 0);
  num_written.int_ = 1234;
  PRINTING_CHECK("foobar", ==, sprintf_, buffer, "foo%nbar", &num_written.int_);
  CHECK(num_written.int_ == 3);
}


#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS
DISABLE_WARNING_POP
#endif

