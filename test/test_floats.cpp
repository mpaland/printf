// printf unit tests: the floating point conversions %f %F %e %E %g %G

#include "test_common.hpp"

#include <cstdio>
#include <iomanip>
#include <iostream>
#include <sstream>


#if TEST_FLOAT

TEST_CASE("decimal conversion", "[floats]")
{
  CHECK_SPRINTF("3.1415",                            "%.4f",        3.1415354);
  CHECK_SPRINTF("30343.142",                         "%.3f",        30343.1415354);
  CHECK_SPRINTF("34",                                "%.0f",        34.1415354);
  CHECK_SPRINTF("1",                                 "%.0f",        1.3);
  CHECK_SPRINTF("2",                                 "%.0f",        1.55);
  CHECK_SPRINTF("1.6",                               "%.1f",        1.64);
  CHECK_SPRINTF("42.90",                             "%.2f",        42.8952);
  CHECK_SPRINTF("42.895200000",                      "%.9f",        42.8952);
  CHECK_SPRINTF("42.8952230000",                     "%.10f",       42.895223);
  CHECK_SPRINTF(" 42.90",                            "%6.2f",       42.8952);
  CHECK_SPRINTF("+42.90",                            "%+6.2f",      42.8952);
  CHECK_SPRINTF("+42.9",                             "%+5.1f",      42.9252);
  CHECK_SPRINTF("42.500000",                         "%f",          42.5);
  CHECK_SPRINTF("42.5",                              "%.1f",        42.5);
  CHECK_SPRINTF("42167.000000",                      "%f",          42167.0);
  CHECK_SPRINTF("-12345.987654321",                  "%.9f",        -12345.987654321);
  CHECK_SPRINTF("4.0",                               "%.1f",        3.999);
  CHECK_SPRINTF("4",                                 "%.0f",        4.5);
  CHECK_SPRINTF("3",                                 "%.0f",        3.49);
  CHECK_SPRINTF("3.5",                               "%.1f",        3.49);
  CHECK_SPRINTF("a0.5  ",                            "a%-5.1f",     0.5);
  CHECK_SPRINTF("a0.5  end",                         "a%-5.1fend",  0.5);
  CHECK_SPRINTF("0.33",                              "%.*f",        2, 0.33333333);
  CHECK_SPRINTF("-0.000000",                         "%f",          -0.0);

  // #108: the whole part is larger than a 16-bit int
  CHECK_SPRINTF("40000.000000",                      "%f",          40000.0);
  CHECK_SPRINTF("999999999.5",                       "%.1f",        999999999.5);

  // at most 9 digits after the decimal point are computed, further digits are zero
  CHECK_SPRINTF("42.895223123000",                   "%.12f",       42.89522312345678);
  CHECK_SPRINTF("42.895223877000",                   "%.12f",       42.89522387654321);
  CHECK_SPRINTF("42477.371093750000000",             "%020.15f",    42477.37109375);
  CHECK_SPRINTF("-67224.546875000000000000",         "%.18f",       -67224.546875);

  // %F, %lf and %Lf are printed like %f
  CHECK_SPRINTF("42.895223123000",                   "%.12F",       42.89522312345678);
  CHECK_SPRINTF("42.895223123000",                   "%.12lf",      42.89522312345678);
  CHECK_SPRINTF("42.895223123000",                   "%.12Lf",      42.89522312345678l);
  CHECK_SPRINTF("42.895223123000",                   "%.12Lf",      42.89522312345678L);
}


// #51: the rounding is not a correctly rounded conversion like the one of glibc:
// the scaled value is rounded half to even, so 0.95 (which is slightly below
// 0.95 in binary) gives "1.0" instead of "0.9", and 0.995 gives "1.00"
// instead of "0.99"
TEST_CASE("decimal rounding", "[floats]")
{
  CHECK_SPRINTF("-1.0",                              "%.1f",        -.95);
  CHECK_SPRINTF("2.0",                               "%.1f",        1.95);
  CHECK_SPRINTF("1.00",                              "%.2f",        0.995);
  CHECK_SPRINTF("0.2",                               "%.1f",        0.25);
  CHECK_SPRINTF("0.8",                               "%.1f",        0.75);
  CHECK_SPRINTF("2",                                 "%.0f",        2.5);
  CHECK_SPRINTF("4",                                 "%.0f",        3.5);
}


