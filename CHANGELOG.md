# Changelog

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
