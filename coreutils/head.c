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

#यदि !ENABLE_FEATURE_FANCY_शीर्ष
# define print_first_N(fp,गणनम्,अष्टकानि) print_first_N(fp,गणनम्)
#यदि_अन्तः
स्थिर शून्य
print_first_N(FILE *fp, अचिह्नित दीर्घ गणनम्, bool गणनम्_अष्टकानि)
{
#यदि !ENABLE_FEATURE_FANCY_शीर्ष
	ध्रुव पूर्णाङ्क गणनम्_अष्टकानि = 0;
#यदि_अन्तः
	यावत् (गणनम्) {
		पूर्णाङ्क c = getc(fp);
		यदि (c == EOF)
			भङ्ग;
		यदि (गणनम्_अष्टकानि || (c == '\n'))
			--गणनम्;
		अक्षरं_लिख(c);
	}
}

#यदि ENABLE_FEATURE_FANCY_शीर्ष
स्थिर शून्य
print_except_N_चरमः_अष्टकानि(FILE *fp, अचिह्नित गणनम्)
{
	अचिह्नित वर्ण *circle = xmalloc(++गणनम्);
	अचिह्नित शीर्ष = 0;
	प्रति (;;) {
		पूर्णाङ्क c;
		c = getc(fp);
		यदि (c == EOF)
			गच्छ ret;
		circle[शीर्ष++] = c;
		यदि (शीर्ष == गणनम्)
			भङ्ग;
	}
	प्रति (;;) {
		पूर्णाङ्क c;
		यदि (शीर्ष == गणनम्)
			शीर्ष = 0;
		अक्षरं_लिख(circle[शीर्ष]);
		c = getc(fp);
		यदि (c == EOF)
			गच्छ ret;
		circle[शीर्ष] = c;
		शीर्ष++;
	}
 ret:
	मुक्त(circle);
}

स्थिर शून्य
print_except_N_चरमः_पङ्क्तयः(FILE *fp, अचिह्नित गणनम्)
{
	वर्ण **circle = xzalloc((++गणनम्) * परिमाण(circle[0]));
	अचिह्नित शीर्ष = 0;
	प्रति (;;) {
		वर्ण *c;
		c = xmalloc_fgets(fp);
		यदि (!c)
			गच्छ ret;
		circle[शीर्ष++] = c;
		यदि (शीर्ष == गणनम्)
			भङ्ग;
	}
	प्रति (;;) {
		वर्ण *c;
		यदि (शीर्ष == गणनम्)
			शीर्ष = 0;
		fputs_stdout(circle[शीर्ष]);
		c = xmalloc_fgets(fp);
		यदि (!c)
			गच्छ ret;
		मुक्त(circle[शीर्ष]);
		circle[शीर्ष++] = c;
	}
 ret:
	शीर्ष = 0;
	प्रति (;;) {
		मुक्त(circle[शीर्ष++]);
		यदि (शीर्ष == गणनम्)
			भङ्ग;
	}
	मुक्त(circle);
}
#अन्यथा
/* Must never be called */
शून्य print_except_N_चरमः_अष्टकानि(FILE *fp, अचिह्नित गणनम्);
शून्य print_except_N_चरमः_पङ्क्तयः(FILE *fp, अचिह्नित गणनम्);
#यदि_अन्तः

#यदि !ENABLE_FEATURE_FANCY_शीर्ष
# define eat_अङ्कः(negative_N,p) eat_अङ्कः(p)
#यदि_अन्तः
स्थिर अचिह्नित दीर्घ
eat_अङ्कः(bool *negative_N, ध्रुव वर्ण *p)
{
#यदि ENABLE_FEATURE_FANCY_शीर्ष
	यदि (*p == '-') {
		*negative_N = 1;
		p++;
	}
#यदि_अन्तः
	प्रत्यागम xatoul_sfx(p, bkm_suffixes);
}

स्थिर ध्रुव वर्ण शीर्ष_opts[] ALIGN1 =
	"n:"
