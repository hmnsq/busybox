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

#అన్తర్భావ "libbb.h"

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

రకనిర్వచనం శూన్య FAST_FUNC (*converter)(స్థిరరాశి అక్షరం *arg, శూన్య *ఫలితం);

స్థిర పూర్ణాంకం multiconvert(స్థిరరాశి అక్షరం *arg, శూన్య *ఫలితం, converter convert)
{
	ఒకవేళ (*arg == '"' || *arg == '\'') {
		arg = utoa((గుర్తులేని అక్షరం)arg[1]);
	}
	errno = 0;
	convert(arg, ఫలితం);
	ఒకవేళ (errno) {
		bb_error_సందేశం("invalid number '%s'", arg);
		తిరిగి 1;
	}
	తిరిగి 0;
}

స్థిర శూన్య FAST_FUNC conv_strtoull(స్థిరరాశి అక్షరం *arg, శూన్య *ఫలితం)
{
	/* Allow leading '+' - bb_strtoull() by itself does not allow it,
	 * and probably shouldn't (other callers might require purely numeric
	 * inputs to be allowed.
	 */
	ఒకవేళ (arg[0] == '+')
		arg++;
	*(గుర్తులేని దీర్ఘ దీర్ఘ*)ఫలితం = bb_strtoull(arg, ఖాళీ, 0);
	/* both coreutils 6.10 and bash 3.2:
	 * $ printf '%x\n' -2
	 * fffffffffffffffe
	 * Mimic that:
	 */
	ఒకవేళ (errno) {
		*(గుర్తులేని దీర్ఘ దీర్ఘ*)ఫలితం = bb_strtoll(arg, ఖాళీ, 0);
	}
}
స్థిర శూన్య FAST_FUNC conv_strtoll(స్థిరరాశి అక్షరం *arg, శూన్య *ఫలితం)
{
	ఒకవేళ (arg[0] == '+')
		arg++;
	*(దీర్ఘ దీర్ఘ*)ఫలితం = bb_strtoll(arg, ఖాళీ, 0);
}
స్థిర శూన్య FAST_FUNC conv_strtod(స్థిరరాశి అక్షరం *arg, శూన్య *ఫలితం)
{
	అక్షరం *చివర;
	/* Well, this one allows leading whitespace... so what? */
	/* What I like much less is that "-" accepted too! :( */
//TODO: needs setlocale(LC_NUMERIC, "C")?
	*(ద్విగుణ*)ఫలితం = strtod(arg, &చివర);
	ఒకవేళ (చివర[0]) {
		errno = ERANGE;
		*(ద్విగుణ*)ఫలితం = 0;
	}
}

/* Callers should check errno to detect errors */
స్థిర గుర్తులేని దీర్ఘ దీర్ఘ my_xstrtoull(స్థిరరాశి అక్షరం *arg)
{
	గుర్తులేని దీర్ఘ దీర్ఘ ఫలితం;
	ఒకవేళ (multiconvert(arg, &ఫలితం, conv_strtoull))
		ఫలితం = 0;
	తిరిగి ఫలితం;
}
స్థిర దీర్ఘ దీర్ఘ my_xstrtoll(స్థిరరాశి అక్షరం *arg)
{
	దీర్ఘ దీర్ఘ ఫలితం;
	ఒకవేళ (multiconvert(arg, &ఫలితం, conv_strtoll))
		ఫలితం = 0;
	తిరిగి ఫలితం;
}
స్థిర ద్విగుణ my_xstrtod(స్థిరరాశి అక్షరం *arg)
{
	ద్విగుణ ఫలితం;
	multiconvert(arg, &ఫలితం, conv_strtod);
	తిరిగి ఫలితం;
}

/* Handles %b; return 1 if output is to be short-circuited by \c */
స్థిర పూర్ణాంకం print_esc_string(స్థిరరాశి అక్షరం *str)
{
	అక్షరం c;
	అయితే ((c = *str) != '\0') {
		str++;
		ఒకవేళ (c == '\\') {
			/* %b also accepts 4-digit octals of the form \0### */
			ఒకవేళ (*str == '0') {
				ఒకవేళ ((గుర్తులేని అక్షరం)(str[1] - '0') < 8) {
					/* 2nd char is 0..7: skip leading '0' */
					str++;
				}
			}
			లేకపోతే ఒకవేళ (*str == 'c') {
				తిరిగి 1;
			}
			{
				/* optimization: don't force arg to be on-stack,
				 * use another variable for that. */
				స్థిరరాశి అక్షరం *z = str;
				c = bb_process_escape_sequence(&z);
				str = z;
			}
		}
		అక్షరం_రాయి(c);
	}

	తిరిగి 0;
}

