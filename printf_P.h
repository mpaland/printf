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
// \brief Tiny printf, sprintf and snprintf implementation, optimized for speed on
//        embedded systems with a very limited resources.
//        Use this instead of bloated standard/newlib printf.
//        These routines are thread safe and reentrant.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _PRINTF_P_H_
#define _PRINTF_P_H_

#include <stdarg.h>
#include <stddef.h>
#include <avr/pgmspace.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _PRINTF_H_
  #error *** printf_P.h (and anything else which uses __attribute__(format(printf,,))) ) must be included *before* printf.h. Because printf.h redefines 'printf' ***
#endif

#ifndef PLATFORM_AVR
  #warning *** symbol PLATFORM_AVR not defined, add -DPLATFORM_AVR to compiler options *** 
  #define PLATFORM_AVR
#endif

/**
 * Output a character to a custom device like UART, used by the printf() function
 * This function is declared here only. You have to write your custom implementation somewhere
 * \param character Character to output
 */
void _putchar(char character);



/**
 * PROGMEM Compatibility for AVR
 * _SL macros (String Literal) for easy cross platform compatibility.
 * 
 * _SL Stores the formatting string literal in program memory on AVR devices.
 * Also allows the compiler to test the parameter types using the format string, which can't work with PSTR().
 * 
 * 	  printf_SL("%i %s", x, y);
 * 
 * is equivalent to :
 * 
 * 	  printf_P(PSTR("%i %s"), x, y);
 * 
 * And produces the same binary if -Os is used.
 * 
 * The programmer is expected to recognize that printf_SL() is a macro, and not do things like:
 * 	printf_SL("%i\n", i++);	// <--- this will increment i twice on AVR.
 * 
 * The symbol PLATFORM_AVR should be defined for AVR devices, add -DPLATFORM_AVR to compiler options. 
 */

//	Compiler will first test argument types based on format string, then remove the empty function during optimization.
static inline void _fmttst_optout(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
static inline void _fmttst_optout(const char* fmt, ...)
{
}

//	_SL macros for AVR
#define printf_SL(_fmtarg, ...) ({int _prv; _prv = printf_P_(PSTR(_fmtarg) ,##__VA_ARGS__); _fmttst_optout(_fmtarg ,##__VA_ARGS__); _prv;})
#define hprintf_SL(_fmtarg, ...) ({char* _prv; _prv = hprintf_P(PSTR(_fmtarg) ,##__VA_ARGS__); _fmttst_optout(_fmtarg ,##__VA_ARGS__); _prv;})
#define sprintf_SL(_dst, _fmtarg, ...) ({int _prv; _prv = sprintf_P_(_dst, PSTR(_fmtarg) ,##__VA_ARGS__); _fmttst_optout(_fmtarg ,##__VA_ARGS__); _prv;})
#define snprintf_SL(_dst, _cnt, _fmtarg, ...) ({int _prv; _prv = snprintf_P_(_dst, _cnt, PSTR(_fmtarg) ,##__VA_ARGS__); _fmttst_optout(_fmtarg ,##__VA_ARGS__); _prv;})
#define fctprintf_SL(_fptr, _fargs, _fmtarg, ...) ({int _prv; _prv = fctprintf_P(_fptr, _fargs, PSTR(_fmtarg) ,##__VA_ARGS__); _fmttst_optout(_fmtarg ,##__VA_ARGS__); _prv;})
#ifdef PRINTF_EXT_FIFO
#define fifoprintf_SL(_dst, _fmtarg, ...) ({int _prv; _prv = fifoprintf_P(_dst, PSTR(_fmtarg) ,##__VA_ARGS__); _fmttst_optout(_fmtarg ,##__VA_ARGS__); _prv;})
#endif


/**
 * Tiny printf implementation
 * You have to implement _putchar if you use printf()
 * To avoid conflicts with the regular printf() API it is overridden by macro defines
 * and internal underscore-appended functions like printf_() are used
 * \param format A string that specifies the format of the output
 * \return The number of characters that are written into the array, not counting the terminating null character
 */
#define printf_P printf_P_
int printf_P_(PGM_P format, ...);


/**
 * hprintf_P, print to a standard C string on the heap.
 * This will allocate sufficient space using malloc then print to it.
 * This breaks the open-close principal, and is a memory leak risk. Use with great care, or not at all.
 * \param format A string that specifies the format of the output
 * \return The output on the heap, as a standard C string. free() after using.
 */
char* hprintf_P(PGM_P format, ...);

#ifdef PRINTF_EXT_FIFO
/**
 * fifoprintf, print to a non-standard fifo module.
 * \param format A string that specifies the format of the output
 * \return The output on the heap, as a standard C string. free() after using.
 */
int fifoprintf_P(struct fifo_struct *dst, PGM_P format, ...);
#endif

/**
 * Tiny sprintf implementation
 * Due to security reasons (buffer overflow) YOU SHOULD CONSIDER USING (V)SNPRINTF INSTEAD!
 * \param buffer A pointer to the buffer where to store the formatted string. MUST be big enough to store the output!
 * \param format A string that specifies the format of the output
 * \return The number of characters that are WRITTEN into the buffer, not counting the terminating null character
 */
#define sprintf_P sprintf_P_
int sprintf_P_(char* buffer, PGM_P format, ...);


/**
 * Tiny snprintf/vsnprintf implementation
 * \param buffer A pointer to the buffer where to store the formatted string
 * \param count The maximum number of characters to store in the buffer, including a terminating null character
 * \param format A string that specifies the format of the output
 * \param va A value identifying a variable arguments list
 * \return The number of characters that COULD have been written into the buffer, not counting the terminating
 *         null character. A value equal or larger than count indicates truncation. Only when the returned value
 *         is non-negative and less than count, the string has been completely written.
 */
#define snprintf_P  snprintf_P_
#define vsnprintf_P vsnprintf_P_
int  snprintf_P_(char* buffer, size_t count, PGM_P format, ...);
int vsnprintf_P_(char* buffer, size_t count, PGM_P format, va_list va);


/**
 * Tiny vprintf implementation
 * \param format A string that specifies the format of the output
 * \param va A value identifying a variable arguments list
 * \return The number of characters that are WRITTEN into the buffer, not counting the terminating null character
 */
#define vprintf_P vprintf_P_
int vprintf_P_(PGM_P format, va_list va);


/**
 * printf with output function
 * You may use this as dynamic alternative to printf() with its fixed _putchar() output
 * \param out An output function which takes one character and an argument pointer
 * \param arg An argument pointer for user data passed to output function
 * \param format A string that specifies the format of the output
 * \return The number of characters that are sent to the output function, not counting the terminating null character
 */
int fctprintf_P(void (*out)(char character, void* arg), void* arg, PGM_P format, ...);	

#ifdef __cplusplus
}
#endif


#endif  // _PRINTF_P_H_
