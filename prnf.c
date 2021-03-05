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

//	minimal is ~2.3k, all options is ~6.4k
	#define PRNF_SUPPORT_LONG_LONG
	#define PRNF_SUPPORT_FLOAT
	#define PRNF_SUPPORT_EXPONENTIAL

// 	'ntoa' conversion buffer size, this must be big enough to hold one converted
// 	numeric number including padded zeros (dynamically created on stack)
	#define PRNF_NTOA_BUFFER_SIZE    32

// 	'ftoa' conversion buffer size, this must be big enough to hold one converted
// 	float number including padded zeros (dynamically created on stack)
// 	default: 32 byte
	#define PRNF_FTOA_BUFFER_SIZE    32

//	Called if above buffer sizes are *reached* (although not necessarily exceeded)
  	#define WARN_NTOA_BUFFER_SIZE()		((void)0)
  	#define WARN_FTOA_BUFFER_SIZE() 	((void)0)

// 	define the default floating point precision
// 	default: 6 digits
	#define PRNF_DEFAULT_FLOAT_PRECISION  6

// 	define the largest float suitable to print with %f
// 	default: 1e9
	#define PRNF_MAX_FLOAT  1e9

//********************************************************************************************************
// Local defines
//********************************************************************************************************

	struct out_buf_vars_struct
	{
		char* buf;
		size_t dst_size;
		size_t bufpos;
	};

// 	internal flag definitions
	#define FLAGS_ZEROPAD   (1 <<  0)
	#define FLAGS_LEFT      (1 <<  1)
	#define FLAGS_PLUS      (1 <<  2)
	#define FLAGS_SPACE     (1 <<  3)
	#define FLAGS_HASH      (1 <<  4)
	#define FLAGS_UPPERCASE (1 <<  5)
	#define FLAGS_CHAR      (1 <<  6)
	#define FLAGS_SHORT     (1 <<  7)
	#define FLAGS_LONG      (1 <<  8)
	#define FLAGS_LONG_LONG (1 <<  9)
	#define FLAGS_PRECISION (1 << 10)
	#define FLAGS_ADAPT_EXP (1 << 11)

//********************************************************************************************************
// Public variables
//********************************************************************************************************

//********************************************************************************************************
// Private variables
//********************************************************************************************************

//********************************************************************************************************
// Private prototypes
//********************************************************************************************************

	static void out_buf(char x, void* out_vars);
	static void out_null(char x, void* out_vars);
	static void out_putc(char x, void* out_vars);
#ifdef PRNF_SUPPORT_FIFO
	static void out_fifo(char x, void* out_vars);
#endif

	static size_t out_rev(void(*out_fptr)(char, void*), void* out_vars, size_t idx, const char* buf, uint8_t len, uint8_t width, uint16_t flags);
	static size_t numtoasc_format(void(*out_fptr)(char, void*), void* out_vars, size_t idx, char* buf, uint8_t len, bool negative, uint8_t base, uint8_t prec, uint8_t width, uint16_t flags);

#ifdef PRNF_SUPPORT_LONG_LONG
	static size_t numtoasc(void(*out_fptr)(char, void*), void* out_vars, size_t idx, unsigned long long value, bool negative, uint8_t base, uint8_t prec, uint8_t width, uint16_t flags);
#else
	static size_t numtoasc(void(*out_fptr)(char, void*), void* out_vars, size_t idx, unsigned long value, bool negative, uint8_t base, uint8_t prec, uint8_t width, uint16_t flags);	
#endif

#ifdef PRNF_SUPPORT_FLOAT
	static size_t fltasc(void(*out_fptr)(char, void*), void* out_vars, size_t idx, double value, uint8_t prec, uint8_t width, uint16_t flags);
#ifdef PRNF_SUPPORT_EXPONENTIAL
	static size_t exptoasc(void(*out_fptr)(char, void*), void* out_vars, size_t idx, double value, uint8_t prec, uint8_t width, uint16_t flags);
#endif
#endif

	static int core_prnf(void(*out_fptr)(char, void*), void* out_vars, const char* format, va_list va);
	static uint8_t asctoint(const char** str);

	static size_t prnf_strnlen_s(const char* str, size_t maxlen);
	static bool prnf_is_digit(char ch);

#ifdef PLATFORM_AVR
	static int core_prnf_P(void(*out_fptr)(char, void*), void* out_vars, PGM_P format, va_list va);
	static uint8_t asctoint_P(const char** str);
#endif

//********************************************************************************************************
// Public functions
//********************************************************************************************************

