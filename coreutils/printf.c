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

قسمتعریف خالی FAST_FUNC (*converter)(ثابت حرف *arg, خالی *نتیجہ);

ساکن صحیح multiconvert(ثابت حرف *arg, خالی *نتیجہ, converter convert)
{
	اگر (*arg == '"' || *arg == '\'') {
		arg = utoa((غیرعلامتی حرف)arg[1]);
	}
	errno = 0;
	convert(arg, نتیجہ);
	اگر (errno) {
		bb_error_پیغام("invalid number '%s'", arg);
		واپس 1;
	}
	واپس 0;
}

ساکن خالی FAST_FUNC conv_strtoull(ثابت حرف *arg, خالی *نتیجہ)
{
	/* Allow leading '+' - bb_strtoull() by itself does not allow it,
	 * and probably shouldn't (other callers might require purely numeric
	 * inputs to be allowed.
	 */
	اگر (arg[0] == '+')
		arg++;
	*(غیرعلامتی طویل طویل*)نتیجہ = bb_strtoull(arg, تہی, 0);
	/* both coreutils 6.10 and bash 3.2:
	 * $ printf '%x\n' -2
	 * fffffffffffffffe
	 * Mimic that:
	 */
	اگر (errno) {
		*(غیرعلامتی طویل طویل*)نتیجہ = bb_strtoll(arg, تہی, 0);
	}
}
ساکن خالی FAST_FUNC conv_strtoll(ثابت حرف *arg, خالی *نتیجہ)
{
	اگر (arg[0] == '+')
		arg++;
	*(طویل طویل*)نتیجہ = bb_strtoll(arg, تہی, 0);
}
ساکن خالی FAST_FUNC conv_strtod(ثابت حرف *arg, خالی *نتیجہ)
{
	حرف *انتہا;
	/* Well, this one allows leading whitespace... so what? */
	/* What I like much less is that "-" accepted too! :( */
//TODO: needs setlocale(LC_NUMERIC, "C")?
	*(دوہرا*)نتیجہ = strtod(arg, &انتہا);
	اگر (انتہا[0]) {
		errno = ERANGE;
		*(دوہرا*)نتیجہ = 0;
	}
}

/* Callers should check errno to detect errors */
ساکن غیرعلامتی طویل طویل my_xstrtoull(ثابت حرف *arg)
{
	غیرعلامتی طویل طویل نتیجہ;
	اگر (multiconvert(arg, &نتیجہ, conv_strtoull))
		نتیجہ = 0;
	واپس نتیجہ;
}
ساکن طویل طویل my_xstrtoll(ثابت حرف *arg)
{
	طویل طویل نتیجہ;
	اگر (multiconvert(arg, &نتیجہ, conv_strtoll))
		نتیجہ = 0;
	واپس نتیجہ;
}
ساکن دوہرا my_xstrtod(ثابت حرف *arg)
{
	دوہرا نتیجہ;
	multiconvert(arg, &نتیجہ, conv_strtod);
	واپس نتیجہ;
}

/* Handles %b; return 1 if output is to be short-circuited by \c */
ساکن صحیح print_esc_string(ثابت حرف *str)
{
	حرف c;
	جب ((c = *str) != '\0') {
		str++;
		اگر (c == '\\') {
			/* %b also accepts 4-digit octals of the form \0### */
			اگر (*str == '0') {
				اگر ((غیرعلامتی حرف)(str[1] - '0') < 8) {
					/* 2nd char is 0..7: skip leading '0' */
					str++;
				}
			}
			ورنہ اگر (*str == 'c') {
				واپس 1;
			}
			{
				/* optimization: don't force arg to be on-stack,
				 * use another variable for that. */
				ثابت حرف *z = str;
				c = bb_process_escape_sequence(&z);
				str = z;
			}
		}
		حرف_لکھو(c);
	}

	واپس 0;
}

