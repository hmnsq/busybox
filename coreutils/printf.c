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

#ଅନ୍ତର୍ଭାଵ "libbb.h"

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

ପ୍ରକାରବ୍ୟାଖ୍ୟା ଶୂନ୍ୟ FAST_FUNC (*converter)(ଅଚଳ ଅକ୍ଷର *arg, ଶୂନ୍ୟ *result);

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା multiconvert(ଅଚଳ ଅକ୍ଷର *arg, ଶୂନ୍ୟ *result, converter convert)
{
	ଯଦି (*arg == '"' || *arg == '\'') {
		arg = utoa((unsigned ଅକ୍ଷର)arg[1]);
	}
	errno = 0;
	convert(arg, result);
	ଯଦି (errno) {
		bb_error_ସନ୍ଦେଶ("invalid number '%s'", arg);
		ଫେରନ୍ତୁ 1;
	}
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ଶୂନ୍ୟ FAST_FUNC conv_strtoull(ଅଚଳ ଅକ୍ଷର *arg, ଶୂନ୍ୟ *result)
{
	/* Allow leading '+' - bb_strtoull() by itself does not allow it,
	 * and probably shouldn't (other callers might require purely numeric
	 * inputs to be allowed.
	 */
	ଯଦି (arg[0] == '+')
		arg++;
	*(unsigned long long*)result = bb_strtoull(arg, ଖାଲି, 0);
	/* both coreutils 6.10 and bash 3.2:
	 * $ printf '%x\n' -2
	 * fffffffffffffffe
	 * Mimic that:
	 */
	ଯଦି (errno) {
		*(unsigned long long*)result = bb_strtoll(arg, ଖାଲି, 0);
	}
}
ସ୍ଥିର ଶୂନ୍ୟ FAST_FUNC conv_strtoll(ଅଚଳ ଅକ୍ଷର *arg, ଶୂନ୍ୟ *result)
{
	ଯଦି (arg[0] == '+')
		arg++;
	*(long long*)result = bb_strtoll(arg, ଖାଲି, 0);
}
ସ୍ଥିର ଶୂନ୍ୟ FAST_FUNC conv_strtod(ଅଚଳ ଅକ୍ଷର *arg, ଶୂନ୍ୟ *result)
{
	ଅକ୍ଷର *end;
	/* Well, this one allows leading whitespace... so what? */
	/* What I like much less is that "-" accepted too! :( */
//TODO: needs setlocale(LC_NUMERIC, "C")?
	*(ଦ୍ୱିଗୁଣ*)result = strtod(arg, &end);
	ଯଦି (end[0]) {
		errno = ERANGE;
		*(ଦ୍ୱିଗୁଣ*)result = 0;
	}
}

/* Callers should check errno to detect errors */
ସ୍ଥିର unsigned long long my_xstrtoull(ଅଚଳ ଅକ୍ଷର *arg)
{
	unsigned long long result;
	ଯଦି (multiconvert(arg, &result, conv_strtoull))
		result = 0;
	ଫେରନ୍ତୁ result;
}
ସ୍ଥିର long long my_xstrtoll(ଅଚଳ ଅକ୍ଷର *arg)
{
	long long result;
	ଯଦି (multiconvert(arg, &result, conv_strtoll))
		result = 0;
	ଫେରନ୍ତୁ result;
}
ସ୍ଥିର ଦ୍ୱିଗୁଣ my_xstrtod(ଅଚଳ ଅକ୍ଷର *arg)
{
	ଦ୍ୱିଗୁଣ result;
	multiconvert(arg, &result, conv_strtod);
	ଫେରନ୍ତୁ result;
}

/* Handles %b; return 1 if output is to be short-circuited by \c */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା print_esc_string(ଅଚଳ ଅକ୍ଷର *str)
{
	ଅକ୍ଷର c;
	ଯେତେବେଳେ ((c = *str) != '\0') {
		str++;
		ଯଦି (c == '\\') {
			/* %b also accepts 4-digit octals of the form \0### */
			ଯଦି (*str == '0') {
				ଯଦି ((unsigned ଅକ୍ଷର)(str[1] - '0') < 8) {
					/* 2nd char is 0..7: skip leading '0' */
					str++;
				}
			}
			ଅନ୍ୟଥା ଯଦି (*str == 'c') {
				ଫେରନ୍ତୁ 1;
			}
			{
				/* optimization: don't force arg to be on-stack,
				 * use another variable for that. */
				ଅଚଳ ଅକ୍ଷର *z = str;
				c = bb_process_escape_sequence(&z);
				str = z;
			}
		}
		putchar(c);
	}

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ଶୂନ୍ୟ print_direc(ଅକ୍ଷର *format, unsigned fmt_length,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା field_width, ପୂର୍ଣ୍ଣସଂଖ୍ୟା precision,
		ଅଚଳ ଅକ୍ଷର *argument)
{
	long long llv;
	ଦ୍ୱିଗୁଣ dv;
	ଅକ୍ଷର saved;
	ଅକ୍ଷର *have_prec, *have_width;

	saved = format[fmt_length];
	format[fmt_length] = '\0';

	have_prec = strstr(format, ".*");
	have_width = strchr(format, '*');
	ଯଦି (have_width - 1 == have_prec)
		have_width = ଖାଲି;

	/* multiconvert sets errno = 0, but %s needs it cleared */
	errno = 0;

	ବାଛ (format[fmt_length - 1]) {
	ସ୍ଥିତି 'c':
		printf(format, *argument);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି 'd':
	ସ୍ଥିତି 'i':
		llv = my_xstrtoll(skip_whitespace(argument));
 print_long:
		ଯଦି (!have_width) {
			ଯଦି (!have_prec)
				printf(format, llv);
			ଅନ୍ୟଥା
				printf(format, precision, llv);
		} ଅନ୍ୟଥା {
			ଯଦି (!have_prec)
				printf(format, field_width, llv);
			ଅନ୍ୟଥା
				printf(format, field_width, precision, llv);
		}
		ଭାଙ୍ଗ;
	ସ୍ଥିତି 'o':
	ସ୍ଥିତି 'u':
	ସ୍ଥିତି 'x':
	ସ୍ଥିତି 'X':
		llv = my_xstrtoull(skip_whitespace(argument));
		/* cheat: unsigned long and long have same width, so... */
		ଯାଅ print_long;
	ସ୍ଥିତି 's':
		/* Are char* and long long the same? */
		ଯଦି (ଆକାର(argument) == ଆକାର(llv)) {
			llv = (long long)(ptrdiff_t)argument;
			ଯାଅ print_long;
		} ଅନ୍ୟଥା {
			/* Hope compiler will optimize it out by moving call
			 * instruction after the ifs... */
			ଯଦି (!have_width) {
				ଯଦି (!have_prec)
					printf(format, argument, /*unused:*/ argument, argument);
				ଅନ୍ୟଥା
					printf(format, precision, argument, /*unused:*/ argument);
			} ଅନ୍ୟଥା {
				ଯଦି (!have_prec)
					printf(format, field_width, argument, /*unused:*/ argument);
				ଅନ୍ୟଥା
					printf(format, field_width, precision, argument);
			}
			ଭାଙ୍ଗ;
		}
	ସ୍ଥିତି 'f':
	ସ୍ଥିତି 'e':
	ସ୍ଥିତି 'E':
	ସ୍ଥିତି 'g':
	ସ୍ଥିତି 'G':
		dv = my_xstrtod(argument);
		ଯଦି (!have_width) {
			ଯଦି (!have_prec)
				printf(format, dv);
			ଅନ୍ୟଥା
				printf(format, precision, dv);
		} ଅନ୍ୟଥା {
			ଯଦି (!have_prec)
				printf(format, field_width, dv);
			ଅନ୍ୟଥା
				printf(format, field_width, precision, dv);
		}
		ଭାଙ୍ଗ;
	} /* switch */

	format[fmt_length] = saved;
}

/* Handle params for "%*.*f". Negative numbers are ok (compat). */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା get_width_prec(ଅଚଳ ଅକ୍ଷର *str)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା v = bb_strtoi(str, ଖାଲି, 10);
	ଯଦି (errno) {
		bb_error_ସନ୍ଦେଶ("invalid number '%s'", str);
		v = 0;
	}
	ଫେରନ୍ତୁ v;
}

/* Print the text in FORMAT, using ARGV for arguments to any '%' directives.
   Return advanced ARGV.  */
ସ୍ଥିର ଅକ୍ଷର **print_formatted(ଅକ୍ଷର *f, ଅକ୍ଷର **argv, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *conv_err)
{
	ଅକ୍ଷର *direc_start;      /* Start of % directive.  */
	unsigned direc_length;  /* Length of % directive.  */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା field_width;        /* Arg to first '*' */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା precision;          /* Arg to second '*' */
	ଅକ୍ଷର **saved_argv = argv;

	ପାଇଁ (; *f; ++f) {
		ବାଛ (*f) {
		ସ୍ଥିତି '%':
			direc_start = f++;
			direc_length = 1;
			field_width = precision = 0;
			ଯଦି (*f == '%') {
				bb_putchar('%');
				ଭାଙ୍ଗ;
			}
			ଯଦି (*f == 'b') {
				ଯଦି (*argv) {
					ଯଦି (print_esc_string(*argv))
						ଫେରନ୍ତୁ saved_argv; /* causes main() to exit */
					++argv;
				}
				ଭାଙ୍ଗ;
			}
			ଯେତେବେଳେ (*f && strchr("-+ #0", *f)) {
				++f;
				++direc_length;
			}
			ଯଦି (*f == '*') {
				++f;
				++direc_length;
				ଯଦି (*argv)
					field_width = get_width_prec(*argv++);
			} ଅନ୍ୟଥା {
				ଯେତେବେଳେ (isdigit(*f)) {
					++f;
					++direc_length;
				}
			}
			ଯଦି (*f == '.') {
				++f;
				++direc_length;
				ଯଦି (*f == '*') {
					++f;
					++direc_length;
					ଯଦି (*argv)
						precision = get_width_prec(*argv++);
				} ଅନ୍ୟଥା {
					ଯେତେବେଳେ (isdigit(*f)) {
						++f;
						++direc_length;
					}
				}
			}

			/* Remove "lLhz" size modifiers, repeatedly.
			 * bash does not like "%lld", but coreutils
			 * happily takes even "%Llllhhzhhzd"!
			 * We are permissive like coreutils */
			ଯେତେବେଳେ ((*f | 0x20) == 'l' || *f == 'h' || *f == 'z') {
				overlapping_strcpy(f, f + 1);
			}
			/* Add "ll" if integer modifier, then print */
			{
				ସ୍ଥିର ଅଚଳ ଅକ୍ଷର format_chars[] ALIGN1 = "diouxXfeEgGcs";
				ଅକ୍ଷର *p = strchr(format_chars, *f);
				/* needed - try "printf %" without it */
				ଯଦି (p == ଖାଲି || *f == '\0') {
					bb_error_ସନ୍ଦେଶ("%s: invalid format", direc_start);
					/* causes main() to exit with error */
					ଫେରନ୍ତୁ saved_argv - 1;
				}
				++direc_length;
				ଯଦି (p - format_chars <= 5) {
					/* it is one of "diouxX" */
					p = xmalloc(direc_length + 3);
					memcpy(p, direc_start, direc_length);
					p[direc_length + 1] = p[direc_length - 1];
					p[direc_length - 1] = 'l';
					p[direc_length] = 'l';
					//bb_error_msg("<%s>", p);
					direc_length += 2;
					direc_start = p;
				} ଅନ୍ୟଥା {
					p = ଖାଲି;
				}
				ଯଦି (*argv) {
					print_direc(direc_start, direc_length, field_width,
								precision, *argv++);
				} ଅନ୍ୟଥା {
					print_direc(direc_start, direc_length, field_width,
								precision, "");
				}
				*conv_err |= errno;
				ମୁକ୍ତ(p);
			}
			ଭାଙ୍ଗ;
		ସ୍ଥିତି '\\':
			ଯଦି (*++f == 'c') {
				ଫେରନ୍ତୁ saved_argv; /* causes main() to exit */
			}
			bb_putchar(bb_process_escape_sequence((ଅଚଳ ଅକ୍ଷର **)&f));
			f--;
			ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			putchar(*f);
		}
	}

	ଫେରନ୍ତୁ argv;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା printf_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc UNUSED_PARAM, ଅକ୍ଷର **argv)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା conv_err;
	ଅକ୍ଷର *format;
	ଅକ୍ଷର **argv2;

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
	ଯଦି (fcntl(1, F_GETFL) == -1)
		ଫେରନ୍ତୁ 1; /* match coreutils 6.10 (sans error msg to stderr) */
	//if (dup2(1, 1) != 1) - old way
	//	return 1;

	/* bash builtin errors out on "printf '-%s-\n' foo",
	 * coreutils-6.9 works. Both work with "printf -- '-%s-\n' foo".
	 * We will mimic coreutils. */
	argv = skip_dash_dash(argv);

	ଯଦି (!argv[0]) {
		ଯଦି ((ENABLE_ASH_PRINTF || ENABLE_HUSH_PRINTF)
		 && applet_name[0] != 'p'
		) {
			bb_simple_error_ସନ୍ଦେଶ("usage: printf FORMAT [ARGUMENT...]");
			ଫେରନ୍ତୁ 2; /* bash compat */
		}
		bb_show_usage();
	}

	format = argv[0];
	argv2 = argv + 1;

	conv_err = 0;
	କର {
		argv = argv2;
		argv2 = print_formatted(format, argv, &conv_err);
	} ଯେତେବେଳେ (argv2 > argv && *argv2);

	/* coreutils compat (bash doesn't do this):
	if (*argv)
		fprintf(stderr, "excess args ignored");
	*/

	ଫେରନ୍ତୁ (argv2 < argv) /* if true, print_formatted errored out */
		|| conv_err; /* print_formatted saw invalid number */
}
