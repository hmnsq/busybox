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

#ಅನ್ತರ್ಭಾವ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

#ಯದಿ !ENABLE_FEATURE_FANCY_HEAD
# define print_first_N(fp,ಎಣಿಕೆ,bytes) print_first_N(fp,ಎಣಿಕೆ)
#endif
ಸ್ಥಿರ ಶೂನ್ಯ
print_first_N(FILE *fp, unsigned long ಎಣಿಕೆ, bool ಎಣಿಕೆ_bytes)
{
#ಯದಿ !ENABLE_FEATURE_FANCY_HEAD
	ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ_bytes = 0;
#endif
	ಯಾವತ್ (ಎಣಿಕೆ) {
		ಪೂರ್ಣಾಙ್ಕ c = getc(fp);
		ಯದಿ (c == EOF)
			ಭಙ್ಗ;
		ಯದಿ (ಎಣಿಕೆ_bytes || (c == '\n'))
			--ಎಣಿಕೆ;
		putchar(c);
	}
}

#ಯದಿ ENABLE_FEATURE_FANCY_HEAD
ಸ್ಥಿರ ಶೂನ್ಯ
print_except_N_last_bytes(FILE *fp, unsigned ಎಣಿಕೆ)
{
	unsigned ವರ್ಣ *circle = xmalloc(++ಎಣಿಕೆ);
	unsigned head = 0;
	ಪ್ರತಿ (;;) {
		ಪೂರ್ಣಾಙ್ಕ c;
		c = getc(fp);
		ಯದಿ (c == EOF)
			ಗಚ್ಛ ret;
		circle[head++] = c;
		ಯದಿ (head == ಎಣಿಕೆ)
			ಭಙ್ಗ;
	}
	ಪ್ರತಿ (;;) {
		ಪೂರ್ಣಾಙ್ಕ c;
		ಯದಿ (head == ಎಣಿಕೆ)
			head = 0;
		putchar(circle[head]);
		c = getc(fp);
		ಯದಿ (c == EOF)
			ಗಚ್ಛ ret;
		circle[head] = c;
		head++;
	}
 ret:
	ಮುಕ್ತ(circle);
}

ಸ್ಥಿರ ಶೂನ್ಯ
print_except_N_last_ಸಾಲುಗಳು(FILE *fp, unsigned ಎಣಿಕೆ)
{
	ವರ್ಣ **circle = xzalloc((++ಎಣಿಕೆ) * ಪರಿಮಾಣ(circle[0]));
	unsigned head = 0;
	ಪ್ರತಿ (;;) {
		ವರ್ಣ *c;
		c = xmalloc_fgets(fp);
		ಯದಿ (!c)
			ಗಚ್ಛ ret;
		circle[head++] = c;
		ಯದಿ (head == ಎಣಿಕೆ)
			ಭಙ್ಗ;
	}
	ಪ್ರತಿ (;;) {
		ವರ್ಣ *c;
		ಯದಿ (head == ಎಣಿಕೆ)
			head = 0;
		fputs_stdout(circle[head]);
		c = xmalloc_fgets(fp);
		ಯದಿ (!c)
			ಗಚ್ಛ ret;
		ಮುಕ್ತ(circle[head]);
		circle[head++] = c;
	}
 ret:
	head = 0;
	ಪ್ರತಿ (;;) {
		ಮುಕ್ತ(circle[head++]);
		ಯದಿ (head == ಎಣಿಕೆ)
			ಭಙ್ಗ;
	}
	ಮುಕ್ತ(circle);
}
#ಅನ್ಯಥಾ
/* Must never be called */
ಶೂನ್ಯ print_except_N_last_bytes(FILE *fp, unsigned ಎಣಿಕೆ);
ಶೂನ್ಯ print_except_N_last_ಸಾಲುಗಳು(FILE *fp, unsigned ಎಣಿಕೆ);
#endif

#ಯದಿ !ENABLE_FEATURE_FANCY_HEAD
# define eat_ಅಂಕೆ(negative_N,p) eat_ಅಂಕೆ(p)
#endif
ಸ್ಥಿರ unsigned long
eat_ಅಂಕೆ(bool *negative_N, ಧ್ರುವ ವರ್ಣ *p)
{
#ಯದಿ ENABLE_FEATURE_FANCY_HEAD
	ಯದಿ (*p == '-') {
		*negative_N = 1;
		p++;
	}
#endif
	ಪ್ರತ್ಯಾಗಮ xatoul_sfx(p, bkm_suffixes);
}

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ head_opts[] ALIGN1 =
	"n:"
