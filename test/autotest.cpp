
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#ifdef _WIN32
#include "getopt.h"
#else
#include <getopt.h>
#endif
#include <assert.h>

#include "printf_config.h"
#include "../src/printf/printf.h"

#ifndef PRINTF_ALIAS_STANDARD_FUNCTION_NAMES
#define PRINTF_ALIAS_STANDARD_FUNCTION_NAMES 0
#endif

#if PRINTF_ALIAS_STANDARD_FUNCTION_NAMES
# define printf_    printf
# define sprintf_   sprintf
# define vsprintf_  vsprintf
# define snprintf_  snprintf
# define vsnprintf_ vsnprintf
# define vprintf_   vprintf
#endif


//*******************************************************
// Defines
//*******************************************************

  #define min(a,b) ( (a) < (b) ? (a) : (b) )
  #define MAIN_TITLE "autotest"

  #define BUF_SIZE  100
  #define WIDTH_MAX 25

// range for float testing
  #define FLOAT_TST_MIN 1E-5
  #define FLOAT_TST_MAX 1E5

  #define PRECISION_LIMIT_DEFAULT 17

  struct cmdopt_struct
  {
    bool i;
    bool x;
    bool o;
    bool u;
    bool f;
    bool e;
    bool g;
    bool left_justify;
    bool zero_pad;
    bool hash;
    bool width;
    bool prec;
    int prec_max;
  };

//  Valid short options
  #define VALIDOPTS  "ixoufeglz#wp::ah"

  #define MSG_USAGE "\n\
Usage: " MAIN_TITLE " [OPTION/s]\n\
Compare randomly formatted strings with the stdio printf()\n\
Matching strings are output to stdout\n\
Errors are output to stderr\r\n\
\n\
   -i  test %%i \n\
   -x  test %%x\n\
   -o  test %%o\n\
   -u  test %%u\n\
   -f  test %%f\n\
   -e  test %%e\n\
   -g  test %%g (float falling back to exp)\n\
   -l  test left justification\r\n\
   -z  test zero padding\n\
   -#  test prepending base 0 0x\n\
   -w  test width specifier\n\
   -p <limit> test precision specifier, with an optional limit for %%f %%e %%g\n\
   -a  test all of the above, with a default precision limit of %i for %%f %%e %%g\n\
\n\
   -h  show these options\n\
\n\
Examples:\n\
  " MAIN_TITLE " -a                           test with all options, showing all passes and failures\n\
  " MAIN_TITLE " -a 1>/dev/null               test with all options, showing only errors, with stdout > null\n\
  " MAIN_TITLE " -ap14 1>/dev/null            test with all options and precision limit of 14 for %%f %%e %%g, showing only errors, with stdout > null\n\
  " MAIN_TITLE " -ixou 1>/dev/null            test only %%i %%x %%o %%u, showing only errors, with stdout > null\n\
\n\
"

//*******************************************************
// Variables
//*******************************************************

  static struct cmdopt_struct opts;

//*******************************************************
// Prototypes
//*******************************************************

  static bool parse_options(int argc, char *argv[]);
  static void run_tests(void);

  static void test_i(void);
  static void test_x(void);
  static void test_o(void);
  static void test_u(void);
  static void test_f(void);
  static void test_e(void);
  static void test_g(void);

  float rand_float(float a, float b);

//*******************************************************
// Functions
//*******************************************************

void putchar_(char character)
{
  (void)character;
}

int main(int argc, char *argv[])
{
  if(!parse_options(argc, argv))
    printf(MSG_USAGE, PRECISION_LIMIT_DEFAULT);
  else
    run_tests();
  return 0;
}

