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
#include <assert.h>
#include <math.h>

#include "printf.h"


// define this globally (e.g. gcc -DPRINTF_INCLUDE_CONFIG_H ...) to include the
// printf_config.h header file
// default: undefined
#ifdef PRINTF_INCLUDE_CONFIG_H
#include "printf_config.h"
#endif


// 'ntoa' conversion buffer size, this must be big enough to hold one converted
// numeric/float number including padded zeros (dynamically created on stack)
// default: 32 byte
#ifndef PRINTF_NTOA_BUFFER_SIZE
#define PRINTF_NTOA_BUFFER_SIZE    32U
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
#define FLAGS_NEGATIVE  (1U << 12U)

// import float.h for DBL_MAX
#if defined(PRINTF_SUPPORT_FLOAT)
#include <float.h>

// implement fmsub without math library
// used code from https://stackoverflow.com/questions/28630864/how-is-fma-implemented
// c is close to a*b, so this algorithm should work correctly
// FPU may have fma instruction, using it (through libm) will be much faster
# ifndef USE_MATH_H
struct doubledouble { double hi; double lo; };
union udbl { double f; uint64_t i;}  ;
static struct doubledouble split(double a) {
    union udbl lo, hi = {a};
    hi.i &= ~(((uint64_t)1U << (DBL_MANT_DIG / 2)) - 1);  // mask low-order mantissa bits
    lo.f = a - hi.f;
    return (struct doubledouble){hi.f,lo.f};
}

double fmsub(double a, double b, double c) {
    struct doubledouble as = split(a), bs = split(b);
    return ((as.hi*bs.hi - c) + as.hi*bs.lo + as.lo*bs.hi) + as.lo*bs.lo;
}

# else /* ifndef USE_MATH_H */

#include <math.h>
static double fmsub(double a, double b, double c)
{
  return fma(a,b,-c);
}
# endif /* ifndef USE_MATH_H */
#endif /* defined(PRINTF_SUPPORT_FLOAT) */

#if !defined(__GNUC__)
// ISO C version, but no type checking
#define container_of(ptr, type, member) \
                      ((type *) ((char *)(ptr) - offsetof(type, member)))
#else
// non ISO variant from linux kernel; checks ptr type, but triggers 'ISO C forbids braced-groups within expressions [-Wpedantic]'
//  __extension__ is here to disable this warning
#define container_of(ptr, type, member)  ( __extension__ ({         \
        const __typeof__( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );}))      \
  /**/
#endif

#ifndef MAX
# define MAX(a,b) ((a) < (b) ? (b) : (a))
#endif
#ifndef MIN
# define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif


// type used for index, used as buffer pointer for s(n)printf
typedef uintptr_t idx_t;

// isnan is C99 macro, use it if defined
#ifndef isnan
// avoid gcc warning
# define isnan(v) ((v) != (v))
#endif

#if 0
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
bool iszero(double v) {
  return v == 0;
}
#pragma GCC diagnostic pop
#else
#define iszero(v) (                                                     \
    __extension__  ({                                                   \
        _Pragma("GCC diagnostic push");                                 \
        _Pragma("GCC diagnostic ignored \"-Wfloat-equal\"");            \
        ((v) == 0);                                        \
        _Pragma("GCC diagnostic pop");                                  \
      }))                                                               \
/**/
#endif

// output function structure, passed as fisrt argument on ofn call
// see how out_userfct augments this structure
struct out_base {
  void (*ofn)(struct out_base const* out, char character, idx_t idx);
};

// state used in printf processing
// can't be merged with out_base, it would disable a lot of optimizations
struct printf_state {
  const struct out_base* out;
  idx_t maxidx;
  unsigned short width;
  unsigned short base;
  unsigned short prec;
  unsigned short flags;
};


// sprintf/snprinf out function. idx is reused as pointer into buffer
static void _out_buffer_fn(struct out_base const* out, char character, idx_t idx)
{
  (void)out;
  *(char*)idx = character;
}
static const struct out_base out_buffer = { _out_buffer_fn };

// internal _putchar wrapper
static void _out_putchar_fn(struct out_base const* out, char character,  idx_t idx)
{
  (void)out; (void)idx;
  _putchar(character);
}

