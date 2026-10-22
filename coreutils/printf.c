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

प्रकारनिर्देश शून्य FAST_FUNC (*converter)(ध्रुव वर्ण *arg, शून्य *परिणाम);

स्थिर पूर्णाङ्क multiconvert(ध्रुव वर्ण *arg, शून्य *परिणाम, converter convert)
{
	यदि (*arg == '"' || *arg == '\'') {
		arg = utoa((अचिह्नित वर्ण)arg[1]);
	}
	errno = 0;
	convert(arg, परिणाम);
	यदि (errno) {
		bb_error_सन्देशः("invalid number '%s'", arg);
		प्रत्यागम 1;
	}
	प्रत्यागम 0;
}

स्थिर शून्य FAST_FUNC conv_strtoull(ध्रुव वर्ण *arg, शून्य *परिणाम)
{
	/* Allow leading '+' - bb_strtoull() by itself does not allow it,
	 * and probably shouldn't (other callers might require purely numeric
	 * inputs to be allowed.
	 */
	यदि (arg[0] == '+')
		arg++;
	*(अचिह्नित दीर्घ दीर्घ*)परिणाम = bb_strtoull(arg, रिक्त, 0);
	/* both coreutils 6.10 and bash 3.2:
	 * $ printf '%x\n' -2
	 * fffffffffffffffe
	 * Mimic that:
	 */
	यदि (errno) {
		*(अचिह्नित दीर्घ दीर्घ*)परिणाम = bb_strtoll(arg, रिक्त, 0);
	}
}
स्थिर शून्य FAST_FUNC conv_strtoll(ध्रुव वर्ण *arg, शून्य *परिणाम)
{
	यदि (arg[0] == '+')
		arg++;
	*(दीर्घ दीर्घ*)परिणाम = bb_strtoll(arg, रिक्त, 0);
}
स्थिर शून्य FAST_FUNC conv_strtod(ध्रुव वर्ण *arg, शून्य *परिणाम)
{
	वर्ण *अन्तः;
	/* Well, this one allows leading whitespace... so what? */
	/* What I like much less is that "-" accepted too! :( */
//TODO: needs setlocale(LC_NUMERIC, "C")?
	*(द्विगुण*)परिणाम = strtod(arg, &अन्तः);
	यदि (अन्तः[0]) {
		errno = ERANGE;
		*(द्विगुण*)परिणाम = 0;
	}
}

/* Callers should check errno to detect errors */
स्थिर अचिह्नित दीर्घ दीर्घ my_xstrtoull(ध्रुव वर्ण *arg)
{
	अचिह्नित दीर्घ दीर्घ परिणाम;
	यदि (multiconvert(arg, &परिणाम, conv_strtoull))
		परिणाम = 0;
	प्रत्यागम परिणाम;
}
स्थिर दीर्घ दीर्घ my_xstrtoll(ध्रुव वर्ण *arg)
{
	दीर्घ दीर्घ परिणाम;
	यदि (multiconvert(arg, &परिणाम, conv_strtoll))
		परिणाम = 0;
	प्रत्यागम परिणाम;
}
स्थिर द्विगुण my_xstrtod(ध्रुव वर्ण *arg)
{
	द्विगुण परिणाम;
	multiconvert(arg, &परिणाम, conv_strtod);
	प्रत्यागम परिणाम;
}

