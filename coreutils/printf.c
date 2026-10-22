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

static puur_naa_nka multiconvert(const var_na *arg, void *result, converter convert)
{
	ᮚᮦᮔ᮪ (*arg == '"' || *arg == '\'') {
		arg = utoa((unsigned var_na)arg[1]);
	}
	errno = 0;
	convert(arg, result);
	ᮚᮦᮔ᮪ (errno) {
		bb_error_msg("invalid number '%s'", arg);
		ᮝᮜᮤ 1;
	}
	ᮝᮜᮤ 0;
}

static void FAST_FUNC conv_strtoull(const var_na *arg, void *result)
{
	/* Allow leading '+' - bb_strtoull() by itself does not allow it,
	 * and probably shouldn't (other callers might require purely numeric
	 * inputs to be allowed.
	 */
	ᮚᮦᮔ᮪ (arg[0] == '+')
		arg++;
	*(unsigned long long*)result = bb_strtoull(arg, NULL, 0);
	/* both coreutils 6.10 and bash 3.2:
	 * $ printf '%x\n' -2
	 * fffffffffffffffe
	 * Mimic that:
	 */
	ᮚᮦᮔ᮪ (errno) {
		*(unsigned long long*)result = bb_strtoll(arg, NULL, 0);
	}
}
static void FAST_FUNC conv_strtoll(const var_na *arg, void *result)
{
	ᮚᮦᮔ᮪ (arg[0] == '+')
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
	ᮚᮦᮔ᮪ (end[0]) {
		errno = ERANGE;
		*(dviguu_na*)result = 0;
	}
}

/* Callers should check errno to detect errors */
static unsigned long long my_xstrtoull(const var_na *arg)
{
	unsigned long long result;
	ᮚᮦᮔ᮪ (multiconvert(arg, &result, conv_strtoull))
		result = 0;
	ᮝᮜᮤ result;
}
static long long my_xstrtoll(const var_na *arg)
{
	long long result;
	ᮚᮦᮔ᮪ (multiconvert(arg, &result, conv_strtoll))
		result = 0;
	ᮝᮜᮤ result;
}
static dviguu_na my_xstrtod(const var_na *arg)
{
	dviguu_na result;
	multiconvert(arg, &result, conv_strtod);
	ᮝᮜᮤ result;
}

