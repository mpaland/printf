#include <printf_config.h>
#include <printf/printf.h>

int strcmp_(const char* lhs, const char* rhs)
{
  while (1) {
    char lhs_c = *lhs++;
    char rhs_c = *rhs++;
    if (lhs_c != rhs_c) { return lhs_c - rhs_c; }
    if (lhs_c == '\0') { return 0; }
  }
}

enum { BufferSize = 100 };
char buffer[BufferSize];

void putchar_(char c)
{
  for(char* ptr = buffer; ptr - buffer < BufferSize; ptr++) {
    if (*ptr == '\0') {
      *ptr++ = c;
      *ptr++ = '\0';
      return;
    }
  }
}

void clear_buffer()
{
  for(int i = 0; i < BufferSize; i++) {
    buffer[i] = '\0';
  }
}

int main()
{
#if PRINTF_ALIAS_STANDARD_FUNCTION_NAMES
  clear_buffer();
  printf("printf'ing an integer: %d and a string: %s\n", 12, "Hello world");
  const char* expected = "printf'ing an integer: 12 and a string: Hello world\n";
  if (strcmp_(buffer, expected) != 0) {
    return(-1);
  }
  clear_buffer();
  sprintf(buffer, "sprintf'ing an integer: %d and a string: %s\n", 34, "quick brown fox");
  expected = "sprintf'ing an integer: 34 and a string: quick brown fox\n";
  if (strcmp_(buffer, expected) != 0) {
    return(-1);
  }
#endif
  return 0;
}
