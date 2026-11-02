// printf unit tests: the API functions, their return values and the buffer handling

#include "test_common.hpp"

#include <cstdarg>


namespace {

int call_vprintf(const char* format, ...)
{
  va_list va;
  va_start(va, format);
  const int ret = vprintf_(format, va);
  va_end(va);
  return ret;
}

int call_vsnprintf(char* buffer, std::size_t count, const char* format, ...)
{
  va_list va;
  va_start(va, format);
  const int ret = vsnprintf_(buffer, count, format, va);
  va_end(va);
  return ret;
}

int call_vfctprintf(const char* format, ...)
{
  va_list va;
  va_start(va, format);
  const int ret = vfctprintf(&test_out_fct, nullptr, format, va);
  va_end(va);
  return ret;
}

} // namespace


TEST_CASE("printf", "[api]")
{
  test_output_reset();
  CHECK(printf_("% d", 4232) == 5);
  CHECK(test_output_str() == " 4232");
  CHECK(test_output[5] == static_cast<char>(0xCC));
}


TEST_CASE("vprintf", "[api]")
{
  test_output_reset();
  CHECK(call_vprintf("%d", 2345) == 4);
  CHECK(test_output_str() == "2345");
  CHECK(test_output[4] == static_cast<char>(0xCC));
}


TEST_CASE("fctprintf", "[api]")
{
  test_output_reset();
  CHECK(fctprintf(&test_out_fct, nullptr, "This is a test of %X", 0x12EFU) == 22);
  CHECK(test_output_str() == "This is a test of 12EF");
  CHECK(test_output[22] == static_cast<char>(0xCC));
}


TEST_CASE("vfctprintf", "[api]")
{
  test_output_reset();
  CHECK(call_vfctprintf("This is a test of %X", 0x12EFU) == 22);
  CHECK(test_output_str() == "This is a test of 12EF");
  CHECK(test_output[22] == static_cast<char>(0xCC));
}


TEST_CASE("sprintf", "[api]")
{
  CHECK_SPRINTF("-1",                                "%d",          -1);
  CHECK_SPRINTF("3 -1000 test",                      "%d %d %s",    3, -1000, "test");
}


TEST_CASE("snprintf", "[api]")
{
  CHECK_SNPRINTF("-1000",                             5,   50U, "%d",          -1000);
  CHECK_SNPRINTF("-1",                                5,   3U,  "%d",          -1000);
}


TEST_CASE("vsnprintf", "[api]")
{
  char buffer[test_buffer_size];

  CHECK(call_vsnprintf(buffer, sizeof(buffer), "%d", -1) == 2);
  CHECK(std::string(buffer) == "-1");

  CHECK(call_vsnprintf(buffer, sizeof(buffer), "%d %d %s", 3, -1000, "test") == 12);
  CHECK(std::string(buffer) == "3 -1000 test");

  CHECK(call_vsnprintf(buffer, 3U, "%d", -1000) == 5);
  CHECK(std::string(buffer) == "-1");
}


// the return value is the length of the untruncated output
TEST_CASE("return value", "[api]")
{
  CHECK_SNPRINTF("01234",                             5,   6U,  "0%s",         "1234");
  CHECK_SNPRINTF("01234",                             6,   6U,  "0%s",         "12345");
  CHECK_SNPRINTF("01234",                             8,   6U,  "0%s",         "1234567");
  CHECK_SNPRINTF("hello, wo",                         12,  10U, "hello, world");
  CHECK_SNPRINTF("10",                                5,   3U,  "%d",          10000);

  DISABLE_WARNING_PUSH
  DISABLE_WARNING_FORMAT_OVERFLOW
  CHECK_SNPRINTF("0(nul",                             7,   6U,  "0%s",         (const char*)NULL);
  DISABLE_WARNING_POP
}


TEST_CASE("buffer length", "[api]")
{
  // a NULL buffer or a buffer size of 0 only counts the output
  CHECK(snprintf_(nullptr, 10U, "%s", "Test") == 4);
  CHECK(snprintf_(nullptr, 0U,  "%s", "Test") == 4);
  CHECK_SNPRINTF("",                                  4,   0U,  "%s",          "Test");

  // a buffer size of 1 only holds the terminating null
  CHECK_SNPRINTF("",                                  4,   1U,  "%s",          "Test");
  CHECK_SNPRINTF("H",                                 5,   2U,  "%s",          "Hello");

  DISABLE_WARNING_PUSH
  DISABLE_WARNING_FORMAT_OVERFLOW
  CHECK_SNPRINTF("(",                                 6,   2U,  "%s",          (const char*)NULL);
  DISABLE_WARNING_POP
}


// #47: a '\0' printed by %c is output like any other character
TEST_CASE("null character", "[api]")
{
  test_output_reset();
  CHECK(printf_("X%cX", '\0') == 3);
  CHECK(test_output_len == 3U);
  CHECK(std::memcmp(test_output, "X\0X", 3U) == 0);

  test_output_reset();
  CHECK(fctprintf(&test_out_fct, nullptr, "X%cX", '\0') == 3);
  CHECK(test_output_len == 3U);
  CHECK(std::memcmp(test_output, "X\0X", 3U) == 0);
}
