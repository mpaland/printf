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

/*
A format specifier follows this prototype: %[flags][width][.precision][length]type

Supported Flags
Flags 	Description
- 	Left-justify within the given field width; Right justification is the default.
+ 	Forces to precede the result with a plus or minus sign (+ or -) even for positive numbers.
By default, only negative numbers are preceded with a - sign.
(space) 	If no sign is going to be written, a blank space is inserted before the value.
# 	Used with o, b, x or X specifiers the value is preceded with 0, 0b, 0x or 0X respectively for values different than zero.
Used with f, F it forces the written output to contain a decimal point even if no more digits follow. By default, if no digits follow, no decimal point is written.
0 	Left-pads the number with zeros (0) instead of spaces when padding is specified (see width sub-specifier).

Supported Width
Width 	Description
(number) 	Minimum number of characters to be printed. If the value to be printed is shorter than this number, the result is padded with blank spaces. The value is not truncated even if the result is larger.
* 	The width is not specified in the format string, but as an additional integer value argument preceding the argument that has to be formatted.

Supported Precision
Precision 	Description
.number 	For integer specifiers (d, i, o, u, x, X): precision specifies the minimum number of digits to be written. If the value to be written is shorter than this number, the result is padded with leading zeros. The value is not truncated even if the result is longer. A precision of 0 means that no character is written for the value 0.
For f and F specifiers: this is the number of digits to be printed after the decimal point. By default, this is 6, maximum is 9.
For s: this is the maximum number of characters to be printed. By default all characters are printed until the ending null character is encountered.
If the period is specified without an explicit value for precision, 0 is assumed.
.* 	The precision is not specified in the format string, but as an additional integer value argument preceding the argument that has to be formatted.

Supported Length

The length sub-specifier modifies the length of the data type.
Length 	d i 	u o x X
(none) 	int 	unsigned int
hh 	char 	unsigned char
h 	short int 	unsigned short int
l 	long int 	unsigned long int
ll 	long long int 	unsigned long long int (if PRINTF_SUPPORT_LONG_LONG is defined)
j 	intmax_t 	uintmax_t
z 	size_t 	size_t
t 	ptrdiff_t 	ptrdiff_t (if PRINTF_SUPPORT_PTRDIFF_T is defined)

Supported Types:
Type 	Output
d or i 	Signed decimal integer
u 	Unsigned decimal integer
b 	Unsigned binary
o 	Unsigned octal
x 	Unsigned hexadecimal integer (lowercase)
X 	Unsigned hexadecimal integer (uppercase)
f or F 	Decimal floating point
e or E 	Scientific-notation (exponential) floating point
g or G 	Scientific or decimal floating point
c 	Single character
s 	String of characters
p 	Pointer address
% 	A % followed by another % character will write a single %

S	String of characters (same as s). Or for AVR targets, a string of characters in program memory.
	Format testing expects wchar_t* for %S, use the wrapper PRNF_ARG_SL("my text") for string literal arguments to prnf
	prnf_SL("%50S\r\n", PRNF_ARG_SL("Right aligned in 50"));
	prnf_SL("%-50S\r\n", PRNF_ARG_SL("Left aligned in 50"));

	The above will store both the formatting string and the argument in program memory on AVR, or ram on non-avr targets

	For string constants in program memory, you will need to cast to (wchat_t*) to avoid type warnings.

*/

//********************************************************************************************************
// Configurable defines
//********************************************************************************************************

//	minimal is ~2.3k, all options is ~6.4k
	#define PRNF_SUPPORT_LONG_LONG
	#define PRNF_SUPPORT_FLOAT
	#define PRNF_SUPPORT_EXPONENTIAL

// 	define the default floating point precision
// 	default: 6 digits
	#define PRNF_DEFAULT_FLOAT_PRECISION  6

// 	define the largest float suitable to print with %f
// 	default: 1e9
	#define PRNF_MAX_FLOAT	1e9

// 	'ntoa' or 'ftoa' conversion buffer size, this must be big enough to hold one converted
// 	int or float number including padded zeros 
	#define PRNF_BUFFER_SIZE    32

//	Called if above buffer size is *reached* (although not necessarily exceeded)
  	#define PRNF_WARN_BUFFER_SIZE()	((void)0)
 
//	Above buffer storage, default is to use the stack which is both re-enterent and thread safe.
//	Or, choose 1 alternative:
//	#define PRNF_BUFFER_STATIC
//	#define PRNF_BUFFER_HEAP

//	Buffer storage for number conversions, buffer size will still 	
//	uncomment to replace std printf (& friends) with prnf using macros
//	*caution* 
//	due to this replacing 'printf' it will break later function declarations attempting to use __attribute__((format(printf, 1, 2)))
//	#define OVERRIDE_STD_PRINTF

//********************************************************************************************************
// Public defines
//********************************************************************************************************


