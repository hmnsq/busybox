/* vi: set sw=4 ts=4: */
/*
 * head implementation for busybox
 *
 * Copyright (C) 2003  Manuel Novoa III  <mjn3@codepoet.org>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config HEAD
//config:	bool "head (4 kb)"
//config:	default y
//config:	help
//config:	head is used to print the first specified number of lines
//config:	from files.
//config:
//config:config FEATURE_FANCY_HEAD
//config:	bool "Enable -c, -q, and -v"
//config:	default y
//config:	depends on HEAD

//applet:IF_HEAD(APPLET_NOEXEC(head, head, BB_DIR_USR_BIN, BB_SUID_DROP, head))

//kbuild:lib-$(CONFIG_HEAD) += head.o

/* BB_AUDIT SUSv3 compliant */
/* BB_AUDIT GNU compatible -c, -q, and -v options in 'fancy' configuration. */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/head.html */

//usage:#define head_trivial_usage
//usage:       "[OPTIONS] [FILE]..."
//usage:#define head_full_usage "\n\n"
//usage:       "Print first 10 lines of FILEs (or stdin).\n"
//usage:       "With more than one FILE, precede each with a filename header.\n"
//usage:     "\n	-n N[bkm]	Print first N lines"
//usage:	IF_FEATURE_FANCY_HEAD(
//usage:     "\n	-n -N[bkm]	Print all except N last lines"
//usage:     "\n	-c [-]N[bkm]	Print first N bytes"
//usage:	)
//usage:     "\n			(b:*512 k:*1024 m:*1024^2)"
//usage:	IF_FEATURE_FANCY_HEAD(
//usage:     "\n	-q		Never print headers"
//usage:     "\n	-v		Always print headers"
//usage:	)
//usage:
//usage:#define head_example_usage
//usage:       "$ head -n 2 /etc/passwd\n"
//usage:       "root:x:0:0:root:/root:/bin/bash\n"
//usage:       "daemon:x:1:1:daemon:/usr/sbin:/bin/sh\n"

#অন্তর্ভাব "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

#যদি !ENABLE_FEATURE_FANCY_শীর্ষ
# define print_first_N(fp,গুনতি,bytes) print_first_N(fp,গুনতি)
#endif
স্থির শূন্য
print_first_N(FILE *fp, unsigned long গুনতি, bool গুনতি_bytes)
{
#যদি !ENABLE_FEATURE_FANCY_শীর্ষ
	ধ্রুবক পূর্ণসংখ্যা গুনতি_bytes = 0;
#endif
	যখন (গুনতি) {
		পূর্ণসংখ্যা c = getc(fp);
		যদি (c == EOF)
			বিরতি;
		যদি (গুনতি_bytes || (c == '\n'))
			--গুনতি;
		putchar(c);
	}
}

#যদি ENABLE_FEATURE_FANCY_শীর্ষ
স্থির শূন্য
print_except_N_last_bytes(FILE *fp, unsigned গুনতি)
{
	unsigned অক্ষর *circle = xmalloc(++গুনতি);
	unsigned শীর্ষ = 0;
	জন্য (;;) {
		পূর্ণসংখ্যা c;
		c = getc(fp);
		যদি (c == EOF)
			যাও ret;
		circle[শীর্ষ++] = c;
		যদি (শীর্ষ == গুনতি)
			বিরতি;
	}
	জন্য (;;) {
		পূর্ণসংখ্যা c;
		যদি (শীর্ষ == গুনতি)
			শীর্ষ = 0;
		putchar(circle[শীর্ষ]);
		c = getc(fp);
		যদি (c == EOF)
			যাও ret;
		circle[শীর্ষ] = c;
		শীর্ষ++;
	}
 ret:
	মুক্ত(circle);
}

স্থির শূন্য
print_except_N_last_সারিগুলি(FILE *fp, unsigned গুনতি)
{
	অক্ষর **circle = xzalloc((++গুনতি) * আকার(circle[0]));
	unsigned শীর্ষ = 0;
	জন্য (;;) {
		অক্ষর *c;
		c = xmalloc_fgets(fp);
		যদি (!c)
			যাও ret;
		circle[শীর্ষ++] = c;
		যদি (শীর্ষ == গুনতি)
			বিরতি;
	}
	জন্য (;;) {
		অক্ষর *c;
		যদি (শীর্ষ == গুনতি)
			শীর্ষ = 0;
		fputs_stdout(circle[শীর্ষ]);
		c = xmalloc_fgets(fp);
		যদি (!c)
			যাও ret;
		মুক্ত(circle[শীর্ষ]);
		circle[শীর্ষ++] = c;
	}
 ret:
	শীর্ষ = 0;
	জন্য (;;) {
		মুক্ত(circle[শীর্ষ++]);
		যদি (শীর্ষ == গুনতি)
			বিরতি;
	}
	মুক্ত(circle);
}
#অন্যথা
/* Must never be called */
শূন্য print_except_N_last_bytes(FILE *fp, unsigned গুনতি);
শূন্য print_except_N_last_সারিগুলি(FILE *fp, unsigned গুনতি);
#endif

