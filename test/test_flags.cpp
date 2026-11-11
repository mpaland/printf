// printf unit tests: the flags ' ', '+', '0', '-' and '#'

#include "test_common.hpp"


TEST_CASE("space flag", "[flags]")
{
  CHECK_SPRINTF(" 42",                               "% d",         42);
  CHECK_SPRINTF("-42",                               "% d",         -42);
  CHECK_SPRINTF("   42",                             "% 5d",        42);
  CHECK_SPRINTF("  -42",                             "% 5d",        -42);
  CHECK_SPRINTF("             42",                   "% 15d",       42);
  CHECK_SPRINTF("            -42",                   "% 15d",       -42);
  CHECK_SPRINTF(" 1024",                             "% d",         1024);
  CHECK_SPRINTF("-1024",                             "% d",         -1024);
  CHECK_SPRINTF(" 1024",                             "% i",         1024);
  CHECK_SPRINTF("-1024",                             "% i",         -1024);
  CHECK_SPRINTF(" ",                                 "% .0d",       0);
#if TEST_FLOAT
  CHECK_SPRINTF("        -42.987",                   "% 15.3f",     -42.987);
  CHECK_SPRINTF("         42.987",                   "% 15.3f",     42.987);
#endif
}


// the space flag is ignored for the unsigned, string and char conversions
TEST_CASE("space flag with unsigned conversions", "[flags][non-standard]")
{
  DISABLE_WARNING_PUSH
  DISABLE_WARNING_NON_STANDARD_FORMAT
  CHECK_SPRINTF("Hello testing",                     "% s",         "Hello testing");
  CHECK_SPRINTF("1024",                              "% u",         1024);
  CHECK_SPRINTF("4294966272",                        "% u",         4294966272U);
  CHECK_SPRINTF("777",                               "% o",         511);
  CHECK_SPRINTF("37777777001",                       "% o",         4294966785U);
  CHECK_SPRINTF("1234abcd",                          "% x",         305441741);
  CHECK_SPRINTF("edcb5433",                          "% x",         3989525555U);
  CHECK_SPRINTF("1234ABCD",                          "% X",         305441741);
  CHECK_SPRINTF("EDCB5433",                          "% X",         3989525555U);
  CHECK_SPRINTF("x",                                 "% c",         'x');
  DISABLE_WARNING_POP
}


TEST_CASE("plus flag", "[flags]")
{
  CHECK_SPRINTF("+42",                               "%+d",         42);
  CHECK_SPRINTF("-42",                               "%+d",         -42);
  CHECK_SPRINTF("  +42",                             "%+5d",        42);
  CHECK_SPRINTF("  -42",                             "%+5d",        -42);
  CHECK_SPRINTF("            +42",                   "%+15d",       42);
  CHECK_SPRINTF("            -42",                   "%+15d",       -42);
  CHECK_SPRINTF("+1024",                             "%+d",         1024);
  CHECK_SPRINTF("-1024",                             "%+d",         -1024);
  CHECK_SPRINTF("+1024",                             "%+i",         1024);
  CHECK_SPRINTF("-1024",                             "%+i",         -1024);
  CHECK_SPRINTF("+",                                 "%+.0d",       0);
}


// the plus flag is ignored for the unsigned, string and char conversions
TEST_CASE("plus flag with unsigned conversions", "[flags][non-standard]")
{
  DISABLE_WARNING_PUSH
  DISABLE_WARNING_NON_STANDARD_FORMAT
  CHECK_SPRINTF("Hello testing",                     "%+s",         "Hello testing");
  CHECK_SPRINTF("1024",                              "%+u",         1024);
  CHECK_SPRINTF("4294966272",                        "%+u",         4294966272U);
  CHECK_SPRINTF("777",                               "%+o",         511);
  CHECK_SPRINTF("37777777001",                       "%+o",         4294966785U);
  CHECK_SPRINTF("1234abcd",                          "%+x",         305441741);
  CHECK_SPRINTF("edcb5433",                          "%+x",         3989525555U);
  CHECK_SPRINTF("1234ABCD",                          "%+X",         305441741);
  CHECK_SPRINTF("EDCB5433",                          "%+X",         3989525555U);
  CHECK_SPRINTF("x",                                 "%+c",         'x');
  DISABLE_WARNING_POP
}


