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

प्रकारव्याख्या शून्य FAST_FUNC (*converter)(स्थिरांक अक्षर *arg, शून्य *result);

स्थिर पूर्णांक multiconvert(स्थिरांक अक्षर *arg, शून्य *result, converter convert)
{
	जर (*arg == '"' || *arg == '\'') {
		arg = utoa((अचिन्हित अक्षर)arg[1]);
	}
	त्रुटि_सं = 0;
	convert(arg, result);
	जर (त्रुटि_सं) {
		bb_error_संदेश("invalid number '%s'", arg);
		परत 1;
	}
	परत 0;
}

स्थिर शून्य FAST_FUNC conv_strtoull(स्थिरांक अक्षर *arg, शून्य *result)
{
	/* Allow leading '+' - bb_strtoull() by itself does not allow it,
	 * and probably shouldn't (other callers might require purely numeric
	 * inputs to be allowed.
	 */
	जर (arg[0] == '+')
		arg++;
	*(अचिन्हित दीर्घ दीर्घ*)result = bb_strtoull(arg, रिक्त, 0);
	/* both coreutils 6.10 and bash 3.2:
	 * $ printf '%x\n' -2
	 * fffffffffffffffe
	 * Mimic that:
	 */
	जर (त्रुटि_सं) {
		*(अचिन्हित दीर्घ दीर्घ*)result = bb_strtoll(arg, रिक्त, 0);
	}
}
स्थिर शून्य FAST_FUNC conv_strtoll(स्थिरांक अक्षर *arg, शून्य *result)
{
	जर (arg[0] == '+')
		arg++;
	*(दीर्घ दीर्घ*)result = bb_strtoll(arg, रिक्त, 0);
}
स्थिर शून्य FAST_FUNC conv_strtod(स्थिरांक अक्षर *arg, शून्य *result)
{
	अक्षर *end;
	/* Well, this one allows leading whitespace... so what? */
	/* What I like much less is that "-" accepted too! :( */
//TODO: needs setlocale(LC_NUMERIC, "C")?
	*(द्विगुण*)result = म_से_भग्न(arg, &end);
	जर (end[0]) {
		त्रुटि_सं = दुस्फल;
		*(द्विगुण*)result = 0;
	}
}

/* Callers should check errno to detect errors */
स्थिर अचिन्हित दीर्घ दीर्घ my_xstrtoull(स्थिरांक अक्षर *arg)
{
	अचिन्हित दीर्घ दीर्घ result;
	जर (multiconvert(arg, &result, conv_strtoull))
		result = 0;
	परत result;
}
स्थिर दीर्घ दीर्घ my_xstrtoll(स्थिरांक अक्षर *arg)
{
	दीर्घ दीर्घ result;
	जर (multiconvert(arg, &result, conv_strtoll))
		result = 0;
	परत result;
}
स्थिर द्विगुण my_xstrtod(स्थिरांक अक्षर *arg)
{
	द्विगुण result;
	multiconvert(arg, &result, conv_strtod);
	परत result;
}

/* Handles %b; return 1 if output is to be short-circuited by \c */
स्थिर पूर्णांक print_esc_string(स्थिरांक अक्षर *str)
{
	अक्षर c;
	पर्यंत ((c = *str) != '\0') {
		str++;
		जर (c == '\\') {
			/* %b also accepts 4-digit octals of the form \0### */
			जर (*str == '0') {
				जर ((अचिन्हित अक्षर)(str[1] - '0') < 8) {
					/* 2nd char is 0..7: skip leading '0' */
					str++;
				}
			}
			अन्यथा जर (*str == 'c') {
				परत 1;
			}
			{
				/* optimization: don't force arg to be on-stack,
				 * use another variable for that. */
				स्थिरांक अक्षर *z = str;
				c = bb_process_escape_sequence(&z);
				str = z;
			}
		}
		अक्षर_दो(c);
	}

	परत 0;
}

