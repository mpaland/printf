// printf unit tests: the integer conversions and the length modifiers

#include "test_common.hpp"

#include <cstdio>
#include <limits>


TEST_CASE("integer conversions", "[integers]")
{
  CHECK_SPRINTF("1024",                              "%d",          1024);
  CHECK_SPRINTF("-1024",                             "%d",          -1024);
  CHECK_SPRINTF("1024",                              "%i",          1024);
  CHECK_SPRINTF("-1024",                             "%i",          -1024);
  CHECK_SPRINTF("1024",                              "%u",          1024);
  CHECK_SPRINTF("777",                               "%o",          511);
  CHECK_SPRINTF("165140",                            "%o",          60000);
#if INT_MAX >= 2147483647LL
  CHECK_SPRINTF("2147483647",                        "%d",          2147483647);
  CHECK_SPRINTF("4294966272",                        "%u",          4294966272U);
  CHECK_SPRINTF("37777777001",                       "%o",          4294966785U);
  CHECK_SPRINTF("1234abcd",                          "%x",          305441741);
  CHECK_SPRINTF("edcb5433",                          "%x",          3989525555U);
  CHECK_SPRINTF("1234ABCD",                          "%X",          305441741);
  CHECK_SPRINTF("EDCB5433",                          "%X",          3989525555U);
#endif
}


TEST_CASE("binary conversion", "[integers][non-standard]")
{
  DISABLE_WARNING_PUSH
  DISABLE_WARNING_NON_STANDARD_FORMAT
  CHECK_SPRINTF("1110101001100000",                  "%b",          60000);
  CHECK_SPRINTF("101111000110000101001110",          "%lb",         12345678L);
  DISABLE_WARNING_POP
}


TEST_CASE("length modifiers", "[integers]")
{
  char array[16];

  CHECK_SPRINTF("0",                                 "%i",          0);
  CHECK_SPRINTF("1234",                              "%i",          1234);
  CHECK_SPRINTF("32767",                             "%i",          32767);
  CHECK_SPRINTF("-32767",                            "%i",          -32767);
  CHECK_SPRINTF("255",                               "%hhu",        (unsigned char)0xFFU);
  CHECK_SPRINTF("4660",                              "%hu",         (unsigned short)0x1234u);
  CHECK_SPRINTF("Test100 65535",                     "%s%hhi %hu",  "Test", (char)100, (unsigned short)0xFFFF);
  CHECK_SPRINTF("30",                                "%li",         30L);
  CHECK_SPRINTF("-2147483647",                       "%li",         -2147483647L);
  CHECK_SPRINTF("2147483647",                        "%li",         2147483647L);
  CHECK_SPRINTF("100000",                            "%lu",         100000L);
  CHECK_SPRINTF("4294967295",                        "%lu",         0xFFFFFFFFL);
  CHECK_SPRINTF("57060516",                          "%lo",         12345678L);
  CHECK_SPRINTF("12345678",                          "%lx",         0x12345678L);
  CHECK_SPRINTF("abcdefab",                          "%lx",         0xabcdefabL);
  CHECK_SPRINTF("ABCDEFAB",                          "%lX",         0xabcdefabL);
#if TEST_LONG_LONG
  CHECK_SPRINTF("30",                                "%lli",        30LL);
  CHECK_SPRINTF("-9223372036854775807",              "%lli",        -9223372036854775807LL);
  CHECK_SPRINTF("9223372036854775807",               "%lli",        9223372036854775807LL);
  CHECK_SPRINTF("281474976710656",                   "%llu",        281474976710656LLU);
  CHECK_SPRINTF("18446744073709551615",              "%llu",        18446744073709551615LLU);
  CHECK_SPRINTF("1234567891234567",                  "%llx",        0x1234567891234567LLU);
#endif
#if TEST_LONG_LONG || (SIZE_MAX == ULONG_MAX)
  CHECK_SPRINTF("2147483647",                        "%zu",         (size_t)2147483647UL);
  CHECK_SPRINTF("2147483647",                        "%zd",         (size_t)2147483647UL);
  CHECK_SPRINTF("-2147483647",                       "%zi",         (ptrdiff_t)-2147483647L);
#endif
#if TEST_LONG_LONG || (INTMAX_MAX == LONG_MAX)
  CHECK_SPRINTF("-2147483647",                       "%ji",         (intmax_t)-2147483647L);
#endif
#if TEST_PTRDIFF_T && (TEST_LONG_LONG || (PTRDIFF_MAX == LONG_MAX))
  CHECK_SPRINTF("a",                                 "%tx",         &array[10] - &array[0]);
#endif
  (void)array;
}