TEST_CASE("zero flag", "[flags]")
{
  CHECK_SPRINTF("42",                                "%0d",         42);
  CHECK_SPRINTF("42",                                "%0ld",        42L);
  CHECK_SPRINTF("-42",                               "%0d",         -42);
  CHECK_SPRINTF("00042",                             "%05d",        42);
  CHECK_SPRINTF("-0042",                             "%05d",        -42);
  CHECK_SPRINTF("000000000000042",                   "%015d",       42);
  CHECK_SPRINTF("-00000000000042",                   "%015d",       -42);
#if TEST_FLOAT
  CHECK_SPRINTF("000000000042.12",                   "%015.2f",     42.1234);
  CHECK_SPRINTF("00000000042.988",                   "%015.3f",     42.9876);
  CHECK_SPRINTF("-00000042.98760",                   "%015.5f",     -42.9876);
#endif
}


TEST_CASE("padding of negative numbers", "[flags]")
{
  // space padding
  CHECK_SPRINTF("-5",                                "% 1d",        -5);
  CHECK_SPRINTF("-5",                                "% 2d",        -5);
  CHECK_SPRINTF(" -5",                               "% 3d",        -5);
  CHECK_SPRINTF("  -5",                              "% 4d",        -5);

  // zero padding
  CHECK_SPRINTF("-5",                                "%01d",        -5);
  CHECK_SPRINTF("-5",                                "%02d",        -5);
  CHECK_SPRINTF("-05",                               "%03d",        -5);
  CHECK_SPRINTF("-005",                              "%04d",        -5);
}


#if TEST_FLOAT
TEST_CASE("padding of negative floating point numbers", "[flags]")
{
  // space padding
  CHECK_SPRINTF("-5.0",                              "% 3.1f",      -5.);
  CHECK_SPRINTF("-5.0",                              "% 4.1f",      -5.);
  CHECK_SPRINTF(" -5.0",                             "% 5.1f",      -5.);
#if TEST_EXPONENTIAL
  CHECK_SPRINTF("    -5",                            "% 6.1g",      -5.);
  CHECK_SPRINTF("-5.0e+00",                          "% 6.1e",      -5.);
  CHECK_SPRINTF("  -5.0e+00",                        "% 10.1e",     -5.);
#endif

  // zero padding
  CHECK_SPRINTF("-5.0",                              "%03.1f",      -5.);
  CHECK_SPRINTF("-5.0",                              "%04.1f",      -5.);
  CHECK_SPRINTF("-05.0",                             "%05.1f",      -5.);
#if TEST_EXPONENTIAL
  CHECK_SPRINTF("-005.0e+00",                        "%010.1e",     -5.);
  CHECK_SPRINTF("-05E+00",                           "%07.0E",      -5.);
  CHECK_SPRINTF("-05",                               "%03.0g",      -5.);
#endif

  // zero padding without decimal point
  CHECK_SPRINTF("-5",                                "%01.0f",      -5.);
  CHECK_SPRINTF("-5",                                "%02.0f",      -5.);
  CHECK_SPRINTF("-05",                               "%03.0f",      -5.);
}
#endif


TEST_CASE("minus flag", "[flags]")
{
  CHECK_SPRINTF("42",                                "%-d",         42);
  CHECK_SPRINTF("-42",                               "%-d",         -42);
  CHECK_SPRINTF("42   ",                             "%-5d",        42);
  CHECK_SPRINTF("-42  ",                             "%-5d",        -42);
  CHECK_SPRINTF("42             ",                   "%-15d",       42);
  CHECK_SPRINTF("-42            ",                   "%-15d",       -42);
}


