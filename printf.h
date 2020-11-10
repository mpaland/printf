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
	int vsnprintf_P_(char* buffer, size_t count, PGM_P format, va_list va);
	int vprintf_P_(PGM_P format, va_list va);
	int fctprintf_P(void (*out)(char character, void* arg), void* arg, PGM_P format, ...);	
#endif

	int sprintf_(char* buffer, const char* format, ...) __attribute__((format(printf, 2, 3)));
	int  snprintf_(char* buffer, size_t count, const char* format, ...) __attribute__((format(printf, 3, 4)));
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
