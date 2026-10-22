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

#شامل "libbb.h"

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

تعریفنوع تهی FAST_FUNC (*converter)(ثابت نویسه *arg, تهی *result);

ایستا صحیح multiconvert(ثابت نویسه *arg, تهی *result, converter convert)
{
	اگر (*arg == '"' || *arg == '\'') {
		arg = utoa((unsigned نویسه)arg[1]);
	}
	errno = 0;
	convert(arg, result);
	اگر (errno) {
		bb_error_پیام("invalid number '%s'", arg);
		بازگشت 1;
	}
	بازگشت 0;
}

ایستا تهی FAST_FUNC conv_strtoull(ثابت نویسه *arg, تهی *result)
{
	/* Allow leading '+' - bb_strtoull() by itself does not allow it,
	 * and probably shouldn't (other callers might require purely numeric
	 * inputs to be allowed.
	 */
	اگر (arg[0] == '+')
		arg++;
	*(unsigned long long*)result = bb_strtoull(arg, پوچ, 0);
	/* both coreutils 6.10 and bash 3.2:
	 * $ printf '%x\n' -2
	 * fffffffffffffffe
	 * Mimic that:
	 */
	اگر (errno) {
		*(unsigned long long*)result = bb_strtoll(arg, پوچ, 0);
	}
}
ایستا تهی FAST_FUNC conv_strtoll(ثابت نویسه *arg, تهی *result)
{
	اگر (arg[0] == '+')
		arg++;
	*(long long*)result = bb_strtoll(arg, پوچ, 0);
}
ایستا تهی FAST_FUNC conv_strtod(ثابت نویسه *arg, تهی *result)
{
	نویسه *end;
	/* Well, this one allows leading whitespace... so what? */
	/* What I like much less is that "-" accepted too! :( */
//TODO: needs setlocale(LC_NUMERIC, "C")?
	*(دوگانه*)result = strtod(arg, &end);
	اگر (end[0]) {
		errno = ERANGE;
		*(دوگانه*)result = 0;
	}
}

/* Callers should check errno to detect errors */
ایستا unsigned long long my_xstrtoull(ثابت نویسه *arg)
{
	unsigned long long result;
	اگر (multiconvert(arg, &result, conv_strtoull))
		result = 0;
	بازگشت result;
}
ایستا long long my_xstrtoll(ثابت نویسه *arg)
{
	long long result;
	اگر (multiconvert(arg, &result, conv_strtoll))
		result = 0;
	بازگشت result;
}
ایستا دوگانه my_xstrtod(ثابت نویسه *arg)
{
	دوگانه result;
	multiconvert(arg, &result, conv_strtod);
	بازگشت result;
}

/* Handles %b; return 1 if output is to be short-circuited by \c */
ایستا صحیح print_esc_string(ثابت نویسه *str)
{
	نویسه c;
	وقتی ((c = *str) != '\0') {
		str++;
		اگر (c == '\\') {
			/* %b also accepts 4-digit octals of the form \0### */
			اگر (*str == '0') {
				اگر ((unsigned نویسه)(str[1] - '0') < 8) {
					/* 2nd char is 0..7: skip leading '0' */
					str++;
				}
			}
			وگرنه اگر (*str == 'c') {
				بازگشت 1;
			}
			{
				/* optimization: don't force arg to be on-stack,
				 * use another variable for that. */
				ثابت نویسه *z = str;
				c = bb_process_escape_sequence(&z);
				str = z;
			}
		}
		putchar(c);
	}

	بازگشت 0;
}

ایستا تهی print_direc(نویسه *format, unsigned fmt_length,
		صحیح field_width, صحیح precision,
		ثابت نویسه *argument)
{
	long long llv;
	دوگانه dv;
	نویسه saved;
	نویسه *have_prec, *have_width;

	saved = format[fmt_length];
	format[fmt_length] = '\0';

	have_prec = strstr(format, ".*");
	have_width = strchr(format, '*');
	اگر (have_width - 1 == have_prec)
		have_width = پوچ;

	/* multiconvert sets errno = 0, but %s needs it cleared */
	errno = 0;

	انتخاب (format[fmt_length - 1]) {
	حالت 'c':
		printf(format, *argument);
		بشکن;
	حالت 'd':
	حالت 'i':
		llv = my_xstrtoll(skip_whitespace(argument));
 print_long:
		اگر (!have_width) {
			اگر (!have_prec)
				printf(format, llv);
			وگرنه
				printf(format, precision, llv);
		} وگرنه {
			اگر (!have_prec)
				printf(format, field_width, llv);
			وگرنه
				printf(format, field_width, precision, llv);
		}
		بشکن;
	حالت 'o':
	حالت 'u':
	حالت 'x':
	حالت 'X':
		llv = my_xstrtoull(skip_whitespace(argument));
		/* cheat: unsigned long and long have same width, so... */
		برو print_long;
	حالت 's':
		/* Are char* and long long the same? */
		اگر (اندازه(argument) == اندازه(llv)) {
			llv = (long long)(ptrdiff_t)argument;
			برو print_long;
		} وگرنه {
			/* Hope compiler will optimize it out by moving call
			 * instruction after the ifs... */
			اگر (!have_width) {
				اگر (!have_prec)
					printf(format, argument, /*unused:*/ argument, argument);
				وگرنه
					printf(format, precision, argument, /*unused:*/ argument);
			} وگرنه {
				اگر (!have_prec)
					printf(format, field_width, argument, /*unused:*/ argument);
				وگرنه
					printf(format, field_width, precision, argument);
			}
			بشکن;
		}
	حالت 'f':
	حالت 'e':
	حالت 'E':
	حالت 'g':
	حالت 'G':
		dv = my_xstrtod(argument);
		اگر (!have_width) {
			اگر (!have_prec)
				printf(format, dv);
			وگرنه
				printf(format, precision, dv);
		} وگرنه {
			اگر (!have_prec)
				printf(format, field_width, dv);
			وگرنه
				printf(format, field_width, precision, dv);
		}
		بشکن;
	} /* switch */

	format[fmt_length] = saved;
}

