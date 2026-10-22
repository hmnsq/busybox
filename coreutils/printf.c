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

#அந்தர்பாவ "libbb.h"

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

வகைவரை வெறுமை FAST_FUNC (*converter)(மாறிலி எழுத்து *arg, வெறுமை *result);

நிலையான முழு multiconvert(மாறிலி எழுத்து *arg, வெறுமை *result, converter convert)
{
	எனில் (*arg == '"' || *arg == '\'') {
		arg = utoa((unsigned எழுத்து)arg[1]);
	}
	errno = 0;
	convert(arg, result);
	எனில் (errno) {
		bb_error_செய்தி("invalid number '%s'", arg);
		திரும்பு 1;
	}
	திரும்பு 0;
}

நிலையான வெறுமை FAST_FUNC conv_strtoull(மாறிலி எழுத்து *arg, வெறுமை *result)
{
	/* Allow leading '+' - bb_strtoull() by itself does not allow it,
	 * and probably shouldn't (other callers might require purely numeric
	 * inputs to be allowed.
	 */
	எனில் (arg[0] == '+')
		arg++;
	*(unsigned long long*)result = bb_strtoull(arg, வெற்று, 0);
	/* both coreutils 6.10 and bash 3.2:
	 * $ printf '%x\n' -2
	 * fffffffffffffffe
	 * Mimic that:
	 */
	எனில் (errno) {
		*(unsigned long long*)result = bb_strtoll(arg, வெற்று, 0);
	}
}
நிலையான வெறுமை FAST_FUNC conv_strtoll(மாறிலி எழுத்து *arg, வெறுமை *result)
{
	எனில் (arg[0] == '+')
		arg++;
	*(long long*)result = bb_strtoll(arg, வெற்று, 0);
}
நிலையான வெறுமை FAST_FUNC conv_strtod(மாறிலி எழுத்து *arg, வெறுமை *result)
{
	எழுத்து *end;
	/* Well, this one allows leading whitespace... so what? */
	/* What I like much less is that "-" accepted too! :( */
//TODO: needs setlocale(LC_NUMERIC, "C")?
	*(இரட்டை*)result = strtod(arg, &end);
	எனில் (end[0]) {
		errno = ERANGE;
		*(இரட்டை*)result = 0;
	}
}

/* Callers should check errno to detect errors */
நிலையான unsigned long long my_xstrtoull(மாறிலி எழுத்து *arg)
{
	unsigned long long result;
	எனில் (multiconvert(arg, &result, conv_strtoull))
		result = 0;
	திரும்பு result;
}
நிலையான long long my_xstrtoll(மாறிலி எழுத்து *arg)
{
	long long result;
	எனில் (multiconvert(arg, &result, conv_strtoll))
		result = 0;
	திரும்பு result;
}
நிலையான இரட்டை my_xstrtod(மாறிலி எழுத்து *arg)
{
	இரட்டை result;
	multiconvert(arg, &result, conv_strtod);
	திரும்பு result;
}

/* Handles %b; return 1 if output is to be short-circuited by \c */
நிலையான முழு print_esc_string(மாறிலி எழுத்து *str)
{
	எழுத்து c;
	போது ((c = *str) != '\0') {
		str++;
		எனில் (c == '\\') {
			/* %b also accepts 4-digit octals of the form \0### */
			எனில் (*str == '0') {
				எனில் ((unsigned எழுத்து)(str[1] - '0') < 8) {
					/* 2nd char is 0..7: skip leading '0' */
					str++;
				}
			}
			இல்லையெனில் எனில் (*str == 'c') {
				திரும்பு 1;
			}
			{
				/* optimization: don't force arg to be on-stack,
				 * use another variable for that. */
				மாறிலி எழுத்து *z = str;
				c = bb_process_escape_sequence(&z);
				str = z;
			}
		}
		putchar(c);
	}

	திரும்பு 0;
}

