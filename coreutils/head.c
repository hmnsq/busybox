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

#அந்தர்பாவ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

#எனில் !ENABLE_FEATURE_FANCY_HEAD
# define print_first_N(fp,மொத்தம்,bytes) print_first_N(fp,மொத்தம்)
#endif
நிலையான வெறுமை
print_first_N(FILE *fp, unsigned long மொத்தம், bool மொத்தம்_bytes)
{
#எனில் !ENABLE_FEATURE_FANCY_HEAD
	மாறிலி முழு மொத்தம்_bytes = 0;
#endif
	போது (மொத்தம்) {
		முழு c = getc(fp);
		எனில் (c == EOF)
			நிறுத்து;
		எனில் (மொத்தம்_bytes || (c == '\n'))
			--மொத்தம்;
		putchar(c);
	}
}

#எனில் ENABLE_FEATURE_FANCY_HEAD
நிலையான வெறுமை
print_except_N_last_bytes(FILE *fp, unsigned மொத்தம்)
{
	unsigned எழுத்து *circle = xmalloc(++மொத்தம்);
	unsigned head = 0;
	க்கு (;;) {
		முழு c;
		c = getc(fp);
		எனில் (c == EOF)
			செல் ret;
		circle[head++] = c;
		எனில் (head == மொத்தம்)
			நிறுத்து;
	}
	க்கு (;;) {
		முழு c;
		எனில் (head == மொத்தம்)
			head = 0;
		putchar(circle[head]);
		c = getc(fp);
		எனில் (c == EOF)
			செல் ret;
		circle[head] = c;
		head++;
	}
 ret:
	விடுவி(circle);
}

நிலையான வெறுமை
print_except_N_last_வரிகள்(FILE *fp, unsigned மொத்தம்)
{
	எழுத்து **circle = xzalloc((++மொத்தம்) * அளவு(circle[0]));
	unsigned head = 0;
	க்கு (;;) {
		எழுத்து *c;
		c = xmalloc_fgets(fp);
		எனில் (!c)
			செல் ret;
		circle[head++] = c;
		எனில் (head == மொத்தம்)
			நிறுத்து;
	}
	க்கு (;;) {
		எழுத்து *c;
		எனில் (head == மொத்தம்)
			head = 0;
		fputs_stdout(circle[head]);
		c = xmalloc_fgets(fp);
		எனில் (!c)
			செல் ret;
		விடுவி(circle[head]);
		circle[head++] = c;
	}
 ret:
	head = 0;
	க்கு (;;) {
		விடுவி(circle[head++]);
		எனில் (head == மொத்தம்)
			நிறுத்து;
	}
	விடுவி(circle);
}
#இல்லையெனில்
/* Must never be called */
வெறுமை print_except_N_last_bytes(FILE *fp, unsigned மொத்தம்);
வெறுமை print_except_N_last_வரிகள்(FILE *fp, unsigned மொத்தம்);
#endif

#எனில் !ENABLE_FEATURE_FANCY_HEAD
# define eat_இலக்கம்(negative_N,p) eat_இலக்கம்(p)
#endif
நிலையான unsigned long
eat_இலக்கம்(bool *negative_N, மாறிலி எழுத்து *p)
{
#எனில் ENABLE_FEATURE_FANCY_HEAD
	எனில் (*p == '-') {
		*negative_N = 1;
		p++;
	}
#endif
	திரும்பு xatoul_sfx(p, bkm_suffixes);
}

நிலையான மாறிலி எழுத்து head_opts[] ALIGN1 =
	"n:"
#எனில் ENABLE_FEATURE_FANCY_HEAD
	"c:qv"
#endif
	;

#define header_fmt_str "\n==> %s <==\n"

முழு head_main(முழு argc, எழுத்து **argv) MAIN_EXTERNALLY_VISIBLE;
முழு head_main(முழு argc, எழுத்து **argv)
{
	unsigned long மொத்தம் = 10;
#எனில் ENABLE_FEATURE_FANCY_HEAD
	முழு header_threshhold = 1;
	bool மொத்தம்_bytes = 0;
	bool negative_N = 0;
#இல்லையெனில்
# define header_threshhold 1
# define மொத்தம்_bytes       0
# define negative_N        0
#endif
	FILE *fp;
	மாறிலி எழுத்து *fmt;
	எழுத்து *p;
	முழு opt;
	முழு retval = EXIT_SUCCESS;

#எனில் ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_HEAD
	/* Allow legacy syntax of an initial numeric option without -n. */
	எனில் (argv[1] && argv[1][0] == '-'
	 && isdigit(argv[1][1])
	) {
		--argc;
		++argv;
		p = argv[0] + 1;
		செல் GET_மொத்தம்;
	}
#endif

	/* No size benefit in converting this to getopt32 */
	போது ((opt = getopt(argc, argv, head_opts)) > 0) {
		தேர்வு (opt) {
#எனில் ENABLE_FEATURE_FANCY_HEAD
		நிலை 'q':
			header_threshhold = INT_MAX;
			நிறுத்து;
		நிலை 'v':
			header_threshhold = -1;
			நிறுத்து;
		நிலை 'c':
			மொத்தம்_bytes = 1;
			/* fall through */
#endif
		நிலை 'n':
			p = optarg;
#எனில் ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_HEAD
 GET_மொத்தம்:
#endif
			மொத்தம் = eat_இலக்கம்(&negative_N, p);
			நிறுத்து;
		இயல்பு:
			bb_show_usage();
		}
	}

	argc -= optind;
	argv += optind;
	எனில் (!*argv)
		*--argv = (எழுத்து*)"-";

	fmt = header_fmt_str + 1;
	எனில் (argc <= header_threshhold) {
#எனில் ENABLE_FEATURE_FANCY_HEAD
		header_threshhold = 0;
#இல்லையெனில்
		fmt += 11; /* "" */
#endif
	}
	எனில் (negative_N) {
		எனில் (மொத்தம் >= INT_MAX / அளவு(எழுத்து*))
			bb_error_செய்தி("count is too big: %lu", மொத்தம்);
	}

	செய் {
		fp = fopen_or_warn_stdin(*argv);
		எனில் (fp) {
			எனில் (fp == stdin) {
				*argv = (எழுத்து *) bb_செய்தி_standard_உள்ளீடு;
			}
			எனில் (header_threshhold) {
				printf(fmt, *argv);
			}
			எனில் (negative_N) {
				எனில் (மொத்தம்_bytes) {
					print_except_N_last_bytes(fp, மொத்தம்);
				} இல்லையெனில் {
					print_except_N_last_வரிகள்(fp, மொத்தம்);
				}
			} இல்லையெனில் {
				print_first_N(fp, மொத்தம், மொத்தம்_bytes);
			}
			die_if_ferror_stdout();
			எனில் (fclose_if_not_stdin(fp)) {
				bb_simple_perror_செய்தி(*argv);
				retval = EXIT_FAILURE;
			}
		} இல்லையெனில் {
			retval = EXIT_FAILURE;
		}
		fmt = header_fmt_str;
	} போது (*++argv);

	fflush_stdout_and_exit(retval);
}
