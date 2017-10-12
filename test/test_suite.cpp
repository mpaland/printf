///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2017, PALANDesign Hannover, Germany
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
// \brief printf unit tests
//
///////////////////////////////////////////////////////////////////////////////

// use the 'catch' test framework
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <string.h>

namespace test {
  // use functions in own test namespace to avoid stdio conflicts
  #include "../printf.h"
  #include "../printf.cpp"
} // namespace test


// dummy putchar
int test::_putchar(char)
{
  return 0;
}



TEST_CASE("printf", "[]" ) {
  char buffer[100] ;

  test::sprintf(buffer, "%.*f", 2, 0.33333333);
  REQUIRE(!strcmp(buffer, "0.33"));

  test::sprintf(buffer, "%.3s", "foobar");
  REQUIRE(!strcmp(buffer, "foo"));

  test::sprintf(buffer, "%10.5d", 4);
  REQUIRE(!strcmp(buffer, "     00004"));
}


TEST_CASE("space flag", "[]" ) {
  char buffer[100] ;

  test::sprintf(buffer, "% d", 42);
  REQUIRE(!strcmp(buffer, " 42"));

  test::sprintf(buffer, "% d", -42);
  REQUIRE(!strcmp(buffer, "-42"));

  test::sprintf(buffer, "% 5d", 42);
  REQUIRE(!strcmp(buffer, "   42"));

  test::sprintf(buffer, "% 5d", -42);
  REQUIRE(!strcmp(buffer, "  -42"));

  test::sprintf(buffer, "% 15d", 42);
  REQUIRE(!strcmp(buffer, "             42"));

  test::sprintf(buffer, "% 15d", -42);
  REQUIRE(!strcmp(buffer, "            -42"));

  test::sprintf(buffer, "% 15d", -42);
  REQUIRE(!strcmp(buffer, "            -42"));

  test::sprintf(buffer, "% s", "Hello testing");
  REQUIRE(!strcmp(buffer, "Hello testing"));

  test::sprintf(buffer, "% d", 1024);
  REQUIRE(!strcmp(buffer, " 1024"));

  test::sprintf(buffer, "% d", -1024);
  REQUIRE(!strcmp(buffer, "-1024"));

  test::sprintf(buffer, "% i", 1024);
  REQUIRE(!strcmp(buffer, " 1024"));

  test::sprintf(buffer, "% i", -1024);
  REQUIRE(!strcmp(buffer, "-1024"));

  test::sprintf(buffer, "% u", 1024);
  REQUIRE(!strcmp(buffer, "1024"));

  test::sprintf(buffer, "% u", 4294966272U);
  REQUIRE(!strcmp(buffer, "4294966272"));

  test::sprintf(buffer, "% o", 511);
  REQUIRE(!strcmp(buffer, "777"));

  test::sprintf(buffer, "% o", 4294966785U);
  REQUIRE(!strcmp(buffer, "37777777001"));

  test::sprintf(buffer, "% x", 305441741);
  REQUIRE(!strcmp(buffer, "1234abcd"));

  test::sprintf(buffer, "% x", 3989525555U);
  REQUIRE(!strcmp(buffer, "edcb5433"));

  test::sprintf(buffer, "% X", 305441741);
  REQUIRE(!strcmp(buffer, "1234ABCD"));

  test::sprintf(buffer, "% X", 3989525555U);
  REQUIRE(!strcmp(buffer, "EDCB5433"));

  test::sprintf(buffer, "% c", 'x');
  REQUIRE(!strcmp(buffer, "x"));
}


TEST_CASE("+ flag", "[]" ) {
  char buffer[100] ;

  test::sprintf(buffer, "%+d", 42);
  REQUIRE(!strcmp(buffer, "+42"));

  test::sprintf(buffer, "%+d", -42);
  REQUIRE(!strcmp(buffer, "-42"));

  test::sprintf(buffer, "%+5d", 42);
  REQUIRE(!strcmp(buffer, "  +42"));

  test::sprintf(buffer, "%+5d", -42);
  REQUIRE(!strcmp(buffer, "  -42"));

  test::sprintf(buffer, "%+15d", 42);
  REQUIRE(!strcmp(buffer, "            +42"));

  test::sprintf(buffer, "%+15d", -42);
  REQUIRE(!strcmp(buffer, "            -42"));

  test::sprintf(buffer, "%+s", "Hello testing");
  REQUIRE(!strcmp(buffer, "Hello testing"));

  test::sprintf(buffer, "%+d", 1024);
  REQUIRE(!strcmp(buffer, "+1024"));

  test::sprintf(buffer, "%+d", -1024);
  REQUIRE(!strcmp(buffer, "-1024"));

  test::sprintf(buffer, "%+i", 1024);
  REQUIRE(!strcmp(buffer, "+1024"));

  test::sprintf(buffer, "%+i", -1024);
  REQUIRE(!strcmp(buffer, "-1024"));

  test::sprintf(buffer, "%+u", 1024);
  REQUIRE(!strcmp(buffer, "1024"));

  test::sprintf(buffer, "%+u", 4294966272U);
  REQUIRE(!strcmp(buffer, "4294966272"));

  test::sprintf(buffer, "%+o", 511);
  REQUIRE(!strcmp(buffer, "777"));

  test::sprintf(buffer, "%+o", 4294966785U);
  REQUIRE(!strcmp(buffer, "37777777001"));

  test::sprintf(buffer, "%+x", 305441741);
  REQUIRE(!strcmp(buffer, "1234abcd"));

  test::sprintf(buffer, "%+x", 3989525555U);
  REQUIRE(!strcmp(buffer, "edcb5433"));

  test::sprintf(buffer, "%+X", 305441741);
  REQUIRE(!strcmp(buffer, "1234ABCD"));

  test::sprintf(buffer, "%+X", 3989525555U);
  REQUIRE(!strcmp(buffer, "EDCB5433"));

  test::sprintf(buffer, "%+c", 'x');
  REQUIRE(!strcmp(buffer, "x"));
}


