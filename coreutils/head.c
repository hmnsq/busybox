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

#जइ !ENABLE_FEATURE_FANCY_HEAD
# define print_first_N(fp,गणनम्,bytes) print_first_N(fp,गणनम्)
#endif
स्थिर शून्य
print_first_N(FILE *fp, unsigned long गणनम्, bool गणनम्_bytes)
{
#जइ !ENABLE_FEATURE_FANCY_HEAD
	ध्रुव पूर्णाङ्क गणनम्_bytes = 0;
#endif
	जाव (गणनम्) {
		पूर्णाङ्क c = getc(fp);
		जइ (c == EOF)
			भङ्ग;
		जइ (गणनम्_bytes || (c == '\n'))
			--गणनम्;
		putchar(c);
	}
}

#जइ ENABLE_FEATURE_FANCY_HEAD
स्थिर शून्य
print_except_N_last_bytes(FILE *fp, unsigned गणनम्)
{
	unsigned वर्ण *circle = xmalloc(++गणनम्);
	unsigned head = 0;
	प्रति (;;) {
		पूर्णाङ्क c;
		c = getc(fp);
		जइ (c == EOF)
			गच्छ ret;
		circle[head++] = c;
		जइ (head == गणनम्)
			भङ्ग;
	}
	प्रति (;;) {
		पूर्णाङ्क c;
		जइ (head == गणनम्)
			head = 0;
		putchar(circle[head]);
		c = getc(fp);
		जइ (c == EOF)
			गच्छ ret;
		circle[head] = c;
		head++;
	}
 ret:
	मुक्त(circle);
}

स्थिर शून्य
print_except_N_last_पङ्क्तयः(FILE *fp, unsigned गणनम्)
{
	वर्ण **circle = xzalloc((++गणनम्) * परिमाण(circle[0]));
	unsigned head = 0;
	प्रति (;;) {
		वर्ण *c;
		c = xmalloc_fgets(fp);
		जइ (!c)
			गच्छ ret;
		circle[head++] = c;
		जइ (head == गणनम्)
			भङ्ग;
	}
	प्रति (;;) {
		वर्ण *c;
		जइ (head == गणनम्)
			head = 0;
		fputs_stdout(circle[head]);
		c = xmalloc_fgets(fp);
		जइ (!c)
			गच्छ ret;
		मुक्त(circle[head]);
		circle[head++] = c;
	}
 ret:
	head = 0;
	प्रति (;;) {
		मुक्त(circle[head++]);
		जइ (head == गणनम्)
			भङ्ग;
	}
	मुक्त(circle);
}
#अण्णहा
/* Must never be called */
शून्य print_except_N_last_bytes(FILE *fp, unsigned गणनम्);
शून्य print_except_N_last_पङ्क्तयः(FILE *fp, unsigned गणनम्);
#endif

#जइ !ENABLE_FEATURE_FANCY_HEAD
# define eat_अङ्कः(negative_N,p) eat_अङ्कः(p)
#endif
स्थिर unsigned long
eat_अङ्कः(bool *negative_N, ध्रुव वर्ण *p)
{
#जइ ENABLE_FEATURE_FANCY_HEAD
	जइ (*p == '-') {
		*negative_N = 1;
		p++;
	}
#endif
	प्रत्यागम xatoul_sfx(p, bkm_suffixes);
}

स्थिर ध्रुव वर्ण head_opts[] ALIGN1 =
	"n:"
#जइ ENABLE_FEATURE_FANCY_HEAD
	"c:qv"
#endif
	;

#define header_fmt_str "\n==> %s <==\n"

पूर्णाङ्क head_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क head_main(पूर्णाङ्क argc, वर्ण **argv)
{
	unsigned long गणनम् = 10;
#जइ ENABLE_FEATURE_FANCY_HEAD
	पूर्णाङ्क header_threshhold = 1;
	bool गणनम्_bytes = 0;
	bool negative_N = 0;
#अण्णहा
# define header_threshhold 1
# define गणनम्_bytes       0
# define negative_N        0
#endif
	FILE *fp;
	ध्रुव वर्ण *fmt;
	वर्ण *p;
	पूर्णाङ्क opt;
	पूर्णाङ्क retval = EXIT_SUCCESS;

#जइ ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_HEAD
	/* Allow legacy syntax of an initial numeric option without -n. */
	जइ (argv[1] && argv[1][0] == '-'
	 && isdigit(argv[1][1])
	) {
		--argc;
		++argv;
		p = argv[0] + 1;
		गच्छ GET_गणनम्;
	}
#endif

	/* No size benefit in converting this to getopt32 */
	जाव ((opt = getopt(argc, argv, head_opts)) > 0) {
		विकल्प (opt) {
#जइ ENABLE_FEATURE_FANCY_HEAD
		पक्ष 'q':
			header_threshhold = INT_MAX;
			भङ्ग;
		पक्ष 'v':
			header_threshhold = -1;
			भङ्ग;
		पक्ष 'c':
			गणनम्_bytes = 1;
			/* fall through */
#endif
		पक्ष 'n':
			p = optarg;
#जइ ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_HEAD
 GET_गणनम्:
#endif
			गणनम् = eat_अङ्कः(&negative_N, p);
			भङ्ग;
		स्वभाव:
			bb_show_usage();
		}
	}

	argc -= optind;
	argv += optind;
	जइ (!*argv)
		*--argv = (वर्ण*)"-";

	fmt = header_fmt_str + 1;
	जइ (argc <= header_threshhold) {
#जइ ENABLE_FEATURE_FANCY_HEAD
		header_threshhold = 0;
#अण्णहा
		fmt += 11; /* "" */
#endif
	}
	जइ (negative_N) {
		जइ (गणनम् >= INT_MAX / परिमाण(वर्ण*))
			bb_error_सन्देशः("count is too big: %lu", गणनम्);
	}

	करेहि {
		fp = fopen_or_warn_stdin(*argv);
		जइ (fp) {
			जइ (fp == stdin) {
				*argv = (वर्ण *) bb_सन्देशः_standard_निवेशः;
			}
			जइ (header_threshhold) {
				printf(fmt, *argv);
			}
			जइ (negative_N) {
				जइ (गणनम्_bytes) {
					print_except_N_last_bytes(fp, गणनम्);
				} अण्णहा {
					print_except_N_last_पङ्क्तयः(fp, गणनम्);
				}
			} अण्णहा {
				print_first_N(fp, गणनम्, गणनम्_bytes);
			}
			die_if_ferror_stdout();
			जइ (fclose_if_not_stdin(fp)) {
				bb_simple_perror_सन्देशः(*argv);
				retval = EXIT_FAILURE;
			}
		} अण्णहा {
			retval = EXIT_FAILURE;
		}
		fmt = header_fmt_str;
	} जाव (*++argv);

	fflush_stdout_and_exit(retval);
}
