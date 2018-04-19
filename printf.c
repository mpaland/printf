///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2018, PALANDesign Hannover, Germany
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
// \brief Tiny printf, sprintf and (v)snprintf implementation, optimized for speed on
//        embedded systems with a very limited resources. These routines are thread
//        safe and reentrant!
//        Use this instead of the bloated standard/newlib printf cause these use
//        malloc for printf (and may not be thread safe).
//
///////////////////////////////////////////////////////////////////////////////

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "printf.h"


// buffer size used omly for printf (created on stack)
#define PRINTF_BUFFER_SIZE        128U

// ntoa conversion buffer size, this must be big enough to hold
// one converted numeric number including padded zeros (created on stack)
#define PRINTF_NTOA_BUFFER_SIZE    32U

// ftoa conversion buffer size, this must be big enough to hold
// one converted float number including padded zeros (created on stack)
#define PRINTF_FTOA_BUFFER_SIZE    32U

// define this to support floating point (%f)
#define PRINTF_FLOAT_SUPPORT

// define this to support long long types (%llu or %p)
#define PRINTF_LONG_LONG_SUPPORT

///////////////////////////////////////////////////////////////////////////////

// internal flag definitions
#define FLAGS_ZEROPAD   (1U << 0U)
#define FLAGS_LEFT      (1U << 1U)
#define FLAGS_PLUS      (1U << 2U)
#define FLAGS_SPACE     (1U << 3U)
#define FLAGS_HASH      (1U << 4U)
#define FLAGS_UPPERCASE (1U << 5U)
#define FLAGS_LONG      (1U << 6U)
#define FLAGS_LONG_LONG (1U << 7U)
#define FLAGS_PRECISION (1U << 8U)
#define FLAGS_WIDTH     (1U << 9U)


// internal strlen
// \return The length of the string (excluding the terminating 0)
static inline size_t _strlen(const char* str)
{
  const char* s;
  for (s = str; *s; ++s);
  return (size_t)(s - str);
}


// internal test if char is a digit (0-9)
// \return true if char is a digit
static inline bool _is_digit(char ch)
{
  return (ch >= '0') && (ch <= '9');
}


// internal ASCII string to unsigned int conversion
static inline unsigned int _atoi(const char** str)
{
  unsigned int i = 0U;
  while (_is_digit(**str)) {
    i = i * 10U + (unsigned int)(*((*str)++) - '0');
  }
  return i;
}


// internal itoa format
static size_t _ntoa_format(char* buffer, char* buf, size_t len, bool negative, unsigned int base, size_t maxlen, unsigned int prec, unsigned int width, unsigned int flags)
{
  if (maxlen == 0U) {
    return 0U;
  }
  if (base > 16U) {
    return 0U;
  }

  // pad leading zeros
  while (!(flags & FLAGS_LEFT) && (len < prec) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
    buf[len++] = '0';
  }
  while (!(flags & FLAGS_LEFT) && (flags & FLAGS_ZEROPAD) && (len < width) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
    buf[len++] = '0';
  }

  // handle hash
  if (flags & FLAGS_HASH) {
    if (((len == prec) || (len == width)) && (len > 0U)) {
      len--;
      if ((base == 16U) && (len > 0U)) {
        len--;
      }
    }
    if ((base == 16U) && !(flags & FLAGS_UPPERCASE) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
      buf[len++] = 'x';
    }
    if ((base == 16U) &&  (flags & FLAGS_UPPERCASE) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
      buf[len++] = 'X';
    }
    if (len < PRINTF_NTOA_BUFFER_SIZE) {
      buf[len++] = '0';
    }
  }

  // handle sign
  if ((len == width) && (negative || (flags & FLAGS_PLUS) || (flags & FLAGS_SPACE))) {
    len--;
  }
  if (len < PRINTF_NTOA_BUFFER_SIZE) {
    if (negative) {
      buf[len++] = '-';
    }
    else if (flags & FLAGS_PLUS) {
      buf[len++] = '+';  // ignore the space if the '+' exists
    }
    else if (flags & FLAGS_SPACE) {
      buf[len++] = ' ';
    }
  }

  // pad spaces up to given width
  size_t idx = 0U;
  if (!(flags & FLAGS_LEFT) && !(flags & FLAGS_ZEROPAD)) {
    for (size_t i = len; (i < width) && (i < maxlen); ++i) {
      buffer[idx++] = ' ';
    }
  }

  // reverse string
  for (size_t i = 0U; (i < len) && (i < maxlen); ++i) {
    buffer[idx++] = buf[len - i - 1U];
  }

  // append pad spaces up to given width
  if (flags & FLAGS_LEFT) {
    while ((idx < width) && (idx < maxlen)) {
      buffer[idx++] = ' ';
    }
  }

  return idx;
}


