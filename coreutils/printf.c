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

#ಅನ್ತರ್ಭಾವ "libbb.h"

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

ಪ್ರಕಾರನಿರ್ದೇಶ ಶೂನ್ಯ FAST_FUNC (*converter)(ಧ್ರುವ ವರ್ಣ *arg, ಶೂನ್ಯ *result);

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ multiconvert(ಧ್ರುವ ವರ್ಣ *arg, ಶೂನ್ಯ *result, converter convert)
{
	ಯದಿ (*arg == '"' || *arg == '\'') {
		arg = utoa((unsigned ವರ್ಣ)arg[1]);
	}
	errno = 0;
	convert(arg, result);
	ಯದಿ (errno) {
		bb_error_ಸಂದೇಶ("invalid number '%s'", arg);
		ಪ್ರತ್ಯಾಗಮ 1;
	}
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ FAST_FUNC conv_strtoull(ಧ್ರುವ ವರ್ಣ *arg, ಶೂನ್ಯ *result)
{
	/* Allow leading '+' - bb_strtoull() by itself does not allow it,
	 * and probably shouldn't (other callers might require purely numeric
	 * inputs to be allowed.
	 */
	ಯದಿ (arg[0] == '+')
		arg++;
	*(unsigned long long*)result = bb_strtoull(arg, ರಿಕ್ತ, 0);
	/* both coreutils 6.10 and bash 3.2:
	 * $ printf '%x\n' -2
	 * fffffffffffffffe
	 * Mimic that:
	 */
	ಯದಿ (errno) {
		*(unsigned long long*)result = bb_strtoll(arg, ರಿಕ್ತ, 0);
	}
}
ಸ್ಥಿರ ಶೂನ್ಯ FAST_FUNC conv_strtoll(ಧ್ರುವ ವರ್ಣ *arg, ಶೂನ್ಯ *result)
{
	ಯದಿ (arg[0] == '+')
		arg++;
	*(long long*)result = bb_strtoll(arg, ರಿಕ್ತ, 0);
}
ಸ್ಥಿರ ಶೂನ್ಯ FAST_FUNC conv_strtod(ಧ್ರುವ ವರ್ಣ *arg, ಶೂನ್ಯ *result)
{
	ವರ್ಣ *end;
	/* Well, this one allows leading whitespace... so what? */
	/* What I like much less is that "-" accepted too! :( */
//TODO: needs setlocale(LC_NUMERIC, "C")?
	*(ದ್ವಿಗುಣ*)result = strtod(arg, &end);
	ಯದಿ (end[0]) {
		errno = ERANGE;
		*(ದ್ವಿಗುಣ*)result = 0;
	}
}

/* Callers should check errno to detect errors */
ಸ್ಥಿರ unsigned long long my_xstrtoull(ಧ್ರುವ ವರ್ಣ *arg)
{
	unsigned long long result;
	ಯದಿ (multiconvert(arg, &result, conv_strtoull))
		result = 0;
	ಪ್ರತ್ಯಾಗಮ result;
}
ಸ್ಥಿರ long long my_xstrtoll(ಧ್ರುವ ವರ್ಣ *arg)
{
	long long result;
	ಯದಿ (multiconvert(arg, &result, conv_strtoll))
		result = 0;
	ಪ್ರತ್ಯಾಗಮ result;
}
ಸ್ಥಿರ ದ್ವಿಗುಣ my_xstrtod(ಧ್ರುವ ವರ್ಣ *arg)
{
	ದ್ವಿಗುಣ result;
	multiconvert(arg, &result, conv_strtod);
	ಪ್ರತ್ಯಾಗಮ result;
}

/* Handles %b; return 1 if output is to be short-circuited by \c */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ print_esc_string(ಧ್ರುವ ವರ್ಣ *str)
{
	ವರ್ಣ c;
	ಯಾವತ್ ((c = *str) != '\0') {
		str++;
		ಯದಿ (c == '\\') {
			/* %b also accepts 4-digit octals of the form \0### */
			ಯದಿ (*str == '0') {
				ಯದಿ ((unsigned ವರ್ಣ)(str[1] - '0') < 8) {
					/* 2nd char is 0..7: skip leading '0' */
					str++;
				}
			}
			ಅನ್ಯಥಾ ಯದಿ (*str == 'c') {
				ಪ್ರತ್ಯಾಗಮ 1;
			}
			{
				/* optimization: don't force arg to be on-stack,
				 * use another variable for that. */
				ಧ್ರುವ ವರ್ಣ *z = str;
				c = bb_process_escape_sequence(&z);
				str = z;
			}
		}
		putchar(c);
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ print_direc(ವರ್ಣ *format, unsigned fmt_length,
		ಪೂರ್ಣಾಙ್ಕ field_width, ಪೂರ್ಣಾಙ್ಕ precision,
		ಧ್ರುವ ವರ್ಣ *argument)
{
	long long llv;
	ದ್ವಿಗುಣ dv;
	ವರ್ಣ saved;
	ವರ್ಣ *have_prec, *have_width;

	saved = format[fmt_length];
	format[fmt_length] = '\0';

	have_prec = strstr(format, ".*");
	have_width = strchr(format, '*');
	ಯದಿ (have_width - 1 == have_prec)
		have_width = ರಿಕ್ತ;

	/* multiconvert sets errno = 0, but %s needs it cleared */
	errno = 0;

	ವಿಕಲ್ಪ (format[fmt_length - 1]) {
	ಪಕ್ಷ 'c':
		printf(format, *argument);
		ಭಙ್ಗ;
	ಪಕ್ಷ 'd':
	ಪಕ್ಷ 'i':
		llv = my_xstrtoll(skip_whitespace(argument));
 print_long:
		ಯದಿ (!have_width) {
			ಯದಿ (!have_prec)
				printf(format, llv);
			ಅನ್ಯಥಾ
				printf(format, precision, llv);
		} ಅನ್ಯಥಾ {
			ಯದಿ (!have_prec)
				printf(format, field_width, llv);
			ಅನ್ಯಥಾ
				printf(format, field_width, precision, llv);
		}
		ಭಙ್ಗ;
	ಪಕ್ಷ 'o':
	ಪಕ್ಷ 'u':
	ಪಕ್ಷ 'x':
	ಪಕ್ಷ 'X':
		llv = my_xstrtoull(skip_whitespace(argument));
		/* cheat: unsigned long and long have same width, so... */
		ಗಚ್ಛ print_long;
	ಪಕ್ಷ 's':
		/* Are char* and long long the same? */
		ಯದಿ (ಪರಿಮಾಣ(argument) == ಪರಿಮಾಣ(llv)) {
			llv = (long long)(ptrdiff_t)argument;
			ಗಚ್ಛ print_long;
		} ಅನ್ಯಥಾ {
			/* Hope compiler will optimize it out by moving call
			 * instruction after the ifs... */
			ಯದಿ (!have_width) {
				ಯದಿ (!have_prec)
					printf(format, argument, /*unused:*/ argument, argument);
				ಅನ್ಯಥಾ
					printf(format, precision, argument, /*unused:*/ argument);
			} ಅನ್ಯಥಾ {
				ಯದಿ (!have_prec)
					printf(format, field_width, argument, /*unused:*/ argument);
				ಅನ್ಯಥಾ
					printf(format, field_width, precision, argument);
			}
			ಭಙ್ಗ;
		}
	ಪಕ್ಷ 'f':
	ಪಕ್ಷ 'e':
	ಪಕ್ಷ 'E':
	ಪಕ್ಷ 'g':
	ಪಕ್ಷ 'G':
		dv = my_xstrtod(argument);
		ಯದಿ (!have_width) {
			ಯದಿ (!have_prec)
				printf(format, dv);
			ಅನ್ಯಥಾ
				printf(format, precision, dv);
		} ಅನ್ಯಥಾ {
			ಯದಿ (!have_prec)
				printf(format, field_width, dv);
			ಅನ್ಯಥಾ
				printf(format, field_width, precision, dv);
		}
		ಭಙ್ಗ;
	} /* switch */

	format[fmt_length] = saved;
}

/* Handle params for "%*.*f". Negative numbers are ok (compat). */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ get_width_prec(ಧ್ರುವ ವರ್ಣ *str)
{
	ಪೂರ್ಣಾಙ್ಕ v = bb_strtoi(str, ರಿಕ್ತ, 10);
	ಯದಿ (errno) {
		bb_error_ಸಂದೇಶ("invalid number '%s'", str);
		v = 0;
	}
	ಪ್ರತ್ಯಾಗಮ v;
}

/* Print the text in FORMAT, using ARGV for arguments to any '%' directives.
   Return advanced ARGV.  */
ಸ್ಥಿರ ವರ್ಣ **print_formatted(ವರ್ಣ *f, ವರ್ಣ **argv, ಪೂರ್ಣಾಙ್ಕ *conv_err)
{
	ವರ್ಣ *direc_start;      /* Start of % directive.  */
	unsigned direc_length;  /* Length of % directive.  */
	ಪೂರ್ಣಾಙ್ಕ field_width;        /* Arg to first '*' */
	ಪೂರ್ಣಾಙ್ಕ precision;          /* Arg to second '*' */
	ವರ್ಣ **saved_argv = argv;

	ಪ್ರತಿ (; *f; ++f) {
		ವಿಕಲ್ಪ (*f) {
		ಪಕ್ಷ '%':
			direc_start = f++;
			direc_length = 1;
			field_width = precision = 0;
			ಯದಿ (*f == '%') {
				bb_putchar('%');
				ಭಙ್ಗ;
			}
			ಯದಿ (*f == 'b') {
				ಯದಿ (*argv) {
					ಯದಿ (print_esc_string(*argv))
						ಪ್ರತ್ಯಾಗಮ saved_argv; /* causes main() to exit */
					++argv;
				}
				ಭಙ್ಗ;
			}
			ಯಾವತ್ (*f && strchr("-+ #0", *f)) {
				++f;
				++direc_length;
			}
			ಯದಿ (*f == '*') {
				++f;
				++direc_length;
				ಯದಿ (*argv)
					field_width = get_width_prec(*argv++);
			} ಅನ್ಯಥಾ {
				ಯಾವತ್ (isdigit(*f)) {
					++f;
					++direc_length;
				}
			}
			ಯದಿ (*f == '.') {
				++f;
				++direc_length;
				ಯದಿ (*f == '*') {
					++f;
					++direc_length;
					ಯದಿ (*argv)
						precision = get_width_prec(*argv++);
				} ಅನ್ಯಥಾ {
					ಯಾವತ್ (isdigit(*f)) {
						++f;
						++direc_length;
					}
				}
			}

			/* Remove "lLhz" size modifiers, repeatedly.
			 * bash does not like "%lld", but coreutils
			 * happily takes even "%Llllhhzhhzd"!
			 * We are permissive like coreutils */
			ಯಾವತ್ ((*f | 0x20) == 'l' || *f == 'h' || *f == 'z') {
				overlapping_strcpy(f, f + 1);
			}
			/* Add "ll" if integer modifier, then print */
			{
				ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ format_chars[] ALIGN1 = "diouxXfeEgGcs";
				ವರ್ಣ *p = strchr(format_chars, *f);
				/* needed - try "printf %" without it */
				ಯದಿ (p == ರಿಕ್ತ || *f == '\0') {
					bb_error_ಸಂದೇಶ("%s: invalid format", direc_start);
					/* causes main() to exit with error */
					ಪ್ರತ್ಯಾಗಮ saved_argv - 1;
				}
				++direc_length;
				ಯದಿ (p - format_chars <= 5) {
					/* it is one of "diouxX" */
					p = xmalloc(direc_length + 3);
					memcpy(p, direc_start, direc_length);
					p[direc_length + 1] = p[direc_length - 1];
					p[direc_length - 1] = 'l';
					p[direc_length] = 'l';
					//bb_error_msg("<%s>", p);
					direc_length += 2;
					direc_start = p;
				} ಅನ್ಯಥಾ {
					p = ರಿಕ್ತ;
				}
				ಯದಿ (*argv) {
					print_direc(direc_start, direc_length, field_width,
								precision, *argv++);
				} ಅನ್ಯಥಾ {
					print_direc(direc_start, direc_length, field_width,
								precision, "");
				}
				*conv_err |= errno;
				ಮುಕ್ತ(p);
			}
			ಭಙ್ಗ;
		ಪಕ್ಷ '\\':
			ಯದಿ (*++f == 'c') {
				ಪ್ರತ್ಯಾಗಮ saved_argv; /* causes main() to exit */
			}
			bb_putchar(bb_process_escape_sequence((ಧ್ರುವ ವರ್ಣ **)&f));
			f--;
			ಭಙ್ಗ;
		ಸ್ವಭಾವ:
			putchar(*f);
		}
	}

	ಪ್ರತ್ಯಾಗಮ argv;
}

ಪೂರ್ಣಾಙ್ಕ printf_main(ಪೂರ್ಣಾಙ್ಕ argc UNUSED_PARAM, ವರ್ಣ **argv)
{
	ಪೂರ್ಣಾಙ್ಕ conv_err;
	ವರ್ಣ *format;
	ವರ್ಣ **argv2;

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
	ಯದಿ (fcntl(1, F_GETFL) == -1)
		ಪ್ರತ್ಯಾಗಮ 1; /* match coreutils 6.10 (sans error msg to stderr) */
	//if (dup2(1, 1) != 1) - old way
	//	return 1;

	/* bash builtin errors out on "printf '-%s-\n' foo",
	 * coreutils-6.9 works. Both work with "printf -- '-%s-\n' foo".
	 * We will mimic coreutils. */
	argv = skip_dash_dash(argv);

	ಯದಿ (!argv[0]) {
		ಯದಿ ((ENABLE_ASH_PRINTF || ENABLE_HUSH_PRINTF)
		 && applet_name[0] != 'p'
		) {
			bb_simple_error_ಸಂದೇಶ("usage: printf FORMAT [ARGUMENT...]");
			ಪ್ರತ್ಯಾಗಮ 2; /* bash compat */
		}
		bb_show_usage();
	}

	format = argv[0];
	argv2 = argv + 1;

	conv_err = 0;
	ಕುರು {
		argv = argv2;
		argv2 = print_formatted(format, argv, &conv_err);
	} ಯಾವತ್ (argv2 > argv && *argv2);

	/* coreutils compat (bash doesn't do this):
	if (*argv)
		fprintf(stderr, "excess args ignored");
	*/

	ಪ್ರತ್ಯಾಗಮ (argv2 < argv) /* if true, print_formatted errored out */
		|| conv_err; /* print_formatted saw invalid number */
}
