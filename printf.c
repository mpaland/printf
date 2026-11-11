///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2026, PALANDesign Hannover, Germany
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
// default: 1e9
#ifndef PRINTF_MAX_FLOAT
#define PRINTF_MAX_FLOAT  1e9
#endif

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

// support for printf_() and vprintf_(), which need the _putchar() output function
// default: activated
#ifndef PRINTF_DISABLE_SUPPORT_PUTCHAR
#define PRINTF_SUPPORT_PUTCHAR
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
#define FLAGS_LONG_DOUBLE (1U << 12U)


// import float.h for DBL_MAX
#if defined(PRINTF_SUPPORT_FLOAT)
#include <float.h>
#endif


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
#if defined(PRINTF_SUPPORT_PUTCHAR)
static inline void _out_char(char character, void* buffer, size_t idx, size_t maxlen)
{
  (void)buffer; (void)idx; (void)maxlen;
  _putchar(character);
}
#endif


// internal output function wrapper
static inline void _out_fct(char character, void* buffer, size_t idx, size_t maxlen)
{
  (void)idx; (void)maxlen;
  // buffer is the output fct pointer
  ((out_fct_wrap_type*)buffer)->fct(character, ((out_fct_wrap_type*)buffer)->arg);
}


// internal secure strlen
// \return The length of the string (excluding the terminating 0) limited by 'maxsize'
static inline unsigned int _strnlen_s(const char* str, size_t maxsize)
{
  const char* s;
  for (s = str; maxsize-- && *s; ++s);   // reads no more than maxsize characters
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


// output a reversed number with additional leading zeros which did not fit into its buffer: the sign and the
// prefix (the last 'reserved' characters of the buffer) come first, then the zeros, then the digits
static size_t _out_rev_zeros(out_fct_type out, char* buffer, size_t idx, size_t maxlen, const char* buf, size_t len, size_t reserved, size_t zeros, unsigned int width, unsigned int flags)
{
  if (!zeros) {
    return _out_rev(out, buffer, idx, maxlen, buf, len, width, flags);
  }

  const size_t start_idx = idx;

  // pad spaces up to given width
  if (!(flags & FLAGS_LEFT)) {
    for (size_t i = len + zeros; i < width; i++) {
      out(' ', buffer, idx++, maxlen);
    }
  }

  idx = _out_rev(out, buffer, idx, maxlen, buf + len - reserved, reserved, 0U, flags);
  while (zeros--) {
    out('0', buffer, idx++, maxlen);
  }
  idx = _out_rev(out, buffer, idx, maxlen, buf, len - reserved, 0U, flags);

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
  // the octal prefix is a leading zero, which is not needed if the number already starts with a zero
  if ((flags & FLAGS_HASH) && (base == 8U) && ((prec > len) || (len && (buf[len - 1U] == '0')))) {
    flags &= ~FLAGS_HASH;
  }

  // room in the buffer for the sign and the prefix
  size_t reserved = (negative || (flags & (FLAGS_PLUS | FLAGS_SPACE))) ? 1U : 0U;
  if (flags & FLAGS_HASH) {
    reserved += (base == 8U) ? 1U : 2U;
  }
  const size_t limit = (reserved < PRINTF_NTOA_BUFFER_SIZE) ? PRINTF_NTOA_BUFFER_SIZE - reserved : 0U;

  // pad leading zeros up to the precision, and up to the width with the '0' flag
  // zeros which do not fit into the buffer are counted and output separately
  size_t zeros = 0U;
  while ((len < prec) && (len < limit)) {
    buf[len++] = '0';
  }
  if (len < prec) {
    zeros = prec - len;
  }
  if (!(flags & FLAGS_LEFT) && (flags & FLAGS_ZEROPAD)) {
    while ((len + zeros + reserved < width) && (len < limit)) {
      buf[len++] = '0';
    }
    if (len + zeros + reserved < width) {
      zeros = width - reserved - len;
    }
  }

  // handle hash
  if (flags & FLAGS_HASH) {
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

  return _out_rev_zeros(out, buffer, idx, maxlen, buf, len, reserved, zeros, width, flags);
}


// internal itoa for 'long' type
static size_t _ntoa_long(out_fct_type out, char* buffer, size_t idx, size_t maxlen, unsigned long value, bool negative, unsigned long base, unsigned int prec, unsigned int width, unsigned int flags)
{
  char buf[PRINTF_NTOA_BUFFER_SIZE];
  size_t len = 0U;
  // the bases 2, 8 and 16 are powers of two: mask and shift instead of a division
  const unsigned int shift = (base == 16U) ? 4U : (base == 8U) ? 3U : (base == 2U) ? 1U : 0U;

  // no hash for 0 values, except for octal where it forces a leading zero
  if (!value && (base != 8U)) {
    flags &= ~FLAGS_HASH;
  }

  // write if precision != 0 and value is != 0
  if (!(flags & FLAGS_PRECISION) || value) {
    do {
      char digit;
      if (shift) {
        digit = (char)(value & (base - 1U));
        value >>= shift;
      }
      else {
        // a constant divisor, which the compiler can replace by a multiplication,
        // and only one division for the quotient and the remainder
        const unsigned long next = value / 10U;
        digit = (char)(value - next * 10U);
        value = next;
      }
      buf[len++] = digit < 10 ? '0' + digit : (flags & FLAGS_UPPERCASE ? 'A' : 'a') + digit - 10;
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
  // the bases 2, 8 and 16 are powers of two: mask and shift instead of a division
  const unsigned int shift = (base == 16U) ? 4U : (base == 8U) ? 3U : (base == 2U) ? 1U : 0U;

  // no hash for 0 values, except for octal where it forces a leading zero
  if (!value && (base != 8U)) {
    flags &= ~FLAGS_HASH;
  }

  // write if precision != 0 and value is != 0
  if (!(flags & FLAGS_PRECISION) || value) {
    do {
      char digit;
      if (shift) {
        digit = (char)(value & (base - 1U));
        value >>= shift;
      }
      else {
        // a constant divisor, which the compiler can replace by a multiplication,
        // and only one division for the quotient and the remainder
        const unsigned long long next = value / 10U;
        digit = (char)(value - next * 10U);
        value = next;
      }
      buf[len++] = digit < 10 ? '0' + digit : (flags & FLAGS_UPPERCASE ? 'A' : 'a') + digit - 10;
    } while (value && (len < PRINTF_NTOA_BUFFER_SIZE));
  }

  return _ntoa_format(out, buffer, idx, maxlen, buf, len, negative, (unsigned int)base, prec, width, flags);
}
#endif  // PRINTF_SUPPORT_LONG_LONG


#if defined(PRINTF_SUPPORT_FLOAT)

// powers of 10
static const double _pow10[] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000 };


// internal test for a negative value, which also detects -0.0 (comparing equal to 0.0)
static inline bool _is_negative(double value)
{
#if DBL_MANT_DIG == 24
  // double is a 32-bit IEEE 754 type
  union {
    uint32_t U;
    double   F;
  } conv;
  conv.F = value;
  return (conv.U >> 31U) != 0U;
#else
  union {
    uint64_t U;
    double   F;
  } conv;
  conv.F = value;
  return (conv.U >> 63U) != 0U;
#endif
}


#if defined(PRINTF_SUPPORT_EXPONENTIAL)
// forward declaration so that _ftoa can switch to exp notation for values > PRINTF_MAX_FLOAT
static size_t _etoa(out_fct_type out, char* buffer, size_t idx, size_t maxlen, double value, unsigned int prec, unsigned int width, unsigned int flags);
#endif


// internal ftoa for fixed decimal floating point
static size_t _ftoa(out_fct_type out, char* buffer, size_t idx, size_t maxlen, double value, unsigned int prec, unsigned int width, unsigned int flags)
{
  char buf[PRINTF_FTOA_BUFFER_SIZE];
  size_t len  = 0U;
  double diff = 0.0;

  // test for special values: "inf" and "nan" like glibc, uppercase for %F, %E and %G, with the sign and the
  // space flag, and padded with spaces as the '0' flag is ignored for them
  const char* special = NULL;
  if (!((value >= -DBL_MAX) && (value <= DBL_MAX))) {
    const bool is_nan = !(value >= -DBL_MAX) && !(value <= DBL_MAX);  // NaN is the only value which compares false both ways
    special = (flags & FLAGS_UPPERCASE) ? (is_nan ? "NAN" : "INF") : (is_nan ? "nan" : "inf");
  }
#if !defined(PRINTF_SUPPORT_EXPONENTIAL)
  // test for very large values, see below: without exponential notation "ovf" (overflow) is printed like a special value
  else if ((value > PRINTF_MAX_FLOAT) || (value < -PRINTF_MAX_FLOAT)) {
    special = (flags & FLAGS_UPPERCASE) ? "OVF" : "ovf";
  }
#endif
  if (special) {
    buf[len++] = special[2];
    buf[len++] = special[1];
    buf[len++] = special[0];
    if (_is_negative(value)) {
      buf[len++] = '-';
    }
    else if (flags & FLAGS_PLUS) {
      buf[len++] = '+';
    }
    else if (flags & FLAGS_SPACE) {
      buf[len++] = ' ';
    }
    return _out_rev(out, buffer, idx, maxlen, buf, len, width, flags & ~FLAGS_ZEROPAD);
  }

#if defined(PRINTF_SUPPORT_EXPONENTIAL)
  // test for very large values
  // standard printf behavior is to print EVERY whole number digit -- which could be 100s of characters overflowing your buffers == bad
  if ((value > PRINTF_MAX_FLOAT) || (value < -PRINTF_MAX_FLOAT)) {
    return _etoa(out, buffer, idx, maxlen, value, prec, width, flags);
  }
#endif

  // test for negative
  const bool negative = _is_negative(value);
  if (negative) {
    value = 0 - value;
  }

  // set default precision, if not set explicitly
  if (!(flags & FLAGS_PRECISION)) {
    prec = PRINTF_DEFAULT_FLOAT_PRECISION;
  }
  // in "%g" mode trailing zeros are removed, unless the '#' flag is given
  const bool strip_zeros = (flags & FLAGS_ADAPT_EXP) && !(flags & FLAGS_HASH);
  if (strip_zeros && (prec > 9U)) {
    prec = 9U;
  }
  // limit precision to 9, cause a prec >= 10 can lead to overflow errors
  while ((len < PRINTF_FTOA_BUFFER_SIZE) && (prec > 9U)) {
    buf[len++] = '0';
    prec--;
  }

  // the whole part is at most PRINTF_MAX_FLOAT, which doesn't fit into a 16-bit int
  unsigned long whole = (unsigned long)value;
  double tmp = (value - (double)whole) * _pow10[prec];
  unsigned long frac = (unsigned long)tmp;
  diff = tmp - (double)frac;

  if (diff > 0.5) {
    ++frac;
  }
  else if (diff < 0.5) {
  }
  else if ((prec > 0U) && ((frac == 0U) || (frac & 1U))) {
    // if halfway, round up if odd OR if last digit is 0 (precision 0 is handled below)
    ++frac;
  }
  // handle rollover, e.g. case 0.99 with prec 1 is 1.0
  if (frac >= _pow10[prec]) {
    frac = 0;
    ++whole;
  }
  if (strip_zeros) {
    while (prec && !(frac % 10U)) {
      frac /= 10U;
      --prec;
    }
  }

  if (prec == 0U) {
    diff = value - (double)whole;
    if ((!(diff < 0.5) || (diff > 0.5)) && (whole & 1U)) {
      // exactly 0.5 and ODD, then round up
      // 1.5 -> 2, but 2.5 -> 2
      ++whole;
    }
    if ((flags & FLAGS_HASH) && (len < PRINTF_FTOA_BUFFER_SIZE)) {
      // the '#' flag forces the decimal point
      buf[len++] = '.';
    }
  }
  else {
    unsigned int count = prec;
    // now do fractional part, as an unsigned number
    while (len < PRINTF_FTOA_BUFFER_SIZE) {
      --count;
      buf[len++] = (char)(48U + (frac % 10U));
      if (!(frac /= 10U)) {
        break;
      }
    }
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
    buf[len++] = (char)(48U + (whole % 10U));
    if (!(whole /= 10U)) {
      break;
    }
  }

  // pad leading zeros up to the width, leaving room for the sign
  // zeros which do not fit into the buffer are counted and output separately
  const size_t reserved = (negative || (flags & (FLAGS_PLUS | FLAGS_SPACE))) ? 1U : 0U;
  size_t zeros = 0U;
  if (!(flags & FLAGS_LEFT) && (flags & FLAGS_ZEROPAD)) {
    while ((len + reserved < width) && (len + reserved < PRINTF_FTOA_BUFFER_SIZE)) {
      buf[len++] = '0';
    }
    if (len + reserved < width) {
      zeros = width - reserved - len;
    }
  }

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

  return _out_rev_zeros(out, buffer, idx, maxlen, buf, len, reserved, zeros, width, flags);
}


#if defined(PRINTF_SUPPORT_EXPONENTIAL)
// internal ftoa variant for exponential floating-point type, contributed by Martijn Jasperse <m.jasperse@gmail.com>
static size_t _etoa(out_fct_type out, char* buffer, size_t idx, size_t maxlen, double value, unsigned int prec, unsigned int width, unsigned int flags)
{
  // powers of 10 to scale the value: 10^(2^i)
  static const double pow10_2i[] = { 1e1, 1e2, 1e4, 1e8, 1e16, 1e32
#if DBL_MAX_10_EXP >= 256
                                   , 1e64, 1e128, 1e256
#endif
  };

  // check for NaN and special values
  if (!((value >= -DBL_MAX) && (value <= DBL_MAX))) {
    return _ftoa(out, buffer, idx, maxlen, value, prec, width, flags);
  }

  // determine the sign
  const bool negative = _is_negative(value);
  if (negative) {
    value = -value;
  }

  // default precision
  if (!(flags & FLAGS_PRECISION)) {
    prec = PRINTF_DEFAULT_FLOAT_PRECISION;
  }
  // in "%g" mode the precision is the number of significant digits (0 is taken as 1), but from
  // here on "prec" is the number of decimals of the mantissa in exponential notation
  if ((flags & FLAGS_ADAPT_EXP) && prec) {
    --prec;
  }
  flags |= FLAGS_PRECISION;  // make sure _ftoa respects the precision

  // determine the decimal exponent and scale the value into [1, 10), 0 keeps the exponent 0
  int expval = 0;
  double mantissa = value;
  double scale = 1.0;  // 10^expval for values >= 10, exact for values < 2^DBL_MANT_DIG
  int i;
  if (mantissa >= 10.0) {
    for (i = (int)(sizeof(pow10_2i) / sizeof(pow10_2i[0])) - 1; i >= 0; --i) {
      if (mantissa >= pow10_2i[i]) {
        mantissa /= pow10_2i[i];
        scale *= pow10_2i[i];
        expval += 1 << i;
      }
    }
  }
  else if ((mantissa < 1.0) && (mantissa > 0.0)) {
    // this also works for subnormal values
    for (i = (int)(sizeof(pow10_2i) / sizeof(pow10_2i[0])) - 1; i >= 0; --i) {
      if (mantissa * pow10_2i[i] < 10.0) {
        mantissa *= pow10_2i[i];
        expval -= 1 << i;
      }
    }
  }

  // round the mantissa to the precision (at most 9 digits are significant, see _ftoa)
  const unsigned int mprec = (prec > 9U) ? 9U : prec;
  if ((expval > 0) && (value < 2.0 / DBL_EPSILON)) {
    // the value has an exact integer part: round the remainder after the first digit in the
    // unscaled domain, so that exact decimal ties like 92523.5 with precision 4 round correctly
    unsigned int digit = (mantissa < 9.0) ? (unsigned int)mantissa : 9U;
    double rem = value - digit * scale;
    if (rem < 0.0) {
      // the scaled mantissa was rounded up to the next digit
      --digit;
      rem += scale;
    }
    rem = ((unsigned int)expval > mprec) ? rem / (scale / _pow10[mprec]) : rem * _pow10[mprec - (unsigned int)expval];
    unsigned long frac = (unsigned long)rem;
    const double diff = rem - (double)frac;
    if ((diff > 0.5) || (!(diff < 0.5) && ((mprec ? frac : digit) & 1U))) {
      // round half to even
      ++frac;
    }
    if (frac >= _pow10[mprec]) {
      frac = 0U;
      ++digit;
    }
    if (digit > 9U) {
      digit = 1U;
      expval++;
    }
    mantissa = digit + frac / _pow10[mprec];
  }
  else if (mantissa >= 10.0 - 0.5 / _pow10[mprec]) {
    // rounding the mantissa to the precision results in 10, e.g. 9.9996 with precision 3
    mantissa /= 10.0;
    expval++;
  }

  // in "%g" mode, use the fixed point notation if P > X >= -4, where P is the number of significant
  // digits and X the exponent (C11 7.21.6.1), unless the value is too large for _ftoa
  if ((flags & FLAGS_ADAPT_EXP) && (expval <= (int)prec) && (expval >= -4) && !(value > PRINTF_MAX_FLOAT)) {
    return _ftoa(out, buffer, idx, maxlen, negative ? -value : value, (unsigned int)((int)prec - expval), width, flags);
  }

  // the exponent format is "%+03d" and the largest value is "324" (subnormal), so set aside 4-5 characters
  const unsigned int minwidth = ((expval < 100) && (expval > -100)) ? 4U : 5U;

  // will everything fit?
  unsigned int fwidth = width;
  if (width > minwidth) {
    // subtract the characters required for the exponent
    fwidth -= minwidth;
  } else {
    // not enough characters, so go back to default sizing
    fwidth = 0U;
  }
  if (flags & FLAGS_LEFT) {
    // if we're padding on the right, DON'T pad the floating part
    fwidth = 0U;
  }

  // output the floating part
  const size_t start_idx = idx;
  idx = _ftoa(out, buffer, idx, maxlen, negative ? -mantissa : mantissa, prec, fwidth, flags);

  // output the exponential symbol
  out((flags & FLAGS_UPPERCASE) ? 'E' : 'e', buffer, idx++, maxlen);
  // output the exponent value
  idx = _ntoa_long(out, buffer, idx, maxlen, (unsigned long)((expval < 0) ? -expval : expval), expval < 0, 10, 0, minwidth-1, FLAGS_ZEROPAD | FLAGS_PLUS);
  // might need to right-pad spaces
  if (flags & FLAGS_LEFT) {
    while (idx - start_idx < width) out(' ', buffer, idx++, maxlen);
  }
  return idx;
}
#endif  // PRINTF_SUPPORT_EXPONENTIAL
#endif  // PRINTF_SUPPORT_FLOAT


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
        case '\'':                        format++; n = 1U; break;  // thousands grouping (POSIX), accepted and ignored
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
        width = 0U - (unsigned int)w;   // negated as unsigned, INT_MIN cannot be negated as an int
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
        const int prec = va_arg(va, int);
        if (prec >= 0) {
          precision = (unsigned int)prec;
        }
        else {
          flags &= ~FLAGS_PRECISION;  // a negative precision is taken as if it were omitted
        }
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
      case 'L' :
        flags |= FLAGS_LONG_DOUBLE;
        format++;
        break;
      default :
        break;
    }

    // an incomplete format specifier at the end of the format string ends the output, like glibc does
    if (*format == '\0') {
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
            idx = _ntoa_long_long(out, buffer, idx, maxlen, (value > 0) ? (unsigned long long)value : 0U - (unsigned long long)value, value < 0, base, precision, width, flags);
#else
            // without long long support the argument is consumed and printed truncated to long
            const long value = (long)va_arg(va, long long);
            idx = _ntoa_long(out, buffer, idx, maxlen, (value > 0) ? (unsigned long)value : 0U - (unsigned long)value, value < 0, base, precision, width, flags);
#endif
          }
          else if (flags & FLAGS_LONG) {
            const long value = va_arg(va, long);
            idx = _ntoa_long(out, buffer, idx, maxlen, (value > 0) ? (unsigned long)value : 0U - (unsigned long)value, value < 0, base, precision, width, flags);
          }
          else {
            const int value = (flags & FLAGS_CHAR) ? (signed char)va_arg(va, int) : (flags & FLAGS_SHORT) ? (short int)va_arg(va, int) : va_arg(va, int);
            idx = _ntoa_long(out, buffer, idx, maxlen, (value > 0) ? (unsigned int)value : 0U - (unsigned int)value, value < 0, base, precision, width, flags);
          }
        }
        else {
          // unsigned
          if (flags & FLAGS_LONG_LONG) {
#if defined(PRINTF_SUPPORT_LONG_LONG)
            idx = _ntoa_long_long(out, buffer, idx, maxlen, va_arg(va, unsigned long long), false, base, precision, width, flags);
#else
            // without long long support the argument is consumed and printed truncated to long
            idx = _ntoa_long(out, buffer, idx, maxlen, (unsigned long)va_arg(va, unsigned long long), false, base, precision, width, flags);
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
        idx = _ftoa(out, buffer, idx, maxlen, (flags & FLAGS_LONG_DOUBLE) ? (double)va_arg(va, long double) : va_arg(va, double), precision, width, flags);
        format++;
        break;
#if defined(PRINTF_SUPPORT_EXPONENTIAL)
      case 'e':
      case 'E':
      case 'g':
      case 'G':
        if ((*format == 'g')||(*format == 'G')) flags |= FLAGS_ADAPT_EXP;
        if ((*format == 'E')||(*format == 'G')) flags |= FLAGS_UPPERCASE;
        idx = _etoa(out, buffer, idx, maxlen, (flags & FLAGS_LONG_DOUBLE) ? (double)va_arg(va, long double) : va_arg(va, double), precision, width, flags);
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
        if (!p) {
          // printing a NULL pointer is undefined behavior, print it like other implementations
          p = "(null)";
          flags &= ~FLAGS_PRECISION;
        }
        // the precision limits the characters read, the string needs no terminator then
        const unsigned int len = _strnlen_s(p, (flags & FLAGS_PRECISION) ? precision : (size_t)-1);
        unsigned int l = len;
        // pre padding
        if (!(flags & FLAGS_LEFT)) {
          while (l++ < width) {
            out(' ', buffer, idx++, maxlen);
          }
        }
        // string output
        for (unsigned int i = 0U; i < len; i++) {
          out(p[i], buffer, idx++, maxlen);
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
        // print like glibc: '0x' prefix and lowercase hex digits, "(nil)" for a NULL pointer
        const uintptr_t value = (uintptr_t)va_arg(va, void*);
        if (value == (uintptr_t)NULL) {
          // padded to the width like a string
          idx = _out_rev(out, buffer, idx, maxlen, ")lin(", 5U, width, flags);
        }
        else {
          // the digits are zero padded to the width, or to all digits of the pointer
          width = (width ? width : sizeof(void*) * 2U) + 2U;
          flags |= FLAGS_ZEROPAD | FLAGS_HASH;
#if defined(PRINTF_SUPPORT_LONG_LONG)
          const bool is_ll = sizeof(uintptr_t) == sizeof(long long);
          if (is_ll) {
            idx = _ntoa_long_long(out, buffer, idx, maxlen, value, false, 16U, precision, width, flags);
          }
          else {
#endif
            idx = _ntoa_long(out, buffer, idx, maxlen, (unsigned long)value, false, 16U, precision, width, flags);
#if defined(PRINTF_SUPPORT_LONG_LONG)
          }
#endif
        }
        format++;
        break;
      }

      case '%' :
        out('%', buffer, idx++, maxlen);
        format++;
        break;

      // the unsupported hex float conversion and the floating point conversions without floating point or
      // exponential support are printed like an unknown conversion, but their argument is consumed so that the
      // following arguments are not shifted
      case 'a' :
      case 'A' :
#if !defined(PRINTF_SUPPORT_FLOAT)
      case 'f' :
      case 'F' :
#endif
#if !defined(PRINTF_SUPPORT_FLOAT) || !defined(PRINTF_SUPPORT_EXPONENTIAL)
      case 'e' :
      case 'E' :
      case 'g' :
      case 'G' :
#endif
        if (flags & FLAGS_LONG_DOUBLE) {
          (void)va_arg(va, long double);
        }
        else {
          (void)va_arg(va, double);
        }
        out(*format, buffer, idx++, maxlen);
        format++;
        break;

      // %n is not supported for security reasons: nothing is stored, the argument is consumed and nothing is printed
      case 'n' :
        (void)va_arg(va, void*);
        format++;
        break;

      default :
        out(*format, buffer, idx++, maxlen);
        format++;
        break;
    }
  }

  // termination, only for buffer output (a NUL printed by %c is a regular character)
  if (out == _out_buffer) {
    out((char)0, buffer, idx < maxlen ? idx : maxlen - 1U, maxlen);
  }

  // return written chars without terminating \0
  return (int)idx;
}


