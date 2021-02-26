#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include <float.h>

//
// Helper macro to record and remove negativity (also works for -0.0)
//
#define _SIGNCHECK(v,n)     if (1.0/v < 0.0) { v = -v; n = 1; } else { n = 0; }

//
// This is actually a fairly efficient implementation than the pow() function as it's
// only power of 10 and doesn't have any edge cases to worry about.
//
double _e10(int exp) {
    static const long pows[7] = { 1, 10, 100, 1000, 10000, 100000, 1000000 };
    double rc = 1;
    if (exp < 0) {
        exp = -exp;
        while (exp > 6) {
            rc /= 1000000;
            exp -= 6;
        }
        rc /= pows[exp];
    } else if (exp > 0) {
        while (exp > 6) {
            rc *= 1000000;
            exp -= 6;
        }
        rc *= pows[exp];
    }
    return rc;
}

// DELME
// output function type
typedef void (*out_fct_type)(char character, void* buffer, size_t idx, size_t maxlen);
#define PRINTF_DEFAULT_FLOAT_PRECISION  6U
// internal flag definitions
#define FLAGS_ZEROPAD   (1U <<  0U)
#define FLAGS_LEFT      (1U <<  1U)
#define FLAGS_PLUS      (1U <<  2U)
#define FLAGS_SPACE     (1U <<  3U)
#define FLAGS_HASH      (1U <<  4U)
#define FLAGS_UPPERCASE (1U <<  5U)
#define FLAGS_CHAR      (1U <<  6U)
#define FLAGS_SHORT     (1U <<  7U)
#define FLAGS_LONG      (1U <<  8U)
#define FLAGS_LONG_LONG (1U <<  9U)
#define FLAGS_PRECISION (1U << 10U)
#define FLAGS_ADAPT_EXP (1U << 11U)

// Make sure you keep these in sync, can't concat the macros, 18 is the limit due to
// limits of (long).
#define MAX_FLOAT_PRECISION     18
#define PRINTF_MAX_FLOAT        1e+18
#define PRINTF_MIN_FLOAT        1e-18

static inline void _out_char(char character, void* buffer, size_t idx, size_t maxlen)
{
  (void)buffer; (void)idx; (void)maxlen;
  if (character) {
    putchar(character);
  }
}

//
// Modified reverse output function that deals with space and zero padding, and sign,
// outside of the actual buffer. Needs to know if the value is negative.
//
static size_t _out_rev2(out_fct_type out, char* buffer, size_t idx, size_t maxlen, const char* buf, size_t len, unsigned int width, unsigned int flags, int negative)
{
    // First figure out the right sign to use...
    int sign=0;
    if (negative) {
        sign='-';
    } else if (flags & FLAGS_PLUS) {
        sign='+';
    } else if (flags & FLAGS_SPACE) {
        sign=' ';
    }
    if (sign) {
        len++;        // so space padding works
    }
    // Now we can output... first padding spaces (if needed) 
    if (!(flags & FLAGS_LEFT) && !(flags & FLAGS_ZEROPAD)) {
        for (int i=len; i < width; i++) {
            out(' ', buffer, idx++, maxlen);
        }
    }
    // Now the sign
    if (sign) {
        out(sign, buffer, idx++,maxlen);
    }
    // Now zeropad (no spaces would have happened)
    if (flags & FLAGS_ZEROPAD) {
        for (int i=len; i < width; i++) {
            out('0', buffer, idx++, maxlen);
        }
    }
    if (sign) {
        len--;        // put back the length
    }
    // The the value
    while(len > 0) {
        out(buf[--len], buffer, idx++, maxlen);
    }
    return idx;
}