#যদি !ENABLE_FEATURE_FANCY_শীর্ষ
# define eat_অঙ্ক(negative_N,p) eat_অঙ্ক(p)
#endif
স্থির unsigned long
eat_অঙ্ক(bool *negative_N, ধ্রুবক অক্ষর *p)
{
#যদি ENABLE_FEATURE_FANCY_শীর্ষ
	যদি (*p == '-') {
		*negative_N = 1;
		p++;
	}
#endif
	ফেরত xatoul_sfx(p, bkm_suffixes);
}

স্থির ধ্রুবক অক্ষর শীর্ষ_opts[] ALIGN1 =
	"n:"
#যদি ENABLE_FEATURE_FANCY_শীর্ষ
	"c:qv"
#endif
	;

#define header_fmt_str "\n==> %s <==\n"

পূর্ণসংখ্যা শীর্ষ_main(পূর্ণসংখ্যা argc, অক্ষর **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণসংখ্যা শীর্ষ_main(পূর্ণসংখ্যা argc, অক্ষর **argv)
{
	unsigned long গুনতি = 10;
#যদি ENABLE_FEATURE_FANCY_শীর্ষ
	পূর্ণসংখ্যা header_threshhold = 1;
	bool গুনতি_bytes = 0;
	bool negative_N = 0;
#অন্যথা
# define header_threshhold 1
# define গুনতি_bytes       0
# define negative_N        0
#endif
	FILE *fp;
	ধ্রুবক অক্ষর *fmt;
	অক্ষর *p;
	পূর্ণসংখ্যা opt;
	পূর্ণসংখ্যা retval = EXIT_SUCCESS;

#যদি ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_শীর্ষ
	/* Allow legacy syntax of an initial numeric option without -n. */
	যদি (argv[1] && argv[1][0] == '-'
	 && isdigit(argv[1][1])
	) {
		--argc;
		++argv;
		p = argv[0] + 1;
		যাও GET_গুনতি;
	}
#endif

	/* No size benefit in converting this to getopt32 */
	যখন ((opt = getopt(argc, argv, শীর্ষ_opts)) > 0) {
		নির্বাচন (opt) {
#যদি ENABLE_FEATURE_FANCY_শীর্ষ
		ক্ষেত্র 'q':
			header_threshhold = INT_MAX;
			বিরতি;
		ক্ষেত্র 'v':
			header_threshhold = -1;
			বিরতি;
		ক্ষেত্র 'c':
			গুনতি_bytes = 1;
			/* fall through */
#endif
		ক্ষেত্র 'n':
			p = optarg;
#যদি ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_শীর্ষ
 GET_গুনতি:
#endif
			গুনতি = eat_অঙ্ক(&negative_N, p);
			বিরতি;
		পূর্বনির্ধারিত:
			bb_show_usage();
		}
	}

	argc -= optind;
	argv += optind;
	যদি (!*argv)
		*--argv = (অক্ষর*)"-";

	fmt = header_fmt_str + 1;
	যদি (argc <= header_threshhold) {
#যদি ENABLE_FEATURE_FANCY_শীর্ষ
		header_threshhold = 0;
#অন্যথা
		fmt += 11; /* "" */
#endif
	}
	যদি (negative_N) {
		যদি (গুনতি >= INT_MAX / আকার(অক্ষর*))
			bb_error_বার্তা("count is too big: %lu", গুনতি);
	}

	করো {
		fp = fopen_or_warn_stdin(*argv);
		যদি (fp) {
			যদি (fp == stdin) {
				*argv = (অক্ষর *) bb_বার্তা_standard_প্রবেশ;
			}
			যদি (header_threshhold) {
				printf(fmt, *argv);
			}
			যদি (negative_N) {
				যদি (গুনতি_bytes) {
					print_except_N_last_bytes(fp, গুনতি);
				} অন্যথা {
					print_except_N_last_সারিগুলি(fp, গুনতি);
				}
			} অন্যথা {
				print_first_N(fp, গুনতি, গুনতি_bytes);
			}
			die_if_ferror_stdout();
			যদি (fclose_if_not_stdin(fp)) {
				bb_simple_perror_বার্তা(*argv);
				retval = EXIT_FAILURE;
			}
		} অন্যথা {
			retval = EXIT_FAILURE;
		}
		fmt = header_fmt_str;
	} যখন (*++argv);

	fflush_stdout_and_exit(retval);
}