#ifdef PLATFORM_AVR
//	Compiler will first test argument types based on format string, then remove the empty function during optimization.
	static inline void fmttst_optout(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
	static inline void fmttst_optout(const char* fmt, ...)
	{
	}

//	_SL macros for AVR
	#define prnf_SL(_fmtarg, ...) 						({int _prv; _prv = prnf_P(PSTR(_fmtarg) ,##__VA_ARGS__); fmttst_optout(_fmtarg ,##__VA_ARGS__); _prv;})
	#define sprnf_SL(_dst, _fmtarg, ...) 				({int _prv; _prv = sprnf_P(_dst, PSTR(_fmtarg) ,##__VA_ARGS__); fmttst_optout(_fmtarg ,##__VA_ARGS__); _prv;})
	#define snprnf_SL(_dst, _dst_size, _fmtarg, ...) 	({int _prv; _prv = snprnf_P(_dst, _dst_size, PSTR(_fmtarg) ,##__VA_ARGS__); fmttst_optout(_fmtarg ,##__VA_ARGS__); _prv;})
	#define snappf_SL(_dst, _dst_size, _fmtarg, ...) 	({int _prv; _prv = snappf_P(_dst, _dst_size, PSTR(_fmtarg) ,##__VA_ARGS__); fmttst_optout(_fmtarg ,##__VA_ARGS__); _prv;})
	#define fptrprnf_SL(_fptr, _fargs, _fmtarg, ...) 	({int _prv; _prv = fctprnf_P(_fptr, _fargs, PSTR(_fmtarg) ,##__VA_ARGS__); fmttst_optout(_fmtarg ,##__VA_ARGS__); _prv;})
	#define PRNF_ARG_SL(_arg)							((wchar_t*)PSTR(_arg))
#else

	#define prnf_SL(_fmtarg, ...) 						prnf(_fmtarg ,##__VA_ARGS__)
	#define sprnf_SL(_dst, _fmtarg, ...) 				sprnf(_dst, _fmtarg ,##__VA_ARGS__)
	#define snprnf_SL(_dst, _dst_size, _fmtarg, ...) 	snprnf(_dst, _dst_size, _fmtarg ,##__VA_ARGS__)
	#define snappf_SL(_dst, _dst_size, _fmtarg, ...) 	snappf(_dst, _dst_size, _fmtarg ,##__VA_ARGS__)
	#define fptrprnf_SL(_fptr, _fargs, _fmtarg, ...)	fctprnf(_fptr, _fargs, _fmtarg ,##__VA_ARGS__)
	#define PRNF_ARG_SL(_arg)							((wchar_t*)(_arg))
#endif


//********************************************************************************************************
// Public prototypes
//********************************************************************************************************

//	Define externally for output of prnf()
	void prnf_putc(char character);

	int prnf(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
	int sprnf(char* dst, const char* fmt, ...) __attribute__((format(printf, 2, 3)));
	int snprnf(char* dst, size_t dst_size, const char* fmt, ...) __attribute__((format(printf, 3, 4)));
	int snappf(char* dst, size_t dst_size, const char* fmt, ...) __attribute__((format(printf, 3, 4)));
	int fptrprnf(void(*out_fptr)(char, void*), void* out_vars, const char* fmt, ...) __attribute__((format(printf, 3, 4)));
	int vprnf(const char* fmt, va_list va);
	int vsnprnf(char* dst, size_t dst_size, const char* fmt, va_list va);
	int vsprnf(char* dst, const char* fmt, va_list va);
	int vfptrprnf(void(*out_fptr)(char, void*), void* out_vars, const char* fmt, va_list va);

#ifdef PLATFORM_AVR
	int prnf_P(PGM_P fmt, ...);
	int sprnf_P(char* dst, PGM_P fmt, ...);
	int snprnf_P(char* dst, size_t dst_size, PGM_P fmt, ...);
	int snappf_P(char* dst, size_t dst_size, PGM_P fmt, ...);
	int fptrprnf_P(void(*out_fptr)(char, void*), void* out_vars, PGM_P fmt, ...);	
	int vprnf_P(PGM_P fmt, va_list va);
	int vsprnf_P(char* dst, PGM_P fmt, va_list va);
	int vsnprnf_P(char* dst, size_t dst_size, PGM_P fmt, va_list va);
	int vfptrprnf_P(void(*out_fptr)(char, void*), void* out_vars, PGM_P fmt, va_list va);
#endif

#ifdef OVERRIDE_STD_PRINTF

//	Todo: check using 'wrap' feature with compiler flags as an alternative?
//	Override stdio interface for AVR and non-avr
	#define sprintf_P 	sprnf_P
	#define printf_P 	prnf_P
	#define snprintf_P  snprnf_P
	#define vsnprintf_P vsnprnf_P
	#define vsprintf_P 	vsprnf_P
	#define vprintf_P 	vprnf_P
	#define snprintf  	snprnf
	#define vsnprintf 	vsnprnf
	#define vsprintf 	vsprnf
	#define sprintf	 	sprnf
	#define vprintf 	vprnf

//	This next line must come last, and after any other declarations which wish to use __attribute__((format(printf
	#define printf 		prnf

#endif
