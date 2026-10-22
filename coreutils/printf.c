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

#समावेश "libbb.h"

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

प्रकार व्योम FAST_FUNC (*converter)(स्थिर अक्षर *arg, व्योम *परिणाम);

अटल पूर्णांक multiconvert(स्थिर अक्षर *arg, व्योम *परिणाम, converter convert)
{
	अगर (*arg == '"' || *arg == '\'') {
		arg = utoa((अचिन्हित अक्षर)arg[1]);
	}
	त्रुटि_सं = 0;
	convert(arg, परिणाम);
	अगर (त्रुटि_सं) {
		bb_error_संदेश("invalid number '%s'", arg);
		वापस 1;
	}
	वापस 0;
}

अटल व्योम FAST_FUNC conv_strtoull(स्थिर अक्षर *arg, व्योम *परिणाम)
{
	/* Allow leading '+' - bb_strtoull() by itself does not allow it,
	 * and probably shouldn't (other callers might require purely numeric
	 * inputs to be allowed.
	 */
	अगर (arg[0] == '+')
		arg++;
	*(अचिन्हित दीर्घ दीर्घ*)परिणाम = bb_strtoull(arg, शून्य, 0);
	/* both coreutils 6.10 and bash 3.2:
	 * $ printf '%x\n' -2
	 * fffffffffffffffe
	 * Mimic that:
	 */
	अगर (त्रुटि_सं) {
		*(अचिन्हित दीर्घ दीर्घ*)परिणाम = bb_strtoll(arg, शून्य, 0);
	}
}
अटल व्योम FAST_FUNC conv_strtoll(स्थिर अक्षर *arg, व्योम *परिणाम)
{
	अगर (arg[0] == '+')
		arg++;
	*(दीर्घ दीर्घ*)परिणाम = bb_strtoll(arg, शून्य, 0);
}
अटल व्योम FAST_FUNC conv_strtod(स्थिर अक्षर *arg, व्योम *परिणाम)
{
	अक्षर *end;
	/* Well, this one allows leading whitespace... so what? */
	/* What I like much less is that "-" accepted too! :( */
//TODO: needs setlocale(LC_NUMERIC, "C")?
	*(द्विगुन*)परिणाम = म_से_भग्न(arg, &end);
	अगर (end[0]) {
		त्रुटि_सं = दुस्फल;
		*(द्विगुन*)परिणाम = 0;
	}
}

/* Callers should check errno to detect errors */
अटल अचिन्हित दीर्घ दीर्घ my_xstrtoull(स्थिर अक्षर *arg)
{
	अचिन्हित दीर्घ दीर्घ परिणाम;
	अगर (multiconvert(arg, &परिणाम, conv_strtoull))
		परिणाम = 0;
	वापस परिणाम;
}
अटल दीर्घ दीर्घ my_xstrtoll(स्थिर अक्षर *arg)
{
	दीर्घ दीर्घ परिणाम;
	अगर (multiconvert(arg, &परिणाम, conv_strtoll))
		परिणाम = 0;
	वापस परिणाम;
}
अटल द्विगुन my_xstrtod(स्थिर अक्षर *arg)
{
	द्विगुन परिणाम;
	multiconvert(arg, &परिणाम, conv_strtod);
	वापस परिणाम;
}

/* Handles %b; return 1 if output is to be short-circuited by \c */
अटल पूर्णांक print_esc_string(स्थिर अक्षर *str)
{
	अक्षर c;
	जबतक ((c = *str) != '\0') {
		str++;
		अगर (c == '\\') {
			/* %b also accepts 4-digit octals of the form \0### */
			अगर (*str == '0') {
				अगर ((अचिन्हित अक्षर)(str[1] - '0') < 8) {
					/* 2nd char is 0..7: skip leading '0' */
					str++;
				}
			}
			अन्यथा अगर (*str == 'c') {
				वापस 1;
			}
			{
				/* optimization: don't force arg to be on-stack,
				 * use another variable for that. */
				स्थिर अक्षर *z = str;
				c = bb_process_escape_sequence(&z);
				str = z;
			}
		}
		अक्षर_दो(c);
	}

	वापस 0;
}