int prnf(const char* format, ...)
{
	va_list va;
	va_start(va, format);
 
	const int ret = vprnf(format, va);

	va_end(va);
	return ret;
}

char* hprnf(const char* format, ...)
{
	va_list va;
	va_start(va, format);

	struct out_buf_vars_struct out_buf_vars;
	char* result;

  	out_buf_vars.dst_size = core_prnf(out_null, NULL, format, va) + 1;
   	out_buf_vars.buf = heap_allocate(out_buf_vars.dst_size);
	out_buf_vars.bufpos = 0;
	result = out_buf_vars.buf;

  	if(out_buf_vars.buf)
  		core_prnf(out_buf, &out_buf_vars, format, va);

  	va_end(va);
  	return result;
}

int sprnf(char* dst, const char* format, ...)
{
	va_list va;
	va_start(va, format);

	const int ret = vsprnf(dst, format, va);

	va_end(va);
	return ret;
}

int snprnf(char* dst, size_t dst_size, const char* format, ...)
{
	va_list va;
	va_start(va, format);

	const int ret = vsnprnf(dst, dst_size, format, va);

	va_end(va);
	return ret;
}

int snappf(char* dst, size_t dst_size, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);

	int chars_written = 0;
	size_t org_len;

	if(dst_size)
		org_len = prnf_strnlen_s(dst, dst_size-1);
	else
		org_len = 0;

	chars_written = vsnprnf(&dst[org_len], dst_size-org_len, fmt, va);

	va_end(va);
	return chars_written;
}

int vprnf(const char* format, va_list va)
{
	return core_prnf(out_putc, NULL, format, va);
}

int vsprnf(char* dst, const char* format, va_list va)
{
	struct out_buf_vars_struct out_buf_vars;

	out_buf_vars.buf = dst;
	out_buf_vars.dst_size = SIZE_MAX;
	out_buf_vars.bufpos = 0;

	return core_prnf(out_buf, &out_buf_vars, format, va);
}

int vsnprnf(char* dst, size_t dst_size, const char* format, va_list va)
{
	struct out_buf_vars_struct out_buf_vars;

	out_buf_vars.buf = dst;
	out_buf_vars.dst_size = dst_size;
	out_buf_vars.bufpos = 0;

	return core_prnf(out_buf, &out_buf_vars, format, va);
}

int fptrprnf(void(*out_fptr)(char, void*), void* out_vars, const char* format, ...)
{
  	va_list va;
  	va_start(va, format);

	const int ret = core_prnf(out_fptr, out_vars, format, va);

	va_end(va);
	return ret;
}

#ifdef PRNF_SUPPORT_FIFO
int fifoprnf(struct fifo_struct *dst, const char* format, ...)
{
	va_list va;
	va_start(va, format);
	const int ret = core_prnf(out_fifo, dst, format, va);
	va_end(va);
	return ret;
}
#endif

#ifdef PLATFORM_AVR

int prnf_P(PGM_P format, ...)
{
	va_list va;
	va_start(va, format);
	
	const int ret = vprnf_P(format, va);

	va_end(va);
	return ret;
}

char* hprnf_P(PGM_P format, ...)
{
	va_list va;
	va_start(va, format);

	struct out_buf_vars_struct out_buf_vars;
	char* result;

  	out_buf_vars.dst_size = core_prnf_P(out_null, NULL, format, va) + 1;
   	out_buf_vars.buf = heap_allocate(out_buf_vars.dst_size);
	out_buf_vars.bufpos = 0;
	result = out_buf_vars.buf;

  	if(out_buf_vars.buf)
  		core_prnf_P(out_buf, &out_buf_vars, format, va);

  	va_end(va);
  	return result;
}

int sprnf_P(char* dst, PGM_P format, ...)
{
	va_list va;
	va_start(va, format);

	const int ret = vsprnf_P(dst, format, va);

	va_end(va);
	return ret;
}

int snprnf_P(char* dst, size_t dst_size, PGM_P format, ...)
{
	va_list va;
	va_start(va, format);

	const int ret = vsnprnf_P(dst, dst_size, format, va);

	va_end(va);
	return ret;
}

int snappf_P(char* dst, size_t dst_size, PGM_P fmt, ...)
{
	int chars_written;
	size_t org_len;
	va_list va;

	if(dst_size)
		org_len = prnf_strnlen_s(dst, dst_size-1);
	else
		org_len = 0;

	va_start(va, fmt);
	chars_written = vsnprnf_P(&dst[org_len], dst_size-org_len, fmt, va);
	va_end(va);

	return chars_written;
}