// internal itoa for 'long' type
static size_t _ntoa_long(char* buffer, unsigned long value, bool negative, unsigned long base, size_t maxlen, unsigned int prec, unsigned int width, unsigned int flags)
{
  char buf[PRINTF_NTOA_BUFFER_SIZE];
  size_t len = 0U;

  // write if precision != 0 and value is != 0
  if (!(flags & FLAGS_PRECISION) || value) {
    do {
      char digit = (char)(value % base);
      buf[len++] = digit < 10 ? '0' + digit : (flags & FLAGS_UPPERCASE ? 'A' : 'a') + digit - 10;
      value /= base;
    } while ((len < PRINTF_NTOA_BUFFER_SIZE) && value);
  }

  return _ntoa_format(buffer, buf, len, negative, (unsigned int)base, maxlen, prec, width, flags);
}


// internal itoa for 'long long' type
#if defined(PRINTF_LONG_LONG_SUPPORT)
static size_t _ntoa_long_long(char* buffer, unsigned long long value, bool negative, unsigned long long base, size_t maxlen, unsigned int prec, unsigned int width, unsigned int flags)
{
  char buf[PRINTF_NTOA_BUFFER_SIZE];
  size_t len = 0U;

  // write if precision != 0 and value is != 0
  if (!(flags & FLAGS_PRECISION) || value) {
    do {
      char digit = (char)(value % base);
      buf[len++] = digit < 10 ? '0' + digit : (flags & FLAGS_UPPERCASE ? 'A' : 'a') + digit - 10;
      value /= base;
    } while ((len < PRINTF_NTOA_BUFFER_SIZE) && value);
  }

  return _ntoa_format(buffer, buf, len, negative, (unsigned int)base, maxlen, prec, width, flags);
}
#endif  // PRINTF_LONG_LONG_SUPPORT


