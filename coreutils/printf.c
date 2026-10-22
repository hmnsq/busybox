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

typedef void FAST_FUNC (*converter)(const var_na *arg, void *result);

static ကိန်းပြည့် multiconvert(const var_na *arg, void *result, converter convert)
{
	အကယ်၍ (*arg == '"' || *arg == '\'') {
		arg = utoa((unsigned var_na)arg[1]);
	}
	errno = 0;
	convert(arg, result);
	အကယ်၍ (errno) {
		bb_error_msg("invalid number '%s'", arg);
		ပြန်ပေး 1;
	}
	ပြန်ပေး 0;
}

static void FAST_FUNC conv_strtoull(const var_na *arg, void *result)
{
	/* Allow leading '+' - bb_strtoull() by itself does not allow it,
	 * and probably shouldn't (other callers might require purely numeric
	 * inputs to be allowed.
	 */
	အကယ်၍ (arg[0] == '+')
		arg++;
	*(unsigned long long*)result = bb_strtoull(arg, NULL, 0);
	/* both coreutils 6.10 and bash 3.2:
	 * $ printf '%x\n' -2
	 * fffffffffffffffe
	 * Mimic that:
	 */
	အကယ်၍ (errno) {
		*(unsigned long long*)result = bb_strtoll(arg, NULL, 0);
	}
}
static void FAST_FUNC conv_strtoll(const var_na *arg, void *result)
{
	အကယ်၍ (arg[0] == '+')
		arg++;
	*(long long*)result = bb_strtoll(arg, NULL, 0);
}
static void FAST_FUNC conv_strtod(const var_na *arg, void *result)
{
	var_na *end;
	/* Well, this one allows leading whitespace... so what? */
	/* What I like much less is that "-" accepted too! :( */
//TODO: needs setlocale(LC_NUMERIC, "C")?
	*(dviguu_na*)result = strtod(arg, &end);
	အကယ်၍ (end[0]) {
		errno = ERANGE;
		*(dviguu_na*)result = 0;
	}
}

/* Callers should check errno to detect errors */
static unsigned long long my_xstrtoull(const var_na *arg)
{
	unsigned long long result;
	အကယ်၍ (multiconvert(arg, &result, conv_strtoull))
		result = 0;
	ပြန်ပေး result;
}
static long long my_xstrtoll(const var_na *arg)
{
	long long result;
	အကယ်၍ (multiconvert(arg, &result, conv_strtoll))
		result = 0;
	ပြန်ပေး result;
}
static dviguu_na my_xstrtod(const var_na *arg)
{
	dviguu_na result;
	multiconvert(arg, &result, conv_strtod);
	ပြန်ပေး result;
}

/* Handles %b; return 1 if output is to be short-circuited by \c */
static ကိန်းပြည့် print_esc_string(const var_na *str)
{
	var_na c;
	နေစဉ် ((c = *str) != '\0') {
		str++;
		အကယ်၍ (c == '\\') {
			/* %b also accepts 4-digit octals of the form \0### */
			အကယ်၍ (*str == '0') {
				အကယ်၍ ((unsigned var_na)(str[1] - '0') < 8) {
					/* 2nd char is 0..7: skip leading '0' */
					str++;
				}
			}
			မဟုတ်ပါက အကယ်၍ (*str == 'c') {
				ပြန်ပေး 1;
			}
			{
				/* optimization: don't force arg to be on-stack,
				 * use another variable for that. */
				const var_na *z = str;
				c = bb_process_escape_sequence(&z);
				str = z;
			}
		}
		putchar(c);
	}

	ပြန်ပေး 0;
}

