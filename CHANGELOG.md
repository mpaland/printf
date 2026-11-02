# Changelog

## [4.2.0](https://github.com/mpaland/printf/compare/v4.1.0...v4.2.0) (2026-11-02)


### Features

* format attribute for GCC and Clang ([0d673d2](https://github.com/mpaland/printf/commit/0d673d29035bd46343848e073de9a760a4996eae)), closes [#79](https://github.com/mpaland/printf/issues/79)
* PRINTF_DISABLE_SUPPORT_PUTCHAR to build without _putchar() ([fcffaf9](https://github.com/mpaland/printf/commit/fcffaf9fcf42c346c65a2e3c744ed8e83563cd67))


### Bug Fixes

* %f above PRINTF_MAX_FLOAT without exponential support kept nothing ([5c46ea9](https://github.com/mpaland/printf/commit/5c46ea927b4922e0e4f087d1db68b976657dbadf))
* %p with '0x' prefix, lowercase hex digits and "(nil)" for NULL ([121f287](https://github.com/mpaland/printf/commit/121f2871a5b30ec17b918dc3961892ec90cb34df)), closes [#57](https://github.com/mpaland/printf/issues/57) [#84](https://github.com/mpaland/printf/issues/84)
* %s with precision read one character beyond the precision ([4cf7dba](https://github.com/mpaland/printf/commit/4cf7dba36c0a84372182da3493e4f1375a1d32ac))
* field width for %p ([7eb16e7](https://github.com/mpaland/printf/commit/7eb16e792aca0a442e3aab88699cf0f09bef2aaf))
* incomplete format specifier at the end of the format string ([db914c5](https://github.com/mpaland/printf/commit/db914c5c3ece33bc164f3fb18c4934d72e9ea37d))
* INF and NAN in uppercase for %F, %E and %G ([092e9f2](https://github.com/mpaland/printf/commit/092e9f2256b78b5bbcd579b7fd99d9dd0e2078fd))
* width of INT_MIN from an argument ([bd0bc40](https://github.com/mpaland/printf/commit/bd0bc4097256146869fc8009c0bf48ea30cc1079))
* zero padding and precision beyond the number buffer ([37f4913](https://github.com/mpaland/printf/commit/37f491365de2fa40f9affceea90eca87e9bea5d1))


### Performance

* no division by a variable base in _ntoa ([93be85a](https://github.com/mpaland/printf/commit/93be85af72dbac904eacce06bd3c6088fa939925)), closes [#117](https://github.com/mpaland/printf/issues/117)


### Build System

* remove the pre-CMake Makefile ([aff0698](https://github.com/mpaland/printf/commit/aff069855008b88034259b121c13d8620f39ec81))
* **test:** update Catch2 to v3.16.1 ([55ce53b](https://github.com/mpaland/printf/commit/55ce53b1db8941a94592bb2a21075aa66f01b5ab))


### Documentation

* **readme:** Include new test suite layout and Catch2 v3 ([61c6eb5](https://github.com/mpaland/printf/commit/61c6eb50eb3d48751cf999e1d463ebed9393491e))
* **readme:** update the check count of the test suite ([9467319](https://github.com/mpaland/printf/commit/946731941a869d0d34547a7ddebb7e9af1417666))

## [4.1.0](https://github.com/mpaland/printf/compare/v4.0.0...v4.1.0) (2026-10-31)


### Features

* support the 'L' length modifier for floating point values ([332524f](https://github.com/mpaland/printf/commit/332524f5dc2ae31d4609a86c8f4d36de1c1c227c))


### Bug Fixes

* %g according to C11, '#' flag for floating point values ([4fe92fd](https://github.com/mpaland/printf/commit/4fe92fdf3402606ce441423181d1d479668f0883)), closes [#48](https://github.com/mpaland/printf/issues/48) [#55](https://github.com/mpaland/printf/issues/55) [#75](https://github.com/mpaland/printf/issues/75) [#111](https://github.com/mpaland/printf/issues/111) [#121](https://github.com/mpaland/printf/issues/121) [#122](https://github.com/mpaland/printf/issues/122) [#124](https://github.com/mpaland/printf/issues/124)
* avoid signed integer overflow, handle negative '*' precision ([ecd5a1a](https://github.com/mpaland/printf/commit/ecd5a1abcd40a45fdb9f06313bb967157e6c6291)), closes [#118](https://github.com/mpaland/printf/issues/118)
* compiler warnings reported by GCC ([83f01fb](https://github.com/mpaland/printf/commit/83f01fb84d5041e73576585988c916ce8280a466)), closes [#102](https://github.com/mpaland/printf/issues/102)
* exact decimal exponent for %e, zero, subnormals, 32-bit double ([36f6079](https://github.com/mpaland/printf/commit/36f60793d64ca2906bddb7f5c73c9d92e76c9601)), closes [#54](https://github.com/mpaland/printf/issues/54) [#94](https://github.com/mpaland/printf/issues/94) [#98](https://github.com/mpaland/printf/issues/98) [#119](https://github.com/mpaland/printf/issues/119) [#120](https://github.com/mpaland/printf/issues/120)
* honour the precision of integers with the '-' flag ([b74de50](https://github.com/mpaland/printf/commit/b74de500f0fa2463f240d6498b62cdadf89aaad3)), closes [#49](https://github.com/mpaland/printf/issues/49) [#113](https://github.com/mpaland/printf/issues/113)
* NULL pointer for %s, %.0s string length, %c with '\0' ([fb4c361](https://github.com/mpaland/printf/commit/fb4c3613719baa5cdb0adfe60d358f18a4a335ba)), closes [#63](https://github.com/mpaland/printf/issues/63) [#47](https://github.com/mpaland/printf/issues/47)
* rounding carry at a decimal tie, whole part on 16-bit platforms ([abb4726](https://github.com/mpaland/printf/commit/abb4726f094e4519269c878145e07df029d1981b)), closes [#51](https://github.com/mpaland/printf/issues/51) [#108](https://github.com/mpaland/printf/issues/108)
* rules of the '#' flag for integers ([5bfef75](https://github.com/mpaland/printf/commit/5bfef753edf279b6372cd10d1463b739c3a83ab6)), closes [#50](https://github.com/mpaland/printf/issues/50) [#53](https://github.com/mpaland/printf/issues/53) [#99](https://github.com/mpaland/printf/issues/99) [#109](https://github.com/mpaland/printf/issues/109) [#110](https://github.com/mpaland/printf/issues/110) [#114](https://github.com/mpaland/printf/issues/114)


### Refactoring

* fctprintf() delegates to vfctprintf() ([a3de51c](https://github.com/mpaland/printf/commit/a3de51cab55d2a4ec7c594b22b7904ed3d26f4f9))


### Build System

* add CMake build with CTest integration ([38ec9dc](https://github.com/mpaland/printf/commit/38ec9dc40287f2fed4e3316bdfcf277ad93dce4f)), closes [#106](https://github.com/mpaland/printf/issues/106)
* **make:** define CPPFILT for the demangled symbol list ([d3f8d2e](https://github.com/mpaland/printf/commit/d3f8d2e25f6bdc3f2b77e9f47d83e70e4e8fdabd))


### Documentation

* **readme:** update for the fixes, the test suite and CMake ([f9f57d1](https://github.com/mpaland/printf/commit/f9f57d12dc56f623283f66ce7dc27ff849f3e50d))