/* Handle params for "%*.*f". Negative numbers are ok (compat). */
ایستا صحیح get_width_prec(ثابت نویسه *str)
{
	صحیح v = bb_strtoi(str, پوچ, 10);
	اگر (errno) {
		bb_error_پیام("invalid number '%s'", str);
		v = 0;
	}
	بازگشت v;
}

/* Print the text in FORMAT, using ARGV for arguments to any '%' directives.
   Return advanced ARGV.  */
ایستا نویسه **print_formatted(نویسه *f, نویسه **argv, صحیح *conv_err)
{
	نویسه *direc_start;      /* Start of % directive.  */
	unsigned direc_length;  /* Length of % directive.  */
	صحیح field_width;        /* Arg to first '*' */
	صحیح precision;          /* Arg to second '*' */
	نویسه **saved_argv = argv;

	برای (; *f; ++f) {
		انتخاب (*f) {
		حالت '%':
			direc_start = f++;
			direc_length = 1;
			field_width = precision = 0;
			اگر (*f == '%') {
				bb_putchar('%');
				بشکن;
			}
			اگر (*f == 'b') {
				اگر (*argv) {
					اگر (print_esc_string(*argv))
						بازگشت saved_argv; /* causes main() to exit */
					++argv;
				}
				بشکن;
			}
			وقتی (*f && strchr("-+ #0", *f)) {
				++f;
				++direc_length;
			}
			اگر (*f == '*') {
				++f;
				++direc_length;
				اگر (*argv)
					field_width = get_width_prec(*argv++);
			} وگرنه {
				وقتی (isdigit(*f)) {
					++f;
					++direc_length;
				}
			}
			اگر (*f == '.') {
				++f;
				++direc_length;
				اگر (*f == '*') {
					++f;
					++direc_length;
					اگر (*argv)
						precision = get_width_prec(*argv++);
				} وگرنه {
					وقتی (isdigit(*f)) {
						++f;
						++direc_length;
					}
				}
			}

			/* Remove "lLhz" size modifiers, repeatedly.
			 * bash does not like "%lld", but coreutils
			 * happily takes even "%Llllhhzhhzd"!
			 * We are permissive like coreutils */
			وقتی ((*f | 0x20) == 'l' || *f == 'h' || *f == 'z') {
				overlapping_strcpy(f, f + 1);
			}
			/* Add "ll" if integer modifier, then print */
			{
				ایستا ثابت نویسه format_chars[] ALIGN1 = "diouxXfeEgGcs";
				نویسه *p = strchr(format_chars, *f);
				/* needed - try "printf %" without it */
				اگر (p == پوچ || *f == '\0') {
					bb_error_پیام("%s: invalid format", direc_start);
					/* causes main() to exit with error */
					بازگشت saved_argv - 1;
				}
				++direc_length;
				اگر (p - format_chars <= 5) {
					/* it is one of "diouxX" */
					p = xmalloc(direc_length + 3);
					memcpy(p, direc_start, direc_length);
					p[direc_length + 1] = p[direc_length - 1];
					p[direc_length - 1] = 'l';
					p[direc_length] = 'l';
					//bb_error_msg("<%s>", p);
					direc_length += 2;
					direc_start = p;
				} وگرنه {
					p = پوچ;
				}
				اگر (*argv) {
					print_direc(direc_start, direc_length, field_width,
								precision, *argv++);
				} وگرنه {
					print_direc(direc_start, direc_length, field_width,
								precision, "");
				}
				*conv_err |= errno;
				آزاد(p);
			}
			بشکن;
		حالت '\\':
			اگر (*++f == 'c') {
				بازگشت saved_argv; /* causes main() to exit */
			}
			bb_putchar(bb_process_escape_sequence((ثابت نویسه **)&f));
			f--;
			بشکن;
		پیشفرض:
			putchar(*f);
		}
	}

	بازگشت argv;
}

صحیح printf_main(صحیح argc UNUSED_PARAM, نویسه **argv)
{
	صحیح conv_err;
	نویسه *format;
	نویسه **argv2;

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
	اگر (fcntl(1, F_GETFL) == -1)
		بازگشت 1; /* match coreutils 6.10 (sans error msg to stderr) */
	//if (dup2(1, 1) != 1) - old way
	//	return 1;

	/* bash builtin errors out on "printf '-%s-\n' foo",
	 * coreutils-6.9 works. Both work with "printf -- '-%s-\n' foo".
	 * We will mimic coreutils. */
	argv = skip_dash_dash(argv);

	اگر (!argv[0]) {
		اگر ((ENABLE_ASH_PRINTF || ENABLE_HUSH_PRINTF)
		 && applet_name[0] != 'p'
		) {
			bb_simple_error_پیام("usage: printf FORMAT [ARGUMENT...]");
			بازگشت 2; /* bash compat */
		}
		bb_show_usage();
	}

	format = argv[0];
	argv2 = argv + 1;

	conv_err = 0;
	انجامبده {
		argv = argv2;
		argv2 = print_formatted(format, argv, &conv_err);
	} وقتی (argv2 > argv && *argv2);

	/* coreutils compat (bash doesn't do this):
	if (*argv)
		fprintf(stderr, "excess args ignored");
	*/

	بازگشت (argv2 < argv) /* if true, print_formatted errored out */
		|| conv_err; /* print_formatted saw invalid number */
}
