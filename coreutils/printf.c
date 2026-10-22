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

#अन्तर्भाव "libbb.h"

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

प्रकारनिर्देश शून्य FAST_FUNC (*converter)(ध्रुव वर्ण *arg, शून्य *result);

स्थिर पूर्णाङ्क multiconvert(ध्रुव वर्ण *arg, शून्य *result, converter convert)
{
	जइ (*arg == '"' || *arg == '\'') {
		arg = utoa((unsigned वर्ण)arg[1]);
	}
	errno = 0;
	convert(arg, result);
	जइ (errno) {
		bb_error_सन्देशः("invalid number '%s'", arg);
		प्रत्यागम 1;
	}
	प्रत्यागम 0;
}

स्थिर शून्य FAST_FUNC conv_strtoull(ध्रुव वर्ण *arg, शून्य *result)
{
	/* Allow leading '+' - bb_strtoull() by itself does not allow it,
	 * and probably shouldn't (other callers might require purely numeric
	 * inputs to be allowed.
	 */
	जइ (arg[0] == '+')
		arg++;
	*(unsigned long long*)result = bb_strtoull(arg, रिक्त, 0);
	/* both coreutils 6.10 and bash 3.2:
	 * $ printf '%x\n' -2
	 * fffffffffffffffe
	 * Mimic that:
	 */
	जइ (errno) {
		*(unsigned long long*)result = bb_strtoll(arg, रिक्त, 0);
	}
}
स्थिर शून्य FAST_FUNC conv_strtoll(ध्रुव वर्ण *arg, शून्य *result)
{
	जइ (arg[0] == '+')
		arg++;
	*(long long*)result = bb_strtoll(arg, रिक्त, 0);
}
स्थिर शून्य FAST_FUNC conv_strtod(ध्रुव वर्ण *arg, शून्य *result)
{
	वर्ण *end;
	/* Well, this one allows leading whitespace... so what? */
	/* What I like much less is that "-" accepted too! :( */
//TODO: needs setlocale(LC_NUMERIC, "C")?
	*(द्विगुण*)result = strtod(arg, &end);
	जइ (end[0]) {
		errno = ERANGE;
		*(द्विगुण*)result = 0;
	}
}

/* Callers should check errno to detect errors */
स्थिर unsigned long long my_xstrtoull(ध्रुव वर्ण *arg)
{
	unsigned long long result;
	जइ (multiconvert(arg, &result, conv_strtoull))
		result = 0;
	प्रत्यागम result;
}
स्थिर long long my_xstrtoll(ध्रुव वर्ण *arg)
{
	long long result;
	जइ (multiconvert(arg, &result, conv_strtoll))
		result = 0;
	प्रत्यागम result;
}
स्थिर द्विगुण my_xstrtod(ध्रुव वर्ण *arg)
{
	द्विगुण result;
	multiconvert(arg, &result, conv_strtod);
	प्रत्यागम result;
}

/* Handles %b; return 1 if output is to be short-circuited by \c */
स्थिर पूर्णाङ्क print_esc_string(ध्रुव वर्ण *str)
{
	वर्ण c;
	जाव ((c = *str) != '\0') {
		str++;
		जइ (c == '\\') {
			/* %b also accepts 4-digit octals of the form \0### */
			जइ (*str == '0') {
				जइ ((unsigned वर्ण)(str[1] - '0') < 8) {
					/* 2nd char is 0..7: skip leading '0' */
					str++;
				}
			}
			अण्णहा जइ (*str == 'c') {
				प्रत्यागम 1;
			}
			{
				/* optimization: don't force arg to be on-stack,
				 * use another variable for that. */
				ध्रुव वर्ण *z = str;
				c = bb_process_escape_sequence(&z);
				str = z;
			}
		}
		putchar(c);
	}

	प्रत्यागम 0;
}

