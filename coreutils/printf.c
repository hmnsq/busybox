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

#ضمن "libbb.h"

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

تعريفنوع فراغ FAST_FUNC (*converter)(ثابت حرف *arg, فراغ *result);

ساكن صحيح multiconvert(ثابت حرف *arg, فراغ *result, converter convert)
{
	𐤀𐤌 (*arg == '"' || *arg == '\'') {
		arg = utoa((unsigned حرف)arg[1]);
	}
	errno = 0;
	convert(arg, result);
	𐤀𐤌 (errno) {
		bb_error_msg("invalid number '%s'", arg);
		أرجع 1;
	}
	أرجع 0;
}

ساكن فراغ FAST_FUNC conv_strtoull(ثابت حرف *arg, فراغ *result)
{
	/* Allow leading '+' - bb_strtoull() by itself does not allow it,
	 * and probably shouldn't (other callers might require purely numeric
	 * inputs to be allowed.
	 */
	𐤀𐤌 (arg[0] == '+')
		arg++;
	*(unsigned long long*)result = bb_strtoull(arg, عدم, 0);
	/* both coreutils 6.10 and bash 3.2:
	 * $ printf '%x\n' -2
	 * fffffffffffffffe
	 * Mimic that:
	 */
	𐤀𐤌 (errno) {
		*(unsigned long long*)result = bb_strtoll(arg, عدم, 0);
	}
}
ساكن فراغ FAST_FUNC conv_strtoll(ثابت حرف *arg, فراغ *result)
{
	𐤀𐤌 (arg[0] == '+')
		arg++;
	*(long long*)result = bb_strtoll(arg, عدم, 0);
}
ساكن فراغ FAST_FUNC conv_strtod(ثابت حرف *arg, فراغ *result)
{
	حرف *end;
	/* Well, this one allows leading whitespace... so what? */
	/* What I like much less is that "-" accepted too! :( */
//TODO: needs setlocale(LC_NUMERIC, "C")?
	*(مزدوج*)result = strtod(arg, &end);
	𐤀𐤌 (end[0]) {
		errno = ERANGE;
		*(مزدوج*)result = 0;
	}
}

/* Callers should check errno to detect errors */
ساكن unsigned long long my_xstrtoull(ثابت حرف *arg)
{
	unsigned long long result;
	𐤀𐤌 (multiconvert(arg, &result, conv_strtoull))
		result = 0;
	أرجع result;
}
ساكن long long my_xstrtoll(ثابت حرف *arg)
{
	long long result;
	𐤀𐤌 (multiconvert(arg, &result, conv_strtoll))
		result = 0;
	أرجع result;
}
ساكن مزدوج my_xstrtod(ثابت حرف *arg)
{
	مزدوج result;
	multiconvert(arg, &result, conv_strtod);
	أرجع result;
}

/* Handles %b; return 1 if output is to be short-circuited by \c */
ساكن صحيح print_esc_string(ثابت حرف *str)
{
	حرف c;
	طالما ((c = *str) != '\0') {
		str++;
		𐤀𐤌 (c == '\\') {
			/* %b also accepts 4-digit octals of the form \0### */
			𐤀𐤌 (*str == '0') {
				𐤀𐤌 ((unsigned حرف)(str[1] - '0') < 8) {
					/* 2nd char is 0..7: skip leading '0' */
					str++;
				}
			}
			وإلا 𐤀𐤌 (*str == 'c') {
				أرجع 1;
			}
			{
				/* optimization: don't force arg to be on-stack,
				 * use another variable for that. */
				ثابت حرف *z = str;
				c = bb_process_escape_sequence(&z);
				str = z;
			}
		}
		putchar(c);
	}

	أرجع 0;
}