// values up to PRINTF_MAX_FLOAT (1e9) are printed in decimal notation,
// larger values fall back to the exponential notation
TEST_CASE("fallback from decimal to exponential", "[floats]")
{
  CHECK_SPRINTF("1000",                              "%.0f",        1e3);
  CHECK_SPRINTF("1000000",                           "%.0f",        1e6);
  CHECK_SPRINTF("1000000000",                        "%.0f",        1e9);
#if TEST_EXPONENTIAL
  CHECK_SPRINTF("1e+12",                             "%.0f",        1e12);
  CHECK_SPRINTF("1e+15",                             "%.0f",        1e15);
  CHECK_SPRINTF("1.0e+20",                           "%.1f",        1e20);
  CHECK_SPRINTF("2.1474836470e+09",       "%.10f",      2147483647.0);   // 2^31 - 1
  CHECK_SPRINTF("2.1474836480e+09",       "%.10f",      2147483648.0);   // 2^31
  CHECK_SPRINTF("4.2949672950e+09",       "%.10f",      4294967295.0);   // 2^32 - 1
  CHECK_SPRINTF("4.2949672960e+09",       "%.10f",      4294967296.0);   // 2^32
  // like %f, the decimal notation of %g is limited to PRINTF_MAX_FLOAT
  CHECK_SPRINTF("2.147483647e+09",        "%.10g",      2147483647.0);   // 2^31 - 1
  CHECK_SPRINTF("2.147483648e+09",        "%.10g",      2147483648.0);   // 2^31
  CHECK_SPRINTF("4.294967295e+09",        "%.10g",      4294967295.0);   // 2^32 - 1
  CHECK_SPRINTF("4.294967296e+09",        "%.10g",      4294967296.0);   // 2^32
#else
  // without exponential notation "ovf" (overflow) is printed like the special values inf and nan
  CHECK_SPRINTF("ovf",                               "%.0f",        1e12);
  CHECK_SPRINTF("ovf",                               "%.0f",        1e15);
  CHECK_SPRINTF("ovf",                               "%.1f",        1e20);
  CHECK_SPRINTF("-ovf",                              "%f",          -1e20);
  CHECK_SPRINTF("+ovf",                              "%+f",         1e20);
  CHECK_SPRINTF(" ovf",                              "% f",         1e20);
  CHECK_SPRINTF("     ovf",                          "%8.1f",       1e20);
  CHECK_SPRINTF("     ovf",                          "%08.1f",      1e20);
  CHECK_SPRINTF("ovf     ",                          "%-8f",        1e20);
  CHECK_SPRINTF("OVF",                               "%F",          1e20);
  CHECK_SPRINTF("-OVF",                              "%F",          -1e20);
  CHECK_SPRINTF("ovf",                               "%f",          1000000001.0);
  CHECK_SPRINTF("abc ovf def",                       "abc %.0f def", 1e12);
  CHECK_SPRINTF("1 -ovf 2",                          "%d %f %d",    1, -1e20, 2);
#endif
}


