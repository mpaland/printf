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

#include <stdbool.h>
#include <stdint.h>

#include "printf.h"


// define this globally (e.g. gcc -DPRINTF_INCLUDE_CONFIG_H ...) to include the
// printf_config.h header file
// default: undefined
#ifdef PRINTF_INCLUDE_CONFIG_H
#include "printf_config.h"
#endif


// 'ntoa' conversion buffer size, this must be big enough to hold one converted
// numeric number including padded zeros (dynamically created on stack)
// default: 32 byte
#ifndef PRINTF_NTOA_BUFFER_SIZE
#define PRINTF_NTOA_BUFFER_SIZE    32U
#endif

// 'ftoa' conversion buffer size, this must be big enough to hold one converted
// float number including padded zeros (dynamically created on stack)
// default: 32 byte
#ifndef PRINTF_FTOA_BUFFER_SIZE
#define PRINTF_FTOA_BUFFER_SIZE    32U
#endif

// support for the floating point type (%f)
// default: activated
#ifndef PRINTF_DISABLE_SUPPORT_FLOAT
#define PRINTF_SUPPORT_FLOAT
#endif

// support for exponential floating point notation (%e/%g)
// default: activated
#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
#define PRINTF_SUPPORT_EXPONENTIAL
#endif

// define the default floating point precision
// default: 6 digits
#ifndef PRINTF_DEFAULT_FLOAT_PRECISION
#define PRINTF_DEFAULT_FLOAT_PRECISION  6U
#endif

// define the largest float suitable to print with %f
// default: 1e18 (18 is largest because of int64_t limits)
#ifndef PRINTF_MAX_FLOAT_PRECISION
#define PRINTF_MAX_FLOAT_PRECISION     18
#endif
#define PRINTF_MAX_FLOAT        _e10(PRINTF_MAX_FLOAT_PRECISION)
#define PRINTF_MIN_FLOAT        _e10(-PRINTF_MAX_FLOAT_PRECISION)

// support for the long long types (%llu or %p)
// default: activated
#ifndef PRINTF_DISABLE_SUPPORT_LONG_LONG
#define PRINTF_SUPPORT_LONG_LONG
#endif

// support for the ptrdiff_t type (%t)
// ptrdiff_t is normally defined in <stddef.h> as long or long long type
// default: activated
#ifndef PRINTF_DISABLE_SUPPORT_PTRDIFF_T
#define PRINTF_SUPPORT_PTRDIFF_T
#endif

///////////////////////////////////////////////////////////////////////////////

// internal flag definitions
#define FLAGS_ZEROPAD   (1U <<  0U)
#define FLAGS_LEFT      (1U <<  1U)
#define FLAGS_PLUS      (1U <<  2U)
#define FLAGS_SPACE     (1U <<  3U)
#define FLAGS_HASH      (1U <<  4U)
#define FLAGS_UPPERCASE (1U <<  5U)
#define FLAGS_CHAR      (1U <<  6U)
#define FLAGS_SHORT     (1U <<  7U)
#define FLAGS_LONG      (1U <<  8U)
#define FLAGS_LONG_LONG (1U <<  9U)
#define FLAGS_PRECISION (1U << 10U)
#define FLAGS_ADAPT_EXP (1U << 11U)

// import float.h for DBL_MAX
#if defined(PRINTF_SUPPORT_FLOAT)
#include <float.h>
#endif

//
// Helper macro to record and remove negativity (also works for -0.0)
//
#define _ISIGNCHECK(i,n)     if (i < 0) { i = -i; n = 1; } else { n = 0; } 
#define _FSIGNCHECK(f,n)     if (1.0/f < 0.0) { f = -f; n = 1; } else { n = 0; } 

