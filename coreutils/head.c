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

#ಆದರೆ !ENABLE_FEATURE_FANCY_ತಲೆ
# define print_ಮೊದಲ_N(fp,ಎಣಿಕೆ_2,ಬೈಟುಗಳು) print_ಮೊದಲ_N(fp,ಎಣಿಕೆ_2)
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ ಶೂನ್ಯ
print_ಮೊದಲ_N(FILE *fp, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಎಣಿಕೆ_2, bool ಎಣಿಕೆ_ಬೈಟುಗಳು)
{
#ಆದರೆ !ENABLE_FEATURE_FANCY_ತಲೆ
	ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_ಬೈಟುಗಳು = 0;
#ಆದರೆ_ಅಂತ್ಯ
	ಆಗ (ಎಣಿಕೆ_2) {
		ಪೂರ್ಣಾಂಕ c = getc(fp);
		ಆದರೆ (c == EOF)
			ನಿಲ್ಲಿಸು;
		ಆದರೆ (ಎಣಿಕೆ_ಬೈಟುಗಳು || (c == '\n'))
			--ಎಣಿಕೆ_2;
		ಅಕ್ಷರ_ಬರೆ(c);
	}
}

#ಆದರೆ ENABLE_FEATURE_FANCY_ತಲೆ
ಸ್ಥಿರ ಶೂನ್ಯ
print_except_N_ಕೊನೆಯ_ಬೈಟುಗಳು(FILE *fp, ಚಿಹ್ನೆರಹಿತ ಎಣಿಕೆ_2)
{
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *circle = xmalloc(++ಎಣಿಕೆ_2);
	ಚಿಹ್ನೆರಹಿತ ತಲೆ = 0;
	ಗಾಗಿ (;;) {
		ಪೂರ್ಣಾಂಕ c;
		c = getc(fp);
		ಆದರೆ (c == EOF)
			ಹೋಗು ret;
		circle[ತಲೆ++] = c;
		ಆದರೆ (ತಲೆ == ಎಣಿಕೆ_2)
			ನಿಲ್ಲಿಸು;
	}
	ಗಾಗಿ (;;) {
		ಪೂರ್ಣಾಂಕ c;
		ಆದರೆ (ತಲೆ == ಎಣಿಕೆ_2)
			ತಲೆ = 0;
		ಅಕ್ಷರ_ಬರೆ(circle[ತಲೆ]);
		c = getc(fp);
		ಆದರೆ (c == EOF)
			ಹೋಗು ret;
		circle[ತಲೆ] = c;
		ತಲೆ++;
	}
 ret:
	ಬಿಡುಗಡೆ(circle);
}