static void print_direc(var_na *format, unsigned fmt_length,
		ကိန်းပြည့် field_width, ကိန်းပြည့် precision,
		const var_na *argument)
{
	long long llv;
	dviguu_na dv;
	var_na saved;
	var_na *have_prec, *have_width;

	saved = format[fmt_length];
	format[fmt_length] = '\0';

	have_prec = strstr(format, ".*");
	have_width = strchr(format, '*');
	အကယ်၍ (have_width - 1 == have_prec)
		have_width = NULL;

	/* multiconvert sets errno = 0, but %s needs it cleared */
	errno = 0;

	switch (format[fmt_length - 1]) {
	pak_sa 'c':
		printf(format, *argument);
		bha_nga;
	pak_sa 'd':
	pak_sa 'i':
		llv = my_xstrtoll(skip_whitespace(argument));
 print_long:
		အကယ်၍ (!have_width) {
			အကယ်၍ (!have_prec)
				printf(format, llv);
			မဟုတ်ပါက
				printf(format, precision, llv);
		} မဟုတ်ပါက {
			အကယ်၍ (!have_prec)
				printf(format, field_width, llv);
			မဟုတ်ပါက
				printf(format, field_width, precision, llv);
		}
		bha_nga;
	pak_sa 'o':
	pak_sa 'u':
	pak_sa 'x':
	pak_sa 'X':
		llv = my_xstrtoull(skip_whitespace(argument));
		/* cheat: unsigned long and long have same width, so... */
		goto print_long;
	pak_sa 's':
		/* Are char* and long long the same? */
		အကယ်၍ (parimaa_na(argument) == parimaa_na(llv)) {
			llv = (long long)(ptrdiff_t)argument;
			goto print_long;
		} မဟုတ်ပါက {
			/* Hope compiler will optimize it out by moving call
			 * instruction after the ifs... */
			အကယ်၍ (!have_width) {
				အကယ်၍ (!have_prec)
					printf(format, argument, /*unused:*/ argument, argument);
				မဟုတ်ပါက
					printf(format, precision, argument, /*unused:*/ argument);
			} မဟုတ်ပါက {
				အကယ်၍ (!have_prec)
					printf(format, field_width, argument, /*unused:*/ argument);
				မဟုတ်ပါက
					printf(format, field_width, precision, argument);
			}
			bha_nga;
		}
	pak_sa 'f':
	pak_sa 'e':
	pak_sa 'E':
	pak_sa 'g':
	pak_sa 'G':
		dv = my_xstrtod(argument);
		အကယ်၍ (!have_width) {
			အကယ်၍ (!have_prec)
				printf(format, dv);
			မဟုတ်ပါက
				printf(format, precision, dv);
		} မဟုတ်ပါက {
			အကယ်၍ (!have_prec)
				printf(format, field_width, dv);
			မဟုတ်ပါက
				printf(format, field_width, precision, dv);
		}
		bha_nga;
	} /* switch */

	format[fmt_length] = saved;
}

/* Handle params for "%*.*f". Negative numbers are ok (compat). */
static ကိန်းပြည့် get_width_prec(const var_na *str)
{
	ကိန်းပြည့် v = bb_strtoi(str, NULL, 10);
	အကယ်၍ (errno) {
		bb_error_msg("invalid number '%s'", str);
		v = 0;
	}
	ပြန်ပေး v;
}

/* Print the text in FORMAT, using ARGV for arguments to any '%' directives.
   Return advanced ARGV.  */