#ಯದಿ ENABLE_FEATURE_FANCY_HEAD
	"c:qv"
#endif
	;

#define header_fmt_str "\n==> %s <==\n"

ಪೂರ್ಣಾಙ್ಕ head_main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಙ್ಕ head_main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv)
{
	unsigned long ಎಣಿಕೆ = 10;
#ಯದಿ ENABLE_FEATURE_FANCY_HEAD
	ಪೂರ್ಣಾಙ್ಕ header_threshhold = 1;
	bool ಎಣಿಕೆ_bytes = 0;
	bool negative_N = 0;
#ಅನ್ಯಥಾ
# define header_threshhold 1
# define ಎಣಿಕೆ_bytes       0
# define negative_N        0
#endif
	FILE *fp;
	ಧ್ರುವ ವರ್ಣ *fmt;
	ವರ್ಣ *p;
	ಪೂರ್ಣಾಙ್ಕ opt;
	ಪೂರ್ಣಾಙ್ಕ retval = EXIT_SUCCESS;

#ಯದಿ ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_HEAD
	/* Allow legacy syntax of an initial numeric option without -n. */
	ಯದಿ (argv[1] && argv[1][0] == '-'
	 && isdigit(argv[1][1])
	) {
		--argc;
		++argv;
		p = argv[0] + 1;
		ಗಚ್ಛ GET_ಎಣಿಕೆ;
	}
#endif

	/* No size benefit in converting this to getopt32 */
	ಯಾವತ್ ((opt = getopt(argc, argv, head_opts)) > 0) {
		ವಿಕಲ್ಪ (opt) {
#ಯದಿ ENABLE_FEATURE_FANCY_HEAD
		ಪಕ್ಷ 'q':
			header_threshhold = INT_MAX;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'v':
			header_threshhold = -1;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'c':
			ಎಣಿಕೆ_bytes = 1;
			/* fall through */
#endif
		ಪಕ್ಷ 'n':
			p = optarg;
#ಯದಿ ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_HEAD
 GET_ಎಣಿಕೆ:
#endif
			ಎಣಿಕೆ = eat_ಅಂಕೆ(&negative_N, p);
			ಭಙ್ಗ;
		ಸ್ವಭಾವ:
			bb_show_usage();
		}
	}

	argc -= optind;
	argv += optind;
	ಯದಿ (!*argv)
		*--argv = (ವರ್ಣ*)"-";

	fmt = header_fmt_str + 1;
	ಯದಿ (argc <= header_threshhold) {
#ಯದಿ ENABLE_FEATURE_FANCY_HEAD
		header_threshhold = 0;
#ಅನ್ಯಥಾ
		fmt += 11; /* "" */
#endif
	}
	ಯದಿ (negative_N) {
		ಯದಿ (ಎಣಿಕೆ >= INT_MAX / ಪರಿಮಾಣ(ವರ್ಣ*))
			bb_error_ಸಂದೇಶ("count is too big: %lu", ಎಣಿಕೆ);
	}

	ಕುರು {
		fp = fopen_or_warn_stdin(*argv);
		ಯದಿ (fp) {
			ಯದಿ (fp == stdin) {
				*argv = (ವರ್ಣ *) bb_ಸಂದೇಶ_standard_ಒಳಹರಿವು;
			}
			ಯದಿ (header_threshhold) {
				printf(fmt, *argv);
			}
			ಯದಿ (negative_N) {
				ಯದಿ (ಎಣಿಕೆ_bytes) {
					print_except_N_last_bytes(fp, ಎಣಿಕೆ);
				} ಅನ್ಯಥಾ {
					print_except_N_last_ಸಾಲುಗಳು(fp, ಎಣಿಕೆ);
				}
			} ಅನ್ಯಥಾ {
				print_first_N(fp, ಎಣಿಕೆ, ಎಣಿಕೆ_bytes);
			}
			die_if_ferror_stdout();
			ಯದಿ (fclose_if_not_stdin(fp)) {
				bb_simple_perror_ಸಂದೇಶ(*argv);
				retval = EXIT_FAILURE;
			}
		} ಅನ್ಯಥಾ {
			retval = EXIT_FAILURE;
		}
		fmt = header_fmt_str;
	} ಯಾವತ್ (*++argv);

	fflush_stdout_and_exit(retval);
}