ಸ್ಥಿರ ಶೂನ್ಯ
print_except_N_ಕೊನೆಯ_ಸಾಲುಗಳು(FILE *fp, ಚಿಹ್ನೆರಹಿತ ಎಣಿಕೆ_2)
{
	ಅಕ್ಷರ **circle = xzalloc((++ಎಣಿಕೆ_2) * ಗಾತ್ರ(circle[0]));
	ಚಿಹ್ನೆರಹಿತ ತಲೆ = 0;
	ಗಾಗಿ (;;) {
		ಅಕ್ಷರ *c;
		c = xmalloc_fgets(fp);
		ಆದರೆ (!c)
			ಹೋಗು ret;
		circle[ತಲೆ++] = c;
		ಆದರೆ (ತಲೆ == ಎಣಿಕೆ_2)
			ನಿಲ್ಲಿಸು;
	}
	ಗಾಗಿ (;;) {
		ಅಕ್ಷರ *c;
		ಆದರೆ (ತಲೆ == ಎಣಿಕೆ_2)
			ತಲೆ = 0;
		fputs_stdout(circle[ತಲೆ]);
		c = xmalloc_fgets(fp);
		ಆದರೆ (!c)
			ಹೋಗು ret;
		ಬಿಡುಗಡೆ(circle[ತಲೆ]);
		circle[ತಲೆ++] = c;
	}
 ret:
	ತಲೆ = 0;
	ಗಾಗಿ (;;) {
		ಬಿಡುಗಡೆ(circle[ತಲೆ++]);
		ಆದರೆ (ತಲೆ == ಎಣಿಕೆ_2)
			ನಿಲ್ಲಿಸು;
	}
	ಬಿಡುಗಡೆ(circle);
}
#ಇಲ್ಲದಿದ್ದರೆ
/* Must never be called */
ಶೂನ್ಯ print_except_N_ಕೊನೆಯ_ಬೈಟುಗಳು(FILE *fp, ಚಿಹ್ನೆರಹಿತ ಎಣಿಕೆ_2);
ಶೂನ್ಯ print_except_N_ಕೊನೆಯ_ಸಾಲುಗಳು(FILE *fp, ಚಿಹ್ನೆರಹಿತ ಎಣಿಕೆ_2);
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !ENABLE_FEATURE_FANCY_ತಲೆ
# define eat_ಅಂಕೆ(negative_N,p) eat_ಅಂಕೆ(p)
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ
eat_ಅಂಕೆ(bool *negative_N, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p)
{
#ಆದರೆ ENABLE_FEATURE_FANCY_ತಲೆ
	ಆದರೆ (*p == '-') {
		*negative_N = 1;
		p++;
	}
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ xatoul_sfx(p, bkm_suffixes);
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ ತಲೆ_opts[] ALIGN1 =
	"n:"
#ಆದರೆ ENABLE_FEATURE_FANCY_ತಲೆ
	"c:qv"
#ಆದರೆ_ಅಂತ್ಯ
	;

#ವ್ಯಾಖ್ಯಾನಿಸು ಶೀರ್ಷಿಕೆ_fmt_str "\n==> %s <==\n"

ಪೂರ್ಣಾಂಕ ತಲೆ_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ ತಲೆ_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಎಣಿಕೆ_2 = 10;
#ಆದರೆ ENABLE_FEATURE_FANCY_ತಲೆ
	ಪೂರ್ಣಾಂಕ ಶೀರ್ಷಿಕೆ_threshhold = 1;
	bool ಎಣಿಕೆ_ಬೈಟುಗಳು = 0;
	bool negative_N = 0;
#ಇಲ್ಲದಿದ್ದರೆ
# define ಶೀರ್ಷಿಕೆ_threshhold 1
# define ಎಣಿಕೆ_ಬೈಟುಗಳು       0
# define negative_N        0
#ಆದರೆ_ಅಂತ್ಯ
	FILE *fp;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt;
	ಅಕ್ಷರ *p;
	ಪೂರ್ಣಾಂಕ opt;
	ಪೂರ್ಣಾಂಕ retval = EXIT_SUCCESS;

#ಆದರೆ ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_ತಲೆ
	/* Allow legacy syntax of an initial numeric option without -n. */
	ಆದರೆ (argv[1] && argv[1][0] == '-'
	 && ಅಂಕಿಯೇ(argv[1][1])
	) {
		--argc;
		++argv;
		p = argv[0] + 1;
		ಹೋಗು GET_ಎಣಿಕೆ;
	}
#ಆದರೆ_ಅಂತ್ಯ

	/* No size benefit in converting this to getopt32 */
	ಆಗ ((opt = getopt(argc, argv, ತಲೆ_opts)) > 0) {
		ಆಯ್ಕೆ (opt) {
#ಆದರೆ ENABLE_FEATURE_FANCY_ತಲೆ
		ಸ್ಥಿತಿ 'q':
			ಶೀರ್ಷಿಕೆ_threshhold = ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'v':
			ಶೀರ್ಷಿಕೆ_threshhold = -1;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'c':
			ಎಣಿಕೆ_ಬೈಟುಗಳು = 1;
			/* fall through */
#ಆದರೆ_ಅಂತ್ಯ
		ಸ್ಥಿತಿ 'n':
			p = optarg;
#ಆದರೆ ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_ತಲೆ
 GET_ಎಣಿಕೆ:
#ಆದರೆ_ಅಂತ್ಯ
			ಎಣಿಕೆ_2 = eat_ಅಂಕೆ(&negative_N, p);
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			bb_show_usage();
		}
	}

	argc -= optind;
	argv += optind;
	ಆದರೆ (!*argv)
		*--argv = (ಅಕ್ಷರ*)"-";

	fmt = ಶೀರ್ಷಿಕೆ_fmt_str + 1;
	ಆದರೆ (argc <= ಶೀರ್ಷಿಕೆ_threshhold) {
#ಆದರೆ ENABLE_FEATURE_FANCY_ತಲೆ
		ಶೀರ್ಷಿಕೆ_threshhold = 0;
#ಇಲ್ಲದಿದ್ದರೆ
		fmt += 11; /* "" */
#ಆದರೆ_ಅಂತ್ಯ
	}
	ಆದರೆ (negative_N) {
		ಆದರೆ (ಎಣಿಕೆ_2 >= ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ / ಗಾತ್ರ(ಅಕ್ಷರ*))
			bb_error_ಸಂದೇಶ("count is too big: %lu", ಎಣಿಕೆ_2);
	}

	ಮಾಡು {
		fp = fopen_or_warn_stdin(*argv);
		ಆದರೆ (fp) {
			ಆದರೆ (fp == stdin) {
				*argv = (ಅಕ್ಷರ *) bb_ಸಂದೇಶ_standard_ಒಳಹರಿವು;
			}
			ಆದರೆ (ಶೀರ್ಷಿಕೆ_threshhold) {
				ಬರೆ(fmt, *argv);
			}
			ಆದರೆ (negative_N) {
				ಆದರೆ (ಎಣಿಕೆ_ಬೈಟುಗಳು) {
					print_except_N_ಕೊನೆಯ_ಬೈಟುಗಳು(fp, ಎಣಿಕೆ_2);
				} ಇಲ್ಲದಿದ್ದರೆ {
					print_except_N_ಕೊನೆಯ_ಸಾಲುಗಳು(fp, ಎಣಿಕೆ_2);
				}
			} ಇಲ್ಲದಿದ್ದರೆ {
				print_ಮೊದಲ_N(fp, ಎಣಿಕೆ_2, ಎಣಿಕೆ_ಬೈಟುಗಳು);
			}
			die_if_ferror_stdout();
			ಆದರೆ (fclose_if_not_stdin(fp)) {
				bb_simple_perror_ಸಂದೇಶ(*argv);
				retval = EXIT_FAILURE;
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			retval = EXIT_FAILURE;
		}
		fmt = ಶೀರ್ಷಿಕೆ_fmt_str;
	} ಆಗ (*++argv);

	fflush_stdout_and_exit(retval);
}
