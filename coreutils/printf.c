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

#অন্তর্ভাব "libbb.h"

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

প্রকারলেখ শূন্য FAST_FUNC (*converter)(ধ্রুবক অক্ষর *arg, শূন্য *ফলাফল);

স্থির পূর্ণসংখ্যা multiconvert(ধ্রুবক অক্ষর *arg, শূন্য *ফলাফল, converter convert)
{
	যদি (*arg == '"' || *arg == '\'') {
		arg = utoa((unsigned অক্ষর)arg[1]);
	}
	errno = 0;
	convert(arg, ফলাফল);
	যদি (errno) {
		bb_error_বার্তা("invalid number '%s'", arg);
		ফেরত 1;
	}
	ফেরত 0;
}

স্থির শূন্য FAST_FUNC conv_strtoull(ধ্রুবক অক্ষর *arg, শূন্য *ফলাফল)
{
	/* Allow leading '+' - bb_strtoull() by itself does not allow it,
	 * and probably shouldn't (other callers might require purely numeric
	 * inputs to be allowed.
	 */
	যদি (arg[0] == '+')
		arg++;
	*(unsigned long long*)ফলাফল = bb_strtoull(arg, ফাঁকা, 0);
	/* both coreutils 6.10 and bash 3.2:
	 * $ printf '%x\n' -2
	 * fffffffffffffffe
	 * Mimic that:
	 */
	যদি (errno) {
		*(unsigned long long*)ফলাফল = bb_strtoll(arg, ফাঁকা, 0);
	}
}
স্থির শূন্য FAST_FUNC conv_strtoll(ধ্রুবক অক্ষর *arg, শূন্য *ফলাফল)
{
	যদি (arg[0] == '+')
		arg++;
	*(long long*)ফলাফল = bb_strtoll(arg, ফাঁকা, 0);
}
স্থির শূন্য FAST_FUNC conv_strtod(ধ্রুবক অক্ষর *arg, শূন্য *ফলাফল)
{
	অক্ষর *end;
	/* Well, this one allows leading whitespace... so what? */
	/* What I like much less is that "-" accepted too! :( */
//TODO: needs setlocale(LC_NUMERIC, "C")?
	*(দ্বিগুণ*)ফলাফল = strtod(arg, &end);
	যদি (end[0]) {
		errno = ERANGE;
		*(দ্বিগুণ*)ফলাফল = 0;
	}
}

/* Callers should check errno to detect errors */
স্থির unsigned long long my_xstrtoull(ধ্রুবক অক্ষর *arg)
{
	unsigned long long ফলাফল;
	যদি (multiconvert(arg, &ফলাফল, conv_strtoull))
		ফলাফল = 0;
	ফেরত ফলাফল;
}
স্থির long long my_xstrtoll(ধ্রুবক অক্ষর *arg)
{
	long long ফলাফল;
	যদি (multiconvert(arg, &ফলাফল, conv_strtoll))
		ফলাফল = 0;
	ফেরত ফলাফল;
}
স্থির দ্বিগুণ my_xstrtod(ধ্রুবক অক্ষর *arg)
{
	দ্বিগুণ ফলাফল;
	multiconvert(arg, &ফলাফল, conv_strtod);
	ফেরত ফলাফল;
}

/* Handles %b; return 1 if output is to be short-circuited by \c */
স্থির পূর্ণসংখ্যা print_esc_string(ধ্রুবক অক্ষর *str)
{
	অক্ষর c;
	যখন ((c = *str) != '\0') {
		str++;
		যদি (c == '\\') {
			/* %b also accepts 4-digit octals of the form \0### */
			যদি (*str == '0') {
				যদি ((unsigned অক্ষর)(str[1] - '0') < 8) {
					/* 2nd char is 0..7: skip leading '0' */
					str++;
				}
			}
			অন্যথা যদি (*str == 'c') {
				ফেরত 1;
			}
			{
				/* optimization: don't force arg to be on-stack,
				 * use another variable for that. */
				ধ্রুবক অক্ষর *z = str;
				c = bb_process_escape_sequence(&z);
				str = z;
			}
		}
		putchar(c);
	}

	ফেরত 0;
}