TEST_CASE("infinity and not a number", "[floats]")
{
  CHECK_SPRINTF("     nan",                          "%8f",         (double)NAN);
  CHECK_SPRINTF("     inf",                          "%8f",         (double)INFINITY);
  CHECK_SPRINTF("-inf    ",                          "%-8f",        (double)-INFINITY);
  CHECK_SPRINTF("    -nan",                          "%8f",         -(double)NAN);
  CHECK_SPRINTF("+inf",                              "%+f",         (double)INFINITY);
  CHECK_SPRINTF(" inf",                              "% f",         (double)INFINITY);
  CHECK_SPRINTF("-inf",                              "%+f",         (double)-INFINITY);
  CHECK_SPRINTF("inf",                               "%.3f",        (double)INFINITY);
  // the '0' flag is ignored
  CHECK_SPRINTF("     inf",                          "%08f",        (double)INFINITY);
  CHECK_SPRINTF("    -inf",                          "%08f",        (double)-INFINITY);
  CHECK_SPRINTF("     nan",                          "%08f",        (double)NAN);
  // uppercase for %F
  CHECK_SPRINTF("INF",                               "%F",          (double)INFINITY);
  CHECK_SPRINTF("-INF",                              "%F",          (double)-INFINITY);
  CHECK_SPRINTF("NAN",                               "%F",          (double)NAN);
  CHECK_SPRINTF("INF     ",                          "%-8F",        (double)INFINITY);
  CHECK_SPRINTF("    +INF",                          "%+8F",        (double)INFINITY);
#if TEST_EXPONENTIAL
  CHECK_SPRINTF("     nan",                          "%8e",         (double)NAN);
  CHECK_SPRINTF("     inf",                          "%8e",         (double)INFINITY);
  CHECK_SPRINTF("-inf    ",                          "%-8e",        (double)-INFINITY);
  CHECK_SPRINTF("    +inf",                          "%+8e",        (double)INFINITY);
  CHECK_SPRINTF("     inf",                          "%08e",        (double)INFINITY);
  CHECK_SPRINTF("     nan",                          "%8g",         (double)NAN);
  CHECK_SPRINTF("    -inf",                          "%8g",         (double)-INFINITY);
  // uppercase for %E and %G
  CHECK_SPRINTF("NAN",                               "%E",          (double)NAN);
  CHECK_SPRINTF("-NAN",                              "%+G",         -(double)NAN);
  CHECK_SPRINTF("INF",                               "%E",          (double)INFINITY);
  CHECK_SPRINTF("    -INF",                          "%8G",         (double)-INFINITY);
  CHECK_SPRINTF(" INF",                              "% G",         (double)INFINITY);
#endif
}

#endif  // TEST_FLOAT


#if TEST_EXPONENTIAL

TEST_CASE("exponential conversion", "[floats]")
{
  CHECK_SPRINTF("1.000e+01",                         "%.3e",        9.9996);
  CHECK_SPRINTF("4.895512e+04",                      "%e",          48955.125);
  CHECK_SPRINTF("9.2524e+04",                        "%.4e",        92523.5);
  CHECK_SPRINTF("-8.380923438e+04",                  "%.9e",        -83809.234375);
  CHECK_SPRINTF("3.33e-01",                          "%.*e",        2, 0.33333333);
  CHECK_SPRINTF("+1.230E+308",                       "%+.3E",       1.23e+308);

  // #98, #120: the mantissa is rounded up to 10
  CHECK_SPRINTF("1.000000e+01",                      "%1.6e",       9.9999999);
  CHECK_SPRINTF("1.0e+01",                           "%.1e",        9.96);

  // powers of 10 were printed like "10.000000e+01"
  CHECK_SPRINTF("1.000000e+02",                      "%e",          100.0);
  CHECK_SPRINTF("1.000000e+100",                     "%e",          1e100);
  {
    bool any_failed = false;
    for (int n = -307; n <= 307; n++) {
      char expected[test_buffer_size];
      char buffer[test_buffer_size];
      std::snprintf(expected, sizeof(expected), "1.000000e%+03d", n);
      sprintf_(buffer, "%e", std::pow(10.0, n));
      if (std::strcmp(buffer, expected) != 0) {
        std::cerr << "sprintf_(\"%e\", 1e" << n << ") = " << buffer << ", expected " << expected << "\n";
        any_failed = true;
      }
    }
    CHECK(!any_failed);
  }

  // #54: zero
  CHECK_SPRINTF("0.000000e+00",                      "%e",          0.0);
  CHECK_SPRINTF("-0.000000e+00",                     "%e",          -0.0);

  // #119: subnormal values
  CHECK_SPRINTF("1.000000e-310",                     "%e",          1e-310);
  CHECK_SPRINTF("4.940656e-324",                     "%e",          4.9406564584124654e-324);

  // this value is barely normal, with a mantissa of 1.1 it would be subnormal
  // at most 9 digits are computed, further digits are zero
  CHECK_SPRINTF("1.2345678900e-308",                 "%.10e",       1.2345678901e-308);

  // %E, %le and %Le are printed like %e
  CHECK_SPRINTF("-8.380923438E+04",                  "%.9E",        -83809.234375);
  CHECK_SPRINTF("-8.380923438e+04",                  "%.9le",       -83809.234375);
  CHECK_SPRINTF("-8.380923438e+04",                  "%.9Le",       -83809.234375l);
  CHECK_SPRINTF("-8.380923438e+04",                  "%.9Le",       -83809.234375L);
}