static bool parse_options(int argc, char *argv[])
{
  char c;
  bool gotopt = false;

  opts.i = false;
  opts.x = false;
  opts.o = false;
  opts.u = false;
  opts.f = false;
  opts.e = false;
  opts.g = false;
  opts.left_justify = false;
  opts.zero_pad = false;
  opts.hash = false;
  opts.width = false;
  opts.prec = false;
  opts.prec_max = -1;

  while ((c = getopt (argc, argv, VALIDOPTS)) != -1)
  {
    gotopt = true;
    if(c == 'i')
      opts.i = true;
    else if(c == 'x')
      opts.x = true;
    else if(c == 'o')
      opts.o = true;
    else if(c == 'u')
      opts.u = true;
    else if(c == 'f')
      opts.f = true;
    else if(c == 'e')
      opts.e = true;
    else if(c == 'g')
      opts.g = true;
    else if(c == 'l')
      opts.left_justify = true;
    else if(c == 'z')
      opts.zero_pad = true;
    else if(c == '#')
      opts.hash = true;
    else if(c == 'w')
      opts.width = true;
    else if(c == 'p')
    {
      opts.prec = true;
      if(optarg)
        opts.prec_max = atoi(optarg);
    }
    else if(c == 'a')
    {
      opts.i = true;
      opts.x = true;
      opts.o = true;
      opts.u = true;
      opts.f = true;
      opts.e = true;
      opts.g = true;
      opts.left_justify = true;
      opts.zero_pad = true;
      opts.hash = true;
      opts.width = true;
      opts.prec = true;
    }
    else if(c != 'h')
    {
      fprintf(stderr, "Unknown option\n");
      assert(false);
    };
  };

  if(opts.prec_max == -1)
    opts.prec_max = PRECISION_LIMIT_DEFAULT;

  return gotopt;
}

static void run_tests(void)
{
  while(true)
  {
    if(opts.i)
      test_i();
    if(opts.x)
      test_x();
    if(opts.o)
      test_o();
    if(opts.u)
      test_u();
    if(opts.f)
      test_f();
    if(opts.e)
      test_e();
    if(opts.g)
      test_g();
  };
}

static void test_i(void)
{
  FILE* dst = stdout;
  char fmt_buf[BUF_SIZE];
  char std_buf[BUF_SIZE];
  char tst_buf[BUF_SIZE];
  bool width_flag;
  int width;
  int value;

  strcpy(fmt_buf, "%");
  if(rand()&1 && opts.left_justify)
    strcat(fmt_buf, "-");
  if(rand()&1)
    strcat(fmt_buf, "+");
  else if(rand()&1)
    strcat(fmt_buf, " ");

  width_flag = (rand()&1 && opts.width);

  width = 1+rand()%WIDTH_MAX;
  if(width_flag)
  {
    if(rand()&1 && opts.zero_pad)
      strcat(fmt_buf, "0");
    sprintf(&fmt_buf[strlen(fmt_buf)], "%i", width);
  };

  if(rand()&1 && opts.prec)
  {
    strcat(fmt_buf, ".");
    sprintf(&fmt_buf[strlen(fmt_buf)], "%i", 1+rand()%width);
  };

  strcat(fmt_buf, "i");
  value = rand();
  if(rand()&1)
    value = 0;
  if(rand()&1)
    value*=-1;

  sprintf(std_buf, fmt_buf, value);
  sprintf_(tst_buf, fmt_buf, value);

  if(strcmp(std_buf,tst_buf))
    dst = stderr;

  fprintf(dst, "\nfmt = \"%s\" value = %i\n", fmt_buf, value);
  fprintf(dst, "gnu = \"%s\"\n", std_buf);
  fprintf(dst, "mpa = \"%s\"\n", tst_buf);
}

static void test_x(void)
{
  FILE* dst = stdout;
  char fmt_buf[BUF_SIZE];
  char std_buf[BUF_SIZE];
  char tst_buf[BUF_SIZE];
  bool width_flag;
  int width;
  unsigned int value;

  strcpy(fmt_buf, "%");
  if(rand()&1 && opts.left_justify)
    strcat(fmt_buf, "-");
  if(rand()&1 && opts.hash)
    strcat(fmt_buf, "#");

  width_flag = (rand()&1 && opts.width);

  width = 1+rand()%WIDTH_MAX;
  if(width_flag)
  {
    if(rand()&1 && opts.zero_pad)
      strcat(fmt_buf, "0");
    sprintf(&fmt_buf[strlen(fmt_buf)], "%i", width);
  };

  if(rand()&1 && opts.prec)
  {
    strcat(fmt_buf, ".");
    sprintf(&fmt_buf[strlen(fmt_buf)], "%i", 1+rand()%width);
  };

  strcat(fmt_buf, "x");
  value = (unsigned)rand();
  if(rand()&1)
    value = 0;

  sprintf(std_buf, fmt_buf, value);
  sprintf_(tst_buf, fmt_buf, value);

  if(strcmp(std_buf,tst_buf))
    dst = stderr;

  fprintf(dst, "\nfmt = \"%s\" value = %#x\n", fmt_buf, value);
  fprintf(dst, "gnu = \"%s\"\n", std_buf);
  fprintf(dst, "mpa = \"%s\"\n", tst_buf);
}

