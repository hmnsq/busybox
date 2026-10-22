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

#ଅନ୍ତର୍ଭାଵ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

#ଯଦି !ENABLE_FEATURE_FANCY_HEAD
# define print_first_N(fp,ଗଣନା_2,bytes) print_first_N(fp,ଗଣନା_2)
#endif
ସ୍ଥିର ଶୂନ୍ୟ
print_first_N(FILE *fp, unsigned long ଗଣନା_2, bool ଗଣନା_bytes)
{
#ଯଦି !ENABLE_FEATURE_FANCY_HEAD
	ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_bytes = 0;
#endif
	ଯେତେବେଳେ (ଗଣନା_2) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା c = getc(fp);
		ଯଦି (c == EOF)
			ଭାଙ୍ଗ;
		ଯଦି (ଗଣନା_bytes || (c == '\n'))
			--ଗଣନା_2;
		putchar(c);
	}
}

#ଯଦି ENABLE_FEATURE_FANCY_HEAD
ସ୍ଥିର ଶୂନ୍ୟ
print_except_N_last_bytes(FILE *fp, unsigned ଗଣନା_2)
{
	unsigned ଅକ୍ଷର *circle = xmalloc(++ଗଣନା_2);
	unsigned head = 0;
	ପାଇଁ (;;) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା c;
		c = getc(fp);
		ଯଦି (c == EOF)
			ଯାଅ ret;
		circle[head++] = c;
		ଯଦି (head == ଗଣନା_2)
			ଭାଙ୍ଗ;
	}
	ପାଇଁ (;;) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା c;
		ଯଦି (head == ଗଣନା_2)
			head = 0;
		putchar(circle[head]);
		c = getc(fp);
		ଯଦି (c == EOF)
			ଯାଅ ret;
		circle[head] = c;
		head++;
	}
 ret:
	ମୁକ୍ତ(circle);
}

ସ୍ଥିର ଶୂନ୍ୟ
print_except_N_last_ଧାଡ଼ିଗୁଡ଼ିକ(FILE *fp, unsigned ଗଣନା_2)
{
	ଅକ୍ଷର **circle = xzalloc((++ଗଣନା_2) * ଆକାର(circle[0]));
	unsigned head = 0;
	ପାଇଁ (;;) {
		ଅକ୍ଷର *c;
		c = xmalloc_fgets(fp);
		ଯଦି (!c)
			ଯାଅ ret;
		circle[head++] = c;
		ଯଦି (head == ଗଣନା_2)
			ଭାଙ୍ଗ;
	}
	ପାଇଁ (;;) {
		ଅକ୍ଷର *c;
		ଯଦି (head == ଗଣନା_2)
			head = 0;
		fputs_stdout(circle[head]);
		c = xmalloc_fgets(fp);
		ଯଦି (!c)
			ଯାଅ ret;
		ମୁକ୍ତ(circle[head]);
		circle[head++] = c;
	}
 ret:
	head = 0;
	ପାଇଁ (;;) {
		ମୁକ୍ତ(circle[head++]);
		ଯଦି (head == ଗଣନା_2)
			ଭାଙ୍ଗ;
	}
	ମୁକ୍ତ(circle);
}
#ଅନ୍ୟଥା
/* Must never be called */
ଶୂନ୍ୟ print_except_N_last_bytes(FILE *fp, unsigned ଗଣନା_2);
ଶୂନ୍ୟ print_except_N_last_ଧାଡ଼ିଗୁଡ଼ିକ(FILE *fp, unsigned ଗଣନା_2);
#endif

#ଯଦି !ENABLE_FEATURE_FANCY_HEAD
# define eat_ଅଙ୍କ(negative_N,p) eat_ଅଙ୍କ(p)
#endif
ସ୍ଥିର unsigned long
eat_ଅଙ୍କ(bool *negative_N, ଅଚଳ ଅକ୍ଷର *p)
{
#ଯଦି ENABLE_FEATURE_FANCY_HEAD
	ଯଦି (*p == '-') {
		*negative_N = 1;
		p++;
	}
#endif
	ଫେରନ୍ତୁ xatoul_sfx(p, bkm_suffixes);
}

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର head_opts[] ALIGN1 =
	"n:"