ساكن فراغ print_direc(حرف *format, unsigned fmt_length,
		صحيح field_width, صحيح precision,
		ثابت حرف *argument)
{
	long long llv;
	مزدوج dv;
	حرف saved;
	حرف *have_prec, *have_width;

	saved = format[fmt_length];
	format[fmt_length] = '\0';

	have_prec = strstr(format, ".*");
	have_width = strchr(format, '*');
	𐤀𐤌 (have_width - 1 == have_prec)
		have_width = عدم;

	/* multiconvert sets errno = 0, but %s needs it cleared */
	errno = 0;

	بدل (format[fmt_length - 1]) {
	حالة 'c':
		printf(format, *argument);
		اكسر;
	حالة 'd':
	حالة 'i':
		llv = my_xstrtoll(skip_whitespace(argument));
 print_long:
		𐤀𐤌 (!have_width) {
			𐤀𐤌 (!have_prec)
				printf(format, llv);
			وإلا
				printf(format, precision, llv);
		} وإلا {
			𐤀𐤌 (!have_prec)
				printf(format, field_width, llv);
			وإلا
				printf(format, field_width, precision, llv);
		}
		اكسر;
	حالة 'o':
	حالة 'u':
	حالة 'x':
	حالة 'X':
		llv = my_xstrtoull(skip_whitespace(argument));
		/* cheat: unsigned long and long have same width, so... */
		اذهب print_long;
	حالة 's':
		/* Are char* and long long the same? */
		𐤀𐤌 (حجم(argument) == حجم(llv)) {
			llv = (long long)(ptrdiff_t)argument;
			اذهب print_long;
		} وإلا {
			/* Hope compiler will optimize it out by moving call
			 * instruction after the ifs... */
			𐤀𐤌 (!have_width) {
				𐤀𐤌 (!have_prec)
					printf(format, argument, /*unused:*/ argument, argument);
				وإلا
					printf(format, precision, argument, /*unused:*/ argument);
			} وإلا {
				𐤀𐤌 (!have_prec)
					printf(format, field_width, argument, /*unused:*/ argument);
				وإلا
					printf(format, field_width, precision, argument);
			}
			اكسر;
		}
	حالة 'f':
	حالة 'e':
	حالة 'E':
	حالة 'g':
	حالة 'G':
		dv = my_xstrtod(argument);
		𐤀𐤌 (!have_width) {
			𐤀𐤌 (!have_prec)
				printf(format, dv);
			وإلا
				printf(format, precision, dv);
		} وإلا {
			𐤀𐤌 (!have_prec)
				printf(format, field_width, dv);
			وإلا
				printf(format, field_width, precision, dv);
		}
		اكسر;
	} /* switch */

	format[fmt_length] = saved;
}

/* Handle params for "%*.*f". Negative numbers are ok (compat). */
ساكن صحيح get_width_prec(ثابت حرف *str)
{
	صحيح v = bb_strtoi(str, عدم, 10);
	𐤀𐤌 (errno) {
		bb_error_msg("invalid number '%s'", str);
		v = 0;
	}
	أرجع v;
}

/* Print the text in FORMAT, using ARGV for arguments to any '%' directives.
   Return advanced ARGV.  */