// the '0' flag is ignored with the '-' flag
TEST_CASE("minus flag with zero flag", "[flags][non-standard]")
{
  DISABLE_WARNING_PUSH
  DISABLE_WARNING_NON_STANDARD_FORMAT
  CHECK_SPRINTF("42",                                "%-0d",        42);
  CHECK_SPRINTF("-42",                               "%-0d",        -42);
  CHECK_SPRINTF("42   ",                             "%-05d",       42);
  CHECK_SPRINTF("-42  ",                             "%-05d",       -42);
  CHECK_SPRINTF("42             ",                   "%-015d",      42);
  CHECK_SPRINTF("-42            ",                   "%-015d",      -42);
  CHECK_SPRINTF("42",                                "%0-d",        42);
  CHECK_SPRINTF("-42",                               "%0-d",        -42);
  CHECK_SPRINTF("42   ",                             "%0-5d",       42);
  CHECK_SPRINTF("-42  ",                             "%0-5d",       -42);
  CHECK_SPRINTF("42             ",                   "%0-15d",      42);
  CHECK_SPRINTF("-42            ",                   "%0-15d",      -42);
#if TEST_EXPONENTIAL
  // #48, #55, #75: %g removes trailing zeros
  CHECK_SPRINTF("-4.200e+01     ",                   "%0-15.3e",    -42.);
  CHECK_SPRINTF("-42            ",                   "%0-15.3g",    -42.);
#else
  CHECK_SPRINTF("e",                                 "%0-15.3e",    -42.);
  CHECK_SPRINTF("g",                                 "%0-15.3g",    -42.);
#endif
  DISABLE_WARNING_POP
}


TEST_CASE("sharp flag", "[flags]")
{
  // #53, #109, #110, #114: the octal prefix is a leading zero, which is not added if the precision provides it
  CHECK_SPRINTF("0",                                 "%#o",         0);
  CHECK_SPRINTF("0",                                 "%#0o",        0);
  CHECK_SPRINTF("0",                                 "%#.0o",       0);
  CHECK_SPRINTF("0",                                 "%#.1o",       0);
  CHECK_SPRINTF("   0",                              "%#4o",        0);
  CHECK_SPRINTF("0000",                              "%#.4o",       0);
  CHECK_SPRINTF("01",                                "%#o",         1);
  CHECK_SPRINTF("01",                                "%#0o",        1);
  CHECK_SPRINTF("01",                                "%#.0o",       1);
  CHECK_SPRINTF("01",                                "%#.1o",       1);
  CHECK_SPRINTF("  01",                              "%#4o",        1);
  CHECK_SPRINTF("0001",                              "%#.4o",       1);
  CHECK_SPRINTF("077",                               "%#03o",       077);
  CHECK_SPRINTF("0077",                              "%#04o",       077);
  CHECK_SPRINTF("01001",                             "%#04o",       01001);
  CHECK_SPRINTF("06143",                             "%#.5o",       06143);
  CHECK_SPRINTF("006143",                            "%#.6o",       06143);

  // #50, #99: the hex prefix is omitted for zero and does not replace digits
  CHECK_SPRINTF("",                                  "%#.0x",       0);
  CHECK_SPRINTF("0",                                 "%#.1x",       0);
  CHECK_SPRINTF("0x0000614e",                        "%#.8x",       0x614e);
  CHECK_SPRINTF("0x1234",                            "%#4x",        0x1234);
  CHECK_SPRINTF("0x1001",                            "%#04x",       0x1001);
  CHECK_SPRINTF("0x1001",                            "%#05x",       0x1001);
  CHECK_SPRINTF("0x01001",                           "%#07x",       0x1001);
}