ساکن خالی print_direc(حرف *format, غیرعلامتی fmt_لمبائی,
		صحیح میدان_چوڑائی, صحیح precision,
		ثابت حرف *دلیل)
{
	طویل طویل llv;
	دوہرا dv;
	حرف saved;
	حرف *have_prec, *have_چوڑائی;

	saved = format[fmt_لمبائی];
	format[fmt_لمبائی] = '\0';

	have_prec = لڑی_تلاش(format, ".*");
	have_چوڑائی = strchr(format, '*');
	اگر (have_چوڑائی - 1 == have_prec)
		have_چوڑائی = تہی;

	/* multiconvert sets errno = 0, but %s needs it cleared */
	errno = 0;

	انتخاب (format[fmt_لمبائی - 1]) {
	حالت 'c':
		لکھو(format, *دلیل);
		توڑو;
	حالت 'd':
	حالت 'i':
		llv = my_xstrtoll(skip_whitespace(دلیل));
 print_long:
		اگر (!have_چوڑائی) {
			اگر (!have_prec)
				لکھو(format, llv);
			ورنہ
				لکھو(format, precision, llv);
		} ورنہ {
			اگر (!have_prec)
				لکھو(format, میدان_چوڑائی, llv);
			ورنہ
				لکھو(format, میدان_چوڑائی, precision, llv);
		}
		توڑو;
	حالت 'o':
	حالت 'u':
	حالت 'x':
	حالت 'X':
		llv = my_xstrtoull(skip_whitespace(دلیل));
		/* cheat: unsigned long and long have same width, so... */
		جاؤ print_long;
	حالت 's':
		/* Are char* and long long the same? */
		اگر (حجم(دلیل) == حجم(llv)) {
			llv = (طویل طویل)(ptrdiff_t)دلیل;
			جاؤ print_long;
		} ورنہ {
			/* Hope compiler will optimize it out by moving call
			 * instruction after the ifs... */
			اگر (!have_چوڑائی) {
				اگر (!have_prec)
					لکھو(format, دلیل, /*unused:*/ دلیل, دلیل);
				ورنہ
					لکھو(format, precision, دلیل, /*unused:*/ دلیل);
			} ورنہ {
				اگر (!have_prec)
					لکھو(format, میدان_چوڑائی, دلیل, /*unused:*/ دلیل);
				ورنہ
					لکھو(format, میدان_چوڑائی, precision, دلیل);
			}
			توڑو;
		}
	حالت 'f':
	حالت 'e':
	حالت 'E':
	حالت 'g':
	حالت 'G':
		dv = my_xstrtod(دلیل);
		اگر (!have_چوڑائی) {
			اگر (!have_prec)
				لکھو(format, dv);
			ورنہ
				لکھو(format, precision, dv);
		} ورنہ {
			اگر (!have_prec)
				لکھو(format, میدان_چوڑائی, dv);
			ورنہ
				لکھو(format, میدان_چوڑائی, precision, dv);
		}
		توڑو;
	} /* switch */

	format[fmt_لمبائی] = saved;
}

/* Handle params for "%*.*f". Negative numbers are ok (compat). */
ساکن صحیح get_چوڑائی_prec(ثابت حرف *str)
{
	صحیح v = bb_strtoi(str, تہی, 10);
	اگر (errno) {
		bb_error_پیغام("invalid number '%s'", str);
		v = 0;
	}
	واپس v;
}

/* Print the text in FORMAT, using ARGV for arguments to any '%' directives.
   Return advanced ARGV.  */