ساكن حرف **print_formatted(حرف *f, حرف **argv, صحيح *conv_err)
{
	حرف *direc_start;      /* Start of % directive.  */
	unsigned direc_length;  /* Length of % directive.  */
	صحيح field_width;        /* Arg to first '*' */
	صحيح precision;          /* Arg to second '*' */
	حرف **saved_argv = argv;

	لكل (; *f; ++f) {
		بدل (*f) {
		حالة '%':
			direc_start = f++;
			direc_length = 1;
			field_width = precision = 0;
			𐤀𐤌 (*f == '%') {
				bb_putchar('%');
				اكسر;
			}
			𐤀𐤌 (*f == 'b') {
				𐤀𐤌 (*argv) {
					𐤀𐤌 (print_esc_string(*argv))
						أرجع saved_argv; /* causes main() to exit */
					++argv;
				}
				اكسر;
			}
			طالما (*f && strchr("-+ #0", *f)) {
				++f;
				++direc_length;
			}
			𐤀𐤌 (*f == '*') {
				++f;
				++direc_length;
				𐤀𐤌 (*argv)
					field_width = get_width_prec(*argv++);
			} وإلا {
				طالما (isdigit(*f)) {
					++f;
					++direc_length;
				}
			}
			𐤀𐤌 (*f == '.') {
				++f;
				++direc_length;
				𐤀𐤌 (*f == '*') {
					++f;
					++direc_length;
					𐤀𐤌 (*argv)
						precision = get_width_prec(*argv++);
				} وإلا {
					طالما (isdigit(*f)) {
						++f;
						++direc_length;
					}
				}
			}

			/* Remove "lLhz" size modifiers, repeatedly.
			 * bash does not like "%lld", but coreutils
			 * happily takes even "%Llllhhzhhzd"!
			 * We are permissive like coreutils */
			طالما ((*f | 0x20) == 'l' || *f == 'h' || *f == 'z') {
				overlapping_strcpy(f, f + 1);
			}
			/* Add "ll" if integer modifier, then print */
			{
				ساكن ثابت حرف format_chars[] ALIGN1 = "diouxXfeEgGcs";
				حرف *p = strchr(format_chars, *f);
				/* needed - try "printf %" without it */
				𐤀𐤌 (p == عدم || *f == '\0') {
					bb_error_msg("%s: invalid format", direc_start);
					/* causes main() to exit with error */
					أرجع saved_argv - 1;
				}
				++direc_length;
				𐤀𐤌 (p - format_chars <= 5) {
					/* it is one of "diouxX" */
					p = xmalloc(direc_length + 3);
					memcpy(p, direc_start, direc_length);
					p[direc_length + 1] = p[direc_length - 1];
					p[direc_length - 1] = 'l';
					p[direc_length] = 'l';
					//bb_error_msg("<%s>", p);
					direc_length += 2;
					direc_start = p;
				} وإلا {
					p = عدم;
				}
				𐤀𐤌 (*argv) {
					print_direc(direc_start, direc_length, field_width,
								precision, *argv++);
				} وإلا {
					print_direc(direc_start, direc_length, field_width,
								precision, "");
				}
				*conv_err |= errno;
				حرر(p);
			}
			اكسر;
		حالة '\\':
			𐤀𐤌 (*++f == 'c') {
				أرجع saved_argv; /* causes main() to exit */
			}
			bb_putchar(bb_process_escape_sequence((ثابت حرف **)&f));
			f--;
			اكسر;
		افتراضي:
			putchar(*f);
		}
	}

	أرجع argv;
}

صحيح printf_main(صحيح argc UNUSED_PARAM, حرف **argv)
{
	صحيح conv_err;
	حرف *format;
	حرف **argv2;

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
	𐤀𐤌 (fcntl(1, F_GETFL) == -1)
		أرجع 1; /* match coreutils 6.10 (sans error msg to stderr) */
	//if (dup2(1, 1) != 1) - old way
	//	return 1;

	/* bash builtin errors out on "printf '-%s-\n' foo",
	 * coreutils-6.9 works. Both work with "printf -- '-%s-\n' foo".
	 * We will mimic coreutils. */
	argv = skip_dash_dash(argv);

	𐤀𐤌 (!argv[0]) {
		𐤀𐤌 ((ENABLE_ASH_PRINTF || ENABLE_HUSH_PRINTF)
		 && applet_name[0] != 'p'
		) {
			bb_simple_error_msg("usage: printf FORMAT [ARGUMENT...]");
			أرجع 2; /* bash compat */
		}
		bb_show_usage();
	}

	format = argv[0];
	argv2 = argv + 1;

	conv_err = 0;
	𐤐𐤏𐤋 {
		argv = argv2;
		argv2 = print_formatted(format, argv, &conv_err);
	} طالما (argv2 > argv && *argv2);

	/* coreutils compat (bash doesn't do this):
	if (*argv)
		fprintf(stderr, "excess args ignored");
	*/

	أرجع (argv2 < argv) /* if true, print_formatted errored out */
		|| conv_err; /* print_formatted saw invalid number */
}