TEST_CASE("exponential conversion of tiny values", "[floats]")
{
  // the Boltzmann constant
  CHECK_SPRINTF("1e-23",                             "%.0e",        1.380651569e-23);
  CHECK_SPRINTF("1.4e-23",                           "%.1e",        1.380651569e-23);
  CHECK_SPRINTF("1.38e-23",                          "%.2e",        1.380651569e-23);
  CHECK_SPRINTF("1.381e-23",                         "%.3e",        1.380651569e-23);
  CHECK_SPRINTF("1.3807e-23",                        "%.4e",        1.380651569e-23);
  CHECK_SPRINTF("1.38065e-23",                       "%.5e",        1.380651569e-23);
  CHECK_SPRINTF("1.380652e-23",                      "%.6e",        1.380651569e-23);
  CHECK_SPRINTF("1.3806516e-23",                     "%.7e",        1.380651569e-23);
  CHECK_SPRINTF("1.38065157e-23",                    "%.8e",        1.380651569e-23);
  CHECK_SPRINTF("1.380651569e-23",                   "%.9e",        1.380651569e-23);
  CHECK_SPRINTF("1.3806515690e-23",                  "%.10e",       1.380651569e-23);
  CHECK_SPRINTF("1.38065156900e-23",                 "%.11e",       1.380651569e-23);
  // these go beyond the 9 computed digits, enable them and they will likely fail
  //CHECK_SPRINTF("1.380651569000e-23",    "%.12e",      1.380651569e-23);
  //CHECK_SPRINTF("1.3806515690000e-23",   "%.13e",      1.380651569e-23);
  //CHECK_SPRINTF("1.38065156900000e-23",  "%.14e",      1.380651569e-23);
  //CHECK_SPRINTF("1.380651569000000e-23", "%.15e",      1.380651569e-23);
  //CHECK_SPRINTF("1.3806515690000000e-23", "%.16e",     1.380651569e-23);
}


// %g uses the decimal notation if the exponent is in the range -4 to the
// precision, and removes trailing zeros
TEST_CASE("general conversion", "[floats]")
{
  CHECK_SPRINTF("0.5",                               "%.4g",        0.5);
  CHECK_SPRINTF("1",                                 "%.4g",        1.0);
  CHECK_SPRINTF("12345.7",                           "%G",          12345.678);
  CHECK_SPRINTF("12345.68",                          "%.7G",        12345.678);
  CHECK_SPRINTF("1.2346E+08",                        "%.5G",        123456789.);
  CHECK_SPRINTF("12345",                             "%.6G",        12345.);
  CHECK_SPRINTF("  +1.235e+08",                      "%+12.4g",     123456789.);
  CHECK_SPRINTF("0.0012",                            "%.2G",        0.001234);
  CHECK_SPRINTF(" +0.001234",                        "%+10.4G",     0.001234);
  CHECK_SPRINTF("+001.234e-05",                      "%+012.4g",    0.00001234);
  CHECK_SPRINTF("-1.23e-308",                        "%.3g",        -1.2345e-308);
  CHECK_SPRINTF("0.33",                              "%.*g",        2, 0.33333333);

  // #48, #55, #75: trailing zeros are removed
  CHECK_SPRINTF("0.0001",                            "%g",          0.0001);
  CHECK_SPRINTF("1e-05",                             "%g",          0.00001);
  CHECK_SPRINTF("4",                                 "%.4g",        4.0);
  CHECK_SPRINTF("100",                               "%g",          100.0);

  // #121, #122, #124: the notation is chosen by the precision and the rounded exponent
  CHECK_SPRINTF("0",                                 "%g",          0.0);
  CHECK_SPRINTF("-0",                                "%g",          -0.0);
  CHECK_SPRINTF("+0",                                "%+g",         0.0);
  CHECK_SPRINTF("-0",                                "%+g",         -0.0);
  CHECK_SPRINTF("-4e+04",                            "%.1g",        -40661.5);
  CHECK_SPRINTF("100",                               "%.3g",        99.99);
  CHECK_SPRINTF("1e+06",                             "%g",          999999.5);

  // #220 (eyalroz/printf): a high power of 10 caused undefined behavior
  CHECK_SPRINTF("0.0014664",                         "%g",          0.0014663999499999749);
}