ساکن حرف **print_formatted(حرف *f, حرف **argv, صحیح *conv_err)
{
	حرف *direc_آغاز;      /* Start of % directive.  */
	غیرعلامتی direc_لمبائی;  /* Length of % directive.  */
	صحیح میدان_چوڑائی;        /* Arg to first '*' */
	صحیح precision;          /* Arg to second '*' */
	حرف **saved_argv = argv;

	برائے (; *f; ++f) {
		انتخاب (*f) {
		حالت '%':
			direc_آغاز = f++;
			direc_لمبائی = 1;
			میدان_چوڑائی = precision = 0;
			اگر (*f == '%') {
				bb_putchar('%');
				توڑو;
			}
			اگر (*f == 'b') {
				اگر (*argv) {
					اگر (print_esc_string(*argv))
						واپس saved_argv; /* causes main() to exit */
					++argv;
				}
				توڑو;
			}
			جب (*f && strchr("-+ #0", *f)) {
				++f;
				++direc_لمبائی;
			}
			اگر (*f == '*') {
				++f;
				++direc_لمبائی;
				اگر (*argv)
					میدان_چوڑائی = get_چوڑائی_prec(*argv++);
			} ورنہ {
				جب (ہندسہ_ہے(*f)) {
					++f;
					++direc_لمبائی;
				}
			}
			اگر (*f == '.') {
				++f;
				++direc_لمبائی;
				اگر (*f == '*') {
					++f;
					++direc_لمبائی;
					اگر (*argv)
						precision = get_چوڑائی_prec(*argv++);
				} ورنہ {
					جب (ہندسہ_ہے(*f)) {
						++f;
						++direc_لمبائی;
					}
				}
			}

			/* Remove "lLhz" size modifiers, repeatedly.
			 * bash does not like "%lld", but coreutils
			 * happily takes even "%Llllhhzhhzd"!
			 * We are permissive like coreutils */
			جب ((*f | 0x20) == 'l' || *f == 'h' || *f == 'z') {
				overlapping_strcpy(f, f + 1);
			}
			/* Add "ll" if integer modifier, then print */
			{
				ساکن ثابت حرف format_chars[] ALIGN1 = "diouxXfeEgGcs";
				حرف *p = strchr(format_chars, *f);
				/* needed - try "printf %" without it */
				اگر (p == تہی || *f == '\0') {
					bb_error_پیغام("%s: invalid format", direc_آغاز);
					/* causes main() to exit with error */
					واپس saved_argv - 1;
				}
				++direc_لمبائی;
				اگر (p - format_chars <= 5) {
					/* it is one of "diouxX" */
					p = xmalloc(direc_لمبائی + 3);
					حافظہ_نقل(p, direc_آغاز, direc_لمبائی);
					p[direc_لمبائی + 1] = p[direc_لمبائی - 1];
					p[direc_لمبائی - 1] = 'l';
					p[direc_لمبائی] = 'l';
					//bb_error_msg("<%s>", p);
					direc_لمبائی += 2;
					direc_آغاز = p;
				} ورنہ {
					p = تہی;
				}
				اگر (*argv) {
					print_direc(direc_آغاز, direc_لمبائی, میدان_چوڑائی,
								precision, *argv++);
				} ورنہ {
					print_direc(direc_آغاز, direc_لمبائی, میدان_چوڑائی,
								precision, "");
				}
				*conv_err |= errno;
				آزاد(p);
			}
			توڑو;
		حالت '\\':
			اگر (*++f == 'c') {
				واپس saved_argv; /* causes main() to exit */
			}
			bb_putchar(bb_process_escape_sequence((ثابت حرف **)&f));
			f--;
			توڑو;
		طےشدہ:
			حرف_لکھو(*f);
		}
	}

	واپس argv;
}

صحیح printf_main(صحیح argc غیر_مستعمل_PARAM, حرف **argv)
{
	صحیح conv_err;
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
	اگر (fcntl(1, F_GETFL) == -1)
		واپس 1; /* match coreutils 6.10 (sans error msg to stderr) */
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
			bb_simple_error_پیغام("usage: printf FORMAT [ARGUMENT...]");
			واپس 2; /* bash compat */
		}
		bb_show_usage();
	}

	format = argv[0];
	argv2 = argv + 1;

	conv_err = 0;
	کرو {
		argv = argv2;
		argv2 = print_formatted(format, argv, &conv_err);
	} جب (argv2 > argv && *argv2);

	/* coreutils compat (bash doesn't do this):
	if (*argv)
		fprintf(stderr, "excess args ignored");
	*/

	واپس (argv2 < argv) /* if true, print_formatted errored out */
		|| conv_err; /* print_formatted saw invalid number */
}
