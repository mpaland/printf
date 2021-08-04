# printf / sprintf Implementation for Embedded Systems

[![Build Status](https://travis-ci.com/eyalroz/printf.svg?branch=master)](https://travis-ci.com/eyalroz/printf)
[![Github Releases](https://img.shields.io/github/release/mpaland/printf.svg)](https://github.com/mpaland/printf/releases)
[![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](https://raw.githubusercontent.com/eyalroz/printf/master/LICENSE)
[![Github Bug-type issues](https://shields.io/github/issues-search/eyalroz/printf?query=is:open%20is:issue%20label:bug&label=open%20bugs)](http://github.com/eyalroz/printf/issues)
Parent repo: [![Github issues (original repo)](https://img.shields.io/github/issues/mpaland/printf.svg)](http://github.com/mpaland/printf/issues)

(This is a fork of the [mpaland/printf](https://github.com/mpaland/printf) repository, with multiple bug fixes and a few more features applied.)

This is a tiny but **fully loaded** printf, sprintf and (v)snprintf implementation.
Primarily designed for usage in embedded systems, where printf is not available due to memory issues or in avoidance of linking against libc.
Using the standard libc printf may pull **a lot** of unwanted library stuff and can bloat code size about 20k or is not 100% thread safe. In this cases the following implementation can be used.
Absolutely **NO dependencies** are required, *printf.c* brings all necessary routines, even its own fast `ftoa` (floating point), `ntoa` (decimal) conversion.

If memory footprint is really a critical issue, floating point, exponential and 'long long' support and can be turned off via compiler definitions or CMake c onfiguration options (details below). When using this library's `printf()` and `vprintf()` functions (instead of the standard library's) you have to provide your own `_putchar()` low-level function - as this library is isolated from dealing with console/serial output, files etc.

## Highlights and Design Goals

There is a boatload of so called 'tiny' printf implementations around. So why this one? [Marco Paland](https://github.com/mpaland) before creating this one, but most of them had very limited flag/specifier support, a lot of other dependencies or were just not standard compliant, and failing most tests in this repository's current test suite. Macro therefore decided to write his own implementation, with the following goals in mind:

 - Very small implementation (under 700 lines of code as of July 2021)
 - NO dependencies on other packages or libraries, no multiple compiled objects, just one object file.
 - Support of all standard flags, and all width and precision sub-specifiers (see below)
 - Support of decimal/floating number representation (with an internal, relatively fast `itoa`/`ftoa` implementation)
 - Reentrant and thread-safe; `malloc()` free; no global or static-local variables or buffers.
 - Clean, mature and robust code; passes linting and compilation with no warnings; full coverage by testcases; automotive ready.
 - Extensive test suite (currently over 400 test cases) passing.
 - Simply the best *printf* around the net <- (although that's always a matter of opinion)
 - MIT license


## Usage

There are at least 4 ways to use this library:

1. Use CMake to configure, build and install the library. Then, in another CMake project, use `find_package(mpaland-printf)` and make sure the install location is in the package search path.
2. Use CMake to configure and build the library. You now have a library file (shared or static, depending on your choice), e.g. `printf.a` on Unix machines; and a header file, `printf.h`. In your project, if you include `printf.h` and link against the library file, you're all set (remember - no dependencies).
3. Copy `printf.c` and `printf.h` into your own project, and build them yourself. Note the various preprocessor options controlling the library's behavior! You will have to set them yourself, or live with the default values (which are quite reasonable). Remember that the library requires compilation with the C99 language standard.
4. Include the contents of `printf.c` into your own code. This works well enough - whether it's a C or C++ file, and even within a namespace. With this option also you need to consider the preprocessor options controlling behavior, and the language standard.

One caveat to the above, is that if you want to use the `printf()` function and print to the standard output stream or to a file - you will need to implement a low-level output function for the library to use:

```C
void _putchar(char character)
{
  // send char to console etc.
}
```

Function names within the library correspond to function names in `stdio.h`, but with a `_` suffix so as not to clash with them:
```C
int printf_(const char* format, ...);
int sprintf_(char* buffer, const char* format, ...);
int snprintf_(char* buffer, size_t count, const char* format, ...);
int vsnprintf_(char* buffer, size_t count, const char* format, va_list va);

// Higher-order functions - not exist in the standard library
int fctprintf_(void (*out)(char character, void* arg), void* arg, const char* format, ...);
int vfctprintf_(void (*out)(char character, void* arg), void* arg, const char* format, va_list va);
```
You also have the option to enable an aliasing of the standard function names, so that `printf()`, `vprintf()` etc. invoke the functions from this library.

```

**Important note: Due to general security reasons it is highly recommended to prefer and use `snprintf` (with the max buffer size as `count` parameter) instead of `sprintf`.** `sprintf` has no buffer limitation, and will "happily" overflow your buffer; so only use it when absolutely necessary, and with care!

### Streamlike Usage
Besides the regular standard `printf()` functions, this module also provides `fctprintf()` and `vfctprintf()`, which take an output function as their first parameter, instead of a buffer - enabling a stream-like output, generalizing the mechanism of `fprintf()`:
```C

// define the output function
void my_stream_output(char character, void* arg)
{
  // opt. evaluate the argument and send the char somewhere
}

{
  // in your code
  void* arg = (void*)100;  // this argument is passed to the output function
  fctprintf(&my_stream_output, arg, "This is a test: %X", 0xAA);
  fctprintf(&my_stream_output, nullptr, "Send to null dev");
}
```

## Format Specifiers

A format specifier follows this prototype: `%[flags][width][.precision][length]type`
The following format specifiers are supported:


### Supported Types

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
| %      | A % followed by another % character will write a single % |

Note: the `%a` specifier for hexadecimal floating-point notation (introduced in C99 and C++11) is _not_ currently supported.

### Supported Flags

| Flags | Description |
|-------|-------------|
| -     | Left-justify within the given field width; Right justification is the default. |
| +     | Forces to precede the result with a plus or minus sign (+ or -) even for positive numbers.<br>By default, only negative numbers are preceded with a - sign. |
| (space) | If no sign is going to be written, a blank space is inserted before the value. |
| #     | Used with o, b, x or X specifiers the value is preceded with 0, 0b, 0x or 0X respectively for values different than zero.<br>Used with f, F it forces the written output to contain a decimal point even if no more digits follow. By default, if no digits follow, no decimal point is written. |
| 0     | Left-pads the number with zeros (0) instead of spaces when padding is specified (see width sub-specifier). |


### Supported Width Specifiers

| Width    | Description |
|----------|-------------|
| (number) | Minimum number of characters to be printed. If the value to be printed is shorter than this number, the result is padded with blank spaces. The value is not truncated even if the result is larger. |
| *        | The width is not specified in the format string, but as an additional integer value argument preceding the argument that has to be formatted. |


### Supported Precision Specifiers

| Precision	| Description |
|-----------|-------------|
| .number   | For integer specifiers (d, i, o, u, x, X): precision specifies the minimum number of digits to be written. If the value to be written is shorter than this number, the result is padded with leading zeros. The value is not truncated even if the result is longer. A precision of 0 means that no character is written for the value 0.<br>For f and F specifiers: this is the number of digits to be printed after the decimal point. **By default, this is 6, maximum is 9**.<br>For s: this is the maximum number of characters to be printed. By default all characters are printed until the ending null character is encountered.<br>If the period is specified without an explicit value for precision, 0 is assumed. |
| .*        | The precision is not specified in the format string, but as an additional integer value argument preceding the argument that has to be formatted. |


### Supported Length modifiers

The length sub-specifier modifies the length of the data type.

| Length | d i           | u o x X                | Support controlled by preprocessor variable... |
|--------|---------------|------------------------|------------------------------------------------|
| (none) | int           | unsigned int           | |
| hh     | signed char   | unsigned char          | |
| h      | short int     | unsigned short int     | |
| l      | long int      | unsigned long int      | |
| ll     | long long int | unsigned long long int | PRINTF_SUPPORT_LONG_LONG |
| j      | intmax_t      | uintmax_t              | |
| z      | size_t        | size_t                 | |
| t      | ptrdiff_t     | ptrdiff_t              | PRINTF_SUPPORT_PTRDIFF_T |

Note: the `L` modifier, for `long double`, is not currently supported.


### Return Value

Upon successful return, all functions return the number of characters written, _excluding_ the terminating NUL character used to end the string.
Functions `snprintf()` and `vsnprintf()` don't write more than `count` bytes, _including_ the terminating NUL character ('\0').
Anyway, if the output was truncated due to this limit, the return value is the number of characters that _could_ have been written.
Notice that a value equal or larger than `count` indicates a truncation. Only when the returned value is non-negative and less than `count`,
the string has been completely written.
If any error is encountered, `-1` is returned.

If `buffer` is set to `NULL` (`nullptr`) nothing is written, but the formatted length is returned.
```C
int length = sprintf(NULL, "Hello, world"); // length is set to 12
```

## Compiler Switches/Defines

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


## Test Suite
For testing just compile, build and run the test suite located in `test/test_suite.cpp`. This uses the [catch](https://github.com/catchorg/Catch2) framework for unit-tests, which generates a `main()` function alongside the various testcases.


## Projects Using `printf`
- [turnkeyboard](https://github.com/mpaland/turnkeyboard) uses printf as log and generic tty (formatting) output.
- printf is part of [embeddedartistry/libc](https://github.com/embeddedartistry/libc), a libc targeted for embedded systems usage.
- The [Hatchling Platform]( https://github.com/adrian3git/HatchlingPlatform) uses printf.

(Just send [eyalroz](https://github.com/eyalroz) a mail, or open an issue/PR to get *your* project listed as well.)


## Contributing

The following assumes Marco Paland's original repository remains mostly-inactive in terms of commits (which it has been, as of the time of writing, for 2 years).

0. Give this project a :star:
1. Create an issue and describe your idea
2. [Fork it](https://github.com/eyalroz/printf/fork)
3. Create your feature branch (`git checkout -b my-new-feature`).
4. Implement your features; don't forget to make sure all existing tests still pass.
5. Add new checks or test-cases to the test suite - both for any problems you have identified and for any new functionality you have introduced.
4. Commit your changes (`git commit -am 'Add some feature'`)
5. Publish the branch (`git push origin my-new-feature`)
6. Create a new pull request against this repository.
7. I will try to attend to PRs promptly.


## License
Both Marco Paland's [original `printf`](https://github.com/mpaland/printf/) and this fork are published under the [MIT license](http://www.opensource.org/licenses/MIT).
