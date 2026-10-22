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

#અન્તર્ભાવ "libbb.h"

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

પ્રકારવ્યાખ્યા શૂન્ય FAST_FUNC (*converter)(અચલ અક્ષર *arg, શૂન્ય *result);

સ્થિર પૂર્ણાંક multiconvert(અચલ અક્ષર *arg, શૂન્ય *result, converter convert)
{
	જો (*arg == '"' || *arg == '\'') {
		arg = utoa((unsigned અક્ષર)arg[1]);
	}
	errno = 0;
	convert(arg, result);
	જો (errno) {
		bb_error_સંદેશ("invalid number '%s'", arg);
		પાછા 1;
	}
	પાછા 0;
}

સ્થિર શૂન્ય FAST_FUNC conv_strtoull(અચલ અક્ષર *arg, શૂન્ય *result)
{
	/* Allow leading '+' - bb_strtoull() by itself does not allow it,
	 * and probably shouldn't (other callers might require purely numeric
	 * inputs to be allowed.
	 */
	જો (arg[0] == '+')
		arg++;
	*(unsigned long long*)result = bb_strtoull(arg, ખાલી, 0);
	/* both coreutils 6.10 and bash 3.2:
	 * $ printf '%x\n' -2
	 * fffffffffffffffe
	 * Mimic that:
	 */
	જો (errno) {
		*(unsigned long long*)result = bb_strtoll(arg, ખાલી, 0);
	}
}
સ્થિર શૂન્ય FAST_FUNC conv_strtoll(અચલ અક્ષર *arg, શૂન્ય *result)
{
	જો (arg[0] == '+')
		arg++;
	*(long long*)result = bb_strtoll(arg, ખાલી, 0);
}
સ્થિર શૂન્ય FAST_FUNC conv_strtod(અચલ અક્ષર *arg, શૂન્ય *result)
{
	અક્ષર *end;
	/* Well, this one allows leading whitespace... so what? */
	/* What I like much less is that "-" accepted too! :( */
//TODO: needs setlocale(LC_NUMERIC, "C")?
	*(દ્વિગુણ*)result = strtod(arg, &end);
	જો (end[0]) {
		errno = ERANGE;
		*(દ્વિગુણ*)result = 0;
	}
}

/* Callers should check errno to detect errors */
સ્થિર unsigned long long my_xstrtoull(અચલ અક્ષર *arg)
{
	unsigned long long result;
	જો (multiconvert(arg, &result, conv_strtoull))
		result = 0;
	પાછા result;
}
સ્થિર long long my_xstrtoll(અચલ અક્ષર *arg)
{
	long long result;
	જો (multiconvert(arg, &result, conv_strtoll))
		result = 0;
	પાછા result;
}
સ્થિર દ્વિગુણ my_xstrtod(અચલ અક્ષર *arg)
{
	દ્વિગુણ result;
	multiconvert(arg, &result, conv_strtod);
	પાછા result;
}

/* Handles %b; return 1 if output is to be short-circuited by \c */
સ્થિર પૂર્ણાંક print_esc_string(અચલ અક્ષર *str)
{
	અક્ષર c;
	જ્યારે ((c = *str) != '\0') {
		str++;
		જો (c == '\\') {
			/* %b also accepts 4-digit octals of the form \0### */
			જો (*str == '0') {
				જો ((unsigned અક્ષર)(str[1] - '0') < 8) {
					/* 2nd char is 0..7: skip leading '0' */
					str++;
				}
			}
			નહીંતર જો (*str == 'c') {
				પાછા 1;
			}
			{
				/* optimization: don't force arg to be on-stack,
				 * use another variable for that. */
				અચલ અક્ષર *z = str;
				c = bb_process_escape_sequence(&z);
				str = z;
			}
		}
		putchar(c);
	}

	પાછા 0;
}

