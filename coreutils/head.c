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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

#ਜੇ !ENABLE_FEATURE_FANCY_ਸਿਰ
# define print_ਪਹਿਲਾ_N(fp,ਗਿਣਤੀ_2,ਬਾਈਟਾਂ) print_ਪਹਿਲਾ_N(fp,ਗਿਣਤੀ_2)
#ਜੇ_ਅੰਤ
ਸਥਿਰ ਖਾਲੀ
print_ਪਹਿਲਾ_N(FILE *fp, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਗਿਣਤੀ_2, bool ਗਿਣਤੀ_ਬਾਈਟਾਂ)
{
#ਜੇ !ENABLE_FEATURE_FANCY_ਸਿਰ
	ਅਚਲ ਪੂਰਨ ਗਿਣਤੀ_ਬਾਈਟਾਂ = 0;
#ਜੇ_ਅੰਤ
	ਜਦੋਂ (ਗਿਣਤੀ_2) {
		ਪੂਰਨ c = getc(fp);
		ਜੇ (c == EOF)
			ਰੋਕੋ;
		ਜੇ (ਗਿਣਤੀ_ਬਾਈਟਾਂ || (c == '\n'))
			--ਗਿਣਤੀ_2;
		ਅੱਖਰ_ਲਿਖੋ(c);
	}
}

#ਜੇ ENABLE_FEATURE_FANCY_ਸਿਰ
ਸਥਿਰ ਖਾਲੀ
print_except_N_ਆਖਰੀ_ਬਾਈਟਾਂ(FILE *fp, ਬਿਨਾਨਿਸ਼ਾਨੀ ਗਿਣਤੀ_2)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *circle = xmalloc(++ਗਿਣਤੀ_2);
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਿਰ = 0;
	ਲਈ (;;) {
		ਪੂਰਨ c;
		c = getc(fp);
		ਜੇ (c == EOF)
			ਜਾਓ ret;
		circle[ਸਿਰ++] = c;
		ਜੇ (ਸਿਰ == ਗਿਣਤੀ_2)
			ਰੋਕੋ;
	}
	ਲਈ (;;) {
		ਪੂਰਨ c;
		ਜੇ (ਸਿਰ == ਗਿਣਤੀ_2)
			ਸਿਰ = 0;
		ਅੱਖਰ_ਲਿਖੋ(circle[ਸਿਰ]);
		c = getc(fp);
		ਜੇ (c == EOF)
			ਜਾਓ ret;
		circle[ਸਿਰ] = c;
		ਸਿਰ++;
	}
 ret:
	ਮੁਕਤ(circle);
}

ਸਥਿਰ ਖਾਲੀ
print_except_N_ਆਖਰੀ_ਸਤਰਾਂ(FILE *fp, ਬਿਨਾਨਿਸ਼ਾਨੀ ਗਿਣਤੀ_2)
{
	ਅੱਖਰ **circle = xzalloc((++ਗਿਣਤੀ_2) * ਆਕਾਰ(circle[0]));
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਿਰ = 0;
	ਲਈ (;;) {
		ਅੱਖਰ *c;
		c = xmalloc_fgets(fp);
		ਜੇ (!c)
			ਜਾਓ ret;
		circle[ਸਿਰ++] = c;
		ਜੇ (ਸਿਰ == ਗਿਣਤੀ_2)
			ਰੋਕੋ;
	}
	ਲਈ (;;) {
		ਅੱਖਰ *c;
		ਜੇ (ਸਿਰ == ਗਿਣਤੀ_2)
			ਸਿਰ = 0;
		fputs_stdout(circle[ਸਿਰ]);
		c = xmalloc_fgets(fp);
		ਜੇ (!c)
			ਜਾਓ ret;
		ਮੁਕਤ(circle[ਸਿਰ]);
		circle[ਸਿਰ++] = c;
	}
 ret:
	ਸਿਰ = 0;
	ਲਈ (;;) {
		ਮੁਕਤ(circle[ਸਿਰ++]);
		ਜੇ (ਸਿਰ == ਗਿਣਤੀ_2)
			ਰੋਕੋ;
	}
	ਮੁਕਤ(circle);
}
#ਨਹੀਂਤਾਂ
/* Must never be called */
ਖਾਲੀ print_except_N_ਆਖਰੀ_ਬਾਈਟਾਂ(FILE *fp, ਬਿਨਾਨਿਸ਼ਾਨੀ ਗਿਣਤੀ_2);
ਖਾਲੀ print_except_N_ਆਖਰੀ_ਸਤਰਾਂ(FILE *fp, ਬਿਨਾਨਿਸ਼ਾਨੀ ਗਿਣਤੀ_2);
#ਜੇ_ਅੰਤ

#ਜੇ !ENABLE_FEATURE_FANCY_ਸਿਰ
# define eat_ਅੰਕ(negative_N,p) eat_ਅੰਕ(p)
#ਜੇ_ਅੰਤ
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ
eat_ਅੰਕ(bool *negative_N, ਅਚਲ ਅੱਖਰ *p)
{
#ਜੇ ENABLE_FEATURE_FANCY_ਸਿਰ
	ਜੇ (*p == '-') {
		*negative_N = 1;
		p++;
	}
#ਜੇ_ਅੰਤ
	ਵਾਪਸ xatoul_sfx(p, bkm_suffixes);
}

