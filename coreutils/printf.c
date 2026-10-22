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

#അന്തര്ഭാവ "libbb.h"

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

തരനിർവചനം ശൂന്യം FAST_FUNC (*converter)(സ്ഥിരം അക്ഷരം *arg, ശൂന്യം *result);

സ്ഥിതം പൂർണ്ണസംഖ്യ multiconvert(സ്ഥിരം അക്ഷരം *arg, ശൂന്യം *result, converter convert)
{
	എങ്കിൽ (*arg == '"' || *arg == '\'') {
		arg = utoa((unsigned അക്ഷരം)arg[1]);
	}
	errno = 0;
	convert(arg, result);
	എങ്കിൽ (errno) {
		bb_error_സന്ദേശം("invalid number '%s'", arg);
		തിരികെ 1;
	}
	തിരികെ 0;
}

സ്ഥിതം ശൂന്യം FAST_FUNC conv_strtoull(സ്ഥിരം അക്ഷരം *arg, ശൂന്യം *result)
{
	/* Allow leading '+' - bb_strtoull() by itself does not allow it,
	 * and probably shouldn't (other callers might require purely numeric
	 * inputs to be allowed.
	 */
	എങ്കിൽ (arg[0] == '+')
		arg++;
	*(unsigned long long*)result = bb_strtoull(arg, ശൂന്യ, 0);
	/* both coreutils 6.10 and bash 3.2:
	 * $ printf '%x\n' -2
	 * fffffffffffffffe
	 * Mimic that:
	 */
	എങ്കിൽ (errno) {
		*(unsigned long long*)result = bb_strtoll(arg, ശൂന്യ, 0);
	}
}
സ്ഥിതം ശൂന്യം FAST_FUNC conv_strtoll(സ്ഥിരം അക്ഷരം *arg, ശൂന്യം *result)
{
	എങ്കിൽ (arg[0] == '+')
		arg++;
	*(long long*)result = bb_strtoll(arg, ശൂന്യ, 0);
}
സ്ഥിതം ശൂന്യം FAST_FUNC conv_strtod(സ്ഥിരം അക്ഷരം *arg, ശൂന്യം *result)
{
	അക്ഷരം *end;
	/* Well, this one allows leading whitespace... so what? */
	/* What I like much less is that "-" accepted too! :( */
//TODO: needs setlocale(LC_NUMERIC, "C")?
	*(ഇരട്ട*)result = strtod(arg, &end);
	എങ്കിൽ (end[0]) {
		errno = ERANGE;
		*(ഇരട്ട*)result = 0;
	}
}

/* Callers should check errno to detect errors */
സ്ഥിതം unsigned long long my_xstrtoull(സ്ഥിരം അക്ഷരം *arg)
{
	unsigned long long result;
	എങ്കിൽ (multiconvert(arg, &result, conv_strtoull))
		result = 0;
	തിരികെ result;
}
സ്ഥിതം long long my_xstrtoll(സ്ഥിരം അക്ഷരം *arg)
{
	long long result;
	എങ്കിൽ (multiconvert(arg, &result, conv_strtoll))
		result = 0;
	തിരികെ result;
}
സ്ഥിതം ഇരട്ട my_xstrtod(സ്ഥിരം അക്ഷരം *arg)
{
	ഇരട്ട result;
	multiconvert(arg, &result, conv_strtod);
	തിരികെ result;
}

/* Handles %b; return 1 if output is to be short-circuited by \c */
സ്ഥിതം പൂർണ്ണസംഖ്യ print_esc_string(സ്ഥിരം അക്ഷരം *str)
{
	അക്ഷരം c;
	ആകുമ്പോൾ ((c = *str) != '\0') {
		str++;
		എങ്കിൽ (c == '\\') {
			/* %b also accepts 4-digit octals of the form \0### */
			എങ്കിൽ (*str == '0') {
				എങ്കിൽ ((unsigned അക്ഷരം)(str[1] - '0') < 8) {
					/* 2nd char is 0..7: skip leading '0' */
					str++;
				}
			}
			അല്ലെങ്കിൽ എങ്കിൽ (*str == 'c') {
				തിരികെ 1;
			}
			{
				/* optimization: don't force arg to be on-stack,
				 * use another variable for that. */
				സ്ഥിരം അക്ഷരം *z = str;
				c = bb_process_escape_sequence(&z);
				str = z;
			}
		}
		putchar(c);
	}

	തിരികെ 0;
}

