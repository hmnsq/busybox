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
# define print_پهريون_N(fp,ڳڻپ,بائيٽون) print_پهريون_N(fp,ڳڻپ)
#جيڪڏهن_پورو
ساكن فراغ
print_پهريون_N(FILE *fp, بنانشان ڊگهو ڳڻپ, bool ڳڻپ_بائيٽون)
{
#إذا !ENABLE_FEATURE_FANCY_سر
	ثابت صحيح ڳڻپ_بائيٽون = 0;
#جيڪڏهن_پورو
	طالما (ڳڻپ) {
		صحيح c = getc(fp);
		إذا (c == EOF)
			اكسر;
		إذا (ڳڻپ_بائيٽون || (c == '\n'))
			--ڳڻپ;
		اکر_لکو(c);
	}
}

#إذا ENABLE_FEATURE_FANCY_سر
ساكن فراغ
print_except_N_آخري_بائيٽون(FILE *fp, بنانشان ڳڻپ)
{
	بنانشان حرف *circle = xmalloc(++ڳڻپ);
	بنانشان سر = 0;
	لكل (;;) {
		صحيح c;
		c = getc(fp);
		إذا (c == EOF)
			اذهب ret;
		circle[سر++] = c;
		إذا (سر == ڳڻپ)
			اكسر;
	}
	لكل (;;) {
		صحيح c;
		إذا (سر == ڳڻپ)
			سر = 0;
		اکر_لکو(circle[سر]);
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
print_except_N_آخري_سٽون(FILE *fp, بنانشان ڳڻپ)
{
	حرف **circle = xzalloc((++ڳڻپ) * حجم(circle[0]));
	بنانشان سر = 0;
	لكل (;;) {
		حرف *c;
		c = xmalloc_fgets(fp);
		إذا (!c)
			اذهب ret;
		circle[سر++] = c;
		إذا (سر == ڳڻپ)
			اكسر;
	}
	لكل (;;) {
		حرف *c;
		إذا (سر == ڳڻپ)
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
		إذا (سر == ڳڻپ)
			اكسر;
	}
	حرر(circle);
}
#وإلا
/* Must never be called */
فراغ print_except_N_آخري_بائيٽون(FILE *fp, بنانشان ڳڻپ);
فراغ print_except_N_آخري_سٽون(FILE *fp, بنانشان ڳڻپ);
#جيڪڏهن_پورو

#إذا !ENABLE_FEATURE_FANCY_سر
# define eat_انگ(negative_N,p) eat_انگ(p)
#جيڪڏهن_پورو
ساكن بنانشان ڊگهو
eat_انگ(bool *negative_N, ثابت حرف *p)
{
#إذا ENABLE_FEATURE_FANCY_سر
	إذا (*p == '-') {
		*negative_N = 1;
		p++;
	}
#جيڪڏهن_پورو
	أرجع xatoul_sfx(p, bkm_suffixes);
}

ساكن ثابت حرف سر_opts[] ALIGN1 =
	"n:"
#إذا ENABLE_FEATURE_FANCY_سر
	"c:qv"
#جيڪڏهن_پورو
	;

#وصف_ڪريو سرو_fmt_str "\n==> %s <==\n"

صحيح سر_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح سر_main(صحيح argc, حرف **argv)
{
	بنانشان ڊگهو ڳڻپ = 10;
#إذا ENABLE_FEATURE_FANCY_سر
	صحيح سرو_threshhold = 1;
	bool ڳڻپ_بائيٽون = 0;
	bool negative_N = 0;
#وإلا
# define سرو_threshhold 1
# define ڳڻپ_بائيٽون       0
# define negative_N        0
#جيڪڏهن_پورو
	FILE *fp;
	ثابت حرف *fmt;
	حرف *p;
	صحيح opt;
	صحيح retval = EXIT_SUCCESS;

#إذا ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_سر
	/* Allow legacy syntax of an initial numeric option without -n. */
	إذا (argv[1] && argv[1][0] == '-'
	 && انگ_آهي(argv[1][1])
	) {
		--argc;
		++argv;
		p = argv[0] + 1;
		اذهب GET_ڳڻپ;
	}
#جيڪڏهن_پورو

	/* No size benefit in converting this to getopt32 */
	طالما ((opt = getopt(argc, argv, سر_opts)) > 0) {
		بدل (opt) {
#إذا ENABLE_FEATURE_FANCY_سر
		حالة 'q':
			سرو_threshhold = وڌ_کان_وڌ_عدد;
			اكسر;
		حالة 'v':
			سرو_threshhold = -1;
			اكسر;
		حالة 'c':
			ڳڻپ_بائيٽون = 1;
			/* fall through */
#جيڪڏهن_پورو
		حالة 'n':
			p = optarg;
#إذا ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_سر
 GET_ڳڻپ:
#جيڪڏهن_پورو
			ڳڻپ = eat_انگ(&negative_N, p);
			اكسر;
		افتراضي:
			bb_show_usage();
		}
	}

	argc -= optind;
	argv += optind;
	إذا (!*argv)
		*--argv = (حرف*)"-";

	fmt = سرو_fmt_str + 1;
	إذا (argc <= سرو_threshhold) {
#إذا ENABLE_FEATURE_FANCY_سر
		سرو_threshhold = 0;
#وإلا
		fmt += 11; /* "" */
#جيڪڏهن_پورو
	}
	إذا (negative_N) {
		إذا (ڳڻپ >= وڌ_کان_وڌ_عدد / حجم(حرف*))
			bb_error_نياپو("count is too big: %lu", ڳڻپ);
	}

	افعل {
		fp = fopen_or_warn_stdin(*argv);
		إذا (fp) {
			إذا (fp == stdin) {
				*argv = (حرف *) bb_نياپو_standard_داخلا;
			}
			إذا (سرو_threshhold) {
				لکو(fmt, *argv);
			}
			إذا (negative_N) {
				إذا (ڳڻپ_بائيٽون) {
					print_except_N_آخري_بائيٽون(fp, ڳڻپ);
				} وإلا {
					print_except_N_آخري_سٽون(fp, ڳڻپ);
				}
			} وإلا {
				print_پهريون_N(fp, ڳڻپ, ڳڻپ_بائيٽون);
			}
			die_if_ferror_stdout();
			إذا (fclose_if_not_stdin(fp)) {
				bb_simple_perror_نياپو(*argv);
				retval = EXIT_FAILURE;
			}
		} وإلا {
			retval = EXIT_FAILURE;
		}
		fmt = سرو_fmt_str;
	} طالما (*++argv);

	fflush_stdout_and_exit(retval);
}