static const struct out_base out_putchar = { _out_putchar_fn };

struct out_userfct {
  struct out_base out_base;
  void  (*fct)(char character, void* arg);
  void* arg;
};

static void _out_userfct_fn(struct out_base const* out, char character,  idx_t idx)
{
  struct out_userfct* self = container_of(out, struct out_userfct, out_base);
  (void)idx;
  self->fct(character, self->arg);
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

static inline void _out(struct printf_state* st, char character, idx_t idx)
{
  if (idx < st->maxidx) {
    st->out->ofn(st->out, character, idx);
  }
}

// add count padchar characters, return adjusted index
// count can be negative
static idx_t _out_pad(struct printf_state* st, char padchar, idx_t idx, int count)
{
  if (count <= 0)
    return idx;

  const idx_t top = idx + (unsigned)count;
  const idx_t end = MIN(top, st->maxidx);
  const struct out_base * const out = st->out;   // cache it in register
  while(idx < end)
    out->ofn(out, padchar, idx++);
  return top;
}

// output the specified string in reverse, taking care of any space-padding
// zero-padding is already performed in buf
static idx_t _out_rev(struct printf_state* st, idx_t idx, const char* buf, size_t len)
{
  const int pad = st->width - len;

  // pad spaces up to given width
  if (!(st->flags & FLAGS_LEFT)) {
    idx = _out_pad(st, ' ', idx, pad);
  }

  // reverse string
  // TODO - optimize this loop like out_pad
  while (len) {
    _out(st, buf[--len], idx++);
  }

  // append pad spaces up to given width
  if ((st->flags & FLAGS_LEFT)) {
    idx = _out_pad(st, ' ', idx, pad);
  }

  return idx;
}


// internal itoa format
static idx_t _ntoa_format(struct printf_state* st, idx_t idx, char* buf, size_t len)
{
  // pad leading zeros
  unsigned zeropad = 0;
  if (st->prec || st->flags & FLAGS_PRECISION) {
    // no other source of zero-padding if prec is specified (FLAGS_PRECISION may be unset)
    zeropad = st->prec;
  }
  else if (st->width && (st->flags & FLAGS_ZEROPAD)) {
    zeropad = st->width;
    if (st->flags & (FLAGS_NEGATIVE | FLAGS_PLUS | FLAGS_SPACE)) {
      // keep one space for sign
      zeropad--;
    }
    if (st->flags & FLAGS_HASH) {
      // keep space for 0x / 0b
      // octal is handled separatelly
      if (zeropad > 1 && (st->base == 16U || st->base == 2U)) {
        zeropad -= 2;
      }
    }
  }

  while (len < MIN(zeropad, PRINTF_NTOA_BUFFER_SIZE)) {
    buf[len++] = '0';
  }

  // handle hash
  if (st->flags & FLAGS_HASH) {
    switch (st->base) {
      case 2U:
        if (len < PRINTF_NTOA_BUFFER_SIZE)
          buf[len++] = 'b';
        if (len < PRINTF_NTOA_BUFFER_SIZE)
          buf[len++] = '0';
        break;
      case 8U:
        // output zero prefix, but only if last digit/pad is not zero
        if ((len < PRINTF_NTOA_BUFFER_SIZE)
            && (!len || buf[len - 1] != '0') )
          buf[len++] = '0';
        break;
      case 16U:
        if (len < PRINTF_NTOA_BUFFER_SIZE)
          buf[len++] = (st->flags & FLAGS_UPPERCASE) ? 'X' : 'x';
        if (len < PRINTF_NTOA_BUFFER_SIZE)
          buf[len++] = '0';
        break;
      default:;
    }
  }

  if (len < PRINTF_NTOA_BUFFER_SIZE) {
    if (st->flags & FLAGS_NEGATIVE) {
      buf[len++] = '-';
    }
    else if (st->flags & FLAGS_PLUS) {
      buf[len++] = '+';  // ignore the space if the '+' exists
    }
    else if (st->flags & FLAGS_SPACE) {
      buf[len++] = ' ';
    }
  }

  return _out_rev(st, idx, buf, len);
}


// long/long long itoa code
#define NTOA_CODE                                                       \
  char buf[PRINTF_NTOA_BUFFER_SIZE];                                    \
  size_t len = 0U;                                                      \
                                                                        \
  /* no hash for 0 values, except octal 0 */                            \
  if (!value && st->base !=8) {                                         \
    st->flags &= ~FLAGS_HASH;                                           \
  }                                                                     \
                                                                        \
  /* if precision is specified and zero, don't print zero value */      \
  /* if precision > 0, zero padding code will supply zeroes */          \
  if (!(st->flags & FLAGS_PRECISION) || value) {                        \
    unsigned base = st->base;                                           \
    do {                                                                \
      const char digit = (char)(value % base);                          \
      buf[len++] = digit < 10 ? '0' + digit :                           \
        (st->flags & FLAGS_UPPERCASE ? 'A' : 'a') -10 + digit;          \
      value /= base;                                                    \
    } while (value && (len < PRINTF_NTOA_BUFFER_SIZE));                 \
  }                                                                     \
  return _ntoa_format(st, idx, buf, len);                               \
  /**/

// internal itoa for 'long' type
static idx_t _ntoa_long(struct printf_state* st, idx_t idx, unsigned long value)
{
  NTOA_CODE;
}

#if defined(PRINTF_SUPPORT_LONG_LONG)
// internal itoa for 'long long' type
static idx_t _ntoa_long_long(struct printf_state* st, idx_t idx, unsigned long long value)
{
  NTOA_CODE;
}
#endif  // PRINTF_SUPPORT_LONG_LONG


#if defined(PRINTF_SUPPORT_FLOAT)

#if defined(PRINTF_SUPPORT_EXPONENTIAL)
// forward declaration so that _ftoa can switch to exp notation for values > PRINTF_MAX_FLOAT
static idx_t _etoa(struct printf_state* st, idx_t idx, double value);
#endif


// internal ftoa for fixed decimal floating point
static idx_t _ftoa(struct printf_state* st, idx_t idx, double value)
{
  char buf[PRINTF_NTOA_BUFFER_SIZE];
  size_t len  = 0U;

  // powers of 10
  static const double pow10[] = { 1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9 };

  // test for special values
  if (isnan(value))
    return _out_rev(st, idx, "nan", 3);
  if (value < -DBL_MAX)
    return _out_rev(st, idx, "fni-", 4);
  if (value > DBL_MAX)
    return _out_rev(st, idx, (st->flags & FLAGS_PLUS) ? "fni+" : "fni", (st->flags & FLAGS_PLUS) ? 4U : 3U);

  // test for very large values
  // standard printf behavior is to print EVERY whole number digit -- which could be 100s of characters overflowing your buffers == bad
  // TODO - emit string indicating overflow (possibly as  +-inf)
  if ((value > PRINTF_MAX_FLOAT) || (value < -PRINTF_MAX_FLOAT)) {
#if defined(PRINTF_SUPPORT_EXPONENTIAL)
    return _etoa(st, idx, value);
#else
    return 0U;
#endif
  }

  // test for negative
  // if FLAGS_NEGATIVE was passed, print value as negative (used in _etoa)
  const bool negative = value < 0;
  if (negative) {
    value = -value;
    st->flags |= FLAGS_NEGATIVE;
  }

  unsigned prec = (st->flags & FLAGS_PRECISION) ? st->prec : PRINTF_DEFAULT_FLOAT_PRECISION;
  // limit precision to 9, cause a prec >= 10 can lead to overflow errors (if using 32bit integer type)
  while ((len < PRINTF_NTOA_BUFFER_SIZE) && (prec > 9U)) {
    buf[len++] = '0';
    prec--;
  }

  // use signed value for double/int conversions, some CPUs don't support unsigned conversion, making the code larger
  unsigned long whole = (unsigned long)(long)value;
  // calculation tmp/frac is safe - only whole part is subtracted
  double fracdbl = value - (long)whole;
  unsigned long frac = (unsigned long)(long)(fracdbl * pow10[prec]);
  // we need better accuracy to calculate diff - * pow10 provides inaccurate result
  // using fused multiply accumulate gets correct value
  double diff = fmsub(fracdbl, pow10[prec], (double)(long)frac + 0.5);

  if (diff < 0) {
    // round down
  }
  else if (diff > 0) {
    ++frac;
  }
  else {
    // half-way, round to even
    if (prec == 0U) {
      whole = (whole + 1) & ~1U;  // round whole to even
    } else {
      frac = (frac + 1) & ~1U;    // round frac to even
    }
  }
  // handle rollover, e.g. case 0.99 with prec 1 is 1.0
  if (frac >= pow10[prec]) {
    frac = 0;
    ++whole;
  }

  if (prec > 0U) {
    unsigned int count = prec;
    // now do fractional part
    // digits(frac) <= prec
    while (len < PRINTF_NTOA_BUFFER_SIZE && frac) {
      --count;
      buf[len++] = (char)((unsigned)'0' + (frac % 10U));
      frac /= 10U;
    }
    // add extra 0s
    while ((len < PRINTF_NTOA_BUFFER_SIZE) && (count-- > 0U)) {
      buf[len++] = '0';
    }
    if (len < PRINTF_NTOA_BUFFER_SIZE) {
      // add decimal
      buf[len++] = '.';
    }
  } else if (st->flags & FLAGS_HASH) {
    if (len < PRINTF_NTOA_BUFFER_SIZE) {
      // add decimal point if precision is zero and hash flag is set
      buf[len++] = '.';
    }
  }

  // do whole part, number is reversed
    buf[len++] = (char)((unsigned)'0' + (whole % 10U));
  while (len < PRINTF_NTOA_BUFFER_SIZE) {
    if (!(whole /= 10U)) {  // output at least one zero
      break;
    }
  }
  // maybe we printed trailing zeroes for %g (not %#g) - erase them
  char *bp = buf;
    if ((st->flags & FLAGS_ADAPT_EXP) && !(st->flags & FLAGS_HASH)) {
    while(len && (*bp == '0')) {
      bp++; len--;
    }
    // '.' too
    if(len && (*bp == '.')) {
       bp++; len--;
    }
  }
  st->flags &= ~ (FLAGS_HASH|FLAGS_PRECISION);
  st->prec = 0;  // precision is not used for decimal point, not zero padding in %f
  return _ntoa_format(st, idx, bp, len);
}


#if defined(PRINTF_SUPPORT_EXPONENTIAL)
// internal ftoa variant for exponential floating-point type, contributed by Martijn Jasperse <m.jasperse@gmail.com>
static size_t _etoa(struct printf_state* st, idx_t idx,  double value)
{
  // check for NaN and special values
  if (isnan(value) || (value > DBL_MAX) || (value < -DBL_MAX)) {
    return _ftoa(st, idx, value);
  }

  // determine the sign
  const bool negative = value < 0;
  if (negative) {
    value = -value;
    st->flags |= FLAGS_NEGATIVE;
  }

  unsigned prec = (st->flags & FLAGS_PRECISION) ? st->prec : PRINTF_DEFAULT_FLOAT_PRECISION;

  // determine the decimal exponent
  // based on the algorithm by David Gay (https://www.ampl.com/netlib/fp/dtoa.c)
  union {
    uint64_t U;
    double   F;
  } conv;
  int expval;

  if (!iszero(value)) {
    conv.F = value;
    int exp2 = (int)((conv.U >> 52U) & 0x07FFU) - 1023;           // effectively log2
    conv.U = (conv.U & ((1ULL << 52U) - 1U)) | (1023ULL << 52U);  // drop the exponent so conv.F is now in [1,2)
    // now approximate log10 from the log2 integer part and an expansion of ln around 1.5
    expval = (int)(0.1760912590558 + exp2 * 0.301029995663981 + (conv.F - 1.5) * 0.289529654602168);
    // now we want to compute 10^expval but we want to be sure it won't overflow
    exp2 = (int)(expval * 3.321928094887362 + 0.5);
    const double z  = expval * 2.302585092994046 - exp2 * 0.6931471805599453;
    const double z2 = z * z;
    conv.U = (uint64_t)(exp2 + 1023) << 52U;
    // compute exp(z) using continued fractions, see https://en.wikipedia.org/wiki/Exponential_function#Continued_fractions_for_ex
    conv.F *= 1 + 2 * z / (2 - z + (z2 / (6 + (z2 / (10 + z2 / 14)))));
    // correct for rounding errors
    if (value < conv.F) {
      expval--;
      conv.F /= 10;
    }
  } else {
    // treat zero as 0.0e0
    conv.F = 0;
    expval = 0;
  }
  // the exponent format is "%+03d" and largest value is "307", so set aside 4-5 characters
  unsigned int minwidth = ((expval > -100) && (expval < 100)) ? 4U : 5U;

  // in "%g" mode, "prec" is the number of *significant figures* not decimals
  if (st->flags & FLAGS_ADAPT_EXP) {
    st->flags |= FLAGS_PRECISION;   // make sure _ftoa respects precision (1 digit of default precision is taken for exp format)
    // do we want to fall-back to "%f" mode?
//    if (((value >= 1e-4) && (value < 1e6)) || value == 0) {
    // check if value is between 1e-4 and 1e6 or it can be printed with precision digits (and within ftoa limit)
    if ((expval >= -4) && ((expval < 6) || ((expval < (int)prec) && (expval <= 9)))) {
      if ((int)prec > expval) {
        prec = (unsigned)((int)prec - expval - 1);
      }
      else {
        prec = 0;
      }
      // no characters in exponent
      minwidth = 0U;
      expval   = 0;
    }
    else {
      // we use one sigfig for the whole part
//      if ((prec > 0) && (st->flags & FLAGS_PRECISION)) {
      if (prec > 0) {
        --prec;
      }
    }
  }

  // will everything fit?
  unsigned int width =  st->width;  // remember original width here, state is modified
  unsigned int fwidth = width;

  if (width > minwidth) {
   // we didn't fall-back so subtract the characters required for the exponent
    fwidth -= minwidth;
  } else {
    // not enough characters, so go back to default sizing
    fwidth = 0U;
  }
  if ((st->flags & FLAGS_LEFT) && minwidth) {
    // if we're padding on the right, DON'T pad the floating part
    fwidth = 0U;
  }

  // rescale the float value
  if (expval) {
    value /= conv.F;
  }

  // output the floating part
  const size_t start_idx = idx;
  st->prec = prec;
  st->width = fwidth;
  //st->flags &= ~FLAGS_ADAPT_EXP;  // TODO
  // ftoa respects FLAGS_NEGATIVE
  idx = _ftoa(st, idx, value);

  // output the exponent part
  if (minwidth) {
    // keep value before overwriting it
    const bool rpad = st->flags & FLAGS_LEFT;
    // output the exponential symbol
    _out(st, (st->flags & FLAGS_UPPERCASE) ? 'E' : 'e', idx++);
    // output the exponent value
    st->prec = 0;
    st->width = minwidth - 1;
    st->flags = FLAGS_ZEROPAD | FLAGS_PLUS;
    if(expval < 0) {
      st->flags |= FLAGS_NEGATIVE;
      expval = -expval;
    }
    idx = _ntoa_long(st, idx, (unsigned)expval);
    // might need to right-pad spaces
    if (rpad) {
      idx = _out_pad(st, ' ', idx, width - (idx - start_idx));
    }
  }
  return idx;
}
#endif  // PRINTF_SUPPORT_EXPONENTIAL
#endif  // PRINTF_SUPPORT_FLOAT


// internal vsnprintf
static int _vsnprintf(struct printf_state* st, idx_t idx, const char* format, va_list va)
{
  idx_t start_idx = idx;

  while (*format) {
    // format specifier?  %[flags][width][.precision][length]
    if (*format != '%') {
      // no
      _out(st, *format, idx++);
      format++;
      continue;
    }
    else {
      // yes, evaluate it
      format++;
    }

    // evaluate flags
    unsigned flags = 0U;
    for (unsigned cont = 1U; cont; ) {
      switch (*format) {
        case '0': flags |= FLAGS_ZEROPAD; format++; break;
        case '-': flags |= FLAGS_LEFT;    format++; break;
        case '+': flags |= FLAGS_PLUS;    format++; break;
        case ' ': flags |= FLAGS_SPACE;   format++; break;
        case '#': flags |= FLAGS_HASH;    format++; break;
        default : cont = 0U; break;
      }
    }
    // evaluate width field
    unsigned width = 0U;
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
    unsigned precision = 0U;
    if (*format == '.') {
      flags |= FLAGS_PRECISION;
      format++;
      if (_is_digit(*format)) {
        precision = _atoi(&format);
      }
      else if (*format == '*') {
        const int prec = (int)va_arg(va, int);
        if (prec >= 0) {
          precision = (unsigned)prec;
        } else {
          // negative precision is like no precision specifier at all
          flags &= ~FLAGS_PRECISION;
        }
        format++;
      }
    }
#if 0
    if (flags & FLAGS_LEFT) {
      flags &= ~FLAGS_ZEROPAD;
    }
#endif
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

        // ignore '0' flag when precision is given or padding to left
        if (flags & (FLAGS_PRECISION | FLAGS_LEFT)) {
          flags &= ~FLAGS_ZEROPAD;
        }

        // store values into state
        st->width = width;
        st->base = base;
        st->prec = precision;
        // flags are stored later

        // convert the integer
        if ((*format == 'i') || (*format == 'd')) {
          // signed
          if (flags & FLAGS_LONG_LONG) {
#if defined(PRINTF_SUPPORT_LONG_LONG)
            long long value = va_arg(va, long long);
            if(value < 0) {
              flags |= FLAGS_NEGATIVE;
            }
            st->flags = flags;
            idx = _ntoa_long_long(st, idx, value < 0 ? 0L-(unsigned long long)value : (unsigned long long)value);
#endif
          }
          else if (flags & FLAGS_LONG) {
            long value = va_arg(va, long);
            if(value < 0) {
              flags |= FLAGS_NEGATIVE;
            }
            st->flags = flags;
            // not that -LONG_MIN is undefined
            idx = _ntoa_long(st, idx, value < 0 ? 0U-(unsigned long)value : (unsigned long)value);
          }
          else {
            int value = (flags & FLAGS_CHAR) ? (char)va_arg(va, int) : (flags & FLAGS_SHORT) ? (short int)va_arg(va, int) : va_arg(va, int);
            if (value < 0) {
              flags |= FLAGS_NEGATIVE;
              value = -value;
            }
            st->flags = flags;
            idx = _ntoa_long(st, idx,  value < 0 ? 0U-(unsigned int)value : (unsigned int)value);
          }
        }
        else {
          // unsigned
          st->flags = flags;
          if (flags & FLAGS_LONG_LONG) {
#if defined(PRINTF_SUPPORT_LONG_LONG)
            idx = _ntoa_long_long(st, idx, va_arg(va, unsigned long long));
#endif
          }
          else if (flags & FLAGS_LONG) {
            idx = _ntoa_long(st, idx, va_arg(va, unsigned long));
          }
          else {
            const unsigned int value = (flags & FLAGS_CHAR) ? (unsigned char)va_arg(va, unsigned int) : (flags & FLAGS_SHORT) ? (unsigned short int)va_arg(va, unsigned int) : va_arg(va, unsigned int);
            idx = _ntoa_long(st, idx, value);
          }
        }
        format++;
        break;
      }
#if defined(PRINTF_SUPPORT_FLOAT)
      case 'f' :
      case 'F' :
        if (flags & FLAGS_LEFT) {
          flags &= ~FLAGS_ZEROPAD;
        }
        if (*format == 'F') flags |= FLAGS_UPPERCASE;
        st->width = width;
        st->base = 10U;
        st->prec = precision;
        st->flags = flags;
        idx = _ftoa(st, idx, va_arg(va, double));
        format++;
        break;
#if defined(PRINTF_SUPPORT_EXPONENTIAL)
      case 'e':
      case 'E':
      case 'g':
      case 'G':
        if ((*format == 'g')||(*format == 'G')) flags |= FLAGS_ADAPT_EXP;
        if ((*format == 'E')||(*format == 'G')) flags |= FLAGS_UPPERCASE;
        if (flags & FLAGS_LEFT) {
          flags &= ~FLAGS_ZEROPAD;
        }
        st->width = width;
        st->base = 10U;
        st->prec = precision;
        st->flags = flags;

        idx = _etoa(st, idx, va_arg(va, double));
        format++;
        break;
#endif  // PRINTF_SUPPORT_EXPONENTIAL
#endif  // PRINTF_SUPPORT_FLOAT
      case 'c' : {
        // pre padding
        if (!(flags & FLAGS_LEFT)) {
          idx = _out_pad(st, ' ', idx, (int)width - 1);
        }
        // char output
        _out(st, (char)va_arg(va, int), idx++);
        // post padding
        if (flags & FLAGS_LEFT) {
          idx = _out_pad(st, ' ', idx, (int)width - 1);
        }
        format++;
        break;
      }

      case 's' : {
        const char* p = va_arg(va, char*);
        unsigned int toprint = _strnlen_s(p, flags & FLAGS_PRECISION ? precision : (size_t)-1);
        if (!(flags & FLAGS_LEFT)) {
          idx = _out_pad(st, ' ', idx, (int)(width - toprint));
        }
        // string output
        for (unsigned int i = 0; i < toprint; i++) {
          _out(st, *(p++), idx++);
        }
        // post padding
        if (flags & FLAGS_LEFT) {
          idx = _out_pad(st, ' ', idx, (int)(width - toprint));
        }
        format++;
        break;
      }

      case 'p' : {
        flags |= FLAGS_ZEROPAD | FLAGS_UPPERCASE;

        st->width = sizeof(void*) * 2U;
        st->base = 16U;
        st->prec = precision;
        st->flags = flags;
#if defined(PRINTF_SUPPORT_LONG_LONG)
        const bool is_ll = sizeof(uintptr_t) == sizeof(long long);
        if (is_ll) {
          idx = _ntoa_long_long(st, idx, (uintptr_t)va_arg(va, void*));
        }
        else {
#endif
          idx = _ntoa_long(st, idx, (unsigned long)((uintptr_t)va_arg(va, void*)));
#if defined(PRINTF_SUPPORT_LONG_LONG)
        }
#endif
        format++;
        break;
      }

      case '%' :
        _out(st, '%', idx++);
        format++;
        break;

      default :
        // TODO - remember %, ouptut full printf specifier
        _out(st, *format, idx++);
        format++;
        break;
    }
  }

  // termination in calling function
  //_out(out, (char)0, idx < maxlen ? idx : maxlen - 1U, maxlen);

  // return written chars
  return (int)(idx - start_idx);
}


