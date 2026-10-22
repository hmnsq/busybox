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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"

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

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਖਾਲੀ FAST_FUNC (*converter)(ਅਚਲ ਅੱਖਰ *arg, ਖਾਲੀ *ਨਤੀਜਾ);

ਸਥਿਰ ਪੂਰਨ multiconvert(ਅਚਲ ਅੱਖਰ *arg, ਖਾਲੀ *ਨਤੀਜਾ, converter convert)
{
	ਜੇ (*arg == '"' || *arg == '\'') {
		arg = utoa((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)arg[1]);
	}
	errno = 0;
	convert(arg, ਨਤੀਜਾ);
	ਜੇ (errno) {
		bb_error_ਸੁਨੇਹਾ("invalid number '%s'", arg);
		ਵਾਪਸ 1;
	}
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਖਾਲੀ FAST_FUNC conv_strtoull(ਅਚਲ ਅੱਖਰ *arg, ਖਾਲੀ *ਨਤੀਜਾ)
{
	/* Allow leading '+' - bb_strtoull() by itself does not allow it,
	 * and probably shouldn't (other callers might require purely numeric
	 * inputs to be allowed.
	 */
	ਜੇ (arg[0] == '+')
		arg++;
	*(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ*)ਨਤੀਜਾ = bb_strtoull(arg, ਨਿਰਾ, 0);
	/* both coreutils 6.10 and bash 3.2:
	 * $ printf '%x\n' -2
	 * fffffffffffffffe
	 * Mimic that:
	 */
	ਜੇ (errno) {
		*(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ*)ਨਤੀਜਾ = bb_strtoll(arg, ਨਿਰਾ, 0);
	}
}
ਸਥਿਰ ਖਾਲੀ FAST_FUNC conv_strtoll(ਅਚਲ ਅੱਖਰ *arg, ਖਾਲੀ *ਨਤੀਜਾ)
{
	ਜੇ (arg[0] == '+')
		arg++;
	*(ਲੰਮਾ ਲੰਮਾ*)ਨਤੀਜਾ = bb_strtoll(arg, ਨਿਰਾ, 0);
}
ਸਥਿਰ ਖਾਲੀ FAST_FUNC conv_strtod(ਅਚਲ ਅੱਖਰ *arg, ਖਾਲੀ *ਨਤੀਜਾ)
{
	ਅੱਖਰ *ਸਿਰਾ;
	/* Well, this one allows leading whitespace... so what? */
	/* What I like much less is that "-" accepted too! :( */
//TODO: needs setlocale(LC_NUMERIC, "C")?
	*(ਦੁੱਗਣਾ*)ਨਤੀਜਾ = strtod(arg, &ਸਿਰਾ);
	ਜੇ (ਸਿਰਾ[0]) {
		errno = ERANGE;
		*(ਦੁੱਗਣਾ*)ਨਤੀਜਾ = 0;
	}
}

/* Callers should check errno to detect errors */
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ my_xstrtoull(ਅਚਲ ਅੱਖਰ *arg)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ ਨਤੀਜਾ;
	ਜੇ (multiconvert(arg, &ਨਤੀਜਾ, conv_strtoull))
		ਨਤੀਜਾ = 0;
	ਵਾਪਸ ਨਤੀਜਾ;
}
ਸਥਿਰ ਲੰਮਾ ਲੰਮਾ my_xstrtoll(ਅਚਲ ਅੱਖਰ *arg)
{
	ਲੰਮਾ ਲੰਮਾ ਨਤੀਜਾ;
	ਜੇ (multiconvert(arg, &ਨਤੀਜਾ, conv_strtoll))
		ਨਤੀਜਾ = 0;
	ਵਾਪਸ ਨਤੀਜਾ;
}
ਸਥਿਰ ਦੁੱਗਣਾ my_xstrtod(ਅਚਲ ਅੱਖਰ *arg)
{
	ਦੁੱਗਣਾ ਨਤੀਜਾ;
	multiconvert(arg, &ਨਤੀਜਾ, conv_strtod);
	ਵਾਪਸ ਨਤੀਜਾ;
}