स्थिर शून्य print_direc(वर्ण *format, unsigned fmt_length,
		पूर्णाङ्क field_width, पूर्णाङ्क precision,
		ध्रुव वर्ण *argument)
{
	long long llv;
	द्विगुण dv;
	वर्ण saved;
	वर्ण *have_prec, *have_width;

	saved = format[fmt_length];
	format[fmt_length] = '\0';

	have_prec = strstr(format, ".*");
	have_width = strchr(format, '*');
	जइ (have_width - 1 == have_prec)
		have_width = रिक्त;

	/* multiconvert sets errno = 0, but %s needs it cleared */
	errno = 0;

	विकल्प (format[fmt_length - 1]) {
	पक्ष 'c':
		printf(format, *argument);
		भङ्ग;
	पक्ष 'd':
	पक्ष 'i':
		llv = my_xstrtoll(skip_whitespace(argument));
 print_long:
		जइ (!have_width) {
			जइ (!have_prec)
				printf(format, llv);
			अण्णहा
				printf(format, precision, llv);
		} अण्णहा {
			जइ (!have_prec)
				printf(format, field_width, llv);
			अण्णहा
				printf(format, field_width, precision, llv);
		}
		भङ्ग;
	पक्ष 'o':
	पक्ष 'u':
	पक्ष 'x':
	पक्ष 'X':
		llv = my_xstrtoull(skip_whitespace(argument));
		/* cheat: unsigned long and long have same width, so... */
		गच्छ print_long;
	पक्ष 's':
		/* Are char* and long long the same? */
		जइ (परिमाण(argument) == परिमाण(llv)) {
			llv = (long long)(ptrdiff_t)argument;
			गच्छ print_long;
		} अण्णहा {
			/* Hope compiler will optimize it out by moving call
			 * instruction after the ifs... */
			जइ (!have_width) {
				जइ (!have_prec)
					printf(format, argument, /*unused:*/ argument, argument);
				अण्णहा
					printf(format, precision, argument, /*unused:*/ argument);
			} अण्णहा {
				जइ (!have_prec)
					printf(format, field_width, argument, /*unused:*/ argument);
				अण्णहा
					printf(format, field_width, precision, argument);
			}
			भङ्ग;
		}
	पक्ष 'f':
	पक्ष 'e':
	पक्ष 'E':
	पक्ष 'g':
	पक्ष 'G':
		dv = my_xstrtod(argument);
		जइ (!have_width) {
			जइ (!have_prec)
				printf(format, dv);
			अण्णहा
				printf(format, precision, dv);
		} अण्णहा {
			जइ (!have_prec)
				printf(format, field_width, dv);
			अण्णहा
				printf(format, field_width, precision, dv);
		}
		भङ्ग;
	} /* switch */

	format[fmt_length] = saved;
}

/* Handle params for "%*.*f". Negative numbers are ok (compat). */
स्थिर पूर्णाङ्क get_width_prec(ध्रुव वर्ण *str)
{
	पूर्णाङ्क v = bb_strtoi(str, रिक्त, 10);
	जइ (errno) {
		bb_error_सन्देशः("invalid number '%s'", str);
		v = 0;
	}
	प्रत्यागम v;
}

/* Print the text in FORMAT, using ARGV for arguments to any '%' directives.
   Return advanced ARGV.  */