अटल व्योम print_direc(अक्षर *format, अचिन्हित fmt_लंबाई,
		पूर्णांक field_width, पूर्णांक precision,
		स्थिर अक्षर *argument)
{
	दीर्घ दीर्घ llv;
	द्विगुन dv;
	अक्षर saved;
	अक्षर *have_prec, *have_width;

	saved = format[fmt_लंबाई];
	format[fmt_लंबाई] = '\0';

	have_prec = म_माला(format, ".*");
	have_width = म_अक्षर(format, '*');
	अगर (have_width - 1 == have_prec)
		have_width = शून्य;

	/* multiconvert sets errno = 0, but %s needs it cleared */
	त्रुटि_सं = 0;

	चयन (format[fmt_लंबाई - 1]) {
	हाल 'c':
		म_लिखो(format, *argument);
		अवरोध;
	हाल 'd':
	हाल 'i':
		llv = my_xstrtoll(skip_whitespace(argument));
 print_long:
		अगर (!have_width) {
			अगर (!have_prec)
				म_लिखो(format, llv);
			अन्यथा
				म_लिखो(format, precision, llv);
		} अन्यथा {
			अगर (!have_prec)
				म_लिखो(format, field_width, llv);
			अन्यथा
				म_लिखो(format, field_width, precision, llv);
		}
		अवरोध;
	हाल 'o':
	हाल 'u':
	हाल 'x':
	हाल 'X':
		llv = my_xstrtoull(skip_whitespace(argument));
		/* cheat: unsigned long and long have same width, so... */
		जाओ print_long;
	हाल 's':
		/* Are char* and long long the same? */
		अगर (माप(argument) == माप(llv)) {
			llv = (दीर्घ दीर्घ)(सूचक_भेद_प्रकार)argument;
			जाओ print_long;
		} अन्यथा {
			/* Hope compiler will optimize it out by moving call
			 * instruction after the ifs... */
			अगर (!have_width) {
				अगर (!have_prec)
					म_लिखो(format, argument, /*unused:*/ argument, argument);
				अन्यथा
					म_लिखो(format, precision, argument, /*unused:*/ argument);
			} अन्यथा {
				अगर (!have_prec)
					म_लिखो(format, field_width, argument, /*unused:*/ argument);
				अन्यथा
					म_लिखो(format, field_width, precision, argument);
			}
			अवरोध;
		}
	हाल 'f':
	हाल 'e':
	हाल 'E':
	हाल 'g':
	हाल 'G':
		dv = my_xstrtod(argument);
		अगर (!have_width) {
			अगर (!have_prec)
				म_लिखो(format, dv);
			अन्यथा
				म_लिखो(format, precision, dv);
		} अन्यथा {
			अगर (!have_prec)
				म_लिखो(format, field_width, dv);
			अन्यथा
				म_लिखो(format, field_width, precision, dv);
		}
		अवरोध;
	} /* switch */

	format[fmt_लंबाई] = saved;
}

/* Handle params for "%*.*f". Negative numbers are ok (compat). */
अटल पूर्णांक get_width_prec(स्थिर अक्षर *str)
{
	पूर्णांक v = bb_strtoi(str, शून्य, 10);
	अगर (त्रुटि_सं) {
		bb_error_संदेश("invalid number '%s'", str);
		v = 0;
	}
	वापस v;
}

/* Print the text in FORMAT, using ARGV for arguments to any '%' directives.
   Return advanced ARGV.  */