/* Handles %b; return 1 if output is to be short-circuited by \c */
ਸਥਿਰ ਪੂਰਨ print_esc_string(ਅਚਲ ਅੱਖਰ *str)
{
	ਅੱਖਰ c;
	ਜਦੋਂ ((c = *str) != '\0') {
		str++;
		ਜੇ (c == '\\') {
			/* %b also accepts 4-digit octals of the form \0### */
			ਜੇ (*str == '0') {
				ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(str[1] - '0') < 8) {
					/* 2nd char is 0..7: skip leading '0' */
					str++;
				}
			}
			ਨਹੀਂਤਾਂ ਜੇ (*str == 'c') {
				ਵਾਪਸ 1;
			}
			{
				/* optimization: don't force arg to be on-stack,
				 * use another variable for that. */
				ਅਚਲ ਅੱਖਰ *z = str;
				c = bb_process_escape_sequence(&z);
				str = z;
			}
		}
		ਅੱਖਰ_ਲਿਖੋ(c);
	}

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਖਾਲੀ print_direc(ਅੱਖਰ *format, ਬਿਨਾਨਿਸ਼ਾਨੀ fmt_ਲੰਮਾਈ,
		ਪੂਰਨ ਖੇਤਰ_ਚੌੜਾਈ, ਪੂਰਨ precision,
		ਅਚਲ ਅੱਖਰ *ਦਲੀਲ)
{
	ਲੰਮਾ ਲੰਮਾ llv;
	ਦੁੱਗਣਾ dv;
	ਅੱਖਰ saved;
	ਅੱਖਰ *have_prec, *have_ਚੌੜਾਈ;

	saved = format[fmt_ਲੰਮਾਈ];
	format[fmt_ਲੰਮਾਈ] = '\0';

	have_prec = ਲੜੀ_ਖੋਜ(format, ".*");
	have_ਚੌੜਾਈ = strchr(format, '*');
	ਜੇ (have_ਚੌੜਾਈ - 1 == have_prec)
		have_ਚੌੜਾਈ = ਨਿਰਾ;

	/* multiconvert sets errno = 0, but %s needs it cleared */
	errno = 0;

	ਚੋਣ (format[fmt_ਲੰਮਾਈ - 1]) {
	ਹਾਲਤ 'c':
		ਲਿਖੋ(format, *ਦਲੀਲ);
		ਰੋਕੋ;
	ਹਾਲਤ 'd':
	ਹਾਲਤ 'i':
		llv = my_xstrtoll(skip_whitespace(ਦਲੀਲ));
 print_long:
		ਜੇ (!have_ਚੌੜਾਈ) {
			ਜੇ (!have_prec)
				ਲਿਖੋ(format, llv);
			ਨਹੀਂਤਾਂ
				ਲਿਖੋ(format, precision, llv);
		} ਨਹੀਂਤਾਂ {
			ਜੇ (!have_prec)
				ਲਿਖੋ(format, ਖੇਤਰ_ਚੌੜਾਈ, llv);
			ਨਹੀਂਤਾਂ
				ਲਿਖੋ(format, ਖੇਤਰ_ਚੌੜਾਈ, precision, llv);
		}
		ਰੋਕੋ;
	ਹਾਲਤ 'o':
	ਹਾਲਤ 'u':
	ਹਾਲਤ 'x':
	ਹਾਲਤ 'X':
		llv = my_xstrtoull(skip_whitespace(ਦਲੀਲ));
		/* cheat: unsigned long and long have same width, so... */
		ਜਾਓ print_long;
	ਹਾਲਤ 's':
		/* Are char* and long long the same? */
		ਜੇ (ਆਕਾਰ(ਦਲੀਲ) == ਆਕਾਰ(llv)) {
			llv = (ਲੰਮਾ ਲੰਮਾ)(ptrdiff_t)ਦਲੀਲ;
			ਜਾਓ print_long;
		} ਨਹੀਂਤਾਂ {
			/* Hope compiler will optimize it out by moving call
			 * instruction after the ifs... */
			ਜੇ (!have_ਚੌੜਾਈ) {
				ਜੇ (!have_prec)
					ਲਿਖੋ(format, ਦਲੀਲ, /*unused:*/ ਦਲੀਲ, ਦਲੀਲ);
				ਨਹੀਂਤਾਂ
					ਲਿਖੋ(format, precision, ਦਲੀਲ, /*unused:*/ ਦਲੀਲ);
			} ਨਹੀਂਤਾਂ {
				ਜੇ (!have_prec)
					ਲਿਖੋ(format, ਖੇਤਰ_ਚੌੜਾਈ, ਦਲੀਲ, /*unused:*/ ਦਲੀਲ);
				ਨਹੀਂਤਾਂ
					ਲਿਖੋ(format, ਖੇਤਰ_ਚੌੜਾਈ, precision, ਦਲੀਲ);
			}
			ਰੋਕੋ;
		}
	ਹਾਲਤ 'f':
	ਹਾਲਤ 'e':
	ਹਾਲਤ 'E':
	ਹਾਲਤ 'g':
	ਹਾਲਤ 'G':
		dv = my_xstrtod(ਦਲੀਲ);
		ਜੇ (!have_ਚੌੜਾਈ) {
			ਜੇ (!have_prec)
				ਲਿਖੋ(format, dv);
			ਨਹੀਂਤਾਂ
				ਲਿਖੋ(format, precision, dv);
		} ਨਹੀਂਤਾਂ {
			ਜੇ (!have_prec)
				ਲਿਖੋ(format, ਖੇਤਰ_ਚੌੜਾਈ, dv);
			ਨਹੀਂਤਾਂ
				ਲਿਖੋ(format, ਖੇਤਰ_ਚੌੜਾਈ, precision, dv);
		}
		ਰੋਕੋ;
	} /* switch */

	format[fmt_ਲੰਮਾਈ] = saved;
}

