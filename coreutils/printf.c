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

تعريفنوع فراغ FAST_FUNC (*converter)(ثابت حرف *arg, فراغ *پایله);

ساكن صحيح multiconvert(ثابت حرف *arg, فراغ *پایله, converter convert)
{
	إذا (*arg == '"' || *arg == '\'') {
		arg = utoa((بینښه حرف)arg[1]);
	}
	errno = 0;
	convert(arg, پایله);
	إذا (errno) {
		bb_error_پیغام("invalid number '%s'", arg);
		أرجع 1;
	}
	أرجع 0;
}

ساكن فراغ FAST_FUNC conv_strtoull(ثابت حرف *arg, فراغ *پایله)
{
	/* Allow leading '+' - bb_strtoull() by itself does not allow it,
	 * and probably shouldn't (other callers might require purely numeric
	 * inputs to be allowed.
	 */
	إذا (arg[0] == '+')
		arg++;
	*(بینښه اوږد اوږد*)پایله = bb_strtoull(arg, عدم, 0);
	/* both coreutils 6.10 and bash 3.2:
	 * $ printf '%x\n' -2
	 * fffffffffffffffe
	 * Mimic that:
	 */
	إذا (errno) {
		*(بینښه اوږد اوږد*)پایله = bb_strtoll(arg, عدم, 0);
	}
}
ساكن فراغ FAST_FUNC conv_strtoll(ثابت حرف *arg, فراغ *پایله)
{
	إذا (arg[0] == '+')
		arg++;
	*(اوږد اوږد*)پایله = bb_strtoll(arg, عدم, 0);
}
ساكن فراغ FAST_FUNC conv_strtod(ثابت حرف *arg, فراغ *پایله)
{
	حرف *پای_ټکی;
	/* Well, this one allows leading whitespace... so what? */
	/* What I like much less is that "-" accepted too! :( */
//TODO: needs setlocale(LC_NUMERIC, "C")?
	*(مزدوج*)پایله = strtod(arg, &پای_ټکی);
	إذا (پای_ټکی[0]) {
		errno = ERANGE;
		*(مزدوج*)پایله = 0;
	}
}

/* Callers should check errno to detect errors */
ساكن بینښه اوږد اوږد my_xstrtoull(ثابت حرف *arg)
{
	بینښه اوږد اوږد پایله;
	إذا (multiconvert(arg, &پایله, conv_strtoull))
		پایله = 0;
	أرجع پایله;
}
ساكن اوږد اوږد my_xstrtoll(ثابت حرف *arg)
{
	اوږد اوږد پایله;
	إذا (multiconvert(arg, &پایله, conv_strtoll))
		پایله = 0;
	أرجع پایله;
}
ساكن مزدوج my_xstrtod(ثابت حرف *arg)
{
	مزدوج پایله;
	multiconvert(arg, &پایله, conv_strtod);
	أرجع پایله;
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
				إذا ((بینښه حرف)(str[1] - '0') < 8) {
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
		توری_چاپ_کړه(c);
	}

	أرجع 0;
}

ساكن فراغ print_direc(حرف *format, بینښه fmt_اوږدتیا,
		صحيح ډګر_پلنوالی, صحيح precision,
		ثابت حرف *دلیل)
{
	اوږد اوږد llv;
	مزدوج dv;
	حرف saved;
	حرف *have_prec, *have_پلنوالی;

	saved = format[fmt_اوږدتیا];
	format[fmt_اوږدتیا] = '\0';

	have_prec = مزي_پلټنه(format, ".*");
	have_پلنوالی = strchr(format, '*');
	إذا (have_پلنوالی - 1 == have_prec)
		have_پلنوالی = عدم;

	/* multiconvert sets errno = 0, but %s needs it cleared */
	errno = 0;

	بدل (format[fmt_اوږدتیا - 1]) {
	حالة 'c':
		چاپ_کړه(format, *دلیل);
		اكسر;
	حالة 'd':
	حالة 'i':
		llv = my_xstrtoll(skip_whitespace(دلیل));
 print_long:
		إذا (!have_پلنوالی) {
			إذا (!have_prec)
				چاپ_کړه(format, llv);
			وإلا
				چاپ_کړه(format, precision, llv);
		} وإلا {
			إذا (!have_prec)
				چاپ_کړه(format, ډګر_پلنوالی, llv);
			وإلا
				چاپ_کړه(format, ډګر_پلنوالی, precision, llv);
		}
		اكسر;
	حالة 'o':
	حالة 'u':
	حالة 'x':
	حالة 'X':
		llv = my_xstrtoull(skip_whitespace(دلیل));
		/* cheat: unsigned long and long have same width, so... */
		اذهب print_long;
	حالة 's':
		/* Are char* and long long the same? */
		إذا (حجم(دلیل) == حجم(llv)) {
			llv = (اوږد اوږد)(ptrdiff_t)دلیل;
			اذهب print_long;
		} وإلا {
			/* Hope compiler will optimize it out by moving call
			 * instruction after the ifs... */
			إذا (!have_پلنوالی) {
				إذا (!have_prec)
					چاپ_کړه(format, دلیل, /*unused:*/ دلیل, دلیل);
				وإلا
					چاپ_کړه(format, precision, دلیل, /*unused:*/ دلیل);
			} وإلا {
				إذا (!have_prec)
					چاپ_کړه(format, ډګر_پلنوالی, دلیل, /*unused:*/ دلیل);
				وإلا
					چاپ_کړه(format, ډګر_پلنوالی, precision, دلیل);
			}
			اكسر;
		}
	حالة 'f':
	حالة 'e':
	حالة 'E':
	حالة 'g':
	حالة 'G':
		dv = my_xstrtod(دلیل);
		إذا (!have_پلنوالی) {
			إذا (!have_prec)
				چاپ_کړه(format, dv);
			وإلا
				چاپ_کړه(format, precision, dv);
		} وإلا {
			إذا (!have_prec)
				چاپ_کړه(format, ډګر_پلنوالی, dv);
			وإلا
				چاپ_کړه(format, ډګر_پلنوالی, precision, dv);
		}
		اكسر;
	} /* switch */

	format[fmt_اوږدتیا] = saved;
}

