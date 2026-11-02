// printf unit tests: the field width and the precision

#include "test_common.hpp"


TEST_CASE("width 1", "[width]")
{
  CHECK_SPRINTF("Hello testing",                     "%1s",         "Hello testing");
  CHECK_SPRINTF("1024",                              "%1d",         1024);
  CHECK_SPRINTF("-1024",                             "%1d",         -1024);
  CHECK_SPRINTF("1024",                              "%1i",         1024);
  CHECK_SPRINTF("-1024",                             "%1i",         -1024);
  CHECK_SPRINTF("1024",                              "%1u",         1024);
  CHECK_SPRINTF("4294966272",                        "%1u",         4294966272U);
  CHECK_SPRINTF("777",                               "%1o",         511);
  CHECK_SPRINTF("37777777001",                       "%1o",         4294966785U);
  CHECK_SPRINTF("1234abcd",                          "%1x",         305441741);
  CHECK_SPRINTF("edcb5433",                          "%1x",         3989525555U);
  CHECK_SPRINTF("1234ABCD",                          "%1X",         305441741);
  CHECK_SPRINTF("EDCB5433",                          "%1X",         3989525555U);
  CHECK_SPRINTF("x",                                 "%1c",         'x');
}


TEST_CASE("width 20", "[width]")
{
  CHECK_SPRINTF("               Hello",              "%20s",        "Hello");
  CHECK_SPRINTF("                1024",              "%20d",        1024);
  CHECK_SPRINTF("               -1024",              "%20d",        -1024);
  CHECK_SPRINTF("                1024",              "%20i",        1024);
  CHECK_SPRINTF("               -1024",              "%20i",        -1024);
  CHECK_SPRINTF("                   0",              "%20i",        0);
  CHECK_SPRINTF("                1024",              "%20u",        1024);
  CHECK_SPRINTF("          4294966272",              "%20u",        4294966272U);
  CHECK_SPRINTF("                 777",              "%20o",        511);
  CHECK_SPRINTF("         37777777001",              "%20o",        4294966785U);
  CHECK_SPRINTF("            1234abcd",              "%20x",        305441741);
  CHECK_SPRINTF("            edcb5433",              "%20x",        3989525555U);
  CHECK_SPRINTF("            1234ABCD",              "%20X",        305441741);
  CHECK_SPRINTF("            EDCB5433",              "%20X",        3989525555U);
  CHECK_SPRINTF("                   0",              "%20X",        0);
  CHECK_SPRINTF("                   0",              "%20X",        0U);
#if TEST_LONG_LONG
  CHECK_SPRINTF("                   0",              "%20llX",      0ULL);
#endif
  CHECK_SPRINTF("                   x",              "%20c",        'x');
}


TEST_CASE("width from argument", "[width]")
{
  CHECK_SPRINTF("               Hello",              "%*s",         20, "Hello");
  CHECK_SPRINTF("                1024",              "%*d",         20, 1024);
  CHECK_SPRINTF("               -1024",              "%*d",         20, -1024);
  CHECK_SPRINTF("                1024",              "%*i",         20, 1024);
  CHECK_SPRINTF("               -1024",              "%*i",         20, -1024);
  CHECK_SPRINTF("                1024",              "%*u",         20, 1024);
  CHECK_SPRINTF("          4294966272",              "%*u",         20, 4294966272U);
  CHECK_SPRINTF("                 777",              "%*o",         20, 511);
  CHECK_SPRINTF("         37777777001",              "%*o",         20, 4294966785U);
  CHECK_SPRINTF("            1234abcd",              "%*x",         20, 305441741);
  CHECK_SPRINTF("            edcb5433",              "%*x",         20, 3989525555U);
  CHECK_SPRINTF("            1234ABCD",              "%*X",         20, 305441741);
  CHECK_SPRINTF("            EDCB5433",              "%*X",         20, 3989525555U);
  CHECK_SPRINTF("                   x",              "%*c",         20, 'x');

  // a negative width is taken as the '-' flag with a positive width
  CHECK_SPRINTF("hi x",                              "%*sx",        -3, "hi");
}