int vprnf_P(PGM_P format, va_list va)
{
	return core_prnf_P(out_putc, NULL, format, va);
}

int vsprnf_P(char* dst, PGM_P format, va_list va)
{
	struct out_buf_vars_struct out_buf_vars;

	out_buf_vars.buf = dst;
	out_buf_vars.dst_size = SIZE_MAX;
	out_buf_vars.bufpos = 0;

	return core_prnf_P(out_buf, &out_buf_vars, format, va);
}

int vsnprnf_P(char* dst, size_t dst_size, PGM_P format, va_list va)
{
	struct out_buf_vars_struct out_buf_vars;

	out_buf_vars.buf = dst;
	out_buf_vars.dst_size = dst_size;
	out_buf_vars.bufpos = 0;

	return core_prnf_P(out_buf, &out_buf_vars, format, va);
}

int fptrprnf_P(void(*out_fptr)(char, void*), void* out_vars, PGM_P format, ...)
{
  	va_list va;
  	va_start(va, format);

	const int ret = core_prnf_P(out_fptr, out_vars, format, va);

	va_end(va);
	return ret;
}

#ifdef PRNF_SUPPORT_FIFO
int fifoprnf_P(struct fifo_struct *dst, PGM_P format, ...)
{
	va_list va;
	va_start(va, format);
	const int ret = core_prnf_P(out_fifo, dst, format, va);
	va_end(va);
	return ret;
}
#endif

#endif

//********************************************************************************************************
// Private functions
//********************************************************************************************************

// internal buffer output
static void out_buf(char x, void* out_vars)
{
	struct out_buf_vars_struct* buf_vars_ptr;

	buf_vars_ptr = out_vars;

	if(buf_vars_ptr->dst_size)
	{
		if(buf_vars_ptr->bufpos < buf_vars_ptr->dst_size-1)
		{
			*(buf_vars_ptr->buf++) = x;
			buf_vars_ptr->bufpos++;
		}
		else
			*(buf_vars_ptr->buf) = 0;
	};
}

// internal null output
static void out_null(char x, void* out_vars)
{
	(void)x; (void)out_vars;
}

// internal prnf_putc wrapper
static void out_putc(char x, void* out_vars)
{
	(void)out_vars;
	if(x)
    	prnf_putc(x);
}

#ifdef PRNF_SUPPORT_FIFO
static void out_fifo(char x, void* out_vars)
{
	if(x)
		fifo_write_char((struct fifo_struct*)out_vars, x);
}
#endif

// output the specified string in reverse, taking care of any zero-padding
static size_t out_rev(void(*out_fptr)(char, void*), void* out_vars, size_t idx, const char* buf, uint8_t len, uint8_t width, uint16_t flags)
{
	const size_t start_idx = idx;

	// pad spaces up to given width
	if (!(flags & FLAGS_LEFT) && !(flags & FLAGS_ZEROPAD))
	{
		for(uint8_t i = len; i < width; i++)
		{
			out_fptr(' ', out_vars);
			idx++;
		};
    };

	// reverse string
	while(len)
	{
    	out_fptr(buf[--len], out_vars);
    	idx++;
  	};

	// append pad spaces up to given width
  	if(flags & FLAGS_LEFT)
	{
		while(idx - start_idx < width)
		{
      		out_fptr(' ', out_vars);
      		idx++;
      	};
    };

	return idx;
}

// 	internal secure strlen
// 	\return The length of the string (excluding the terminating 0) limited by 'maxlen'
static size_t prnf_strnlen_s(const char* str, size_t maxlen)
{
	const char* s;
	for (s = str; *s && maxlen--; ++s);
	return (size_t)(s - str);
}

// 	internal test if char is a digit (0-9)
// 	\return true if char is a digit
static bool prnf_is_digit(char ch)
{
	return (ch >= '0') && (ch <= '9');
}

