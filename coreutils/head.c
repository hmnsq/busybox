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

#जर !ENABLE_FEATURE_FANCY_HEAD
# define print_first_N(fp,मोजणी,bytes) print_first_N(fp,मोजणी)
#endif
स्थिर शून्य
print_first_N(खाता *fp, अचिन्हित दीर्घ मोजणी, bool मोजणी_bytes)
{
#जर !ENABLE_FEATURE_FANCY_HEAD
	स्थिरांक पूर्णांक मोजणी_bytes = 0;
#endif
	पर्यंत (मोजणी) {
		पूर्णांक c = अ_लो(fp);
		जर (c == खातापूर्ण)
			थांबा;
		जर (मोजणी_bytes || (c == '\n'))
			--मोजणी;
		अक्षर_दो(c);
	}
}

#जर ENABLE_FEATURE_FANCY_HEAD
स्थिर शून्य
print_except_N_last_bytes(खाता *fp, अचिन्हित मोजणी)
{
	अचिन्हित अक्षर *circle = xmalloc(++मोजणी);
	अचिन्हित head = 0;
	साठी (;;) {
		पूर्णांक c;
		c = अ_लो(fp);
		जर (c == खातापूर्ण)
			जा ret;
		circle[head++] = c;
		जर (head == मोजणी)
			थांबा;
	}
	साठी (;;) {
		पूर्णांक c;
		जर (head == मोजणी)
			head = 0;
		अक्षर_दो(circle[head]);
		c = अ_लो(fp);
		जर (c == खातापूर्ण)
			जा ret;
		circle[head] = c;
		head++;
	}
 ret:
	मुक्त(circle);
}

स्थिर शून्य
print_except_N_last_ओळी(खाता *fp, अचिन्हित मोजणी)
{
	अक्षर **circle = xzalloc((++मोजणी) * आकार(circle[0]));
	अचिन्हित head = 0;
	साठी (;;) {
		अक्षर *c;
		c = xmalloc_fgets(fp);
		जर (!c)
			जा ret;
		circle[head++] = c;
		जर (head == मोजणी)
			थांबा;
	}
	साठी (;;) {
		अक्षर *c;
		जर (head == मोजणी)
			head = 0;
		fputs_stdout(circle[head]);
		c = xmalloc_fgets(fp);
		जर (!c)
			जा ret;
		मुक्त(circle[head]);
		circle[head++] = c;
	}
 ret:
	head = 0;
	साठी (;;) {
		मुक्त(circle[head++]);
		जर (head == मोजणी)
			थांबा;
	}
	मुक्त(circle);
}
#अन्यथा
/* Must never be called */
शून्य print_except_N_last_bytes(खाता *fp, अचिन्हित मोजणी);
शून्य print_except_N_last_ओळी(खाता *fp, अचिन्हित मोजणी);
#endif

#जर !ENABLE_FEATURE_FANCY_HEAD
# define eat_अंक(negative_N,p) eat_अंक(p)
#endif
स्थिर अचिन्हित दीर्घ
eat_अंक(bool *negative_N, स्थिरांक अक्षर *p)
{
#जर ENABLE_FEATURE_FANCY_HEAD
	जर (*p == '-') {
		*negative_N = 1;
		p++;
	}
#endif
	परत xatoul_sfx(p, bkm_suffixes);
}

स्थिर स्थिरांक अक्षर head_opts[] ALIGN1 =
	"n:"
#जर ENABLE_FEATURE_FANCY_HEAD
	"c:qv"
#endif
	;

#define header_fmt_str "\n==> %s <==\n"

पूर्णांक head_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक head_main(पूर्णांक argc, अक्षर **argv)
{
	अचिन्हित दीर्घ मोजणी = 10;
#जर ENABLE_FEATURE_FANCY_HEAD
	पूर्णांक header_threshhold = 1;
	bool मोजणी_bytes = 0;
	bool negative_N = 0;
#अन्यथा
# define header_threshhold 1
# define मोजणी_bytes       0
# define negative_N        0
#endif
	खाता *fp;
	स्थिरांक अक्षर *fmt;
	अक्षर *p;
	पूर्णांक opt;
	पूर्णांक retval = निकास_सफल;

#जर ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_HEAD
	/* Allow legacy syntax of an initial numeric option without -n. */
	जर (argv[1] && argv[1][0] == '-'
	 && है_अंक(argv[1][1])
	) {
		--argc;
		++argv;
		p = argv[0] + 1;
		जा GET_मोजणी;
	}
#endif

	/* No size benefit in converting this to getopt32 */
	पर्यंत ((opt = getopt(argc, argv, head_opts)) > 0) {
		निवड (opt) {
#जर ENABLE_FEATURE_FANCY_HEAD
		स्थिती 'q':
			header_threshhold = पूर्णांक_उच्च;
			थांबा;
		स्थिती 'v':
			header_threshhold = -1;
			थांबा;
		स्थिती 'c':
			मोजणी_bytes = 1;
			/* fall through */
#endif
		स्थिती 'n':
			p = optarg;
#जर ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_HEAD
 GET_मोजणी:
#endif
			मोजणी = eat_अंक(&negative_N, p);
			थांबा;
		पूर्वनिर्धारित:
			bb_show_usage();
		}
	}

	argc -= optind;
	argv += optind;
	जर (!*argv)
		*--argv = (अक्षर*)"-";

	fmt = header_fmt_str + 1;
	जर (argc <= header_threshhold) {
#जर ENABLE_FEATURE_FANCY_HEAD
		header_threshhold = 0;
#अन्यथा
		fmt += 11; /* "" */
#endif
	}
	जर (negative_N) {
		जर (मोजणी >= पूर्णांक_उच्च / आकार(अक्षर*))
			bb_error_संदेश("count is too big: %lu", मोजणी);
	}

	करा {
		fp = fopen_or_warn_stdin(*argv);
		जर (fp) {
			जर (fp == मानक_निवेश) {
				*argv = (अक्षर *) bb_संदेश_standard_आदान;
			}
			जर (header_threshhold) {
				म_लिखो(fmt, *argv);
			}
			जर (negative_N) {
				जर (मोजणी_bytes) {
					print_except_N_last_bytes(fp, मोजणी);
				} अन्यथा {
					print_except_N_last_ओळी(fp, मोजणी);
				}
			} अन्यथा {
				print_first_N(fp, मोजणी, मोजणी_bytes);
			}
			die_if_ferror_stdout();
			जर (fclose_if_not_stdin(fp)) {
				bb_simple_perror_संदेश(*argv);
				retval = निकास_त्रुटि;
			}
		} अन्यथा {
			retval = निकास_त्रुटि;
		}
		fmt = header_fmt_str;
	} पर्यंत (*++argv);

	fflush_stdout_and_exit(retval);
}
