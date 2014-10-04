printf
======

This is a very tiny but fully loaded printf, sprintf and snprintf implementation.
Designed for usage in embedded systems, where printf is not available due to memory issues or avoidance of linking against libc.

All type and format specifiers are supported.

This is written in C++ and uses a templated itoa. But you can easily port it to plain C if you can't use C++.


Design goals:

 - No dependencies
 - LINT and compiler L4 warning free, clean code
 - MIT license