// internal itoa format
static size_t numtoasc_format(void(*out_fptr)(char, void*), void* out_vars, size_t idx, char* buf, uint8_t len, bool negative, uint8_t base, uint8_t prec, uint8_t width, uint16_t flags)
{
	// pad leading zeros
	if(!(flags & FLAGS_LEFT))
	{
    	if(width && (flags & FLAGS_ZEROPAD) && (negative || (flags & (FLAGS_PLUS | FLAGS_SPACE))))
			width--;

		while((flags & FLAGS_ZEROPAD) && (len < width) && (len < PRNF_NTOA_BUFFER_SIZE))
     		buf[len++] = '0';
  	};

	while((len < prec) && (len < PRNF_NTOA_BUFFER_SIZE))
		buf[len++] = '0';

	// handle hash
	if(flags & FLAGS_HASH)
	{
		if((base == 16) && !(flags & FLAGS_UPPERCASE) && (len < PRNF_NTOA_BUFFER_SIZE))
      		buf[len++] = 'x';

		else if((base == 16) && (flags & FLAGS_UPPERCASE) && (len < PRNF_NTOA_BUFFER_SIZE))
      		buf[len++] = 'X';

    	else if ((base == 2) && (len < PRNF_NTOA_BUFFER_SIZE))
      		buf[len++] = 'b';


    	if (len < PRNF_NTOA_BUFFER_SIZE)
    	{
			// Only add a zero if number does not already start with a zero
      		if(len)
      		{
        		if(buf[len-1]!='0')
					buf[len++] = '0';
      		}
      		else
        		buf[len++] = '0';
    	};
	};

	if(len < PRNF_NTOA_BUFFER_SIZE)
	{
    	if(negative)
    		buf[len++] = '-';

    	else if(flags & FLAGS_PLUS)
      		buf[len++] = '+';  // ignore the space if the '+' exists

    	else if(flags & FLAGS_SPACE)
      		buf[len++] = ' ';
  	};

	if(len == PRNF_NTOA_BUFFER_SIZE)
  		WARN_NTOA_BUFFER_SIZE();

	return out_rev(out_fptr, out_vars, idx, buf, len, width, flags);
}


// internal itoa for 'long' type
#ifdef PRNF_SUPPORT_LONG_LONG
static size_t numtoasc(void(*out_fptr)(char, void*), void* out_vars, size_t idx, unsigned long long value, bool negative, uint8_t base, uint8_t prec, uint8_t width, uint16_t flags)
#else
static size_t numtoasc(void(*out_fptr)(char, void*), void* out_vars, size_t idx, unsigned long value, bool negative, uint8_t base, uint8_t prec, uint8_t width, uint16_t flags)
#endif
{
	size_t len = 0;
	size_t ret;

	#ifdef PRNF_USE_HEAP
		char *buf = heap_allocate(PRNF_NTOA_BUFFER_SIZE);
	#else
		char buf[PRNF_NTOA_BUFFER_SIZE];
	#endif

	// no hash for 0 values
	if(!value)
		flags &= ~FLAGS_HASH;

  	// write if precision != 0 and value is != 0
  	if(!(flags & FLAGS_PRECISION) || value)
  	{
    	do
    	{
      		const char digit = (char)(value % base);
      		buf[len++] = digit < 10 ? '0' + digit : (flags & FLAGS_UPPERCASE ? 'A' : 'a') + digit - 10;
      		value /= base;
    	}while(value && (len < PRNF_NTOA_BUFFER_SIZE));
  	};

	ret = numtoasc_format(out_fptr, out_vars, idx, buf, len, negative, base, prec, width, flags);

	#ifdef PRNF_USE_HEAP
		heap_free(buf);
	#endif
	return ret;
}


