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

#ضمن "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

#إذا !ENABLE_FEATURE_FANCY_HEAD
# define print_first_N(fp,ژماردن,bytes) print_first_N(fp,ژماردن)
#endif
ساكن فراغ
print_first_N(FILE *fp, unsigned long ژماردن, bool ژماردن_bytes)
{
#إذا !ENABLE_FEATURE_FANCY_HEAD
	ثابت صحيح ژماردن_bytes = 0;
#endif
	طالما (ژماردن) {
		صحيح c = getc(fp);
		إذا (c == EOF)
			اكسر;
		إذا (ژماردن_bytes || (c == '\n'))
			--ژماردن;
		putchar(c);
	}
}

#إذا ENABLE_FEATURE_FANCY_HEAD
ساكن فراغ
print_except_N_last_bytes(FILE *fp, unsigned ژماردن)
{
	unsigned حرف *circle = xmalloc(++ژماردن);
	unsigned head = 0;
	لكل (;;) {
		صحيح c;
		c = getc(fp);
		إذا (c == EOF)
			اذهب ret;
		circle[head++] = c;
		إذا (head == ژماردن)
			اكسر;
	}
	لكل (;;) {
		صحيح c;
		إذا (head == ژماردن)
			head = 0;
		putchar(circle[head]);
		c = getc(fp);
		إذا (c == EOF)
			اذهب ret;
		circle[head] = c;
		head++;
	}
 ret:
	حرر(circle);
}

ساكن فراغ
print_except_N_last_هێڵەکان(FILE *fp, unsigned ژماردن)
{
	حرف **circle = xzalloc((++ژماردن) * حجم(circle[0]));
	unsigned head = 0;
	لكل (;;) {
		حرف *c;
		c = xmalloc_fgets(fp);
		إذا (!c)
			اذهب ret;
		circle[head++] = c;
		إذا (head == ژماردن)
			اكسر;
	}
	لكل (;;) {
		حرف *c;
		إذا (head == ژماردن)
			head = 0;
		fputs_stdout(circle[head]);
		c = xmalloc_fgets(fp);
		إذا (!c)
			اذهب ret;
		حرر(circle[head]);
		circle[head++] = c;
	}
 ret:
	head = 0;
	لكل (;;) {
		حرر(circle[head++]);
		إذا (head == ژماردن)
			اكسر;
	}
	حرر(circle);
}
#وإلا
/* Must never be called */
فراغ print_except_N_last_bytes(FILE *fp, unsigned ژماردن);
فراغ print_except_N_last_هێڵەکان(FILE *fp, unsigned ژماردن);
#endif

#إذا !ENABLE_FEATURE_FANCY_HEAD
# define eat_ژمارە(negative_N,p) eat_ژمارە(p)
#endif
ساكن unsigned long
eat_ژمارە(bool *negative_N, ثابت حرف *p)
{
#إذا ENABLE_FEATURE_FANCY_HEAD
	إذا (*p == '-') {
		*negative_N = 1;
		p++;
	}
#endif
	أرجع xatoul_sfx(p, bkm_suffixes);
}

ساكن ثابت حرف head_opts[] ALIGN1 =
	"n:"
#إذا ENABLE_FEATURE_FANCY_HEAD
	"c:qv"
#endif
	;

#define header_fmt_str "\n==> %s <==\n"

صحيح head_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح head_main(صحيح argc, حرف **argv)
{
	unsigned long ژماردن = 10;
#إذا ENABLE_FEATURE_FANCY_HEAD
	صحيح header_threshhold = 1;
	bool ژماردن_bytes = 0;
	bool negative_N = 0;
#وإلا
# define header_threshhold 1
# define ژماردن_bytes       0
# define negative_N        0
#endif
	FILE *fp;
	ثابت حرف *fmt;
	حرف *p;
	صحيح opt;
	صحيح retval = EXIT_SUCCESS;

#إذا ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_HEAD
	/* Allow legacy syntax of an initial numeric option without -n. */
	إذا (argv[1] && argv[1][0] == '-'
	 && isdigit(argv[1][1])
	) {
		--argc;
		++argv;
		p = argv[0] + 1;
		اذهب GET_ژماردن;
	}
#endif

	/* No size benefit in converting this to getopt32 */
	طالما ((opt = getopt(argc, argv, head_opts)) > 0) {
		بدل (opt) {
#إذا ENABLE_FEATURE_FANCY_HEAD
		حالة 'q':
			header_threshhold = INT_MAX;
			اكسر;
		حالة 'v':
			header_threshhold = -1;
			اكسر;
		حالة 'c':
			ژماردن_bytes = 1;
			/* fall through */
#endif
		حالة 'n':
			p = optarg;
#إذا ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_HEAD
 GET_ژماردن:
#endif
			ژماردن = eat_ژمارە(&negative_N, p);
			اكسر;
		افتراضي:
			bb_show_usage();
		}
	}

	argc -= optind;
	argv += optind;
	إذا (!*argv)
		*--argv = (حرف*)"-";

	fmt = header_fmt_str + 1;
	إذا (argc <= header_threshhold) {
#إذا ENABLE_FEATURE_FANCY_HEAD
		header_threshhold = 0;
#وإلا
		fmt += 11; /* "" */
#endif
	}
	إذا (negative_N) {
		إذا (ژماردن >= INT_MAX / حجم(حرف*))
			bb_error_پەیام("count is too big: %lu", ژماردن);
	}

	افعل {
		fp = fopen_or_warn_stdin(*argv);
		إذا (fp) {
			إذا (fp == stdin) {
				*argv = (حرف *) bb_پەیام_standard_تێخستن;
			}
			إذا (header_threshhold) {
				printf(fmt, *argv);
			}
			إذا (negative_N) {
				إذا (ژماردن_bytes) {
					print_except_N_last_bytes(fp, ژماردن);
				} وإلا {
					print_except_N_last_هێڵەکان(fp, ژماردن);
				}
			} وإلا {
				print_first_N(fp, ژماردن, ژماردن_bytes);
			}
			die_if_ferror_stdout();
			إذا (fclose_if_not_stdin(fp)) {
				bb_simple_perror_پەیام(*argv);
				retval = EXIT_FAILURE;
			}
		} وإلا {
			retval = EXIT_FAILURE;
		}
		fmt = header_fmt_str;
	} طالما (*++argv);

	fflush_stdout_and_exit(retval);
}
