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

تعريفنوع فراغ FAST_FUNC (*converter)(ثابت حرف *arg, فراغ *نتيجو);

ساكن صحيح multiconvert(ثابت حرف *arg, فراغ *نتيجو, converter convert)
{
	إذا (*arg == '"' || *arg == '\'') {
		arg = utoa((بنانشان حرف)arg[1]);
	}
	errno = 0;
	convert(arg, نتيجو);
	إذا (errno) {
		bb_error_نياپو("invalid number '%s'", arg);
		أرجع 1;
	}
	أرجع 0;
}

ساكن فراغ FAST_FUNC conv_strtoull(ثابت حرف *arg, فراغ *نتيجو)
{
	/* Allow leading '+' - bb_strtoull() by itself does not allow it,
	 * and probably shouldn't (other callers might require purely numeric
	 * inputs to be allowed.
	 */
	إذا (arg[0] == '+')
		arg++;
	*(بنانشان ڊگهو ڊگهو*)نتيجو = bb_strtoull(arg, عدم, 0);
	/* both coreutils 6.10 and bash 3.2:
	 * $ printf '%x\n' -2
	 * fffffffffffffffe
	 * Mimic that:
	 */
	إذا (errno) {
		*(بنانشان ڊگهو ڊگهو*)نتيجو = bb_strtoll(arg, عدم, 0);
	}
}
ساكن فراغ FAST_FUNC conv_strtoll(ثابت حرف *arg, فراغ *نتيجو)
{
	إذا (arg[0] == '+')
		arg++;
	*(ڊگهو ڊگهو*)نتيجو = bb_strtoll(arg, عدم, 0);
}
ساكن فراغ FAST_FUNC conv_strtod(ثابت حرف *arg, فراغ *نتيجو)
{
	حرف *پڇاڙي;
	/* Well, this one allows leading whitespace... so what? */
	/* What I like much less is that "-" accepted too! :( */
//TODO: needs setlocale(LC_NUMERIC, "C")?
	*(مزدوج*)نتيجو = strtod(arg, &پڇاڙي);
	إذا (پڇاڙي[0]) {
		errno = ERANGE;
		*(مزدوج*)نتيجو = 0;
	}
}

/* Callers should check errno to detect errors */
ساكن بنانشان ڊگهو ڊگهو my_xstrtoull(ثابت حرف *arg)
{
	بنانشان ڊگهو ڊگهو نتيجو;
	إذا (multiconvert(arg, &نتيجو, conv_strtoull))
		نتيجو = 0;
	أرجع نتيجو;
}
ساكن ڊگهو ڊگهو my_xstrtoll(ثابت حرف *arg)
{
	ڊگهو ڊگهو نتيجو;
	إذا (multiconvert(arg, &نتيجو, conv_strtoll))
		نتيجو = 0;
	أرجع نتيجو;
}
ساكن مزدوج my_xstrtod(ثابت حرف *arg)
{
	مزدوج نتيجو;
	multiconvert(arg, &نتيجو, conv_strtod);
	أرجع نتيجو;
}