TEST_CASE("width with minus flag", "[width]")
{
  CHECK_SPRINTF("Hello               ",              "%-20s",       "Hello");
  CHECK_SPRINTF("1024                ",              "%-20d",       1024);
  CHECK_SPRINTF("-1024               ",              "%-20d",       -1024);
  CHECK_SPRINTF("1024                ",              "%-20i",       1024);
  CHECK_SPRINTF("-1024               ",              "%-20i",       -1024);
  CHECK_SPRINTF("1024                ",              "%-20u",       1024);
  CHECK_SPRINTF("4294966272          ",              "%-20u",       4294966272U);
  CHECK_SPRINTF("777                 ",              "%-20o",       511);
  CHECK_SPRINTF("37777777001         ",              "%-20o",       4294966785U);
  CHECK_SPRINTF("1234abcd            ",              "%-20x",       305441741);
  CHECK_SPRINTF("edcb5433            ",              "%-20x",       3989525555U);
  CHECK_SPRINTF("1234ABCD            ",              "%-20X",       305441741);
  CHECK_SPRINTF("EDCB5433            ",              "%-20X",       3989525555U);
  CHECK_SPRINTF("x                   ",              "%-20c",       'x');
#if TEST_FLOAT
  CHECK_SPRINTF("1024.1234           ",              "%-20.4f",     1024.1234);
#endif
  CHECK_SPRINTF("|    9| |9 | |    9|",              "|%5d| |%-2d| |%5d|", 9, 9, 9);
  CHECK_SPRINTF("|   10| |10| |   10|",              "|%5d| |%-2d| |%5d|", 10, 10, 10);
  CHECK_SPRINTF("|    9| |9           | |    9|",    "|%5d| |%-12d| |%5d|", 9, 9, 9);
  CHECK_SPRINTF("|   10| |10          | |   10|",    "|%5d| |%-12d| |%5d|", 10, 10, 10);
}


// the '0' flag is ignored with the '-' flag
TEST_CASE("width with minus and zero flag", "[width][non-standard]")
{
  DISABLE_WARNING_PUSH
  DISABLE_WARNING_NON_STANDARD_FORMAT
  CHECK_SPRINTF("Hello               ",              "%0-20s",      "Hello");
  CHECK_SPRINTF("1024                ",              "%0-20d",      1024);
  CHECK_SPRINTF("-1024               ",              "%0-20d",      -1024);
  CHECK_SPRINTF("1024                ",              "%0-20i",      1024);
  CHECK_SPRINTF("-1024               ",              "%0-20i",      -1024);
  CHECK_SPRINTF("1024                ",              "%0-20u",      1024);
  CHECK_SPRINTF("4294966272          ",              "%0-20u",      4294966272U);
  CHECK_SPRINTF("777                 ",              "%0-20o",      511);
  CHECK_SPRINTF("37777777001         ",              "%0-20o",      4294966785U);
  CHECK_SPRINTF("1234abcd            ",              "%0-20x",      305441741);
  CHECK_SPRINTF("edcb5433            ",              "%0-20x",      3989525555U);
  CHECK_SPRINTF("1234ABCD            ",              "%0-20X",      305441741);
  CHECK_SPRINTF("EDCB5433            ",              "%0-20X",      3989525555U);
  CHECK_SPRINTF("x                   ",              "%0-20c",      'x');
  DISABLE_WARNING_POP
}