TEST_CASE("sharp flag with binary", "[flags][non-standard]")
{
  DISABLE_WARNING_PUSH
  DISABLE_WARNING_NON_STANDARD_FORMAT
  CHECK_SPRINTF("0b110",                             "%#b",         6);
  CHECK_SPRINTF("0b00000101",                        "%#010b",      5);
  CHECK_SPRINTF("0b11111111",                        "%#010b",      0xff);
  CHECK_SPRINTF("0b011111111",                       "%#011b",      0xff);
  DISABLE_WARNING_POP
}


#if TEST_LONG_LONG
TEST_CASE("sharp flag with long long", "[flags]")
{
  CHECK_SPRINTF("0",                                 "%#llo",       (long long)0);
  CHECK_SPRINTF("0",                                 "%#0llo",      (long long)0);
  CHECK_SPRINTF("0",                                 "%#.0llo",     (long long)0);
  CHECK_SPRINTF("0",                                 "%#.1llo",     (long long)0);
  CHECK_SPRINTF("   0",                              "%#4llo",      (long long)0);
  CHECK_SPRINTF("0000",                              "%#.4llo",     (long long)0);
  CHECK_SPRINTF("01",                                "%#llo",       (long long)1);
  CHECK_SPRINTF("01",                                "%#0llo",      (long long)1);
  CHECK_SPRINTF("01",                                "%#.0llo",     (long long)1);
  CHECK_SPRINTF("01",                                "%#.1llo",     (long long)1);
  CHECK_SPRINTF("  01",                              "%#4llo",      (long long)1);
  CHECK_SPRINTF("0001",                              "%#.4llo",     (long long)1);
  CHECK_SPRINTF("01001",                             "%#04llo",     (long long)01001);
  CHECK_SPRINTF("",                                  "%#.0llx",     (long long)0);
  CHECK_SPRINTF("0x0000614e",                        "%#.8llx",     (long long)0x614e);
  CHECK_SPRINTF("0x1001",                            "%#04llx",     (long long)0x1001);
}


TEST_CASE("sharp flag with long long binary", "[flags][non-standard]")
{
  DISABLE_WARNING_PUSH
  DISABLE_WARNING_NON_STANDARD_FORMAT
  CHECK_SPRINTF("0b110",                             "%#llb",       (long long)6);
  DISABLE_WARNING_POP
}
#endif


TEST_CASE("sharp flag with width", "[flags]")
{
  CHECK_SPRINTF("                0777",              "%#20o",       511);
  CHECK_SPRINTF("        037777777001",              "%#20o",       4294966785U);
  CHECK_SPRINTF("          0x1234abcd",              "%#20x",       305441741);
  CHECK_SPRINTF("          0xedcb5433",              "%#20x",       3989525555U);
  CHECK_SPRINTF("          0X1234ABCD",              "%#20X",       305441741);
  CHECK_SPRINTF("          0XEDCB5433",              "%#20X",       3989525555U);

  // the zero padding goes between the prefix and the digits
  CHECK_SPRINTF("00000000000000000777",              "%#020o",      511);
  CHECK_SPRINTF("00000000037777777001",              "%#020o",      4294966785U);
  CHECK_SPRINTF("0x00000000001234abcd",              "%#020x",      305441741);
  CHECK_SPRINTF("0x0000000000edcb5433",              "%#020x",      3989525555U);
  CHECK_SPRINTF("0X00000000001234ABCD",              "%#020X",      305441741);
  CHECK_SPRINTF("0X0000000000EDCB5433",              "%#020X",      3989525555U);
}