स्थिर शून्य print_direc(अक्षर *format, अचिन्हित fmt_length,
		पूर्णांक field_width, पूर्णांक precision,
		स्थिरांक अक्षर *argument)
{
	दीर्घ दीर्घ llv;
	द्विगुण dv;
	अक्षर saved;
	अक्षर *have_prec, *have_width;

	saved = format[fmt_length];
	format[fmt_length] = '\0';

	have_prec = म_माला(format, ".*");
	have_width = म_अक्षर(format, '*');
	जर (have_width - 1 == have_prec)
		have_width = रिक्त;

	/* multiconvert sets errno = 0, but %s needs it cleared */
	त्रुटि_सं = 0;

	निवड (format[fmt_length - 1]) {
	स्थिती 'c':
		म_लिखो(format, *argument);
		थांबा;
	स्थिती 'd':
	स्थिती 'i':
		llv = my_xstrtoll(skip_whitespace(argument));
 print_long:
		जर (!have_width) {
			जर (!have_prec)
				म_लिखो(format, llv);
			अन्यथा
				म_लिखो(format, precision, llv);
		} अन्यथा {
			जर (!have_prec)
				म_लिखो(format, field_width, llv);
			अन्यथा
				म_लिखो(format, field_width, precision, llv);
		}
		थांबा;
	स्थिती 'o':
	स्थिती 'u':
	स्थिती 'x':
	स्थिती 'X':
		llv = my_xstrtoull(skip_whitespace(argument));
		/* cheat: unsigned long and long have same width, so... */
		जा print_long;
	स्थिती 's':
		/* Are char* and long long the same? */
		जर (आकार(argument) == आकार(llv)) {
			llv = (दीर्घ दीर्घ)(सूचक_भेद_प्रकार)argument;
			जा print_long;
		} अन्यथा {
			/* Hope compiler will optimize it out by moving call
			 * instruction after the ifs... */
			जर (!have_width) {
				जर (!have_prec)
					म_लिखो(format, argument, /*unused:*/ argument, argument);
				अन्यथा
					म_लिखो(format, precision, argument, /*unused:*/ argument);
			} अन्यथा {
				जर (!have_prec)
					म_लिखो(format, field_width, argument, /*unused:*/ argument);
				अन्यथा
					म_लिखो(format, field_width, precision, argument);
			}
			थांबा;
		}
	स्थिती 'f':
	स्थिती 'e':
	स्थिती 'E':
	स्थिती 'g':
	स्थिती 'G':
		dv = my_xstrtod(argument);
		जर (!have_width) {
			जर (!have_prec)
				म_लिखो(format, dv);
			अन्यथा
				म_लिखो(format, precision, dv);
		} अन्यथा {
			जर (!have_prec)
				म_लिखो(format, field_width, dv);
			अन्यथा
				म_लिखो(format, field_width, precision, dv);
		}
		थांबा;
	} /* switch */

	format[fmt_length] = saved;
}

/* Handle params for "%*.*f". Negative numbers are ok (compat). */
स्थिर पूर्णांक get_width_prec(स्थिरांक अक्षर *str)
{
	पूर्णांक v = bb_strtoi(str, रिक्त, 10);
	जर (त्रुटि_सं) {
		bb_error_संदेश("invalid number '%s'", str);
		v = 0;
	}
	परत v;
}

/* Print the text in FORMAT, using ARGV for arguments to any '%' directives.
   Return advanced ARGV.  */