TEST_CASE("0 flag", "[]" ) {
  char buffer[100] ;

  test::sprintf(buffer, "%0d", 42);
  REQUIRE(!strcmp(buffer, "42"));

  test::sprintf(buffer, "%0d", -42);
  REQUIRE(!strcmp(buffer, "-42"));

  test::sprintf(buffer, "%05d", 42);
  REQUIRE(!strcmp(buffer, "00042"));

  test::sprintf(buffer, "%05d", -42);
  REQUIRE(!strcmp(buffer, "-0042"));

  test::sprintf(buffer, "%015d", 42);
  REQUIRE(!strcmp(buffer, "000000000000042"));

  test::sprintf(buffer, "%015d", -42);
  REQUIRE(!strcmp(buffer, "-00000000000042"));
}


TEST_CASE("- flag", "[]" ) {
  char buffer[100];

  test::sprintf(buffer, "%-d", 42);
  REQUIRE(!strcmp(buffer, "42"));

  test::sprintf(buffer, "%-d", -42);
  REQUIRE(!strcmp(buffer, "-42"));

  test::sprintf(buffer, "%-5d", 42);
  REQUIRE(!strcmp(buffer, "42   "));

  test::sprintf(buffer, "%-5d", -42);
  REQUIRE(!strcmp(buffer, "-42  "));

  test::sprintf(buffer, "%-15d", 42);
  REQUIRE(!strcmp(buffer, "42             "));

  test::sprintf(buffer, "%-15d", -42);
  REQUIRE(!strcmp(buffer, "-42            "));

  test::sprintf(buffer, "%-0d", 42);
  REQUIRE(!strcmp(buffer, "42"));

  test::sprintf(buffer, "%-0d", -42);
  REQUIRE(!strcmp(buffer, "-42"));

  test::sprintf(buffer, "%-05d", 42);
  REQUIRE(!strcmp(buffer, "42   "));

  test::sprintf(buffer, "%-05d", -42);
  REQUIRE(!strcmp(buffer, "-42  "));

  test::sprintf(buffer, "%-015d", 42);
  REQUIRE(!strcmp(buffer, "42             "));

  test::sprintf(buffer, "%-015d", -42);
  REQUIRE(!strcmp(buffer, "-42            "));

  test::sprintf(buffer, "%0-d", 42);
  REQUIRE(!strcmp(buffer, "42"));

  test::sprintf(buffer, "%0-d", -42);
  REQUIRE(!strcmp(buffer, "-42"));

  test::sprintf(buffer, "%0-5d", 42);
  REQUIRE(!strcmp(buffer, "42   "));

  test::sprintf(buffer, "%0-5d", -42);
  REQUIRE(!strcmp(buffer, "-42  "));

  test::sprintf(buffer, "%0-15d", 42);
  REQUIRE(!strcmp(buffer, "42             "));

  test::sprintf(buffer, "%0-15d", -42);
  REQUIRE(!strcmp(buffer, "-42            "));
}


TEST_CASE("specifier", "[]" ) {
  char buffer[100];

  test::sprintf(buffer, "Hello testing");
  REQUIRE(!strcmp(buffer, "Hello testing"));

  test::sprintf(buffer, "%s", "Hello testing");
  REQUIRE(!strcmp(buffer, "Hello testing"));

  test::sprintf(buffer, "%d", 1024);
  REQUIRE(!strcmp(buffer, "1024"));

  test::sprintf(buffer, "%d", -1024);
  REQUIRE(!strcmp(buffer, "-1024"));

  test::sprintf(buffer, "%i", 1024);
  REQUIRE(!strcmp(buffer, "1024"));

  test::sprintf(buffer, "%i", -1024);
  REQUIRE(!strcmp(buffer, "-1024"));

  test::sprintf(buffer, "%u", 1024);
  REQUIRE(!strcmp(buffer, "1024"));

  test::sprintf(buffer, "%u", 4294966272U);
  REQUIRE(!strcmp(buffer, "4294966272"));

  test::sprintf(buffer, "%o", 511);
  REQUIRE(!strcmp(buffer, "777"));

  test::sprintf(buffer, "%o", 4294966785U);
  REQUIRE(!strcmp(buffer, "37777777001"));

  test::sprintf(buffer, "%x", 305441741);
  REQUIRE(!strcmp(buffer, "1234abcd"));

  test::sprintf(buffer, "%x", 3989525555U);
  REQUIRE(!strcmp(buffer, "edcb5433"));

  test::sprintf(buffer, "%X", 305441741);
  REQUIRE(!strcmp(buffer, "1234ABCD"));

  test::sprintf(buffer, "%X", 3989525555U);
  REQUIRE(!strcmp(buffer, "EDCB5433"));

  test::sprintf(buffer, "%%");
  REQUIRE(!strcmp(buffer, "%"));


}
