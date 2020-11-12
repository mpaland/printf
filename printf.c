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


#ifndef SECOND_PASS

	#include "includes.h"

//********************************************************************************************************
// Configurable defines
//********************************************************************************************************

//	minimal is ~2.6k, all options is ~6.8k
	#define PRINTF_SUPPORT_LONG_LONG
	#define PRINTF_SUPPORT_FLOAT
	#define PRINTF_SUPPORT_EXPONENTIAL

// 	'ntoa' conversion buffer size, this must be big enough to hold one converted
// 	numeric number including padded zeros (dynamically created on stack)
	#define PRINTF_NTOA_BUFFER_SIZE    32U

// 	'ftoa' conversion buffer size, this must be big enough to hold one converted
// 	float number including padded zeros (dynamically created on stack)
// 	default: 32 byte
	#define PRINTF_FTOA_BUFFER_SIZE    32U


// 	define the default floating point precision
// 	default: 6 digits
	#define PRINTF_DEFAULT_FLOAT_PRECISION  6U


// 	define the largest float suitable to print with %f
// 	default: 1e9
	#define PRINTF_MAX_FLOAT  1e9

//********************************************************************************************************
// Local defines
//********************************************************************************************************

// 	internal flag definitions
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

// 	output function type
	typedef void (*out_fct_type)(char character, void* buffer, size_t idx, size_t maxlen);

// 	wrapper (used as buffer) for output function type
	typedef struct
	{
  		void  (*fct)(char character, void* arg);
  		void* arg;
	} out_fct_wrap_type;

// 	internal secure strlen
// 	\return The length of the string (excluding the terminating 0) limited by 'maxsize'
	static inline size_t _strnlen_s(const char* str, size_t maxsize)
	{
  		const char* s;
  		for (s = str; *s && maxsize--; ++s);
  		return (size_t)(s - str);
	}


// 	internal test if char is a digit (0-9)
// 	\return true if char is a digit
	static inline bool _is_digit(char ch)
	{
  		return (ch >= '0') && (ch <= '9');
	}

//********************************************************************************************************
// Public variables
//********************************************************************************************************

//********************************************************************************************************
// Private variables
//********************************************************************************************************

//********************************************************************************************************
// Private prototypes
//********************************************************************************************************

	static void _out_buffer(char character, void* buffer, size_t idx, size_t maxlen);
	static void _out_fifo(char character, void* fifo, size_t idx, size_t maxlen);
	static void _out_null(char character, void* buffer, size_t idx, size_t maxlen);
	static void _out_char(char character, void* buffer, size_t idx, size_t maxlen);
	static void _out_fct(char character, void* buffer, size_t idx, size_t maxlen);

	static size_t _out_rev(out_fct_type out, char* buffer, size_t idx, size_t maxlen, const char* buf, uint8_t len, uint8_t width, uint16_t flags);
	static size_t _ntoa_format(out_fct_type out, char* buffer, size_t idx, size_t maxlen, char* buf, uint8_t len, bool negative, uint8_t base, uint8_t prec, uint8_t width, uint16_t flags);

#ifdef PRINTF_SUPPORT_LONG_LONG
	static size_t _ntoa(out_fct_type out, char* buffer, size_t idx, size_t maxlen, unsigned long long value, bool negative, uint8_t base, uint8_t prec, uint8_t width, uint16_t flags);
#else
	static size_t _ntoa(out_fct_type out, char* buffer, size_t idx, size_t maxlen, unsigned long value, bool negative, uint8_t base, uint8_t prec, uint8_t width, uint16_t flags);	
#endif

#ifdef PRINTF_SUPPORT_FLOAT
	static size_t _ftoa(out_fct_type out, char* buffer, size_t idx, size_t maxlen, double value, uint8_t prec, uint8_t width, uint16_t flags);
#ifdef PRINTF_SUPPORT_EXPONENTIAL
	static size_t _etoa(out_fct_type out, char* buffer, size_t idx, size_t maxlen, double value, uint8_t prec, uint8_t width, uint16_t flags);
#endif
#endif

	static int _vsnprintf(out_fct_type out, char* buffer, const size_t maxlen, const char* format, va_list va);
	static unsigned int _atoi(const char** str);

