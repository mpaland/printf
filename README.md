# Standalone printf/sprintf formatted printing function library

[![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](https://raw.githubusercontent.com/eyalroz/printf/master/LICENSE)
[![Github Bug-type issues](https://shields.io/github/issues-search/eyalroz/printf?query=is:open%20is:issue%20label:bug&label=open%20bugs)](http://github.com/eyalroz/printf/issues)
<sup>Parent repository: </sup>[![Github issues (original repo)](https://img.shields.io/github/issues/mpaland/printf.svg)](http://github.com/mpaland/printf/issues)
<!-- Can't use Travis - they stopped offering free builds [![Build Status](https://travis-ci.com/eyalroz/printf.svg?branch=master)](https://travis-ci.com/eyalroz/printf) -->
<!-- No releases yet... [![Github Releases](https://img.shields.io/github/release/eyalroz/printf.svg)](https://github.com/mpaland/eyalroz/releases)-->


| Table of contents |
|:------------------|
|<sub>[Highlights, design goals and the fork](#highlights-design-goals-and-the-fork)<br>[Using the `printf` library in your project](#using-the-printf-library-in-your-project)<br>  - [CMake options and preprocessor definitions](#cmake-options-and-preprocessor-definitions)<br>[Library API](#library-api)<br>  - [Implemented functions](#implemented-functions)<br>  - [Supported Format Specifiers](#supported-format-specifiers)<br>  - [Return Value](#return-value)<br>[Contributing](#contributing)<br>[License](#license) </sub>|


This is a small but **fully-loaded** implementation of C's formatted printing family of functions. It was originally designed by Marco Paland, with the primary use case being in embedded systems - where these functions are unavailable, or when one needs to avoid the memory footprint of linking against a full-fledged libc. The library can be made even smaller by partially excluding some of the supported format specifiers during compilation. The library stands alone, with **No external dependencies**.

It is a fork of the original [mpaland/printf](https://github.com/mpaland/printf) repository by [Marco Paland](https://github.com/mpaland), with multiple bug fixes and a few more features.

## Highlights, design goals and the fork

If you use a typical libc's `sprintf()` implementation (or similar function), you are likely to pull in a *lot* of unwanted library definitions and can bloat code size - typically by as much as 20 KiB. Now, there is a boatload of so called 'tiny' `printf()`-family implementations around. So why this one? Or rather, why [mpaland/printf](https://github.com/mpaland/printf), and then why this fork? 

Well, Marco tried out many of the available `printf()` implementations, but was disappointed: Some are not thread-safe; some have indirect dependencies on libc or other libraries, making them inconvenient to build and larger when compiled; some only offer extremely limited flag and specifier support; and some produce non-standard-compiled output, failing many tests no found in the repository's test suite.

Marco therefore decided to write his own implementation, with the following goals in mind (I've dropped a few relative to his original description):

 - Very small implementation
 - NO dependencies on other packages or libraries; no multiple compiled objects, just one object file.
 - Support for all standard specifiers and flags, and all width and precision sub-specifiers (see below).
 - Support of decimal/floating number representation (with an internal, relatively fast `itoa`/`ftoa` implementation)
 - Reentrancy and thread-safety; `malloc()` freeness.
 - Clean, robust code.
 - Extensive test coverage.
 - MIT license

Marco's repository upheld most of these goals - but did not quite make it all of the way. As of mid-2021, it still had many C-standard-non-compliance bugs; the test suite was quite lacking in coverage; some goals were simply discarded (like avoiding global/local-static constants) etc. The repository had become quite popular, but unfortunately, Marco had been otherwise preoccupied; he had not really touched the code in the two years prior; many bug reports were pending, and so were many pull requests from eary adopters who had fixed some of the bugs they had encountered.

The author of this fork was one of the latercomer bug-reporters-and-PR-authors; and when noticing nothing was moving forward, decided to take up the same goals (sans the discarded ones); and integrate the existing forks and available PRs into a single "consensus fork" which would continue where Marco had left off. Along the way, numerous other issues were observed; the build system was improved; the test suite streamlined and expanded; and other contributors also lent a hand (especially [@mickjc750](https://github.com/mickjc750/)). We are now very close to fully realizing the project goals.

## Using the `printf` library in your project

There are at least four ways to use this library:

1. Use CMake to configure, build and install the library. Then, in another CMake project, use `find_package(printf)` and make sure the install location is in the package search path.
2. Use CMake to configure and build the library. You now have a library file (named `printf.a`, or `printf.so`, or `printf.dll` depending on your platform and choice of static vs dynamic linking), a header file, `printf.h`, and an optional extra header file `printf_config.h` with the build configuration details (usually unnecessary). In your project, if you include `printf.h` and link against the library file, you're all set: There are no dependencies.
3. Copy `printf.c` and `printf.h` into your own project, and build them yourself. Note the various preprocessor options controlling the library's behavior! You will have to set them yourself, or accept with the default values (which are quite reasonable). Remember that the library requires compilation with the C99 language standard enabled.
4. Include the contents of `printf.c` into your own code. This works well enough - whether it's a C or C++ file, and even within a namespace. You again need to consider the preprocessor options controlling behavior, and the language standard.

As mentioned earlier, and with all four options - using `printf()` or `vprintf()` will require implementing a `_putchar(char c)` function. If you like, you can have the library functions alias the standard library function names (e.g. have `snprintf()` really invoke `snprintf_()`) using a CMake configuration option or directly with an appropriate macro.

If you've started using the library in a publicly-available (FOSS or commercial) project, please consider emailing [@eyalroz](https://github.com/eyalroz), or open an [issue](https://github.com/eyalroz/printf/issues/), to announce this.

**Important note: The `sprintf` function is dangerous and insecure, and should be avoided in favor of `snprintf`:** `sprintf()` is unaware of the amount of memory allocated for the printed string, and will "happily" overflow your buffer; instead, pass your buffer size to `snprintf()` - and avoid overflow.

### CMake options and preprocessor definitions

| CMake option name               | Preprocessor definition                | Default value | Description |
|---------------------------------|----------------------------------------|---------------|-------------|
| (always on)                     | PRINTF_INCLUDE_CONFIG_H                | undefined     | Define this as compiler switch (e.g. `gcc -DPRINTF_INCLUDE_CONFIG_H`) to include a "printf_config.h" definition file |
| ALIAS_STANDARD_FUNCTION_NAMES   | PRINTF_ALIAS_STANDARD_FUNCTION_NAMES   | No            | Alias the standard library function names (`printf()`, `sprintf()` etc.) to the library's functions |
| NTOA_BUFFER_SIZE                | PRINTF_NTOA_BUFFER_SIZE                | 32            | ntoa (integer) conversion buffer size. This must be big enough to hold one converted numeric number _including_ leading zeros, normally 32 is a sufficient value. Created on the stack. |
| FTOA_BUFFER_SIZE                | PRINTF_FTOA_BUFFER_SIZE                | 32            | ftoa (float) conversion buffer size. This must be big enough to hold one converted float number _including_ leading zeros, normally 32 is a sufficient value. Created on the stack. |
| DEFAULT_FLOAT_PRECISION         | PRINTF_DEFAULT_FLOAT_PRECISION         | 6             | Define the default floating point precision|
| MAX_INTEGRAL_DIGITS_FOR_DECIMAL | PRINTF_MAX_INTEGRAL_DIGITS_FOR_DECIMAL | 9             | Maximum number of integral-part digits of a floating-point value for which printing with %f uses decimal (non-exponential) notation |
| SUPPORT_FLOAT_SPECIFIERS        | PRINTF_SUPPORT_FLOAT_SPECIFIERS        | Yes           | Support decimal notation floating-point conversion specifiers (%f,%F) |
| SUPPORT_EXPONENTIAL_SPECIFIERS  | PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS  | Yes           | Support exponential floating point format conversion specifiers (%e,%E,%g,%G)" |
| SUPPORT_LONG_LONG               | PRINTF_SUPPORT_LONG_LONG               | Yes           | Support long long integral types (allows for the ll length modifier and affects %p) |
| SUPPORT_PTRDIFF_LENGTH_MODIFIER | PRINTF_SUPPORT_PTRDIFF_LENGTH_MODIFIER | Yes           | Support the pointer difference specifier (%t), used for `ptrdiff_t` variables" |
| BUILD_STATIC_LIBRARY            | (none)                                 | No            | Build a library out of a shared object (dynamically linked at load time) rather than a static one (baked into the executables you build)|

Note: The preprocessor definitions are taken into account when compiling `printf.c`, _not_ when using the compiled library by including `printf.h`.

## Library API

### Implemented functions

The library offers the following, with the same signatures as in the standard C library (plus an extra underscore):
```
int printf_(const char* format, ...);
int sprintf_(char* buffer, const char* format, ...);
int vsprintf_(char* buffer, const char* format, va_list va);
int snprintf_(char* buffer, size_t count, const char* format, ...);
int vsnprintf_(char* buffer, size_t count, const char* format, va_list va);
int vprintf_(const char* format, va_list va);
```
Note that `printf()` and `vprintf()`  don't actually write anything on their own: In addition to their parameters, you must provide them with a lower-level `_putchar()` function which they can call for actual printing. This is part of this library's independence: It is isolated from dealing with console/serial output, files etc.

Two additional functions are provided beyond those available in the standard library:
```
int fctprintf(void (*out)(char character, void* arg), void* arg, const char* format, ...);
int vfctprintf(void (*out)(char character, void* arg), void* arg, const char* format, va_list va);
```
These higher-order functions allow for better flexibility of use: You can decide to do different things with the individual output characters: Encode them, compress them, filter them, append them to a buffer or a file, or just discard them. This is achieved by you passing a pointer to your own state information - through `(v)fctprintf()` and all the way to your own `out()` function.

#### "... but I don't like the underscore-suffix names :-("

You can [configure](#CMake-options-and-preprocessor-definitions) the library to alias the standard library's names, in which case it exposes `printf()`, `sprintf()`, `vsprintf()` and so on. Alternatively, you can write short wrappers with your preferred names. This is completely trivial with the v-functions, e.g.:
```
int my_vprintf(const char* format, va_list va)
{
  return vprintf_(format, va);
}
```
and is still pretty straightforward with the variable-number-of-arguments functions:
```
int my_sprintf(char* buffer, const char* format, ...)
{
  va_list va;
  va_start(va, format);
  const int ret = vsprintf_(buffer, format, va);
  va_end(va);
  return ret;
}
```

### Supported Format Specifiers

A format specifier follows this prototype: `%[flags][width][.precision][length]type`
The following format specifiers are supported:

#### Types

| Type   | Output |
|--------|--------|
| d or i | Signed decimal integer |
| u      | Unsigned decimal integer	|
| b      | Unsigned binary |
| o      | Unsigned octal |
| x      | Unsigned hexadecimal integer (lowercase) |
| X      | Unsigned hexadecimal integer (uppercase) |
| f or F | Decimal floating point |
| e or E | Scientific-notation (exponential) floating point |
| g or G | Scientific or decimal floating point |
| c      | Single character |
| s      | String of characters |
| p      | Pointer address |

Notes:

* The `%a` specifier for hexadecimal floating-point notation (introduced in C99 and C++11) is _not_ currently supported.
* If you want to print the percent sign (`%`, US-ASCII character 37), use "%%" in your format string.
* `printf()`-style functions don't accept `float` arguments, only `double`'s; this is true in the C standard library and for this one as well. `float`'s get converted.

#### Flags

| Flags | Description |
|-------|-------------|
| -     | Left-justify within the given field width; Right justification is the default. |
| +     | Forces to precede the result with a plus or minus sign (+ or -) even for positive numbers.<br>By default, only negative numbers are preceded with a - sign. |
| (space) | If no sign is going to be written, a blank space is inserted before the value. |
| #     | Used with o, b, x or X specifiers the value is preceded with 0, 0b, 0x or 0X respectively for values different than zero.<br>Used with f, F it forces the written output to contain a decimal point even if no more digits follow. By default, if no digits follow, no decimal point is written. |
| 0     | Left-pads the number with zeros (0) instead of spaces when padding is specified (see width sub-specifier). |


#### Width Specifiers

| Width    | Description |
|----------|-------------|
| (number) | Minimum number of characters to be printed. If the value to be printed is shorter than this number, the result is padded with blank spaces. The value is not truncated even if the result is larger. |
| *        | The width is not specified in the format string, but as an additional integer value argument preceding the argument that has to be formatted. |


#### Precision Specifiers

| Precision	| Description |
|-----------|-------------|
| .number   | For integer specifiers (d, i, o, u, x, X): precision specifies the minimum number of digits to be written. If the value to be written is shorter than this number, the result is padded with leading zeros. The value is not truncated even if the result is longer. A precision of 0 means that no character is written for the value 0.<br>For f and F specifiers: this is the number of digits to be printed after the decimal point. **By default, this is 6, and a maximum is defined when building the library**.<br>For s: this is the maximum number of characters to be printed. By default all characters are printed until the ending null character is encountered.<br>If the period is specified without an explicit value for precision, 0 is assumed. |
| .*        | The precision is not specified in the format string, but as an additional integer value argument preceding the argument that has to be formatted. |


#### Length modifiers

The length sub-specifier modifies the length of the data type.

| Length | With `d`, `i`             | With `u`,`o`,`x`, `X`  | Support enabled by...           |
|--------|---------------------------|------------------------|---------------------------------|
| (none) | int                       | unsigned int           |                                 |
| hh     | signed char               | unsigned char          |                                 |
| h      | short int                 | unsigned short int     |                                 |
| l      | long int                  | unsigned long int      |                                 |
| ll     | long long int             | unsigned long long int | PRINTF_SUPPORT_LONG_LONG        |
| j      | intmax_t                  | uintmax_t              |                                 |
| z      | signed version of size_t  | size_t                 |                                 |
| t      | ptrdiff_t                 | ptrdiff_t              | SUPPORT_PTRDIFF_LENGTH_MODIFIER |

Notes:

* The `L` modifier, for `long double`, is not currently supported.
* A `"%zd"` or `"%zi"` takes a signed integer of the same size as `size_t`. 
* The implementation currently assumes each of `intmax_t`, signed `size_t`, and `ptrdiff_t` has the same size as `long int` or as `long long int`. If this is not the case for your platform, please open an issue.

### Return Value

Upon successful return, all functions return the number of characters written, _excluding_ the terminating NUL character used to end the string.
Functions `snprintf()` and `vsnprintf()` don't write more than `count` bytes, including the terminating NUL character ('\0').
Anyway, if the output was truncated due to this limit, the return value is the number of characters that _could_ have been written.
Notice that a value equal or larger than `count` indicates a truncation. Only when the returned value is non-negative and less than `count`,
the string has been completely written with a terminating NUL.
If any error is encountered, `-1` is returned.

If `NULL` is passed for the `buffer` parameter, nothing is written, but the formatted length is returned. For example:
```C
int length = sprintf(NULL, "Hello, world"); // length is set to 12
```

## Contributing

The following assumes Marco Paland's original repository remains mostly-inactive in terms of commits.

0. Give this repository a :star: (even if you've already starred the original repository).
1. Create an [issue](https://github.com/eyalroz/issues) and describe your idea. Make sure it is in line with the library's design goals.
2. Fork the repository
3. Create your feature branch (`git checkout -b my-new-feature`).
4. Implement your feature/idea; don't forget to make sure all existing tests still pass.
5. Add new checks or test-cases to the test suite - both for any problems you have identified and for any new functionality you have introduced.
4. Commit your changes (`git commit -a -m "Added some feature"`)
5. Publish the branch (`git push origin my-new-feature`)
6. Create a new pull request against this repository.

I will try to attend to PRs promptly.


## License

This library is published under the terms of the [MIT license](http://www.opensource.org/licenses/MIT).

