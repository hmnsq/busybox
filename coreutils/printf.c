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

#ಅನ್ತರ್ಭಾವ "libbb.h"

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

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಶೂನ್ಯ FAST_FUNC (*converter)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *arg, ಶೂನ್ಯ *ಫಲಿತಾಂಶ);

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ multiconvert(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *arg, ಶೂನ್ಯ *ಫಲಿತಾಂಶ, converter convert)
{
	ಆದರೆ (*arg == '"' || *arg == '\'') {
		arg = utoa((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)arg[1]);
	}
	errno = 0;
	convert(arg, ಫಲಿತಾಂಶ);
	ಆದರೆ (errno) {
		bb_error_ಸಂದೇಶ("invalid number '%s'", arg);
		ಹಿಂತಿರುಗಿ 1;
	}
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ FAST_FUNC conv_strtoull(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *arg, ಶೂನ್ಯ *ಫಲಿತಾಂಶ)
{
	/* Allow leading '+' - bb_strtoull() by itself does not allow it,
	 * and probably shouldn't (other callers might require purely numeric
	 * inputs to be allowed.
	 */
	ಆದರೆ (arg[0] == '+')
		arg++;
	*(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ*)ಫಲಿತಾಂಶ = bb_strtoull(arg, ಖಾಲಿ, 0);
	/* both coreutils 6.10 and bash 3.2:
	 * $ printf '%x\n' -2
	 * fffffffffffffffe
	 * Mimic that:
	 */
	ಆದರೆ (errno) {
		*(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ*)ಫಲಿತಾಂಶ = bb_strtoll(arg, ಖಾಲಿ, 0);
	}
}
ಸ್ಥಿರ ಶೂನ್ಯ FAST_FUNC conv_strtoll(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *arg, ಶೂನ್ಯ *ಫಲಿತಾಂಶ)
{
	ಆದರೆ (arg[0] == '+')
		arg++;
	*(ದೀರ್ಘ ದೀರ್ಘ*)ಫಲಿತಾಂಶ = bb_strtoll(arg, ಖಾಲಿ, 0);
}
ಸ್ಥಿರ ಶೂನ್ಯ FAST_FUNC conv_strtod(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *arg, ಶೂನ್ಯ *ಫಲಿತಾಂಶ)
{
	ಅಕ್ಷರ *ಕೊನೆ;
	/* Well, this one allows leading whitespace... so what? */
	/* What I like much less is that "-" accepted too! :( */
//TODO: needs setlocale(LC_NUMERIC, "C")?
	*(ದ್ವಿಗುಣ*)ಫಲಿತಾಂಶ = strtod(arg, &ಕೊನೆ);
	ಆದರೆ (ಕೊನೆ[0]) {
		errno = ERANGE;
		*(ದ್ವಿಗುಣ*)ಫಲಿತಾಂಶ = 0;
	}
}

/* Callers should check errno to detect errors */
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ my_xstrtoull(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *arg)
{
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ ಫಲಿತಾಂಶ;
	ಆದರೆ (multiconvert(arg, &ಫಲಿತಾಂಶ, conv_strtoull))
		ಫಲಿತಾಂಶ = 0;
	ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}
ಸ್ಥಿರ ದೀರ್ಘ ದೀರ್ಘ my_xstrtoll(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *arg)
{
	ದೀರ್ಘ ದೀರ್ಘ ಫಲಿತಾಂಶ;
	ಆದರೆ (multiconvert(arg, &ಫಲಿತಾಂಶ, conv_strtoll))
		ಫಲಿತಾಂಶ = 0;
	ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}
ಸ್ಥಿರ ದ್ವಿಗುಣ my_xstrtod(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *arg)
{
	ದ್ವಿಗುಣ ಫಲಿತಾಂಶ;
	multiconvert(arg, &ಫಲಿತಾಂಶ, conv_strtod);
	ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

/* Handles %b; return 1 if output is to be short-circuited by \c */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ print_esc_string(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str)
{
	ಅಕ್ಷರ c;
	ಆಗ ((c = *str) != '\0') {
		str++;
		ಆದರೆ (c == '\\') {
			/* %b also accepts 4-digit octals of the form \0### */
			ಆದರೆ (*str == '0') {
				ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(str[1] - '0') < 8) {
					/* 2nd char is 0..7: skip leading '0' */
					str++;
				}
			}
			ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*str == 'c') {
				ಹಿಂತಿರುಗಿ 1;
			}
			{
				/* optimization: don't force arg to be on-stack,
				 * use another variable for that. */
				ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z = str;
				c = bb_process_escape_sequence(&z);
				str = z;
			}
		}
		ಅಕ್ಷರ_ಬರೆ(c);
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ print_direc(ಅಕ್ಷರ *format, ಚಿಹ್ನೆರಹಿತ fmt_ಉದ್ದಳತೆ,
		ಪೂರ್ಣಾಂಕ ಕ್ಷೇತ್ರ_ಅಗಲ, ಪೂರ್ಣಾಂಕ precision,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ವಾದ)
{
	ದೀರ್ಘ ದೀರ್ಘ llv;
	ದ್ವಿಗುಣ dv;
	ಅಕ್ಷರ saved;
	ಅಕ್ಷರ *have_prec, *have_ಅಗಲ;

	saved = format[fmt_ಉದ್ದಳತೆ];
	format[fmt_ಉದ್ದಳತೆ] = '\0';

	have_prec = ಅಕ್ಷರಸರಣಿ_ಹುಡುಕು(format, ".*");
	have_ಅಗಲ = strchr(format, '*');
	ಆದರೆ (have_ಅಗಲ - 1 == have_prec)
		have_ಅಗಲ = ಖಾಲಿ;

	/* multiconvert sets errno = 0, but %s needs it cleared */
	errno = 0;

	ಆಯ್ಕೆ (format[fmt_ಉದ್ದಳತೆ - 1]) {
	ಸ್ಥಿತಿ 'c':
		ಬರೆ(format, *ವಾದ);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ 'd':
	ಸ್ಥಿತಿ 'i':
		llv = my_xstrtoll(skip_whitespace(ವಾದ));
 print_long:
		ಆದರೆ (!have_ಅಗಲ) {
			ಆದರೆ (!have_prec)
				ಬರೆ(format, llv);
			ಇಲ್ಲದಿದ್ದರೆ
				ಬರೆ(format, precision, llv);
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (!have_prec)
				ಬರೆ(format, ಕ್ಷೇತ್ರ_ಅಗಲ, llv);
			ಇಲ್ಲದಿದ್ದರೆ
				ಬರೆ(format, ಕ್ಷೇತ್ರ_ಅಗಲ, precision, llv);
		}
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ 'o':
	ಸ್ಥಿತಿ 'u':
	ಸ್ಥಿತಿ 'x':
	ಸ್ಥಿತಿ 'X':
		llv = my_xstrtoull(skip_whitespace(ವಾದ));
		/* cheat: unsigned long and long have same width, so... */
		ಹೋಗು print_long;
	ಸ್ಥಿತಿ 's':
		/* Are char* and long long the same? */
		ಆದರೆ (ಗಾತ್ರ(ವಾದ) == ಗಾತ್ರ(llv)) {
			llv = (ದೀರ್ಘ ದೀರ್ಘ)(ptrdiff_t)ವಾದ;
			ಹೋಗು print_long;
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* Hope compiler will optimize it out by moving call
			 * instruction after the ifs... */
			ಆದರೆ (!have_ಅಗಲ) {
				ಆದರೆ (!have_prec)
					ಬರೆ(format, ವಾದ, /*unused:*/ ವಾದ, ವಾದ);
				ಇಲ್ಲದಿದ್ದರೆ
					ಬರೆ(format, precision, ವಾದ, /*unused:*/ ವಾದ);
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಆದರೆ (!have_prec)
					ಬರೆ(format, ಕ್ಷೇತ್ರ_ಅಗಲ, ವಾದ, /*unused:*/ ವಾದ);
				ಇಲ್ಲದಿದ್ದರೆ
					ಬರೆ(format, ಕ್ಷೇತ್ರ_ಅಗಲ, precision, ವಾದ);
			}
			ನಿಲ್ಲಿಸು;
		}
	ಸ್ಥಿತಿ 'f':
	ಸ್ಥಿತಿ 'e':
	ಸ್ಥಿತಿ 'E':
	ಸ್ಥಿತಿ 'g':
	ಸ್ಥಿತಿ 'G':
		dv = my_xstrtod(ವಾದ);
		ಆದರೆ (!have_ಅಗಲ) {
			ಆದರೆ (!have_prec)
				ಬರೆ(format, dv);
			ಇಲ್ಲದಿದ್ದರೆ
				ಬರೆ(format, precision, dv);
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (!have_prec)
				ಬರೆ(format, ಕ್ಷೇತ್ರ_ಅಗಲ, dv);
			ಇಲ್ಲದಿದ್ದರೆ
				ಬರೆ(format, ಕ್ಷೇತ್ರ_ಅಗಲ, precision, dv);
		}
		ನಿಲ್ಲಿಸು;
	} /* switch */

	format[fmt_ಉದ್ದಳತೆ] = saved;
}

/* Handle params for "%*.*f". Negative numbers are ok (compat). */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ get_ಅಗಲ_prec(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str)
{
	ಪೂರ್ಣಾಂಕ v = bb_strtoi(str, ಖಾಲಿ, 10);
	ಆದರೆ (errno) {
		bb_error_ಸಂದೇಶ("invalid number '%s'", str);
		v = 0;
	}
	ಹಿಂತಿರುಗಿ v;
}

/* Print the text in FORMAT, using ARGV for arguments to any '%' directives.
   Return advanced ARGV.  */
ಸ್ಥಿರ ಅಕ್ಷರ **print_formatted(ಅಕ್ಷರ *f, ಅಕ್ಷರ **argv, ಪೂರ್ಣಾಂಕ *conv_err)
{
	ಅಕ್ಷರ *direc_ಆರಂಭ;      /* Start of % directive.  */
	ಚಿಹ್ನೆರಹಿತ direc_ಉದ್ದಳತೆ;  /* Length of % directive.  */
	ಪೂರ್ಣಾಂಕ ಕ್ಷೇತ್ರ_ಅಗಲ;        /* Arg to first '*' */
	ಪೂರ್ಣಾಂಕ precision;          /* Arg to second '*' */
	ಅಕ್ಷರ **saved_argv = argv;

	ಗಾಗಿ (; *f; ++f) {
		ಆಯ್ಕೆ (*f) {
		ಸ್ಥಿತಿ '%':
			direc_ಆರಂಭ = f++;
			direc_ಉದ್ದಳತೆ = 1;
			ಕ್ಷೇತ್ರ_ಅಗಲ = precision = 0;
			ಆದರೆ (*f == '%') {
				bb_putchar('%');
				ನಿಲ್ಲಿಸು;
			}
			ಆದರೆ (*f == 'b') {
				ಆದರೆ (*argv) {
					ಆದರೆ (print_esc_string(*argv))
						ಹಿಂತಿರುಗಿ saved_argv; /* causes main() to exit */
					++argv;
				}
				ನಿಲ್ಲಿಸು;
			}
			ಆಗ (*f && strchr("-+ #0", *f)) {
				++f;
				++direc_ಉದ್ದಳತೆ;
			}
			ಆದರೆ (*f == '*') {
				++f;
				++direc_ಉದ್ದಳತೆ;
				ಆದರೆ (*argv)
					ಕ್ಷೇತ್ರ_ಅಗಲ = get_ಅಗಲ_prec(*argv++);
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಆಗ (ಅಂಕಿಯೇ(*f)) {
					++f;
					++direc_ಉದ್ದಳತೆ;
				}
			}
			ಆದರೆ (*f == '.') {
				++f;
				++direc_ಉದ್ದಳತೆ;
				ಆದರೆ (*f == '*') {
					++f;
					++direc_ಉದ್ದಳತೆ;
					ಆದರೆ (*argv)
						precision = get_ಅಗಲ_prec(*argv++);
				} ಇಲ್ಲದಿದ್ದರೆ {
					ಆಗ (ಅಂಕಿಯೇ(*f)) {
						++f;
						++direc_ಉದ್ದಳತೆ;
					}
				}
			}

			/* Remove "lLhz" size modifiers, repeatedly.
			 * bash does not like "%lld", but coreutils
			 * happily takes even "%Llllhhzhhzd"!
			 * We are permissive like coreutils */
			ಆಗ ((*f | 0x20) == 'l' || *f == 'h' || *f == 'z') {
				overlapping_strcpy(f, f + 1);
			}
			/* Add "ll" if integer modifier, then print */
			{
				ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ format_chars[] ALIGN1 = "diouxXfeEgGcs";
				ಅಕ್ಷರ *p = strchr(format_chars, *f);
				/* needed - try "printf %" without it */
				ಆದರೆ (p == ಖಾಲಿ || *f == '\0') {
					bb_error_ಸಂದೇಶ("%s: invalid format", direc_ಆರಂಭ);
					/* causes main() to exit with error */
					ಹಿಂತಿರುಗಿ saved_argv - 1;
				}
				++direc_ಉದ್ದಳತೆ;
				ಆದರೆ (p - format_chars <= 5) {
					/* it is one of "diouxX" */
					p = xmalloc(direc_ಉದ್ದಳತೆ + 3);
					ಸ್ಮರಣೆ_ನಕಲು(p, direc_ಆರಂಭ, direc_ಉದ್ದಳತೆ);
					p[direc_ಉದ್ದಳತೆ + 1] = p[direc_ಉದ್ದಳತೆ - 1];
					p[direc_ಉದ್ದಳತೆ - 1] = 'l';
					p[direc_ಉದ್ದಳತೆ] = 'l';
					//bb_error_msg("<%s>", p);
					direc_ಉದ್ದಳತೆ += 2;
					direc_ಆರಂಭ = p;
				} ಇಲ್ಲದಿದ್ದರೆ {
					p = ಖಾಲಿ;
				}
				ಆದರೆ (*argv) {
					print_direc(direc_ಆರಂಭ, direc_ಉದ್ದಳತೆ, ಕ್ಷೇತ್ರ_ಅಗಲ,
								precision, *argv++);
				} ಇಲ್ಲದಿದ್ದರೆ {
					print_direc(direc_ಆರಂಭ, direc_ಉದ್ದಳತೆ, ಕ್ಷೇತ್ರ_ಅಗಲ,
								precision, "");
				}
				*conv_err |= errno;
				ಬಿಡುಗಡೆ(p);
			}
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ '\\':
			ಆದರೆ (*++f == 'c') {
				ಹಿಂತಿರುಗಿ saved_argv; /* causes main() to exit */
			}
			bb_putchar(bb_process_escape_sequence((ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **)&f));
			f--;
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			ಅಕ್ಷರ_ಬರೆ(*f);
		}
	}

	ಹಿಂತಿರುಗಿ argv;
}

ಪೂರ್ಣಾಂಕ printf_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	ಪೂರ್ಣಾಂಕ conv_err;
	ಅಕ್ಷರ *format;
	ಅಕ್ಷರ **argv2;

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
	ಆದರೆ (fcntl(1, F_GETFL) == -1)
		ಹಿಂತಿರುಗಿ 1; /* match coreutils 6.10 (sans error msg to stderr) */
	//if (dup2(1, 1) != 1) - old way
	//	return 1;

	/* bash builtin errors out on "printf '-%s-\n' foo",
	 * coreutils-6.9 works. Both work with "printf -- '-%s-\n' foo".
	 * We will mimic coreutils. */
	argv = skip_dash_dash(argv);

	ಆದರೆ (!argv[0]) {
		ಆದರೆ ((ENABLE_ASH_PRINTF || ENABLE_HUSH_PRINTF)
		 && applet_name[0] != 'p'
		) {
			bb_simple_error_ಸಂದೇಶ("usage: printf FORMAT [ARGUMENT...]");
			ಹಿಂತಿರುಗಿ 2; /* bash compat */
		}
		bb_show_usage();
	}

	format = argv[0];
	argv2 = argv + 1;

	conv_err = 0;
	ಮಾಡು {
		argv = argv2;
		argv2 = print_formatted(format, argv, &conv_err);
	} ಆಗ (argv2 > argv && *argv2);

	/* coreutils compat (bash doesn't do this):
	if (*argv)
		fprintf(stderr, "excess args ignored");
	*/

	ಹಿಂತಿರುಗಿ (argv2 < argv) /* if true, print_formatted errored out */
		|| conv_err; /* print_formatted saw invalid number */
}
