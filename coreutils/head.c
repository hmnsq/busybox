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

#अन्तर्भाव "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

#यदि !ENABLE_FEATURE_FANCY_HEAD
# define print_first_N(fp,मोजणी,bytes) print_first_N(fp,मोजणी)
#endif
स्थिर शून्य
print_first_N(FILE *fp, unsigned long मोजणी, bool मोजणी_bytes)
{
#यदि !ENABLE_FEATURE_FANCY_HEAD
	ध्रुव पूर्णाङ्क मोजणी_bytes = 0;
#endif
	यावत् (मोजणी) {
		पूर्णाङ्क c = getc(fp);
		यदि (c == EOF)
			भङ्ग;
		यदि (मोजणी_bytes || (c == '\n'))
			--मोजणी;
		putchar(c);
	}
}

#यदि ENABLE_FEATURE_FANCY_HEAD
स्थिर शून्य
print_except_N_last_bytes(FILE *fp, unsigned मोजणी)
{
	unsigned वर्ण *circle = xmalloc(++मोजणी);
	unsigned head = 0;
	प्रति (;;) {
		पूर्णाङ्क c;
		c = getc(fp);
		यदि (c == EOF)
			गच्छ ret;
		circle[head++] = c;
		यदि (head == मोजणी)
			भङ्ग;
	}
	प्रति (;;) {
		पूर्णाङ्क c;
		यदि (head == मोजणी)
			head = 0;
		putchar(circle[head]);
		c = getc(fp);
		यदि (c == EOF)
			गच्छ ret;
		circle[head] = c;
		head++;
	}
 ret:
	मुक्त(circle);
}

स्थिर शून्य
print_except_N_last_ओळी(FILE *fp, unsigned मोजणी)
{
	वर्ण **circle = xzalloc((++मोजणी) * परिमाण(circle[0]));
	unsigned head = 0;
	प्रति (;;) {
		वर्ण *c;
		c = xmalloc_fgets(fp);
		यदि (!c)
			गच्छ ret;
		circle[head++] = c;
		यदि (head == मोजणी)
			भङ्ग;
	}
	प्रति (;;) {
		वर्ण *c;
		यदि (head == मोजणी)
			head = 0;
		fputs_stdout(circle[head]);
		c = xmalloc_fgets(fp);
		यदि (!c)
			गच्छ ret;
		मुक्त(circle[head]);
		circle[head++] = c;
	}
 ret:
	head = 0;
	प्रति (;;) {
		मुक्त(circle[head++]);
		यदि (head == मोजणी)
			भङ्ग;
	}
	मुक्त(circle);
}
#अन्यथा
/* Must never be called */
शून्य print_except_N_last_bytes(FILE *fp, unsigned मोजणी);
शून्य print_except_N_last_ओळी(FILE *fp, unsigned मोजणी);
#endif

#यदि !ENABLE_FEATURE_FANCY_HEAD
# define eat_अंक(negative_N,p) eat_अंक(p)
#endif
स्थिर unsigned long
eat_अंक(bool *negative_N, ध्रुव वर्ण *p)
{
#यदि ENABLE_FEATURE_FANCY_HEAD
	यदि (*p == '-') {
		*negative_N = 1;
		p++;
	}
#endif
	प्रत्यागम xatoul_sfx(p, bkm_suffixes);
}

स्थिर ध्रुव वर्ण head_opts[] ALIGN1 =
	"n:"
#यदि ENABLE_FEATURE_FANCY_HEAD
	"c:qv"
#endif
	;

#define header_fmt_str "\n==> %s <==\n"

पूर्णाङ्क head_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क head_main(पूर्णाङ्क argc, वर्ण **argv)
{
	unsigned long मोजणी = 10;
#यदि ENABLE_FEATURE_FANCY_HEAD
	पूर्णाङ्क header_threshhold = 1;
	bool मोजणी_bytes = 0;
	bool negative_N = 0;
#अन्यथा
# define header_threshhold 1
# define मोजणी_bytes       0
# define negative_N        0
#endif
	FILE *fp;
	ध्रुव वर्ण *fmt;
	वर्ण *p;
	पूर्णाङ्क opt;
	पूर्णाङ्क retval = EXIT_SUCCESS;

#यदि ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_HEAD
	/* Allow legacy syntax of an initial numeric option without -n. */
	यदि (argv[1] && argv[1][0] == '-'
	 && isdigit(argv[1][1])
	) {
		--argc;
		++argv;
		p = argv[0] + 1;
		गच्छ GET_मोजणी;
	}
#endif

	/* No size benefit in converting this to getopt32 */
	यावत् ((opt = getopt(argc, argv, head_opts)) > 0) {
		विकल्प (opt) {
#यदि ENABLE_FEATURE_FANCY_HEAD
		पक्ष 'q':
			header_threshhold = INT_MAX;
			भङ्ग;
		पक्ष 'v':
			header_threshhold = -1;
			भङ्ग;
		पक्ष 'c':
			मोजणी_bytes = 1;
			/* fall through */
#endif
		पक्ष 'n':
			p = optarg;
#यदि ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_HEAD
 GET_मोजणी:
#endif
			मोजणी = eat_अंक(&negative_N, p);
			भङ्ग;
		स्वभाव:
			bb_show_usage();
		}
	}

	argc -= optind;
	argv += optind;
	यदि (!*argv)
		*--argv = (वर्ण*)"-";

	fmt = header_fmt_str + 1;
	यदि (argc <= header_threshhold) {
#यदि ENABLE_FEATURE_FANCY_HEAD
		header_threshhold = 0;
#अन्यथा
		fmt += 11; /* "" */
#endif
	}
	यदि (negative_N) {
		यदि (मोजणी >= INT_MAX / परिमाण(वर्ण*))
			bb_error_संदेश("count is too big: %lu", मोजणी);
	}

	कुरु {
		fp = fopen_or_warn_stdin(*argv);
		यदि (fp) {
			यदि (fp == stdin) {
				*argv = (वर्ण *) bb_संदेश_standard_आदान;
			}
			यदि (header_threshhold) {
				printf(fmt, *argv);
			}
			यदि (negative_N) {
				यदि (मोजणी_bytes) {
					print_except_N_last_bytes(fp, मोजणी);
				} अन्यथा {
					print_except_N_last_ओळी(fp, मोजणी);
				}
			} अन्यथा {
				print_first_N(fp, मोजणी, मोजणी_bytes);
			}
			die_if_ferror_stdout();
			यदि (fclose_if_not_stdin(fp)) {
				bb_simple_perror_संदेश(*argv);
				retval = EXIT_FAILURE;
			}
		} अन्यथा {
			retval = EXIT_FAILURE;
		}
		fmt = header_fmt_str;
	} यावत् (*++argv);

	fflush_stdout_and_exit(retval);
}