நிலையான வெறுமை print_direc(எழுத்து *format, unsigned fmt_length,
		முழு field_width, முழு precision,
		மாறிலி எழுத்து *argument)
{
	long long llv;
	இரட்டை dv;
	எழுத்து saved;
	எழுத்து *have_prec, *have_width;

	saved = format[fmt_length];
	format[fmt_length] = '\0';

	have_prec = strstr(format, ".*");
	have_width = strchr(format, '*');
	எனில் (have_width - 1 == have_prec)
		have_width = வெற்று;

	/* multiconvert sets errno = 0, but %s needs it cleared */
	errno = 0;

	தேர்வு (format[fmt_length - 1]) {
	நிலை 'c':
		printf(format, *argument);
		நிறுத்து;
	நிலை 'd':
	நிலை 'i':
		llv = my_xstrtoll(skip_whitespace(argument));
 print_long:
		எனில் (!have_width) {
			எனில் (!have_prec)
				printf(format, llv);
			இல்லையெனில்
				printf(format, precision, llv);
		} இல்லையெனில் {
			எனில் (!have_prec)
				printf(format, field_width, llv);
			இல்லையெனில்
				printf(format, field_width, precision, llv);
		}
		நிறுத்து;
	நிலை 'o':
	நிலை 'u':
	நிலை 'x':
	நிலை 'X':
		llv = my_xstrtoull(skip_whitespace(argument));
		/* cheat: unsigned long and long have same width, so... */
		செல் print_long;
	நிலை 's':
		/* Are char* and long long the same? */
		எனில் (அளவு(argument) == அளவு(llv)) {
			llv = (long long)(ptrdiff_t)argument;
			செல் print_long;
		} இல்லையெனில் {
			/* Hope compiler will optimize it out by moving call
			 * instruction after the ifs... */
			எனில் (!have_width) {
				எனில் (!have_prec)
					printf(format, argument, /*unused:*/ argument, argument);
				இல்லையெனில்
					printf(format, precision, argument, /*unused:*/ argument);
			} இல்லையெனில் {
				எனில் (!have_prec)
					printf(format, field_width, argument, /*unused:*/ argument);
				இல்லையெனில்
					printf(format, field_width, precision, argument);
			}
			நிறுத்து;
		}
	நிலை 'f':
	நிலை 'e':
	நிலை 'E':
	நிலை 'g':
	நிலை 'G':
		dv = my_xstrtod(argument);
		எனில் (!have_width) {
			எனில் (!have_prec)
				printf(format, dv);
			இல்லையெனில்
				printf(format, precision, dv);
		} இல்லையெனில் {
			எனில் (!have_prec)
				printf(format, field_width, dv);
			இல்லையெனில்
				printf(format, field_width, precision, dv);
		}
		நிறுத்து;
	} /* switch */

	format[fmt_length] = saved;
}

/* Handle params for "%*.*f". Negative numbers are ok (compat). */
நிலையான முழு get_width_prec(மாறிலி எழுத்து *str)
{
	முழு v = bb_strtoi(str, வெற்று, 10);
	எனில் (errno) {
		bb_error_செய்தி("invalid number '%s'", str);
		v = 0;
	}
	திரும்பு v;
}

/* Print the text in FORMAT, using ARGV for arguments to any '%' directives.
   Return advanced ARGV.  */