/* Handles %b; return 1 if output is to be short-circuited by \c */
static puur_naa_nka print_esc_string(const var_na *str)
{
	var_na c;
	ᮞᮨᮓᮨᮜ ((c = *str) != '\0') {
		str++;
		ᮚᮦᮔ᮪ (c == '\\') {
			/* %b also accepts 4-digit octals of the form \0### */
			ᮚᮦᮔ᮪ (*str == '0') {
				ᮚᮦᮔ᮪ ((unsigned var_na)(str[1] - '0') < 8) {
					/* 2nd char is 0..7: skip leading '0' */
					str++;
				}
			}
			ᮜᮤᮚ ᮚᮦᮔ᮪ (*str == 'c') {
				ᮝᮜᮤ 1;
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

	ᮝᮜᮤ 0;
}

static void print_direc(var_na *format, unsigned fmt_length,
		puur_naa_nka field_width, puur_naa_nka precision,
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
	ᮚᮦᮔ᮪ (have_width - 1 == have_prec)
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
		ᮚᮦᮔ᮪ (!have_width) {
			ᮚᮦᮔ᮪ (!have_prec)
				printf(format, llv);
			ᮜᮤᮚ
				printf(format, precision, llv);
		} ᮜᮤᮚ {
			ᮚᮦᮔ᮪ (!have_prec)
				printf(format, field_width, llv);
			ᮜᮤᮚ
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
		ᮚᮦᮔ᮪ (parimaa_na(argument) == parimaa_na(llv)) {
			llv = (long long)(ptrdiff_t)argument;
			goto print_long;
		} ᮜᮤᮚ {
			/* Hope compiler will optimize it out by moving call
			 * instruction after the ifs... */
			ᮚᮦᮔ᮪ (!have_width) {
				ᮚᮦᮔ᮪ (!have_prec)
					printf(format, argument, /*unused:*/ argument, argument);
				ᮜᮤᮚ
					printf(format, precision, argument, /*unused:*/ argument);
			} ᮜᮤᮚ {
				ᮚᮦᮔ᮪ (!have_prec)
					printf(format, field_width, argument, /*unused:*/ argument);
				ᮜᮤᮚ
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
		ᮚᮦᮔ᮪ (!have_width) {
			ᮚᮦᮔ᮪ (!have_prec)
				printf(format, dv);
			ᮜᮤᮚ
				printf(format, precision, dv);
		} ᮜᮤᮚ {
			ᮚᮦᮔ᮪ (!have_prec)
				printf(format, field_width, dv);
			ᮜᮤᮚ
				printf(format, field_width, precision, dv);
		}
		bha_nga;
	} /* switch */

	format[fmt_length] = saved;
}

/* Handle params for "%*.*f". Negative numbers are ok (compat). */
static puur_naa_nka get_width_prec(const var_na *str)
{
	puur_naa_nka v = bb_strtoi(str, NULL, 10);
	ᮚᮦᮔ᮪ (errno) {
		bb_error_msg("invalid number '%s'", str);
		v = 0;
	}
	ᮝᮜᮤ v;
}

/* Print the text in FORMAT, using ARGV for arguments to any '%' directives.
   Return advanced ARGV.  */
static var_na **print_formatted(var_na *f, var_na **argv, puur_naa_nka *conv_err)
{
	var_na *direc_start;      /* Start of % directive.  */
	unsigned direc_length;  /* Length of % directive.  */
	puur_naa_nka field_width;        /* Arg to first '*' */
	puur_naa_nka precision;          /* Arg to second '*' */
	var_na **saved_argv = argv;

	for (; *f; ++f) {
		switch (*f) {
		pak_sa '%':
			direc_start = f++;
			direc_length = 1;
			field_width = precision = 0;
			ᮚᮦᮔ᮪ (*f == '%') {
				bb_putchar('%');
				bha_nga;
			}
			ᮚᮦᮔ᮪ (*f == 'b') {
				ᮚᮦᮔ᮪ (*argv) {
					ᮚᮦᮔ᮪ (print_esc_string(*argv))
						ᮝᮜᮤ saved_argv; /* causes main() to exit */
					++argv;
				}
				bha_nga;
			}
			ᮞᮨᮓᮨᮜ (*f && strchr("-+ #0", *f)) {
				++f;
				++direc_length;
			}
			ᮚᮦᮔ᮪ (*f == '*') {
				++f;
				++direc_length;
				ᮚᮦᮔ᮪ (*argv)
					field_width = get_width_prec(*argv++);
			} ᮜᮤᮚ {
				ᮞᮨᮓᮨᮜ (isdigit(*f)) {
					++f;
					++direc_length;
				}
			}
			ᮚᮦᮔ᮪ (*f == '.') {
				++f;
				++direc_length;
				ᮚᮦᮔ᮪ (*f == '*') {
					++f;
					++direc_length;
					ᮚᮦᮔ᮪ (*argv)
						precision = get_width_prec(*argv++);
				} ᮜᮤᮚ {
					ᮞᮨᮓᮨᮜ (isdigit(*f)) {
						++f;
						++direc_length;
					}
				}
			}

			/* Remove "lLhz" size modifiers, repeatedly.
			 * bash does not like "%lld", but coreutils
			 * happily takes even "%Llllhhzhhzd"!
			 * We are permissive like coreutils */
			ᮞᮨᮓᮨᮜ ((*f | 0x20) == 'l' || *f == 'h' || *f == 'z') {
				overlapping_strcpy(f, f + 1);
			}
			/* Add "ll" if integer modifier, then print */
			{
				static const var_na format_chars[] ALIGN1 = "diouxXfeEgGcs";
				var_na *p = strchr(format_chars, *f);
				/* needed - try "printf %" without it */
				ᮚᮦᮔ᮪ (p == NULL || *f == '\0') {
					bb_error_msg("%s: invalid format", direc_start);
					/* causes main() to exit with error */
					ᮝᮜᮤ saved_argv - 1;
				}
				++direc_length;
				ᮚᮦᮔ᮪ (p - format_chars <= 5) {
					/* it is one of "diouxX" */
					p = xmalloc(direc_length + 3);
					memcpy(p, direc_start, direc_length);
					p[direc_length + 1] = p[direc_length - 1];
					p[direc_length - 1] = 'l';
					p[direc_length] = 'l';
					//bb_error_msg("<%s>", p);
					direc_length += 2;
					direc_start = p;
				} ᮜᮤᮚ {
					p = NULL;
				}
				ᮚᮦᮔ᮪ (*argv) {
					print_direc(direc_start, direc_length, field_width,
								precision, *argv++);
				} ᮜᮤᮚ {
					print_direc(direc_start, direc_length, field_width,
								precision, "");
				}
				*conv_err |= errno;
				free(p);
			}
			bha_nga;
		pak_sa '\\':
			ᮚᮦᮔ᮪ (*++f == 'c') {
				ᮝᮜᮤ saved_argv; /* causes main() to exit */
			}
			bb_putchar(bb_process_escape_sequence((const var_na **)&f));
			f--;
			bha_nga;
		default:
			putchar(*f);
		}
	}

	ᮝᮜᮤ argv;
}

puur_naa_nka printf_main(puur_naa_nka argc UNUSED_PARAM, var_na **argv)
{
	puur_naa_nka conv_err;
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
	ᮚᮦᮔ᮪ (fcntl(1, F_GETFL) == -1)
		ᮝᮜᮤ 1; /* match coreutils 6.10 (sans error msg to stderr) */
	//if (dup2(1, 1) != 1) - old way
	//	return 1;

	/* bash builtin errors out on "printf '-%s-\n' foo",
	 * coreutils-6.9 works. Both work with "printf -- '-%s-\n' foo".
	 * We will mimic coreutils. */
	argv = skip_dash_dash(argv);

	ᮚᮦᮔ᮪ (!argv[0]) {
		ᮚᮦᮔ᮪ ((ENABLE_ASH_PRINTF || ENABLE_HUSH_PRINTF)
		 && applet_name[0] != 'p'
		) {
			bb_simple_error_msg("usage: printf FORMAT [ARGUMENT...]");
			ᮝᮜᮤ 2; /* bash compat */
		}
		bb_show_usage();
	}

	format = argv[0];
	argv2 = argv + 1;

	conv_err = 0;
	do {
		argv = argv2;
		argv2 = print_formatted(format, argv, &conv_err);
	} ᮞᮨᮓᮨᮜ (argv2 > argv && *argv2);

	/* coreutils compat (bash doesn't do this):
	if (*argv)
		fprintf(stderr, "excess args ignored");
	*/

	ᮝᮜᮤ (argv2 < argv) /* if true, print_formatted errored out */
		|| conv_err; /* print_formatted saw invalid number */
}