#if defined(PRINTF_FLOAT_SUPPORT)
static size_t _ftoa(double value, char* buffer, size_t maxlen, unsigned int prec, unsigned int width, unsigned int flags)
{
  char buf[PRINTF_FTOA_BUFFER_SIZE];
  size_t len  = 0U;
  double diff = 0.0;

  // if input is larger than thres_max, revert to exponential
  const double thres_max = (double)0x7FFFFFFF;

  // powers of 10
  static const double pow10[] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000 };

  // test for negative
  bool negative = false;
  if (value < 0) {
    negative = true;
    value = 0 - value;
  }

  // limit precision
  if (!(flags & FLAGS_PRECISION)) {
    prec = 6U;  // by default, precesion is 6
  }
  if (prec > 9U) {
    // precision of >= 10 can lead to overflow errors
    prec = 9U;
  }

  int whole = (int)value;
  double tmp = (value - whole) * pow10[prec];
  unsigned long frac = (unsigned long)tmp;
  diff = tmp - frac;

  if (diff > 0.5) {
    ++frac;
    // handle rollover, e.g. case 0.99 with prec 1 is 1.0
    if (frac >= pow10[prec]) {
      frac = 0;
      ++whole;
    }
  }
  else if ((diff == 0.5) && ((frac == 0) || (frac & 1))) {
    // if halfway, round up if odd, OR if last digit is 0
    ++frac;
  }

  // TBD: for very large numbers switch back to native sprintf for exponentials. anyone want to write code to replace this?
  // normal printf behavior is to print EVERY whole number digit which can be 100s of characters overflowing your buffers == bad
  if (value > thres_max) {
    return 0U;
  }

  if (prec == 0U) {
    diff = value - whole;
    if (diff > 0.5) {
      // greater than 0.5, round up, e.g. 1.6 -> 2
      ++whole;
    }
    else if ((diff == 0.5) && (whole & 1)) {
      // exactly 0.5 and ODD, then round up
      // 1.5 -> 2, but 2.5 -> 2
      ++whole;
    }
  }
  else {
    unsigned int count = prec;
    // now do fractional part, as an unsigned number
    do {
      --count;
      buf[len++] = (char)(48U + (frac % 10U));
    } while ((len < PRINTF_FTOA_BUFFER_SIZE) && (frac /= 10U));
    // add extra 0s
    while ((len < PRINTF_FTOA_BUFFER_SIZE) && (count-- > 0U)) {
      buf[len++] = '0';
    }
    if (len < PRINTF_FTOA_BUFFER_SIZE) {
      // add decimal
      buf[len++] = '.';
    }
  }

  // do whole part, number is reversed
  while (len < PRINTF_FTOA_BUFFER_SIZE) {
    buf[len++] = (char)(48 + (whole % 10));
    if (!(whole /= 10)) {
      break;
    }
  }
  
  // pad leading zeros
  while (!(flags & FLAGS_LEFT) && (len < prec) && (len < PRINTF_FTOA_BUFFER_SIZE)) {
    buf[len++] = '0';
  }
  while (!(flags & FLAGS_LEFT) && (flags & FLAGS_ZEROPAD) && (len < width) && (len < PRINTF_FTOA_BUFFER_SIZE)) {
    buf[len++] = '0';
  }

  // handle sign
  if (len < PRINTF_FTOA_BUFFER_SIZE) {
    if (negative) {
      buf[len++] = '-';
    }
    else if (flags & FLAGS_PLUS) {
      buf[len++] = '+';  // ignore the space if the '+' exists
    }
    else if (flags & FLAGS_SPACE) {
      buf[len++] = ' ';
    }
  }

  // pad spaces up to given width
  size_t idx = 0U;
  if (!(flags & FLAGS_LEFT) && !(flags & FLAGS_ZEROPAD)) {
    for (size_t i = len; (i < width) && (i < maxlen); ++i) {
      buffer[idx++] = ' ';
    }
  }

  // reverse string
  for (size_t i = 0U; (i < len) && (i < maxlen); ++i) {
    buffer[idx++] = buf[len - i - 1U];
  }

  // append pad spaces up to given width
  if (flags & FLAGS_LEFT) {
    while ((idx < width) && (idx < maxlen)) {
      buffer[idx++] = ' ';
    }
  }

  return idx;
}
#endif  // PRINTF_FLOAT_SUPPORT