స్థిర శూన్య print_direc(అక్షరం *format, గుర్తులేని fmt_నిడివి,
		పూర్ణాంకం క్షేత్రం_వెడల్పు, పూర్ణాంకం precision,
		స్థిరరాశి అక్షరం *వాదం)
{
	దీర్ఘ దీర్ఘ llv;
	ద్విగుణ dv;
	అక్షరం saved;
	అక్షరం *have_prec, *have_వెడల్పు;

	saved = format[fmt_నిడివి];
	format[fmt_నిడివి] = '\0';

	have_prec = అక్షరశ్రేణి_శోధన(format, ".*");
	have_వెడల్పు = strchr(format, '*');
	ఒకవేళ (have_వెడల్పు - 1 == have_prec)
		have_వెడల్పు = ఖాళీ;

	/* multiconvert sets errno = 0, but %s needs it cleared */
	errno = 0;

	ఎంపిక (format[fmt_నిడివి - 1]) {
	స్థితి 'c':
		రాయి(format, *వాదం);
		ఆపు;
	స్థితి 'd':
	స్థితి 'i':
		llv = my_xstrtoll(skip_whitespace(వాదం));
 print_long:
		ఒకవేళ (!have_వెడల్పు) {
			ఒకవేళ (!have_prec)
				రాయి(format, llv);
			లేకపోతే
				రాయి(format, precision, llv);
		} లేకపోతే {
			ఒకవేళ (!have_prec)
				రాయి(format, క్షేత్రం_వెడల్పు, llv);
			లేకపోతే
				రాయి(format, క్షేత్రం_వెడల్పు, precision, llv);
		}
		ఆపు;
	స్థితి 'o':
	స్థితి 'u':
	స్థితి 'x':
	స్థితి 'X':
		llv = my_xstrtoull(skip_whitespace(వాదం));
		/* cheat: unsigned long and long have same width, so... */
		వెళ్ళు print_long;
	స్థితి 's':
		/* Are char* and long long the same? */
		ఒకవేళ (పరిమాణం(వాదం) == పరిమాణం(llv)) {
			llv = (దీర్ఘ దీర్ఘ)(ptrdiff_t)వాదం;
			వెళ్ళు print_long;
		} లేకపోతే {
			/* Hope compiler will optimize it out by moving call
			 * instruction after the ifs... */
			ఒకవేళ (!have_వెడల్పు) {
				ఒకవేళ (!have_prec)
					రాయి(format, వాదం, /*unused:*/ వాదం, వాదం);
				లేకపోతే
					రాయి(format, precision, వాదం, /*unused:*/ వాదం);
			} లేకపోతే {
				ఒకవేళ (!have_prec)
					రాయి(format, క్షేత్రం_వెడల్పు, వాదం, /*unused:*/ వాదం);
				లేకపోతే
					రాయి(format, క్షేత్రం_వెడల్పు, precision, వాదం);
			}
			ఆపు;
		}
	స్థితి 'f':
	స్థితి 'e':
	స్థితి 'E':
	స్థితి 'g':
	స్థితి 'G':
		dv = my_xstrtod(వాదం);
		ఒకవేళ (!have_వెడల్పు) {
			ఒకవేళ (!have_prec)
				రాయి(format, dv);
			లేకపోతే
				రాయి(format, precision, dv);
		} లేకపోతే {
			ఒకవేళ (!have_prec)
				రాయి(format, క్షేత్రం_వెడల్పు, dv);
			లేకపోతే
				రాయి(format, క్షేత్రం_వెడల్పు, precision, dv);
		}
		ఆపు;
	} /* switch */

	format[fmt_నిడివి] = saved;
}

/* Handle params for "%*.*f". Negative numbers are ok (compat). */
స్థిర పూర్ణాంకం get_వెడల్పు_prec(స్థిరరాశి అక్షరం *str)
{
	పూర్ణాంకం v = bb_strtoi(str, ఖాళీ, 10);
	ఒకవేళ (errno) {
		bb_error_సందేశం("invalid number '%s'", str);
		v = 0;
	}
	తిరిగి v;
}

/* Print the text in FORMAT, using ARGV for arguments to any '%' directives.
   Return advanced ARGV.  */