TEST_CASE("width with zero flag", "[width]")
{
  CHECK_SPRINTF("00000000000000001024",              "%020d",       1024);
  CHECK_SPRINTF("-0000000000000001024",              "%020d",       -1024);
  CHECK_SPRINTF("00000000000000001024",              "%020i",       1024);
  CHECK_SPRINTF("-0000000000000001024",              "%020i",       -1024);
  CHECK_SPRINTF("00000000000000001024",              "%020u",       1024);
  CHECK_SPRINTF("00000000004294966272",              "%020u",       4294966272U);
  CHECK_SPRINTF("00000000000000000777",              "%020o",       511);
  CHECK_SPRINTF("00000000037777777001",              "%020o",       4294966785U);
  CHECK_SPRINTF("0000000000001234abcd",              "%020x",       305441741);
  CHECK_SPRINTF("000000000000edcb5433",              "%020x",       3989525555U);
  CHECK_SPRINTF("0000000000001234ABCD",              "%020X",       305441741);
  CHECK_SPRINTF("000000000000EDCB5433",              "%020X",       3989525555U);
}


// the precision of an integer is the minimum number of digits
TEST_CASE("precision 20", "[width]")
{
  CHECK_SPRINTF("00000000000000001024",              "%.20d",       1024);
  CHECK_SPRINTF("-00000000000000001024",             "%.20d",       -1024);
  CHECK_SPRINTF("00000000000000001024",              "%.20i",       1024);
  CHECK_SPRINTF("-00000000000000001024",             "%.20i",       -1024);
  CHECK_SPRINTF("00000000000000001024",              "%.20u",       1024);
  CHECK_SPRINTF("00000000004294966272",              "%.20u",       4294966272U);
  CHECK_SPRINTF("00000000000000000777",              "%.20o",       511);
  CHECK_SPRINTF("00000000037777777001",              "%.20o",       4294966785U);
  CHECK_SPRINTF("0000000000001234abcd",              "%.20x",       305441741);
  CHECK_SPRINTF("000000000000edcb5433",              "%.20x",       3989525555U);
  CHECK_SPRINTF("0000000000001234ABCD",              "%.20X",       305441741);
  CHECK_SPRINTF("000000000000EDCB5433",              "%.20X",       3989525555U);
}


TEST_CASE("width with precision", "[width]")
{
  CHECK_SPRINTF("               01024",              "%20.5d",      1024);
  CHECK_SPRINTF("              -01024",              "%20.5d",      -1024);
  CHECK_SPRINTF("               01024",              "%20.5i",      1024);
  CHECK_SPRINTF("              -01024",              "%20.5i",      -1024);
  CHECK_SPRINTF("               01024",              "%20.5u",      1024);
  CHECK_SPRINTF("          4294966272",              "%20.5u",      4294966272U);
  CHECK_SPRINTF("               00777",              "%20.5o",      511);
  CHECK_SPRINTF("         37777777001",              "%20.5o",      4294966785U);
  CHECK_SPRINTF("            1234abcd",              "%20.5x",      305441741);
  CHECK_SPRINTF("          00edcb5433",              "%20.10x",     3989525555U);
  CHECK_SPRINTF("            1234ABCD",              "%20.5X",      305441741);
  CHECK_SPRINTF("          00EDCB5433",              "%20.10X",     3989525555U);
  CHECK_SPRINTF("     00004",                        "%10.5d",      4);

  // #49, #113: the precision is honoured with the '-' flag
  CHECK_SPRINTF("001024    ",                        "%-10.6d",     1024);
  CHECK_SPRINTF("00123               ",              "%-20.5i",     123);
}