સ્થિર શૂન્ય print_direc(અક્ષર *format, unsigned fmt_length,
		પૂર્ણાંક field_width, પૂર્ણાંક precision,
		અચલ અક્ષર *argument)
{
	long long llv;
	દ્વિગુણ dv;
	અક્ષર saved;
	અક્ષર *have_prec, *have_width;

	saved = format[fmt_length];
	format[fmt_length] = '\0';

	have_prec = strstr(format, ".*");
	have_width = strchr(format, '*');
	જો (have_width - 1 == have_prec)
		have_width = ખાલી;

	/* multiconvert sets errno = 0, but %s needs it cleared */
	errno = 0;

	પસંદગી (format[fmt_length - 1]) {
	સ્થિતિ 'c':
		printf(format, *argument);
		તોડો;
	સ્થિતિ 'd':
	સ્થિતિ 'i':
		llv = my_xstrtoll(skip_whitespace(argument));
 print_long:
		જો (!have_width) {
			જો (!have_prec)
				printf(format, llv);
			નહીંતર
				printf(format, precision, llv);
		} નહીંતર {
			જો (!have_prec)
				printf(format, field_width, llv);
			નહીંતર
				printf(format, field_width, precision, llv);
		}
		તોડો;
	સ્થિતિ 'o':
	સ્થિતિ 'u':
	સ્થિતિ 'x':
	સ્થિતિ 'X':
		llv = my_xstrtoull(skip_whitespace(argument));
		/* cheat: unsigned long and long have same width, so... */
		જાઓ print_long;
	સ્થિતિ 's':
		/* Are char* and long long the same? */
		જો (કદ(argument) == કદ(llv)) {
			llv = (long long)(ptrdiff_t)argument;
			જાઓ print_long;
		} નહીંતર {
			/* Hope compiler will optimize it out by moving call
			 * instruction after the ifs... */
			જો (!have_width) {
				જો (!have_prec)
					printf(format, argument, /*unused:*/ argument, argument);
				નહીંતર
					printf(format, precision, argument, /*unused:*/ argument);
			} નહીંતર {
				જો (!have_prec)
					printf(format, field_width, argument, /*unused:*/ argument);
				નહીંતર
					printf(format, field_width, precision, argument);
			}
			તોડો;
		}
	સ્થિતિ 'f':
	સ્થિતિ 'e':
	સ્થિતિ 'E':
	સ્થિતિ 'g':
	સ્થિતિ 'G':
		dv = my_xstrtod(argument);
		જો (!have_width) {
			જો (!have_prec)
				printf(format, dv);
			નહીંતર
				printf(format, precision, dv);
		} નહીંતર {
			જો (!have_prec)
				printf(format, field_width, dv);
			નહીંતર
				printf(format, field_width, precision, dv);
		}
		તોડો;
	} /* switch */

	format[fmt_length] = saved;
}

/* Handle params for "%*.*f". Negative numbers are ok (compat). */
સ્થિર પૂર્ણાંક get_width_prec(અચલ અક્ષર *str)
{
	પૂર્ણાંક v = bb_strtoi(str, ખાલી, 10);
	જો (errno) {
		bb_error_સંદેશ("invalid number '%s'", str);
		v = 0;
	}
	પાછા v;
}

/* Print the text in FORMAT, using ARGV for arguments to any '%' directives.
   Return advanced ARGV.  */