#ifdef PRNF_SUPPORT_FLOAT
// internal ftoa for fixed decimal floating point
static size_t fltasc(void(*out_fptr)(char, void*), void* out_vars, size_t idx, double value, uint8_t prec, uint8_t width, uint16_t flags)
{
	uint8_t len  = 0;
	double diff = 0.0;
	size_t ret;

	#ifdef PRNF_USE_HEAP
		char *buf = heap_allocate(PRNF_FTOA_BUFFER_SIZE);
	#else
		char buf[PRNF_FTOA_BUFFER_SIZE];
	#endif

	// powers of 10
	static const double pow10[] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000 };

  	// test for special values
  	if(value != value)
    	return out_rev(out_fptr, out_vars, idx, "nan", 3, width, flags);

  	if(value < -DBL_MAX)
	    return out_rev(out_fptr, out_vars, idx, "fni-", 4, width, flags);

  	if(value > DBL_MAX)
	    return out_rev(out_fptr, out_vars, idx, (flags & FLAGS_PLUS) ? "fni+" : "fni", (flags & FLAGS_PLUS) ? 4 : 3, width, flags);

  	// test for very large values
  	// standard printf behavior is to print EVERY whole number digit -- which could be 100s of characters overflowing your buffers == bad
  	if((value > PRNF_MAX_FLOAT) || (value < -PRNF_MAX_FLOAT))
		return exptoasc(out_fptr, out_vars, idx, value, prec, width, flags);

	// test for negative
	bool negative = false;
	if(value < 0)
	{
		negative = true;
		value = 0 - value;
  	};

	// set default precision, if not set explicitly
  	if(!(flags & FLAGS_PRECISION))
		prec = PRNF_DEFAULT_FLOAT_PRECISION;

  	// limit precision to 9, cause a prec >= 10 can lead to overflow errors
  	while((len < PRNF_FTOA_BUFFER_SIZE) && (prec > 9))
  	{
    	buf[len++] = '0';
    	prec--;
  	};

  	int whole = (int)value;
  	double tmp = (value - whole) * pow10[prec];
  	unsigned long frac = (unsigned long)tmp;
  	diff = tmp - frac;

  	if(diff > 0.5)
  	{
    	++frac;
    	// handle rollover, e.g. case 0.99 with prec 1 is 1.0
    	if (frac >= pow10[prec])
    	{
      		frac = 0;
      		++whole;
    	};
  	}
  	else if(diff < 0.5)
  	{
  	}
  	// if halfway, round up if odd OR if last digit is 0
  	else if((frac == 0) || (frac & 1))
  		frac++;	

	if(prec == 0)
	{
		diff = value - (double)whole;
  		// if exactly 0.5 and ODD, then round up
   		// 1.5 -> 2, but 2.5 -> 2
		if((!(diff < 0.5) || (diff > 0.5)) && (whole & 1))
      		whole++;
  	}
  	else
  	{
    	uint8_t count = prec;
    	// now do fractional part, as an unsigned number
    	while(len < PRNF_FTOA_BUFFER_SIZE)
    	{
      		count--;
      		buf[len++] = (char)(48 + (frac % 10));
      		if(!(frac /= 10))
        		break;
    	};

    	// add extra 0s
    	while((len < PRNF_FTOA_BUFFER_SIZE) && (count-- > 0))
      		buf[len++] = '0';

      	// add decimal
		if(len < PRNF_FTOA_BUFFER_SIZE)
      		buf[len++] = '.';
	};

	// do whole part, number is reversed
  	while(len < PRNF_FTOA_BUFFER_SIZE)
  	{
    	buf[len++] = (char)(48 + (whole % 10));
    	if(!(whole /= 10))
      		break;
    };

	// pad leading zeros
  	if(!(flags & FLAGS_LEFT) && (flags & FLAGS_ZEROPAD))
  	{
    	if(width && (negative || (flags & (FLAGS_PLUS | FLAGS_SPACE)))) {
      	width--;
    	}
    	while((len < width) && (len < PRNF_FTOA_BUFFER_SIZE))
			buf[len++] = '0';
	}

  	if(len < PRNF_FTOA_BUFFER_SIZE)
  	{
    	if(negative)
      		buf[len++] = '-';

    	else if(flags & FLAGS_PLUS)
      		buf[len++] = '+';  // ignore the space if the '+' exists

    	else if(flags & FLAGS_SPACE)
      		buf[len++] = ' ';
  	};

	if(len == PRNF_FTOA_BUFFER_SIZE)
  		WARN_FTOA_BUFFER_SIZE();

	ret = out_rev(out_fptr, out_vars, idx, buf, len, width, flags);
	#ifdef PRNF_USE_HEAP
		heap_free(buf);
	#endif
	return ret;
}

