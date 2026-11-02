// printf unit tests: the conversions %s %c %p and %%

#include "test_common.hpp"


TEST_CASE("string conversion", "[strings]")
{
  CHECK_SPRINTF("Hello testing",                     "Hello testing");
  CHECK_SPRINTF("Hello testing",                     "%s",          "Hello testing");
  CHECK_SPRINTF("A Test",                            "%s",          "A Test");
  CHECK_SPRINTF("{",                                 "%s",          "{");

  // #63: a NULL pointer is printed like glibc does
  DISABLE_WARNING_PUSH
  DISABLE_WARNING_FORMAT_OVERFLOW
  CHECK_SPRINTF("(null)",                            "%s",          (const char*)NULL);
  CHECK_SPRINTF("  (null)",                          "%8s",         (const char*)NULL);
  CHECK_SPRINTF("(null)",                            "%.*s",        3, (const char*)NULL);
  DISABLE_WARNING_POP
}


// the precision is the maximum number of characters
TEST_CASE("string precision", "[strings]")
{
  CHECK_SPRINTF("This",                              "%.4s",        "This is a test");
  CHECK_SPRINTF("test",                              "%.4s",        "test");
  CHECK_SPRINTF("foo",                               "%.3s",        "foobar");
  CHECK_SPRINTF("123",                               "%.7s",        "123");
  CHECK_SPRINTF("",                                  "%.7s",        "");
  CHECK_SPRINTF("1234ab",                            "%.4s%.2s",    "123456", "abcdef");
  CHECK_SPRINTF("123",                               "%.*s",        3, "123456");
}


TEST_CASE("string precision with non-standard format", "[strings][non-standard]")
{
  DISABLE_WARNING_PUSH
  DISABLE_WARNING_NON_STANDARD_FORMAT
  CHECK_SPRINTF(".2s",                               "%.4.2s",      "123456");
  DISABLE_WARNING_POP
}


TEST_CASE("char conversion", "[strings]")
{
  CHECK_SPRINTF("v",                                 "%c",          'v');
  CHECK_SPRINTF("wv",                                "%cv",         'w');
}


TEST_CASE("percent", "[strings]")
{
  CHECK_SPRINTF("%",                                 "%%");
}


// %p prints like glibc: a '0x' prefix and lowercase hex digits, zero padded
// to the width or to all digits of a pointer, and "(nil)" for a NULL pointer
TEST_CASE("pointer conversion", "[strings]")
{
  if (sizeof(void*) == 4U) {
    CHECK_SPRINTF("0x00001234",                        "%p",          (void*)0x1234U);
    CHECK_SPRINTF("0x12345678",                        "%p",          (void*)0x12345678U);
    CHECK_SPRINTF("0xffffffff",                        "%p",          (void*)(uintptr_t)0xFFFFFFFFU);
    CHECK_SPRINTF("0x12345678-0x7edcba98",             "%p-%p",       (void*)0x12345678U, (void*)0x7EDCBA98U);
  }
  else {
    CHECK_SPRINTF("0x0000000000001234",                "%p",          (void*)0x1234U);
    CHECK_SPRINTF("0x0000000012345678",                "%p",          (void*)0x12345678U);
    CHECK_SPRINTF("0x00000000ffffffff",                "%p",          (void*)(uintptr_t)0xFFFFFFFFU);
    CHECK_SPRINTF("0x0000000012345678-0x000000007edcba98",
                                                     "%p-%p",       (void*)0x12345678U, (void*)0x7EDCBA98U);
  }
  CHECK_SPRINTF("0x0ff",                             "%3p",         (const void*)0xff);
  CHECK_SPRINTF("(nil)",                             "%p",          (const void*)NULL);
  CHECK_SPRINTF("   (nil)",                          "%8p",         (const void*)NULL);
}


// an unknown conversion is printed as is, the argument is skipped
TEST_CASE("unknown conversion", "[strings][non-standard]")
{
  DISABLE_WARNING_PUSH
  DISABLE_WARNING_NON_STANDARD_FORMAT
  CHECK_SPRINTF("kmarco",                            "%kmarco",     42, 37);
  DISABLE_WARNING_POP
}


TEST_CASE("mixed conversions", "[strings]")
{
  CHECK_SPRINTF("53000atest-20 bit",                 "%u%u%ctest%d %s", 5, 3000, 'a', -20, "bit");
}


TEST_CASE("percent with conversions", "[strings]")
{
  CHECK_SPRINTF("%d",                                "%%d");
  CHECK_SPRINTF("%%",                                "%%%%");
  CHECK_SPRINTF("100%",                              "%d%%",        100);
  CHECK_SPRINTF("%42%",                              "%%%d%%",      42);
}


// the width is the number of digits, the '0x' prefix comes on top
TEST_CASE("pointer conversion with width", "[strings]")
{
  CHECK_SPRINTF("0x00000000000000001234",            "%20p",        (void*)0x1234U);
  CHECK_SPRINTF("0x1234                ",            "%-20p",       (void*)0x1234U);
  CHECK_SPRINTF("(nil)   ",                          "%-8p",        (const void*)NULL);
}


// the unsupported conversions %a, %A and %n are printed like an unknown conversion
TEST_CASE("unsupported conversions", "[strings][non-standard]")
{
  int n = 0;

  DISABLE_WARNING_PUSH
  DISABLE_WARNING_NON_STANDARD_FORMAT
  CHECK_SPRINTF("a",                                 "%a",          1.0);
  CHECK_SPRINTF("A",                                 "%A",          1.0);
  CHECK_SPRINTF("n",                                 "%n",          &n);
  DISABLE_WARNING_POP
  CHECK(n == 0);
}


TEST_CASE("empty format", "[strings][non-standard]")
{
  DISABLE_WARNING_PUSH
  DISABLE_WARNING_NON_STANDARD_FORMAT
  CHECK_SPRINTF("",                                  "");
  DISABLE_WARNING_POP
}


// an incomplete format specifier at the end of the format string ends the output
TEST_CASE("incomplete format specifier", "[strings][non-standard]")
{
  DISABLE_WARNING_PUSH
  DISABLE_WARNING_NON_STANDARD_FORMAT
  CHECK_SPRINTF("abc",                               "abc%");
  CHECK_SPRINTF("abc",                               "abc%5");
  CHECK_SPRINTF("abc",                               "abc%-05.3l");
  CHECK_SPRINTF("",                                  "%");
  CHECK_SPRINTF("1 ",                                "%d %",        1);
  DISABLE_WARNING_POP
}
