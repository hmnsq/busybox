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

#অন্তর্ভাৱ "libbb.h"

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

প্রকারনির্দেশ শূন্য FAST_FUNC (*converter)(ধ্রুৱ ৱর্ণ *arg, শূন্য *result);

স্থির পূর্ণাঙ্ক multiconvert(ধ্রুৱ ৱর্ণ *arg, শূন্য *result, converter convert)
{
	যদি (*arg == '"' || *arg == '\'') {
		arg = utoa((unsigned ৱর্ণ)arg[1]);
	}
	errno = 0;
	convert(arg, result);
	যদি (errno) {
		bb_error_বাৰ্তা("invalid number '%s'", arg);
		ঘূৰাই 1;
	}
	ঘূৰাই 0;
}

স্থির শূন্য FAST_FUNC conv_strtoull(ধ্রুৱ ৱর্ণ *arg, শূন্য *result)
{
	/* Allow leading '+' - bb_strtoull() by itself does not allow it,
	 * and probably shouldn't (other callers might require purely numeric
	 * inputs to be allowed.
	 */
	যদি (arg[0] == '+')
		arg++;
	*(unsigned long long*)result = bb_strtoull(arg, রিক্ত, 0);
	/* both coreutils 6.10 and bash 3.2:
	 * $ printf '%x\n' -2
	 * fffffffffffffffe
	 * Mimic that:
	 */
	যদি (errno) {
		*(unsigned long long*)result = bb_strtoll(arg, রিক্ত, 0);
	}
}
স্থির শূন্য FAST_FUNC conv_strtoll(ধ্রুৱ ৱর্ণ *arg, শূন্য *result)
{
	যদি (arg[0] == '+')
		arg++;
	*(long long*)result = bb_strtoll(arg, রিক্ত, 0);
}
স্থির শূন্য FAST_FUNC conv_strtod(ধ্রুৱ ৱর্ণ *arg, শূন্য *result)
{
	ৱর্ণ *end;
	/* Well, this one allows leading whitespace... so what? */
	/* What I like much less is that "-" accepted too! :( */
//TODO: needs setlocale(LC_NUMERIC, "C")?
	*(দ্ৱিগুণ*)result = strtod(arg, &end);
	যদি (end[0]) {
		errno = ERANGE;
		*(দ্ৱিগুণ*)result = 0;
	}
}

/* Callers should check errno to detect errors */
স্থির unsigned long long my_xstrtoull(ধ্রুৱ ৱর্ণ *arg)
{
	unsigned long long result;
	যদি (multiconvert(arg, &result, conv_strtoull))
		result = 0;
	ঘূৰাই result;
}
স্থির long long my_xstrtoll(ধ্রুৱ ৱর্ণ *arg)
{
	long long result;
	যদি (multiconvert(arg, &result, conv_strtoll))
		result = 0;
	ঘূৰাই result;
}
স্থির দ্ৱিগুণ my_xstrtod(ধ্রুৱ ৱর্ণ *arg)
{
	দ্ৱিগুণ result;
	multiconvert(arg, &result, conv_strtod);
	ঘূৰাই result;
}

/* Handles %b; return 1 if output is to be short-circuited by \c */
স্থির পূর্ণাঙ্ক print_esc_string(ধ্রুৱ ৱর্ণ *str)
{
	ৱর্ণ c;
	যেতিয়ালৈ ((c = *str) != '\0') {
		str++;
		যদি (c == '\\') {
			/* %b also accepts 4-digit octals of the form \0### */
			যদি (*str == '0') {
				যদি ((unsigned ৱর্ণ)(str[1] - '0') < 8) {
					/* 2nd char is 0..7: skip leading '0' */
					str++;
				}
			}
			অন্যথা যদি (*str == 'c') {
				ঘূৰাই 1;
			}
			{
				/* optimization: don't force arg to be on-stack,
				 * use another variable for that. */
				ধ্রুৱ ৱর্ণ *z = str;
				c = bb_process_escape_sequence(&z);
				str = z;
			}
		}
		putchar(c);
	}

	ঘূৰাই 0;
}