#ifdef PLATFORM_AVR
	static int _vsnprintf_P(out_fct_type out, char* buffer, const size_t maxlen, PGM_P format, va_list va);
	static unsigned int _atoi_P(const char** str);
#endif

//********************************************************************************************************
// Public functions
//********************************************************************************************************

int printf_(const char* format, ...)
{
  va_list va;
  va_start(va, format);
  char buffer[1];
  const int ret = _vsnprintf(_out_char, buffer, (size_t)-1, format, va);
  va_end(va);
  return ret;
}

char* hprintf(const char* format, ...)
{
  va_list va;
  va_start(va, format);
  char buffer[1];
  int length;
  char* ret = NULL;

  length = _vsnprintf(_out_null, buffer, (size_t)-1, format, va);
  ret = heap_allocate(length+1);
  if(ret)
  	_vsnprintf(_out_buffer, ret, length+1, format, va);

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

int fifoprintf(struct fifo_struct *dst, const char* format, ...)
{
  va_list va;
  va_start(va, format);
  const int ret = _vsnprintf(_out_fifo, (char*)dst, (size_t)-1, format, va);
  va_end(va);
  return ret;
}

#ifdef PLATFORM_AVR

int printf_P_(PGM_P format, ...)
{
  va_list va;
  va_start(va, format);
  char buffer[1];
  const int ret = _vsnprintf_P(_out_char, buffer, (size_t)-1, format, va);
  va_end(va);
  return ret;
}

char* hprintf_P(PGM_P format, ...)
{
  va_list va;
  va_start(va, format);
  char buffer[1];
  int length;
  char* ret = NULL;

  length = _vsnprintf_P(_out_null, buffer, (size_t)-1, format, va);
  ret = heap_allocate(length+1);
  if(ret)
  	_vsnprintf_P(_out_buffer, ret, length+1, format, va);

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

int fifoprintf_P(struct fifo_struct *dst, const char* format, ...)
{
  va_list va;
  va_start(va, format);
  const int ret = _vsnprintf_P(_out_fifo, (char*)dst, (size_t)-1, format, va);
  va_end(va);
  return ret;
}
#endif


//********************************************************************************************************
// Private functions
//********************************************************************************************************

// internal buffer output
static void _out_buffer(char character, void* buffer, size_t idx, size_t maxlen)
{
  if (idx < maxlen) {
    ((char*)buffer)[idx] = character;
  }
}

// fifo output
static void _out_fifo(char character, void* fifo, size_t idx, size_t maxlen)
{
  (void)idx; (void)maxlen;
  if (character) {
    fifo_write_char(fifo, character);
  }
}

// internal null output
static void _out_null(char character, void* buffer, size_t idx, size_t maxlen)
{
  (void)character; (void)buffer; (void)idx; (void)maxlen;
}

// internal _putchar wrapper
static void _out_char(char character, void* buffer, size_t idx, size_t maxlen)
{
  (void)buffer; (void)idx; (void)maxlen;
  if (character) {
    _putchar(character);
  }
}

// internal output function wrapper
static void _out_fct(char character, void* buffer, size_t idx, size_t maxlen)
{
  (void)idx; (void)maxlen;
  if (character) {
    // buffer is the output fct pointer
    ((out_fct_wrap_type*)buffer)->fct(character, ((out_fct_wrap_type*)buffer)->arg);
  }
}

// output the specified string in reverse, taking care of any zero-padding
static size_t _out_rev(out_fct_type out, char* buffer, size_t idx, size_t maxlen, const char* buf, uint8_t len, uint8_t width, uint16_t flags)
{
  const size_t start_idx = idx;

  // pad spaces up to given width
  if (!(flags & FLAGS_LEFT) && !(flags & FLAGS_ZEROPAD)) {
    for (uint8_t i = len; i < width; i++) {
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
static size_t _ntoa_format(out_fct_type out, char* buffer, size_t idx, size_t maxlen, char* buf, uint8_t len, bool negative, uint8_t base, uint8_t prec, uint8_t width, uint16_t flags)
{
  // pad leading zeros
  if (!(flags & FLAGS_LEFT)) {
    if (width && (flags & FLAGS_ZEROPAD) && (negative || (flags & (FLAGS_PLUS | FLAGS_SPACE)))) {
      width--;
    }
    while ((flags & FLAGS_ZEROPAD) && (len < width) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
      buf[len++] = '0';
    }
  }
  while ((len < prec) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
    buf[len++] = '0';
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
#ifdef PRINTF_SUPPORT_LONG_LONG
static size_t _ntoa(out_fct_type out, char* buffer, size_t idx, size_t maxlen, unsigned long long value, bool negative, uint8_t base, uint8_t prec, uint8_t width, uint16_t flags)
#else
static size_t _ntoa(out_fct_type out, char* buffer, size_t idx, size_t maxlen, unsigned long value, bool negative, uint8_t base, uint8_t prec, uint8_t width, uint16_t flags)
#endif
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


#ifdef PRINTF_SUPPORT_FLOAT
// internal ftoa for fixed decimal floating point
static size_t _ftoa(out_fct_type out, char* buffer, size_t idx, size_t maxlen, double value, uint8_t prec, uint8_t width, uint16_t flags)
{
  char buf[PRINTF_FTOA_BUFFER_SIZE];
  uint8_t len  = 0U;
  double diff = 0.0;

  // powers of 10
  static const double pow10[] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000 };

  // test for special values
  if (value != value)
    return _out_rev(out, buffer, idx, maxlen, "nan", 3, width, flags);
  if (value < -DBL_MAX)
    return _out_rev(out, buffer, idx, maxlen, "fni-", 4, width, flags);
  if (value > DBL_MAX)
    return _out_rev(out, buffer, idx, maxlen, (flags & FLAGS_PLUS) ? "fni+" : "fni", (flags & FLAGS_PLUS) ? 4U : 3U, width, flags);

  // test for very large values
  // standard printf behavior is to print EVERY whole number digit -- which could be 100s of characters overflowing your buffers == bad
  if ((value > PRINTF_MAX_FLOAT) || (value < -PRINTF_MAX_FLOAT)) {
    return _etoa(out, buffer, idx, maxlen, value, prec, width, flags);
  }

  // test for negative
  bool negative = false;
  if (value < 0) {
    negative = true;
    value = 0 - value;
  }

  // set default precision, if not set explicitly
  if (!(flags & FLAGS_PRECISION)) {
    prec = PRINTF_DEFAULT_FLOAT_PRECISION;
  }
  // limit precision to 9, cause a prec >= 10 can lead to overflow errors
  while ((len < PRINTF_FTOA_BUFFER_SIZE) && (prec > 9U)) {
    buf[len++] = '0';
    prec--;
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
  else if (diff < 0.5) {
  }
  else if ((frac == 0U) || (frac & 1U)) {
    // if halfway, round up if odd OR if last digit is 0
    ++frac;
  }

  if (prec == 0U) {
    diff = value - (double)whole;
    if ((!(diff < 0.5) || (diff > 0.5)) && (whole & 1)) {
      // exactly 0.5 and ODD, then round up
      // 1.5 -> 2, but 2.5 -> 2
      ++whole;
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
    buf[len++] = (char)(48 + (whole % 10));
    if (!(whole /= 10)) {
      break;
    }
  }

  // pad leading zeros
  if (!(flags & FLAGS_LEFT) && (flags & FLAGS_ZEROPAD)) {
    if (width && (negative || (flags & (FLAGS_PLUS | FLAGS_SPACE)))) {
      width--;
    }
    while ((len < width) && (len < PRINTF_FTOA_BUFFER_SIZE)) {
      buf[len++] = '0';
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

  return _out_rev(out, buffer, idx, maxlen, buf, len, width, flags);
}

#ifdef PRINTF_SUPPORT_EXPONENTIAL
// internal ftoa variant for exponential floating-point type, contributed by Martijn Jasperse <m.jasperse@gmail.com>
static size_t _etoa(out_fct_type out, char* buffer, size_t idx, size_t maxlen, double value, uint8_t prec, uint8_t width, uint16_t flags)
{
  // check for NaN and special values
  if ((value != value) || (value > DBL_MAX) || (value < -DBL_MAX)) {
    return _ftoa(out, buffer, idx, maxlen, value, prec, width, flags);
  }

  // determine the sign
  const bool negative = value < 0;
  if (negative) {
    value = -value;
  }

  // default precision
  if (!(flags & FLAGS_PRECISION)) {
    prec = PRINTF_DEFAULT_FLOAT_PRECISION;
  }

  // determine the decimal exponent
  // based on the algorithm by David Gay (https://www.ampl.com/netlib/fp/dtoa.c)
  union {
    uint64_t U;
    double   F;
  } conv;

  conv.F = value;
  int exp2 = (int)((conv.U >> 52U) & 0x07FFU) - 1023;           // effectively log2
  conv.U = (conv.U & ((1ULL << 52U) - 1U)) | (1023ULL << 52U);  // drop the exponent so conv.F is now in [1,2)
  // now approximate log10 from the log2 integer part and an expansion of ln around 1.5
  int expval = (int)(0.1760912590558 + exp2 * 0.301029995663981 + (conv.F - 1.5) * 0.289529654602168);
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

  // the exponent format is "%+03d" and largest value is "307", so set aside 4-5 characters
  uint8_t minwidth = ((expval < 100) && (expval > -100)) ? 4U : 5U;

  // in "%g" mode, "prec" is the number of *significant figures* not decimals
  if (flags & FLAGS_ADAPT_EXP) {
    // do we want to fall-back to "%f" mode?
    if ((value >= 1e-4) && (value < 1e6)) {
      if ((int)prec > expval) {
        prec = (unsigned)((int)prec - expval - 1);
      }
      else {
        prec = 0;
      }
      flags |= FLAGS_PRECISION;   // make sure _ftoa respects precision
      // no characters in exponent
      minwidth = 0U;
      expval   = 0;
    }
    else {
      // we use one sigfig for the whole part
      if ((prec > 0) && (flags & FLAGS_PRECISION)) {
        --prec;
      }
    }
  }

  // will everything fit?
  uint8_t fwidth = width;
  if (width > minwidth) {
    // we didn't fall-back so subtract the characters required for the exponent
    fwidth -= minwidth;
  } else {
    // not enough characters, so go back to default sizing
    fwidth = 0U;
  }
  if ((flags & FLAGS_LEFT) && minwidth) {
    // if we're padding on the right, DON'T pad the floating part
    fwidth = 0U;
  }

  // rescale the float value
  if (expval) {
    value /= conv.F;
  }

  // output the floating part
  const size_t start_idx = idx;
  idx = _ftoa(out, buffer, idx, maxlen, negative ? -value : value, prec, fwidth, flags & ~FLAGS_ADAPT_EXP);

  // output the exponent part
  if (minwidth) {
    // output the exponential symbol
    out((flags & FLAGS_UPPERCASE) ? 'E' : 'e', buffer, idx++, maxlen);
    // output the exponent value
    idx = _ntoa(out, buffer, idx, maxlen, (expval < 0) ? -expval : expval, expval < 0, 10, 0, minwidth-1, FLAGS_ZEROPAD | FLAGS_PLUS);
    // might need to right-pad spaces
    if (flags & FLAGS_LEFT) {
      while (idx - start_idx < width) out(' ', buffer, idx++, maxlen);
    }
  }
  return idx;
}
#endif //	PRINTF_SUPPORT_FLOAT
#endif //	PRINTF_SUPPORT_EXPONENTIAL
#endif // 	ndef SECOND_PASS

//********************************************************************************************************
// PSTR compatibility, this code may be compiled twice for PGM and non-PGM versions
//********************************************************************************************************

// wrappers for PGM or non-PGM access
#ifndef SECOND_PASS
  #define FMTRD(_fmt) (*(_fmt))
  #define ATOI(_src) _atoi(_src)
#else
  #define FMTRD(_fmt) pgm_read_byte(_fmt)
  #define ATOI(_src) _atoi_P(_src)
#endif

// internal ASCII string to unsigned int conversion
#ifndef SECOND_PASS
static size_t _atoi(const char** str)
#else
static size_t _atoi_P(const char** str)
#endif
{
  size_t i = 0U;
    while (_is_digit(FMTRD(*str))) {
	    i = i * 10U + (FMTRD((*str)++) - '0');
    }
  return i;
}

// internal vsnprintf
#ifndef SECOND_PASS
static int _vsnprintf(out_fct_type out, char* buffer, const size_t maxlen, const char* format, va_list va)
#else
static int _vsnprintf_P(out_fct_type out, char* buffer, const size_t maxlen, PGM_P format, va_list va)
#endif
{
  uint16_t flags;
  size_t width;
  uint8_t precision, n;
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
        const int8_t prec = (int8_t)va_arg(va, int);
        precision = prec > 0 ? prec : 0U;
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

      case 't' :
        flags |= (sizeof(ptrdiff_t) == sizeof(long) ? FLAGS_LONG : FLAGS_LONG_LONG);
        format++;
        break;

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
        uint8_t base;
        if (FMTRD(format) == 'x' || FMTRD(format) == 'X') {
          base = 16;
        }
        else if (FMTRD(format) == 'o') {
          base =  8;
        }
        else if (FMTRD(format) == 'b') {
          base =  2;
        }
        else {
          base = 10;
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
#ifdef PRINTF_SUPPORT_LONG_LONG
            const long long value = va_arg(va, long long);
            idx = _ntoa(out, buffer, idx, maxlen, (unsigned long long)(value > 0 ? value : 0 - value), value < 0, base, precision, width, flags);
#endif
          }
          else if (flags & FLAGS_LONG) {
            const long value = va_arg(va, long);
            idx = _ntoa(out, buffer, idx, maxlen, (unsigned long)(value > 0 ? value : 0 - value), value < 0, base, precision, width, flags);
          }
          else {
            const int value = (flags & FLAGS_CHAR) ? (char)va_arg(va, int) : (flags & FLAGS_SHORT) ? (short int)va_arg(va, int) : va_arg(va, int);
            idx = _ntoa(out, buffer, idx, maxlen, (unsigned int)(value > 0 ? value : 0 - value), value < 0, base, precision, width, flags);
          }
        }
        else {
          // unsigned
          if (flags & FLAGS_LONG_LONG) {
#ifdef PRINTF_SUPPORT_LONG_LONG
            idx = _ntoa(out, buffer, idx, maxlen, va_arg(va, unsigned long long), false, base, precision, width, flags);
#endif
          }
          else if (flags & FLAGS_LONG) {
            idx = _ntoa(out, buffer, idx, maxlen, va_arg(va, unsigned long), false, base, precision, width, flags);
          }
          else {
            const unsigned int value = (flags & FLAGS_CHAR) ? (unsigned char)va_arg(va, unsigned int) : (flags & FLAGS_SHORT) ? (unsigned short int)va_arg(va, unsigned int) : va_arg(va, unsigned int);
            idx = _ntoa(out, buffer, idx, maxlen, value, false, base, precision, width, flags);
          }
        }
        format++;
        break;
      }
#ifdef PRINTF_SUPPORT_FLOAT
      case 'f' :
      case 'F' :
        if (FMTRD(format) == 'F') flags |= FLAGS_UPPERCASE;
        idx = _ftoa(out, buffer, idx, maxlen, va_arg(va, double), precision, width, flags);
        format++;
        break;
#ifdef PRINTF_SUPPORT_EXPONENTIAL
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
        size_t l = 1U;
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
        size_t l = _strnlen_s(p, precision ? precision : (size_t)-1);
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
        const bool is_ll = sizeof(uintptr_t) == sizeof(long long);
        if (is_ll) {
          idx = _ntoa(out, buffer, idx, maxlen, (uintptr_t)va_arg(va, void*), false, 16U, precision, width, flags);
        }
        else {
          idx = _ntoa(out, buffer, idx, maxlen, (unsigned long)((uintptr_t)va_arg(va, void*)), false, 16U, precision, width, flags);
        }
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
  #undef FMTRD
  #undef ATOI


// Compile _P version for PROGMEM access
#ifdef PLATFORM_AVR
	#ifndef SECOND_PASS
		#define SECOND_PASS
		#include "printf.c"
	#endif
#endif
