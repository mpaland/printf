///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2019, PALANDesign Hannover, Germany
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


// wrappers for PGM or non-PGM access
#ifdef PRINTF_AVR_PGM
  #include <avr/pgmspace.h>
  #define FMTRD(_fmt) pgm_read_byte(_fmt)
  #define ATOI(_src) _atoi_P(_src)
#else
  #define FMTRD(_fmt) (*(_fmt))
  #define ATOI(_src) _atoi(_src)
#endif


// internal ASCII string to unsigned int conversion
#ifdef PRINTF_AVR_PGM
static unsigned int _atoi_P(const char** str)
#else
static unsigned int _atoi(const char** str)
#endif
{
  unsigned int i = 0U;
    while (_is_digit(FMTRD(*str))) {
	    i = i * 10U + (unsigned int)(FMTRD((*str)++) - '0');
    }
  return i;
}


// internal vsnprintf
#ifdef PRINTF_AVR_PGM
static int _vsnprintf_P(out_fct_type out, char* buffer, const size_t maxlen, PGM_P format, va_list va)
#else
static int _vsnprintf(out_fct_type out, char* buffer, const size_t maxlen, const char* format, va_list va)
#endif
{
  unsigned int flags, width, precision, n;
  size_t idx = 0U;

  if (!buffer) {
    // use null output function
    out = _out_null;
  }

  while (FMTRD(format))
  {
    // format specifier?  %[flags][width][.precision][length]
    if (FMTRD(format) != '%') {
      // no
      out(FMTRD(format), buffer, idx++, maxlen);
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
      switch (FMTRD(format)) {
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
    if (_is_digit(FMTRD(format))) {
      width = ATOI(&format);
    }
    else if (FMTRD(format) == '*') {
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
    if (FMTRD(format) == '.') {
      flags |= FLAGS_PRECISION;
      format++;
      if (_is_digit(FMTRD(format))) {
        precision = ATOI(&format);
      }
      else if (FMTRD(format) == '*') {
        const int prec = (int)va_arg(va, int);
        precision = prec > 0 ? (unsigned int)prec : 0U;
        format++;
      }
    }

    // evaluate length field
    switch (FMTRD(format)) {
      case 'l' :
        flags |= FLAGS_LONG;
        format++;
        if (FMTRD(format) == 'l') {
          flags |= FLAGS_LONG_LONG;
          format++;
        }
        break;
      case 'h' :
        flags |= FLAGS_SHORT;
        format++;
        if (FMTRD(format) == 'h') {
          flags |= FLAGS_CHAR;
          format++;
        }
        break;
#if defined(PRINTF_SUPPORT_PTRDIFF_T)
      case 't' :
        flags |= (sizeof(ptrdiff_t) == sizeof(long) ? FLAGS_LONG : FLAGS_LONG_LONG);
        format++;
        break;
#endif
      case 'j' :
        flags |= (sizeof(intmax_t) == sizeof(long) ? FLAGS_LONG : FLAGS_LONG_LONG);
        format++;
        break;
      case 'z' :
        flags |= (sizeof(size_t) == sizeof(long) ? FLAGS_LONG : FLAGS_LONG_LONG);
        format++;
        break;
      default :
        break;
    }

    // evaluate specifier
    switch (FMTRD(format)) {
      case 'd' :
      case 'i' :
      case 'u' :
      case 'x' :
      case 'X' :
      case 'o' :
      case 'b' : {
        // set the base
        unsigned int base;
        if (FMTRD(format) == 'x' || FMTRD(format) == 'X') {
          base = 16U;
        }
        else if (FMTRD(format) == 'o') {
          base =  8U;
        }
        else if (FMTRD(format) == 'b') {
          base =  2U;
        }
        else {
          base = 10U;
          flags &= ~FLAGS_HASH;   // no hash for dec format
        }
        // uppercase
        if (FMTRD(format) == 'X') {
          flags |= FLAGS_UPPERCASE;
        }

        // no plus or space flag for u, x, X, o, b
        if ((FMTRD(format) != 'i') && (FMTRD(format) != 'd')) {
          flags &= ~(FLAGS_PLUS | FLAGS_SPACE);
        }

        // ignore '0' flag when precision is given
        if (flags & FLAGS_PRECISION) {
          flags &= ~FLAGS_ZEROPAD;
        }

        // convert the integer
        if ((FMTRD(format) == 'i') || (FMTRD(format) == 'd')) {
          // signed
          if (flags & FLAGS_LONG_LONG) {
#if defined(PRINTF_SUPPORT_LONG_LONG)
            const long long value = va_arg(va, long long);
            idx = _ntoa_long_long(out, buffer, idx, maxlen, (unsigned long long)(value > 0 ? value : 0 - value), value < 0, base, precision, width, flags);
#endif
          }
          else if (flags & FLAGS_LONG) {
            const long value = va_arg(va, long);
            idx = _ntoa_long(out, buffer, idx, maxlen, (unsigned long)(value > 0 ? value : 0 - value), value < 0, base, precision, width, flags);
          }
          else {
            const int value = (flags & FLAGS_CHAR) ? (char)va_arg(va, int) : (flags & FLAGS_SHORT) ? (short int)va_arg(va, int) : va_arg(va, int);
            idx = _ntoa_long(out, buffer, idx, maxlen, (unsigned int)(value > 0 ? value : 0 - value), value < 0, base, precision, width, flags);
          }
        }
        else {
          // unsigned
          if (flags & FLAGS_LONG_LONG) {
#if defined(PRINTF_SUPPORT_LONG_LONG)
            idx = _ntoa_long_long(out, buffer, idx, maxlen, va_arg(va, unsigned long long), false, base, precision, width, flags);
#endif
          }
          else if (flags & FLAGS_LONG) {
            idx = _ntoa_long(out, buffer, idx, maxlen, va_arg(va, unsigned long), false, base, precision, width, flags);
          }
          else {
            const unsigned int value = (flags & FLAGS_CHAR) ? (unsigned char)va_arg(va, unsigned int) : (flags & FLAGS_SHORT) ? (unsigned short int)va_arg(va, unsigned int) : va_arg(va, unsigned int);
            idx = _ntoa_long(out, buffer, idx, maxlen, value, false, base, precision, width, flags);
          }
        }
        format++;
        break;
      }
#if defined(PRINTF_SUPPORT_FLOAT)
      case 'f' :
      case 'F' :
        if (FMTRD(format) == 'F') flags |= FLAGS_UPPERCASE;
        idx = _ftoa(out, buffer, idx, maxlen, va_arg(va, double), precision, width, flags);
        format++;
        break;
#if defined(PRINTF_SUPPORT_EXPONENTIAL)
      case 'e':
      case 'E':
      case 'g':
      case 'G':
        if ((FMTRD(format) == 'g')||(FMTRD(format) == 'G')) flags |= FLAGS_ADAPT_EXP;
        if ((FMTRD(format) == 'E')||(FMTRD(format) == 'G')) flags |= FLAGS_UPPERCASE;
        idx = _etoa(out, buffer, idx, maxlen, va_arg(va, double), precision, width, flags);
        format++;
        break;
#endif  // PRINTF_SUPPORT_EXPONENTIAL
#endif  // PRINTF_SUPPORT_FLOAT
      case 'c' : {
        unsigned int l = 1U;
        // pre padding
        if (!(flags & FLAGS_LEFT)) {
          while (l++ < width) {
            out(' ', buffer, idx++, maxlen);
          }
        }
        // char output
        out((char)va_arg(va, int), buffer, idx++, maxlen);
        // post padding
        if (flags & FLAGS_LEFT) {
          while (l++ < width) {
            out(' ', buffer, idx++, maxlen);
          }
        }
        format++;
        break;
      }

      case 's' : {
        const char* p = va_arg(va, char*);
        unsigned int l = _strnlen_s(p, precision ? precision : (size_t)-1);
        // pre padding
        if (flags & FLAGS_PRECISION) {
          l = (l < precision ? l : precision);
        }
        if (!(flags & FLAGS_LEFT)) {
          while (l++ < width) {
            out(' ', buffer, idx++, maxlen);
          }
        }
        // string output
        while ((*p != 0) && (!(flags & FLAGS_PRECISION) || precision--)) {
          out(*(p++), buffer, idx++, maxlen);
        }
        // post padding
        if (flags & FLAGS_LEFT) {
          while (l++ < width) {
            out(' ', buffer, idx++, maxlen);
          }
        }
        format++;
        break;
      }

      case 'p' : {
        width = sizeof(void*) * 2U;
        flags |= FLAGS_ZEROPAD | FLAGS_UPPERCASE;
#if defined(PRINTF_SUPPORT_LONG_LONG)
        const bool is_ll = sizeof(uintptr_t) == sizeof(long long);
        if (is_ll) {
          idx = _ntoa_long_long(out, buffer, idx, maxlen, (uintptr_t)va_arg(va, void*), false, 16U, precision, width, flags);
        }
        else {
#endif
          idx = _ntoa_long(out, buffer, idx, maxlen, (unsigned long)((uintptr_t)va_arg(va, void*)), false, 16U, precision, width, flags);
#if defined(PRINTF_SUPPORT_LONG_LONG)
        }
#endif
        format++;
        break;
      }

      case '%' :
        out('%', buffer, idx++, maxlen);
        format++;
        break;

      default :
        out(FMTRD(format), buffer, idx++, maxlen);
        format++;
        break;
    }
  }

  // termination
  out((char)0, buffer, idx < maxlen ? idx : maxlen - 1U, maxlen);

  // return written chars without terminating \0
  return (int)idx;
}


// Application interface functions for PROGMEM access
#ifdef PRINTF_AVR_PGM

int printf_P_(PGM_P format, ...)
{
  va_list va;
  va_start(va, format);
  char buffer[1];
  const int ret = _vsnprintf_P(_out_char, buffer, (size_t)-1, format, va);
  va_end(va);
  return ret;
}


int sprintf_P_(char* buffer, PGM_P format, ...)
{
  va_list va;
  va_start(va, format);
  const int ret = _vsnprintf_P(_out_buffer, buffer, (size_t)-1, format, va);
  va_end(va);
  return ret;
}


int snprintf_P_(char* buffer, size_t count, PGM_P format, ...)
{
  va_list va;
  va_start(va, format);
  const int ret = _vsnprintf_P(_out_buffer, buffer, count, format, va);
  va_end(va);
  return ret;
}


int vprintf_P_(PGM_P format, va_list va)
{
  char buffer[1];
  return _vsnprintf_P(_out_char, buffer, (size_t)-1, format, va);
}


int vsnprintf_P_(char* buffer, size_t count, PGM_P format, va_list va)
{
  return _vsnprintf_P(_out_buffer, buffer, count, format, va);
}


int fctprintf_P(void (*out)(char character, void* arg), void* arg, PGM_P format, ...)
{
  va_list va;
  va_start(va, format);
  const out_fct_wrap_type out_fct_wrap = { out, arg };
  const int ret = _vsnprintf_P(_out_fct, (char*)(uintptr_t)&out_fct_wrap, (size_t)-1, format, va);
  va_end(va);
  return ret;
}

#endif

#undef FMTRD
#undef ATOI