//
// Given a float print the scientific notation using a specific number of significant
// digits (not precision), if FLAGS_ADAPT_EXP is set then try to print as a normal
// float first only switch to scientific if we need to.
//
// Changes compared to mpaland/printf routine:
//
// 1. Keeps padding (zero/space) out of internal buffer
// 2. Supports signficant digits up to 18.
// 3. Supports FLAGS_HASH
// 4. Max number length fits in buffer, so no length checking needed
//
static size_t _etoa(out_fct_type out, char* buffer, size_t idx, size_t maxlen, double value, unsigned int prec, unsigned int width, unsigned int flags) {
    char buf[MAX_FLOAT_PRECISION + 1 + 5];  // number, plus decimal, plus e+XXX
    int len = 0;
    int hadsig = 0;         // used for trailing zero removal

    // If we are a normal 'e' type output then we need an extra digit to be
    // consistent with the standard printf
    if (!(flags & FLAGS_ADAPT_EXP)) {
        prec++;
    }
    if (prec > MAX_FLOAT_PRECISION) {
        prec=MAX_FLOAT_PRECISION;
    }
    // Adjust for 0 precision requests...
    if (prec == 0) {
        prec = 1;
    }
    // Handle the various verions of scientific with/without trailing zeros
    if (!(flags & FLAGS_ADAPT_EXP)) {
        // Straight scientific with trailing zeros
        hadsig = 1;
    } else if (value >= _e10(prec) || (value <= 1e-05 && value != 0)) {
        // Too large, so switch to scientific without trailing zeros
        flags &= ~FLAGS_ADAPT_EXP;
    }

    // Store and remove negativity
    int negative;
    _SIGNCHECK(value, negative);

    // Move so we are within the range 0 to <1
    int exp=0;
    long whole = 0;
    while (value < 0.00001 && value > 0) {
        value *= 100000;
        exp -=5;
    }
    while (value < 0.1 && value > 0) {
        value *= 10;
        exp--;
    }
    while (value > 100000) {
        value /= 100000;
        exp += 5;
    }
    while (value >= 1) {
        value /= 10;
        exp++;
    }
    // If we are zero then fix exponential value
    if (value == 0) {
        exp++;
    }
    value *= _e10(prec);
    exp -= prec;
    whole = (long)value;
    if ((value - whole) >= 0.5) whole++;

    // There are some strange rounding issues that cause problems
    if (whole == _e10(prec)) {
        whole /= 10;
        exp++;
    }
    // If we are printing in scientific notation then we need to adjust
    // the exponent accordingly and print out the e value
    if (!(flags & FLAGS_ADAPT_EXP)) {
        int exval = exp + (prec-1);
        int neg = exval < 0;
        exval = (neg ? -exval : exval);

        // Always print the lowest 2 digits (even if zero)
        buf[len++] = '0' + exval%10;
        exval /= 10;
        buf[len++] = '0' + exval%10;
        exval /= 10;
        // Print the third only if we have one
        if (exval) 
            buf[len++] = '0' + exval%10;
        // Print the sign and 'e'
        buf[len++] = (neg ? '-' : '+');
        buf[len++] = (flags & FLAGS_UPPERCASE ? 'E' : 'e');
        exp = -(prec-1);
    }

    // If we have a hash, then we don't strip zeros, and if we are
    // zero (with stripping) then make it easy.
    if (flags & FLAGS_HASH) {
        hadsig=1;
    } else if (whole == 0) {
        if (!hadsig) 
            exp = 0;
    }

    // Now output the trailing zeros for non decimals
    while (exp > 0) {
        buf[len++] = '0';
        exp--;
    }
    if ((flags & FLAGS_HASH) && exp == 0) {
        buf[len++] = '.';
    }
    if (whole == 0 && exp == 0) {
        buf[len++] = '0';
    }
    while (whole || exp < 0) {
        int n = whole%10;
        whole /= 10;
        exp++;
        if (!hadsig && n == 0 && exp < 1) continue;
        hadsig = 1;
        buf[len++] = '0' + n;
        if (exp == 0) {
            buf[len++] = '.';
            if (!whole) {
                buf[len++] = '0';
            }
        } 
    }
    return _out_rev2(out, buffer, idx, maxlen, buf, len, width, flags, negative);
}

