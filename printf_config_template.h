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
// \brief printf configuration template
//        Copy this file as printf_config.h into a directory of your project
//        which is on the include path, and define PRINTF_INCLUDE_CONFIG_H
//        globally (e.g. gcc -DPRINTF_INCLUDE_CONFIG_H ...) to include it in printf.c.
//        All switches are commented out, so printf.c uses its defaults.
//        Uncomment the switches you need and adjust their values. They can be
//        defined on the compiler command line as well (e.g. -DPRINTF_MAX_FLOAT=1e12)
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _PRINTF_CONFIG_H_
#define _PRINTF_CONFIG_H_


///////////////////////////////////////////////////////////////////////////////
// conversion buffers

// 'ntoa' conversion buffer size, this must be big enough to hold one converted
// integer number including padded zeros (dynamically created on stack)
// default: 32 byte
//#define PRINTF_NTOA_BUFFER_SIZE    32U

// 'ftoa' conversion buffer size, this must be big enough to hold one converted
// float number including padded zeros (dynamically created on stack)
// default: 32 byte
//#define PRINTF_FTOA_BUFFER_SIZE    32U


///////////////////////////////////////////////////////////////////////////////
// floating point

// disable the support for floating point types (%f, %F, %e, %E, %g, %G)
// the argument of a disabled conversion is consumed and the conversion letter
// is printed, like for an unknown conversion
// default: undefined (floating point support activated)
//#define PRINTF_DISABLE_SUPPORT_FLOAT

// disable the support for the exponential floating point notation (%e, %E, %g, %G)
// the argument of a disabled conversion is consumed and the conversion letter
// is printed. %f prints "ovf" for values above PRINTF_MAX_FLOAT then
// default: undefined (exponential notation activated)
//#define PRINTF_DISABLE_SUPPORT_EXPONENTIAL

// define the default floating point precision, used when no precision is given
// default: 6 digits
//#define PRINTF_DEFAULT_FLOAT_PRECISION  6U

// define the largest float suitable to print with %f (and %g in decimal notation)
// larger values are printed in exponential notation, or as "ovf" without
// exponential support
// default: 1e9
//#define PRINTF_MAX_FLOAT  1e9


///////////////////////////////////////////////////////////////////////////////
// integer types

// disable the support for the long long types (%lld, %llu, %llx...)
// long long arguments are consumed and printed truncated to long then. This also
// applies to %z, %j, %t and %p where these types are wider than long
// (e.g. on 64-bit Windows)
// default: undefined (long long support activated)
//#define PRINTF_DISABLE_SUPPORT_LONG_LONG

// disable the support for the ptrdiff_t type (%t)
// ptrdiff_t is normally defined in <stddef.h> as long or long long type
// default: undefined (ptrdiff_t support activated)
//#define PRINTF_DISABLE_SUPPORT_PTRDIFF_T


///////////////////////////////////////////////////////////////////////////////
// output

// remove printf() and vprintf(), so no _putchar() output function is needed when
// only sprintf(), snprintf(), vsnprintf(), fctprintf() and vfctprintf() are used
// default: undefined (printf() and vprintf() provided)
//#define PRINTF_DISABLE_SUPPORT_PUTCHAR


#if !defined(PRINTF_DISABLE_SUPPORT_PUTCHAR)
/**
 * Output a character from printf to xxx
 * Implement your output here (e.g. write to a UART), or remove this function if _putchar() is
 * implemented elsewhere
 * \param character Character to output
 */
inline void _putchar(char character)
{
  (void)character;
}
#endif


#endif // _PRINTF_CONFIG_H_
