///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2015, PALANDesign Hannover, Germany
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
// \brief Tiny printf, sprintf and snprintf implementation, optimized for speed on
//        embedded systems with a very limited resources. These routines are thread
//        safe and reentrant!
//        Use this instead of the bloated standard/newlib printf cause these use
//        malloc for printf (and may not be thread safe).
//
///////////////////////////////////////////////////////////////////////////////

#include <stdarg.h>
#include "printf.h"


// buffer size used for printf
#define PRINTF_BUFFER_SIZE    128U

// ntoa conversion buffer size, this must be big enough to hold one converted numeric number
#define NTOA_BUFFER_SIZE      32U

// ftoa conversion buffer size, this must be big enough to hold one converted float number
#define FTOA_BUFFER_SIZE      32U

// define this to support floating point (%f)
#define PRINTF_FLOAT_SUPPORT


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


// returns 1 if char is a digit, 0 if not
static inline unsigned int _is_digit(char ch)
{
  return (ch >= '0' && ch <= '9') ? 1U : 0U;
}


// internal ASCII to unsigned int conversion
static inline unsigned int _atoi(const char** str)
{
  unsigned int i = 0U;
  while (_is_digit(**str)) {
    i = i * 10U + *((*str)++) - '0';
  }
  return i;
}


// internal itoa
template<typename T>
static size_t _ntoa(T value, char* buffer, unsigned int base, size_t maxlen, unsigned int width, unsigned int flags)
{
  char buf[NTOA_BUFFER_SIZE];
  size_t len = 0U;
  unsigned int negative = 0U;

  if (maxlen == 0U) {
    return 0U;
  }
  if (base > 16U) {
    return 0U;
  }
  if (value < 0) {
    negative = 1;
    value = 0 - value;
  }

  do {
    char digit = (char)((unsigned)value % base);
    buf[len++] = digit < 10 ? '0' + digit : (flags & FLAGS_UPPERCASE ? 'A' : 'a') + digit - 10;
    value /= (T)base;
  } while ((len < NTOA_BUFFER_SIZE) && (value > 0));
  // pad zeros
  while ((flags & FLAGS_ZEROPAD) && (len < width) && (len < NTOA_BUFFER_SIZE)) {
    buf[len++] = '0';
  }
  // handle sign
  if (len < NTOA_BUFFER_SIZE) {
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
  while ((len < width) && (len < NTOA_BUFFER_SIZE)) {
    buf[len++] = ' ';
  }

  // reverse it
  for (size_t i = 0U; (i < len) && (i < maxlen); ++i) {
    buffer[i] = buf[len - i - 1];
  }

  return len;
}


#if defined(PRINTF_FLOAT_SUPPORT)
static size_t _ftoa(double value, char* buffer, size_t maxlen, unsigned int prec, unsigned int width, unsigned int flags)
{
  // test for NaN
  if (!(value == value) && (maxlen > 2U)) {
    buffer[0] = 'n'; buffer[1] = 'a'; buffer[2] = 'n';
    return (size_t)3;
  }
  // if input is larger than thres_max, revert to exponential
  const double thres_max = (double)(0x7FFFFFFF);

  char buf[FTOA_BUFFER_SIZE];
  size_t len = 0U;
  double diff = 0.0;

  // powers of 10
  static const double pow10[] = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000};

  // limit precision
  if (prec > 9U) {
    // precision of >= 10 can lead to overflow errors
    prec = 9U;
  }

  unsigned int negative = 0U;
  if (value < 0) {
    negative = 1U;
    value = value * -1;
  }

  int whole = (int)value;
  double tmp = (value - whole) * pow10[prec];
  unsigned long frac = (unsigned long)tmp;
  diff = tmp - frac;

  if (diff > 0.5) {
    ++frac;
    // handle rollover, e.g.  case 0.99 with prec 1 is 1.0
    if (frac >= pow10[prec]) {
      frac = 0;
      ++whole;
    }
  }
  else if ((diff == 0.5) && ((frac == 0) || (frac & 1))) {
    // if halfway, round up if odd, OR if last digit is 0
    ++frac;
  }

  // for very large numbers switch back to native sprintf for exponentials. anyone want to write code to replace this?
  // normal printf behavior is to print EVERY whole number digit which can be 100s of characters overflowing your buffers == bad
  if (value > thres_max) {
    return 0;
  }

  if (prec == 0) {
    diff = value - whole;
    if (diff > 0.5) {
      // greater than 0.5, round up, e.g. 1.6 -> 2
      ++whole;
    }
    else if (diff == 0.5 && (whole & 1)) {
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
    } while ((len < FTOA_BUFFER_SIZE) && (frac /= 10U));
    // add extra 0s
    while ((len < FTOA_BUFFER_SIZE) && (count-- > 0)) {
      buf[len++] = '0';
    }
    if (len < FTOA_BUFFER_SIZE) {
      // add decimal
      buf[len++] = '.';
    }
  }

  // do whole part
  // Take care of sign conversion. Number is reversed
  size_t wlen = 0U;
  do {
    buf[len++] = (char)(48 + (whole % 10));
    wlen++;
  } while ((len < FTOA_BUFFER_SIZE) && (whole /= 10));
  // pad zeros
  while ((flags & FLAGS_ZEROPAD) && (wlen < width) && (len < FTOA_BUFFER_SIZE)) {
    buf[len++] = '0';
    wlen++;
  }
  // handle sign
  if (len < FTOA_BUFFER_SIZE) {
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
  while ((wlen < width) && (len < FTOA_BUFFER_SIZE)) {
    buf[len++] = ' ';
    wlen++;
  }

  // reverse it
  for (size_t i = 0U; (i < len) && (i < maxlen); ++i) {
    buffer[i] = buf[len - i - 1];
  }

  return len;
}
#endif


// internal vsnprintf
static size_t vsnprintf(char* buffer, size_t buffer_len, const char* format, va_list va)
{
  unsigned int flags, width, precision, n;
  size_t idx = 0U;

  while (idx < buffer_len) {
    // end reached?
    if (*format == '\0') {
      buffer[idx] = '\0';
      break;
    }

    // format specifier?
    if (*format != '%') {
      buffer[idx++] = *format;
      format++;
      continue;
    }

    // evaluate flags
    flags = 0U;
    do {
      format++;
      switch (*format) {
        case '0': flags |= FLAGS_ZEROPAD; n = 1U; break;
        case '-': flags |= FLAGS_LEFT;    n = 1U; break;
        case '+': flags |= FLAGS_PLUS;    n = 1U; break;
        case ' ': flags |= FLAGS_SPACE;   n = 1U; break;
        case '#': flags |= FLAGS_HASH;    n = 1U; break;
        default :                         n = 0U; break;
      }
    } while (n);

    // evaluate width field
    width = 1U;
    if (_is_digit(*format)) {
      width = _atoi(&format);
    }
    else if (*format == '*') {
      width = (unsigned int)va_arg(va, int);
      format++;
    }

    // evaluate precision field
    precision = 0U;
    if (*format == '.') {
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

    // evaluate specifier
    switch (*format) {
      case 'u' :
      case 'd' :
      case 'i' :
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
        }
        else {
          base = 10U;
        }
        // uppercase
        if (*format == 'X') {
          flags |= FLAGS_UPPERCASE;
        }

        if (flags & FLAGS_HASH) {
          if (buffer_len - idx > 0U) {
            buffer[idx++] = '0';
          }
          if ((*format == 'x') && (buffer_len - idx > 0U)) {
            buffer[idx++] = 'x';
          }
          if ((*format == 'X') && (buffer_len - idx > 0U)) {
            buffer[idx++] = 'X';
          }
        }

        // convert the integer
        if (*format == 'i' || *format == 'd') {
          // signed
          if (flags & FLAGS_LONG_LONG) {
            idx += _ntoa<long long>(va_arg(va, long long), &buffer[idx], base, buffer_len - idx, width, flags);
          }
          else if (flags & FLAGS_LONG) {
            idx += _ntoa<long>(va_arg(va, long), &buffer[idx], base, buffer_len - idx, width, flags);
          }
          else {
            idx += _ntoa<int>(va_arg(va, int), &buffer[idx], base, buffer_len - idx, width, flags);
          }
        }
        else {
          // unsigned
          if (flags & FLAGS_LONG_LONG) {
            idx += _ntoa<unsigned long long>(va_arg(va, unsigned long long), &buffer[idx], base, buffer_len - idx, width, flags);
          }
          else if (flags & FLAGS_LONG) {
            idx += _ntoa<unsigned long>(va_arg(va, unsigned long), &buffer[idx], base, buffer_len - idx, width, flags);
          }
          else {
            idx += _ntoa<unsigned int>(va_arg(va, unsigned int), &buffer[idx], base, buffer_len - idx, width, flags);
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
#endif
      case 'c' :
        buffer[idx++] = (char)va_arg(va, int);
        format++;
        break;

      case 's' : {
        char* p = va_arg(va, char*);
        while ((idx < buffer_len) && (*p != 0)) {
          buffer[idx++] = *(p++);
        }
        format++;
        break;
      }

      case 'p' : {
        width = sizeof(void*) * 2U;
        flags |= FLAGS_ZEROPAD;
        size_t size_void = sizeof(void*);
        if (size_void > sizeof(long)) {
          idx +=_ntoa<unsigned long long>(reinterpret_cast<unsigned long long>(va_arg(va, void*)), &buffer[idx], 16U, buffer_len - idx, width, flags);
        }
        else {
          idx += _ntoa<unsigned long>(reinterpret_cast<unsigned long>(va_arg(va, void*)), &buffer[idx], 16U, buffer_len - idx, width, flags);
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
        break;
    }
  }

  return idx;
}

///////////////////////////////////////////////////////////////////////////////

int printf(const char* format, ...)
{
  va_list va;
  va_start(va, format);
  char buffer[PRINTF_BUFFER_SIZE];
  size_t ret = vsnprintf(buffer, PRINTF_BUFFER_SIZE, format, va);
  va_end(va);
  for (size_t i = 0; i < ret; ++i) {
    _putchar(buffer[i]);
  }
  return (int)ret;
}


int sprintf(char* buffer, const char* format, ...)
{
  va_list va;
  va_start(va, format);
  size_t ret = vsnprintf(buffer, (size_t)-1, format, va);
  va_end(va);
  return (int)ret;
}


int snprintf(char* buffer, size_t count, const char* format, ...)
{
  va_list va;
  va_start(va, format);
  size_t ret = vsnprintf(buffer, count, format, va);
  va_end(va);
  return (int)ret;
}