//
// Given a float value, turn it into a string with the given number of
// "prec" decimal places. If the number is too large to be calculated
// (MAX_FLOAT) then switch to scientific notation (with 6 sig digs)
//
// Changes compared to mpaland/printf routine:
//
// 1. Keeps padding (zero/space) out of internal buffer, increasing space
//    available for the result.
// 2. Supports precision up to 18 digits (e+18 and e-18)
// 3. Supports FLAGS_HASH
// 4. Max number length fits in buffer, so no length checking needed
//
static size_t _ftoa(out_fct_type out, char* buffer, size_t idx, size_t maxlen, double value, unsigned int prec, unsigned int width, unsigned int flags) {
    char    buf[(2*MAX_FLOAT_PRECISION)+1];
    int     len = 0;

    // Special cases (nan, inf-, inf+)
    if (value != value) {
        return _out_rev2(out, buffer, idx, maxlen, "nan", 3, width, flags, 0);
    }
    if (value < -DBL_MAX) {
        return _out_rev2(out, buffer, idx, maxlen, "fni", 3, width, (flags&~FLAGS_ZEROPAD), 1);
    }
    if (value > DBL_MAX) {
        return _out_rev2(out, buffer, idx, maxlen, "fni", 3, width, (flags&~FLAGS_ZEROPAD), 0);
    }

    if (!(flags & FLAGS_PRECISION)) {
        prec = PRINTF_DEFAULT_FLOAT_PRECISION;
    } else if (prec > MAX_FLOAT_PRECISION) {
        prec=MAX_FLOAT_PRECISION;
    }

    // Store and remove negativity
    int negative;
    _SIGNCHECK(value, negative);

    // Check for very large or very small numbers that might break things
    if (value > PRINTF_MAX_FLOAT || (value < PRINTF_MIN_FLOAT && value != 0)) {
        return _etoa(out, buffer, idx, maxlen, (negative ? -value : value), PRINTF_DEFAULT_FLOAT_PRECISION, width, flags);
    }

    // First we separate the whole and fractioanal parts
    long whole = (long)value;
    double dfrac = value - whole;
    
    // Now deal with fractional precision (and rounding)
    dfrac *= _e10(prec);
    long frac = (long)dfrac;

    if ((dfrac - frac) >= 0.5) {
        frac++;
        if (frac >= _e10(prec)) {
            whole++;
            frac = 0;
        }
    }
    // Build the number output
    if (prec) {
        while (prec--) {
            buf[len++] = '0' + frac%10;
            frac /= 10;
        }
        buf[len++] = '.';
    } else if (flags & FLAGS_HASH) {
        buf[len++] = '.';
    }
    if (!whole) 
        buf[len++] = '0';
    while (whole) {
        buf[len++] = '0' + whole%10;
        whole /= 10;
    }
    return _out_rev2(out, buffer, idx, maxlen, buf, len, width, flags, negative);
}


int main(int argc, char *argv[]) {
    char buf[64];
    char *p = buf;
    int negative = 0;


//    do_sigs(atof(argv[1]), atoi(argv[2]));
    printf("STANDARD OUTPUT (f):  [%*.*f]\n", atoi(argv[2]), atoi(argv[3]), atof(argv[1]));
    printf("STANDARD OUTPUT (e):  [%*.*e]\n", atoi(argv[2]), atoi(argv[3]), atof(argv[1]));
    printf("STANDARD OUTPUT (g):  [%*.*g]\n", atoi(argv[2]), atoi(argv[3]), atof(argv[1]));
    printf("STANDARD OUTPUT (#f): [%#*.*f]\n", atoi(argv[2]), atoi(argv[3]), atof(argv[1]));
    printf("STANDARD OUTPUT (#e): [%#*.*e]\n", atoi(argv[2]), atoi(argv[3]), atof(argv[1]));
    printf("STANDARD OUTPUT (#g): [%#*.*g]\n", atoi(argv[2]), atoi(argv[3]), atof(argv[1]));
    printf("---\n");
    printf("MY OUTPUT (f)      :  [");
    _ftoa(_out_char, buf, 0, 64, atof(argv[1]), atoi(argv[3]), atoi(argv[2]), FLAGS_PRECISION);
    printf("]\n");
    printf("MY OUTPUT (e)      :  [");
    _etoa(_out_char, buf, 0, 64, atof(argv[1]), atoi(argv[3]), atoi(argv[2]), FLAGS_PRECISION);
    printf("]\n");
    printf("MY OUTPUT (g)      :  [");
    _etoa(_out_char, buf, 0, 64, atof(argv[1]), atoi(argv[3]), atoi(argv[2]), FLAGS_PRECISION|FLAGS_ADAPT_EXP);
    printf("]\n");
    printf("MY OUTPUT (#f)      : [");
    _ftoa(_out_char, buf, 0, 64, atof(argv[1]), atoi(argv[3]), atoi(argv[2]), FLAGS_PRECISION|FLAGS_HASH);
    printf("]\n");
    printf("MY OUTPUT (#e)      : [");
    _etoa(_out_char, buf, 0, 64, atof(argv[1]), atoi(argv[3]), atoi(argv[2]), FLAGS_PRECISION|FLAGS_HASH);
    printf("]\n");
    printf("MY OUTPUT (#g)      : [");
    _etoa(_out_char, buf, 0, 64, atof(argv[1]), atoi(argv[3]), atoi(argv[2]), FLAGS_PRECISION|FLAGS_ADAPT_EXP|FLAGS_HASH);
    printf("]\n");

    exit(0);
}