#ifdef PRNF_SUPPORT_EXPONENTIAL
// internal ftoa variant for exponential floating-point type, contributed by Martijn Jasperse <m.jasperse@gmail.com>
static size_t exptoasc(void(*out_fptr)(char, void*), void* out_vars, size_t idx, double value, uint8_t prec, uint8_t width, uint16_t flags)
{
	// check for NaN and special values
	if((value != value) || (value > DBL_MAX) || (value < -DBL_MAX))
		return fltasc(out_fptr, out_vars, idx, value, prec, width, flags);

  	// determine the sign
  	const bool negative = value < 0;
  	if(negative)
    	value = -value;

  	// default precision
  	if(!(flags & FLAGS_PRECISION))
		prec = PRNF_DEFAULT_FLOAT_PRECISION;

  	// determine the decimal exponent
  	// based on the algorithm by David Gay (https://www.ampl.com/netlib/fp/dtoa.c)
  	union
  	{
		uint64_t U;
		double   F;
  	} conv;

	int expval;
	conv.F = value;
	// Don't give 0.0 a massive -308 exponent
  	if(value == 0.0)
    	expval = 0;
  	else
  	{
    	int exp2 = (int)((conv.U >> 52) & 0x07FF) - 1023;           // effectively log2
    	conv.U = (conv.U & ((1ULL << 52) - 1)) | (1023ULL << 52);  // drop the exponent so conv.F is now in [1,2)

    	// now approximate log10 from the log2 integer part and an expansion of ln around 1.5
    	expval = (int)(0.1760912590558 + exp2 * 0.301029995663981 + (conv.F - 1.5) * 0.289529654602168);

    	// now we want to compute 10^expval but we want to be sure it won't overflow
    	exp2 = (int)(expval * 3.321928094887362 + 0.5);
    	const double z  = expval * 2.302585092994046 - exp2 * 0.6931471805599453;
    	const double z2 = z * z;
    	conv.U = (uint64_t)(exp2 + 1023) << 52;

    	// compute exp(z) using continued fractions, see https://en.wikipedia.org/wiki/Exponential_function#Continued_fractions_for_ex
    	conv.F *= 1 + 2 * z / (2 - z + (z2 / (6 + (z2 / (10 + z2 / 14)))));

    	// correct for rounding errors
    	if(value < conv.F)
		{
			expval--;
			conv.F /= 10;
		};
	};

	// the exponent format is "%+03d" and largest value is "307", so set aside 4-5 characters
	uint8_t minwidth = ((expval < 100) && (expval > -100)) ? 4 : 5;

	// in "%g" mode, "prec" is the number of *significant figures* not decimals
	if(flags & FLAGS_ADAPT_EXP)
	{
    	// do we want to fall-back to "%f" mode?
    	if((value >= 1e-4) && (value < 1e6))
    	{
			if((int)prec > expval)
				prec = (unsigned)((int)prec - expval - 1);
			else
				prec = 0;

			flags |= FLAGS_PRECISION;   // make sure fltasc respects precision

			// no characters in exponent
			minwidth = 0;
			expval   = 0;
    	}
    	else if((prec > 0) && (flags & FLAGS_PRECISION))
       		prec--;	// we use one sigfig for the whole part
	};

	// will everything fit?
	uint8_t fwidth = width;
	if(width > minwidth)	
		fwidth -= minwidth;	// we didn't fall-back so subtract the characters required for the exponent
	else
    	fwidth = 0;		// not enough characters, so go back to default sizing

	if((flags & FLAGS_LEFT) && minwidth)
    	fwidth = 0;	// if we're padding on the right, DON'T pad the floating part

  	// rescale the float value
	if(expval)
		value /= conv.F;

	// output the floating part
	const size_t start_idx = idx;
	idx = fltasc(out_fptr, out_vars, idx, negative ? -value : value, prec, fwidth, flags & ~FLAGS_ADAPT_EXP);

	// output the exponent part
	if(minwidth)
	{
		// output the exponential symbol
		out_fptr((flags & FLAGS_UPPERCASE) ? 'E' : 'e', out_vars);
		idx++;

		// output the exponent value
		idx = numtoasc(out_fptr, out_vars, idx, (expval < 0) ? -expval : expval, expval < 0, 10, 0, minwidth-1, FLAGS_ZEROPAD | FLAGS_PLUS);

		// might need to right-pad spaces
		if(flags & FLAGS_LEFT) 
		{
			while(idx - start_idx < width)
			{
				out_fptr(' ', out_vars);
				idx++;
			};
		};
	};

  return idx;
}

#endif //	PRNF_SUPPORT_FLOAT
#endif //	PRNF_SUPPORT_EXPONENTIAL
#endif // 	ndef SECOND_PASS

//********************************************************************************************************
// PSTR compatibility, this code may be compiled twice for PGM and non-PGM versions
//********************************************************************************************************

// wrappers for PGM or non-PGM access
#ifndef SECOND_PASS
	#define FMTRD(_fmt) (*(_fmt))
	#define ATOI(_src)	asctoint(_src)
#else
	#define FMTRD(_fmt)	pgm_read_byte(_fmt)
	#define ATOI(_src) 	asctoint_P(_src)
#endif

// internal ASCII string to unsigned int conversion
#ifndef SECOND_PASS
static uint8_t asctoint(const char** str)
#else
static uint8_t asctoint_P(const char** str)
#endif
{
	uint8_t i = 0;
	while(prnf_is_digit(FMTRD(*str)))
	{
		i *= 10;
		i += FMTRD((*str)++) - '0';
	};
  return i;
}