/* Handle params for "%*.*f". Negative numbers are ok (compat). */
ਸਥਿਰ ਪੂਰਨ get_ਚੌੜਾਈ_prec(ਅਚਲ ਅੱਖਰ *str)
{
	ਪੂਰਨ v = bb_strtoi(str, ਨਿਰਾ, 10);
	ਜੇ (errno) {
		bb_error_ਸੁਨੇਹਾ("invalid number '%s'", str);
		v = 0;
	}
	ਵਾਪਸ v;
}

/* Print the text in FORMAT, using ARGV for arguments to any '%' directives.
   Return advanced ARGV.  */
ਸਥਿਰ ਅੱਖਰ **print_formatted(ਅੱਖਰ *f, ਅੱਖਰ **argv, ਪੂਰਨ *conv_err)
{
	ਅੱਖਰ *direc_ਸ਼ੁਰੂ;      /* Start of % directive.  */
	ਬਿਨਾਨਿਸ਼ਾਨੀ direc_ਲੰਮਾਈ;  /* Length of % directive.  */
	ਪੂਰਨ ਖੇਤਰ_ਚੌੜਾਈ;        /* Arg to first '*' */
	ਪੂਰਨ precision;          /* Arg to second '*' */
	ਅੱਖਰ **saved_argv = argv;

	ਲਈ (; *f; ++f) {
		ਚੋਣ (*f) {
		ਹਾਲਤ '%':
			direc_ਸ਼ੁਰੂ = f++;
			direc_ਲੰਮਾਈ = 1;
			ਖੇਤਰ_ਚੌੜਾਈ = precision = 0;
			ਜੇ (*f == '%') {
				bb_putchar('%');
				ਰੋਕੋ;
			}
			ਜੇ (*f == 'b') {
				ਜੇ (*argv) {
					ਜੇ (print_esc_string(*argv))
						ਵਾਪਸ saved_argv; /* causes main() to exit */
					++argv;
				}
				ਰੋਕੋ;
			}
			ਜਦੋਂ (*f && strchr("-+ #0", *f)) {
				++f;
				++direc_ਲੰਮਾਈ;
			}
			ਜੇ (*f == '*') {
				++f;
				++direc_ਲੰਮਾਈ;
				ਜੇ (*argv)
					ਖੇਤਰ_ਚੌੜਾਈ = get_ਚੌੜਾਈ_prec(*argv++);
			} ਨਹੀਂਤਾਂ {
				ਜਦੋਂ (ਅੰਕ_ਹੈ(*f)) {
					++f;
					++direc_ਲੰਮਾਈ;
				}
			}
			ਜੇ (*f == '.') {
				++f;
				++direc_ਲੰਮਾਈ;
				ਜੇ (*f == '*') {
					++f;
					++direc_ਲੰਮਾਈ;
					ਜੇ (*argv)
						precision = get_ਚੌੜਾਈ_prec(*argv++);
				} ਨਹੀਂਤਾਂ {
					ਜਦੋਂ (ਅੰਕ_ਹੈ(*f)) {
						++f;
						++direc_ਲੰਮਾਈ;
					}
				}
			}

			/* Remove "lLhz" size modifiers, repeatedly.
			 * bash does not like "%lld", but coreutils
			 * happily takes even "%Llllhhzhhzd"!
			 * We are permissive like coreutils */
			ਜਦੋਂ ((*f | 0x20) == 'l' || *f == 'h' || *f == 'z') {
				overlapping_strcpy(f, f + 1);
			}
			/* Add "ll" if integer modifier, then print */
			{
				ਸਥਿਰ ਅਚਲ ਅੱਖਰ format_chars[] ALIGN1 = "diouxXfeEgGcs";
				ਅੱਖਰ *p = strchr(format_chars, *f);
				/* needed - try "printf %" without it */
				ਜੇ (p == ਨਿਰਾ || *f == '\0') {
					bb_error_ਸੁਨੇਹਾ("%s: invalid format", direc_ਸ਼ੁਰੂ);
					/* causes main() to exit with error */
					ਵਾਪਸ saved_argv - 1;
				}
				++direc_ਲੰਮਾਈ;
				ਜੇ (p - format_chars <= 5) {
					/* it is one of "diouxX" */
					p = xmalloc(direc_ਲੰਮਾਈ + 3);
					ਸਮ੍ਰਿਤੀ_ਨਕਲ(p, direc_ਸ਼ੁਰੂ, direc_ਲੰਮਾਈ);
					p[direc_ਲੰਮਾਈ + 1] = p[direc_ਲੰਮਾਈ - 1];
					p[direc_ਲੰਮਾਈ - 1] = 'l';
					p[direc_ਲੰਮਾਈ] = 'l';
					//bb_error_msg("<%s>", p);
					direc_ਲੰਮਾਈ += 2;
					direc_ਸ਼ੁਰੂ = p;
				} ਨਹੀਂਤਾਂ {
					p = ਨਿਰਾ;
				}
				ਜੇ (*argv) {
					print_direc(direc_ਸ਼ੁਰੂ, direc_ਲੰਮਾਈ, ਖੇਤਰ_ਚੌੜਾਈ,
								precision, *argv++);
				} ਨਹੀਂਤਾਂ {
					print_direc(direc_ਸ਼ੁਰੂ, direc_ਲੰਮਾਈ, ਖੇਤਰ_ਚੌੜਾਈ,
								precision, "");
				}
				*conv_err |= errno;
				ਮੁਕਤ(p);
			}
			ਰੋਕੋ;
		ਹਾਲਤ '\\':
			ਜੇ (*++f == 'c') {
				ਵਾਪਸ saved_argv; /* causes main() to exit */
			}
			bb_putchar(bb_process_escape_sequence((ਅਚਲ ਅੱਖਰ **)&f));
			f--;
			ਰੋਕੋ;
		ਮੂਲ:
			ਅੱਖਰ_ਲਿਖੋ(*f);
		}
	}

	ਵਾਪਸ argv;
}

