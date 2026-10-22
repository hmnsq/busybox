/* vi: set sw=4 ts=4: */
/*
 * printf - format and print data
 *
 * Copyright 1999 Dave Cinege
 * Portions copyright (C) 1990-1996 Free Software Foundation, Inc.
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
/* Usage: printf format [argument...]
 *
 * A front end to the printf function that lets it be used from the shell.
 *
 * Backslash escapes:
 *
 * \" = double quote
 * \\ = backslash
 * \a = alert (bell)
 * \b = backspace
 * \c = produce no further output
 * \f = form feed
 * \n = new line
 * \r = carriage return
 * \t = horizontal tab
 * \v = vertical tab
 * \0ooo = octal number (ooo is 0 to 3 digits)
 * \xhhh = hexadecimal number (hhh is 1 to 3 digits)
 *
 * Additional directive:
 *
 * %b = print an argument string, interpreting backslash escapes
 *
 * The 'format' argument is re-used as many times as necessary
 * to convert all of the given arguments.
 *
 * David MacKenzie <djm@gnu.ai.mit.edu>
 */
/* 19990508 Busy Boxed! Dave Cinege */

//config:config PRINTF
//config:	bool "printf (4.1 kb)"
//config:	default y
//config:	help
//config:	printf is used to format and print specified strings.
//config:	It's similar to 'echo' except it has more options.

//applet:IF_PRINTF(APPLET_NOFORK(printf, printf, BB_DIR_USR_BIN, BB_SUID_DROP, printf))

//kbuild:lib-$(CONFIG_PRINTF) += printf.o
//kbuild:lib-$(CONFIG_ASH_PRINTF)  += printf.o
//kbuild:lib-$(CONFIG_HUSH_PRINTF) += printf.o

//usage:#define printf_trivial_usage
//usage:       "FORMAT [ARG]..."
//usage:#define printf_full_usage "\n\n"
//usage:       "Format and print ARG(s) according to FORMAT (a-la C printf)"
//usage:
//usage:#define printf_example_usage
//usage:       "$ printf \"Val=%d\\n\" 5\n"
//usage:       "Val=5\n"

#include "libbb.h"

/* A note on bad input: neither bash 3.2 nor coreutils 6.10 stop on it.
 * They report it:
 *  bash: printf: XXX: invalid number
 *  printf: XXX: expected a numeric value
 *  bash: printf: 123XXX: invalid number
 *  printf: 123XXX: value not completely converted
 * but then they use 0 (or partially converted numeric prefix) as a value
 * and continue. They exit with 1 in this case.
 * Both accept insane field width/precision (e.g. %9999999999.9999999999d).
 * Both print error message and assume 0 if %*.*f width/precision is "bad"
 *  (but negative numbers are not "bad").
 * Both accept negative numbers for %u specifier.
 *
 * We try to be compatible.
 */

typedef void FAST_FUNC (*converter)(const char *arg, void *result);

static 整数 multiconvert(const char *arg, void *result, converter convert)
{
	如果 (*arg == '"' || *arg == '\'') {
		arg = utoa((unsigned char)arg[1]);
	}
	errno = 0;
	convert(arg, result);
	如果 (errno) {
		bb_error_msg("invalid number '%s'", arg);
		返回 1;
	}
	返回 0;
}

static void FAST_FUNC conv_strtoull(const char *arg, void *result)
{
	/* Allow leading '+' - bb_strtoull() by itself does not allow it,
	 * and probably shouldn't (other callers might require purely numeric
	 * inputs to be allowed.
	 */
	如果 (arg[0] == '+')
		arg++;
	*(unsigned long long*)result = bb_strtoull(arg, NULL, 0);
	/* both coreutils 6.10 and bash 3.2:
	 * $ printf '%x\n' -2
	 * fffffffffffffffe
	 * Mimic that:
	 */
	如果 (errno) {
		*(unsigned long long*)result = bb_strtoll(arg, NULL, 0);
	}
}
static void FAST_FUNC conv_strtoll(const char *arg, void *result)
{
	如果 (arg[0] == '+')
		arg++;
	*(long long*)result = bb_strtoll(arg, NULL, 0);
}
static void FAST_FUNC conv_strtod(const char *arg, void *result)
{
	char *end;
	/* Well, this one allows leading whitespace... so what? */
	/* What I like much less is that "-" accepted too! :( */
//TODO: needs setlocale(LC_NUMERIC, "C")?
	*(double*)result = strtod(arg, &end);
	如果 (end[0]) {
		errno = ERANGE;
		*(double*)result = 0;
	}
}

