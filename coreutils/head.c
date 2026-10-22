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

#اگر !ENABLE_FEATURE_FANCY_سر
# define print_پہلا_N(fp,گنتی,بائٹیں) print_پہلا_N(fp,گنتی)
#اگر_ختم
ساکن خالی
print_پہلا_N(FILE *fp, غیرعلامتی طویل گنتی, bool گنتی_بائٹیں)
{
#اگر !ENABLE_FEATURE_FANCY_سر
	ثابت صحیح گنتی_بائٹیں = 0;
#اگر_ختم
	جب (گنتی) {
		صحیح c = getc(fp);
		اگر (c == EOF)
			توڑو;
		اگر (گنتی_بائٹیں || (c == '\n'))
			--گنتی;
		حرف_لکھو(c);
	}
}

#اگر ENABLE_FEATURE_FANCY_سر
ساکن خالی
print_except_N_آخری_بائٹیں(FILE *fp, غیرعلامتی گنتی)
{
	غیرعلامتی حرف *circle = xmalloc(++گنتی);
	غیرعلامتی سر = 0;
	برائے (;;) {
		صحیح c;
		c = getc(fp);
		اگر (c == EOF)
			جاؤ ret;
		circle[سر++] = c;
		اگر (سر == گنتی)
			توڑو;
	}
	برائے (;;) {
		صحیح c;
		اگر (سر == گنتی)
			سر = 0;
		حرف_لکھو(circle[سر]);
		c = getc(fp);
		اگر (c == EOF)
			جاؤ ret;
		circle[سر] = c;
		سر++;
	}
 ret:
	آزاد(circle);
}

ساکن خالی
print_except_N_آخری_سطریں(FILE *fp, غیرعلامتی گنتی)
{
	حرف **circle = xzalloc((++گنتی) * حجم(circle[0]));
	غیرعلامتی سر = 0;
	برائے (;;) {
		حرف *c;
		c = xmalloc_fgets(fp);
		اگر (!c)
			جاؤ ret;
		circle[سر++] = c;
		اگر (سر == گنتی)
			توڑو;
	}
	برائے (;;) {
		حرف *c;
		اگر (سر == گنتی)
			سر = 0;
		fputs_stdout(circle[سر]);
		c = xmalloc_fgets(fp);
		اگر (!c)
			جاؤ ret;
		آزاد(circle[سر]);
		circle[سر++] = c;
	}
 ret:
	سر = 0;
	برائے (;;) {
		آزاد(circle[سر++]);
		اگر (سر == گنتی)
			توڑو;
	}
	آزاد(circle);
}
#ورنہ
/* Must never be called */
خالی print_except_N_آخری_بائٹیں(FILE *fp, غیرعلامتی گنتی);
خالی print_except_N_آخری_سطریں(FILE *fp, غیرعلامتی گنتی);
#اگر_ختم

#اگر !ENABLE_FEATURE_FANCY_سر
# define eat_عدد(negative_N,p) eat_عدد(p)
#اگر_ختم
ساکن غیرعلامتی طویل
eat_عدد(bool *negative_N, ثابت حرف *p)
{
#اگر ENABLE_FEATURE_FANCY_سر
	اگر (*p == '-') {
		*negative_N = 1;
		p++;
	}
#اگر_ختم
	واپس xatoul_sfx(p, bkm_suffixes);
}

ساکن ثابت حرف سر_opts[] ALIGN1 =
	"n:"
#اگر ENABLE_FEATURE_FANCY_سر
	"c:qv"
#اگر_ختم
	;

#تعریف_کرو سرخی_fmt_str "\n==> %s <==\n"

صحیح سر_main(صحیح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح سر_main(صحیح argc, حرف **argv)
{
	غیرعلامتی طویل گنتی = 10;
#اگر ENABLE_FEATURE_FANCY_سر
	صحیح سرخی_threshhold = 1;
	bool گنتی_بائٹیں = 0;
	bool negative_N = 0;
#ورنہ
# define سرخی_threshhold 1
# define گنتی_بائٹیں       0
# define negative_N        0
#اگر_ختم
	FILE *fp;
	ثابت حرف *fmt;
	حرف *p;
	صحیح opt;
	صحیح retval = EXIT_SUCCESS;

#اگر ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_سر
	/* Allow legacy syntax of an initial numeric option without -n. */
	اگر (argv[1] && argv[1][0] == '-'
	 && ہندسہ_ہے(argv[1][1])
	) {
		--argc;
		++argv;
		p = argv[0] + 1;
		جاؤ GET_گنتی;
	}
#اگر_ختم

	/* No size benefit in converting this to getopt32 */
	جب ((opt = getopt(argc, argv, سر_opts)) > 0) {
		انتخاب (opt) {
#اگر ENABLE_FEATURE_FANCY_سر
		حالت 'q':
			سرخی_threshhold = زیادہ_سے_زیادہ_عدد;
			توڑو;
		حالت 'v':
			سرخی_threshhold = -1;
			توڑو;
		حالت 'c':
			گنتی_بائٹیں = 1;
			/* fall through */
#اگر_ختم
		حالت 'n':
			p = optarg;
#اگر ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_سر
 GET_گنتی:
#اگر_ختم
			گنتی = eat_عدد(&negative_N, p);
			توڑو;
		طےشدہ:
			bb_show_usage();
		}
	}

	argc -= optind;
	argv += optind;
	اگر (!*argv)
		*--argv = (حرف*)"-";

	fmt = سرخی_fmt_str + 1;
	اگر (argc <= سرخی_threshhold) {
#اگر ENABLE_FEATURE_FANCY_سر
		سرخی_threshhold = 0;
#ورنہ
		fmt += 11; /* "" */
#اگر_ختم
	}
	اگر (negative_N) {
		اگر (گنتی >= زیادہ_سے_زیادہ_عدد / حجم(حرف*))
			bb_error_پیغام("count is too big: %lu", گنتی);
	}

	کرو {
		fp = fopen_or_warn_stdin(*argv);
		اگر (fp) {
			اگر (fp == stdin) {
				*argv = (حرف *) bb_پیغام_standard_داخلہ;
			}
			اگر (سرخی_threshhold) {
				لکھو(fmt, *argv);
			}
			اگر (negative_N) {
				اگر (گنتی_بائٹیں) {
					print_except_N_آخری_بائٹیں(fp, گنتی);
				} ورنہ {
					print_except_N_آخری_سطریں(fp, گنتی);
				}
			} ورنہ {
				print_پہلا_N(fp, گنتی, گنتی_بائٹیں);
			}
			die_if_ferror_stdout();
			اگر (fclose_if_not_stdin(fp)) {
				bb_simple_perror_پیغام(*argv);
				retval = EXIT_FAILURE;
			}
		} ورنہ {
			retval = EXIT_FAILURE;
		}
		fmt = سرخی_fmt_str;
	} جب (*++argv);

	fflush_stdout_and_exit(retval);
}
