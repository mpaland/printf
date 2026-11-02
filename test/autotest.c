///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2017-2026, PALANDesign Hannover, Germany
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
// \brief Random comparison of this implementation against the printf of the
//        standard library: randomly generated format strings and values are
//        printed by both, mismatches are reported.
//
///////////////////////////////////////////////////////////////////////////////

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../printf.h"

// printf.h maps the standard function names to this implementation, but this
// tool compares against the standard library
#undef printf
#undef sprintf
#undef snprintf
#undef vsnprintf
#undef vprintf


#define BUFFER_SIZE   100
#define WIDTH_MAX     25

// range of the random floating point values
#define FLOAT_MIN     1e-5
#define FLOAT_MAX     1e5

// this implementation computes at most 9 digits after the decimal point
#define PRECISION_LIMIT_DEFAULT 9

#define USAGE \
  "Usage: autotest [OPTION]...\n" \
  "Compare randomly formatted values of this printf with the standard library printf.\n" \
  "Matches are written to stdout, mismatches to stderr.\n" \
  "\n" \
  "  -i          test %%i\n" \
  "  -x          test %%x\n" \
  "  -o          test %%o\n" \
  "  -u          test %%u\n" \
  "  -f          test %%f\n" \
  "  -e          test %%e\n" \
  "  -g          test %%g\n" \
  "  -l          test the '-' flag (left justification)\n" \
  "  -z          test the '0' flag (zero padding)\n" \
  "  -#          test the '#' flag (alternative form)\n" \
  "  -w          test the field width\n" \
  "  -p[LIMIT]   test the precision, limited to LIMIT for %%f %%e %%g (default %d)\n" \
  "  -a          test all of the above\n" \
  "  -n COUNT    stop after COUNT rounds over the selected conversions (default: run until stopped)\n" \
  "  -s SEED     seed of the random generator (default 1)\n" \
  "  -h          show this help\n" \
  "\n" \
  "The exit code is 1 if any mismatch was found.\n" \
  "\n" \
  "Examples:\n" \
  "  autotest -a                   test everything, show all comparisons\n" \
  "  autotest -a 1>/dev/null       test everything, show only the mismatches\n" \
  "  autotest -ap14 -n 1000        1000 rounds with a precision limit of 14 for %%f %%e %%g\n" \
  "  autotest -ixou -n 1000 -s 42  1000 rounds of the integer conversions with seed 42\n"


// the selected options
static struct {
  bool          conversion[128];  // indexed by the conversion character
  bool          left_justify;
  bool          zero_pad;
  bool          hash;
  bool          width;
  bool          precision;
  int           precision_limit;
  unsigned long count;
  unsigned int  seed;
} opts;

static unsigned long comparisons = 0UL;
static unsigned long mismatches  = 0UL;


void _putchar(char character)
{
  (void)character;
}


static bool parse_options(int argc, char* argv[])
{
  bool any = false;

  memset(&opts, 0, sizeof(opts));
  opts.precision_limit = PRECISION_LIMIT_DEFAULT;
  opts.seed            = 1U;

  for (int i = 1; i < argc; i++) {
    const char* arg = argv[i];
    if (arg[0] != '-' || arg[1] == '\0') {
      fprintf(stderr, "Unknown argument '%s'\n", arg);
      return false;
    }
    for (const char* c = arg + 1; *c != '\0'; c++) {
      any = true;
      switch (*c) {
        case 'i': case 'x': case 'o': case 'u':
        case 'f': case 'e': case 'g':
          opts.conversion[(unsigned char)*c] = true;
          break;
        case 'l': opts.left_justify = true; break;
        case 'z': opts.zero_pad     = true; break;
        case '#': opts.hash         = true; break;
        case 'w': opts.width        = true; break;
        case 'p':
          opts.precision = true;
          if (c[1] >= '0' && c[1] <= '9') {
            opts.precision_limit = atoi(c + 1);
            c += strlen(c) - 1;
          }
          break;
        case 'a':
          opts.conversion['i'] = opts.conversion['x'] = opts.conversion['o'] = opts.conversion['u'] = true;
          opts.conversion['f'] = opts.conversion['e'] = opts.conversion['g'] = true;
          opts.left_justify = opts.zero_pad = opts.hash = opts.width = opts.precision = true;
          break;
        case 'n':
        case 's': {
          const char* value = (c[1] != '\0') ? c + 1 : (i + 1 < argc) ? argv[++i] : NULL;
          if (value == NULL) {
            fprintf(stderr, "Option -%c needs a value\n", *c);
            return false;
          }
          if (*c == 'n') {
            opts.count = strtoul(value, NULL, 10);
          }
          else {
            opts.seed = (unsigned int)strtoul(value, NULL, 10);
          }
          c += strlen(c) - 1;
          break;
        }
        case 'h':
          return false;
        default:
          fprintf(stderr, "Unknown option -%c\n", *c);
          return false;
      }
    }
  }
  return any;
}