static void test_o(void)
{
  FILE* dst = stdout;
  char fmt_buf[BUF_SIZE];
  char std_buf[BUF_SIZE];
  char tst_buf[BUF_SIZE];
  bool width_flag;
  int width;
  unsigned int value;

  strcpy(fmt_buf, "%");
  if(rand()&1 && opts.left_justify)
    strcat(fmt_buf, "-");
  if(rand()&1 && opts.hash)
    strcat(fmt_buf, "#");

  width_flag = (rand()&1 && opts.width);

  width = 1+rand()%WIDTH_MAX;
  if(width_flag)
  {
    if(rand()&1 && opts.zero_pad)
      strcat(fmt_buf, "0");
    sprintf(&fmt_buf[strlen(fmt_buf)], "%i", width);
  };

  if(rand()&1 && opts.prec)
  {
    strcat(fmt_buf, ".");
    sprintf(&fmt_buf[strlen(fmt_buf)], "%i", 1+rand()%width);
  };

  strcat(fmt_buf, "o");
  value = (unsigned)rand();
  if(rand()&1)
    value = 0;

  sprintf(std_buf, fmt_buf, value);
  sprintf_(tst_buf, fmt_buf, value);

  if(strcmp(std_buf,tst_buf))
    dst = stderr;

  fprintf(dst, "\nfmt = \"%s\" value = %o\n", fmt_buf, value);
  fprintf(dst, "gnu = \"%s\"\n", std_buf);
  fprintf(dst, "mpa = \"%s\"\n", tst_buf);
}

static void test_u(void)
{
  FILE* dst = stdout;
  char fmt_buf[BUF_SIZE];
  char std_buf[BUF_SIZE];
  char tst_buf[BUF_SIZE];
  bool width_flag;
  int width;
  unsigned int value;

  strcpy(fmt_buf, "%");
  if(rand()&1 && opts.left_justify)
    strcat(fmt_buf, "-");

  width_flag = (rand()&1 && opts.width);

  width = 1+rand()%WIDTH_MAX;
  if(width_flag)
  {
    if(rand()&1 && opts.zero_pad)
      strcat(fmt_buf, "0");
    sprintf(&fmt_buf[strlen(fmt_buf)], "%i", width);
  };

  if(rand()&1 && opts.prec)
  {
    strcat(fmt_buf, ".");
    sprintf(&fmt_buf[strlen(fmt_buf)], "%i", 1+rand()%width);
  };

  strcat(fmt_buf, "u");
  value = (unsigned)rand();
  if(rand()&1)
    value = 0;

  sprintf(std_buf, fmt_buf, value);
  sprintf_(tst_buf, fmt_buf, value);

  if(strcmp(std_buf,tst_buf))
    dst = stderr;

  fprintf(dst, "\nfmt = \"%s\" value = %u\n", fmt_buf, value);
  fprintf(dst, "gnu = \"%s\"\n", std_buf);
  fprintf(dst, "mpa = \"%s\"\n", tst_buf);
}

static void test_f(void)
{
  FILE* dst = stdout;
  char fmt_buf[BUF_SIZE];
  char std_buf[BUF_SIZE];
  char tst_buf[BUF_SIZE];
  bool width_flag;
  int width;
  double value;

  strcpy(fmt_buf, "%");
  if(rand()&1 && opts.left_justify)
    strcat(fmt_buf, "-");
  if(rand()&1 && opts.hash)
    strcat(fmt_buf, "#");
  if(rand()&1)
    strcat(fmt_buf, "+");
  else if(rand()&1)
    strcat(fmt_buf, " ");

  width_flag = (rand()&1 && opts.width);

  width = 1+rand()%WIDTH_MAX;
  if(width_flag)
  {
    if(rand()&1 && opts.zero_pad)
      strcat(fmt_buf, "0");
    sprintf(&fmt_buf[strlen(fmt_buf)], "%i", width);
  };

  if(rand()&1 && opts.prec)
  {
    strcat(fmt_buf, ".");
    sprintf(&fmt_buf[strlen(fmt_buf)], "%i", 1+rand()%min(width, opts.prec_max));
  };

  strcat(fmt_buf, "f");

   if(rand()&1)
    value = 0;
  else
    value = (double) rand_float(FLOAT_TST_MIN, FLOAT_TST_MAX);

  if(rand()&1)
    value*=-1;

  sprintf(std_buf, fmt_buf, value);
  sprintf_(tst_buf, fmt_buf, value);

  if(strcmp(std_buf,tst_buf))
    dst = stderr;

  fprintf(dst, "\nfmt = \"%s\" value = %.18f\n", fmt_buf, value);
  fprintf(dst, "gnu = \"%s\"\n", std_buf);
  fprintf(dst, "mpa = \"%s\"\n", tst_buf);
}