স্থির শূন্য print_direc(ৱর্ণ *format, unsigned fmt_length,
		পূর্ণাঙ্ক field_width, পূর্ণাঙ্ক precision,
		ধ্রুৱ ৱর্ণ *argument)
{
	long long llv;
	দ্ৱিগুণ dv;
	ৱর্ণ saved;
	ৱর্ণ *have_prec, *have_width;

	saved = format[fmt_length];
	format[fmt_length] = '\0';

	have_prec = strstr(format, ".*");
	have_width = strchr(format, '*');
	যদি (have_width - 1 == have_prec)
		have_width = রিক্ত;

	/* multiconvert sets errno = 0, but %s needs it cleared */
	errno = 0;

	ৱিকল্প (format[fmt_length - 1]) {
	পক্ষ 'c':
		printf(format, *argument);
		ভঙ্গ;
	পক্ষ 'd':
	পক্ষ 'i':
		llv = my_xstrtoll(skip_whitespace(argument));
 print_long:
		যদি (!have_width) {
			যদি (!have_prec)
				printf(format, llv);
			অন্যথা
				printf(format, precision, llv);
		} অন্যথা {
			যদি (!have_prec)
				printf(format, field_width, llv);
			অন্যথা
				printf(format, field_width, precision, llv);
		}
		ভঙ্গ;
	পক্ষ 'o':
	পক্ষ 'u':
	পক্ষ 'x':
	পক্ষ 'X':
		llv = my_xstrtoull(skip_whitespace(argument));
		/* cheat: unsigned long and long have same width, so... */
		গচ্ছ print_long;
	পক্ষ 's':
		/* Are char* and long long the same? */
		যদি (পরিমাণ(argument) == পরিমাণ(llv)) {
			llv = (long long)(ptrdiff_t)argument;
			গচ্ছ print_long;
		} অন্যথা {
			/* Hope compiler will optimize it out by moving call
			 * instruction after the ifs... */
			যদি (!have_width) {
				যদি (!have_prec)
					printf(format, argument, /*unused:*/ argument, argument);
				অন্যথা
					printf(format, precision, argument, /*unused:*/ argument);
			} অন্যথা {
				যদি (!have_prec)
					printf(format, field_width, argument, /*unused:*/ argument);
				অন্যথা
					printf(format, field_width, precision, argument);
			}
			ভঙ্গ;
		}
	পক্ষ 'f':
	পক্ষ 'e':
	পক্ষ 'E':
	পক্ষ 'g':
	পক্ষ 'G':
		dv = my_xstrtod(argument);
		যদি (!have_width) {
			যদি (!have_prec)
				printf(format, dv);
			অন্যথা
				printf(format, precision, dv);
		} অন্যথা {
			যদি (!have_prec)
				printf(format, field_width, dv);
			অন্যথা
				printf(format, field_width, precision, dv);
		}
		ভঙ্গ;
	} /* switch */

	format[fmt_length] = saved;
}

/* Handle params for "%*.*f". Negative numbers are ok (compat). */
স্থির পূর্ণাঙ্ক get_width_prec(ধ্রুৱ ৱর্ণ *str)
{
	পূর্ণাঙ্ক v = bb_strtoi(str, রিক্ত, 10);
	যদি (errno) {
		bb_error_বাৰ্তা("invalid number '%s'", str);
		v = 0;
	}
	ঘূৰাই v;
}

/* Print the text in FORMAT, using ARGV for arguments to any '%' directives.
   Return advanced ARGV.  */
