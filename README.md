# printf / sprintf for embedded systems

This is a tiny but fully loaded printf, sprintf and snprintf implementation.  
Primary designed for usage in embedded systems, where printf is not available due to memory issues or in avoidance of linking against libc.  
Using the standard libc printf may pull a lot of unwanted library stuff. In this case the following implementation can be used.  
Absolutely **NO dependencies** are required, printf.cpp brings all necessary routines, even its own fast ftoa conversion.

If memory footprint is really an issue the floating point support can be turned off via the `PRINTF_FLOAT_SUPPORT` compiler switch.

When using printf (instead of sprintf) you have to provide your own `_putchar()` low level function as console output.

## Design goals

 - NO dependencies, no libs, just one module file
 - Support of all flags, width and precision sub-specifiers
 - Reentrant and thread-safe, malloc free
 - LINT and compiler L4 warning free, clean code
 - Extensive passing test suite
 - MIT license


## Usage
This is 1:1 like the according stdio.h library version:  

`int printf(const char* format, ...);`  
`int sprintf(char* buffer, const char* format, ...);`  
`int snprintf(char* buffer, size_t count, const char* format, ...);`  


## Format specifiers

A format specifier follows this prototype: `%[flags][width][.precision][length]type`  
The following format specifiers are supported:


### Supported types

| Type   | Output |
|--------|--------|
| d or i | Signed decimal integer |
| u      | Unsigned decimal integer	|
| b      | Unsigned binary |
| o      | Unsigned octal |
| x      | Unsigned hexadecimal integer |
| X      | Unsigned hexadecimal integer (uppercase) |
| f      | Decimal floating point, lowercase |
| F      | Decimal floating point, uppercase |
| c      | Single character |
| s      | String of characters |
| p      | Pointer address |
| %      | A % followed by another % character will write a single % |


### Supported flags

| Flags | Description |
|-------|-------------|
| -     | Left-justify within the given field width; Right justification is the default. |
| +     | Forces to preceed the result with a plus or minus sign (+ or -) even for positive numbers.<br>By default, only negative numbers are preceded with a - sign. |
| (space) | If no sign is going to be written, a blank space is inserted before the value. |
| #     | Used with o, x or X specifiers the value is preceeded with 0, 0x or 0X respectively for values different than zero.<br>Used with f, F it forces the written output to contain a decimal point even if no more digits follow. By default, if no digits follow, no decimal point is written. |
| 0     | Left-pads the number with zeroes (0) instead of spaces when padding is specified (see width sub-specifier). |


### Supported precision

| Precision	| Description |
|-----------|-------------|
| .number   | For integer specifiers (d, i, o, u, x, X): precision specifies the minimum number of digits to be written. If the value to be written is shorter than this number, the result is padded with leading zeros. The value is not truncated even if the result is longer. A precision of 0 means that no character is written for the value 0.<br>For f and F specifiers: this is the number of digits to be printed after the decimal point. By default, this is 6, maximum is 9.<br>For s: this is the maximum number of characters to be printed. By default all characters are printed until the ending null character is encountered.<br>If the period is specified without an explicit value for precision, 0 is assumed. |
| .*        | The precision is not specified in the format string, but as an additional integer value argument preceding the argument that has to be formatted. |


### Supported width

| Width    | Description |
|----------|-------------|
| (number) | Minimum number of characters to be printed. If the value to be printed is shorter than this number, the result is padded with blank spaces. The value is not truncated even if the result is larger. |
| *        | The width is not specified in the format string, but as an additional integer value argument preceding the argument that has to be formatted. |


### Supported length

The length sub-specifier modifies the length of the data type.

| Length | d i  | u o x X |
|--------|------|---------|
| (none) | int  | unsigned int |
| l      | long int | unsigned long int |
| ll     | long long int | unsigned long long int |


## Compiler switches/defines

| Name | Default value | Description |
|------|---------------|-------------|
| PRINTF_BUFFER_SIZE   | 128 | The buffer size used for printf |
| NTOA_BUFFER_SIZE     | 32  | ntoa (integer) conversion buffer size. This must be big enough to hold one converted numeric number, normally 32 is a sufficient value. |
| FTOA_BUFFER_SIZE     | 32  | ftoa (float) conversion buffer size. This must be big enough to hold one converted float number, normally 32 is a sufficient value. |
| PRINTF_FLOAT_SUPPORT | defined | Define this to enable floating point (%f) support |


## Test suite
For testing just compile, build and run the test suite located in `test/test_suite.cpp`. This uses the [catch](https://github.com/philsquared/Catch) framework for unit-tests, which is auto-adding main().


## Contributing

1. Create an issue and describe your idea
2. [Fork it](https://github.com/mpaland/printf/fork)
3. Create your feature branch (`git checkout -b my-new-feature`)
4. Commit your changes (`git commit -am 'Add some feature'`)
5. Publish the branch (`git push origin my-new-feature`)
6. Create a new pull request
7. Profit! :white_check_mark:


## License
printf is written under the [MIT license](http://www.opensource.org/licenses/MIT).
