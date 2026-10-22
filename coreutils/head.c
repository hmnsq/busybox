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

#অন্তর্ভাৱ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

#যদি !ENABLE_FEATURE_FANCY_HEAD
# define print_first_N(fp,গুনতি,bytes) print_first_N(fp,গুনতি)
#endif
স্থির শূন্য
print_first_N(FILE *fp, unsigned long গুনতি, bool গুনতি_bytes)
{
#যদি !ENABLE_FEATURE_FANCY_HEAD
	ধ্রুৱ পূর্ণাঙ্ক গুনতি_bytes = 0;
#endif
	যেতিয়ালৈ (গুনতি) {
		পূর্ণাঙ্ক c = getc(fp);
		যদি (c == EOF)
			ভঙ্গ;
		যদি (গুনতি_bytes || (c == '\n'))
			--গুনতি;
		putchar(c);
	}
}

#যদি ENABLE_FEATURE_FANCY_HEAD
স্থির শূন্য
print_except_N_last_bytes(FILE *fp, unsigned গুনতি)
{
	unsigned ৱর্ণ *circle = xmalloc(++গুনতি);
	unsigned head = 0;
	বাবে (;;) {
		পূর্ণাঙ্ক c;
		c = getc(fp);
		যদি (c == EOF)
			গচ্ছ ret;
		circle[head++] = c;
		যদি (head == গুনতি)
			ভঙ্গ;
	}
	বাবে (;;) {
		পূর্ণাঙ্ক c;
		যদি (head == গুনতি)
			head = 0;
		putchar(circle[head]);
		c = getc(fp);
		যদি (c == EOF)
			গচ্ছ ret;
		circle[head] = c;
		head++;
	}
 ret:
	মুক্ত(circle);
}

স্থির শূন্য
print_except_N_last_সাৰিগুলি(FILE *fp, unsigned গুনতি)
{
	ৱর্ণ **circle = xzalloc((++গুনতি) * পরিমাণ(circle[0]));
	unsigned head = 0;
	বাবে (;;) {
		ৱর্ণ *c;
		c = xmalloc_fgets(fp);
		যদি (!c)
			গচ্ছ ret;
		circle[head++] = c;
		যদি (head == গুনতি)
			ভঙ্গ;
	}
	বাবে (;;) {
		ৱর্ণ *c;
		যদি (head == গুনতি)
			head = 0;
		fputs_stdout(circle[head]);
		c = xmalloc_fgets(fp);
		যদি (!c)
			গচ্ছ ret;
		মুক্ত(circle[head]);
		circle[head++] = c;
	}
 ret:
	head = 0;
	বাবে (;;) {
		মুক্ত(circle[head++]);
		যদি (head == গুনতি)
			ভঙ্গ;
	}
	মুক্ত(circle);
}
#অন্যথা
/* Must never be called */
শূন্য print_except_N_last_bytes(FILE *fp, unsigned গুনতি);
শূন্য print_except_N_last_সাৰিগুলি(FILE *fp, unsigned গুনতি);
#endif

#যদি !ENABLE_FEATURE_FANCY_HEAD
# define eat_অঙ্ক(negative_N,p) eat_অঙ্ক(p)
#endif
স্থির unsigned long
eat_অঙ্ক(bool *negative_N, ধ্রুৱ ৱর্ণ *p)
{
#যদি ENABLE_FEATURE_FANCY_HEAD
	যদি (*p == '-') {
		*negative_N = 1;
		p++;
	}
#endif
	ঘূৰাই xatoul_sfx(p, bkm_suffixes);
}

স্থির ধ্রুৱ ৱর্ণ head_opts[] ALIGN1 =
	"n:"
#যদি ENABLE_FEATURE_FANCY_HEAD
	"c:qv"
#endif
	;

#define header_fmt_str "\n==> %s <==\n"

পূর্ণাঙ্ক head_main(পূর্ণাঙ্ক argc, ৱর্ণ **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণাঙ্ক head_main(পূর্ণাঙ্ক argc, ৱর্ণ **argv)
{
	unsigned long গুনতি = 10;
#যদি ENABLE_FEATURE_FANCY_HEAD
	পূর্ণাঙ্ক header_threshhold = 1;
	bool গুনতি_bytes = 0;
	bool negative_N = 0;
#অন্যথা
# define header_threshhold 1
# define গুনতি_bytes       0
# define negative_N        0
#endif
	FILE *fp;
	ধ্রুৱ ৱর্ণ *fmt;
	ৱর্ণ *p;
	পূর্ণাঙ্ক opt;
	পূর্ণাঙ্ক retval = EXIT_SUCCESS;

#যদি ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_HEAD
	/* Allow legacy syntax of an initial numeric option without -n. */
	যদি (argv[1] && argv[1][0] == '-'
	 && isdigit(argv[1][1])
	) {
		--argc;
		++argv;
		p = argv[0] + 1;
		গচ্ছ GET_গুনতি;
	}
#endif

	/* No size benefit in converting this to getopt32 */
	যেতিয়ালৈ ((opt = getopt(argc, argv, head_opts)) > 0) {
		ৱিকল্প (opt) {
#যদি ENABLE_FEATURE_FANCY_HEAD
		পক্ষ 'q':
			header_threshhold = INT_MAX;
			ভঙ্গ;
		পক্ষ 'v':
			header_threshhold = -1;
			ভঙ্গ;
		পক্ষ 'c':
			গুনতি_bytes = 1;
			/* fall through */
#endif
		পক্ষ 'n':
			p = optarg;
#যদি ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_HEAD
 GET_গুনতি:
#endif
			গুনতি = eat_অঙ্ক(&negative_N, p);
			ভঙ্গ;
		স্ৱভাৱ:
			bb_show_usage();
		}
	}

	argc -= optind;
	argv += optind;
	যদি (!*argv)
		*--argv = (ৱর্ণ*)"-";

	fmt = header_fmt_str + 1;
	যদি (argc <= header_threshhold) {
#যদি ENABLE_FEATURE_FANCY_HEAD
		header_threshhold = 0;
#অন্যথা
		fmt += 11; /* "" */
#endif
	}
	যদি (negative_N) {
		যদি (গুনতি >= INT_MAX / পরিমাণ(ৱর্ণ*))
			bb_error_বাৰ্তা("count is too big: %lu", গুনতি);
	}

	কৰা {
		fp = fopen_or_warn_stdin(*argv);
		যদি (fp) {
			যদি (fp == stdin) {
				*argv = (ৱর্ণ *) bb_বাৰ্তা_standard_প্ৰবেশ;
			}
			যদি (header_threshhold) {
				printf(fmt, *argv);
			}
			যদি (negative_N) {
				যদি (গুনতি_bytes) {
					print_except_N_last_bytes(fp, গুনতি);
				} অন্যথা {
					print_except_N_last_সাৰিগুলি(fp, গুনতি);
				}
			} অন্যথা {
				print_first_N(fp, গুনতি, গুনতি_bytes);
			}
			die_if_ferror_stdout();
			যদি (fclose_if_not_stdin(fp)) {
				bb_simple_perror_বাৰ্তা(*argv);
				retval = EXIT_FAILURE;
			}
		} অন্যথা {
			retval = EXIT_FAILURE;
		}
		fmt = header_fmt_str;
	} যেতিয়ালৈ (*++argv);

	fflush_stdout_and_exit(retval);
}