// internal core_prnf
#ifndef SECOND_PASS
static int core_prnf(void(*out_fptr)(char, void*), void* out_vars, const char* format, va_list va)
#else
static int core_prnf_P(void(*out_fptr)(char, void*), void* out_vars, PGM_P format, va_list va)
#endif
{
	uint16_t flags;
	size_t width;
	uint8_t precision, n;
	size_t idx = 0;

	while(FMTRD(format))
	{
		// format specifier?  %[flags][width][.precision][length]
		if(FMTRD(format) != '%')
		{
			// no
			out_fptr(FMTRD(format), out_vars);
			idx++;
			format++;
			continue;
		}
		else
			// yes, evaluate it
			format++;

		// evaluate flags
		flags = 0;
		do
		{
			switch (FMTRD(format))
			{
				case '0': flags |= FLAGS_ZEROPAD; format++; n = 1; break;
				case '-': flags |= FLAGS_LEFT;    format++; n = 1; break;
				case '+': flags |= FLAGS_PLUS;    format++; n = 1; break;
				case ' ': flags |= FLAGS_SPACE;   format++; n = 1; break;
				case '#': flags |= FLAGS_HASH;    format++; n = 1; break;
				default :                                   n = 0; break;
			};
		}while(n);

		// evaluate width field
		width = 0;
		if(prnf_is_digit(FMTRD(format)))
			width = ATOI(&format);
		else if(FMTRD(format) == '*')
		{
			const int w = va_arg(va, int);
			if(w < 0)
			{
				flags |= FLAGS_LEFT;    // reverse padding
				width = (unsigned int)-w;
			}
			else
				width = (unsigned int)w;

			format++;
		};

		// evaluate precision field
		precision = 0;
		if(FMTRD(format) == '.')
		{
			flags |= FLAGS_PRECISION;
			format++;
			if(prnf_is_digit(FMTRD(format)))
			{
				precision = ATOI(&format);
			}
			else if(FMTRD(format) == '*')
			{
				const int8_t prec = (int8_t)va_arg(va, int);
				precision = prec > 0 ? prec : 0;
				format++;
			};
		};

		// evaluate length field
		switch(FMTRD(format))
		{
			case 'l' :
				flags |= FLAGS_LONG;
				format++;
				if(FMTRD(format) == 'l')
				{
					flags |= FLAGS_LONG_LONG;
					format++;
				};
				break;

			case 'h' :
				flags |= FLAGS_SHORT;
				format++;
				if(FMTRD(format) == 'h')
				{
					flags |= FLAGS_CHAR;
					format++;
				};
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
		};

		// evaluate specifier
		switch(FMTRD(format))
		{
			case 'd' :
			case 'i' :
			case 'u' :
			case 'x' :
			case 'X' :
			case 'o' :
			case 'b' :
			{
				// set the base
				uint8_t base;
				if(FMTRD(format) == 'x' || FMTRD(format) == 'X')
					base = 16;

				else if(FMTRD(format) == 'o')
					base =  8;

				else if(FMTRD(format) == 'b')
					base =  2;
				else
				{
					base = 10;
					flags &= ~FLAGS_HASH;   // no hash for dec format
				};

				// uppercase
				if(FMTRD(format) == 'X')
					flags |= FLAGS_UPPERCASE;

				// no plus or space flag for u, x, X, o, b
				if((FMTRD(format) != 'i') && (FMTRD(format) != 'd'))
					flags &= ~(FLAGS_PLUS | FLAGS_SPACE);

				// ignore '0' flag when precision is given
				if(flags & FLAGS_PRECISION)
					flags &= ~FLAGS_ZEROPAD;

				// convert the integer
				if((FMTRD(format) == 'i') || (FMTRD(format) == 'd'))
				{
					// signed
					if(flags & FLAGS_LONG_LONG)
					{
						#ifdef PRNF_SUPPORT_LONG_LONG
						const long long value = va_arg(va, long long);
						idx = numtoasc(out_fptr, out_vars, idx, (unsigned long long)(value > 0 ? value : 0 - value), value < 0, base, precision, width, flags);
						#endif
					}
					else if(flags & FLAGS_LONG)
					{
						const long value = va_arg(va, long);
						idx = numtoasc(out_fptr, out_vars, idx, (unsigned long)(value > 0 ? value : 0 - value), value < 0, base, precision, width, flags);
					}
					else
					{
						const int value = (flags & FLAGS_CHAR) ? (char)va_arg(va, int) : (flags & FLAGS_SHORT) ? (short int)va_arg(va, int) : va_arg(va, int);
						idx = numtoasc(out_fptr, out_vars, idx, (unsigned int)(value > 0 ? value : 0 - value), value < 0, base, precision, width, flags);
					};
				}
				else
				{
					// unsigned
					if(flags & FLAGS_LONG_LONG)
					{
						#ifdef PRNF_SUPPORT_LONG_LONG
						idx = numtoasc(out_fptr, out_vars, idx, va_arg(va, unsigned long long), false, base, precision, width, flags);
						#endif
					}
					else if(flags & FLAGS_LONG)
						idx = numtoasc(out_fptr, out_vars, idx, va_arg(va, unsigned long), false, base, precision, width, flags);
					else
					{
						const unsigned int value = (flags & FLAGS_CHAR) ? (unsigned char)va_arg(va, unsigned int) : (flags & FLAGS_SHORT) ? (unsigned short int)va_arg(va, unsigned int) : va_arg(va, unsigned int);
						idx = numtoasc(out_fptr, out_vars, idx, value, false, base, precision, width, flags);
					};
				};
				format++;
				break;
			};

	#ifdef PRNF_SUPPORT_FLOAT
			case 'f' :
			case 'F' :
				if(FMTRD(format) == 'F')
					flags |= FLAGS_UPPERCASE;
				idx = fltasc(out_fptr, out_vars, idx, va_arg(va, double), precision, width, flags);
				format++;
				break;

	#ifdef PRNF_SUPPORT_EXPONENTIAL
			case 'e':
			case 'E':
			case 'g':
			case 'G':
				if((FMTRD(format) == 'g')||(FMTRD(format) == 'G'))
					flags |= FLAGS_ADAPT_EXP;
				if((FMTRD(format) == 'E')||(FMTRD(format) == 'G'))
					flags |= FLAGS_UPPERCASE;
				idx = exptoasc(out_fptr, out_vars, idx, va_arg(va, double), precision, width, flags);
				format++;
				break;
	#endif  // PRNF_SUPPORT_EXPONENTIAL
	#endif  // PRNF_SUPPORT_FLOAT

			case 'c' :
			{
				size_t l = 1;
				// pre padding
				if(!(flags & FLAGS_LEFT))
				{
					while(l++ < width)
					{
						out_fptr(' ', out_vars);
						idx++;
					};
				};
				// char output
				out_fptr((char)va_arg(va, int), out_vars);
				idx++;
				// post padding
				if(flags & FLAGS_LEFT)
				{
					while(l++ < width)
					{
						out_fptr(' ', out_vars);
						idx++;
					};
				};
				format++;
				break;
			};

			case 's' :
			{
				const char* p = va_arg(va, char*);
				size_t l = prnf_strnlen_s(p, precision ? precision : (size_t)-1);
				// pre padding
				if(flags & FLAGS_PRECISION)
					l = (l < precision ? l : precision);
				if(!(flags & FLAGS_LEFT))
				{
					while(l++ < width)
					{
						out_fptr(' ', out_vars);
						idx++;
					};
				};
				// string output
				while((*p != 0) && (!(flags & FLAGS_PRECISION) || precision--))
				{
					out_fptr(*(p++), out_vars);
					idx++;
				};
				// post padding
				if(flags & FLAGS_LEFT)
				{
					while(l++ < width)
					{
						out_fptr(' ', out_vars);
						idx++;
					};
				};
				format++;
				break;
			};

			case 'p' :
			{
				width = sizeof(void*) * 2;
				flags |= FLAGS_ZEROPAD | FLAGS_UPPERCASE;
				const bool is_ll = sizeof(uintptr_t) == sizeof(long long);
				if(is_ll)
					idx = numtoasc(out_fptr, out_vars, idx, (uintptr_t)va_arg(va, void*), false, 16, precision, width, flags);
				else
					idx = numtoasc(out_fptr, out_vars, idx, (unsigned long)((uintptr_t)va_arg(va, void*)), false, 16, precision, width, flags);
				format++;
				break;
			};

			case '%' :
				out_fptr('%', out_vars);
				idx++;
				format++;
				break;

			default :
				out_fptr(FMTRD(format), out_vars);
				idx++;
				format++;
				break;
		};
	};

	// termination
	out_fptr((char)0, out_vars);

	// return written chars without terminating \0
	return (int)idx;
}
#undef FMTRD
#undef ATOI

// Compile _P version for PROGMEM access
#ifdef PLATFORM_AVR
	#ifndef SECOND_PASS
		#define SECOND_PASS
		#include "prnf.c"
	#endif
#endif