స్థిర అక్షరం **print_formatted(అక్షరం *f, అక్షరం **argv, పూర్ణాంకం *conv_err)
{
	అక్షరం *direc_ప్రారంభం;      /* Start of % directive.  */
	గుర్తులేని direc_నిడివి;  /* Length of % directive.  */
	పూర్ణాంకం క్షేత్రం_వెడల్పు;        /* Arg to first '*' */
	పూర్ణాంకం precision;          /* Arg to second '*' */
	అక్షరం **saved_argv = argv;

	కొరకు (; *f; ++f) {
		ఎంపిక (*f) {
		స్థితి '%':
			direc_ప్రారంభం = f++;
			direc_నిడివి = 1;
			క్షేత్రం_వెడల్పు = precision = 0;
			ఒకవేళ (*f == '%') {
				bb_putchar('%');
				ఆపు;
			}
			ఒకవేళ (*f == 'b') {
				ఒకవేళ (*argv) {
					ఒకవేళ (print_esc_string(*argv))
						తిరిగి saved_argv; /* causes main() to exit */
					++argv;
				}
				ఆపు;
			}
			అయితే (*f && strchr("-+ #0", *f)) {
				++f;
				++direc_నిడివి;
			}
			ఒకవేళ (*f == '*') {
				++f;
				++direc_నిడివి;
				ఒకవేళ (*argv)
					క్షేత్రం_వెడల్పు = get_వెడల్పు_prec(*argv++);
			} లేకపోతే {
				అయితే (అంకెనా(*f)) {
					++f;
					++direc_నిడివి;
				}
			}
			ఒకవేళ (*f == '.') {
				++f;
				++direc_నిడివి;
				ఒకవేళ (*f == '*') {
					++f;
					++direc_నిడివి;
					ఒకవేళ (*argv)
						precision = get_వెడల్పు_prec(*argv++);
				} లేకపోతే {
					అయితే (అంకెనా(*f)) {
						++f;
						++direc_నిడివి;
					}
				}
			}

			/* Remove "lLhz" size modifiers, repeatedly.
			 * bash does not like "%lld", but coreutils
			 * happily takes even "%Llllhhzhhzd"!
			 * We are permissive like coreutils */
			అయితే ((*f | 0x20) == 'l' || *f == 'h' || *f == 'z') {
				overlapping_strcpy(f, f + 1);
			}
			/* Add "ll" if integer modifier, then print */
			{
				స్థిర స్థిరరాశి అక్షరం format_chars[] ALIGN1 = "diouxXfeEgGcs";
				అక్షరం *p = strchr(format_chars, *f);
				/* needed - try "printf %" without it */
				ఒకవేళ (p == ఖాళీ || *f == '\0') {
					bb_error_సందేశం("%s: invalid format", direc_ప్రారంభం);
					/* causes main() to exit with error */
					తిరిగి saved_argv - 1;
				}
				++direc_నిడివి;
				ఒకవేళ (p - format_chars <= 5) {
					/* it is one of "diouxX" */
					p = xmalloc(direc_నిడివి + 3);
					జ్ఞాపకం_నకలు(p, direc_ప్రారంభం, direc_నిడివి);
					p[direc_నిడివి + 1] = p[direc_నిడివి - 1];
					p[direc_నిడివి - 1] = 'l';
					p[direc_నిడివి] = 'l';
					//bb_error_msg("<%s>", p);
					direc_నిడివి += 2;
					direc_ప్రారంభం = p;
				} లేకపోతే {
					p = ఖాళీ;
				}
				ఒకవేళ (*argv) {
					print_direc(direc_ప్రారంభం, direc_నిడివి, క్షేత్రం_వెడల్పు,
								precision, *argv++);
				} లేకపోతే {
					print_direc(direc_ప్రారంభం, direc_నిడివి, క్షేత్రం_వెడల్పు,
								precision, "");
				}
				*conv_err |= errno;
				విడుదల(p);
			}
			ఆపు;
		స్థితి '\\':
			ఒకవేళ (*++f == 'c') {
				తిరిగి saved_argv; /* causes main() to exit */
			}
			bb_putchar(bb_process_escape_sequence((స్థిరరాశి అక్షరం **)&f));
			f--;
			ఆపు;
		అప్రమేయ:
			అక్షరం_రాయి(*f);
		}
	}

	తిరిగి argv;
}

పూర్ణాంకం printf_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	పూర్ణాంకం conv_err;
	అక్షరం *format;
	అక్షరం **argv2;

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
	ఒకవేళ (fcntl(1, F_GETFL) == -1)
		తిరిగి 1; /* match coreutils 6.10 (sans error msg to stderr) */
	//if (dup2(1, 1) != 1) - old way
	//	return 1;

	/* bash builtin errors out on "printf '-%s-\n' foo",
	 * coreutils-6.9 works. Both work with "printf -- '-%s-\n' foo".
	 * We will mimic coreutils. */
	argv = skip_dash_dash(argv);

	ఒకవేళ (!argv[0]) {
		ఒకవేళ ((ENABLE_ASH_PRINTF || ENABLE_HUSH_PRINTF)
		 && applet_name[0] != 'p'
		) {
			bb_simple_error_సందేశం("usage: printf FORMAT [ARGUMENT...]");
			తిరిగి 2; /* bash compat */
		}
		bb_show_usage();
	}

	format = argv[0];
	argv2 = argv + 1;

	conv_err = 0;
	చేయి {
		argv = argv2;
		argv2 = print_formatted(format, argv, &conv_err);
	} అయితే (argv2 > argv && *argv2);

	/* coreutils compat (bash doesn't do this):
	if (*argv)
		fprintf(stderr, "excess args ignored");
	*/

	తిరిగి (argv2 < argv) /* if true, print_formatted errored out */
		|| conv_err; /* print_formatted saw invalid number */
}