स्थिर वर्ण **print_formatted(वर्ण *f, वर्ण **argv, पूर्णाङ्क *conv_err)
{
	वर्ण *direc_start;      /* Start of % directive.  */
	unsigned direc_length;  /* Length of % directive.  */
	पूर्णाङ्क field_width;        /* Arg to first '*' */
	पूर्णाङ्क precision;          /* Arg to second '*' */
	वर्ण **saved_argv = argv;

	प्रति (; *f; ++f) {
		विकल्प (*f) {
		पक्ष '%':
			direc_start = f++;
			direc_length = 1;
			field_width = precision = 0;
			जइ (*f == '%') {
				bb_putchar('%');
				भङ्ग;
			}
			जइ (*f == 'b') {
				जइ (*argv) {
					जइ (print_esc_string(*argv))
						प्रत्यागम saved_argv; /* causes main() to exit */
					++argv;
				}
				भङ्ग;
			}
			जाव (*f && strchr("-+ #0", *f)) {
				++f;
				++direc_length;
			}
			जइ (*f == '*') {
				++f;
				++direc_length;
				जइ (*argv)
					field_width = get_width_prec(*argv++);
			} अण्णहा {
				जाव (isdigit(*f)) {
					++f;
					++direc_length;
				}
			}
			जइ (*f == '.') {
				++f;
				++direc_length;
				जइ (*f == '*') {
					++f;
					++direc_length;
					जइ (*argv)
						precision = get_width_prec(*argv++);
				} अण्णहा {
					जाव (isdigit(*f)) {
						++f;
						++direc_length;
					}
				}
			}

			/* Remove "lLhz" size modifiers, repeatedly.
			 * bash does not like "%lld", but coreutils
			 * happily takes even "%Llllhhzhhzd"!
			 * We are permissive like coreutils */
			जाव ((*f | 0x20) == 'l' || *f == 'h' || *f == 'z') {
				overlapping_strcpy(f, f + 1);
			}
			/* Add "ll" if integer modifier, then print */
			{
				स्थिर ध्रुव वर्ण format_chars[] ALIGN1 = "diouxXfeEgGcs";
				वर्ण *p = strchr(format_chars, *f);
				/* needed - try "printf %" without it */
				जइ (p == रिक्त || *f == '\0') {
					bb_error_सन्देशः("%s: invalid format", direc_start);
					/* causes main() to exit with error */
					प्रत्यागम saved_argv - 1;
				}
				++direc_length;
				जइ (p - format_chars <= 5) {
					/* it is one of "diouxX" */
					p = xmalloc(direc_length + 3);
					memcpy(p, direc_start, direc_length);
					p[direc_length + 1] = p[direc_length - 1];
					p[direc_length - 1] = 'l';
					p[direc_length] = 'l';
					//bb_error_msg("<%s>", p);
					direc_length += 2;
					direc_start = p;
				} अण्णहा {
					p = रिक्त;
				}
				जइ (*argv) {
					print_direc(direc_start, direc_length, field_width,
								precision, *argv++);
				} अण्णहा {
					print_direc(direc_start, direc_length, field_width,
								precision, "");
				}
				*conv_err |= errno;
				मुक्त(p);
			}
			भङ्ग;
		पक्ष '\\':
			जइ (*++f == 'c') {
				प्रत्यागम saved_argv; /* causes main() to exit */
			}
			bb_putchar(bb_process_escape_sequence((ध्रुव वर्ण **)&f));
			f--;
			भङ्ग;
		स्वभाव:
			putchar(*f);
		}
	}

	प्रत्यागम argv;
}

पूर्णाङ्क printf_main(पूर्णाङ्क argc UNUSED_PARAM, वर्ण **argv)
{
	पूर्णाङ्क conv_err;
	वर्ण *format;
	वर्ण **argv2;

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
	जइ (fcntl(1, F_GETFL) == -1)
		प्रत्यागम 1; /* match coreutils 6.10 (sans error msg to stderr) */
	//if (dup2(1, 1) != 1) - old way
	//	return 1;

	/* bash builtin errors out on "printf '-%s-\n' foo",
	 * coreutils-6.9 works. Both work with "printf -- '-%s-\n' foo".
	 * We will mimic coreutils. */
	argv = skip_dash_dash(argv);

	जइ (!argv[0]) {
		जइ ((ENABLE_ASH_PRINTF || ENABLE_HUSH_PRINTF)
		 && applet_name[0] != 'p'
		) {
			bb_simple_error_सन्देशः("usage: printf FORMAT [ARGUMENT...]");
			प्रत्यागम 2; /* bash compat */
		}
		bb_show_usage();
	}

	format = argv[0];
	argv2 = argv + 1;

	conv_err = 0;
	करेहि {
		argv = argv2;
		argv2 = print_formatted(format, argv, &conv_err);
	} जाव (argv2 > argv && *argv2);

	/* coreutils compat (bash doesn't do this):
	if (*argv)
		fprintf(stderr, "excess args ignored");
	*/

	प्रत्यागम (argv2 < argv) /* if true, print_formatted errored out */
		|| conv_err; /* print_formatted saw invalid number */
}