// #111: the '#' flag keeps the decimal point and the trailing zeros
TEST_CASE("sharp flag with floating point values", "[floats]")
{
  CHECK_SPRINTF("1.",                                "%#.0f",       1.0);
  CHECK_SPRINTF("1.e+00",                            "%#.0e",       1.0);
  CHECK_SPRINTF("1.00000",                           "%#g",         1.0);
  CHECK_SPRINTF("100.",                              "%#.3g",       99.99);
  CHECK_SPRINTF("-4.e+04",                           "%#.1g",       -40661.5);
  CHECK_SPRINTF(" 1.e+01",                           "%# 01.1g",    9.8);

  // %G, %lg and %Lg are printed like %g
  CHECK_SPRINTF("100.",                              "%#.3g",       99.998580932617187500);
  CHECK_SPRINTF("100.",                              "%#.3G",       99.998580932617187500);
  CHECK_SPRINTF("100.",                              "%#.3lg",      99.998580932617187500);
  CHECK_SPRINTF("100.",                              "%#.3Lg",      99.998580932617187500l);
  CHECK_SPRINTF("100.",                              "%#.3Lg",      99.998580932617187500L);
}


TEST_CASE("decimal conversion at PRINTF_MAX_FLOAT", "[floats]")
{
  CHECK_SPRINTF("1000000000.000000",                 "%f",          1000000000.0);
  CHECK_SPRINTF("999999999.000000",                  "%f",          999999999.0);
  CHECK_SPRINTF("-1000000000.000000",                "%f",          -1000000000.0);
  CHECK_SPRINTF("1.000000e+09",                      "%f",          1000000001.0);
  CHECK_SPRINTF("-1.000000e+09",                     "%f",          -1000000001.0);
}


// the expected values are the correctly rounded ones of the standard library
TEST_CASE("exponential conversion of extremal values", "[floats]")
{
  CHECK_SPRINTF("1.797693e+308",                     "%e",          DBL_MAX);
  CHECK_SPRINTF("-1.797693e+308",                    "%e",          -DBL_MAX);
  CHECK_SPRINTF("2.225074e-308",                     "%e",          DBL_MIN);
  CHECK_SPRINTF("2.220446e-16",                      "%e",          DBL_EPSILON);
  CHECK_SPRINTF("1.79769e+308",                      "%g",          DBL_MAX);
  CHECK_SPRINTF("2.22507e-308",                      "%g",          DBL_MIN);
  CHECK_SPRINTF("2.22045e-16",                       "%g",          DBL_EPSILON);
}


