// printf.c is compiled with PRINTF_DISABLE_SUPPORT_PUTCHAR for this test and
// _putchar() is not defined here, so it only links if nothing references it

#include <stdarg.h>
#include "../printf.h"

static int strcmp_(const char* lhs, const char* rhs)
{
  while (1) {
    char lhs_c = *lhs++;
    char rhs_c = *rhs++;
    if (lhs_c != rhs_c) { return lhs_c - rhs_c; }
    if (lhs_c == '\0') { return 0; }
  }
}

enum { BufferSize = 100 };
static char buffer[BufferSize];
static int buffer_idx;

static void out_fct(char character, void* arg)
{
  char* buf = (char*)arg;
  if (buffer_idx < BufferSize - 1) {
    buf[buffer_idx++] = character;
    buf[buffer_idx] = '\0';
  }
}

static int call_vsnprintf(char* buf, size_t count, const char* format, ...)
{
  va_list va;
  va_start(va, format);
  const int ret = vsnprintf_(buf, count, format, va);
  va_end(va);
  return ret;
}

static int call_vfctprintf(const char* format, ...)
{
  va_list va;
  va_start(va, format);
  const int ret = vfctprintf(&out_fct, buffer, format, va);
  va_end(va);
  return ret;
}

int main(void)
{
  if ((sprintf_(buffer, "%d %s", 12, "abc") != 6) || strcmp_(buffer, "12 abc")) {
    return -1;
  }
  if ((snprintf_(buffer, 5U, "%d", 123456) != 6) || strcmp_(buffer, "1234")) {
    return -1;
  }
  if ((call_vsnprintf(buffer, BufferSize, "%x-%c", 0xBEEFU, 'z') != 6) || strcmp_(buffer, "beef-z")) {
    return -1;
  }
  buffer_idx = 0;
  if ((fctprintf(&out_fct, buffer, "%X", 0x12EFU) != 4) || strcmp_(buffer, "12EF")) {
    return -1;
  }
  buffer_idx = 0;
  if ((call_vfctprintf("%05d", -42) != 5) || strcmp_(buffer, "-0042")) {
    return -1;
  }
  return 0;
}