/* Handles %b; return 1 if output is to be short-circuited by \c */
स्थिर पूर्णाङ्क print_esc_string(ध्रुव वर्ण *str)
{
	वर्ण c;
	यावत् ((c = *str) != '\0') {
		str++;
		यदि (c == '\\') {
			/* %b also accepts 4-digit octals of the form \0### */
			यदि (*str == '0') {
				यदि ((अचिह्नित वर्ण)(str[1] - '0') < 8) {
					/* 2nd char is 0..7: skip leading '0' */
					str++;
				}
			}
			अन्यथा यदि (*str == 'c') {
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
		अक्षरं_लिख(c);
	}

	प्रत्यागम 0;
}

स्थिर शून्य print_direc(वर्ण *format, अचिह्नित fmt_दीर्घता,
		पूर्णाङ्क क्षेत्रम्_विस्तारः, पूर्णाङ्क precision,
		ध्रुव वर्ण *तर्कः)
{
	दीर्घ दीर्घ llv;
	द्विगुण dv;
	वर्ण saved;
	वर्ण *have_prec, *have_विस्तारः;

	saved = format[fmt_दीर्घता];
	format[fmt_दीर्घता] = '\0';

	have_prec = वाक्ये_अन्विष(format, ".*");
	have_विस्तारः = strchr(format, '*');
	यदि (have_विस्तारः - 1 == have_prec)
		have_विस्तारः = रिक्त;

	/* multiconvert sets errno = 0, but %s needs it cleared */
	errno = 0;

	विकल्प (format[fmt_दीर्घता - 1]) {
	पक्ष 'c':
		प्रारूप_लिख(format, *तर्कः);
		भङ्ग;
	पक्ष 'd':
	पक्ष 'i':
		llv = my_xstrtoll(skip_whitespace(तर्कः));
 print_long:
		यदि (!have_विस्तारः) {
			यदि (!have_prec)
				प्रारूप_लिख(format, llv);
			अन्यथा
				प्रारूप_लिख(format, precision, llv);
		} अन्यथा {
			यदि (!have_prec)
				प्रारूप_लिख(format, क्षेत्रम्_विस्तारः, llv);
			अन्यथा
				प्रारूप_लिख(format, क्षेत्रम्_विस्तारः, precision, llv);
		}
		भङ्ग;
	पक्ष 'o':
	पक्ष 'u':
	पक्ष 'x':
	पक्ष 'X':
		llv = my_xstrtoull(skip_whitespace(तर्कः));
		/* cheat: unsigned long and long have same width, so... */
		गच्छ print_long;
	पक्ष 's':
		/* Are char* and long long the same? */
		यदि (परिमाण(तर्कः) == परिमाण(llv)) {
			llv = (दीर्घ दीर्घ)(ptrdiff_t)तर्कः;
			गच्छ print_long;
		} अन्यथा {
			/* Hope compiler will optimize it out by moving call
			 * instruction after the ifs... */
			यदि (!have_विस्तारः) {
				यदि (!have_prec)
					प्रारूप_लिख(format, तर्कः, /*unused:*/ तर्कः, तर्कः);
				अन्यथा
					प्रारूप_लिख(format, precision, तर्कः, /*unused:*/ तर्कः);
			} अन्यथा {
				यदि (!have_prec)
					प्रारूप_लिख(format, क्षेत्रम्_विस्तारः, तर्कः, /*unused:*/ तर्कः);
				अन्यथा
					प्रारूप_लिख(format, क्षेत्रम्_विस्तारः, precision, तर्कः);
			}
			भङ्ग;
		}
	पक्ष 'f':
	पक्ष 'e':
	पक्ष 'E':
	पक्ष 'g':
	पक्ष 'G':
		dv = my_xstrtod(तर्कः);
		यदि (!have_विस्तारः) {
			यदि (!have_prec)
				प्रारूप_लिख(format, dv);
			अन्यथा
				प्रारूप_लिख(format, precision, dv);
		} अन्यथा {
			यदि (!have_prec)
				प्रारूप_लिख(format, क्षेत्रम्_विस्तारः, dv);
			अन्यथा
				प्रारूप_लिख(format, क्षेत्रम्_विस्तारः, precision, dv);
		}
		भङ्ग;
	} /* switch */

	format[fmt_दीर्घता] = saved;
}

/* Handle params for "%*.*f". Negative numbers are ok (compat). */
स्थिर पूर्णाङ्क get_विस्तारः_prec(ध्रुव वर्ण *str)
{
	पूर्णाङ्क v = bb_strtoi(str, रिक्त, 10);
	यदि (errno) {
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
	अचिह्नित direc_दीर्घता;  /* Length of % directive.  */
	पूर्णाङ्क क्षेत्रम्_विस्तारः;        /* Arg to first '*' */
	पूर्णाङ्क precision;          /* Arg to second '*' */
	वर्ण **saved_argv = argv;

	प्रति (; *f; ++f) {
		विकल्प (*f) {
		पक्ष '%':
			direc_start = f++;
			direc_दीर्घता = 1;
			क्षेत्रम्_विस्तारः = precision = 0;
			यदि (*f == '%') {
				bb_putchar('%');
				भङ्ग;
			}
			यदि (*f == 'b') {
				यदि (*argv) {
					यदि (print_esc_string(*argv))
						प्रत्यागम saved_argv; /* causes main() to exit */
					++argv;
				}
				भङ्ग;
			}
			यावत् (*f && strchr("-+ #0", *f)) {
				++f;
				++direc_दीर्घता;
			}
			यदि (*f == '*') {
				++f;
				++direc_दीर्घता;
				यदि (*argv)
					क्षेत्रम्_विस्तारः = get_विस्तारः_prec(*argv++);
			} अन्यथा {
				यावत् (अङ्कः_वा(*f)) {
					++f;
					++direc_दीर्घता;
				}
			}
			यदि (*f == '.') {
				++f;
				++direc_दीर्घता;
				यदि (*f == '*') {
					++f;
					++direc_दीर्घता;
					यदि (*argv)
						precision = get_विस्तारः_prec(*argv++);
				} अन्यथा {
					यावत् (अङ्कः_वा(*f)) {
						++f;
						++direc_दीर्घता;
					}
				}
			}

			/* Remove "lLhz" size modifiers, repeatedly.
			 * bash does not like "%lld", but coreutils
			 * happily takes even "%Llllhhzhhzd"!
			 * We are permissive like coreutils */
			यावत् ((*f | 0x20) == 'l' || *f == 'h' || *f == 'z') {
				overlapping_strcpy(f, f + 1);
			}
			/* Add "ll" if integer modifier, then print */
			{
				स्थिर ध्रुव वर्ण format_chars[] ALIGN1 = "diouxXfeEgGcs";
				वर्ण *p = strchr(format_chars, *f);
				/* needed - try "printf %" without it */
				यदि (p == रिक्त || *f == '\0') {
					bb_error_सन्देशः("%s: invalid format", direc_start);
					/* causes main() to exit with error */
					प्रत्यागम saved_argv - 1;
				}
				++direc_दीर्घता;
				यदि (p - format_chars <= 5) {
					/* it is one of "diouxX" */
					p = xmalloc(direc_दीर्घता + 3);
					स्मृतिं_प्रतिलिख(p, direc_start, direc_दीर्घता);
					p[direc_दीर्घता + 1] = p[direc_दीर्घता - 1];
					p[direc_दीर्घता - 1] = 'l';
					p[direc_दीर्घता] = 'l';
					//bb_error_msg("<%s>", p);
					direc_दीर्घता += 2;
					direc_start = p;
				} अन्यथा {
					p = रिक्त;
				}
				यदि (*argv) {
					print_direc(direc_start, direc_दीर्घता, क्षेत्रम्_विस्तारः,
								precision, *argv++);
				} अन्यथा {
					print_direc(direc_start, direc_दीर्घता, क्षेत्रम्_विस्तारः,
								precision, "");
				}
				*conv_err |= errno;
				मुक्त(p);
			}
			भङ्ग;
		पक्ष '\\':
			यदि (*++f == 'c') {
				प्रत्यागम saved_argv; /* causes main() to exit */
			}
			bb_putchar(bb_process_escape_sequence((ध्रुव वर्ण **)&f));
			f--;
			भङ्ग;
		स्वभाव:
			अक्षरं_लिख(*f);
		}
	}

	प्रत्यागम argv;
}

पूर्णाङ्क printf_main(पूर्णाङ्क argc अप्रयुक्तम्_PARAM, वर्ण **argv)
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
	यदि (fcntl(1, F_GETFL) == -1)
		प्रत्यागम 1; /* match coreutils 6.10 (sans error msg to stderr) */
	//if (dup2(1, 1) != 1) - old way
	//	return 1;

	/* bash builtin errors out on "printf '-%s-\n' foo",
	 * coreutils-6.9 works. Both work with "printf -- '-%s-\n' foo".
	 * We will mimic coreutils. */
	argv = skip_dash_dash(argv);

	यदि (!argv[0]) {
		यदि ((ENABLE_ASH_PRINTF || ENABLE_HUSH_PRINTF)
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
	कुरु {
		argv = argv2;
		argv2 = print_formatted(format, argv, &conv_err);
	} यावत् (argv2 > argv && *argv2);

	/* coreutils compat (bash doesn't do this):
	if (*argv)
		fprintf(stderr, "excess args ignored");
	*/

	प्रत्यागम (argv2 < argv) /* if true, print_formatted errored out */
		|| conv_err; /* print_formatted saw invalid number */
}