ਪੂਰਨ printf_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
	ਪੂਰਨ conv_err;
	ਅੱਖਰ *format;
	ਅੱਖਰ **argv2;

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
	ਜੇ (fcntl(1, F_GETFL) == -1)
		ਵਾਪਸ 1; /* match coreutils 6.10 (sans error msg to stderr) */
	//if (dup2(1, 1) != 1) - old way
	//	return 1;

	/* bash builtin errors out on "printf '-%s-\n' foo",
	 * coreutils-6.9 works. Both work with "printf -- '-%s-\n' foo".
	 * We will mimic coreutils. */
	argv = skip_dash_dash(argv);

	ਜੇ (!argv[0]) {
		ਜੇ ((ENABLE_ASH_PRINTF || ENABLE_HUSH_PRINTF)
		 && applet_name[0] != 'p'
		) {
			bb_simple_error_ਸੁਨੇਹਾ("usage: printf FORMAT [ARGUMENT...]");
			ਵਾਪਸ 2; /* bash compat */
		}
		bb_show_usage();
	}

	format = argv[0];
	argv2 = argv + 1;

	conv_err = 0;
	ਕਰੋ {
		argv = argv2;
		argv2 = print_formatted(format, argv, &conv_err);
	} ਜਦੋਂ (argv2 > argv && *argv2);

	/* coreutils compat (bash doesn't do this):
	if (*argv)
		fprintf(stderr, "excess args ignored");
	*/

	ਵਾਪਸ (argv2 < argv) /* if true, print_formatted errored out */
		|| conv_err; /* print_formatted saw invalid number */
}