স্থির ৱর্ণ **print_formatted(ৱর্ণ *f, ৱর্ণ **argv, পূর্ণাঙ্ক *conv_err)
{
	ৱর্ণ *direc_start;      /* Start of % directive.  */
	unsigned direc_length;  /* Length of % directive.  */
	পূর্ণাঙ্ক field_width;        /* Arg to first '*' */
	পূর্ণাঙ্ক precision;          /* Arg to second '*' */
	ৱর্ণ **saved_argv = argv;

	বাবে (; *f; ++f) {
		ৱিকল্প (*f) {
		পক্ষ '%':
			direc_start = f++;
			direc_length = 1;
			field_width = precision = 0;
			যদি (*f == '%') {
				bb_putchar('%');
				ভঙ্গ;
			}
			যদি (*f == 'b') {
				যদি (*argv) {
					যদি (print_esc_string(*argv))
						ঘূৰাই saved_argv; /* causes main() to exit */
					++argv;
				}
				ভঙ্গ;
			}
			যেতিয়ালৈ (*f && strchr("-+ #0", *f)) {
				++f;
				++direc_length;
			}
			যদি (*f == '*') {
				++f;
				++direc_length;
				যদি (*argv)
					field_width = get_width_prec(*argv++);
			} অন্যথা {
				যেতিয়ালৈ (isdigit(*f)) {
					++f;
					++direc_length;
				}
			}
			যদি (*f == '.') {
				++f;
				++direc_length;
				যদি (*f == '*') {
					++f;
					++direc_length;
					যদি (*argv)
						precision = get_width_prec(*argv++);
				} অন্যথা {
					যেতিয়ালৈ (isdigit(*f)) {
						++f;
						++direc_length;
					}
				}
			}

			/* Remove "lLhz" size modifiers, repeatedly.
			 * bash does not like "%lld", but coreutils
			 * happily takes even "%Llllhhzhhzd"!
			 * We are permissive like coreutils */
			যেতিয়ালৈ ((*f | 0x20) == 'l' || *f == 'h' || *f == 'z') {
				overlapping_strcpy(f, f + 1);
			}
			/* Add "ll" if integer modifier, then print */
			{
				স্থির ধ্রুৱ ৱর্ণ format_chars[] ALIGN1 = "diouxXfeEgGcs";
				ৱর্ণ *p = strchr(format_chars, *f);
				/* needed - try "printf %" without it */
				যদি (p == রিক্ত || *f == '\0') {
					bb_error_বাৰ্তা("%s: invalid format", direc_start);
					/* causes main() to exit with error */
					ঘূৰাই saved_argv - 1;
				}
				++direc_length;
				যদি (p - format_chars <= 5) {
					/* it is one of "diouxX" */
					p = xmalloc(direc_length + 3);
					memcpy(p, direc_start, direc_length);
					p[direc_length + 1] = p[direc_length - 1];
					p[direc_length - 1] = 'l';
					p[direc_length] = 'l';
					//bb_error_msg("<%s>", p);
					direc_length += 2;
					direc_start = p;
				} অন্যথা {
					p = রিক্ত;
				}
				যদি (*argv) {
					print_direc(direc_start, direc_length, field_width,
								precision, *argv++);
				} অন্যথা {
					print_direc(direc_start, direc_length, field_width,
								precision, "");
				}
				*conv_err |= errno;
				মুক্ত(p);
			}
			ভঙ্গ;
		পক্ষ '\\':
			যদি (*++f == 'c') {
				ঘূৰাই saved_argv; /* causes main() to exit */
			}
			bb_putchar(bb_process_escape_sequence((ধ্রুৱ ৱর্ণ **)&f));
			f--;
			ভঙ্গ;
		স্ৱভাৱ:
			putchar(*f);
		}
	}

	ঘূৰাই argv;
}

পূর্ণাঙ্ক printf_main(পূর্ণাঙ্ক argc UNUSED_PARAM, ৱর্ণ **argv)
{
	পূর্ণাঙ্ক conv_err;
	ৱর্ণ *format;
	ৱর্ণ **argv2;

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
	যদি (fcntl(1, F_GETFL) == -1)
		ঘূৰাই 1; /* match coreutils 6.10 (sans error msg to stderr) */
	//if (dup2(1, 1) != 1) - old way
	//	return 1;

	/* bash builtin errors out on "printf '-%s-\n' foo",
	 * coreutils-6.9 works. Both work with "printf -- '-%s-\n' foo".
	 * We will mimic coreutils. */
	argv = skip_dash_dash(argv);

	যদি (!argv[0]) {
		যদি ((ENABLE_ASH_PRINTF || ENABLE_HUSH_PRINTF)
		 && applet_name[0] != 'p'
		) {
			bb_simple_error_বাৰ্তা("usage: printf FORMAT [ARGUMENT...]");
			ঘূৰাই 2; /* bash compat */
		}
		bb_show_usage();
	}

	format = argv[0];
	argv2 = argv + 1;

	conv_err = 0;
	কৰা {
		argv = argv2;
		argv2 = print_formatted(format, argv, &conv_err);
	} যেতিয়ালৈ (argv2 > argv && *argv2);

	/* coreutils compat (bash doesn't do this):
	if (*argv)
		fprintf(stderr, "excess args ignored");
	*/

	ঘূৰাই (argv2 < argv) /* if true, print_formatted errored out */
		|| conv_err; /* print_formatted saw invalid number */
}