static void test_e(void)
{
  FILE* dst = stdout;
  char fmt_buf[BUF_SIZE];
  char std_buf[BUF_SIZE];
  char tst_buf[BUF_SIZE];
  bool width_flag;
  int width;
  double value;

  strcpy(fmt_buf, "%");
  if(rand()&1 && opts.left_justify)
    strcat(fmt_buf, "-");
  if(rand()&1 && opts.hash)
    strcat(fmt_buf, "#");
  if(rand()&1)
    strcat(fmt_buf, "+");
  else if(rand()&1)
    strcat(fmt_buf, " ");

  width_flag = (rand()&1 && opts.width);

  width = 1+rand()%WIDTH_MAX;
  if(width_flag)
  {
    if(rand()&1 && opts.zero_pad)
      strcat(fmt_buf, "0");
    sprintf(&fmt_buf[strlen(fmt_buf)], "%i", width);
  };

  if(rand()&1 && opts.prec)
  {
    strcat(fmt_buf, ".");
    sprintf(&fmt_buf[strlen(fmt_buf)], "%i", 1+rand()%min(width,opts.prec_max));
  };

  strcat(fmt_buf, "e");

   if(rand()&1)
    value = 0;
  else
    value = (double) rand_float(FLOAT_TST_MIN, FLOAT_TST_MAX);

  if(rand()&1)
    value*=-1;

  sprintf(std_buf, fmt_buf, value);
  sprintf_(tst_buf, fmt_buf, value);

  if(strcmp(std_buf,tst_buf))
    dst = stderr;

  fprintf(dst, "\nfmt = \"%s\" value = %.18f\n", fmt_buf, value);
  fprintf(dst, "gnu = \"%s\"\n", std_buf);
  fprintf(dst, "mpa = \"%s\"\n", tst_buf);
}

static void test_g(void)
{
  FILE* dst = stdout;
  char fmt_buf[BUF_SIZE];
  char std_buf[BUF_SIZE];
  char tst_buf[BUF_SIZE];
  bool width_flag;
  int width;
  double value;

  strcpy(fmt_buf, "%");
  if(rand()&1 && opts.left_justify)
    strcat(fmt_buf, "-");
  if(rand()&1 && opts.hash)
    strcat(fmt_buf, "#");
  if(rand()&1)
    strcat(fmt_buf, "+");
  else if(rand()&1)
    strcat(fmt_buf, " ");

  width_flag = (rand()&1 && opts.width);

  width = 1+rand()%WIDTH_MAX;
  if(width_flag)
  {
    if(rand()&1 && opts.zero_pad)
      strcat(fmt_buf, "0");
    sprintf(&fmt_buf[strlen(fmt_buf)], "%i", width);
  };

  if(rand()&1 && opts.prec)
  {
    strcat(fmt_buf, ".");
    sprintf(&fmt_buf[strlen(fmt_buf)], "%i", 1+rand()%min(width,opts.prec_max));
  };

  strcat(fmt_buf, "g");

   if(rand()&1)
    value = 0;
  else
    value = (double) rand_float(FLOAT_TST_MIN, FLOAT_TST_MAX);

  if(rand()&1)
    value*=-1;

  sprintf(std_buf, fmt_buf, value);
  sprintf_(tst_buf, fmt_buf, value);

  if(strcmp(std_buf,tst_buf))
    dst = stderr;

  fprintf(dst, "\nfmt = \"%s\" value = %.18f\n", fmt_buf, value);
  fprintf(dst, "gnu = \"%s\"\n", std_buf);
  fprintf(dst, "mpa = \"%s\"\n", tst_buf);
}

float rand_float(float a, float b)
{
  float random = ((float) rand()) / (float) RAND_MAX;
  float diff = b - a;
  float r = random * diff;
  return a + r;
}
