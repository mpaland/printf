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


*/

//********************************************************************************************************
// Public defines
//********************************************************************************************************


#ifdef PLATFORM_AVR
//	Compiler will first test argument types based on format string, then remove the empty function during optimization.
	static inline void _fmttst_optout(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
	static inline void _fmttst_optout(const char* fmt, ...)
	{
	}

//	_SL macros for AVR
	#define printf_SL(_fmtarg, ...) 					({int _prv; _prv = printf_P_(PSTR(_fmtarg) ,##__VA_ARGS__); _fmttst_optout(_fmtarg ,##__VA_ARGS__); _prv;})
	#define hprintf_SL(_fmtarg, ...) 					({char* _prv; _prv = hprintf_P(PSTR(_fmtarg) ,##__VA_ARGS__); _fmttst_optout(_fmtarg ,##__VA_ARGS__); _prv;})
	#define sprintf_SL(_dst, _fmtarg, ...) 				({int _prv; _prv = sprintf_P_(_dst, PSTR(_fmtarg) ,##__VA_ARGS__); _fmttst_optout(_fmtarg ,##__VA_ARGS__); _prv;})
	#define snprintf_SL(_dst, _cnt, _fmtarg, ...) 		({int _prv; _prv = snprintf_P_(_dst, _cnt, PSTR(_fmtarg) ,##__VA_ARGS__); _fmttst_optout(_fmtarg ,##__VA_ARGS__); _prv;})
	#define fctprintf_SL(_fptr, _fargs, _fmtarg, ...) 	({int _prv; _prv = fctprintf_P(_fptr, _fargs, PSTR(_fmtarg) ,##__VA_ARGS__); _fmttst_optout(_fmtarg ,##__VA_ARGS__); _prv;})
	#define fifoprintf_SL(_dst, _fmtarg, ...) 			({int _prv; _prv = fifoprintf_P(_dst, PSTR(_fmtarg) ,##__VA_ARGS__); _fmttst_optout(_fmtarg ,##__VA_ARGS__); _prv;})

#else

	#define printf_SL(_fmtarg, ...) 					printf_(_fmtarg ,##__VA_ARGS__)
	#define hprintf_SL(_fmtarg, ...) 					hprintf(_fmtarg ,##__VA_ARGS__)
	#define sprintf_SL(_dst, _fmtarg, ...) 				sprintf_(_dst, _fmtarg ,##__VA_ARGS__)
	#define snprintf_SL(_dst, _cnt, _fmtarg, ...) 		snprintf_(_dst, _cnt, _fmtarg ,##__VA_ARGS__)
	#define fctprintf_SL(_fptr, _fargs, _fmtarg, ...)	fctprintf(_fptr, _fargs, _fmtarg ,##__VA_ARGS__)
	#define fifoprintf_SL(_dst, _fmtarg, ...) 			fifoprintf(_dst, _fmtarg ,##__VA_ARGS__)

#endif


//********************************************************************************************************
// Public prototypes
//********************************************************************************************************

//	Define externally for output of printf()
	void _putchar(char character);

#ifdef PLATFORM_AVR
	int printf_P_(PGM_P format, ...);
	char* hprintf_P(PGM_P format, ...);
	int fifoprintf_P(struct fifo_struct *dst, PGM_P format, ...);
	int sprintf_P_(char* buffer, PGM_P format, ...);
	int  snprintf_P_(char* buffer, size_t count, PGM_P format, ...);
	int snappendf_P(char* dst, size_t cnt, PGM_P fmt, ...);
	int vsnprintf_P_(char* buffer, size_t count, PGM_P format, va_list va);
	int vprintf_P_(PGM_P format, va_list va);
	int fctprintf_P(void (*out)(char character, void* arg), void* arg, PGM_P format, ...);	
#endif

	int sprintf_(char* buffer, const char* format, ...) __attribute__((format(printf, 2, 3)));
	int snprintf_(char* buffer, size_t count, const char* format, ...) __attribute__((format(printf, 3, 4)));
	int snappendf(char* buffer, size_t count, const char* format, ...) __attribute__((format(printf, 3, 4)));
	int vsnprintf_(char* buffer, size_t count, const char* format, va_list va);
	int vprintf_(const char* format, va_list va);
	int fctprintf(void (*out)(char character, void* arg), void* arg, const char* format, ...) __attribute__((format(printf, 3, 4)));
	char* hprintf(const char* format, ...) __attribute__((format(printf, 1, 2)));
	int fifoprintf(struct fifo_struct *dst, const char* format, ...) __attribute__((format(printf, 2, 3)));
	int printf_(const char* format, ...) __attribute__((format(printf, 1, 2)));

//	Override stdio interface for AVR and non-avr
	#define sprintf_P sprintf_P_
	#define printf_P printf_P_
	#define snprintf_P  snprintf_P_
	#define vsnprintf_P vsnprintf_P_
	#define vprintf_P vprintf_P_
	#define snprintf  snprintf_
	#define vsnprintf vsnprintf_
	#define sprintf sprintf_
	#define vprintf vprintf_

//	This next line must come last, and after any other declarations which wish to use __attribute__((format(printf
	#define printf printf_