/* Handles %b; return 1 if output is to be short-circuited by \c */
ساكن صحيح print_esc_string(ثابت حرف *str)
{
	حرف c;
	طالما ((c = *str) != '\0') {
		str++;
		إذا (c == '\\') {
			/* %b also accepts 4-digit octals of the form \0### */
			إذا (*str == '0') {
				إذا ((بنانشان حرف)(str[1] - '0') < 8) {
					/* 2nd char is 0..7: skip leading '0' */
					str++;
				}
			}
			وإلا إذا (*str == 'c') {
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
		اکر_لکو(c);
	}

	أرجع 0;
}

ساكن فراغ print_direc(حرف *format, بنانشان fmt_ڊيگهائي,
		صحيح ميدان_ويڪر, صحيح precision,
		ثابت حرف *دليل)
{
	ڊگهو ڊگهو llv;
	مزدوج dv;
	حرف saved;
	حرف *have_prec, *have_ويڪر;

	saved = format[fmt_ڊيگهائي];
	format[fmt_ڊيگهائي] = '\0';

	have_prec = لڙي_ڳولا(format, ".*");
	have_ويڪر = strchr(format, '*');
	إذا (have_ويڪر - 1 == have_prec)
		have_ويڪر = عدم;

	/* multiconvert sets errno = 0, but %s needs it cleared */
	errno = 0;

	بدل (format[fmt_ڊيگهائي - 1]) {
	حالة 'c':
		لکو(format, *دليل);
		اكسر;
	حالة 'd':
	حالة 'i':
		llv = my_xstrtoll(skip_whitespace(دليل));
 print_long:
		إذا (!have_ويڪر) {
			إذا (!have_prec)
				لکو(format, llv);
			وإلا
				لکو(format, precision, llv);
		} وإلا {
			إذا (!have_prec)
				لکو(format, ميدان_ويڪر, llv);
			وإلا
				لکو(format, ميدان_ويڪر, precision, llv);
		}
		اكسر;
	حالة 'o':
	حالة 'u':
	حالة 'x':
	حالة 'X':
		llv = my_xstrtoull(skip_whitespace(دليل));
		/* cheat: unsigned long and long have same width, so... */
		اذهب print_long;
	حالة 's':
		/* Are char* and long long the same? */
		إذا (حجم(دليل) == حجم(llv)) {
			llv = (ڊگهو ڊگهو)(ptrdiff_t)دليل;
			اذهب print_long;
		} وإلا {
			/* Hope compiler will optimize it out by moving call
			 * instruction after the ifs... */
			إذا (!have_ويڪر) {
				إذا (!have_prec)
					لکو(format, دليل, /*unused:*/ دليل, دليل);
				وإلا
					لکو(format, precision, دليل, /*unused:*/ دليل);
			} وإلا {
				إذا (!have_prec)
					لکو(format, ميدان_ويڪر, دليل, /*unused:*/ دليل);
				وإلا
					لکو(format, ميدان_ويڪر, precision, دليل);
			}
			اكسر;
		}
	حالة 'f':
	حالة 'e':
	حالة 'E':
	حالة 'g':
	حالة 'G':
		dv = my_xstrtod(دليل);
		إذا (!have_ويڪر) {
			إذا (!have_prec)
				لکو(format, dv);
			وإلا
				لکو(format, precision, dv);
		} وإلا {
			إذا (!have_prec)
				لکو(format, ميدان_ويڪر, dv);
			وإلا
				لکو(format, ميدان_ويڪر, precision, dv);
		}
		اكسر;
	} /* switch */

	format[fmt_ڊيگهائي] = saved;
}

/* Handle params for "%*.*f". Negative numbers are ok (compat). */
ساكن صحيح get_ويڪر_prec(ثابت حرف *str)
{
	صحيح v = bb_strtoi(str, عدم, 10);
	إذا (errno) {
		bb_error_نياپو("invalid number '%s'", str);
		v = 0;
	}
	أرجع v;
}

/* Print the text in FORMAT, using ARGV for arguments to any '%' directives.
   Return advanced ARGV.  */
ساكن حرف **print_formatted(حرف *f, حرف **argv, صحيح *conv_err)
{
	حرف *direc_شروعات;      /* Start of % directive.  */
	بنانشان direc_ڊيگهائي;  /* Length of % directive.  */
	صحيح ميدان_ويڪر;        /* Arg to first '*' */
	صحيح precision;          /* Arg to second '*' */
	حرف **saved_argv = argv;

	لكل (; *f; ++f) {
		بدل (*f) {
		حالة '%':
			direc_شروعات = f++;
			direc_ڊيگهائي = 1;
			ميدان_ويڪر = precision = 0;
			إذا (*f == '%') {
				bb_putchar('%');
				اكسر;
			}
			إذا (*f == 'b') {
				إذا (*argv) {
					إذا (print_esc_string(*argv))
						أرجع saved_argv; /* causes main() to exit */
					++argv;
				}
				اكسر;
			}
			طالما (*f && strchr("-+ #0", *f)) {
				++f;
				++direc_ڊيگهائي;
			}
			إذا (*f == '*') {
				++f;
				++direc_ڊيگهائي;
				إذا (*argv)
					ميدان_ويڪر = get_ويڪر_prec(*argv++);
			} وإلا {
				طالما (انگ_آهي(*f)) {
					++f;
					++direc_ڊيگهائي;
				}
			}
			إذا (*f == '.') {
				++f;
				++direc_ڊيگهائي;
				إذا (*f == '*') {
					++f;
					++direc_ڊيگهائي;
					إذا (*argv)
						precision = get_ويڪر_prec(*argv++);
				} وإلا {
					طالما (انگ_آهي(*f)) {
						++f;
						++direc_ڊيگهائي;
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
				إذا (p == عدم || *f == '\0') {
					bb_error_نياپو("%s: invalid format", direc_شروعات);
					/* causes main() to exit with error */
					أرجع saved_argv - 1;
				}
				++direc_ڊيگهائي;
				إذا (p - format_chars <= 5) {
					/* it is one of "diouxX" */
					p = xmalloc(direc_ڊيگهائي + 3);
					يادگيري_نقل(p, direc_شروعات, direc_ڊيگهائي);
					p[direc_ڊيگهائي + 1] = p[direc_ڊيگهائي - 1];
					p[direc_ڊيگهائي - 1] = 'l';
					p[direc_ڊيگهائي] = 'l';
					//bb_error_msg("<%s>", p);
					direc_ڊيگهائي += 2;
					direc_شروعات = p;
				} وإلا {
					p = عدم;
				}
				إذا (*argv) {
					print_direc(direc_شروعات, direc_ڊيگهائي, ميدان_ويڪر,
								precision, *argv++);
				} وإلا {
					print_direc(direc_شروعات, direc_ڊيگهائي, ميدان_ويڪر,
								precision, "");
				}
				*conv_err |= errno;
				حرر(p);
			}
			اكسر;
		حالة '\\':
			إذا (*++f == 'c') {
				أرجع saved_argv; /* causes main() to exit */
			}
			bb_putchar(bb_process_escape_sequence((ثابت حرف **)&f));
			f--;
			اكسر;
		افتراضي:
			اکر_لکو(*f);
		}
	}

	أرجع argv;
}

صحيح printf_main(صحيح argc اڻ_استعمال_ٿيل_PARAM, حرف **argv)
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
	إذا (fcntl(1, F_GETFL) == -1)
		أرجع 1; /* match coreutils 6.10 (sans error msg to stderr) */
	//if (dup2(1, 1) != 1) - old way
	//	return 1;

	/* bash builtin errors out on "printf '-%s-\n' foo",
	 * coreutils-6.9 works. Both work with "printf -- '-%s-\n' foo".
	 * We will mimic coreutils. */
	argv = skip_dash_dash(argv);

	إذا (!argv[0]) {
		إذا ((ENABLE_ASH_PRINTF || ENABLE_HUSH_PRINTF)
		 && applet_name[0] != 'p'
		) {
			bb_simple_error_نياپو("usage: printf FORMAT [ARGUMENT...]");
			أرجع 2; /* bash compat */
		}
		bb_show_usage();
	}

	format = argv[0];
	argv2 = argv + 1;

	conv_err = 0;
	افعل {
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
