// output capture of printf_(), vprintf_() and fctprintf() for the unit tests

#include "test_common.hpp"

char        test_output[test_buffer_size];
std::size_t test_output_len = 0U;


// the _putchar() of this test executable, appends to test_output
extern "C" void _putchar(char character)
{
  if (test_output_len < test_buffer_size) {
    test_output[test_output_len++] = character;
  }
}


void test_out_fct(char character, void* arg)
{
  (void)arg;
  _putchar(character);
}


void test_output_reset()
{
  std::memset(test_output, 0xCC, sizeof(test_output));
  test_output_len = 0U;
}


std::string test_output_str()
{
  return std::string(test_output, test_output_len);
}
