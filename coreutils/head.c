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

#إذا !ENABLE_FEATURE_FANCY_سر
# define print_اولی_N(fp,شمار,بایتها) print_اولی_N(fp,شمار)
#پایان_اگر
ساكن فراغ
print_اولی_N(FILE *fp, بیعلامت دراز شمار, bool شمار_بایتها)
{
#إذا !ENABLE_FEATURE_FANCY_سر
	ثابت صحيح شمار_بایتها = 0;
#پایان_اگر
	طالما (شمار) {
		صحيح c = getc(fp);
		إذا (c == EOF)
			اكسر;
		إذا (شمار_بایتها || (c == '\n'))
			--شمار;
		حرف_چاپ_کن(c);
	}
}

#إذا ENABLE_FEATURE_FANCY_سر
ساكن فراغ
print_except_N_آخرین_بایتها(FILE *fp, بیعلامت شمار)
{
	بیعلامت حرف *circle = xmalloc(++شمار);
	بیعلامت سر = 0;
	لكل (;;) {
		صحيح c;
		c = getc(fp);
		إذا (c == EOF)
			اذهب ret;
		circle[سر++] = c;
		إذا (سر == شمار)
			اكسر;
	}
	لكل (;;) {
		صحيح c;
		إذا (سر == شمار)
			سر = 0;
		حرف_چاپ_کن(circle[سر]);
		c = getc(fp);
		إذا (c == EOF)
			اذهب ret;
		circle[سر] = c;
		سر++;
	}
 ret:
	حرر(circle);
}

ساكن فراغ
print_except_N_آخرین_سطرها(FILE *fp, بیعلامت شمار)
{
	حرف **circle = xzalloc((++شمار) * حجم(circle[0]));
	بیعلامت سر = 0;
	لكل (;;) {
		حرف *c;
		c = xmalloc_fgets(fp);
		إذا (!c)
			اذهب ret;
		circle[سر++] = c;
		إذا (سر == شمار)
			اكسر;
	}
	لكل (;;) {
		حرف *c;
		إذا (سر == شمار)
			سر = 0;
		fputs_stdout(circle[سر]);
		c = xmalloc_fgets(fp);
		إذا (!c)
			اذهب ret;
		حرر(circle[سر]);
		circle[سر++] = c;
	}
 ret:
	سر = 0;
	لكل (;;) {
		حرر(circle[سر++]);
		إذا (سر == شمار)
			اكسر;
	}
	حرر(circle);
}
#وإلا
/* Must never be called */
فراغ print_except_N_آخرین_بایتها(FILE *fp, بیعلامت شمار);
فراغ print_except_N_آخرین_سطرها(FILE *fp, بیعلامت شمار);
#پایان_اگر

#إذا !ENABLE_FEATURE_FANCY_سر
# define eat_رقم(negative_N,p) eat_رقم(p)
#پایان_اگر
ساكن بیعلامت دراز
eat_رقم(bool *negative_N, ثابت حرف *p)
{
#إذا ENABLE_FEATURE_FANCY_سر
	إذا (*p == '-') {
		*negative_N = 1;
		p++;
	}
#پایان_اگر
	أرجع xatoul_sfx(p, bkm_suffixes);
}

ساكن ثابت حرف سر_opts[] ALIGN1 =
	"n:"
#إذا ENABLE_FEATURE_FANCY_سر
	"c:qv"
#پایان_اگر
	;

#تعریف_کن سرآیند_fmt_str "\n==> %s <==\n"

صحيح سر_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح سر_main(صحيح argc, حرف **argv)
{
	بیعلامت دراز شمار = 10;
#إذا ENABLE_FEATURE_FANCY_سر
	صحيح سرآیند_threshhold = 1;
	bool شمار_بایتها = 0;
	bool negative_N = 0;
#وإلا
# define سرآیند_threshhold 1
# define شمار_بایتها       0
# define negative_N        0
#پایان_اگر
	FILE *fp;
	ثابت حرف *fmt;
	حرف *p;
	صحيح opt;
	صحيح retval = EXIT_SUCCESS;

#إذا ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_سر
	/* Allow legacy syntax of an initial numeric option without -n. */
	إذا (argv[1] && argv[1][0] == '-'
	 && آیا_رقم(argv[1][1])
	) {
		--argc;
		++argv;
		p = argv[0] + 1;
		اذهب GET_شمار;
	}
#پایان_اگر

	/* No size benefit in converting this to getopt32 */
	طالما ((opt = getopt(argc, argv, سر_opts)) > 0) {
		بدل (opt) {
#إذا ENABLE_FEATURE_FANCY_سر
		حالة 'q':
			سرآیند_threshhold = بیشترین_صحیح;
			اكسر;
		حالة 'v':
			سرآیند_threshhold = -1;
			اكسر;
		حالة 'c':
			شمار_بایتها = 1;
			/* fall through */
#پایان_اگر
		حالة 'n':
			p = optarg;
#إذا ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_سر
 GET_شمار:
#پایان_اگر
			شمار = eat_رقم(&negative_N, p);
			اكسر;
		افتراضي:
			bb_show_usage();
		}
	}

	argc -= optind;
	argv += optind;
	إذا (!*argv)
		*--argv = (حرف*)"-";

	fmt = سرآیند_fmt_str + 1;
	إذا (argc <= سرآیند_threshhold) {
#إذا ENABLE_FEATURE_FANCY_سر
		سرآیند_threshhold = 0;
#وإلا
		fmt += 11; /* "" */
#پایان_اگر
	}
	إذا (negative_N) {
		إذا (شمار >= بیشترین_صحیح / حجم(حرف*))
			bb_error_پیام("count is too big: %lu", شمار);
	}

	افعل {
		fp = fopen_or_warn_stdin(*argv);
		إذا (fp) {
			إذا (fp == stdin) {
				*argv = (حرف *) bb_پیام_standard_درون_داد;
			}
			إذا (سرآیند_threshhold) {
				چاپ_کن(fmt, *argv);
			}
			إذا (negative_N) {
				إذا (شمار_بایتها) {
					print_except_N_آخرین_بایتها(fp, شمار);
				} وإلا {
					print_except_N_آخرین_سطرها(fp, شمار);
				}
			} وإلا {
				print_اولی_N(fp, شمار, شمار_بایتها);
			}
			die_if_ferror_stdout();
			إذا (fclose_if_not_stdin(fp)) {
				bb_simple_perror_پیام(*argv);
				retval = EXIT_FAILURE;
			}
		} وإلا {
			retval = EXIT_FAILURE;
		}
		fmt = سرآیند_fmt_str;
	} طالما (*++argv);

	fflush_stdout_and_exit(retval);
}