// the expected values come from the standard library, so that the width of
// the types does not matter
TEST_CASE("extremal signed values", "[integers]")
{
  char expected[test_buffer_size];

  std::snprintf(expected, sizeof(expected), "%hhd", std::numeric_limits<signed char>::min());
  CHECK_SPRINTF(expected,                            "%hhd",        std::numeric_limits<signed char>::min());
  std::snprintf(expected, sizeof(expected), "%hhd", std::numeric_limits<signed char>::max());
  CHECK_SPRINTF(expected,                            "%hhd",        std::numeric_limits<signed char>::max());
  std::snprintf(expected, sizeof(expected), "%hd", std::numeric_limits<short>::min());
  CHECK_SPRINTF(expected,                            "%hd",         std::numeric_limits<short>::min());
  std::snprintf(expected, sizeof(expected), "%hd", std::numeric_limits<short>::max());
  CHECK_SPRINTF(expected,                            "%hd",         std::numeric_limits<short>::max());

  // #118: no signed integer overflow for the most negative values
  std::snprintf(expected, sizeof(expected), "%d", std::numeric_limits<int>::min());
  CHECK_SPRINTF(expected,                            "%d",          std::numeric_limits<int>::min());
  std::snprintf(expected, sizeof(expected), "%d", std::numeric_limits<int>::max());
  CHECK_SPRINTF(expected,                            "%d",          std::numeric_limits<int>::max());
  std::snprintf(expected, sizeof(expected), "%ld", std::numeric_limits<long>::min());
  CHECK_SPRINTF(expected,                            "%ld",         std::numeric_limits<long>::min());
  std::snprintf(expected, sizeof(expected), "%ld", std::numeric_limits<long>::max());
  CHECK_SPRINTF(expected,                            "%ld",         std::numeric_limits<long>::max());
#if TEST_LONG_LONG
  std::snprintf(expected, sizeof(expected), "%lld", std::numeric_limits<long long>::min());
  CHECK_SPRINTF(expected,                            "%lld",        std::numeric_limits<long long>::min());
  std::snprintf(expected, sizeof(expected), "%lld", std::numeric_limits<long long>::max());
  CHECK_SPRINTF(expected,                            "%lld",        std::numeric_limits<long long>::max());
#endif
}


TEST_CASE("extremal unsigned values", "[integers]")
{
  char expected[test_buffer_size];

  std::snprintf(expected, sizeof(expected), "%hhu", std::numeric_limits<unsigned char>::max());
  CHECK_SPRINTF(expected,                            "%hhu",        std::numeric_limits<unsigned char>::max());
  std::snprintf(expected, sizeof(expected), "%hu", std::numeric_limits<unsigned short>::max());
  CHECK_SPRINTF(expected,                            "%hu",         std::numeric_limits<unsigned short>::max());
  std::snprintf(expected, sizeof(expected), "%u", std::numeric_limits<unsigned>::max());
  CHECK_SPRINTF(expected,                            "%u",          std::numeric_limits<unsigned>::max());
  std::snprintf(expected, sizeof(expected), "%lu", std::numeric_limits<unsigned long>::max());
  CHECK_SPRINTF(expected,                            "%lu",         std::numeric_limits<unsigned long>::max());
#if TEST_LONG_LONG
  std::snprintf(expected, sizeof(expected), "%llu", std::numeric_limits<unsigned long long>::max());
  CHECK_SPRINTF(expected,                            "%llu",        std::numeric_limits<unsigned long long>::max());
#endif
}


// the argument is converted to the type of the length modifier
TEST_CASE("length modifier wrap-around", "[integers]")
{
  CHECK_SPRINTF("0",                                 "%hhd",        256);
  CHECK_SPRINTF("-1",                                "%hhd",        255);
  CHECK_SPRINTF("127",                               "%hhd",        -129);
  CHECK_SPRINTF("ff",                                "%hhx",        -1);
  CHECK_SPRINTF("255",                               "%hhu",        -1);
  CHECK_SPRINTF("0",                                 "%hu",         65536);
  CHECK_SPRINTF("-1",                                "%hd",         65535);
  CHECK_SPRINTF("2345",                              "%hx",         0x12345);
  CHECK_SPRINTF("ffff",                              "%hx",         -1);
}


TEST_CASE("length modifiers j, z and t", "[integers]")
{
  char array[16];

#if TEST_LONG_LONG || (INTMAX_MAX == LONG_MAX)
  CHECK_SPRINTF("123",                               "%ju",         (uintmax_t)123);
  CHECK_SPRINTF("-123",                              "%jd",         (intmax_t)-123);
  CHECK_SPRINTF("7b",                                "%jx",         (uintmax_t)123);
#endif
#if TEST_LONG_LONG || (SIZE_MAX == ULONG_MAX)
  CHECK_SPRINTF("beef",                              "%zx",         (size_t)0xbeef);
  CHECK_SPRINTF("BEEF",                              "%zX",         (size_t)0xbeef);
  CHECK_SPRINTF("17",                                "%zo",         (size_t)15);
#endif
#if TEST_PTRDIFF_T && (TEST_LONG_LONG || (PTRDIFF_MAX == LONG_MAX))
  CHECK_SPRINTF("-5",                                "%td",         (ptrdiff_t)-5);
  CHECK_SPRINTF("10",                                "%td",         &array[10] - &array[0]);
  CHECK_SPRINTF("-10",                               "%ti",         &array[0] - &array[10]);
#endif
  (void)array;
}