സ്ഥിതം ശൂന്യം print_direc(അക്ഷരം *format, unsigned fmt_length,
		പൂർണ്ണസംഖ്യ field_width, പൂർണ്ണസംഖ്യ precision,
		സ്ഥിരം അക്ഷരം *argument)
{
	long long llv;
	ഇരട്ട dv;
	അക്ഷരം saved;
	അക്ഷരം *have_prec, *have_width;

	saved = format[fmt_length];
	format[fmt_length] = '\0';

	have_prec = strstr(format, ".*");
	have_width = strchr(format, '*');
	എങ്കിൽ (have_width - 1 == have_prec)
		have_width = ശൂന്യ;

	/* multiconvert sets errno = 0, but %s needs it cleared */
	errno = 0;

	തിരഞ്ഞെടുപ്പ് (format[fmt_length - 1]) {
	അവസ്ഥ 'c':
		printf(format, *argument);
		നിർത്തുക;
	അവസ്ഥ 'd':
	അവസ്ഥ 'i':
		llv = my_xstrtoll(skip_whitespace(argument));
 print_long:
		എങ്കിൽ (!have_width) {
			എങ്കിൽ (!have_prec)
				printf(format, llv);
			അല്ലെങ്കിൽ
				printf(format, precision, llv);
		} അല്ലെങ്കിൽ {
			എങ്കിൽ (!have_prec)
				printf(format, field_width, llv);
			അല്ലെങ്കിൽ
				printf(format, field_width, precision, llv);
		}
		നിർത്തുക;
	അവസ്ഥ 'o':
	അവസ്ഥ 'u':
	അവസ്ഥ 'x':
	അവസ്ഥ 'X':
		llv = my_xstrtoull(skip_whitespace(argument));
		/* cheat: unsigned long and long have same width, so... */
		പോകുക print_long;
	അവസ്ഥ 's':
		/* Are char* and long long the same? */
		എങ്കിൽ (വലുപ്പം(argument) == വലുപ്പം(llv)) {
			llv = (long long)(ptrdiff_t)argument;
			പോകുക print_long;
		} അല്ലെങ്കിൽ {
			/* Hope compiler will optimize it out by moving call
			 * instruction after the ifs... */
			എങ്കിൽ (!have_width) {
				എങ്കിൽ (!have_prec)
					printf(format, argument, /*unused:*/ argument, argument);
				അല്ലെങ്കിൽ
					printf(format, precision, argument, /*unused:*/ argument);
			} അല്ലെങ്കിൽ {
				എങ്കിൽ (!have_prec)
					printf(format, field_width, argument, /*unused:*/ argument);
				അല്ലെങ്കിൽ
					printf(format, field_width, precision, argument);
			}
			നിർത്തുക;
		}
	അവസ്ഥ 'f':
	അവസ്ഥ 'e':
	അവസ്ഥ 'E':
	അവസ്ഥ 'g':
	അവസ്ഥ 'G':
		dv = my_xstrtod(argument);
		എങ്കിൽ (!have_width) {
			എങ്കിൽ (!have_prec)
				printf(format, dv);
			അല്ലെങ്കിൽ
				printf(format, precision, dv);
		} അല്ലെങ്കിൽ {
			എങ്കിൽ (!have_prec)
				printf(format, field_width, dv);
			അല്ലെങ്കിൽ
				printf(format, field_width, precision, dv);
		}
		നിർത്തുക;
	} /* switch */

	format[fmt_length] = saved;
}

/* Handle params for "%*.*f". Negative numbers are ok (compat). */
സ്ഥിതം പൂർണ്ണസംഖ്യ get_width_prec(സ്ഥിരം അക്ഷരം *str)
{
	പൂർണ്ണസംഖ്യ v = bb_strtoi(str, ശൂന്യ, 10);
	എങ്കിൽ (errno) {
		bb_error_സന്ദേശം("invalid number '%s'", str);
		v = 0;
	}
	തിരികെ v;
}

/* Print the text in FORMAT, using ARGV for arguments to any '%' directives.
   Return advanced ARGV.  */