स्थिर अक्षर **print_formatted(अक्षर *f, अक्षर **argv, पूर्णांक *conv_err)
{
	अक्षर *direc_start;      /* Start of % directive.  */
	अचिन्हित direc_length;  /* Length of % directive.  */
	पूर्णांक field_width;        /* Arg to first '*' */
	पूर्णांक precision;          /* Arg to second '*' */
	अक्षर **saved_argv = argv;

	साठी (; *f; ++f) {
		निवड (*f) {
		स्थिती '%':
			direc_start = f++;
			direc_length = 1;
			field_width = precision = 0;
			जर (*f == '%') {
				bb_putchar('%');
				थांबा;
			}
			जर (*f == 'b') {
				जर (*argv) {
					जर (print_esc_string(*argv))
						परत saved_argv; /* causes main() to exit */
					++argv;
				}
				थांबा;
			}
			पर्यंत (*f && म_अक्षर("-+ #0", *f)) {
				++f;
				++direc_length;
			}
			जर (*f == '*') {
				++f;
				++direc_length;
				जर (*argv)
					field_width = get_width_prec(*argv++);
			} अन्यथा {
				पर्यंत (है_अंक(*f)) {
					++f;
					++direc_length;
				}
			}
			जर (*f == '.') {
				++f;
				++direc_length;
				जर (*f == '*') {
					++f;
					++direc_length;
					जर (*argv)
						precision = get_width_prec(*argv++);
				} अन्यथा {
					पर्यंत (है_अंक(*f)) {
						++f;
						++direc_length;
					}
				}
			}

			/* Remove "lLhz" size modifiers, repeatedly.
			 * bash does not like "%lld", but coreutils
			 * happily takes even "%Llllhhzhhzd"!
			 * We are permissive like coreutils */
			पर्यंत ((*f | 0x20) == 'l' || *f == 'h' || *f == 'z') {
				overlapping_strcpy(f, f + 1);
			}
			/* Add "ll" if integer modifier, then print */
			{
				स्थिर स्थिरांक अक्षर format_chars[] ALIGN1 = "diouxXfeEgGcs";
				अक्षर *p = म_अक्षर(format_chars, *f);
				/* needed - try "printf %" without it */
				जर (p == रिक्त || *f == '\0') {
					bb_error_संदेश("%s: invalid format", direc_start);
					/* causes main() to exit with error */
					परत saved_argv - 1;
				}
				++direc_length;
				जर (p - format_chars <= 5) {
					/* it is one of "diouxX" */
					p = xmalloc(direc_length + 3);
					स_नकल(p, direc_start, direc_length);
					p[direc_length + 1] = p[direc_length - 1];
					p[direc_length - 1] = 'l';
					p[direc_length] = 'l';
					//bb_error_msg("<%s>", p);
					direc_length += 2;
					direc_start = p;
				} अन्यथा {
					p = रिक्त;
				}
				जर (*argv) {
					print_direc(direc_start, direc_length, field_width,
								precision, *argv++);
				} अन्यथा {
					print_direc(direc_start, direc_length, field_width,
								precision, "");
				}
				*conv_err |= त्रुटि_सं;
				मुक्त(p);
			}
			थांबा;
		स्थिती '\\':
			जर (*++f == 'c') {
				परत saved_argv; /* causes main() to exit */
			}
			bb_putchar(bb_process_escape_sequence((स्थिरांक अक्षर **)&f));
			f--;
			थांबा;
		पूर्वनिर्धारित:
			अक्षर_दो(*f);
		}
	}

	परत argv;
}

पूर्णांक printf_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	पूर्णांक conv_err;
	अक्षर *format;
	अक्षर **argv2;

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
	जर (fcntl(1, F_GETFL) == -1)
		परत 1; /* match coreutils 6.10 (sans error msg to stderr) */
	//if (dup2(1, 1) != 1) - old way
	//	return 1;

	/* bash builtin errors out on "printf '-%s-\n' foo",
	 * coreutils-6.9 works. Both work with "printf -- '-%s-\n' foo".
	 * We will mimic coreutils. */
	argv = skip_dash_dash(argv);

	जर (!argv[0]) {
		जर ((ENABLE_ASH_PRINTF || ENABLE_HUSH_PRINTF)
		 && applet_name[0] != 'p'
		) {
			bb_simple_error_संदेश("usage: printf FORMAT [ARGUMENT...]");
			परत 2; /* bash compat */
		}
		bb_show_usage();
	}

	format = argv[0];
	argv2 = argv + 1;

	conv_err = 0;
	करा {
		argv = argv2;
		argv2 = print_formatted(format, argv, &conv_err);
	} पर्यंत (argv2 > argv && *argv2);

	/* coreutils compat (bash doesn't do this):
	if (*argv)
		fprintf(stderr, "excess args ignored");
	*/

	परत (argv2 < argv) /* if true, print_formatted errored out */
		|| conv_err; /* print_formatted saw invalid number */
}