#if defined(PRINTF_SUPPORT_FLOAT)
//
// This is actually a fairly efficient implementation compared to pow() as it's
// only power of 10 and doesn't have any edge cases to worry about.
//
double _e10(int exp) {
    static const int64_t pows[] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000,
                                    100000000, 1000000000, 10000000000, 100000000000,
                                    (int64_t)1e+12, (int64_t)1e+13, (int64_t)1e+14, (int64_t)1e+15, 
                                    (int64_t)1e+16, (int64_t)1e+17, (int64_t)1e+18 };
   
    double  rc = 1.0;
    int negative;
    _ISIGNCHECK(exp, negative);

    // Handle exponents above our constants...  
    while (exp > 18) {
        rc *= 1e+18;
        exp -= 18;
    }
    // Now deal with what's left...
    rc *= pows[exp];
    // And adjust for plus or minus...
    return(negative ? 1.0/rc : rc);
}
#endif // defined(PRINTF_SUPPORT_FLOAT)

// output function type
typedef void (*out_fct_type)(char character, void* buffer, size_t idx, size_t maxlen);


// wrapper (used as buffer) for output function type
typedef struct {
  void  (*fct)(char character, void* arg);
  void* arg;
} out_fct_wrap_type;


// internal buffer output
static inline void _out_buffer(char character, void* buffer, size_t idx, size_t maxlen)
{
  if (idx < maxlen) {
    ((char*)buffer)[idx] = character;
  }
}


// internal null output
static inline void _out_null(char character, void* buffer, size_t idx, size_t maxlen)
{
  (void)character; (void)buffer; (void)idx; (void)maxlen;
}


// internal _putchar wrapper
static inline void _out_char(char character, void* buffer, size_t idx, size_t maxlen)
{
  (void)buffer; (void)idx; (void)maxlen;
  if (character) {
    _putchar(character);
  }
}


// internal output function wrapper
static inline void _out_fct(char character, void* buffer, size_t idx, size_t maxlen)
{
  (void)idx; (void)maxlen;
  if (character) {
    // buffer is the output fct pointer
    ((out_fct_wrap_type*)buffer)->fct(character, ((out_fct_wrap_type*)buffer)->arg);
  }
}


// internal secure strlen
// \return The length of the string (excluding the terminating 0) limited by 'maxsize'
static inline unsigned int _strnlen_s(const char* str, size_t maxsize)
{
  const char* s;
  for (s = str; *s && maxsize--; ++s);
  return (unsigned int)(s - str);
}


// internal test if char is a digit (0-9)
// \return true if char is a digit
static inline bool _is_digit(char ch)
{
  return (ch >= '0') && (ch <= '9');
}


// internal ASCII string to unsigned int conversion
static unsigned int _atoi(const char** str)
{
  unsigned int i = 0U;
  while (_is_digit(**str)) {
    i = i * 10U + (unsigned int)(*((*str)++) - '0');
  }
  return i;
}

//
// Modified reverse output function that deals with space and zero padding, and sign,
// outside of the actual buffer. Needs to know if the value is negative.
//
// TODO: replace other out_rev calls
//
static size_t _out_rev2(out_fct_type out, char* buffer, size_t idx, size_t maxlen, const char* buf, size_t len, unsigned int width, unsigned int flags, int negative)
{
    const size_t start_idx = idx;

    // First figure out the right sign to use...
    int sign=0;
    if (negative) {
        sign='-';
    } else if (flags & FLAGS_PLUS) {
        sign='+';
    } else if (flags & FLAGS_SPACE) {
        sign=' ';
    }
    if (sign) {
        len++;        // so space padding works
    }
    // Now we can output... first padding spaces (if needed) 
    if (!(flags & FLAGS_LEFT) && !(flags & FLAGS_ZEROPAD)) {
        for (int i=len; i < width; i++) {
            out(' ', buffer, idx++, maxlen);
        }
    }
    // Now the sign
    if (sign) {
        out(sign, buffer, idx++,maxlen);
    }
    // Now zeropad (no spaces would have happened)
    if (!(flags & FLAGS_LEFT) && (flags & FLAGS_ZEROPAD)) {
        for (int i=len; i < width; i++) {
            out('0', buffer, idx++, maxlen);
        }
    }
    if (sign) {
        len--;        // put back the length
    }
    // The the value
    while(len > 0) {
        out(buf[--len], buffer, idx++, maxlen);
    }
    // append pad spaces up to given width
    if (flags & FLAGS_LEFT) {
        while (idx - start_idx < width) {
          out(' ', buffer, idx++, maxlen);
        }
    }
    return idx;
}