স্থির শূন্য print_direc(অক্ষর *format, unsigned fmt_দৈর্ঘ্য,
		পূর্ণসংখ্যা field_width, পূর্ণসংখ্যা precision,
		ধ্রুবক অক্ষর *argument)
{
	long long llv;
	দ্বিগুণ dv;
	অক্ষর saved;
	অক্ষর *have_prec, *have_width;

	saved = format[fmt_দৈর্ঘ্য];
	format[fmt_দৈর্ঘ্য] = '\0';

	have_prec = strstr(format, ".*");
	have_width = strchr(format, '*');
	যদি (have_width - 1 == have_prec)
		have_width = ফাঁকা;

	/* multiconvert sets errno = 0, but %s needs it cleared */
	errno = 0;

	নির্বাচন (format[fmt_দৈর্ঘ্য - 1]) {
	ক্ষেত্র 'c':
		printf(format, *argument);
		বিরতি;
	ক্ষেত্র 'd':
	ক্ষেত্র 'i':
		llv = my_xstrtoll(skip_whitespace(argument));
 print_long:
		যদি (!have_width) {
			যদি (!have_prec)
				printf(format, llv);
			অন্যথা
				printf(format, precision, llv);
		} অন্যথা {
			যদি (!have_prec)
				printf(format, field_width, llv);
			অন্যথা
				printf(format, field_width, precision, llv);
		}
		বিরতি;
	ক্ষেত্র 'o':
	ক্ষেত্র 'u':
	ক্ষেত্র 'x':
	ক্ষেত্র 'X':
		llv = my_xstrtoull(skip_whitespace(argument));
		/* cheat: unsigned long and long have same width, so... */
		যাও print_long;
	ক্ষেত্র 's':
		/* Are char* and long long the same? */
		যদি (আকার(argument) == আকার(llv)) {
			llv = (long long)(ptrdiff_t)argument;
			যাও print_long;
		} অন্যথা {
			/* Hope compiler will optimize it out by moving call
			 * instruction after the ifs... */
			যদি (!have_width) {
				যদি (!have_prec)
					printf(format, argument, /*unused:*/ argument, argument);
				অন্যথা
					printf(format, precision, argument, /*unused:*/ argument);
			} অন্যথা {
				যদি (!have_prec)
					printf(format, field_width, argument, /*unused:*/ argument);
				অন্যথা
					printf(format, field_width, precision, argument);
			}
			বিরতি;
		}
	ক্ষেত্র 'f':
	ক্ষেত্র 'e':
	ক্ষেত্র 'E':
	ক্ষেত্র 'g':
	ক্ষেত্র 'G':
		dv = my_xstrtod(argument);
		যদি (!have_width) {
			যদি (!have_prec)
				printf(format, dv);
			অন্যথা
				printf(format, precision, dv);
		} অন্যথা {
			যদি (!have_prec)
				printf(format, field_width, dv);
			অন্যথা
				printf(format, field_width, precision, dv);
		}
		বিরতি;
	} /* switch */

	format[fmt_দৈর্ঘ্য] = saved;
}

/* Handle params for "%*.*f". Negative numbers are ok (compat). */
স্থির পূর্ণসংখ্যা get_width_prec(ধ্রুবক অক্ষর *str)
{
	পূর্ণসংখ্যা v = bb_strtoi(str, ফাঁকা, 10);
	যদি (errno) {
		bb_error_বার্তা("invalid number '%s'", str);
		v = 0;
	}
	ফেরত v;
}

/* Print the text in FORMAT, using ARGV for arguments to any '%' directives.
   Return advanced ARGV.  */