// the exponent X of the value decides the notation: decimal if -4 <= X < P
// (P is the precision, 6 by default), exponential otherwise
TEST_CASE("general conversion at the notation boundaries", "[floats]")
{
  CHECK_SPRINTF("0.0001",                            "%g",          0.0001);
  CHECK_SPRINTF("1e-05",                             "%g",          0.00001);
  CHECK_SPRINTF("0.1",                               "%g",          0.1);
  CHECK_SPRINTF("1",                                 "%g",          1.0);
  CHECK_SPRINTF("100000",                            "%g",          100000.0);
  CHECK_SPRINTF("123456",                            "%g",          123456.0);
  CHECK_SPRINTF("999999",                            "%g",          999999.0);
  CHECK_SPRINTF("1e+06",                             "%g",          1000000.0);
  CHECK_SPRINTF("1.23457e+06",                       "%g",          1234567.0);
  CHECK_SPRINTF("123",                               "%.3g",        123.0);
  CHECK_SPRINTF("1.23e+03",                          "%.3g",        1234.0);
  CHECK_SPRINTF("1e+100",                            "%g",          1e100);
  CHECK_SPRINTF("1E-100",                            "%G",          1e-100);
  CHECK_SPRINTF("-1.5E+300",                         "%G",          -1.5e300);
}

#endif  // TEST_EXPONENTIAL


#if TEST_FLOAT

// compares a range of values against the standard library
TEST_CASE("brute force", "[floats]")
{
  char buffer[test_buffer_size];
  bool any_failed = false;
  std::stringstream sstr;

  // decimal notation
  sstr.precision(5);
  for (float i = -100000; i < 100000; i += (float)1) {
    sprintf_(buffer, "%.5f", (double)(i / 10000));
    sstr.str("");
    sstr << std::fixed << i / 10000;
    const std::string expected = sstr.str();
    if (expected != buffer) {
      std::cerr << "sprintf_(\"%.5f\", " << std::setw(6) << i << ") = " << std::setw(10) << buffer << ", "
                << "expected " << std::setw(10) << expected << "\n";
      any_failed = true;
    }
  }
  CHECK(!any_failed);

#if TEST_EXPONENTIAL
  // exponential notation
  sstr.setf(std::ios::scientific, std::ios::floatfield);
  any_failed = false;
  for (float i = (float)-1e20; i < (float)1e20; i += (float)1e15) {
    sprintf_(buffer, "%.4f", (double)i);
    sstr.str("");
    sstr << std::setprecision(4) << i;
    const std::string expected = sstr.str();
    if (expected != buffer) {
      std::cerr << "sprintf_(\"%.4f\", " << std::setw(18) << std::setprecision(30) << i << ") = " << std::setw(15)
                << buffer << ", " << "expected " << std::setw(12) << expected << "\n";
      any_failed = true;
    }
  }
  CHECK(!any_failed);
#endif
}

#endif  // TEST_FLOAT


#if !TEST_FLOAT
// without floating point support the conversions are printed like an unknown
// conversion, their argument is consumed
TEST_CASE("decimal conversion without floating point support", "[floats][non-standard]")
{
  CHECK_SPRINTF("f",                                 "%f",          1.5);
  CHECK_SPRINTF("F",                                 "%F",          1.5);
  CHECK_SPRINTF("f 7",                               "%f %d",       1.5, 7);
  CHECK_SPRINTF("F abc",                             "%8.3F %s",    -1.5, "abc");
  CHECK_SPRINTF("f 7",                               "%Lf %d",      1.5L, 7);
  CHECK_SPRINTF("1 f 2",                             "%d %f %d",    1, 1e20, 2);
}
#endif


#if !TEST_EXPONENTIAL
// without exponential support the conversions are printed like an unknown
// conversion, their argument is consumed
TEST_CASE("exponential conversion without exponential support", "[floats][non-standard]")
{
  CHECK_SPRINTF("e",                                 "%e",          1.5);
  CHECK_SPRINTF("E 7",                               "%E %d",       1.5, 7);
  CHECK_SPRINTF("g 7",                               "%g %d",       1.5, 7);
  CHECK_SPRINTF("G abc",                             "%10.3G %s",   -1.5, "abc");
  CHECK_SPRINTF("e 7",                               "%Le %d",      1.5L, 7);
  CHECK_SPRINTF("1 e 2 g 3",                         "%d %e %d %g %d", 1, 1.5, 2, 2.5, 3);
}
#endif
