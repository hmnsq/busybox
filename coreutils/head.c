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

#شامل "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

#اگر !ENABLE_FEATURE_FANCY_HEAD
# define print_first_N(fp,شمار,bytes) print_first_N(fp,شمار)
#endif
ایستا تهی
print_first_N(FILE *fp, unsigned long شمار, bool شمار_bytes)
{
#اگر !ENABLE_FEATURE_FANCY_HEAD
	ثابت صحیح شمار_bytes = 0;
#endif
	وقتی (شمار) {
		صحیح c = getc(fp);
		اگر (c == EOF)
			بشکن;
		اگر (شمار_bytes || (c == '\n'))
			--شمار;
		putchar(c);
	}
}

#اگر ENABLE_FEATURE_FANCY_HEAD
ایستا تهی
print_except_N_last_bytes(FILE *fp, unsigned شمار)
{
	unsigned نویسه *circle = xmalloc(++شمار);
	unsigned head = 0;
	برای (;;) {
		صحیح c;
		c = getc(fp);
		اگر (c == EOF)
			برو ret;
		circle[head++] = c;
		اگر (head == شمار)
			بشکن;
	}
	برای (;;) {
		صحیح c;
		اگر (head == شمار)
			head = 0;
		putchar(circle[head]);
		c = getc(fp);
		اگر (c == EOF)
			برو ret;
		circle[head] = c;
		head++;
	}
 ret:
	آزاد(circle);
}

ایستا تهی
print_except_N_last_سطرها(FILE *fp, unsigned شمار)
{
	نویسه **circle = xzalloc((++شمار) * اندازه(circle[0]));
	unsigned head = 0;
	برای (;;) {
		نویسه *c;
		c = xmalloc_fgets(fp);
		اگر (!c)
			برو ret;
		circle[head++] = c;
		اگر (head == شمار)
			بشکن;
	}
	برای (;;) {
		نویسه *c;
		اگر (head == شمار)
			head = 0;
		fputs_stdout(circle[head]);
		c = xmalloc_fgets(fp);
		اگر (!c)
			برو ret;
		آزاد(circle[head]);
		circle[head++] = c;
	}
 ret:
	head = 0;
	برای (;;) {
		آزاد(circle[head++]);
		اگر (head == شمار)
			بشکن;
	}
	آزاد(circle);
}
#وگرنه
/* Must never be called */
تهی print_except_N_last_bytes(FILE *fp, unsigned شمار);
تهی print_except_N_last_سطرها(FILE *fp, unsigned شمار);
#endif

#اگر !ENABLE_FEATURE_FANCY_HEAD
# define eat_رقم(negative_N,p) eat_رقم(p)
#endif
ایستا unsigned long
eat_رقم(bool *negative_N, ثابت نویسه *p)
{
#اگر ENABLE_FEATURE_FANCY_HEAD
	اگر (*p == '-') {
		*negative_N = 1;
		p++;
	}
#endif
	بازگشت xatoul_sfx(p, bkm_suffixes);
}

ایستا ثابت نویسه head_opts[] ALIGN1 =
	"n:"
#اگر ENABLE_FEATURE_FANCY_HEAD
	"c:qv"
#endif
	;

#define header_fmt_str "\n==> %s <==\n"

صحیح head_main(صحیح argc, نویسه **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح head_main(صحیح argc, نویسه **argv)
{
	unsigned long شمار = 10;
#اگر ENABLE_FEATURE_FANCY_HEAD
	صحیح header_threshhold = 1;
	bool شمار_bytes = 0;
	bool negative_N = 0;
#وگرنه
# define header_threshhold 1
# define شمار_bytes       0
# define negative_N        0
#endif
	FILE *fp;
	ثابت نویسه *fmt;
	نویسه *p;
	صحیح opt;
	صحیح retval = EXIT_SUCCESS;

#اگر ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_HEAD
	/* Allow legacy syntax of an initial numeric option without -n. */
	اگر (argv[1] && argv[1][0] == '-'
	 && isdigit(argv[1][1])
	) {
		--argc;
		++argv;
		p = argv[0] + 1;
		برو GET_شمار;
	}
#endif

	/* No size benefit in converting this to getopt32 */
	وقتی ((opt = getopt(argc, argv, head_opts)) > 0) {
		انتخاب (opt) {
#اگر ENABLE_FEATURE_FANCY_HEAD
		حالت 'q':
			header_threshhold = INT_MAX;
			بشکن;
		حالت 'v':
			header_threshhold = -1;
			بشکن;
		حالت 'c':
			شمار_bytes = 1;
			/* fall through */
#endif
		حالت 'n':
			p = optarg;
#اگر ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_HEAD
 GET_شمار:
#endif
			شمار = eat_رقم(&negative_N, p);
			بشکن;
		پیشفرض:
			bb_show_usage();
		}
	}

	argc -= optind;
	argv += optind;
	اگر (!*argv)
		*--argv = (نویسه*)"-";

	fmt = header_fmt_str + 1;
	اگر (argc <= header_threshhold) {
#اگر ENABLE_FEATURE_FANCY_HEAD
		header_threshhold = 0;
#وگرنه
		fmt += 11; /* "" */
#endif
	}
	اگر (negative_N) {
		اگر (شمار >= INT_MAX / اندازه(نویسه*))
			bb_error_پیام("count is too big: %lu", شمار);
	}

	انجامبده {
		fp = fopen_or_warn_stdin(*argv);
		اگر (fp) {
			اگر (fp == stdin) {
				*argv = (نویسه *) bb_پیام_standard_درون_داد;
			}
			اگر (header_threshhold) {
				printf(fmt, *argv);
			}
			اگر (negative_N) {
				اگر (شمار_bytes) {
					print_except_N_last_bytes(fp, شمار);
				} وگرنه {
					print_except_N_last_سطرها(fp, شمار);
				}
			} وگرنه {
				print_first_N(fp, شمار, شمار_bytes);
			}
			die_if_ferror_stdout();
			اگر (fclose_if_not_stdin(fp)) {
				bb_simple_perror_پیام(*argv);
				retval = EXIT_FAILURE;
			}
		} وگرنه {
			retval = EXIT_FAILURE;
		}
		fmt = header_fmt_str;
	} وقتی (*++argv);

	fflush_stdout_and_exit(retval);
}
