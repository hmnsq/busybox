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

#אם !ENABLE_FEATURE_FANCY_HEAD
# define print_first_N(fp,count,bytes) print_first_N(fp,count)
#endif
ساكن فراغ
print_first_N(FILE *fp, unsigned long count, bool count_bytes)
{
#אם !ENABLE_FEATURE_FANCY_HEAD
	ثابت صحيح count_bytes = 0;
#endif
	כלעוד (count) {
		صحيح c = getc(fp);
		אם (c == EOF)
			اكسر;
		אם (count_bytes || (c == '\n'))
			--count;
		putchar(c);
	}
}

#אם ENABLE_FEATURE_FANCY_HEAD
ساكن فراغ
print_except_N_last_bytes(FILE *fp, unsigned count)
{
	unsigned حرف *circle = xmalloc(++count);
	unsigned head = 0;
	עבור (;;) {
		صحيح c;
		c = getc(fp);
		אם (c == EOF)
			اذهب ret;
		circle[head++] = c;
		אם (head == count)
			اكسر;
	}
	עבור (;;) {
		صحيح c;
		אם (head == count)
			head = 0;
		putchar(circle[head]);
		c = getc(fp);
		אם (c == EOF)
			اذهب ret;
		circle[head] = c;
		head++;
	}
 ret:
	حرر(circle);
}

ساكن فراغ
print_except_N_last_lines(FILE *fp, unsigned count)
{
	حرف **circle = xzalloc((++count) * حجم(circle[0]));
	unsigned head = 0;
	עבור (;;) {
		حرف *c;
		c = xmalloc_fgets(fp);
		אם (!c)
			اذهب ret;
		circle[head++] = c;
		אם (head == count)
			اكسر;
	}
	עבור (;;) {
		حرف *c;
		אם (head == count)
			head = 0;
		fputs_stdout(circle[head]);
		c = xmalloc_fgets(fp);
		אם (!c)
			اذهب ret;
		حرر(circle[head]);
		circle[head++] = c;
	}
 ret:
	head = 0;
	עבור (;;) {
		حرر(circle[head++]);
		אם (head == count)
			اكسر;
	}
	حرر(circle);
}
#אחרת
/* Must never be called */
فراغ print_except_N_last_bytes(FILE *fp, unsigned count);
فراغ print_except_N_last_lines(FILE *fp, unsigned count);
#endif

#אם !ENABLE_FEATURE_FANCY_HEAD
# define eat_num(negative_N,p) eat_num(p)
#endif
ساكن unsigned long
eat_num(bool *negative_N, ثابت حرف *p)
{
#אם ENABLE_FEATURE_FANCY_HEAD
	אם (*p == '-') {
		*negative_N = 1;
		p++;
	}
#endif
	החזר xatoul_sfx(p, bkm_suffixes);
}

ساكن ثابت حرف head_opts[] ALIGN1 =
	"n:"
#אם ENABLE_FEATURE_FANCY_HEAD
	"c:qv"
#endif
	;

#define header_fmt_str "\n==> %s <==\n"

صحيح head_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح head_main(صحيح argc, حرف **argv)
{
	unsigned long count = 10;
#אם ENABLE_FEATURE_FANCY_HEAD
	صحيح header_threshhold = 1;
	bool count_bytes = 0;
	bool negative_N = 0;
#אחרת
# define header_threshhold 1
# define count_bytes       0
# define negative_N        0
#endif
	FILE *fp;
	ثابت حرف *fmt;
	حرف *p;
	صحيح opt;
	صحيح retval = EXIT_SUCCESS;

#אם ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_HEAD
	/* Allow legacy syntax of an initial numeric option without -n. */
	אם (argv[1] && argv[1][0] == '-'
	 && isdigit(argv[1][1])
	) {
		--argc;
		++argv;
		p = argv[0] + 1;
		اذهب GET_COUNT;
	}
#endif

	/* No size benefit in converting this to getopt32 */
	כלעוד ((opt = getopt(argc, argv, head_opts)) > 0) {
		بدل (opt) {
#אם ENABLE_FEATURE_FANCY_HEAD
		حالة 'q':
			header_threshhold = INT_MAX;
			اكسر;
		حالة 'v':
			header_threshhold = -1;
			اكسر;
		حالة 'c':
			count_bytes = 1;
			/* fall through */
#endif
		حالة 'n':
			p = optarg;
#אם ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_HEAD
 GET_COUNT:
#endif
			count = eat_num(&negative_N, p);
			اكسر;
		افتراضي:
			bb_show_usage();
		}
	}

	argc -= optind;
	argv += optind;
	אם (!*argv)
		*--argv = (حرف*)"-";

	fmt = header_fmt_str + 1;
	אם (argc <= header_threshhold) {
#אם ENABLE_FEATURE_FANCY_HEAD
		header_threshhold = 0;
#אחרת
		fmt += 11; /* "" */
#endif
	}
	אם (negative_N) {
		אם (count >= INT_MAX / حجم(حرف*))
			bb_error_msg("count is too big: %lu", count);
	}

	עשה {
		fp = fopen_or_warn_stdin(*argv);
		אם (fp) {
			אם (fp == stdin) {
				*argv = (حرف *) bb_msg_standard_input;
			}
			אם (header_threshhold) {
				printf(fmt, *argv);
			}
			אם (negative_N) {
				אם (count_bytes) {
					print_except_N_last_bytes(fp, count);
				} אחרת {
					print_except_N_last_lines(fp, count);
				}
			} אחרת {
				print_first_N(fp, count, count_bytes);
			}
			die_if_ferror_stdout();
			אם (fclose_if_not_stdin(fp)) {
				bb_simple_perror_msg(*argv);
				retval = EXIT_FAILURE;
			}
		} אחרת {
			retval = EXIT_FAILURE;
		}
		fmt = header_fmt_str;
	} כלעוד (*++argv);

	fflush_stdout_and_exit(retval);
}