স্থির অক্ষর **print_formatted(অক্ষর *f, অক্ষর **argv, পূর্ণসংখ্যা *conv_err)
{
	অক্ষর *direc_start;      /* Start of % directive.  */
	unsigned direc_দৈর্ঘ্য;  /* Length of % directive.  */
	পূর্ণসংখ্যা field_width;        /* Arg to first '*' */
	পূর্ণসংখ্যা precision;          /* Arg to second '*' */
	অক্ষর **saved_argv = argv;

	জন্য (; *f; ++f) {
		নির্বাচন (*f) {
		ক্ষেত্র '%':
			direc_start = f++;
			direc_দৈর্ঘ্য = 1;
			field_width = precision = 0;
			যদি (*f == '%') {
				bb_putchar('%');
				বিরতি;
			}
			যদি (*f == 'b') {
				যদি (*argv) {
					যদি (print_esc_string(*argv))
						ফেরত saved_argv; /* causes main() to exit */
					++argv;
				}
				বিরতি;
			}
			যখন (*f && strchr("-+ #0", *f)) {
				++f;
				++direc_দৈর্ঘ্য;
			}
			যদি (*f == '*') {
				++f;
				++direc_দৈর্ঘ্য;
				যদি (*argv)
					field_width = get_width_prec(*argv++);
			} অন্যথা {
				যখন (isdigit(*f)) {
					++f;
					++direc_দৈর্ঘ্য;
				}
			}
			যদি (*f == '.') {
				++f;
				++direc_দৈর্ঘ্য;
				যদি (*f == '*') {
					++f;
					++direc_দৈর্ঘ্য;
					যদি (*argv)
						precision = get_width_prec(*argv++);
				} অন্যথা {
					যখন (isdigit(*f)) {
						++f;
						++direc_দৈর্ঘ্য;
					}
				}
			}

			/* Remove "lLhz" size modifiers, repeatedly.
			 * bash does not like "%lld", but coreutils
			 * happily takes even "%Llllhhzhhzd"!
			 * We are permissive like coreutils */
			যখন ((*f | 0x20) == 'l' || *f == 'h' || *f == 'z') {
				overlapping_strcpy(f, f + 1);
			}
			/* Add "ll" if integer modifier, then print */
			{
				স্থির ধ্রুবক অক্ষর format_chars[] ALIGN1 = "diouxXfeEgGcs";
				অক্ষর *p = strchr(format_chars, *f);
				/* needed - try "printf %" without it */
				যদি (p == ফাঁকা || *f == '\0') {
					bb_error_বার্তা("%s: invalid format", direc_start);
					/* causes main() to exit with error */
					ফেরত saved_argv - 1;
				}
				++direc_দৈর্ঘ্য;
				যদি (p - format_chars <= 5) {
					/* it is one of "diouxX" */
					p = xmalloc(direc_দৈর্ঘ্য + 3);
					memcpy(p, direc_start, direc_দৈর্ঘ্য);
					p[direc_দৈর্ঘ্য + 1] = p[direc_দৈর্ঘ্য - 1];
					p[direc_দৈর্ঘ্য - 1] = 'l';
					p[direc_দৈর্ঘ্য] = 'l';
					//bb_error_msg("<%s>", p);
					direc_দৈর্ঘ্য += 2;
					direc_start = p;
				} অন্যথা {
					p = ফাঁকা;
				}
				যদি (*argv) {
					print_direc(direc_start, direc_দৈর্ঘ্য, field_width,
								precision, *argv++);
				} অন্যথা {
					print_direc(direc_start, direc_দৈর্ঘ্য, field_width,
								precision, "");
				}
				*conv_err |= errno;
				মুক্ত(p);
			}
			বিরতি;
		ক্ষেত্র '\\':
			যদি (*++f == 'c') {
				ফেরত saved_argv; /* causes main() to exit */
			}
			bb_putchar(bb_process_escape_sequence((ধ্রুবক অক্ষর **)&f));
			f--;
			বিরতি;
		পূর্বনির্ধারিত:
			putchar(*f);
		}
	}

	ফেরত argv;
}

পূর্ণসংখ্যা printf_main(পূর্ণসংখ্যা argc UNUSED_PARAM, অক্ষর **argv)
{
	পূর্ণসংখ্যা conv_err;
	অক্ষর *format;
	অক্ষর **argv2;

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
	যদি (fcntl(1, F_GETFL) == -1)
		ফেরত 1; /* match coreutils 6.10 (sans error msg to stderr) */
	//if (dup2(1, 1) != 1) - old way
	//	return 1;

	/* bash builtin errors out on "printf '-%s-\n' foo",
	 * coreutils-6.9 works. Both work with "printf -- '-%s-\n' foo".
	 * We will mimic coreutils. */
	argv = skip_dash_dash(argv);

	যদি (!argv[0]) {
		যদি ((ENABLE_ASH_PRINTF || ENABLE_HUSH_PRINTF)
		 && applet_name[0] != 'p'
		) {
			bb_simple_error_বার্তা("usage: printf FORMAT [ARGUMENT...]");
			ফেরত 2; /* bash compat */
		}
		bb_show_usage();
	}

	format = argv[0];
	argv2 = argv + 1;

	conv_err = 0;
	করো {
		argv = argv2;
		argv2 = print_formatted(format, argv, &conv_err);
	} যখন (argv2 > argv && *argv2);

	/* coreutils compat (bash doesn't do this):
	if (*argv)
		fprintf(stderr, "excess args ignored");
	*/

	ফেরত (argv2 < argv) /* if true, print_formatted errored out */
		|| conv_err; /* print_formatted saw invalid number */
}