///////////////////////////////////////////////////////////////////////////////

#if defined(PRINTF_SUPPORT_PUTCHAR)
int printf_(const char* format, ...)
{
  va_list va;
  va_start(va, format);
  char buffer[1];
  const int ret = _vsnprintf(_out_char, buffer, (size_t)-1, format, va);
  va_end(va);
  return ret;
}
#endif


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


#if defined(PRINTF_SUPPORT_PUTCHAR)
int vprintf_(const char* format, va_list va)
{
  char buffer[1];
  return _vsnprintf(_out_char, buffer, (size_t)-1, format, va);
}
#endif


int vsnprintf_(char* buffer, size_t count, const char* format, va_list va)
{
  return _vsnprintf(_out_buffer, buffer, count, format, va);
}


int fctprintf(void (*out)(char character, void* arg), void* arg, const char* format, ...)
{
  va_list va;
  va_start(va, format);
  const int ret = vfctprintf(out, arg, format, va);
  va_end(va);
  return ret;
}


int vfctprintf(void (*out)(char character, void* arg), void* arg, const char* format, va_list va)
{
  const out_fct_wrap_type out_fct_wrap = { out, arg };
  return _vsnprintf(_out_fct, (char*)(uintptr_t)&out_fct_wrap, (size_t)-1, format, va);
}
