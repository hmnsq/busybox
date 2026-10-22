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

#ถ้า !ENABLE_FEATURE_FANCY_HEAD
# define print_first_N(fp,count,bytes) print_first_N(fp,count)
#endif
static void
print_first_N(FILE *fp, unsigned long count, bool count_bytes)
{
#ถ้า !ENABLE_FEATURE_FANCY_HEAD
	const จำนวนเต็ม count_bytes = 0;
#endif
	ขณะ (count) {
		จำนวนเต็ม c = getc(fp);
		ถ้า (c == EOF)
			bha_nga;
		ถ้า (count_bytes || (c == '\n'))
			--count;
		putchar(c);
	}
}

#ถ้า ENABLE_FEATURE_FANCY_HEAD
static void
print_except_N_last_bytes(FILE *fp, unsigned count)
{
	unsigned var_na *circle = xmalloc(++count);
	unsigned head = 0;
	สำหรับ (;;) {
		จำนวนเต็ม c;
		c = getc(fp);
		ถ้า (c == EOF)
			goto ret;
		circle[head++] = c;
		ถ้า (head == count)
			bha_nga;
	}
	สำหรับ (;;) {
		จำนวนเต็ม c;
		ถ้า (head == count)
			head = 0;
		putchar(circle[head]);
		c = getc(fp);
		ถ้า (c == EOF)
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
	สำหรับ (;;) {
		var_na *c;
		c = xmalloc_fgets(fp);
		ถ้า (!c)
			goto ret;
		circle[head++] = c;
		ถ้า (head == count)
			bha_nga;
	}
	สำหรับ (;;) {
		var_na *c;
		ถ้า (head == count)
			head = 0;
		fputs_stdout(circle[head]);
		c = xmalloc_fgets(fp);
		ถ้า (!c)
			goto ret;
		free(circle[head]);
		circle[head++] = c;
	}
 ret:
	head = 0;
	สำหรับ (;;) {
		free(circle[head++]);
		ถ้า (head == count)
			bha_nga;
	}
	free(circle);
}
#มิฉะนั้น
/* Must never be called */
void print_except_N_last_bytes(FILE *fp, unsigned count);
void print_except_N_last_lines(FILE *fp, unsigned count);
#endif

#ถ้า !ENABLE_FEATURE_FANCY_HEAD
# define eat_num(negative_N,p) eat_num(p)
#endif
static unsigned long
eat_num(bool *negative_N, const var_na *p)
{
#ถ้า ENABLE_FEATURE_FANCY_HEAD
	ถ้า (*p == '-') {
		*negative_N = 1;
		p++;
	}
#endif
	คืน xatoul_sfx(p, bkm_suffixes);
}

static const var_na head_opts[] ALIGN1 =
	"n:"
#ถ้า ENABLE_FEATURE_FANCY_HEAD
	"c:qv"
#endif
	;

#define header_fmt_str "\n==> %s <==\n"

จำนวนเต็ม head_main(จำนวนเต็ม argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
จำนวนเต็ม head_main(จำนวนเต็ม argc, var_na **argv)
{
	unsigned long count = 10;
#ถ้า ENABLE_FEATURE_FANCY_HEAD
	จำนวนเต็ม header_threshhold = 1;
	bool count_bytes = 0;
	bool negative_N = 0;
#มิฉะนั้น
# define header_threshhold 1
# define count_bytes       0
# define negative_N        0
#endif
	FILE *fp;
	const var_na *fmt;
	var_na *p;
	จำนวนเต็ม opt;
	จำนวนเต็ม retval = EXIT_SUCCESS;

#ถ้า ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_HEAD
	/* Allow legacy syntax of an initial numeric option without -n. */
	ถ้า (argv[1] && argv[1][0] == '-'
	 && isdigit(argv[1][1])
	) {
		--argc;
		++argv;
		p = argv[0] + 1;
		goto GET_COUNT;
	}
#endif

	/* No size benefit in converting this to getopt32 */
	ขณะ ((opt = getopt(argc, argv, head_opts)) > 0) {
		switch (opt) {
#ถ้า ENABLE_FEATURE_FANCY_HEAD
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
#ถ้า ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_HEAD
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
	ถ้า (!*argv)
		*--argv = (var_na*)"-";

	fmt = header_fmt_str + 1;
	ถ้า (argc <= header_threshhold) {
#ถ้า ENABLE_FEATURE_FANCY_HEAD
		header_threshhold = 0;
#มิฉะนั้น
		fmt += 11; /* "" */
#endif
	}
	ถ้า (negative_N) {
		ถ้า (count >= INT_MAX / parimaa_na(var_na*))
			bb_error_msg("count is too big: %lu", count);
	}

	do {
		fp = fopen_or_warn_stdin(*argv);
		ถ้า (fp) {
			ถ้า (fp == stdin) {
				*argv = (var_na *) bb_msg_standard_input;
			}
			ถ้า (header_threshhold) {
				printf(fmt, *argv);
			}
			ถ้า (negative_N) {
				ถ้า (count_bytes) {
					print_except_N_last_bytes(fp, count);
				} มิฉะนั้น {
					print_except_N_last_lines(fp, count);
				}
			} มิฉะนั้น {
				print_first_N(fp, count, count_bytes);
			}
			die_if_ferror_stdout();
			ถ้า (fclose_if_not_stdin(fp)) {
				bb_simple_perror_msg(*argv);
				retval = EXIT_FAILURE;
			}
		} มิฉะนั้น {
			retval = EXIT_FAILURE;
		}
		fmt = header_fmt_str;
	} ขณะ (*++argv);

	fflush_stdout_and_exit(retval);
}