സ്ഥിതം അക്ഷരം **print_formatted(അക്ഷരം *f, അക്ഷരം **argv, പൂർണ്ണസംഖ്യ *conv_err)
{
	അക്ഷരം *direc_start;      /* Start of % directive.  */
	unsigned direc_length;  /* Length of % directive.  */
	പൂർണ്ണസംഖ്യ field_width;        /* Arg to first '*' */
	പൂർണ്ണസംഖ്യ precision;          /* Arg to second '*' */
	അക്ഷരം **saved_argv = argv;

	വേണ്ടി (; *f; ++f) {
		തിരഞ്ഞെടുപ്പ് (*f) {
		അവസ്ഥ '%':
			direc_start = f++;
			direc_length = 1;
			field_width = precision = 0;
			എങ്കിൽ (*f == '%') {
				bb_putchar('%');
				നിർത്തുക;
			}
			എങ്കിൽ (*f == 'b') {
				എങ്കിൽ (*argv) {
					എങ്കിൽ (print_esc_string(*argv))
						തിരികെ saved_argv; /* causes main() to exit */
					++argv;
				}
				നിർത്തുക;
			}
			ആകുമ്പോൾ (*f && strchr("-+ #0", *f)) {
				++f;
				++direc_length;
			}
			എങ്കിൽ (*f == '*') {
				++f;
				++direc_length;
				എങ്കിൽ (*argv)
					field_width = get_width_prec(*argv++);
			} അല്ലെങ്കിൽ {
				ആകുമ്പോൾ (isdigit(*f)) {
					++f;
					++direc_length;
				}
			}
			എങ്കിൽ (*f == '.') {
				++f;
				++direc_length;
				എങ്കിൽ (*f == '*') {
					++f;
					++direc_length;
					എങ്കിൽ (*argv)
						precision = get_width_prec(*argv++);
				} അല്ലെങ്കിൽ {
					ആകുമ്പോൾ (isdigit(*f)) {
						++f;
						++direc_length;
					}
				}
			}

			/* Remove "lLhz" size modifiers, repeatedly.
			 * bash does not like "%lld", but coreutils
			 * happily takes even "%Llllhhzhhzd"!
			 * We are permissive like coreutils */
			ആകുമ്പോൾ ((*f | 0x20) == 'l' || *f == 'h' || *f == 'z') {
				overlapping_strcpy(f, f + 1);
			}
			/* Add "ll" if integer modifier, then print */
			{
				സ്ഥിതം സ്ഥിരം അക്ഷരം format_chars[] ALIGN1 = "diouxXfeEgGcs";
				അക്ഷരം *p = strchr(format_chars, *f);
				/* needed - try "printf %" without it */
				എങ്കിൽ (p == ശൂന്യ || *f == '\0') {
					bb_error_സന്ദേശം("%s: invalid format", direc_start);
					/* causes main() to exit with error */
					തിരികെ saved_argv - 1;
				}
				++direc_length;
				എങ്കിൽ (p - format_chars <= 5) {
					/* it is one of "diouxX" */
					p = xmalloc(direc_length + 3);
					memcpy(p, direc_start, direc_length);
					p[direc_length + 1] = p[direc_length - 1];
					p[direc_length - 1] = 'l';
					p[direc_length] = 'l';
					//bb_error_msg("<%s>", p);
					direc_length += 2;
					direc_start = p;
				} അല്ലെങ്കിൽ {
					p = ശൂന്യ;
				}
				എങ്കിൽ (*argv) {
					print_direc(direc_start, direc_length, field_width,
								precision, *argv++);
				} അല്ലെങ്കിൽ {
					print_direc(direc_start, direc_length, field_width,
								precision, "");
				}
				*conv_err |= errno;
				സ്വതന്ത്രം(p);
			}
			നിർത്തുക;
		അവസ്ഥ '\\':
			എങ്കിൽ (*++f == 'c') {
				തിരികെ saved_argv; /* causes main() to exit */
			}
			bb_putchar(bb_process_escape_sequence((സ്ഥിരം അക്ഷരം **)&f));
			f--;
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
			putchar(*f);
		}
	}

	തിരികെ argv;
}

പൂർണ്ണസംഖ്യ printf_main(പൂർണ്ണസംഖ്യ argc UNUSED_PARAM, അക്ഷരം **argv)
{
	പൂർണ്ണസംഖ്യ conv_err;
	അക്ഷരം *format;
	അക്ഷരം **argv2;

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
	എങ്കിൽ (fcntl(1, F_GETFL) == -1)
		തിരികെ 1; /* match coreutils 6.10 (sans error msg to stderr) */
	//if (dup2(1, 1) != 1) - old way
	//	return 1;

	/* bash builtin errors out on "printf '-%s-\n' foo",
	 * coreutils-6.9 works. Both work with "printf -- '-%s-\n' foo".
	 * We will mimic coreutils. */
	argv = skip_dash_dash(argv);

	എങ്കിൽ (!argv[0]) {
		എങ്കിൽ ((ENABLE_ASH_PRINTF || ENABLE_HUSH_PRINTF)
		 && applet_name[0] != 'p'
		) {
			bb_simple_error_സന്ദേശം("usage: printf FORMAT [ARGUMENT...]");
			തിരികെ 2; /* bash compat */
		}
		bb_show_usage();
	}

	format = argv[0];
	argv2 = argv + 1;

	conv_err = 0;
	ചെയ്യുക {
		argv = argv2;
		argv2 = print_formatted(format, argv, &conv_err);
	} ആകുമ്പോൾ (argv2 > argv && *argv2);

	/* coreutils compat (bash doesn't do this):
	if (*argv)
		fprintf(stderr, "excess args ignored");
	*/

	തിരികെ (argv2 < argv) /* if true, print_formatted errored out */
		|| conv_err; /* print_formatted saw invalid number */
}