अटल अक्षर **print_formatted(अक्षर *f, अक्षर **argv, पूर्णांक *conv_err)
{
	अक्षर *direc_start;      /* Start of % directive.  */
	अचिन्हित direc_लंबाई;  /* Length of % directive.  */
	पूर्णांक field_width;        /* Arg to first '*' */
	पूर्णांक precision;          /* Arg to second '*' */
	अक्षर **saved_argv = argv;

	क्रम (; *f; ++f) {
		चयन (*f) {
		हाल '%':
			direc_start = f++;
			direc_लंबाई = 1;
			field_width = precision = 0;
			अगर (*f == '%') {
				bb_putchar('%');
				अवरोध;
			}
			अगर (*f == 'b') {
				अगर (*argv) {
					अगर (print_esc_string(*argv))
						वापस saved_argv; /* causes main() to exit */
					++argv;
				}
				अवरोध;
			}
			जबतक (*f && म_अक्षर("-+ #0", *f)) {
				++f;
				++direc_लंबाई;
			}
			अगर (*f == '*') {
				++f;
				++direc_लंबाई;
				अगर (*argv)
					field_width = get_width_prec(*argv++);
			} अन्यथा {
				जबतक (है_अंक(*f)) {
					++f;
					++direc_लंबाई;
				}
			}
			अगर (*f == '.') {
				++f;
				++direc_लंबाई;
				अगर (*f == '*') {
					++f;
					++direc_लंबाई;
					अगर (*argv)
						precision = get_width_prec(*argv++);
				} अन्यथा {
					जबतक (है_अंक(*f)) {
						++f;
						++direc_लंबाई;
					}
				}
			}

			/* Remove "lLhz" size modifiers, repeatedly.
			 * bash does not like "%lld", but coreutils
			 * happily takes even "%Llllhhzhhzd"!
			 * We are permissive like coreutils */
			जबतक ((*f | 0x20) == 'l' || *f == 'h' || *f == 'z') {
				overlapping_strcpy(f, f + 1);
			}
			/* Add "ll" if integer modifier, then print */
			{
				अटल स्थिर अक्षर format_chars[] ALIGN1 = "diouxXfeEgGcs";
				अक्षर *p = म_अक्षर(format_chars, *f);
				/* needed - try "printf %" without it */
				अगर (p == शून्य || *f == '\0') {
					bb_error_संदेश("%s: invalid format", direc_start);
					/* causes main() to exit with error */
					वापस saved_argv - 1;
				}
				++direc_लंबाई;
				अगर (p - format_chars <= 5) {
					/* it is one of "diouxX" */
					p = xmalloc(direc_लंबाई + 3);
					स_नकल(p, direc_start, direc_लंबाई);
					p[direc_लंबाई + 1] = p[direc_लंबाई - 1];
					p[direc_लंबाई - 1] = 'l';
					p[direc_लंबाई] = 'l';
					//bb_error_msg("<%s>", p);
					direc_लंबाई += 2;
					direc_start = p;
				} अन्यथा {
					p = शून्य;
				}
				अगर (*argv) {
					print_direc(direc_start, direc_लंबाई, field_width,
								precision, *argv++);
				} अन्यथा {
					print_direc(direc_start, direc_लंबाई, field_width,
								precision, "");
				}
				*conv_err |= त्रुटि_सं;
				मुक्त(p);
			}
			अवरोध;
		हाल '\\':
			अगर (*++f == 'c') {
				वापस saved_argv; /* causes main() to exit */
			}
			bb_putchar(bb_process_escape_sequence((स्थिर अक्षर **)&f));
			f--;
			अवरोध;
		शेष:
			अक्षर_दो(*f);
		}
	}

	वापस argv;
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
	अगर (fcntl(1, F_GETFL) == -1)
		वापस 1; /* match coreutils 6.10 (sans error msg to stderr) */
	//if (dup2(1, 1) != 1) - old way
	//	return 1;

	/* bash builtin errors out on "printf '-%s-\n' foo",
	 * coreutils-6.9 works. Both work with "printf -- '-%s-\n' foo".
	 * We will mimic coreutils. */
	argv = skip_dash_dash(argv);

	अगर (!argv[0]) {
		अगर ((ENABLE_ASH_PRINTF || ENABLE_HUSH_PRINTF)
		 && applet_name[0] != 'p'
		) {
			bb_simple_error_संदेश("usage: printf FORMAT [ARGUMENT...]");
			वापस 2; /* bash compat */
		}
		bb_show_usage();
	}

	format = argv[0];
	argv2 = argv + 1;

	conv_err = 0;
	करो {
		argv = argv2;
		argv2 = print_formatted(format, argv, &conv_err);
	} जबतक (argv2 > argv && *argv2);

	/* coreutils compat (bash doesn't do this):
	if (*argv)
		fprintf(stderr, "excess args ignored");
	*/

	वापस (argv2 < argv) /* if true, print_formatted errored out */
		|| conv_err; /* print_formatted saw invalid number */
}