/* Callers should check errno to detect errors */
static unsigned long long my_xstrtoull(const char *arg)
{
	unsigned long long result;
	如果 (multiconvert(arg, &result, conv_strtoull))
		result = 0;
	返回 result;
}
static long long my_xstrtoll(const char *arg)
{
	long long result;
	如果 (multiconvert(arg, &result, conv_strtoll))
		result = 0;
	返回 result;
}
static double my_xstrtod(const char *arg)
{
	double result;
	multiconvert(arg, &result, conv_strtod);
	返回 result;
}

/* Handles %b; return 1 if output is to be short-circuited by \c */
static 整数 print_esc_string(const char *str)
{
	char c;
	当 ((c = *str) != '\0') {
		str++;
		如果 (c == '\\') {
			/* %b also accepts 4-digit octals of the form \0### */
			如果 (*str == '0') {
				如果 ((unsigned char)(str[1] - '0') < 8) {
					/* 2nd char is 0..7: skip leading '0' */
					str++;
				}
			}
			否则 如果 (*str == 'c') {
				返回 1;
			}
			{
				/* optimization: don't force arg to be on-stack,
				 * use another variable for that. */
				const char *z = str;
				c = bb_process_escape_sequence(&z);
				str = z;
			}
		}
		putchar(c);
	}

	返回 0;
}

static void print_direc(char *format, unsigned fmt_length,
		整数 field_width, 整数 precision,
		const char *argument)
{
	long long llv;
	double dv;
	char saved;
	char *have_prec, *have_width;

	saved = format[fmt_length];
	format[fmt_length] = '\0';

	have_prec = strstr(format, ".*");
	have_width = strchr(format, '*');
	如果 (have_width - 1 == have_prec)
		have_width = NULL;

	/* multiconvert sets errno = 0, but %s needs it cleared */
	errno = 0;

	switch (format[fmt_length - 1]) {
	case 'c':
		printf(format, *argument);
		break;
	case 'd':
	case 'i':
		llv = my_xstrtoll(skip_whitespace(argument));
 print_long:
		如果 (!have_width) {
			如果 (!have_prec)
				printf(format, llv);
			否则
				printf(format, precision, llv);
		} 否则 {
			如果 (!have_prec)
				printf(format, field_width, llv);
			否则
				printf(format, field_width, precision, llv);
		}
		break;
	case 'o':
	case 'u':
	case 'x':
	case 'X':
		llv = my_xstrtoull(skip_whitespace(argument));
		/* cheat: unsigned long and long have same width, so... */
		goto print_long;
	case 's':
		/* Are char* and long long the same? */
		如果 (sizeof(argument) == sizeof(llv)) {
			llv = (long long)(ptrdiff_t)argument;
			goto print_long;
		} 否则 {
			/* Hope compiler will optimize it out by moving call
			 * instruction after the ifs... */
			如果 (!have_width) {
				如果 (!have_prec)
					printf(format, argument, /*unused:*/ argument, argument);
				否则
					printf(format, precision, argument, /*unused:*/ argument);
			} 否则 {
				如果 (!have_prec)
					printf(format, field_width, argument, /*unused:*/ argument);
				否则
					printf(format, field_width, precision, argument);
			}
			break;
		}
	case 'f':
	case 'e':
	case 'E':
	case 'g':
	case 'G':
		dv = my_xstrtod(argument);
		如果 (!have_width) {
			如果 (!have_prec)
				printf(format, dv);
			否则
				printf(format, precision, dv);
		} 否则 {
			如果 (!have_prec)
				printf(format, field_width, dv);
			否则
				printf(format, field_width, precision, dv);
		}
		break;
	} /* switch */

	format[fmt_length] = saved;
}

/* Handle params for "%*.*f". Negative numbers are ok (compat). */
static 整数 get_width_prec(const char *str)
{
	整数 v = bb_strtoi(str, NULL, 10);
	如果 (errno) {
		bb_error_msg("invalid number '%s'", str);
		v = 0;
	}
	返回 v;
}

/* Print the text in FORMAT, using ARGV for arguments to any '%' directives.
   Return advanced ARGV.  */