// output the specified string in reverse, taking care of any zero-padding
static size_t _out_rev(out_fct_type out, char* buffer, size_t idx, size_t maxlen, const char* buf, size_t len, unsigned int width, unsigned int flags)
{
  const size_t start_idx = idx;

  // pad spaces up to given width
  if (!(flags & FLAGS_LEFT) && !(flags & FLAGS_ZEROPAD)) {
    for (size_t i = len; i < width; i++) {
      out(' ', buffer, idx++, maxlen);
    }
  }

  // reverse string
  while (len) {
    out(buf[--len], buffer, idx++, maxlen);
  }

  // append pad spaces up to given width
  if (flags & FLAGS_LEFT) {
    while (idx - start_idx < width) {
      out(' ', buffer, idx++, maxlen);
    }
  }

  return idx;
}


// internal itoa format
static size_t _ntoa_format(out_fct_type out, char* buffer, size_t idx, size_t maxlen, char* buf, size_t len, bool negative, unsigned int base, unsigned int prec, unsigned int width, unsigned int flags)
{
  // pad leading zeros
  if (!(flags & FLAGS_LEFT)) {
    if (width && (flags & FLAGS_ZEROPAD) && (negative || (flags & (FLAGS_PLUS | FLAGS_SPACE)))) {
      width--;
    }
    while ((len < prec) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
      buf[len++] = '0';
    }
    while ((flags & FLAGS_ZEROPAD) && (len < width) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
      buf[len++] = '0';
    }
  }

  // handle hash
  if (flags & FLAGS_HASH) {
    if (!(flags & FLAGS_PRECISION) && len && ((len == prec) || (len == width))) {
      len--;
      if (len && (base == 16U)) {
        len--;
      }
    }
    if ((base == 16U) && !(flags & FLAGS_UPPERCASE) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
      buf[len++] = 'x';
    }
    else if ((base == 16U) && (flags & FLAGS_UPPERCASE) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
      buf[len++] = 'X';
    }
    else if ((base == 2U) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
      buf[len++] = 'b';
    }
    if (len < PRINTF_NTOA_BUFFER_SIZE) {
      buf[len++] = '0';
    }
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

  return _out_rev(out, buffer, idx, maxlen, buf, len, width, flags);
}


// internal itoa for 'long' type
static size_t _ntoa_long(out_fct_type out, char* buffer, size_t idx, size_t maxlen, unsigned long value, bool negative, unsigned long base, unsigned int prec, unsigned int width, unsigned int flags)
{
  char buf[PRINTF_NTOA_BUFFER_SIZE];
  size_t len = 0U;

  // no hash for 0 values
  if (!value) {
    flags &= ~FLAGS_HASH;
  }

  // write if precision != 0 and value is != 0
  if (!(flags & FLAGS_PRECISION) || value) {
    do {
      const char digit = (char)(value % base);
      buf[len++] = digit < 10 ? '0' + digit : (flags & FLAGS_UPPERCASE ? 'A' : 'a') + digit - 10;
      value /= base;
    } while (value && (len < PRINTF_NTOA_BUFFER_SIZE));
  }

  return _ntoa_format(out, buffer, idx, maxlen, buf, len, negative, (unsigned int)base, prec, width, flags);
}


// internal itoa for 'long long' type
#if defined(PRINTF_SUPPORT_LONG_LONG)
static size_t _ntoa_long_long(out_fct_type out, char* buffer, size_t idx, size_t maxlen, unsigned long long value, bool negative, unsigned long long base, unsigned int prec, unsigned int width, unsigned int flags)
{
  char buf[PRINTF_NTOA_BUFFER_SIZE];
  size_t len = 0U;

  // no hash for 0 values
  if (!value) {
    flags &= ~FLAGS_HASH;
  }

  // write if precision != 0 and value is != 0
  if (!(flags & FLAGS_PRECISION) || value) {
    do {
      const char digit = (char)(value % base);
      buf[len++] = digit < 10 ? '0' + digit : (flags & FLAGS_UPPERCASE ? 'A' : 'a') + digit - 10;
      value /= base;
    } while (value && (len < PRINTF_NTOA_BUFFER_SIZE));
  }

  return _ntoa_format(out, buffer, idx, maxlen, buf, len, negative, (unsigned int)base, prec, width, flags);
}
#endif  // PRINTF_SUPPORT_LONG_LONG


#if defined(PRINTF_SUPPORT_FLOAT)
#if defined(PRINTF_SUPPORT_EXPONENTIAL)
// forward declaration so that _ftoa can switch to exp notation for values where needed
static size_t _etoa(out_fct_type out, char* buffer, size_t idx, size_t maxlen, double value, unsigned int prec, unsigned int width, unsigned int flags);
#endif

//
// Given a float value, turn it into a string with the given number of
// "prec" decimal places. If the number is too large to be calculated
// (MAX_FLOAT) then switch to scientific notation (with 6 sig digs)
//
// Changes compared to mpaland/printf routine:
//
// 1. Keeps padding (zero/space) out of internal buffer, increasing space
//    available for the result.
// 2. Supports precision up to 18 digits (e+18 and e-18)
// 3. Supports FLAGS_HASH
// 4. Max number length fits in buffer, so no length checking needed
//
static size_t _ftoa(out_fct_type out, char* buffer, size_t idx, size_t maxlen, double value, unsigned int prec, unsigned int width, unsigned int flags) {
    char    buf[(2*PRINTF_MAX_FLOAT_PRECISION)+1];
    size_t  len = 0;

    // Special cases (nan, inf-, inf+)
    if (value != value) {
        return _out_rev2(out, buffer, idx, maxlen, "nan", 3, width, flags, 0);
    }
    if (value < -DBL_MAX) {
        return _out_rev2(out, buffer, idx, maxlen, "fni", 3, width, (flags&~FLAGS_ZEROPAD), 1);
    }
    if (value > DBL_MAX) {
        return _out_rev2(out, buffer, idx, maxlen, "fni", 3, width, (flags&~FLAGS_ZEROPAD), 0);
    }

    if (!(flags & FLAGS_PRECISION)) {
        prec = PRINTF_DEFAULT_FLOAT_PRECISION;
    } else if (prec > PRINTF_MAX_FLOAT_PRECISION) {
        prec=PRINTF_MAX_FLOAT_PRECISION;
    }

    // Store and remove negativity
    int negative;
    _FSIGNCHECK(value, negative);

    // Check for very large or very small numbers that might break things
    if (value > PRINTF_MAX_FLOAT || (value < PRINTF_MIN_FLOAT && value != 0)) {
#if defined(PRINTF_SUPPORT_EXPONENTIAL)
        return _etoa(out, buffer, idx, maxlen, (negative ? -value : value), PRINTF_DEFAULT_FLOAT_PRECISION, width, flags);
#else
        return 0U;
#endif
    }

    // First we separate the whole and fractioanal parts
    int64_t whole = (int64_t)value;
    double dfrac = value - whole;
    
    // Now deal with fractional precision (and rounding)
    dfrac *= _e10((int)prec);
    int64_t frac = (int64_t)dfrac;

    if ((dfrac - frac) >= 0.5) {
        frac++;
        if (frac >= _e10((int)prec)) {
            whole++;
            frac = 0;
        }
    }
    // Build the number output
    if (prec) {
        while (prec--) {
            buf[len++] = '0' + frac%10;
            frac /= 10;
        }
        buf[len++] = '.';
    } else if (flags & FLAGS_HASH) {
        buf[len++] = '.';
    }
    if (!whole) 
        buf[len++] = '0';
    while (whole) {
        buf[len++] = '0' + whole%10;
        whole /= 10;
    }
    return _out_rev2(out, buffer, idx, maxlen, buf, len, width, flags, negative);
}
#if defined(PRINTF_SUPPORT_EXPONENTIAL)
//
// Given a float print the scientific notation using a specific number of significant
// digits (not precision), if FLAGS_ADAPT_EXP is set then try to print as a normal
// float first only switch to scientific if we need to.
//
// Changes compared to mpaland/printf routine:
//
// 1. Keeps padding (zero/space) out of internal buffer
// 2. Supports signficant digits up to 18.
// 3. Supports FLAGS_HASH
// 4. Max number length fits in buffer, so no length checking needed
//
static size_t _etoa(out_fct_type out, char* buffer, size_t idx, size_t maxlen, double value, unsigned int prec, unsigned int width, unsigned int flags) {
    char    buf[PRINTF_MAX_FLOAT_PRECISION + 1 + 5];  // number, plus decimal, plus e+XXX
    size_t  len = 0;
    int     hadsig = 0;         // used for trailing zero removal

    // check for NaN and special values
    if ((value != value) || (value > DBL_MAX) || (value < -DBL_MAX)) {
        return _ftoa(out, buffer, idx, maxlen, value, prec, width, flags);
    }

    // If we are a normal 'e' type output then we need an extra digit to be
    // consistent with the standard printf
    if (!(flags & FLAGS_ADAPT_EXP)) {
        prec++;
    }
    if (prec > PRINTF_MAX_FLOAT_PRECISION) {
        prec=PRINTF_MAX_FLOAT_PRECISION;
    }
    // Adjust for 0 precision requests...
    if (prec == 0) {
        prec = 1;
    }
    // Store and remove negativity
    int negative;
    _FSIGNCHECK(value, negative);

    // Handle the various verions of scientific with/without trailing zeros
    if (!(flags & FLAGS_ADAPT_EXP)) {
        // Straight scientific with trailing zeros
        hadsig = 1;
    } else if (value >= _e10((int)prec) || (value <= 1e-05 && value != 0)) {
        // Too large, so switch to scientific without trailing zeros
        flags &= ~FLAGS_ADAPT_EXP;
    }

    // Move so we are within the range 0 to <1
    int exp=0;
    int64_t whole = 0;
    while (value < 0.00001 && value > 0) {
        value *= 100000;
        exp -=5;
    }
    while (value < 0.1 && value > 0) {
        value *= 10;
        exp--;
    }
    while (value > 100000) {
        value /= 100000;
        exp += 5;
    }
    while (value >= 1) {
        value /= 10;
        exp++;
    }
    // If we are zero then fix exponential value
    if (value == 0) {
        exp++;
    }
    value *= _e10((int)prec);
    exp -= prec;
    whole = (int64_t)value;
    if ((value - whole) >= 0.5) whole++;

    // There are some strange rounding issues that cause problems
    if (whole == _e10((int)prec)) {
        whole /= 10;
        exp++;
    }
    // If we are printing in scientific notation then we need to adjust
    // the exponent accordingly and print out the e value
    if (!(flags & FLAGS_ADAPT_EXP)) {
        int exval = exp + (prec-1);
        int neg = exval < 0;
        exval = (neg ? -exval : exval);

        // Always print the lowest 2 digits (even if zero)
        buf[len++] = '0' + exval%10;
        exval /= 10;
        buf[len++] = '0' + exval%10;
        exval /= 10;
        // Print the third only if we have one
        if (exval) 
            buf[len++] = '0' + exval%10;
        // Print the sign and 'e'
        buf[len++] = (neg ? '-' : '+');
        buf[len++] = (flags & FLAGS_UPPERCASE ? 'E' : 'e');
        exp = -(prec-1);
    }

    // If we have a hash, then we don't strip zeros, and if we are
    // zero (with stripping) then make it easy.
    if (flags & FLAGS_HASH) {
        hadsig=1;
    } else if (whole == 0) {
        if (!hadsig) 
            exp = 0;
    }

    // Now output the trailing zeros for non decimals
    while (exp > 0) {
        buf[len++] = '0';
        exp--;
    }
    if ((flags & FLAGS_HASH) && exp == 0) {
        buf[len++] = '.';
    }
    if (whole == 0 && exp == 0) {
        buf[len++] = '0';
    }
    while (whole || exp < 0) {
        int n = whole%10;
        whole /= 10;
        exp++;
        if (!hadsig && n == 0 && exp < 1) continue;
        hadsig = 1;
        buf[len++] = '0' + n;
        if (exp == 0) {
            buf[len++] = '.';
            if (!whole) {
                buf[len++] = '0';
            }
        } 
    }
    return _out_rev2(out, buffer, idx, maxlen, buf, len, width, flags, negative);
}
#endif //defined(PRINTF_SUPPORT_EXPONENTIAL)

#endif //defined(PRINTF_SUPPORT_FLOAT)

// internal vsnprintf
static int _vsnprintf(out_fct_type out, char* buffer, const size_t maxlen, const char* format, va_list va)
{
  unsigned int flags, width, precision, n;
  size_t idx = 0U;

  if (!buffer) {
    // use null output function
    out = _out_null;
  }

  while (*format)
  {
    // format specifier?  %[flags][width][.precision][length]
    if (*format != '%') {
      // no
      out(*format, buffer, idx++, maxlen);
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
        const int prec = (int)va_arg(va, int);
        precision = prec > 0 ? (unsigned int)prec : 0U;
        format++;
      }
    }

    // evaluate length field
    switch (*format) {
      case 'l' :
        flags |= FLAGS_LONG;
        format++;
        if (*format == 'l') {
          flags |= FLAGS_LONG_LONG;
          format++;
        }
        break;
      case 'h' :
        flags |= FLAGS_SHORT;
        format++;
        if (*format == 'h') {
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

        // ignore '0' flag when precision is given
        if (flags & FLAGS_PRECISION) {
          flags &= ~FLAGS_ZEROPAD;
        }

        // convert the integer
        if ((*format == 'i') || (*format == 'd')) {
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
        if (*format == 'F') flags |= FLAGS_UPPERCASE;
        idx = _ftoa(out, buffer, idx, maxlen, va_arg(va, double), precision, width, flags);
        format++;
        break;
#if defined(PRINTF_SUPPORT_EXPONENTIAL)
      case 'e':
      case 'E':
      case 'g':
      case 'G':
        if ((*format == 'g')||(*format == 'G')) flags |= FLAGS_ADAPT_EXP;
        if ((*format == 'E')||(*format == 'G')) flags |= FLAGS_UPPERCASE;
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
        out(*format, buffer, idx++, maxlen);
        format++;
        break;
    }
  }

  // termination
  out((char)0, buffer, idx < maxlen ? idx : maxlen - 1U, maxlen);

  // return written chars without terminating \0
  return (int)idx;
}


///////////////////////////////////////////////////////////////////////////////

int printf_(const char* format, ...)
{
  va_list va;
  va_start(va, format);
  char buffer[1];
  const int ret = _vsnprintf(_out_char, buffer, (size_t)-1, format, va);
  va_end(va);
  return ret;
}


int sprintf_(char* buffer, const char* format, ...)
{
  va_list va;
  va_start(va, format);
  const int ret = _vsnprintf(_out_buffer, buffer, (size_t)-1, format, va);
  va_end(va);
  return ret;
}


int snprintf_(char* buffer, size_t count, const char* format, ...)
{
  va_list va;
  va_start(va, format);
  const int ret = _vsnprintf(_out_buffer, buffer, count, format, va);
  va_end(va);
  return ret;
}


int vprintf_(const char* format, va_list va)
{
  char buffer[1];
  return _vsnprintf(_out_char, buffer, (size_t)-1, format, va);
}


int vsnprintf_(char* buffer, size_t count, const char* format, va_list va)
{
  return _vsnprintf(_out_buffer, buffer, count, format, va);
}


int fctprintf(void (*out)(char character, void* arg), void* arg, const char* format, ...)
{
  va_list va;
  va_start(va, format);
  const out_fct_wrap_type out_fct_wrap = { out, arg };
  const int ret = _vsnprintf(_out_fct, (char*)(uintptr_t)&out_fct_wrap, (size_t)-1, format, va);
  va_end(va);
  return ret;
}