///////////////////////////////////////////////////////////////////////////////

int vprintf_(const char* format, va_list va)
{
  struct printf_state st = { .out = &out_putchar, .maxidx = UINTPTR_MAX};
  return _vsnprintf(&st, 0, format, va);
}

int printf_(const char* format, ...)
{
  va_list va;
  va_start(va, format);
  const int ret = vprintf_(format, va);
  va_end(va);
  return ret;
}

int vsnprintf_(char* buffer, size_t count, const char* format, va_list va)
{
  struct printf_state st = { .out = &out_buffer};
  if (buffer && count) {  // avoid case when there is no space for '\0' in buffer
    idx_t idx = (uintptr_t)buffer;
    idx_t eidx = idx + count;
    if (eidx < idx) {  // overflow
      eidx = UINTPTR_MAX;
    }
    st.maxidx = eidx;
    int ret = _vsnprintf(&st, idx, format, va);
    // terminate string
    _out(&st, '\0', MIN(idx + (unsigned)ret, eidx - 1));
    return ret;
  } else {
    st.maxidx = 0;
    return _vsnprintf(&st, 0, format, va);
  }
}

int sprintf_(char* buffer, const char* format, ...)
{
  va_list va;
  int ret;
  va_start(va, format);
  return vsnprintf_(buffer, SIZE_MAX, format, va);
  va_end(va);
  return ret;
}


int snprintf_(char* buffer, size_t count, const char* format, ...)
{
  va_list va;
  va_start(va, format);
  const int ret = vsnprintf_(buffer, count, format, va);
  va_end(va);
  return ret;
}

int fctprintf(void (*outfn)(char character, void* arg), void* arg, const char* format, ...)
{
  va_list va;
  va_start(va, format);
  const struct out_userfct usr = {.out_base = {_out_userfct_fn}, .fct = outfn, .arg = arg};
  struct printf_state st = {.out = &usr.out_base, .maxidx = UINTPTR_MAX};
  const int ret = _vsnprintf(&st, 0, format, va);
  va_end(va);
  return ret;
}