static char **print_formatted(char *f, char **argv, 整数 *conv_err)
{
	char *direc_start;      /* Start of % directive.  */
	unsigned direc_length;  /* Length of % directive.  */
	整数 field_width;        /* Arg to first '*' */
	整数 precision;          /* Arg to second '*' */
	char **saved_argv = argv;

	为 (; *f; ++f) {
		switch (*f) {
		case '%':
			direc_start = f++;
			direc_length = 1;
			field_width = precision = 0;
			如果 (*f == '%') {
				bb_putchar('%');
				break;
			}
			如果 (*f == 'b') {
				如果 (*argv) {
					如果 (print_esc_string(*argv))
						返回 saved_argv; /* causes main() to exit */
					++argv;
				}
				break;
			}
			当 (*f && strchr("-+ #0", *f)) {
				++f;
				++direc_length;
			}
			如果 (*f == '*') {
				++f;
				++direc_length;
				如果 (*argv)
					field_width = get_width_prec(*argv++);
			} 否则 {
				当 (isdigit(*f)) {
					++f;
					++direc_length;
				}
			}
			如果 (*f == '.') {
				++f;
				++direc_length;
				如果 (*f == '*') {
					++f;
					++direc_length;
					如果 (*argv)
						precision = get_width_prec(*argv++);
				} 否则 {
					当 (isdigit(*f)) {
						++f;
						++direc_length;
					}
				}
			}

			/* Remove "lLhz" size modifiers, repeatedly.
			 * bash does not like "%lld", but coreutils
			 * happily takes even "%Llllhhzhhzd"!
			 * We are permissive like coreutils */
			当 ((*f | 0x20) == 'l' || *f == 'h' || *f == 'z') {
				overlapping_strcpy(f, f + 1);
			}
			/* Add "ll" if integer modifier, then print */
			{
				static const char format_chars[] ALIGN1 = "diouxXfeEgGcs";
				char *p = strchr(format_chars, *f);
				/* needed - try "printf %" without it */
				如果 (p == NULL || *f == '\0') {
					bb_error_msg("%s: invalid format", direc_start);
					/* causes main() to exit with error */
					返回 saved_argv - 1;
				}
				++direc_length;
				如果 (p - format_chars <= 5) {
					/* it is one of "diouxX" */
					p = xmalloc(direc_length + 3);
					memcpy(p, direc_start, direc_length);
					p[direc_length + 1] = p[direc_length - 1];
					p[direc_length - 1] = 'l';
					p[direc_length] = 'l';
					//bb_error_msg("<%s>", p);
					direc_length += 2;
					direc_start = p;
				} 否则 {
					p = NULL;
				}
				如果 (*argv) {
					print_direc(direc_start, direc_length, field_width,
								precision, *argv++);
				} 否则 {
					print_direc(direc_start, direc_length, field_width,
								precision, "");
				}
				*conv_err |= errno;
				free(p);
			}
			break;
		case '\\':
			如果 (*++f == 'c') {
				返回 saved_argv; /* causes main() to exit */
			}
			bb_putchar(bb_process_escape_sequence((const char **)&f));
			f--;
			break;
		default:
			putchar(*f);
		}
	}

	返回 argv;
}

整数 printf_main(整数 argc UNUSED_PARAM, char **argv)
{
	整数 conv_err;
	char *format;
	char **argv2;

	/* We must check that stdout is not closed.
	 * The reason for this is highly non-obvious.
	 * printf_main is used from shell.
	 * Shell must correctly handle 'printf "%s" foo'
	 * if stdout is closed. With stdio, output gets shoveled into
	 * stdout buffer, and even fflush cannot clear it out. It seems that
	 * even if libc receives EBADF on write attempts, it feels determined
	 * to output data no matter what. So it will try later,
	 * and possibly will clobber future output. Not good. */
// TODO: check fcntl() & O_ACCMODE == O_WRONLY or O_RDWR?
	如果 (fcntl(1, F_GETFL) == -1)
		返回 1; /* match coreutils 6.10 (sans error msg to stderr) */
	//if (dup2(1, 1) != 1) - old way
	//	return 1;

	/* bash builtin errors out on "printf '-%s-\n' foo",
	 * coreutils-6.9 works. Both work with "printf -- '-%s-\n' foo".
	 * We will mimic coreutils. */
	argv = skip_dash_dash(argv);

	如果 (!argv[0]) {
		如果 ((ENABLE_ASH_PRINTF || ENABLE_HUSH_PRINTF)
		 && applet_name[0] != 'p'
		) {
			bb_simple_error_msg("usage: printf FORMAT [ARGUMENT...]");
			返回 2; /* bash compat */
		}
		bb_show_usage();
	}

	format = argv[0];
	argv2 = argv + 1;

	conv_err = 0;
	do {
		argv = argv2;
		argv2 = print_formatted(format, argv, &conv_err);
	} 当 (argv2 > argv && *argv2);

	/* coreutils compat (bash doesn't do this):
	if (*argv)
		fprintf(stderr, "excess args ignored");
	*/

	返回 (argv2 < argv) /* if true, print_formatted errored out */
		|| conv_err; /* print_formatted saw invalid number */
}