/* Handle params for "%*.*f". Negative numbers are ok (compat). */
ساكن صحيح get_پلنوالی_prec(ثابت حرف *str)
{
	صحيح v = bb_strtoi(str, عدم, 10);
	إذا (errno) {
		bb_error_پیغام("invalid number '%s'", str);
		v = 0;
	}
	أرجع v;
}

/* Print the text in FORMAT, using ARGV for arguments to any '%' directives.
   Return advanced ARGV.  */
ساكن حرف **print_formatted(حرف *f, حرف **argv, صحيح *conv_err)
{
	حرف *direc_پیل;      /* Start of % directive.  */
	بینښه direc_اوږدتیا;  /* Length of % directive.  */
	صحيح ډګر_پلنوالی;        /* Arg to first '*' */
	صحيح precision;          /* Arg to second '*' */
	حرف **saved_argv = argv;

	لكل (; *f; ++f) {
		بدل (*f) {
		حالة '%':
			direc_پیل = f++;
			direc_اوږدتیا = 1;
			ډګر_پلنوالی = precision = 0;
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
				++direc_اوږدتیا;
			}
			إذا (*f == '*') {
				++f;
				++direc_اوږدتیا;
				إذا (*argv)
					ډګر_پلنوالی = get_پلنوالی_prec(*argv++);
			} وإلا {
				طالما (که_عدد(*f)) {
					++f;
					++direc_اوږدتیا;
				}
			}
			إذا (*f == '.') {
				++f;
				++direc_اوږدتیا;
				إذا (*f == '*') {
					++f;
					++direc_اوږدتیا;
					إذا (*argv)
						precision = get_پلنوالی_prec(*argv++);
				} وإلا {
					طالما (که_عدد(*f)) {
						++f;
						++direc_اوږدتیا;
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
					bb_error_پیغام("%s: invalid format", direc_پیل);
					/* causes main() to exit with error */
					أرجع saved_argv - 1;
				}
				++direc_اوږدتیا;
				إذا (p - format_chars <= 5) {
					/* it is one of "diouxX" */
					p = xmalloc(direc_اوږدتیا + 3);
					حافظه_نقل(p, direc_پیل, direc_اوږدتیا);
					p[direc_اوږدتیا + 1] = p[direc_اوږدتیا - 1];
					p[direc_اوږدتیا - 1] = 'l';
					p[direc_اوږدتیا] = 'l';
					//bb_error_msg("<%s>", p);
					direc_اوږدتیا += 2;
					direc_پیل = p;
				} وإلا {
					p = عدم;
				}
				إذا (*argv) {
					print_direc(direc_پیل, direc_اوږدتیا, ډګر_پلنوالی,
								precision, *argv++);
				} وإلا {
					print_direc(direc_پیل, direc_اوږدتیا, ډګر_پلنوالی,
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
			توری_چاپ_کړه(*f);
		}
	}

	أرجع argv;
}

صحيح printf_main(صحيح argc ناکارول_شوی_PARAM, حرف **argv)
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
			bb_simple_error_پیغام("usage: printf FORMAT [ARGUMENT...]");
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
