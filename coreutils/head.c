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
# define print_first_N(fp,गन्ती,bytes) print_first_N(fp,गन्ती)
#endif
स्थिर शून्य
print_first_N(खाता *fp, अचिन्हित दीर्घ गन्ती, bool गन्ती_bytes)
{
#यदि !ENABLE_FEATURE_FANCY_HEAD
	अचर पूर्णाङ्क गन्ती_bytes = 0;
#endif
	जबसम्म (गन्ती) {
		पूर्णाङ्क c = अ_लो(fp);
		यदि (c == खातापूर्ण)
			रोक;
		यदि (गन्ती_bytes || (c == '\n'))
			--गन्ती;
		अक्षर_दो(c);
	}
}

#यदि ENABLE_FEATURE_FANCY_HEAD
स्थिर शून्य
print_except_N_last_bytes(खाता *fp, अचिन्हित गन्ती)
{
	अचिन्हित वर्ण *circle = xmalloc(++गन्ती);
	अचिन्हित head = 0;
	लागि (;;) {
		पूर्णाङ्क c;
		c = अ_लो(fp);
		यदि (c == खातापूर्ण)
			गच्छ ret;
		circle[head++] = c;
		यदि (head == गन्ती)
			रोक;
	}
	लागि (;;) {
		पूर्णाङ्क c;
		यदि (head == गन्ती)
			head = 0;
		अक्षर_दो(circle[head]);
		c = अ_लो(fp);
		यदि (c == खातापूर्ण)
			गच्छ ret;
		circle[head] = c;
		head++;
	}
 ret:
	मुक्त(circle);
}

स्थिर शून्य
print_except_N_last_पङ्क्तिहरू(खाता *fp, अचिन्हित गन्ती)
{
	वर्ण **circle = xzalloc((++गन्ती) * परिमाण(circle[0]));
	अचिन्हित head = 0;
	लागि (;;) {
		वर्ण *c;
		c = xmalloc_fgets(fp);
		यदि (!c)
			गच्छ ret;
		circle[head++] = c;
		यदि (head == गन्ती)
			रोक;
	}
	लागि (;;) {
		वर्ण *c;
		यदि (head == गन्ती)
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
	लागि (;;) {
		मुक्त(circle[head++]);
		यदि (head == गन्ती)
			रोक;
	}
	मुक्त(circle);
}
#अन्यथा
/* Must never be called */
शून्य print_except_N_last_bytes(खाता *fp, अचिन्हित गन्ती);
शून्य print_except_N_last_पङ्क्तिहरू(खाता *fp, अचिन्हित गन्ती);
#endif

#यदि !ENABLE_FEATURE_FANCY_HEAD
# define eat_अङ्क(negative_N,p) eat_अङ्क(p)
#endif
स्थिर अचिन्हित दीर्घ
eat_अङ्क(bool *negative_N, अचर वर्ण *p)
{
#यदि ENABLE_FEATURE_FANCY_HEAD
	यदि (*p == '-') {
		*negative_N = 1;
		p++;
	}
#endif
	फर्काऊ xatoul_sfx(p, bkm_suffixes);
}

स्थिर अचर वर्ण head_opts[] ALIGN1 =
	"n:"
#यदि ENABLE_FEATURE_FANCY_HEAD
	"c:qv"
#endif
	;

#define header_fmt_str "\n==> %s <==\n"

पूर्णाङ्क head_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क head_main(पूर्णाङ्क argc, वर्ण **argv)
{
	अचिन्हित दीर्घ गन्ती = 10;
#यदि ENABLE_FEATURE_FANCY_HEAD
	पूर्णाङ्क header_threshhold = 1;
	bool गन्ती_bytes = 0;
	bool negative_N = 0;
#अन्यथा
# define header_threshhold 1
# define गन्ती_bytes       0
# define negative_N        0
#endif
	खाता *fp;
	अचर वर्ण *fmt;
	वर्ण *p;
	पूर्णाङ्क opt;
	पूर्णाङ्क retval = निकास_सफल;

#यदि ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_HEAD
	/* Allow legacy syntax of an initial numeric option without -n. */
	यदि (argv[1] && argv[1][0] == '-'
	 && है_अंक(argv[1][1])
	) {
		--argc;
		++argv;
		p = argv[0] + 1;
		गच्छ GET_गन्ती;
	}
#endif

	/* No size benefit in converting this to getopt32 */
	जबसम्म ((opt = getopt(argc, argv, head_opts)) > 0) {
		विकल्प (opt) {
#यदि ENABLE_FEATURE_FANCY_HEAD
		पक्ष 'q':
			header_threshhold = पूर्णांक_उच्च;
			रोक;
		पक्ष 'v':
			header_threshhold = -1;
			रोक;
		पक्ष 'c':
			गन्ती_bytes = 1;
			/* fall through */
#endif
		पक्ष 'n':
			p = optarg;
#यदि ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_HEAD
 GET_गन्ती:
#endif
			गन्ती = eat_अङ्क(&negative_N, p);
			रोक;
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
		यदि (गन्ती >= पूर्णांक_उच्च / परिमाण(वर्ण*))
			bb_error_सन्देश("count is too big: %lu", गन्ती);
	}

	गर {
		fp = fopen_or_warn_stdin(*argv);
		यदि (fp) {
			यदि (fp == मानक_निवेश) {
				*argv = (वर्ण *) bb_सन्देश_standard_आगत;
			}
			यदि (header_threshhold) {
				म_लिखो(fmt, *argv);
			}
			यदि (negative_N) {
				यदि (गन्ती_bytes) {
					print_except_N_last_bytes(fp, गन्ती);
				} अन्यथा {
					print_except_N_last_पङ्क्तिहरू(fp, गन्ती);
				}
			} अन्यथा {
				print_first_N(fp, गन्ती, गन्ती_bytes);
			}
			die_if_ferror_stdout();
			यदि (fclose_if_not_stdin(fp)) {
				bb_simple_perror_सन्देश(*argv);
				retval = निकास_त्रुटि;
			}
		} अन्यथा {
			retval = निकास_त्रुटि;
		}
		fmt = header_fmt_str;
	} जबसम्म (*++argv);

	fflush_stdout_and_exit(retval);
}