நிலையான எழுத்து **print_formatted(எழுத்து *f, எழுத்து **argv, முழு *conv_err)
{
	எழுத்து *direc_start;      /* Start of % directive.  */
	unsigned direc_length;  /* Length of % directive.  */
	முழு field_width;        /* Arg to first '*' */
	முழு precision;          /* Arg to second '*' */
	எழுத்து **saved_argv = argv;

	க்கு (; *f; ++f) {
		தேர்வு (*f) {
		நிலை '%':
			direc_start = f++;
			direc_length = 1;
			field_width = precision = 0;
			எனில் (*f == '%') {
				bb_putchar('%');
				நிறுத்து;
			}
			எனில் (*f == 'b') {
				எனில் (*argv) {
					எனில் (print_esc_string(*argv))
						திரும்பு saved_argv; /* causes main() to exit */
					++argv;
				}
				நிறுத்து;
			}
			போது (*f && strchr("-+ #0", *f)) {
				++f;
				++direc_length;
			}
			எனில் (*f == '*') {
				++f;
				++direc_length;
				எனில் (*argv)
					field_width = get_width_prec(*argv++);
			} இல்லையெனில் {
				போது (isdigit(*f)) {
					++f;
					++direc_length;
				}
			}
			எனில் (*f == '.') {
				++f;
				++direc_length;
				எனில் (*f == '*') {
					++f;
					++direc_length;
					எனில் (*argv)
						precision = get_width_prec(*argv++);
				} இல்லையெனில் {
					போது (isdigit(*f)) {
						++f;
						++direc_length;
					}
				}
			}

			/* Remove "lLhz" size modifiers, repeatedly.
			 * bash does not like "%lld", but coreutils
			 * happily takes even "%Llllhhzhhzd"!
			 * We are permissive like coreutils */
			போது ((*f | 0x20) == 'l' || *f == 'h' || *f == 'z') {
				overlapping_strcpy(f, f + 1);
			}
			/* Add "ll" if integer modifier, then print */
			{
				நிலையான மாறிலி எழுத்து format_chars[] ALIGN1 = "diouxXfeEgGcs";
				எழுத்து *p = strchr(format_chars, *f);
				/* needed - try "printf %" without it */
				எனில் (p == வெற்று || *f == '\0') {
					bb_error_செய்தி("%s: invalid format", direc_start);
					/* causes main() to exit with error */
					திரும்பு saved_argv - 1;
				}
				++direc_length;
				எனில் (p - format_chars <= 5) {
					/* it is one of "diouxX" */
					p = xmalloc(direc_length + 3);
					memcpy(p, direc_start, direc_length);
					p[direc_length + 1] = p[direc_length - 1];
					p[direc_length - 1] = 'l';
					p[direc_length] = 'l';
					//bb_error_msg("<%s>", p);
					direc_length += 2;
					direc_start = p;
				} இல்லையெனில் {
					p = வெற்று;
				}
				எனில் (*argv) {
					print_direc(direc_start, direc_length, field_width,
								precision, *argv++);
				} இல்லையெனில் {
					print_direc(direc_start, direc_length, field_width,
								precision, "");
				}
				*conv_err |= errno;
				விடுவி(p);
			}
			நிறுத்து;
		நிலை '\\':
			எனில் (*++f == 'c') {
				திரும்பு saved_argv; /* causes main() to exit */
			}
			bb_putchar(bb_process_escape_sequence((மாறிலி எழுத்து **)&f));
			f--;
			நிறுத்து;
		இயல்பு:
			putchar(*f);
		}
	}

	திரும்பு argv;
}

முழு printf_main(முழு argc UNUSED_PARAM, எழுத்து **argv)
{
	முழு conv_err;
	எழுத்து *format;
	எழுத்து **argv2;

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
	எனில் (fcntl(1, F_GETFL) == -1)
		திரும்பு 1; /* match coreutils 6.10 (sans error msg to stderr) */
	//if (dup2(1, 1) != 1) - old way
	//	return 1;

	/* bash builtin errors out on "printf '-%s-\n' foo",
	 * coreutils-6.9 works. Both work with "printf -- '-%s-\n' foo".
	 * We will mimic coreutils. */
	argv = skip_dash_dash(argv);

	எனில் (!argv[0]) {
		எனில் ((ENABLE_ASH_PRINTF || ENABLE_HUSH_PRINTF)
		 && applet_name[0] != 'p'
		) {
			bb_simple_error_செய்தி("usage: printf FORMAT [ARGUMENT...]");
			திரும்பு 2; /* bash compat */
		}
		bb_show_usage();
	}

	format = argv[0];
	argv2 = argv + 1;

	conv_err = 0;
	செய் {
		argv = argv2;
		argv2 = print_formatted(format, argv, &conv_err);
	} போது (argv2 > argv && *argv2);

	/* coreutils compat (bash doesn't do this):
	if (*argv)
		fprintf(stderr, "excess args ignored");
	*/

	திரும்பு (argv2 < argv) /* if true, print_formatted errored out */
		|| conv_err; /* print_formatted saw invalid number */
}