સ્થિર અક્ષર **print_formatted(અક્ષર *f, અક્ષર **argv, પૂર્ણાંક *conv_err)
{
	અક્ષર *direc_start;      /* Start of % directive.  */
	unsigned direc_length;  /* Length of % directive.  */
	પૂર્ણાંક field_width;        /* Arg to first '*' */
	પૂર્ણાંક precision;          /* Arg to second '*' */
	અક્ષર **saved_argv = argv;

	માટે (; *f; ++f) {
		પસંદગી (*f) {
		સ્થિતિ '%':
			direc_start = f++;
			direc_length = 1;
			field_width = precision = 0;
			જો (*f == '%') {
				bb_putchar('%');
				તોડો;
			}
			જો (*f == 'b') {
				જો (*argv) {
					જો (print_esc_string(*argv))
						પાછા saved_argv; /* causes main() to exit */
					++argv;
				}
				તોડો;
			}
			જ્યારે (*f && strchr("-+ #0", *f)) {
				++f;
				++direc_length;
			}
			જો (*f == '*') {
				++f;
				++direc_length;
				જો (*argv)
					field_width = get_width_prec(*argv++);
			} નહીંતર {
				જ્યારે (isdigit(*f)) {
					++f;
					++direc_length;
				}
			}
			જો (*f == '.') {
				++f;
				++direc_length;
				જો (*f == '*') {
					++f;
					++direc_length;
					જો (*argv)
						precision = get_width_prec(*argv++);
				} નહીંતર {
					જ્યારે (isdigit(*f)) {
						++f;
						++direc_length;
					}
				}
			}

			/* Remove "lLhz" size modifiers, repeatedly.
			 * bash does not like "%lld", but coreutils
			 * happily takes even "%Llllhhzhhzd"!
			 * We are permissive like coreutils */
			જ્યારે ((*f | 0x20) == 'l' || *f == 'h' || *f == 'z') {
				overlapping_strcpy(f, f + 1);
			}
			/* Add "ll" if integer modifier, then print */
			{
				સ્થિર અચલ અક્ષર format_chars[] ALIGN1 = "diouxXfeEgGcs";
				અક્ષર *p = strchr(format_chars, *f);
				/* needed - try "printf %" without it */
				જો (p == ખાલી || *f == '\0') {
					bb_error_સંદેશ("%s: invalid format", direc_start);
					/* causes main() to exit with error */
					પાછા saved_argv - 1;
				}
				++direc_length;
				જો (p - format_chars <= 5) {
					/* it is one of "diouxX" */
					p = xmalloc(direc_length + 3);
					memcpy(p, direc_start, direc_length);
					p[direc_length + 1] = p[direc_length - 1];
					p[direc_length - 1] = 'l';
					p[direc_length] = 'l';
					//bb_error_msg("<%s>", p);
					direc_length += 2;
					direc_start = p;
				} નહીંતર {
					p = ખાલી;
				}
				જો (*argv) {
					print_direc(direc_start, direc_length, field_width,
								precision, *argv++);
				} નહીંતર {
					print_direc(direc_start, direc_length, field_width,
								precision, "");
				}
				*conv_err |= errno;
				મુક્ત(p);
			}
			તોડો;
		સ્થિતિ '\\':
			જો (*++f == 'c') {
				પાછા saved_argv; /* causes main() to exit */
			}
			bb_putchar(bb_process_escape_sequence((અચલ અક્ષર **)&f));
			f--;
			તોડો;
		મૂળભૂત:
			putchar(*f);
		}
	}

	પાછા argv;
}

પૂર્ણાંક printf_main(પૂર્ણાંક argc UNUSED_PARAM, અક્ષર **argv)
{
	પૂર્ણાંક conv_err;
	અક્ષર *format;
	અક્ષર **argv2;

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
	જો (fcntl(1, F_GETFL) == -1)
		પાછા 1; /* match coreutils 6.10 (sans error msg to stderr) */
	//if (dup2(1, 1) != 1) - old way
	//	return 1;

	/* bash builtin errors out on "printf '-%s-\n' foo",
	 * coreutils-6.9 works. Both work with "printf -- '-%s-\n' foo".
	 * We will mimic coreutils. */
	argv = skip_dash_dash(argv);

	જો (!argv[0]) {
		જો ((ENABLE_ASH_PRINTF || ENABLE_HUSH_PRINTF)
		 && applet_name[0] != 'p'
		) {
			bb_simple_error_સંદેશ("usage: printf FORMAT [ARGUMENT...]");
			પાછા 2; /* bash compat */
		}
		bb_show_usage();
	}

	format = argv[0];
	argv2 = argv + 1;

	conv_err = 0;
	કરો {
		argv = argv2;
		argv2 = print_formatted(format, argv, &conv_err);
	} જ્યારે (argv2 > argv && *argv2);

	/* coreutils compat (bash doesn't do this):
	if (*argv)
		fprintf(stderr, "excess args ignored");
	*/

	પાછા (argv2 < argv) /* if true, print_formatted errored out */
		|| conv_err; /* print_formatted saw invalid number */
}