// a precision of zero prints nothing for a string and for the value zero
TEST_CASE("precision 0", "[width]")
{
  CHECK_SPRINTF("",                                  "%.0s",        "Hello testing");
  CHECK_SPRINTF("                    ",              "%20.0s",      "Hello testing");
  CHECK_SPRINTF("",                                  "%.s",         "Hello testing");
  CHECK_SPRINTF("                    ",              "%20.s",       "Hello testing");
  CHECK_SPRINTF("                1024",              "%20.0d",      1024);
  CHECK_SPRINTF("               -1024",              "%20.0d",      -1024);
  CHECK_SPRINTF("                    ",              "%20.d",       0);
  CHECK_SPRINTF("                1024",              "%20.0i",      1024);
  CHECK_SPRINTF("               -1024",              "%20.i",       -1024);
  CHECK_SPRINTF("                    ",              "%20.i",       0);
  CHECK_SPRINTF("                1024",              "%20.u",       1024);
  CHECK_SPRINTF("          4294966272",              "%20.0u",      4294966272U);
  CHECK_SPRINTF("                    ",              "%20.u",       0U);
  CHECK_SPRINTF("                 777",              "%20.o",       511);
  CHECK_SPRINTF("         37777777001",              "%20.0o",      4294966785U);
  CHECK_SPRINTF("                    ",              "%20.o",       0U);
  CHECK_SPRINTF("            1234abcd",              "%20.x",       305441741);
  CHECK_SPRINTF("            edcb5433",              "%20.0x",      3989525555U);
  CHECK_SPRINTF("                    ",              "%20.x",       0U);
  CHECK_SPRINTF("            1234ABCD",              "%20.X",       305441741);
  CHECK_SPRINTF("            EDCB5433",              "%20.0X",      3989525555U);
  CHECK_SPRINTF("                    ",              "%20.X",       0U);
  CHECK_SPRINTF("                                          1234abcd",
                                                     "%50.x",       305441741);
  CHECK_SPRINTF("                                          1234abcd     12345",
                                                     "%50.x%10.u",  305441741, 12345);
}


// the '0' flag is ignored with a precision
TEST_CASE("precision 0 with zero flag", "[width][non-standard]")
{
  DISABLE_WARNING_PUSH
  DISABLE_WARNING_NON_STANDARD_FORMAT
  CHECK_SPRINTF("  ",                                "%02.0u",      0U);
  CHECK_SPRINTF("  ",                                "%02.0d",      0);
  DISABLE_WARNING_POP
}


// a negative precision is taken as if the precision were omitted
TEST_CASE("negative precision", "[width]")
{
  CHECK_SPRINTF("0",                                 "%.*d",        -1, 0);
  CHECK_SPRINTF("1",                                 "%.*d",        -1, 1);
  CHECK_SPRINTF("abc",                               "%.*s",        -1, "abc");
#if TEST_FLOAT
  CHECK_SPRINTF("1.500000",                          "%.*f",        -1, 1.5);
#endif
}


TEST_CASE("width and precision from arguments", "[width]")
{
  CHECK_SPRINTF("     00042",                        "%*.*d",       10, 5, 42);
  CHECK_SPRINTF("00042     ",                        "%-*.*d",      10, 5, 42);
  CHECK_SPRINTF("       Hel",                        "%*.*s",       10, 3, "Hello");
  CHECK_SPRINTF("Hel       ",                        "%-*.*s",      10, 3, "Hello");
#if TEST_FLOAT
  CHECK_SPRINTF("      3.14",                        "%*.*f",       10, 2, 3.14159);
  CHECK_SPRINTF("3.14      ",                        "%-*.*f",      10, 2, 3.14159);
  CHECK_SPRINTF("0000003.14",                        "%0*.*f",      10, 2, 3.14159);
#endif
}


TEST_CASE("width and precision of strings", "[width]")
{
  CHECK_SPRINTF("       Hel",                        "%10.3s",      "Hello");
  CHECK_SPRINTF("Hel       ",                        "%-10.3s",     "Hello");
  CHECK_SPRINTF("     Hello",                        "%10.8s",      "Hello");
  CHECK_SPRINTF("Hello",                             "%3.8s",       "Hello");
}


// #232 (eyalroz/printf): a width of INT_MIN cannot be negated as an int
TEST_CASE("width INT_MIN from argument", "[width]")
{
  char buffer[8];

  std::memset(buffer, 0xCC, sizeof(buffer));
  snprintf_(buffer, sizeof(buffer), "%*d", INT_MIN, 5);
  CHECK(std::string(buffer) == "5      ");
}
