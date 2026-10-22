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

#અન્તર્ભાવ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

#જો !ENABLE_FEATURE_FANCY_HEAD
# define print_first_N(fp,ગણતરી,bytes) print_first_N(fp,ગણતરી)
#endif
સ્થિર શૂન્ય
print_first_N(FILE *fp, unsigned long ગણતરી, bool ગણતરી_bytes)
{
#જો !ENABLE_FEATURE_FANCY_HEAD
	અચલ પૂર્ણાંક ગણતરી_bytes = 0;
#endif
	જ્યારે (ગણતરી) {
		પૂર્ણાંક c = getc(fp);
		જો (c == EOF)
			તોડો;
		જો (ગણતરી_bytes || (c == '\n'))
			--ગણતરી;
		putchar(c);
	}
}

#જો ENABLE_FEATURE_FANCY_HEAD
સ્થિર શૂન્ય
print_except_N_last_bytes(FILE *fp, unsigned ગણતરી)
{
	unsigned અક્ષર *circle = xmalloc(++ગણતરી);
	unsigned head = 0;
	માટે (;;) {
		પૂર્ણાંક c;
		c = getc(fp);
		જો (c == EOF)
			જાઓ ret;
		circle[head++] = c;
		જો (head == ગણતરી)
			તોડો;
	}
	માટે (;;) {
		પૂર્ણાંક c;
		જો (head == ગણતરી)
			head = 0;
		putchar(circle[head]);
		c = getc(fp);
		જો (c == EOF)
			જાઓ ret;
		circle[head] = c;
		head++;
	}
 ret:
	મુક્ત(circle);
}

સ્થિર શૂન્ય
print_except_N_last_લીટીઓ(FILE *fp, unsigned ગણતરી)
{
	અક્ષર **circle = xzalloc((++ગણતરી) * કદ(circle[0]));
	unsigned head = 0;
	માટે (;;) {
		અક્ષર *c;
		c = xmalloc_fgets(fp);
		જો (!c)
			જાઓ ret;
		circle[head++] = c;
		જો (head == ગણતરી)
			તોડો;
	}
	માટે (;;) {
		અક્ષર *c;
		જો (head == ગણતરી)
			head = 0;
		fputs_stdout(circle[head]);
		c = xmalloc_fgets(fp);
		જો (!c)
			જાઓ ret;
		મુક્ત(circle[head]);
		circle[head++] = c;
	}
 ret:
	head = 0;
	માટે (;;) {
		મુક્ત(circle[head++]);
		જો (head == ગણતરી)
			તોડો;
	}
	મુક્ત(circle);
}
#નહીંતર
/* Must never be called */
શૂન્ય print_except_N_last_bytes(FILE *fp, unsigned ગણતરી);
શૂન્ય print_except_N_last_લીટીઓ(FILE *fp, unsigned ગણતરી);
#endif

#જો !ENABLE_FEATURE_FANCY_HEAD
# define eat_અંક(negative_N,p) eat_અંક(p)
#endif
સ્થિર unsigned long
eat_અંક(bool *negative_N, અચલ અક્ષર *p)
{
#જો ENABLE_FEATURE_FANCY_HEAD
	જો (*p == '-') {
		*negative_N = 1;
		p++;
	}
#endif
	પાછા xatoul_sfx(p, bkm_suffixes);
}

સ્થિર અચલ અક્ષર head_opts[] ALIGN1 =
	"n:"
#જો ENABLE_FEATURE_FANCY_HEAD
	"c:qv"
#endif
	;

#define header_fmt_str "\n==> %s <==\n"

પૂર્ણાંક head_main(પૂર્ણાંક argc, અક્ષર **argv) MAIN_EXTERNALLY_VISIBLE;
પૂર્ણાંક head_main(પૂર્ણાંક argc, અક્ષર **argv)
{
	unsigned long ગણતરી = 10;
#જો ENABLE_FEATURE_FANCY_HEAD
	પૂર્ણાંક header_threshhold = 1;
	bool ગણતરી_bytes = 0;
	bool negative_N = 0;
#નહીંતર
# define header_threshhold 1
# define ગણતરી_bytes       0
# define negative_N        0
#endif
	FILE *fp;
	અચલ અક્ષર *fmt;
	અક્ષર *p;
	પૂર્ણાંક opt;
	પૂર્ણાંક retval = EXIT_SUCCESS;

#જો ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_HEAD
	/* Allow legacy syntax of an initial numeric option without -n. */
	જો (argv[1] && argv[1][0] == '-'
	 && isdigit(argv[1][1])
	) {
		--argc;
		++argv;
		p = argv[0] + 1;
		જાઓ GET_ગણતરી;
	}
#endif

	/* No size benefit in converting this to getopt32 */
	જ્યારે ((opt = getopt(argc, argv, head_opts)) > 0) {
		પસંદગી (opt) {
#જો ENABLE_FEATURE_FANCY_HEAD
		સ્થિતિ 'q':
			header_threshhold = INT_MAX;
			તોડો;
		સ્થિતિ 'v':
			header_threshhold = -1;
			તોડો;
		સ્થિતિ 'c':
			ગણતરી_bytes = 1;
			/* fall through */
#endif
		સ્થિતિ 'n':
			p = optarg;
#જો ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_HEAD
 GET_ગણતરી:
#endif
			ગણતરી = eat_અંક(&negative_N, p);
			તોડો;
		મૂળભૂત:
			bb_show_usage();
		}
	}

	argc -= optind;
	argv += optind;
	જો (!*argv)
		*--argv = (અક્ષર*)"-";

	fmt = header_fmt_str + 1;
	જો (argc <= header_threshhold) {
#જો ENABLE_FEATURE_FANCY_HEAD
		header_threshhold = 0;
#નહીંતર
		fmt += 11; /* "" */
#endif
	}
	જો (negative_N) {
		જો (ગણતરી >= INT_MAX / કદ(અક્ષર*))
			bb_error_સંદેશ("count is too big: %lu", ગણતરી);
	}

	કરો {
		fp = fopen_or_warn_stdin(*argv);
		જો (fp) {
			જો (fp == stdin) {
				*argv = (અક્ષર *) bb_સંદેશ_standard_આગત;
			}
			જો (header_threshhold) {
				printf(fmt, *argv);
			}
			જો (negative_N) {
				જો (ગણતરી_bytes) {
					print_except_N_last_bytes(fp, ગણતરી);
				} નહીંતર {
					print_except_N_last_લીટીઓ(fp, ગણતરી);
				}
			} નહીંતર {
				print_first_N(fp, ગણતરી, ગણતરી_bytes);
			}
			die_if_ferror_stdout();
			જો (fclose_if_not_stdin(fp)) {
				bb_simple_perror_સંદેશ(*argv);
				retval = EXIT_FAILURE;
			}
		} નહીંતર {
			retval = EXIT_FAILURE;
		}
		fmt = header_fmt_str;
	} જ્યારે (*++argv);

	fflush_stdout_and_exit(retval);
}