ਸਥਿਰ ਅਚਲ ਅੱਖਰ ਸਿਰ_opts[] ALIGN1 =
	"n:"
#ਜੇ ENABLE_FEATURE_FANCY_ਸਿਰ
	"c:qv"
#ਜੇ_ਅੰਤ
	;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸਿਰਲੇਖ_fmt_str "\n==> %s <==\n"

ਪੂਰਨ ਸਿਰ_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ ਸਿਰ_main(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਗਿਣਤੀ_2 = 10;
#ਜੇ ENABLE_FEATURE_FANCY_ਸਿਰ
	ਪੂਰਨ ਸਿਰਲੇਖ_threshhold = 1;
	bool ਗਿਣਤੀ_ਬਾਈਟਾਂ = 0;
	bool negative_N = 0;
#ਨਹੀਂਤਾਂ
# define ਸਿਰਲੇਖ_threshhold 1
# define ਗਿਣਤੀ_ਬਾਈਟਾਂ       0
# define negative_N        0
#ਜੇ_ਅੰਤ
	FILE *fp;
	ਅਚਲ ਅੱਖਰ *fmt;
	ਅੱਖਰ *p;
	ਪੂਰਨ opt;
	ਪੂਰਨ retval = EXIT_SUCCESS;

#ਜੇ ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_ਸਿਰ
	/* Allow legacy syntax of an initial numeric option without -n. */
	ਜੇ (argv[1] && argv[1][0] == '-'
	 && ਅੰਕ_ਹੈ(argv[1][1])
	) {
		--argc;
		++argv;
		p = argv[0] + 1;
		ਜਾਓ GET_ਗਿਣਤੀ;
	}
#ਜੇ_ਅੰਤ

	/* No size benefit in converting this to getopt32 */
	ਜਦੋਂ ((opt = getopt(argc, argv, ਸਿਰ_opts)) > 0) {
		ਚੋਣ (opt) {
#ਜੇ ENABLE_FEATURE_FANCY_ਸਿਰ
		ਹਾਲਤ 'q':
			ਸਿਰਲੇਖ_threshhold = ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ;
			ਰੋਕੋ;
		ਹਾਲਤ 'v':
			ਸਿਰਲੇਖ_threshhold = -1;
			ਰੋਕੋ;
		ਹਾਲਤ 'c':
			ਗਿਣਤੀ_ਬਾਈਟਾਂ = 1;
			/* fall through */
#ਜੇ_ਅੰਤ
		ਹਾਲਤ 'n':
			p = optarg;
#ਜੇ ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_ਸਿਰ
 GET_ਗਿਣਤੀ:
#ਜੇ_ਅੰਤ
			ਗਿਣਤੀ_2 = eat_ਅੰਕ(&negative_N, p);
			ਰੋਕੋ;
		ਮੂਲ:
			bb_show_usage();
		}
	}

	argc -= optind;
	argv += optind;
	ਜੇ (!*argv)
		*--argv = (ਅੱਖਰ*)"-";

	fmt = ਸਿਰਲੇਖ_fmt_str + 1;
	ਜੇ (argc <= ਸਿਰਲੇਖ_threshhold) {
#ਜੇ ENABLE_FEATURE_FANCY_ਸਿਰ
		ਸਿਰਲੇਖ_threshhold = 0;
#ਨਹੀਂਤਾਂ
		fmt += 11; /* "" */
#ਜੇ_ਅੰਤ
	}
	ਜੇ (negative_N) {
		ਜੇ (ਗਿਣਤੀ_2 >= ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ / ਆਕਾਰ(ਅੱਖਰ*))
			bb_error_ਸੁਨੇਹਾ("count is too big: %lu", ਗਿਣਤੀ_2);
	}

	ਕਰੋ {
		fp = fopen_or_warn_stdin(*argv);
		ਜੇ (fp) {
			ਜੇ (fp == stdin) {
				*argv = (ਅੱਖਰ *) bb_ਸੁਨੇਹਾ_standard_ਨਿਵੇਸ਼;
			}
			ਜੇ (ਸਿਰਲੇਖ_threshhold) {
				ਲਿਖੋ(fmt, *argv);
			}
			ਜੇ (negative_N) {
				ਜੇ (ਗਿਣਤੀ_ਬਾਈਟਾਂ) {
					print_except_N_ਆਖਰੀ_ਬਾਈਟਾਂ(fp, ਗਿਣਤੀ_2);
				} ਨਹੀਂਤਾਂ {
					print_except_N_ਆਖਰੀ_ਸਤਰਾਂ(fp, ਗਿਣਤੀ_2);
				}
			} ਨਹੀਂਤਾਂ {
				print_ਪਹਿਲਾ_N(fp, ਗਿਣਤੀ_2, ਗਿਣਤੀ_ਬਾਈਟਾਂ);
			}
			die_if_ferror_stdout();
			ਜੇ (fclose_if_not_stdin(fp)) {
				bb_simple_perror_ਸੁਨੇਹਾ(*argv);
				retval = EXIT_FAILURE;
			}
		} ਨਹੀਂਤਾਂ {
			retval = EXIT_FAILURE;
		}
		fmt = ਸਿਰਲੇਖ_fmt_str;
	} ਜਦੋਂ (*++argv);

	fflush_stdout_and_exit(retval);
}