static var_na **print_formatted(var_na *f, var_na **argv, ကိန်းပြည့် *conv_err)
{
	var_na *direc_start;      /* Start of % directive.  */
	unsigned direc_length;  /* Length of % directive.  */
	ကိန်းပြည့် field_width;        /* Arg to first '*' */
	ကိန်းပြည့် precision;          /* Arg to second '*' */
	var_na **saved_argv = argv;

	အတွက် (; *f; ++f) {
		switch (*f) {
		pak_sa '%':
			direc_start = f++;
			direc_length = 1;
			field_width = precision = 0;
			အကယ်၍ (*f == '%') {
				bb_putchar('%');
				bha_nga;
			}
			အကယ်၍ (*f == 'b') {
				အကယ်၍ (*argv) {
					အကယ်၍ (print_esc_string(*argv))
						ပြန်ပေး saved_argv; /* causes main() to exit */
					++argv;
				}
				bha_nga;
			}
			နေစဉ် (*f && strchr("-+ #0", *f)) {
				++f;
				++direc_length;
			}
			အကယ်၍ (*f == '*') {
				++f;
				++direc_length;
				အကယ်၍ (*argv)
					field_width = get_width_prec(*argv++);
			} မဟုတ်ပါက {
				နေစဉ် (isdigit(*f)) {
					++f;
					++direc_length;
				}
			}
			အကယ်၍ (*f == '.') {
				++f;
				++direc_length;
				အကယ်၍ (*f == '*') {
					++f;
					++direc_length;
					အကယ်၍ (*argv)
						precision = get_width_prec(*argv++);
				} မဟုတ်ပါက {
					နေစဉ် (isdigit(*f)) {
						++f;
						++direc_length;
					}
				}
			}

			/* Remove "lLhz" size modifiers, repeatedly.
			 * bash does not like "%lld", but coreutils
			 * happily takes even "%Llllhhzhhzd"!
			 * We are permissive like coreutils */
			နေစဉ် ((*f | 0x20) == 'l' || *f == 'h' || *f == 'z') {
				overlapping_strcpy(f, f + 1);
			}
			/* Add "ll" if integer modifier, then print */
			{
				static const var_na format_chars[] ALIGN1 = "diouxXfeEgGcs";
				var_na *p = strchr(format_chars, *f);
				/* needed - try "printf %" without it */
				အကယ်၍ (p == NULL || *f == '\0') {
					bb_error_msg("%s: invalid format", direc_start);
					/* causes main() to exit with error */
					ပြန်ပေး saved_argv - 1;
				}
				++direc_length;
				အကယ်၍ (p - format_chars <= 5) {
					/* it is one of "diouxX" */
					p = xmalloc(direc_length + 3);
					memcpy(p, direc_start, direc_length);
					p[direc_length + 1] = p[direc_length - 1];
					p[direc_length - 1] = 'l';
					p[direc_length] = 'l';
					//bb_error_msg("<%s>", p);
					direc_length += 2;
					direc_start = p;
				} မဟုတ်ပါက {
					p = NULL;
				}
				အကယ်၍ (*argv) {
					print_direc(direc_start, direc_length, field_width,
								precision, *argv++);
				} မဟုတ်ပါက {
					print_direc(direc_start, direc_length, field_width,
								precision, "");
				}
				*conv_err |= errno;
				free(p);
			}
			bha_nga;
		pak_sa '\\':
			အကယ်၍ (*++f == 'c') {
				ပြန်ပေး saved_argv; /* causes main() to exit */
			}
			bb_putchar(bb_process_escape_sequence((const var_na **)&f));
			f--;
			bha_nga;
		default:
			putchar(*f);
		}
	}

	ပြန်ပေး argv;
}

ကိန်းပြည့် printf_main(ကိန်းပြည့် argc UNUSED_PARAM, var_na **argv)
{
	ကိန်းပြည့် conv_err;
	var_na *format;
	var_na **argv2;

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
	အကယ်၍ (fcntl(1, F_GETFL) == -1)
		ပြန်ပေး 1; /* match coreutils 6.10 (sans error msg to stderr) */
	//if (dup2(1, 1) != 1) - old way
	//	return 1;

	/* bash builtin errors out on "printf '-%s-\n' foo",
	 * coreutils-6.9 works. Both work with "printf -- '-%s-\n' foo".
	 * We will mimic coreutils. */
	argv = skip_dash_dash(argv);

	အကယ်၍ (!argv[0]) {
		အကယ်၍ ((ENABLE_ASH_PRINTF || ENABLE_HUSH_PRINTF)
		 && applet_name[0] != 'p'
		) {
			bb_simple_error_msg("usage: printf FORMAT [ARGUMENT...]");
			ပြန်ပေး 2; /* bash compat */
		}
		bb_show_usage();
	}

	format = argv[0];
	argv2 = argv + 1;

	conv_err = 0;
	do {
		argv = argv2;
		argv2 = print_formatted(format, argv, &conv_err);
	} နေစဉ် (argv2 > argv && *argv2);

	/* coreutils compat (bash doesn't do this):
	if (*argv)
		fprintf(stderr, "excess args ignored");
	*/

	ပြန်ပေး (argv2 < argv) /* if true, print_formatted errored out */
		|| conv_err; /* print_formatted saw invalid number */
}