#ଯଦି ENABLE_FEATURE_FANCY_HEAD
	"c:qv"
#endif
	;

#define header_fmt_str "\n==> %s <==\n"

ପୂର୍ଣ୍ଣସଂଖ୍ୟା head_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv) MAIN_EXTERNALLY_VISIBLE;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା head_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv)
{
	unsigned long ଗଣନା_2 = 10;
#ଯଦି ENABLE_FEATURE_FANCY_HEAD
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା header_threshhold = 1;
	bool ଗଣନା_bytes = 0;
	bool negative_N = 0;
#ଅନ୍ୟଥା
# define header_threshhold 1
# define ଗଣନା_bytes       0
# define negative_N        0
#endif
	FILE *fp;
	ଅଚଳ ଅକ୍ଷର *fmt;
	ଅକ୍ଷର *p;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval = EXIT_SUCCESS;

#ଯଦି ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_HEAD
	/* Allow legacy syntax of an initial numeric option without -n. */
	ଯଦି (argv[1] && argv[1][0] == '-'
	 && isdigit(argv[1][1])
	) {
		--argc;
		++argv;
		p = argv[0] + 1;
		ଯାଅ GET_ଗଣନା;
	}
#endif

	/* No size benefit in converting this to getopt32 */
	ଯେତେବେଳେ ((opt = getopt(argc, argv, head_opts)) > 0) {
		ବାଛ (opt) {
#ଯଦି ENABLE_FEATURE_FANCY_HEAD
		ସ୍ଥିତି 'q':
			header_threshhold = INT_MAX;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'v':
			header_threshhold = -1;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'c':
			ଗଣନା_bytes = 1;
			/* fall through */
#endif
		ସ୍ଥିତି 'n':
			p = optarg;
#ଯଦି ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_HEAD
 GET_ଗଣନା:
#endif
			ଗଣନା_2 = eat_ଅଙ୍କ(&negative_N, p);
			ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			bb_show_usage();
		}
	}

	argc -= optind;
	argv += optind;
	ଯଦି (!*argv)
		*--argv = (ଅକ୍ଷର*)"-";

	fmt = header_fmt_str + 1;
	ଯଦି (argc <= header_threshhold) {
#ଯଦି ENABLE_FEATURE_FANCY_HEAD
		header_threshhold = 0;
#ଅନ୍ୟଥା
		fmt += 11; /* "" */
#endif
	}
	ଯଦି (negative_N) {
		ଯଦି (ଗଣନା_2 >= INT_MAX / ଆକାର(ଅକ୍ଷର*))
			bb_error_ସନ୍ଦେଶ("count is too big: %lu", ଗଣନା_2);
	}

	କର {
		fp = fopen_or_warn_stdin(*argv);
		ଯଦି (fp) {
			ଯଦି (fp == stdin) {
				*argv = (ଅକ୍ଷର *) bb_ସନ୍ଦେଶ_standard_ନିବେଶ;
			}
			ଯଦି (header_threshhold) {
				printf(fmt, *argv);
			}
			ଯଦି (negative_N) {
				ଯଦି (ଗଣନା_bytes) {
					print_except_N_last_bytes(fp, ଗଣନା_2);
				} ଅନ୍ୟଥା {
					print_except_N_last_ଧାଡ଼ିଗୁଡ଼ିକ(fp, ଗଣନା_2);
				}
			} ଅନ୍ୟଥା {
				print_first_N(fp, ଗଣନା_2, ଗଣନା_bytes);
			}
			die_if_ferror_stdout();
			ଯଦି (fclose_if_not_stdin(fp)) {
				bb_simple_perror_ସନ୍ଦେଶ(*argv);
				retval = EXIT_FAILURE;
			}
		} ଅନ୍ୟଥା {
			retval = EXIT_FAILURE;
		}
		fmt = header_fmt_str;
	} ଯେତେବେଳେ (*++argv);

	fflush_stdout_and_exit(retval);
}