#यदि ENABLE_FEATURE_FANCY_शीर्ष
	"c:qv"
#यदि_अन्तः
	;

#परिभाषय शीर्षकम्_fmt_str "\n==> %s <==\n"

पूर्णाङ्क शीर्ष_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क शीर्ष_main(पूर्णाङ्क argc, वर्ण **argv)
{
	अचिह्नित दीर्घ गणनम् = 10;
#यदि ENABLE_FEATURE_FANCY_शीर्ष
	पूर्णाङ्क शीर्षकम्_threshhold = 1;
	bool गणनम्_अष्टकानि = 0;
	bool negative_N = 0;
#अन्यथा
# define शीर्षकम्_threshhold 1
# define गणनम्_अष्टकानि       0
# define negative_N        0
#यदि_अन्तः
	FILE *fp;
	ध्रुव वर्ण *fmt;
	वर्ण *p;
	पूर्णाङ्क opt;
	पूर्णाङ्क retval = EXIT_SUCCESS;

#यदि ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_शीर्ष
	/* Allow legacy syntax of an initial numeric option without -n. */
	यदि (argv[1] && argv[1][0] == '-'
	 && अङ्कः_वा(argv[1][1])
	) {
		--argc;
		++argv;
		p = argv[0] + 1;
		गच्छ GET_गणनम्;
	}
#यदि_अन्तः

	/* No size benefit in converting this to getopt32 */
	यावत् ((opt = getopt(argc, argv, शीर्ष_opts)) > 0) {
		विकल्प (opt) {
#यदि ENABLE_FEATURE_FANCY_शीर्ष
		पक्ष 'q':
			शीर्षकम्_threshhold = अधिकतम_पूर्णाङ्कः;
			भङ्ग;
		पक्ष 'v':
			शीर्षकम्_threshhold = -1;
			भङ्ग;
		पक्ष 'c':
			गणनम्_अष्टकानि = 1;
			/* fall through */
#यदि_अन्तः
		पक्ष 'n':
			p = optarg;
#यदि ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_शीर्ष
 GET_गणनम्:
#यदि_अन्तः
			गणनम् = eat_अङ्कः(&negative_N, p);
			भङ्ग;
		स्वभाव:
			bb_show_usage();
		}
	}

	argc -= optind;
	argv += optind;
	यदि (!*argv)
		*--argv = (वर्ण*)"-";

	fmt = शीर्षकम्_fmt_str + 1;
	यदि (argc <= शीर्षकम्_threshhold) {
#यदि ENABLE_FEATURE_FANCY_शीर्ष
		शीर्षकम्_threshhold = 0;
#अन्यथा
		fmt += 11; /* "" */
#यदि_अन्तः
	}
	यदि (negative_N) {
		यदि (गणनम् >= अधिकतम_पूर्णाङ्कः / परिमाण(वर्ण*))
			bb_error_सन्देशः("count is too big: %lu", गणनम्);
	}

	कुरु {
		fp = fopen_or_warn_stdin(*argv);
		यदि (fp) {
			यदि (fp == stdin) {
				*argv = (वर्ण *) bb_सन्देशः_standard_निवेशः;
			}
			यदि (शीर्षकम्_threshhold) {
				प्रारूप_लिख(fmt, *argv);
			}
			यदि (negative_N) {
				यदि (गणनम्_अष्टकानि) {
					print_except_N_चरमः_अष्टकानि(fp, गणनम्);
				} अन्यथा {
					print_except_N_चरमः_पङ्क्तयः(fp, गणनम्);
				}
			} अन्यथा {
				print_first_N(fp, गणनम्, गणनम्_अष्टकानि);
			}
			die_if_ferror_stdout();
			यदि (fclose_if_not_stdin(fp)) {
				bb_simple_perror_सन्देशः(*argv);
				retval = EXIT_FAILURE;
			}
		} अन्यथा {
			retval = EXIT_FAILURE;
		}
		fmt = शीर्षकम्_fmt_str;
	} यावत् (*++argv);

	fflush_stdout_and_exit(retval);
}
