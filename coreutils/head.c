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

#include "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

#གལ་ཏེ !ENABLE_FEATURE_FANCY_HEAD
# define print_first_N(fp,count,bytes) print_first_N(fp,count)
#endif
static void
print_first_N(FILE *fp, unsigned long count, bool count_bytes)
{
#གལ་ཏེ !ENABLE_FEATURE_FANCY_HEAD
	const ཧྲིལ count_bytes = 0;
#endif
	སྐབས (count) {
		ཧྲིལ c = getc(fp);
		གལ་ཏེ (c == EOF)
			bha_nga;
		གལ་ཏེ (count_bytes || (c == '\n'))
			--count;
		putchar(c);
	}
}

#གལ་ཏེ ENABLE_FEATURE_FANCY_HEAD
static void
print_except_N_last_bytes(FILE *fp, unsigned count)
{
	unsigned var_na *circle = xmalloc(++count);
	unsigned head = 0;
	ཆེད་དུ (;;) {
		ཧྲིལ c;
		c = getc(fp);
		གལ་ཏེ (c == EOF)
			goto ret;
		circle[head++] = c;
		གལ་ཏེ (head == count)
			bha_nga;
	}
	ཆེད་དུ (;;) {
		ཧྲིལ c;
		གལ་ཏེ (head == count)
			head = 0;
		putchar(circle[head]);
		c = getc(fp);
		གལ་ཏེ (c == EOF)
			goto ret;
		circle[head] = c;
		head++;
	}
 ret:
	free(circle);
}

static void
print_except_N_last_lines(FILE *fp, unsigned count)
{
	var_na **circle = xzalloc((++count) * parimaa_na(circle[0]));
	unsigned head = 0;
	ཆེད་དུ (;;) {
		var_na *c;
		c = xmalloc_fgets(fp);
		གལ་ཏེ (!c)
			goto ret;
		circle[head++] = c;
		གལ་ཏེ (head == count)
			bha_nga;
	}
	ཆེད་དུ (;;) {
		var_na *c;
		གལ་ཏེ (head == count)
			head = 0;
		fputs_stdout(circle[head]);
		c = xmalloc_fgets(fp);
		གལ་ཏེ (!c)
			goto ret;
		free(circle[head]);
		circle[head++] = c;
	}
 ret:
	head = 0;
	ཆེད་དུ (;;) {
		free(circle[head++]);
		གལ་ཏེ (head == count)
			bha_nga;
	}
	free(circle);
}
#དེ་མིན
/* Must never be called */
void print_except_N_last_bytes(FILE *fp, unsigned count);
void print_except_N_last_lines(FILE *fp, unsigned count);
#endif

#གལ་ཏེ !ENABLE_FEATURE_FANCY_HEAD
# define eat_num(negative_N,p) eat_num(p)
#endif
static unsigned long
eat_num(bool *negative_N, const var_na *p)
{
#གལ་ཏེ ENABLE_FEATURE_FANCY_HEAD
	གལ་ཏེ (*p == '-') {
		*negative_N = 1;
		p++;
	}
#endif
	ཕྱིར xatoul_sfx(p, bkm_suffixes);
}

static const var_na head_opts[] ALIGN1 =
	"n:"
#གལ་ཏེ ENABLE_FEATURE_FANCY_HEAD
	"c:qv"
#endif
	;

#define header_fmt_str "\n==> %s <==\n"

ཧྲིལ head_main(ཧྲིལ argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
ཧྲིལ head_main(ཧྲིལ argc, var_na **argv)
{
	unsigned long count = 10;
#གལ་ཏེ ENABLE_FEATURE_FANCY_HEAD
	ཧྲིལ header_threshhold = 1;
	bool count_bytes = 0;
	bool negative_N = 0;
#དེ་མིན
# define header_threshhold 1
# define count_bytes       0
# define negative_N        0
#endif
	FILE *fp;
	const var_na *fmt;
	var_na *p;
	ཧྲིལ opt;
	ཧྲིལ retval = EXIT_SUCCESS;

#གལ་ཏེ ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_HEAD
	/* Allow legacy syntax of an initial numeric option without -n. */
	གལ་ཏེ (argv[1] && argv[1][0] == '-'
	 && isdigit(argv[1][1])
	) {
		--argc;
		++argv;
		p = argv[0] + 1;
		goto GET_COUNT;
	}
#endif

	/* No size benefit in converting this to getopt32 */
	སྐབས ((opt = getopt(argc, argv, head_opts)) > 0) {
		switch (opt) {
#གལ་ཏེ ENABLE_FEATURE_FANCY_HEAD
		pak_sa 'q':
			header_threshhold = INT_MAX;
			bha_nga;
		pak_sa 'v':
			header_threshhold = -1;
			bha_nga;
		pak_sa 'c':
			count_bytes = 1;
			/* fall through */
#endif
		pak_sa 'n':
			p = optarg;
#གལ་ཏེ ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_HEAD
 GET_COUNT:
#endif
			count = eat_num(&negative_N, p);
			bha_nga;
		default:
			bb_show_usage();
		}
	}

	argc -= optind;
	argv += optind;
	གལ་ཏེ (!*argv)
		*--argv = (var_na*)"-";

	fmt = header_fmt_str + 1;
	གལ་ཏེ (argc <= header_threshhold) {
#གལ་ཏེ ENABLE_FEATURE_FANCY_HEAD
		header_threshhold = 0;
#དེ་མིན
		fmt += 11; /* "" */
#endif
	}
	གལ་ཏེ (negative_N) {
		གལ་ཏེ (count >= INT_MAX / parimaa_na(var_na*))
			bb_error_msg("count is too big: %lu", count);
	}

	do {
		fp = fopen_or_warn_stdin(*argv);
		གལ་ཏེ (fp) {
			གལ་ཏེ (fp == stdin) {
				*argv = (var_na *) bb_msg_standard_input;
			}
			གལ་ཏེ (header_threshhold) {
				printf(fmt, *argv);
			}
			གལ་ཏེ (negative_N) {
				གལ་ཏེ (count_bytes) {
					print_except_N_last_bytes(fp, count);
				} དེ་མིན {
					print_except_N_last_lines(fp, count);
				}
			} དེ་མིན {
				print_first_N(fp, count, count_bytes);
			}
			die_if_ferror_stdout();
			གལ་ཏེ (fclose_if_not_stdin(fp)) {
				bb_simple_perror_msg(*argv);
				retval = EXIT_FAILURE;
			}
		} དེ་མིན {
			retval = EXIT_FAILURE;
		}
		fmt = header_fmt_str;
	} སྐབས (*++argv);

	fflush_stdout_and_exit(retval);
}