// internal vsnprintf
static int _vsnprintf(char* buffer, size_t buffer_len, const char* format, va_list va)
{
  unsigned int flags, width, precision, n;
  size_t idx = 0U;

  // check if buffer is valid
  if (!buffer) {
    return -1;
  }

  while ((idx < buffer_len) && *format)
  {
    // format specifier?  %[flags][width][.precision][length]
    if (*format != '%') {
      // no
      buffer[idx++] = *format;
      format++;
      continue;
    }
    else {
      // yes, evaluate it
      format++;
    }

    // evaluate flags
    flags = 0U;
    do {
      switch (*format) {
        case '0': flags |= FLAGS_ZEROPAD; format++; n = 1U; break;
        case '-': flags |= FLAGS_LEFT;    format++; n = 1U; break;
        case '+': flags |= FLAGS_PLUS;    format++; n = 1U; break;
        case ' ': flags |= FLAGS_SPACE;   format++; n = 1U; break;
        case '#': flags |= FLAGS_HASH;    format++; n = 1U; break;
        default :                                   n = 0U; break;
      }
    } while (n);

    // evaluate width field
    width = 0U;
    if (_is_digit(*format)) {
      width = _atoi(&format);
    }
    else if (*format == '*') {
      const int w = va_arg(va, int);
      if (w < 0) {
        flags |= FLAGS_LEFT;    // reverse padding
        width = (unsigned int)-w;
      }
      else {
        width = (unsigned int)w;
      }
      format++;
    }

    // evaluate precision field
    precision = 0U;
    if (*format == '.') {
      flags |= FLAGS_PRECISION;
      format++;
      if (_is_digit(*format)) {
        precision = _atoi(&format);
      }
      else if (*format == '*') {
        precision = (unsigned int)va_arg(va, int);
        format++;
      }
    }

    // evaluate length field
    if (*format == 'l' || *format == 'L') {
      flags |= FLAGS_LONG;
      format++;
    }
    if ((*format == 'l') && (flags & FLAGS_LONG)) {
      flags |= FLAGS_LONG_LONG;
      format++;
    }
    if (*format == 'z') {
      flags |= (sizeof(size_t) == sizeof(long) ? FLAGS_LONG : FLAGS_LONG_LONG);
      format++;
    }

    // evaluate specifier
    switch (*format) {
      case 'd' :
      case 'i' :
      case 'u' :
      case 'x' :
      case 'X' :
      case 'o' :
      case 'b' : {
        // set the base
        unsigned int base;
        if (*format == 'x' || *format == 'X') {
          base = 16U;
        }
        else if (*format == 'o') {
          base =  8U;
        }
        else if (*format == 'b') {
          base =  2U;
          flags &= ~FLAGS_HASH;   // no hash for bin format
        }
        else {
          base = 10U;
          flags &= ~FLAGS_HASH;   // no hash for dec format
        }
        // uppercase
        if (*format == 'X') {
          flags |= FLAGS_UPPERCASE;
        }

        // no plus or space flag for u, x, X, o, b
        if ((*format != 'i') && (*format != 'd')) {
          flags &= ~(FLAGS_PLUS | FLAGS_SPACE);
        }

        // convert the integer
        if ((*format == 'i') || (*format == 'd')) {
          // signed
          if (flags & FLAGS_LONG_LONG) {
#if defined(PRINTF_LONG_LONG_SUPPORT)
            const long long value = va_arg(va, long long);
            idx += _ntoa_long_long(&buffer[idx], (unsigned long long)(value > 0 ? value : 0 - value), value < 0, base, buffer_len - idx, precision, width, flags);
#endif
          }
          else if (flags & FLAGS_LONG) {
            const long value = va_arg(va, long);
            idx += _ntoa_long(&buffer[idx], (unsigned long)(value > 0 ? value : 0 - value), value < 0, base, buffer_len - idx, precision, width, flags);
          }
          else {
            const int value = va_arg(va, int);
            idx += _ntoa_long(&buffer[idx], (unsigned int)(value > 0 ? value : 0 - value), value < 0, base, buffer_len - idx, precision, width, flags);
          }
        }
        else {
          // unsigned
          if (flags & FLAGS_LONG_LONG) {
#if defined(PRINTF_LONG_LONG_SUPPORT)
            idx += _ntoa_long_long(&buffer[idx], va_arg(va, unsigned long long), false, base, buffer_len - idx, precision, width, flags);
#endif
          }
          else if (flags & FLAGS_LONG) {
            idx += _ntoa_long(&buffer[idx], va_arg(va, unsigned long), false, base, buffer_len - idx, precision, width, flags);
          }
          else {
            idx += _ntoa_long(&buffer[idx], va_arg(va, unsigned int), false, base, buffer_len - idx, precision, width, flags);
          }
        }
        format++;
        break;
      }
#if defined(PRINTF_FLOAT_SUPPORT)
      case 'f' :
      case 'F' :
        idx += _ftoa(va_arg(va, double), &buffer[idx], buffer_len - idx, precision, width, flags);
        format++;
        break;
#endif  // PRINTF_FLOAT_SUPPORT
      case 'c' : {
        size_t l = 1U;
        // pre padding
        if (!(flags & FLAGS_LEFT)) {
          while ((idx < buffer_len) && (l++ < width)) {
            buffer[idx++] = ' ';
          }
        }
        // char output
        buffer[idx++] = (char)va_arg(va, int);
        // post padding
        if (flags & FLAGS_LEFT) {
          while ((idx < buffer_len) && (l++ < width)) {
            buffer[idx++] = ' ';
          }
        }
        format++;
        break;
      }

      case 's' : {
        char* p = va_arg(va, char*);
        size_t l = _strlen(p);
        // pre padding
        if (flags & FLAGS_PRECISION) {
          l = (l < precision ? l : precision);
        }
        if (!(flags & FLAGS_LEFT)) {
          while ((idx < buffer_len) && (l++ < width)) {
            buffer[idx++] = ' ';
          }
        }
        // string output
        while ((idx < buffer_len) && (*p != 0) && (!(flags & FLAGS_PRECISION) || precision--)) {
          buffer[idx++] = *(p++);
        }
        // post padding
        if (flags & FLAGS_LEFT) {
          while ((idx < buffer_len) && (l++ < width)) {
            buffer[idx++] = ' ';
          }
        }
        format++;
        break;
      }

      case 'p' : {
        width = sizeof(void*) * 2U;
        flags |= FLAGS_ZEROPAD | FLAGS_UPPERCASE;
        if (sizeof(uintptr_t) == sizeof(long long)) {
#if defined(PRINTF_LONG_LONG_SUPPORT)
          idx += _ntoa_long_long(&buffer[idx], (uintptr_t)va_arg(va, void*), false, 16U, buffer_len - idx, precision, width, flags);
#endif
        }
        else {
          idx += _ntoa_long(&buffer[idx], (unsigned long)((uintptr_t)va_arg(va, void*)), false, 16U, buffer_len - idx, precision, width, flags);
        }
        format++;
        break;
      }

      case '%' :
        buffer[idx++] = '%';
        format++;
        break;

      default :
        buffer[idx++] = *format;
        format++;
        break;
    }
  }

  // termination
  if (buffer_len > 0U) {
    buffer[idx == buffer_len ? buffer_len - 1U : idx] = (char)0;
  }

  // return written chars without terminating \0
  return (int)idx;
}


///////////////////////////////////////////////////////////////////////////////

int printf(const char* format, ...)
{
  va_list va;
  va_start(va, format);
  char buffer[PRINTF_BUFFER_SIZE];
  int ret = _vsnprintf(buffer, PRINTF_BUFFER_SIZE, format, va);
  va_end(va);
  for (size_t i = 0U; i < ret; ++i) {
    _putchar(buffer[i]);
  }
  return ret;
}


int sprintf(char* buffer, const char* format, ...)
{
  va_list va;
  va_start(va, format);
  int ret = _vsnprintf(buffer, (size_t)-1, format, va);
  va_end(va);
  return ret;
}


int snprintf(char* buffer, size_t count, const char* format, ...)
{
  va_list va;
  va_start(va, format);
  int ret = _vsnprintf(buffer, count, format, va);
  va_end(va);
  return ret;
}


inline int vsnprintf(char* buffer, size_t count, const char* format, va_list va)
{
  return _vsnprintf(buffer, count, format, va);
}