static double rand_double(double min, double max)
{
  return min + ((double)rand() / (double)RAND_MAX) * (max - min);
}


// build a random format string for the conversion and compare the output of
// both implementations for a random value
static void test_conversion(char conversion)
{
  const bool is_signed   = (conversion == 'i');
  const bool is_float    = (conversion == 'f') || (conversion == 'e') || (conversion == 'g');
  const bool has_hash    = (conversion == 'x') || (conversion == 'o') || is_float;
  char format[BUFFER_SIZE];
  char expected[BUFFER_SIZE];
  char actual[BUFFER_SIZE];
  int  width = 1 + rand() % WIDTH_MAX;

  // flags
  strcpy(format, "%");
  if ((rand() & 1) && opts.left_justify) {
    strcat(format, "-");
  }
  if ((rand() & 1) && opts.hash && has_hash) {
    strcat(format, "#");
  }
  if (is_signed || is_float) {
    if (rand() & 1) {
      strcat(format, "+");
    }
    else if (rand() & 1) {
      strcat(format, " ");
    }
  }

  // width
  if ((rand() & 1) && opts.width) {
    if ((rand() & 1) && opts.zero_pad) {
      strcat(format, "0");
    }
    sprintf(format + strlen(format), "%i", width);
  }

  // precision
  if ((rand() & 1) && opts.precision) {
    const int limit = is_float && (opts.precision_limit < width) ? opts.precision_limit : width;
    sprintf(format + strlen(format), ".%i", 1 + rand() % limit);
  }

  // conversion and value
  const size_t len = strlen(format);
  format[len]     = conversion;
  format[len + 1] = '\0';

  if (is_float) {
    double value = (rand() & 1) ? 0.0 : rand_double(FLOAT_MIN, FLOAT_MAX);
    if (rand() & 1) {
      value = -value;
    }
    sprintf(expected, format, value);
    sprintf_(actual, format, value);
    sprintf(format + strlen(format), "\" value = %.18f", value);
  }
  else if (is_signed) {
    int value = (rand() & 1) ? 0 : rand();
    if (rand() & 1) {
      value = -value;
    }
    sprintf(expected, format, value);
    sprintf_(actual, format, value);
    sprintf(format + strlen(format), "\" value = %i", value);
  }
  else {
    const unsigned int value = (rand() & 1) ? 0U : (unsigned int)rand();
    sprintf(expected, format, value);
    sprintf_(actual, format, value);
    sprintf(format + strlen(format), "\" value = %u", value);
  }

  // report
  const bool match = (strcmp(expected, actual) == 0);
  FILE* const dst = match ? stdout : stderr;
  comparisons++;
  if (!match) {
    mismatches++;
  }
  fprintf(dst, "\nformat = \"%s\n", format);
  fprintf(dst, "libc:     \"%s\"\n", expected);
  fprintf(dst, "this:     \"%s\"\n", actual);
}


int main(int argc, char* argv[])
{
  static const char conversions[] = "ixoufeg";

  if (!parse_options(argc, argv)) {
    printf(USAGE, PRECISION_LIMIT_DEFAULT);
    return 2;
  }

  srand(opts.seed);
  for (unsigned long round = 0UL; (opts.count == 0UL) || (round < opts.count); round++) {
    for (const char* c = conversions; *c != '\0'; c++) {
      if (opts.conversion[(unsigned char)*c]) {
        test_conversion(*c);
      }
    }
  }

  fprintf(stderr, "\n%lu comparisons, %lu mismatches\n", comparisons, mismatches);
  return (mismatches == 0UL) ? 0 : 1;
}
