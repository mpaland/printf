// This file is to be included into either test_suite.cpp or another
// variant of the test suite. It is _not_ self contained in any way


PRINTF_TEST_CASE(space_flag)
{
  char buffer[base_buffer_size];
  (void) buffer;

  PRINTING_CHECK(" 42",                     ==, sprintf_, buffer, "% d", 42);
  PRINTING_CHECK("-42",                     ==, sprintf_, buffer, "% d", -42);
  PRINTING_CHECK("   42",                   ==, sprintf_, buffer, "% 5d", 42);
  PRINTING_CHECK("  -42",                   ==, sprintf_, buffer, "% 5d", -42);
  PRINTING_CHECK("             42",         ==, sprintf_, buffer, "% 15d", 42);
  PRINTING_CHECK("            -42",         ==, sprintf_, buffer, "% 15d", -42);
  PRINTING_CHECK("            -42",         ==, sprintf_, buffer, "% 15d", -42);
#if PRINTF_SUPPORT_DECIMAL_SPECIFIERS
  PRINTING_CHECK("        -42.987",         ==, sprintf_, buffer, "% 15.3f", -42.987);
  PRINTING_CHECK("         42.987",         ==, sprintf_, buffer, "% 15.3f", 42.987);
#endif
  PRINTING_CHECK(" 1024",                   ==, sprintf_, buffer, "% d", 1024);
  PRINTING_CHECK("-1024",                   ==, sprintf_, buffer, "% d", -1024);
  PRINTING_CHECK(" 1024",                   ==, sprintf_, buffer, "% i", 1024);
  PRINTING_CHECK("-1024",                   ==, sprintf_, buffer, "% i", -1024);
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS

PRINTF_TEST_CASE(space_flag__non_standard_format)
{
  char buffer[base_buffer_size];
  (void) buffer;

  PRINTING_CHECK("Hello testing",           ==, sprintf_, buffer, "% s", mkstr("Hello testing"));
  PRINTING_CHECK("1024",                    ==, sprintf_, buffer, "% u", 1024);
#ifdef PRINTF_SUPPORT_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("1024",                    ==, sprintf_, buffer, "% I16u", (uint16_t) 1024);
  PRINTING_CHECK("1024",                    ==, sprintf_, buffer, "% I32u", (uint32_t) 1024);
  PRINTING_CHECK("1024",                    ==, sprintf_, buffer, "% I64u", (uint64_t) 1024);
#endif
  PRINTING_CHECK("4294966272",              ==, sprintf_, buffer, "% u", 4294966272U);
#ifdef PRINTF_SUPPORT_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("4294966272",              ==, sprintf_, buffer, "% I32u", (uint32_t) 4294966272U);
  PRINTING_CHECK("4294966272",              ==, sprintf_, buffer, "% I64u", (uint64_t) 4294966272U);
#endif
  PRINTING_CHECK("777",                     ==, sprintf_, buffer, "% o", 511);
  PRINTING_CHECK("37777777001",             ==, sprintf_, buffer, "% o", 4294966785U);
  PRINTING_CHECK("1234abcd",                ==, sprintf_, buffer, "% x", 305441741);
  PRINTING_CHECK("edcb5433",                ==, sprintf_, buffer, "% x", 3989525555U);
  PRINTING_CHECK("1234ABCD",                ==, sprintf_, buffer, "% X", 305441741);
  PRINTING_CHECK("EDCB5433",                ==, sprintf_, buffer, "% X", 3989525555U);
  PRINTING_CHECK("x",                       ==, sprintf_, buffer, "% c", 'x');
}

#endif


PRINTF_TEST_CASE(plus_flag)
{
  char buffer[base_buffer_size];
  (void) buffer;

  PRINTING_CHECK("+42",                     ==, sprintf_, buffer, "%+d", 42);
  PRINTING_CHECK("-42",                     ==, sprintf_, buffer, "%+d", -42);
  PRINTING_CHECK("  +42",                   ==, sprintf_, buffer, "%+5d", 42);
  PRINTING_CHECK("  -42",                   ==, sprintf_, buffer, "%+5d", -42);
  PRINTING_CHECK("            +42",         ==, sprintf_, buffer, "%+15d", 42);
  PRINTING_CHECK("            -42",         ==, sprintf_, buffer, "%+15d", -42);
  PRINTING_CHECK("+1024",                   ==, sprintf_, buffer, "%+d", 1024);
  PRINTING_CHECK("-1024",                   ==, sprintf_, buffer, "%+d", -1024);
  PRINTING_CHECK("+1024",                   ==, sprintf_, buffer, "%+i", 1024);
  PRINTING_CHECK("-1024",                   ==, sprintf_, buffer, "%+i", -1024);
#ifdef PRINTF_SUPPORT_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("+1024",                   ==, sprintf_, buffer, "%+I16d", (int16_t) 1024);
  PRINTING_CHECK("-1024",                   ==, sprintf_, buffer, "%+I16d", (int16_t) -1024);
  PRINTING_CHECK("+1024",                   ==, sprintf_, buffer, "%+I32d", (int32_t) 1024);
  PRINTING_CHECK("-1024",                   ==, sprintf_, buffer, "%+I32d", (int32_t) -1024);
  PRINTING_CHECK("+1024",                   ==, sprintf_, buffer, "%+I64d", (int64_t) 1024);
  PRINTING_CHECK("-1024",                   ==, sprintf_, buffer, "%+I64d", (int64_t) -1024);
#endif
  PRINTING_CHECK("+",                       ==, sprintf_, buffer, "%+.0d", 0);
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS

PRINTF_TEST_CASE(plus_flag__non_standard_format)
{
  char buffer[base_buffer_size];
  (void) buffer;

  PRINTING_CHECK("Hello testing",           ==, sprintf_, buffer, "%+s", mkstr("Hello testing"));
  PRINTING_CHECK("1024",                    ==, sprintf_, buffer, "%+u", 1024);
#ifdef PRINTF_SUPPORT_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("1024",                    ==, sprintf_, buffer, "%+I32u", (uint32_t) 1024);
#endif
  PRINTING_CHECK("4294966272",              ==, sprintf_, buffer, "%+u", 4294966272U);
#ifdef PRINTF_SUPPORT_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("4294966272",              ==, sprintf_, buffer, "%+I32u", (uint32_t) 4294966272U);
#endif
  PRINTING_CHECK("777",                     ==, sprintf_, buffer, "%+o", 511);
  PRINTING_CHECK("37777777001",             ==, sprintf_, buffer, "%+o", 4294966785U);
  PRINTING_CHECK("1234abcd",                ==, sprintf_, buffer, "%+x", 305441741);
  PRINTING_CHECK("edcb5433",                ==, sprintf_, buffer, "%+x", 3989525555U);
  PRINTING_CHECK("1234ABCD",                ==, sprintf_, buffer, "%+X", 305441741);
  PRINTING_CHECK("EDCB5433",                ==, sprintf_, buffer, "%+X", 3989525555U);
  PRINTING_CHECK("x",                       ==, sprintf_, buffer, "%+c", 'x');
}

#endif


PRINTF_TEST_CASE(zero_flag)
{
  char buffer[base_buffer_size];
  (void) buffer;

  PRINTING_CHECK("42",                      ==, sprintf_, buffer, "%0d", 42);
  PRINTING_CHECK("42",                      ==, sprintf_, buffer, "%0ld", 42L);
  PRINTING_CHECK("-42",                     ==, sprintf_, buffer, "%0d", -42);
  PRINTING_CHECK("00042",                   ==, sprintf_, buffer, "%05d", 42);
  PRINTING_CHECK("-0042",                   ==, sprintf_, buffer, "%05d", -42);
  PRINTING_CHECK("000000000000042",         ==, sprintf_, buffer, "%015d", 42);
  PRINTING_CHECK("-00000000000042",         ==, sprintf_, buffer, "%015d", -42);
#if PRINTF_SUPPORT_DECIMAL_SPECIFIERS
  PRINTING_CHECK("000000000042.12",         ==, sprintf_, buffer, "%015.2f", 42.1234);
  PRINTING_CHECK("00000000042.988",         ==, sprintf_, buffer, "%015.3f", 42.9876);
  PRINTING_CHECK("-00000042.98760",         ==, sprintf_, buffer, "%015.5f", -42.9876);
#endif
}


PRINTF_TEST_CASE(minus_flag)
{
  char buffer[base_buffer_size];
  (void) buffer;

  PRINTING_CHECK("42",                      ==, sprintf_, buffer, "%-d", 42);
  PRINTING_CHECK("-42",                     ==, sprintf_, buffer, "%-d", -42);
  PRINTING_CHECK("42   ",                   ==, sprintf_, buffer, "%-5d", 42);
  PRINTING_CHECK("-42  ",                   ==, sprintf_, buffer, "%-5d", -42);
  PRINTING_CHECK("42             ",         ==, sprintf_, buffer, "%-15d", 42);
  PRINTING_CHECK("-42            ",         ==, sprintf_, buffer, "%-15d", -42);
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS

PRINTF_TEST_CASE(minus_flag_and_non_standard_zero_modifier_for_integers)
{
  char buffer[base_buffer_size];
  (void) buffer;

  PRINTING_CHECK("42",                      ==, sprintf_, buffer, "%-0d", 42);
  PRINTING_CHECK("-42",                     ==, sprintf_, buffer, "%-0d", -42);
  PRINTING_CHECK("42   ",                   ==, sprintf_, buffer, "%-05d", 42);
  PRINTING_CHECK("-42  ",                   ==, sprintf_, buffer, "%-05d", -42);
  PRINTING_CHECK("42             ",         ==, sprintf_, buffer, "%-015d", 42);
  PRINTING_CHECK("-42            ",         ==, sprintf_, buffer, "%-015d", -42);
  PRINTING_CHECK("42",                      ==, sprintf_, buffer, "%0-d", 42);
  PRINTING_CHECK("-42",                     ==, sprintf_, buffer, "%0-d", -42);
  PRINTING_CHECK("42   ",                   ==, sprintf_, buffer, "%0-5d", 42);
  PRINTING_CHECK("-42  ",                   ==, sprintf_, buffer, "%0-5d", -42);
  PRINTING_CHECK("42             ",         ==, sprintf_, buffer, "%0-15d", 42);
  PRINTING_CHECK("-42            ",         ==, sprintf_, buffer, "%0-15d", -42);

#if PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS
  PRINTING_CHECK("-4.200e+01     ",         ==, sprintf_, buffer, "%0-15.3e", -42.);
#else
  PRINTING_CHECK("e",                       ==, sprintf_, buffer, "%0-15.3e", -42.);
#endif

#if PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS
  PRINTING_CHECK("-42            ",         ==, sprintf_, buffer, "%0-15.3g", -42.);
#else
  PRINTING_CHECK("g",                       ==, sprintf_, buffer,  "%0-15.3g", -42.);
#endif
}

#endif


PRINTF_TEST_CASE(sharp_flag)
{
  char buffer[base_buffer_size];
  (void) buffer;

  PRINTING_CHECK("0",                       ==, sprintf_, buffer, "%#o",   0);
  PRINTING_CHECK("0",                       ==, sprintf_, buffer, "%#0o",  0);
  PRINTING_CHECK("0",                       ==, sprintf_, buffer, "%#.0o", 0);
  PRINTING_CHECK("0",                       ==, sprintf_, buffer, "%#.1o", 0);
  PRINTING_CHECK("   0",                    ==, sprintf_, buffer, "%#4o",  0);
  PRINTING_CHECK("0000",                    ==, sprintf_, buffer, "%#.4o", 0);
  PRINTING_CHECK("01",                      ==, sprintf_, buffer, "%#o",   1);
  PRINTING_CHECK("01",                      ==, sprintf_, buffer, "%#0o",  1);
  PRINTING_CHECK("01",                      ==, sprintf_, buffer, "%#.0o", 1);
  PRINTING_CHECK("01",                      ==, sprintf_, buffer, "%#.1o", 1);
  PRINTING_CHECK("  01",                    ==, sprintf_, buffer, "%#4o",  1);
  PRINTING_CHECK("0001",                    ==, sprintf_, buffer, "%#.4o", 1);
  PRINTING_CHECK("0x1001",                  ==, sprintf_, buffer, "%#04x", 0x1001);
  PRINTING_CHECK("01001",                   ==, sprintf_, buffer, "%#04o", 01001);
  PRINTING_CHECK("",                        ==, sprintf_, buffer, "%#.0x", 0);
  PRINTING_CHECK("0x0000614e",              ==, sprintf_, buffer, "%#.8x", 0x614e);
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS

PRINTF_TEST_CASE(sharp_flag__non_standard_format)
{
  char buffer[base_buffer_size];
  (void) buffer;

  PRINTING_CHECK("0b110",                   ==, sprintf_, buffer, "%#b",    6);
  PRINTING_CHECK("0b11111111",              ==, sprintf_, buffer, "%#010b", 0xff);
  PRINTING_CHECK("0b011111111",             ==, sprintf_, buffer, "%#011b", 0xff);
  PRINTING_CHECK("077",                     ==, sprintf_, buffer, "%#03o",  077);
  PRINTING_CHECK("0077",                    ==, sprintf_, buffer, "%#04o",  077);
}

#endif

#if PRINTF_SUPPORT_LONG_LONG

PRINTF_TEST_CASE(sharp_flag_with_long_long)
{
  char buffer[base_buffer_size];
  (void) buffer;

  PRINTING_CHECK("0",                       ==, sprintf_, buffer, "%#llo",   (long long) 0);
  PRINTING_CHECK("0",                       ==, sprintf_, buffer, "%#0llo",  (long long) 0);
  PRINTING_CHECK("0",                       ==, sprintf_, buffer, "%#.0llo", (long long) 0);
  PRINTING_CHECK("0",                       ==, sprintf_, buffer, "%#.1llo", (long long) 0);
  PRINTING_CHECK("   0",                    ==, sprintf_, buffer, "%#4llo",  (long long) 0);
  PRINTING_CHECK("0000",                    ==, sprintf_, buffer, "%#.4llo", (long long) 0);
  PRINTING_CHECK("01",                      ==, sprintf_, buffer, "%#llo",   (long long) 1);
  PRINTING_CHECK("01",                      ==, sprintf_, buffer, "%#0llo",  (long long) 1);
  PRINTING_CHECK("01",                      ==, sprintf_, buffer, "%#.0llo", (long long) 1);
  PRINTING_CHECK("01",                      ==, sprintf_, buffer, "%#.1llo", (long long) 1);
  PRINTING_CHECK("  01",                    ==, sprintf_, buffer, "%#4llo",  (long long) 1);
  PRINTING_CHECK("0001",                    ==, sprintf_, buffer, "%#.4llo", (long long) 1);
  PRINTING_CHECK("0x1001",                  ==, sprintf_, buffer, "%#04llx", (long long) 0x1001);
  PRINTING_CHECK("01001",                   ==, sprintf_, buffer, "%#04llo", (long long) 01001);
  PRINTING_CHECK("",                        ==, sprintf_, buffer, "%#.0llx", (long long) 0);
  PRINTING_CHECK("0x0000614e",              ==, sprintf_, buffer, "%#.8llx", (long long) 0x614e);
}


#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS

PRINTF_TEST_CASE(sharp_flag_with_long_long__non_standard_format)
{
  char buffer[base_buffer_size];
  (void) buffer;

  PRINTING_CHECK("0b110",                   ==, sprintf_, buffer, "%#llb", (long long) 6);
}

#endif
#endif // PRINTF_SUPPORT_LONG_LONG

PRINTF_TEST_CASE(specifier)
{
  char buffer[base_buffer_size];
  (void) buffer;

  PRINTING_CHECK("Hello testing",           ==, sprintf_, buffer, "Hello testing");
  PRINTING_CHECK("Hello testing",           ==, sprintf_, buffer, "%s", mkstr("Hello testing"));

  DISABLE_WARNING_PUSH
  DISABLE_WARNING_PRINTF_FORMAT_OVERFLOW
  PRINTING_CHECK("(null)",                  ==, sprintf_, buffer, "%s", (const char *) nullptr);
  DISABLE_WARNING_POP
  PRINTING_CHECK("1024",                    ==, sprintf_, buffer, "%d", 1024);
#if INT_MAX >= 2147483647LL
  PRINTING_CHECK("2147483647",              ==, sprintf_, buffer, "%d", 2147483647);
  PRINTING_CHECK("4294966272",              ==, sprintf_, buffer, "%u", 4294966272U);
  PRINTING_CHECK("37777777001",             ==, sprintf_, buffer, "%o", 4294966785U);
  PRINTING_CHECK("1234abcd",                ==, sprintf_, buffer, "%x", 305441741);
  PRINTING_CHECK("edcb5433",                ==, sprintf_, buffer, "%x", 3989525555U);
  PRINTING_CHECK("1234ABCD",                ==, sprintf_, buffer, "%X", 305441741);
  PRINTING_CHECK("EDCB5433",                ==, sprintf_, buffer, "%X", 3989525555U);
#endif
  PRINTING_CHECK("-1024",                   ==, sprintf_, buffer, "%d", -1024);
  PRINTING_CHECK("1024",                    ==, sprintf_, buffer, "%i", 1024);
  PRINTING_CHECK("-1024",                   ==, sprintf_, buffer, "%i", -1024);
  PRINTING_CHECK("1024",                    ==, sprintf_, buffer, "%u", 1024);
  PRINTING_CHECK("777",                     ==, sprintf_, buffer, "%o", 511);
  PRINTING_CHECK("%",                       ==, sprintf_, buffer, "%%");

#ifdef PRINTF_SUPPORT_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("127",                     ==, sprintf_, buffer, "%I8d", (int8_t) 127LL);
#if (SHRT_MAX >= 32767)
  PRINTING_CHECK("32767",                   ==, sprintf_, buffer, "%I16d", (int16_t) 32767LL);
#endif
#if (LLONG_MAX >= 2147483647)
  PRINTING_CHECK("2147483647",              ==, sprintf_, buffer, "%I32d", (int32_t) 2147483647LL);
#if (LLONG_MAX >= 9223372036854775807LL)
  PRINTING_CHECK("9223372036854775807",     ==, sprintf_, buffer, "%I64d", (int64_t) 9223372036854775807LL);
#endif
#endif
#endif // PRINTF_SUPPORT_MSVC_STYLE_INTEGER_SPECIFIERS
}


PRINTF_TEST_CASE(width)
{
  char buffer[base_buffer_size];
  (void) buffer;

  PRINTING_CHECK("Hello testing",           ==, sprintf_, buffer, "%1s", mkstr("Hello testing"));
  PRINTING_CHECK("1024",                    ==, sprintf_, buffer, "%1d", 1024);
  PRINTING_CHECK("-1024",                   ==, sprintf_, buffer, "%1d", -1024);
  PRINTING_CHECK("1024",                    ==, sprintf_, buffer, "%1i", 1024);
  PRINTING_CHECK("-1024",                   ==, sprintf_, buffer, "%1i", -1024);
  PRINTING_CHECK("1024",                    ==, sprintf_, buffer, "%1u", 1024);
#ifdef PRINTF_SUPPORT_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("1024",                    ==, sprintf_, buffer, "%1I16u", (uint16_t) 1024);
  PRINTING_CHECK("1024",                    ==, sprintf_, buffer, "%1I32u", (uint32_t) 1024);
  PRINTING_CHECK("1024",                    ==, sprintf_, buffer, "%1I64u", (uint64_t) 1024);
#endif
  PRINTING_CHECK("4294966272",              ==, sprintf_, buffer, "%1u", 4294966272U);
#ifdef PRINTF_SUPPORT_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("4294966272",              ==, sprintf_, buffer, "%1I32u", (uint32_t) 4294966272U);
  PRINTING_CHECK("4294966272",              ==, sprintf_, buffer, "%1I64u", (uint64_t) 4294966272U);
#endif
  PRINTING_CHECK("777",                     ==, sprintf_, buffer, "%1o", 511);
  PRINTING_CHECK("37777777001",             ==, sprintf_, buffer, "%1o", 4294966785U);
  PRINTING_CHECK("1234abcd",                ==, sprintf_, buffer, "%1x", 305441741);
  PRINTING_CHECK("edcb5433",                ==, sprintf_, buffer, "%1x", 3989525555U);
  PRINTING_CHECK("1234ABCD",                ==, sprintf_, buffer, "%1X", 305441741);
  PRINTING_CHECK("EDCB5433",                ==, sprintf_, buffer, "%1X", 3989525555U);
  PRINTING_CHECK("x",                       ==, sprintf_, buffer, "%1c", 'x');
}


PRINTF_TEST_CASE(width_20)
{
  char buffer[base_buffer_size];
  (void) buffer;

  PRINTING_CHECK("               Hello",    ==, sprintf_, buffer, "%20s", mkstr("Hello"));
  PRINTING_CHECK("                1024",    ==, sprintf_, buffer, "%20d", 1024);
  PRINTING_CHECK("               -1024",    ==, sprintf_, buffer, "%20d", -1024);
  PRINTING_CHECK("                1024",    ==, sprintf_, buffer, "%20i", 1024);
  PRINTING_CHECK("               -1024",    ==, sprintf_, buffer, "%20i", -1024);
  PRINTING_CHECK("                   0",    ==, sprintf_, buffer, "%20i", 0);
  PRINTING_CHECK("                1024",    ==, sprintf_, buffer, "%20u", 1024);
#ifdef PRINTF_SUPPORT_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("                1024",    ==, sprintf_, buffer, "%20I16u", (uint16_t) 1024);
  PRINTING_CHECK("                1024",    ==, sprintf_, buffer, "%20I32u", (uint32_t) 1024);
  PRINTING_CHECK("                1024",    ==, sprintf_, buffer, "%20I64u", (uint64_t) 1024);
#endif
  PRINTING_CHECK("          4294966272",    ==, sprintf_, buffer, "%20u", 4294966272U);
#ifdef PRINTF_SUPPORT_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("          4294966272",    ==, sprintf_, buffer, "%20I32u", (uint32_t) 4294966272U);
  PRINTING_CHECK("          4294966272",    ==, sprintf_, buffer, "%20I64u", (uint64_t) 4294966272U);
#endif
  PRINTING_CHECK("                 777",    ==, sprintf_, buffer, "%20o", 511);
  PRINTING_CHECK("         37777777001",    ==, sprintf_, buffer, "%20o", 4294966785U);
  PRINTING_CHECK("            1234abcd",    ==, sprintf_, buffer, "%20x", 305441741);
  PRINTING_CHECK("            edcb5433",    ==, sprintf_, buffer, "%20x", 3989525555U);
  PRINTING_CHECK("            1234ABCD",    ==, sprintf_, buffer, "%20X", 305441741);
  PRINTING_CHECK("            EDCB5433",    ==, sprintf_, buffer, "%20X", 3989525555U);
  PRINTING_CHECK("                   0",    ==, sprintf_, buffer, "%20X", 0);
  PRINTING_CHECK("                   0",    ==, sprintf_, buffer, "%20X", 0U);
#if PRINTF_SUPPORT_LONG_LONG
  PRINTING_CHECK("                   0",    ==, sprintf_, buffer, "%20llX", 0ULL);
#endif
  PRINTING_CHECK("                   x",    ==, sprintf_, buffer, "%20c", 'x');
}


PRINTF_TEST_CASE(width_asterisk_20)
{
  char buffer[base_buffer_size];
  (void) buffer;

  PRINTING_CHECK("               Hello",    ==, sprintf_, buffer, "%*s", 20, mkstr("Hello"));
  PRINTING_CHECK("                1024",    ==, sprintf_, buffer, "%*d", 20, 1024);
  PRINTING_CHECK("               -1024",    ==, sprintf_, buffer, "%*d", 20, -1024);
  PRINTING_CHECK("                1024",    ==, sprintf_, buffer, "%*i", 20, 1024);
  PRINTING_CHECK("               -1024",    ==, sprintf_, buffer, "%*i", 20, -1024);
  PRINTING_CHECK("                1024",    ==, sprintf_, buffer, "%*u", 20, 1024);
#ifdef PRINTF_SUPPORT_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("                1024",    ==, sprintf_, buffer, "%*I16u", 20, (uint16_t) 1024);
  PRINTING_CHECK("                1024",    ==, sprintf_, buffer, "%*I32u", 20, (uint32_t) 1024);
  PRINTING_CHECK("                1024",    ==, sprintf_, buffer, "%*I64u", 20, (uint64_t) 1024);
#endif
  PRINTING_CHECK("          4294966272",    ==, sprintf_, buffer, "%*u", 20, 4294966272U);
#ifdef PRINTF_SUPPORT_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("          4294966272",    ==, sprintf_, buffer, "%*I32u", 20, (uint32_t) 4294966272U);
  PRINTING_CHECK("          4294966272",    ==, sprintf_, buffer, "%*I64u", 20, (uint64_t) 4294966272U);
#endif
  PRINTING_CHECK("                 777",    ==, sprintf_, buffer, "%*o", 20, 511);
  PRINTING_CHECK("         37777777001",    ==, sprintf_, buffer, "%*o", 20, 4294966785U);
  PRINTING_CHECK("            1234abcd",    ==, sprintf_, buffer, "%*x", 20, 305441741);
  PRINTING_CHECK("            edcb5433",    ==, sprintf_, buffer, "%*x", 20, 3989525555U);
  PRINTING_CHECK("            1234ABCD",    ==, sprintf_, buffer, "%*X", 20, 305441741);
  PRINTING_CHECK("            EDCB5433",    ==, sprintf_, buffer, "%*X", 20, 3989525555U);
  PRINTING_CHECK("                   x",    ==, sprintf_, buffer, "%*c", 20, 'x');
}


PRINTF_TEST_CASE(width_minus_20)
{
  char buffer[base_buffer_size];
  (void) buffer;

  PRINTING_CHECK("Hello               ",    ==, sprintf_, buffer, "%-20s", mkstr("Hello"));
  PRINTING_CHECK("1024                ",    ==, sprintf_, buffer, "%-20d", 1024);
  PRINTING_CHECK("-1024               ",    ==, sprintf_, buffer, "%-20d", -1024);
  PRINTING_CHECK("1024                ",    ==, sprintf_, buffer, "%-20i", 1024);
  PRINTING_CHECK("-1024               ",    ==, sprintf_, buffer, "%-20i", -1024);
  PRINTING_CHECK("1024                ",    ==, sprintf_, buffer, "%-20u", 1024);
#if PRINTF_SUPPORT_DECIMAL_SPECIFIERS
  PRINTING_CHECK("1024.1234           ",    ==, sprintf_, buffer, "%-20.4f", 1024.1234);
#endif
  PRINTING_CHECK("4294966272          ",    ==, sprintf_, buffer, "%-20u", 4294966272U);
  PRINTING_CHECK("777                 ",    ==, sprintf_, buffer, "%-20o", 511);
  PRINTING_CHECK("37777777001         ",    ==, sprintf_, buffer, "%-20o", 4294966785U);
  PRINTING_CHECK("1234abcd            ",    ==, sprintf_, buffer, "%-20x", 305441741);
  PRINTING_CHECK("edcb5433            ",    ==, sprintf_, buffer, "%-20x", 3989525555U);
  PRINTING_CHECK("1234ABCD            ",    ==, sprintf_, buffer, "%-20X", 305441741);
  PRINTING_CHECK("EDCB5433            ",    ==, sprintf_, buffer, "%-20X", 3989525555U);
  PRINTING_CHECK("x                   ",    ==, sprintf_, buffer, "%-20c", 'x');
  PRINTING_CHECK("|    9| |9 | |    9|",    ==, sprintf_, buffer, "|%5d| |%-2d| |%5d|", 9, 9, 9);
  PRINTING_CHECK("|   10| |10| |   10|",    ==, sprintf_, buffer, "|%5d| |%-2d| |%5d|", 10, 10, 10);
  PRINTING_CHECK("|    9| |9           | |    9|", ==, sprintf_, buffer, "|%5d| |%-12d| |%5d|", 9, 9, 9);
  PRINTING_CHECK("|   10| |10          | |   10|", ==, sprintf_, buffer, "|%5d| |%-12d| |%5d|", 10, 10, 10);
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS

PRINTF_TEST_CASE(width_0_minus_20)
{
  char buffer[base_buffer_size];
  (void) buffer;

  PRINTING_CHECK("Hello               ",    ==, sprintf_, buffer, "%0-20s", mkstr("Hello"));
  PRINTING_CHECK("1024                ",    ==, sprintf_, buffer, "%0-20d", 1024);
  PRINTING_CHECK("-1024               ",    ==, sprintf_, buffer, "%0-20d", -1024);
  PRINTING_CHECK("1024                ",    ==, sprintf_, buffer, "%0-20i", 1024);
  PRINTING_CHECK("-1024               ",    ==, sprintf_, buffer, "%0-20i", -1024);
  PRINTING_CHECK("1024                ",    ==, sprintf_, buffer, "%0-20u", 1024);
  PRINTING_CHECK("4294966272          ",    ==, sprintf_, buffer, "%0-20u", 4294966272U);
  PRINTING_CHECK("777                 ",    ==, sprintf_, buffer, "%0-20o", 511);
  PRINTING_CHECK("37777777001         ",    ==, sprintf_, buffer, "%0-20o", 4294966785U);
  PRINTING_CHECK("1234abcd            ",    ==, sprintf_, buffer, "%0-20x", 305441741);
  PRINTING_CHECK("edcb5433            ",    ==, sprintf_, buffer, "%0-20x", 3989525555U);
  PRINTING_CHECK("1234ABCD            ",    ==, sprintf_, buffer, "%0-20X", 305441741);
  PRINTING_CHECK("EDCB5433            ",    ==, sprintf_, buffer, "%0-20X", 3989525555U);
  PRINTING_CHECK("x                   ",    ==, sprintf_, buffer, "%0-20c", 'x');
}

#endif

PRINTF_TEST_CASE(padding_20)
{
  char buffer[base_buffer_size];
  (void) buffer;

  PRINTING_CHECK("00000000000000001024",    ==, sprintf_, buffer, "%020d", 1024);
  PRINTING_CHECK("-0000000000000001024",    ==, sprintf_, buffer, "%020d", -1024);
  PRINTING_CHECK("00000000000000001024",    ==, sprintf_, buffer, "%020i", 1024);
  PRINTING_CHECK("-0000000000000001024",    ==, sprintf_, buffer, "%020i", -1024);
  PRINTING_CHECK("00000000000000001024",    ==, sprintf_, buffer, "%020u", 1024);
  PRINTING_CHECK("00000000004294966272",    ==, sprintf_, buffer, "%020u", 4294966272U);
  PRINTING_CHECK("00000000000000000777",    ==, sprintf_, buffer, "%020o", 511);
  PRINTING_CHECK("00000000037777777001",    ==, sprintf_, buffer, "%020o", 4294966785U);
  PRINTING_CHECK("0000000000001234abcd",    ==, sprintf_, buffer, "%020x", 305441741);
  PRINTING_CHECK("000000000000edcb5433",    ==, sprintf_, buffer, "%020x", 3989525555U);
  PRINTING_CHECK("0000000000001234ABCD",    ==, sprintf_, buffer, "%020X", 305441741);
  PRINTING_CHECK("000000000000EDCB5433",    ==, sprintf_, buffer, "%020X", 3989525555U);
}


PRINTF_TEST_CASE(padding_dot_20)
{
  char buffer[base_buffer_size];
  (void) buffer;

  PRINTING_CHECK("00000000000000001024",    ==, sprintf_, buffer, "%.20d", 1024);
  PRINTING_CHECK("-00000000000000001024",   ==, sprintf_, buffer, "%.20d", -1024);
  PRINTING_CHECK("00000000000000001024",    ==, sprintf_, buffer, "%.20i", 1024);
  PRINTING_CHECK("-00000000000000001024",   ==, sprintf_, buffer, "%.20i", -1024);
  PRINTING_CHECK("00000000000000001024",    ==, sprintf_, buffer, "%.20u", 1024);
  PRINTING_CHECK("00000000004294966272",    ==, sprintf_, buffer, "%.20u", 4294966272U);
  PRINTING_CHECK("00000000000000000777",    ==, sprintf_, buffer, "%.20o", 511);
  PRINTING_CHECK("00000000037777777001",    ==, sprintf_, buffer, "%.20o", 4294966785U);
  PRINTING_CHECK("0000000000001234abcd",    ==, sprintf_, buffer, "%.20x", 305441741);
  PRINTING_CHECK("000000000000edcb5433",    ==, sprintf_, buffer, "%.20x", 3989525555U);
  PRINTING_CHECK("0000000000001234ABCD",    ==, sprintf_, buffer, "%.20X", 305441741);
  PRINTING_CHECK("000000000000EDCB5433",    ==, sprintf_, buffer, "%.20X", 3989525555U);
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS

PRINTF_TEST_CASE(padding_sharp_020__non_standard_format)
{
  char buffer[base_buffer_size];
  (void) buffer;

  PRINTING_CHECK("00000000000000001024",    ==, sprintf_, buffer, "%#020d", 1024);
  PRINTING_CHECK("-0000000000000001024",    ==, sprintf_, buffer, "%#020d", -1024);
  PRINTING_CHECK("00000000000000001024",    ==, sprintf_, buffer, "%#020i", 1024);
  PRINTING_CHECK("-0000000000000001024",    ==, sprintf_, buffer, "%#020i", -1024);
  PRINTING_CHECK("00000000000000001024",    ==, sprintf_, buffer, "%#020u", 1024);
  PRINTING_CHECK("00000000004294966272",    ==, sprintf_, buffer, "%#020u", 4294966272U);
}

#endif

PRINTF_TEST_CASE(padding_sharp_020)
{
  char buffer[base_buffer_size];
  (void) buffer;

  PRINTING_CHECK("00000000000000000777",    ==, sprintf_, buffer, "%#020o", 511);
  PRINTING_CHECK("00000000037777777001",    ==, sprintf_, buffer, "%#020o", 4294966785U);
  PRINTING_CHECK("0x00000000001234abcd",    ==, sprintf_, buffer, "%#020x", 305441741);
  PRINTING_CHECK("0x0000000000edcb5433",    ==, sprintf_, buffer, "%#020x", 3989525555U);
  PRINTING_CHECK("0X00000000001234ABCD",    ==, sprintf_, buffer, "%#020X", 305441741);
  PRINTING_CHECK("0X0000000000EDCB5433",    ==, sprintf_, buffer, "%#020X", 3989525555U);
}


#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS

PRINTF_TEST_CASE(padding_sharp_20__non_standard_format)
{
  char buffer[base_buffer_size];
  (void) buffer;

  PRINTING_CHECK("                1024",    ==, sprintf_, buffer, "%#20d", 1024);
  PRINTING_CHECK("               -1024",    ==, sprintf_, buffer, "%#20d", -1024);
  PRINTING_CHECK("                1024",    ==, sprintf_, buffer, "%#20i", 1024);
  PRINTING_CHECK("               -1024",    ==, sprintf_, buffer, "%#20i", -1024);
  PRINTING_CHECK("                1024",    ==, sprintf_, buffer, "%#20u", 1024);
  PRINTING_CHECK("          4294966272",    ==, sprintf_, buffer, "%#20u", 4294966272U);
}

#endif

PRINTF_TEST_CASE(padding_sharp_20)
{
  char buffer[base_buffer_size];
  (void) buffer;

  PRINTING_CHECK("                0777",    ==, sprintf_, buffer, "%#20o", 511);
  PRINTING_CHECK("        037777777001",    ==, sprintf_, buffer, "%#20o", 4294966785U);
  PRINTING_CHECK("          0x1234abcd",    ==, sprintf_, buffer, "%#20x", 305441741);
  PRINTING_CHECK("          0xedcb5433",    ==, sprintf_, buffer, "%#20x", 3989525555U);
  PRINTING_CHECK("          0X1234ABCD",    ==, sprintf_, buffer, "%#20X", 305441741);
  PRINTING_CHECK("          0XEDCB5433",    ==, sprintf_, buffer, "%#20X", 3989525555U);
}


PRINTF_TEST_CASE(padding_20_point_5)
{
  char buffer[base_buffer_size];
  (void) buffer;

  PRINTING_CHECK("               01024",    ==, sprintf_, buffer, "%20.5d", 1024);
  PRINTING_CHECK("              -01024",    ==, sprintf_, buffer, "%20.5d", -1024);
  PRINTING_CHECK("               01024",    ==, sprintf_, buffer, "%20.5i", 1024);
  PRINTING_CHECK("              -01024",    ==, sprintf_, buffer, "%20.5i", -1024);
  PRINTING_CHECK("               01024",    ==, sprintf_, buffer, "%20.5u", 1024);
  PRINTING_CHECK("          4294966272",    ==, sprintf_, buffer, "%20.5u", 4294966272U);
  PRINTING_CHECK("               00777",    ==, sprintf_, buffer, "%20.5o", 511);
  PRINTING_CHECK("         37777777001",    ==, sprintf_, buffer, "%20.5o", 4294966785U);
  PRINTING_CHECK("            1234abcd",    ==, sprintf_, buffer, "%20.5x", 305441741);
  PRINTING_CHECK("          00edcb5433",    ==, sprintf_, buffer, "%20.10x", 3989525555U);
  PRINTING_CHECK("            1234ABCD",    ==, sprintf_, buffer, "%20.5X", 305441741);
  PRINTING_CHECK("          00EDCB5433",    ==, sprintf_, buffer, "%20.10X", 3989525555U);
}


PRINTF_TEST_CASE(padding_negative_numbers)
{
  char buffer[base_buffer_size];
  (void) buffer;

// space padding
  PRINTING_CHECK("-5",                      ==, sprintf_, buffer, "% 1d", -5);
  PRINTING_CHECK("-5",                      ==, sprintf_, buffer, "% 2d", -5);
  PRINTING_CHECK(" -5",                     ==, sprintf_, buffer, "% 3d", -5);
  PRINTING_CHECK("  -5",                    ==, sprintf_, buffer, "% 4d", -5);

// zero padding
  PRINTING_CHECK("-5",                      ==, sprintf_, buffer, "%01d", -5);
  PRINTING_CHECK("-5",                      ==, sprintf_, buffer, "%02d", -5);
  PRINTING_CHECK("-05",                     ==, sprintf_, buffer, "%03d", -5);
  PRINTING_CHECK("-005",                    ==, sprintf_, buffer, "%04d", -5);
}


#if PRINTF_SUPPORT_DECIMAL_SPECIFIERS || PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS

PRINTF_TEST_CASE(float_padding_negative_numbers)
{
  char buffer[base_buffer_size];
  (void) buffer;

  // space padding
#if PRINTF_SUPPORT_DECIMAL_SPECIFIERS
  PRINTING_CHECK("-5.0",                    ==, sprintf_, buffer, "% 3.1f", -5.);
  PRINTING_CHECK("-5.0",                    ==, sprintf_, buffer, "% 4.1f", -5.);
  PRINTING_CHECK(" -5.0",                   ==, sprintf_, buffer, "% 5.1f", -5.);
#endif

#if PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS
  PRINTING_CHECK("    -5",                  ==, sprintf_, buffer, "% 6.1g", -5.);
  PRINTING_CHECK("-5.0e+00",                ==, sprintf_, buffer, "% 6.1e", -5.);
  PRINTING_CHECK("  -5.0e+00",              ==, sprintf_, buffer, "% 10.1e", -5.);
#endif

  // zero padding
#if PRINTF_SUPPORT_DECIMAL_SPECIFIERS
  PRINTING_CHECK("-5.0",                    ==, sprintf_, buffer, "%03.1f", -5.);
  PRINTING_CHECK("-5.0",                    ==, sprintf_, buffer, "%04.1f", -5.);
  PRINTING_CHECK("-05.0",                   ==, sprintf_, buffer, "%05.1f", -5.);

  // zero padding no decimal point
  PRINTING_CHECK("-5",                      ==, sprintf_, buffer, "%01.0f", -5.);
  PRINTING_CHECK("-5",                      ==, sprintf_, buffer, "%02.0f", -5.);
  PRINTING_CHECK("-05",                     ==, sprintf_, buffer, "%03.0f", -5.);
#endif

#if PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS
  PRINTING_CHECK("-005.0e+00",              ==, sprintf_, buffer, "%010.1e", -5.);
  PRINTING_CHECK("-05E+00",                 ==, sprintf_, buffer, "%07.0E", -5.);
  PRINTING_CHECK("-05",                     ==, sprintf_, buffer, "%03.0g", -5.);
#endif
}

#endif // PRINTF_SUPPORT_DECIMAL_SPECIFIERS || PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS

PRINTF_TEST_CASE(length)
{
  char buffer[base_buffer_size];
  (void) buffer;

  PRINTING_CHECK("",                        ==, sprintf_, buffer, "%.0s", mkstr("Hello testing"));
  PRINTING_CHECK("                    ",    ==, sprintf_, buffer, "%20.0s", mkstr("Hello testing"));
  PRINTING_CHECK("",                        ==, sprintf_, buffer, "%.s", mkstr("Hello testing"));
  PRINTING_CHECK("                    ",    ==, sprintf_, buffer, "%20.s", mkstr("Hello testing"));
  PRINTING_CHECK("                1024",    ==, sprintf_, buffer, "%20.0d", 1024);
  PRINTING_CHECK("               -1024",    ==, sprintf_, buffer, "%20.0d", -1024);
  PRINTING_CHECK("                    ",    ==, sprintf_, buffer, "%20.d", 0);
  PRINTING_CHECK("                1024",    ==, sprintf_, buffer, "%20.0i", 1024);
  PRINTING_CHECK("               -1024",    ==, sprintf_, buffer, "%20.i", -1024);
  PRINTING_CHECK("                    ",    ==, sprintf_, buffer, "%20.i", 0);
  PRINTING_CHECK("                1024",    ==, sprintf_, buffer, "%20.u", 1024);
  PRINTING_CHECK("          4294966272",    ==, sprintf_, buffer, "%20.0u", 4294966272U);
  PRINTING_CHECK("                    ",    ==, sprintf_, buffer, "%20.u", 0U);
  PRINTING_CHECK("                 777",    ==, sprintf_, buffer, "%20.o", 511);
  PRINTING_CHECK("         37777777001",    ==, sprintf_, buffer, "%20.0o", 4294966785U);
  PRINTING_CHECK("                    ",    ==, sprintf_, buffer, "%20.o", 0U);
  PRINTING_CHECK("            1234abcd",    ==, sprintf_, buffer, "%20.x", 305441741);
  PRINTING_CHECK("                                          1234abcd",
                                            ==, sprintf_, buffer, "%50.x", 305441741);
  PRINTING_CHECK("                                          1234abcd     12345",
                                            ==, sprintf_, buffer, "%50.x%10.u", 305441741, 12345);
  PRINTING_CHECK("            edcb5433",    ==, sprintf_, buffer, "%20.0x", 3989525555U);
  PRINTING_CHECK("                    ",    ==, sprintf_, buffer, "%20.x", 0U);
  PRINTING_CHECK("            1234ABCD",    ==, sprintf_, buffer, "%20.X", 305441741);
  PRINTING_CHECK("            EDCB5433",    ==, sprintf_, buffer, "%20.0X", 3989525555U);
  PRINTING_CHECK("                    ",    ==, sprintf_, buffer, "%20.X", 0U);
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS

PRINTF_TEST_CASE(length__non_standard_format)
{
  char buffer[base_buffer_size];
  (void) buffer;

  PRINTING_CHECK("  ",                      ==, sprintf_, buffer, "%02.0u", 0U);
  PRINTING_CHECK("  ",                      ==, sprintf_, buffer, "%02.0d", 0);
}

#endif


#if PRINTF_SUPPORT_DECIMAL_SPECIFIERS || PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS

PRINTF_TEST_CASE(infinity_and_not_a_number_values)
{
  char buffer[base_buffer_size];
  (void) buffer;

  // test special-case floats using math.h macros
#if PRINTF_SUPPORT_DECIMAL_SPECIFIERS
  PRINTING_CHECK("     nan",                ==, sprintf_, buffer, "%8f", (double) NAN);
  PRINTING_CHECK("     inf",                ==, sprintf_, buffer, "%8f", (double) INFINITY);
  PRINTING_CHECK("-inf    ",                ==, sprintf_, buffer, "%-8f", (double) -INFINITY);
#endif
#if PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS
  PRINTING_CHECK("     nan",                ==, sprintf_, buffer, "%8e", (double) NAN);
  PRINTING_CHECK("     inf",                ==, sprintf_, buffer, "%8e", (double) INFINITY);
  PRINTING_CHECK("-inf    ",                ==, sprintf_, buffer, "%-8e", (double) -INFINITY);
#endif
}

#endif // PRINTF_SUPPORT_DECIMAL_SPECIFIERS || PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS

#if PRINTF_SUPPORT_DECIMAL_SPECIFIERS

PRINTF_TEST_CASE(floating_point_specifiers_with_31_to_32_bit_integer_values)
{
  char buffer[base_buffer_size];
  (void) buffer;

#if PRINTF_MAX_INTEGRAL_DIGITS_FOR_DECIMAL >= 10
  PRINTING_CHECK("2147483647",              ==, sprintf_, buffer, "%.10f", 2147483647.0); // 2^31 - 1
  PRINTING_CHECK("2147483648",              ==, sprintf_, buffer, "%.10f", 2147483648.0); // 2^31
  PRINTING_CHECK("4294967295",              ==, sprintf_, buffer, "%.10f", 4294967295.0); // 2^32 - 1
  PRINTING_CHECK("4294967296",              ==, sprintf_, buffer, "%.10f", 4294967296.0); // 2^32
#else
  PRINTING_CHECK("2.1474836470e+09",        ==, sprintf_, buffer, "%.10f", 2147483647.0); // 2^31 - 1
  PRINTING_CHECK("2.1474836480e+09",        ==, sprintf_, buffer, "%.10f", 2147483648.0); // 2^31
  PRINTING_CHECK("4.2949672950e+09",        ==, sprintf_, buffer, "%.10f", 4294967295.0); // 2^32 - 1
  PRINTING_CHECK("4.2949672960e+09",        ==, sprintf_, buffer, "%.10f", 4294967296.0); // 2^32
#endif
#if PRINTF_MAX_INTEGRAL_DIGITS_FOR_DECIMAL >= 10
  PRINTING_CHECK("2147483647",              ==, sprintf_, buffer, "%.10g", 2147483647.0); // 2^31 - 1
  PRINTING_CHECK("2147483648",              ==, sprintf_, buffer, "%.10g", 2147483648.0); // 2^31
  PRINTING_CHECK("4294967295",              ==, sprintf_, buffer, "%.10g", 4294967295.0); // 2^32 - 1
  PRINTING_CHECK("4294967296",              ==, sprintf_, buffer, "%.10g", 4294967296.0); // 2^32
#else
  // like %f, the fixed point notation of %g is limited to values up to PRINTF_MAX_FLOAT
  PRINTING_CHECK("2.147483647e+09",         ==, sprintf_, buffer, "%.10g", 2147483647.0); // 2^31 - 1
  PRINTING_CHECK("2.147483648e+09",         ==, sprintf_, buffer, "%.10g", 2147483648.0); // 2^31
  PRINTING_CHECK("4.294967295e+09",         ==, sprintf_, buffer, "%.10g", 4294967295.0); // 2^32 - 1
  PRINTING_CHECK("4.294967296e+09",         ==, sprintf_, buffer, "%.10g", 4294967296.0); // 2^32
#endif
}

#endif

#if PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS

PRINTF_TEST_CASE(tiny_floating_point_values)
{
  char buffer[base_buffer_size];
  (void) buffer;

//  boltzman_s_constant = 1.380651569e-23;

  PRINTING_CHECK("1e-23",                   ==, sprintf_, buffer, "%.0e",  1.380651569e-23);
  PRINTING_CHECK("1.4e-23",                 ==, sprintf_, buffer, "%.1e",  1.380651569e-23);
  PRINTING_CHECK("1.38e-23",                ==, sprintf_, buffer, "%.2e",  1.380651569e-23);
  PRINTING_CHECK("1.381e-23",               ==, sprintf_, buffer, "%.3e",  1.380651569e-23);
  PRINTING_CHECK("1.3807e-23",              ==, sprintf_, buffer, "%.4e",  1.380651569e-23);
  PRINTING_CHECK("1.38065e-23",             ==, sprintf_, buffer, "%.5e",  1.380651569e-23);
  PRINTING_CHECK("1.380652e-23",            ==, sprintf_, buffer, "%.6e",  1.380651569e-23);
  PRINTING_CHECK("1.3806516e-23",           ==, sprintf_, buffer, "%.7e",  1.380651569e-23);
  PRINTING_CHECK("1.38065157e-23",          ==, sprintf_, buffer, "%.8e",  1.380651569e-23);
  PRINTING_CHECK("1.380651569e-23",         ==, sprintf_, buffer, "%.9e",  1.380651569e-23);
  PRINTING_CHECK("1.3806515690e-23",        ==, sprintf_, buffer, "%.10e", 1.380651569e-23);
  PRINTING_CHECK("1.38065156900e-23",       ==, sprintf_, buffer, "%.11e", 1.380651569e-23);
// These go beyond our precision abilities; enable them, and they will likely fail.
//  PRINTING_CHECK("1.380651569000e-23",    ==, sprintf_, buffer, "%.12e", 1.380651569e-23);
//  PRINTING_CHECK("1.3806515690000e-23",   ==, sprintf_, buffer, "%.13e", 1.380651569e-23);
//  PRINTING_CHECK("1.38065156900000e-23",  ==, sprintf_, buffer, "%.14e", 1.380651569e-23);
//  PRINTING_CHECK("1.380651569000000e-23", ==, sprintf_, buffer, "%.15e", 1.380651569e-23);
//  PRINTING_CHECK("1.3806515690000000e-23", ==, sprintf_, buffer, "%.16e", 1.380651569e-23);
}

#endif

#if PRINTF_SUPPORT_DECIMAL_SPECIFIERS

PRINTF_TEST_CASE(fallback_from_decimal_to_exponential)
{
  char buffer[base_buffer_size];
  (void) buffer;

  CAPTURE_AND_PRINT(sprintf_, buffer, "%.0f", (double) ((int64_t) 1 * 1000));
  if (PRINTF_MAX_INTEGRAL_DIGITS_FOR_DECIMAL < 3) {
    CHECK(!strcmp(buffer, "1e+3"));
  } else {
    CHECK(!strcmp(buffer, "1000"));
  }

  CAPTURE_AND_PRINT(sprintf_, buffer, "%.0f", (double) ((int64_t) 1 * 1000 * 1000));
  if (PRINTF_MAX_INTEGRAL_DIGITS_FOR_DECIMAL < 6) {
    CHECK(!strcmp(buffer, "1e+6"));
  } else {
    CHECK(!strcmp(buffer, "1000000"));
  }

  CAPTURE_AND_PRINT(sprintf_, buffer, "%.0f", (double) ((int64_t) 1 * 1000 * 1000 * 1000));
  if (PRINTF_MAX_INTEGRAL_DIGITS_FOR_DECIMAL < 9) {
    CHECK(!strcmp(buffer, "1e+9"));
  } else {
    CHECK(!strcmp(buffer, "1000000000"));
  }

  CAPTURE_AND_PRINT(sprintf_, buffer, "%.0f", (double) ((int64_t) 1 * 1000 * 1000 * 1000 * 1000));
  if (PRINTF_MAX_INTEGRAL_DIGITS_FOR_DECIMAL < 12) {
#if PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS
    CHECK(!strcmp(buffer, "1e+12"));
#else
    CHECK(!strcmp(buffer, ""));
#endif
  } else {
    CHECK(!strcmp(buffer, "1000000000000"));
  }

  CAPTURE_AND_PRINT(sprintf_, buffer, "%.0f", (double) ((int64_t) 1 * 1000 * 1000 * 1000 * 1000 * 1000));
  if (PRINTF_MAX_INTEGRAL_DIGITS_FOR_DECIMAL < 15) {
#if PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS
    CHECK(!strcmp(buffer, "1e+15"));
#else
    CHECK(!strcmp(buffer, ""));
#endif
  } else {
    CHECK(!strcmp(buffer, "1000000000000000"));
  }

  // A value which should Should definitely be out of range for float
  CAPTURE_AND_PRINT(sprintf_, buffer, "%.1f", 1E20);
#if PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS
  CHECK(!strcmp(buffer, "1.0e+20"));
#else
  CHECK(!strcmp(buffer, ""));
#endif

}

#endif // PRINTF_SUPPORT_DECIMAL_SPECIFIERS

#if PRINTF_SUPPORT_DECIMAL_SPECIFIERS || PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS

#if PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS
PRINTF_TEST_CASE(avoiding_undefined_behavior)
{
  char buffer[base_buffer_size];
  (void) buffer;

  // May cause UB in call of pow10_of_int() with a high power of 10; see issue #220
  PRINTING_CHECK("0.0014664",               ==, sprintf_, buffer, "%g", 0.0014663999499999749);
}
#endif

PRINTF_TEST_CASE(floating_point_specifiers_precision_and_flags)
{
  char buffer[base_buffer_size];
  (void) buffer;

#if PRINTF_SUPPORT_DECIMAL_SPECIFIERS
  PRINTING_CHECK("3.1415",                  ==, sprintf_, buffer, "%.4f", 3.1415354);
  PRINTING_CHECK("30343.142",               ==, sprintf_, buffer, "%.3f", 30343.1415354);
  PRINTING_CHECK("34",                      ==, sprintf_, buffer, "%.0f", 34.1415354);
  PRINTING_CHECK("1",                       ==, sprintf_, buffer, "%.0f", 1.3);
  PRINTING_CHECK("2",                       ==, sprintf_, buffer, "%.0f", 1.55);
  PRINTING_CHECK("1.6",                     ==, sprintf_, buffer, "%.1f", 1.64);
  PRINTING_CHECK("42.90",                   ==, sprintf_, buffer, "%.2f", 42.8952);
  PRINTING_CHECK("42.895200000",            ==, sprintf_, buffer, "%.9f", 42.8952);
  PRINTING_CHECK("42.8952230000",           ==, sprintf_, buffer, "%.10f", 42.895223);
  // this implementation supports at most 9 decimal digits, further digits are zero
  PRINTING_CHECK("42.895223123000",         ==, sprintf_, buffer, "%.12f", 42.89522312345678);
  PRINTING_CHECK("42477.371093750000000",   ==, sprintf_, buffer, "%020.15f", 42477.37109375);
  PRINTING_CHECK("42.895223877000",         ==, sprintf_, buffer, "%.12f", 42.89522387654321);
  PRINTING_CHECK(" 42.90",                  ==, sprintf_, buffer, "%6.2f", 42.8952);
  PRINTING_CHECK("+42.90",                  ==, sprintf_, buffer, "%+6.2f", 42.8952);
  PRINTING_CHECK("+42.9",                   ==, sprintf_, buffer, "%+5.1f", 42.9252);
  PRINTING_CHECK("42.500000",               ==, sprintf_, buffer, "%f", 42.5);
  PRINTING_CHECK("42.5",                    ==, sprintf_, buffer, "%.1f", 42.5);
  PRINTING_CHECK("42167.000000",            ==, sprintf_, buffer, "%f", 42167.0);
  PRINTING_CHECK("-12345.987654321",        ==, sprintf_, buffer, "%.9f", -12345.987654321);
  PRINTING_CHECK("4.0",                     ==, sprintf_, buffer, "%.1f", 3.999);
  PRINTING_CHECK("4",                       ==, sprintf_, buffer, "%.0f", 3.5);
  PRINTING_CHECK("4",                       ==, sprintf_, buffer, "%.0f", 4.5);
  PRINTING_CHECK("3",                       ==, sprintf_, buffer, "%.0f", 3.49);
  PRINTING_CHECK("3.5",                     ==, sprintf_, buffer, "%.1f", 3.49);
  PRINTING_CHECK("a0.5  ",                  ==, sprintf_, buffer, "a%-5.1f", 0.5);
  PRINTING_CHECK("a0.5  end",               ==, sprintf_, buffer, "a%-5.1fend", 0.5);

  /* %f for double */
  // this implementation supports at most 9 decimal digits, further digits are zero
  PRINTING_CHECK("42.895223123000",         ==, sprintf_, buffer, "%.12f", 42.89522312345678);
  /* %F for double */
  PRINTING_CHECK("42.895223123000",         ==, sprintf_, buffer, "%.12F", 42.89522312345678);
  /* %lf for double */
  PRINTING_CHECK("42.895223123000",         ==, sprintf_, buffer, "%.12lf", 42.89522312345678);
  /* %Lf for long double */
  PRINTING_CHECK("42.895223123000",         ==, sprintf_, buffer, "%.12Lf", 42.89522312345678l);
  PRINTING_CHECK("42.895223123000",         ==, sprintf_, buffer, "%.12Lf", 42.89522312345678L);

#endif
#if PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS
  PRINTING_CHECK("0.5",                     ==, sprintf_, buffer, "%.4g", 0.5);
  PRINTING_CHECK("1",                       ==, sprintf_, buffer, "%.4g", 1.0);
  PRINTING_CHECK("12345.7",                 ==, sprintf_, buffer, "%G", 12345.678);
  PRINTING_CHECK("12345.68",                ==, sprintf_, buffer, "%.7G", 12345.678);
  PRINTING_CHECK("1.2346E+08",              ==, sprintf_, buffer, "%.5G", 123456789.);
  PRINTING_CHECK("12345",                   ==, sprintf_, buffer, "%.6G", 12345.);
  PRINTING_CHECK("  +1.235e+08",            ==, sprintf_, buffer, "%+12.4g", 123456789.);
  PRINTING_CHECK("0.0012",                  ==, sprintf_, buffer, "%.2G", 0.001234);
  PRINTING_CHECK(" +0.001234",              ==, sprintf_, buffer, "%+10.4G", 0.001234);
  PRINTING_CHECK("+001.234e-05",            ==, sprintf_, buffer, "%+012.4g", 0.00001234);
  // Note: The following two values are _barely_ normal; make their mantissa 1.1 and they lose their normality.
  PRINTING_CHECK("-1.23e-308",              ==, sprintf_, buffer, "%.3g", -1.2345e-308);
  PRINTING_CHECK("+1.230E+308",             ==, sprintf_, buffer, "%+.3E", 1.23e+308);
  PRINTING_CHECK("1.000e+01",               ==, sprintf_, buffer, "%.3e", 9.9996);
  PRINTING_CHECK("0",                       ==, sprintf_, buffer, "%g", 0.);
  PRINTING_CHECK("-0",                      ==, sprintf_, buffer, "%g", -0.);
  PRINTING_CHECK("+0",                      ==, sprintf_, buffer, "%+g", 0.);
  PRINTING_CHECK("-0",                      ==, sprintf_, buffer, "%+g", -0.);
  PRINTING_CHECK("-4e+04",                  ==, sprintf_, buffer, "%.1g", -40661.5);
  PRINTING_CHECK("-4.e+04",                 ==, sprintf_, buffer, "%#.1g", -40661.5);
  PRINTING_CHECK("100.",                    ==, sprintf_, buffer, "%#.3g", 99.998580932617187500);
  PRINTING_CHECK(" 1.e+01",                 ==, sprintf_, buffer, "%# 01.1g", 9.8);
  // Note: The following value is _barely_ normal; make the mantissa 1.1 and it loses its normality.
  // this implementation supports at most 9 decimal digits, further digits are zero
  PRINTING_CHECK("1.2345678900e-308",       ==, sprintf_, buffer, "%.10e", 1.2345678901e-308);
  // Rounding-focused checks
  PRINTING_CHECK("4.895512e+04",            ==, sprintf_, buffer, "%e", 48955.125);
  PRINTING_CHECK("9.2524e+04",              ==, sprintf_, buffer, "%.4e", 92523.5);
  PRINTING_CHECK("-8.380923438e+04",        ==, sprintf_, buffer, "%.9e", -83809.234375);

  /* %g for double */
  PRINTING_CHECK("100.",                    ==, sprintf_, buffer, "%#.3g", 99.998580932617187500);
  /* %G for double */
  PRINTING_CHECK("100.",                    ==, sprintf_, buffer, "%#.3G", 99.998580932617187500);
  /* %lg for double */
  PRINTING_CHECK("100.",                    ==, sprintf_, buffer, "%#.3lg", 99.998580932617187500);
  /* %Lg for long double */
  PRINTING_CHECK("100.",                    ==, sprintf_, buffer, "%#.3Lg", 99.998580932617187500l);
  PRINTING_CHECK("100.",                    ==, sprintf_, buffer, "%#.3Lg", 99.998580932617187500L);

  /* %e for double */
  PRINTING_CHECK("-8.380923438e+04",        ==, sprintf_, buffer, "%.9e", -83809.234375);
  /* %E for double */
  PRINTING_CHECK("-8.380923438E+04",        ==, sprintf_, buffer, "%.9E", -83809.234375);
  /* %le for double */
  PRINTING_CHECK("-8.380923438e+04",        ==, sprintf_, buffer, "%.9le", -83809.234375);
  /* %Le for long double */
  PRINTING_CHECK("-8.380923438e+04",        ==, sprintf_, buffer, "%.9Le", -83809.234375l);
  PRINTING_CHECK("-8.380923438e+04",        ==, sprintf_, buffer, "%.9Le", -83809.234375L);
#endif
}
#endif // PRINTF_SUPPORT_DECIMAL_SPECIFIERS || PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS

PRINTF_TEST_CASE(integer_types)
{
  char buffer[base_buffer_size];
  (void) buffer;

  PRINTING_CHECK("0",                       ==, sprintf_, buffer, "%i", 0);
  PRINTING_CHECK("1234",                    ==, sprintf_, buffer, "%i", 1234);
  PRINTING_CHECK("32767",                   ==, sprintf_, buffer, "%i", 32767);
  PRINTING_CHECK("-32767",                  ==, sprintf_, buffer, "%i", -32767);
  PRINTING_CHECK("30",                      ==, sprintf_, buffer, "%li", 30L);
  PRINTING_CHECK("-2147483647",             ==, sprintf_, buffer, "%li", -2147483647L);
  PRINTING_CHECK("2147483647",              ==, sprintf_, buffer, "%li", 2147483647L);
#if PRINTF_SUPPORT_LONG_LONG
  PRINTING_CHECK("30",                      ==, sprintf_, buffer, "%lli", 30LL);
  PRINTING_CHECK("-9223372036854775807",    ==, sprintf_, buffer, "%lli", -9223372036854775807LL);
  PRINTING_CHECK("9223372036854775807",     ==, sprintf_, buffer, "%lli", 9223372036854775807LL);
#endif
  PRINTING_CHECK("100000",                  ==, sprintf_, buffer, "%lu", 100000L);
  PRINTING_CHECK("4294967295",              ==, sprintf_, buffer, "%lu", 0xFFFFFFFFL);
#if PRINTF_SUPPORT_LONG_LONG
  PRINTING_CHECK("281474976710656",         ==, sprintf_, buffer, "%llu", 281474976710656LLU);
  PRINTING_CHECK("18446744073709551615",    ==, sprintf_, buffer, "%llu", 18446744073709551615LLU);
#endif
  PRINTING_CHECK("2147483647",              ==, sprintf_, buffer, "%zu", (size_t) 2147483647UL);
  PRINTING_CHECK("2147483647",              ==, sprintf_, buffer, "%zd", (size_t) 2147483647UL);
  PRINTING_CHECK("-2147483647",             ==, sprintf_, buffer, "%zi", (ssize_t) -2147483647L);
  PRINTING_CHECK("165140",                  ==, sprintf_, buffer, "%o", 60000);
  PRINTING_CHECK("57060516",                ==, sprintf_, buffer, "%lo", 12345678L);
  PRINTING_CHECK("12345678",                ==, sprintf_, buffer, "%lx", 0x12345678L);
#if PRINTF_SUPPORT_LONG_LONG
  PRINTING_CHECK("1234567891234567",        ==, sprintf_, buffer, "%llx", 0x1234567891234567LLU);
#endif
  PRINTING_CHECK("abcdefab",                ==, sprintf_, buffer, "%lx", 0xabcdefabL);
  PRINTING_CHECK("ABCDEFAB",                ==, sprintf_, buffer, "%lX", 0xabcdefabL);
  PRINTING_CHECK("v",                       ==, sprintf_, buffer, "%c", 'v');
  PRINTING_CHECK("wv",                      ==, sprintf_, buffer, "%cv", 'w');
  PRINTING_CHECK("A Test",                  ==, sprintf_, buffer, "%s", mkstr("A Test"));
  PRINTING_CHECK("255",                     ==, sprintf_, buffer, "%hhu", (unsigned char) 0xFFU);
  PRINTING_CHECK("4660",                    ==, sprintf_, buffer, "%hu", (unsigned short) 0x1234u);
  PRINTING_CHECK("Test100 65535",           ==, sprintf_, buffer, "%s%hhi %hu", mkstr("Test"), (char) 100, (unsigned short) 0xFFFF);
  PRINTING_CHECK("a",                       ==, sprintf_, buffer, "%tx", &buffer[10] - &buffer[0]);
  PRINTING_CHECK("-2147483647",             ==, sprintf_, buffer, "%ji", (intmax_t) -2147483647L);
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS

PRINTF_TEST_CASE(types__non_standard_format)
{
  char buffer[base_buffer_size];
  (void) buffer;

  PRINTING_CHECK("1110101001100000",        ==, sprintf_, buffer, "%b", 60000);
  PRINTING_CHECK("101111000110000101001110", ==, sprintf_, buffer, "%lb", 12345678L);
}

#endif

PRINTF_TEST_CASE(pointer)
{
  char buffer[base_buffer_size];
  (void) buffer;

  // the format of %p is implementation-defined: this implementation prints
  // all digits of the pointer in uppercase hex, without a '0x' prefix
  CAPTURE_AND_PRINT(sprintf_, buffer, "%p", (void *) 0x1234U);
  if (sizeof(void *) == 4U) {
    CHECK(!strcmp(buffer, "00001234"));
  } else {
    CHECK(!strcmp(buffer, "0000000000001234"));
  }

  CAPTURE_AND_PRINT(sprintf_, buffer, "%p", (void *) 0x12345678U);
  if (sizeof(void *) == 4U) {
    CHECK(!strcmp(buffer, "12345678"));
  } else {
    CHECK(!strcmp(buffer, "0000000012345678"));
  }

  CAPTURE_AND_PRINT(sprintf_, buffer, "%p-%p", (void *) 0x12345678U, (void *) 0x7EDCBA98U);
  if (sizeof(void *) == 4U) {
    CHECK(!strcmp(buffer, "12345678-7EDCBA98"));
  } else {
    CHECK(!strcmp(buffer, "0000000012345678-000000007EDCBA98"));
  }

  CAPTURE_AND_PRINT(sprintf_, buffer, "%p", (void *) (uintptr_t) 0xFFFFFFFFU);
  if (sizeof(uintptr_t) == sizeof(uint64_t)) {
    CHECK(!strcmp(buffer, "00000000FFFFFFFF"));
  } else {
    CHECK(!strcmp(buffer, "FFFFFFFF"));
  }

  CAPTURE_AND_PRINT(sprintf_, buffer, "%3p", (const void *) 0xff);
  if (sizeof(void *) == 4U) {
    CHECK(!strcmp(buffer, "000000FF"));
  } else {
    CHECK(!strcmp(buffer, "00000000000000FF"));
  }

  CAPTURE_AND_PRINT(sprintf_, buffer, "%p", (const void *) NULL);
  if (sizeof(void *) == 4U) {
    CHECK(!strcmp(buffer, "00000000"));
  } else {
    CHECK(!strcmp(buffer, "0000000000000000"));
  }
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS

PRINTF_TEST_CASE(unknown_flag__non_standard_format)
{
  char buffer[base_buffer_size];
  (void) buffer;

  PRINTING_CHECK("kmarco",                  ==, sprintf_, buffer, "%kmarco", 42, 37);
}

#endif

PRINTF_TEST_CASE(string_length)
{
  char buffer[base_buffer_size];
  (void) buffer;

  PRINTING_CHECK("This",                    ==, sprintf_, buffer, "%.4s", mkstr("This is a test"));
  PRINTING_CHECK("test",                    ==, sprintf_, buffer, "%.4s", mkstr("test"));
  PRINTING_CHECK("123",                     ==, sprintf_, buffer, "%.7s", mkstr("123"));
  PRINTING_CHECK("",                        ==, sprintf_, buffer, "%.7s", mkstr(""));
  PRINTING_CHECK("1234ab",                  ==, sprintf_, buffer, "%.4s%.2s", mkstr("123456"), mkstr("abcdef"));
  PRINTING_CHECK("123",                     ==, sprintf_, buffer, "%.*s", 3, mkstr("123456"));

  DISABLE_WARNING_PUSH
  DISABLE_WARNING_PRINTF_FORMAT_OVERFLOW
  PRINTING_CHECK("(null)",                  ==, sprintf_, buffer, "%.*s", 3, (const char *) NULL);
  DISABLE_WARNING_POP
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS

PRINTF_TEST_CASE(string_length__non_standard_format)
{
  char buffer[base_buffer_size];
  (void) buffer;

  DISABLE_WARNING_PUSH
  DISABLE_WARNING_PRINTF_FORMAT
  DISABLE_WARNING_PRINTF_FORMAT_EXTRA_ARGS
  DISABLE_WARNING_PRINTF_FORMAT_INVALID_SPECIFIER
  PRINTING_CHECK(".2s",                     ==, sprintf_, buffer, "%.4.2s", "123456");
  DISABLE_WARNING_POP
}

#endif


PRINTF_TEST_CASE(buffer_length)
{
  char buffer[base_buffer_size];
  int ret;
  (void) buffer;

  CAPTURE_AND_PRINT_WITH_RETVAL(ret, snprintf_, nullptr, 10, "%s", mkstr("Test"));
  CHECK(ret == 4);
  CAPTURE_AND_PRINT_WITH_RETVAL(ret, snprintf_, nullptr, (size_t) 0, "%s", mkstr("Test"));
  CHECK(ret == 4);

  buffer[0] = (char) 0xA5;
  CAPTURE_AND_PRINT_WITH_RETVAL(ret, snprintf_, buffer, (size_t) 0, "%s", mkstr("Test"));
  CHECK(buffer[0] == (char) 0xA5);
  CHECK(ret == 4);

  buffer[0] = (char) 0xCC;
  CAPTURE_AND_PRINT_WITH_RETVAL(ret, snprintf_, buffer, 1, "%s", mkstr("Test"));
  CHECK(buffer[0] == '\0');
  PRINTING_CHECK_WITH_BUF_SIZE("H",         ==, snprintf_, buffer, 2, "%s", mkstr("Hello"));

  DISABLE_WARNING_PUSH
  DISABLE_WARNING_PRINTF_FORMAT_OVERFLOW
  CAPTURE_AND_PRINT_WITH_RETVAL(ret, snprintf_, buffer, 2, "%s", (const char *) NULL);
  CHECK(!strcmp(buffer, "("));
  DISABLE_WARNING_POP
}


PRINTF_TEST_CASE(misc)
{
  char buffer[base_buffer_size];
  (void) buffer;

  PRINTING_CHECK("53000atest-20 bit",       ==, sprintf_, buffer, "%u%u%ctest%d %s", 5, 3000, 'a', -20, mkstr("bit"));
#if PRINTF_SUPPORT_DECIMAL_SPECIFIERS
  PRINTING_CHECK("0.33",                    ==, sprintf_, buffer, "%.*f", 2, 0.33333333);
  PRINTING_CHECK("1",                       ==, sprintf_, buffer, "%.*d", -1, 1);
  PRINTING_CHECK("foo",                     ==, sprintf_, buffer, "%.3s", mkstr("foobar"));
  PRINTING_CHECK(" ",                       ==, sprintf_, buffer, "% .0d", 0);
  PRINTING_CHECK("     00004",              ==, sprintf_, buffer, "%10.5d", 4);
  PRINTING_CHECK("hi x",                    ==, sprintf_, buffer, "%*sx", -3, mkstr("hi"));
  PRINTING_CHECK("00123               ",    ==, sprintf_, buffer, "%-20.5i", 123);
  PRINTING_CHECK("-67224.546875000000000000", ==, sprintf_, buffer, "%.18f", -67224.546875);
#endif
#if PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS
  PRINTING_CHECK("0.33",                    ==, sprintf_, buffer, "%.*g", 2, 0.33333333);
  PRINTING_CHECK("3.33e-01",                ==, sprintf_, buffer, "%.*e", 2, 0.33333333);
  PRINTING_CHECK("0.000000e+00",            ==, sprintf_, buffer, "%e", 0.0);
  PRINTING_CHECK("-0.000000e+00",           ==, sprintf_, buffer, "%e", -0.0);
#endif
  // What should we get with  sprintf_(buffer, "%.2f", 0.995) ?
  // that number cannot be represented as a double, and the closest double
  // is actually 0.9949999999999999955591... and so on. That should give
  // us 0.99 as the output.
  // still, we can't complain about that value being confused with real 0.995, and
  // so we can't complain if we get the banker's rounding for 0.995 to 1.00 as the
  // output
  //PRINTING_CHECK("1.00",                    ==, sprintf_, buffer, "%.2f", 0.995);

  // Two invocations which made clang 21's integer sanitizer suspicious (issue #220)
  PRINTING_CHECK("{",                         ==, sprintf_, buffer, "%s", "{");
  PRINTING_CHECK("0.0014664",                 ==, sprintf_, buffer, "%g", 0.0014663999499999749);
}


PRINTF_TEST_CASE(extremal_signed_integer_values)
{
  char buffer[base_buffer_size];
  char expected[base_buffer_size];
  (void) buffer;
  (void) expected;

  std::sprintf(expected, "%hhd", std::numeric_limits<char>::max());
  PRINTING_CHECK(expected,                  ==, sprintf_, buffer, "%hhd", std::numeric_limits<char>::max());

  std::sprintf(expected, "%hd", std::numeric_limits<short int>::max());
  PRINTING_CHECK(expected,                  ==, sprintf_, buffer, "%hd", std::numeric_limits<short int>::max());

  std::sprintf(expected, "%hd", std::numeric_limits<short int>::max());
  PRINTING_CHECK(expected,                  ==, sprintf_, buffer, "%hd", std::numeric_limits<short int>::max());

  std::sprintf(expected, "%d", std::numeric_limits<int>::min());
  PRINTING_CHECK(expected,                  ==, sprintf_, buffer, "%d", std::numeric_limits<int>::min());

  std::sprintf(expected, "%d", std::numeric_limits<int>::max());
  PRINTING_CHECK(expected,                  ==, sprintf_, buffer, "%d", std::numeric_limits<int>::max());

  std::sprintf(expected, "%ld", std::numeric_limits<long int>::min());
  PRINTING_CHECK(expected,                  ==, sprintf_, buffer, "%ld", std::numeric_limits<long int>::min());

  std::sprintf(expected, "%ld", std::numeric_limits<long int>::max());
  PRINTING_CHECK(expected,                  ==, sprintf_, buffer, "%ld", std::numeric_limits<long int>::max());

#if PRINTF_SUPPORT_LONG_LONG
  std::sprintf(expected, "%lld", std::numeric_limits<long long int>::min());
  PRINTING_CHECK(expected,                  ==, sprintf_, buffer, "%lld", std::numeric_limits<long long int>::min());

  std::sprintf(expected, "%lld", std::numeric_limits<long long int>::max());
  PRINTING_CHECK(expected,                  ==, sprintf_, buffer, "%lld", std::numeric_limits<long long int>::max());
#endif
}

PRINTF_TEST_CASE(extremal_unsigned_integer_values)
{
  char buffer[base_buffer_size];
  char expected[base_buffer_size];

  (void) buffer;
  (void) expected;

  std::sprintf(expected, "%hhu", std::numeric_limits<char unsigned>::max());
  PRINTING_CHECK(expected,                  ==, sprintf_, buffer, "%hhu", std::numeric_limits<char unsigned>::max());

  std::sprintf(expected, "%hu", std::numeric_limits<short unsigned>::max());
  PRINTING_CHECK(expected,                  ==, sprintf_, buffer, "%hu", std::numeric_limits<short unsigned>::max());

  std::sprintf(expected, "%u", std::numeric_limits<unsigned>::max());
  PRINTING_CHECK(expected,                  ==, sprintf_, buffer, "%u", std::numeric_limits<unsigned>::max());

  std::sprintf(expected, "%lu", std::numeric_limits<long unsigned>::max());
  PRINTING_CHECK(expected,                  ==, sprintf_, buffer, "%lu", std::numeric_limits<long unsigned>::max());

#if PRINTF_SUPPORT_LONG_LONG
  std::sprintf(expected, "%llu", std::numeric_limits<long long unsigned>::max());
  PRINTING_CHECK(expected,                  ==, sprintf_, buffer, "%llu", std::numeric_limits<long long unsigned>::max());
#endif
}