// the sharp flag is ignored for the decimal conversions
TEST_CASE("sharp flag with decimal conversions", "[flags][non-standard]")
{
  DISABLE_WARNING_PUSH
  DISABLE_WARNING_NON_STANDARD_FORMAT
  CHECK_SPRINTF("                1024",              "%#20d",       1024);
  CHECK_SPRINTF("               -1024",              "%#20d",       -1024);
  CHECK_SPRINTF("                1024",              "%#20i",       1024);
  CHECK_SPRINTF("               -1024",              "%#20i",       -1024);
  CHECK_SPRINTF("                1024",              "%#20u",       1024);
  CHECK_SPRINTF("          4294966272",              "%#20u",       4294966272U);
  CHECK_SPRINTF("00000000000000001024",              "%#020d",      1024);
  CHECK_SPRINTF("-0000000000000001024",              "%#020d",      -1024);
  CHECK_SPRINTF("00000000000000001024",              "%#020i",      1024);
  CHECK_SPRINTF("-0000000000000001024",              "%#020i",      -1024);
  CHECK_SPRINTF("00000000000000001024",              "%#020u",      1024);
  CHECK_SPRINTF("00000000004294966272",              "%#020u",      4294966272U);
  DISABLE_WARNING_POP
}


TEST_CASE("sharp flag with zero", "[flags]")
{
  // no prefix for a zero value
  CHECK_SPRINTF("0",                                 "%#x",         0);
  CHECK_SPRINTF("0",                                 "%#X",         0);
  CHECK_SPRINTF("   0",                              "%#4x",        0);
  CHECK_SPRINTF("0000",                              "%#04x",       0);
}


#if TEST_FLOAT
TEST_CASE("sign flags with zero", "[flags]")
{
  CHECK_SPRINTF("+0.000000",                         "%+f",         0.0);
  CHECK_SPRINTF("-0.000000",                         "%+f",         -0.0);
  CHECK_SPRINTF(" 0.000000",                         "% f",         0.0);
  CHECK_SPRINTF("-0.000000",                         "% f",         -0.0);
  CHECK_SPRINTF("+0",                                "%+.0f",       0.0);
#if TEST_EXPONENTIAL
  CHECK_SPRINTF("+0.000000e+00",                     "%+e",         0.0);
  CHECK_SPRINTF(" 0.000000e+00",                     "% e",         0.0);
  CHECK_SPRINTF(" 0",                                "% g",         0.0);
#endif
}
#endif


// #71: the thousands grouping flag of POSIX is accepted and ignored, no
// separators are printed, like glibc does in the "C" locale
TEST_CASE("apostrophe flag", "[flags][non-standard]")
{
  DISABLE_WARNING_PUSH
  DISABLE_WARNING_NON_STANDARD_FORMAT
  CHECK_SPRINTF("1234",                              "%'d",         1234);
  CHECK_SPRINTF("-1234567",                          "%'d",         -1234567);
  CHECK_SPRINTF("1234",                              "%'i",         1234);
  CHECK_SPRINTF("4294966272",                        "%'u",         4294966272U);
  CHECK_SPRINTF("1234abcd",                          "%'x",         305441741);

  // with other flags, in any order
  CHECK_SPRINTF("+1234",                             "%'+d",        1234);
  CHECK_SPRINTF("+1234",                             "%+'d",        1234);
  CHECK_SPRINTF("0000001234",                        "%'010d",      1234);
  CHECK_SPRINTF("0000001234",                        "%0'10d",      1234);
  CHECK_SPRINTF("1234    |",                         "%-'8d|",      1234);
  CHECK_SPRINTF("    1234",                          "%''8d",       1234);

  // the argument is consumed, the following arguments are not shifted
  CHECK_SPRINTF("1234 abc",                          "%'d %s",      1234, "abc");
  CHECK_SPRINTF("1 1234 2",                          "%d %'d %d",   1, 1234, 2);
#if TEST_LONG_LONG
  CHECK_SPRINTF("1234567890123 abc",                 "%'lld %s",    1234567890123LL, "abc");
#endif
#if TEST_FLOAT
  CHECK_SPRINTF("1234.50",                           "%'.2f",       1234.5);
  CHECK_SPRINTF("1234.50 abc",                       "%'.2f %s",    1234.5, "abc");
#endif
  DISABLE_WARNING_POP
}
