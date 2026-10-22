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

#समावेश "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

#अगर !ENABLE_FEATURE_FANCY_शीर्ष
# define print_first_N(fp,गिनती,bytes) print_first_N(fp,गिनती)
#पूर्ण_अगर
अटल व्योम
print_first_N(खाता *fp, अचिन्हित दीर्घ गिनती, bool गिनती_bytes)
{
#अगर !ENABLE_FEATURE_FANCY_शीर्ष
	स्थिर पूर्णांक गिनती_bytes = 0;
#पूर्ण_अगर
	जबतक (गिनती) {
		पूर्णांक c = अ_लो(fp);
		अगर (c == खातापूर्ण)
			अवरोध;
		अगर (गिनती_bytes || (c == '\n'))
			--गिनती;
		अक्षर_दो(c);
	}
}

#अगर ENABLE_FEATURE_FANCY_शीर्ष
अटल व्योम
print_except_N_last_bytes(खाता *fp, अचिन्हित गिनती)
{
	अचिन्हित अक्षर *circle = xmalloc(++गिनती);
	अचिन्हित शीर्ष = 0;
	क्रम (;;) {
		पूर्णांक c;
		c = अ_लो(fp);
		अगर (c == खातापूर्ण)
			जाओ ret;
		circle[शीर्ष++] = c;
		अगर (शीर्ष == गिनती)
			अवरोध;
	}
	क्रम (;;) {
		पूर्णांक c;
		अगर (शीर्ष == गिनती)
			शीर्ष = 0;
		अक्षर_दो(circle[शीर्ष]);
		c = अ_लो(fp);
		अगर (c == खातापूर्ण)
			जाओ ret;
		circle[शीर्ष] = c;
		शीर्ष++;
	}
 ret:
	मुक्त(circle);
}

अटल व्योम
print_except_N_last_पंक्तियाँ(खाता *fp, अचिन्हित गिनती)
{
	अक्षर **circle = xzalloc((++गिनती) * माप(circle[0]));
	अचिन्हित शीर्ष = 0;
	क्रम (;;) {
		अक्षर *c;
		c = xmalloc_fgets(fp);
		अगर (!c)
			जाओ ret;
		circle[शीर्ष++] = c;
		अगर (शीर्ष == गिनती)
			अवरोध;
	}
	क्रम (;;) {
		अक्षर *c;
		अगर (शीर्ष == गिनती)
			शीर्ष = 0;
		fputs_stdout(circle[शीर्ष]);
		c = xmalloc_fgets(fp);
		अगर (!c)
			जाओ ret;
		मुक्त(circle[शीर्ष]);
		circle[शीर्ष++] = c;
	}
 ret:
	शीर्ष = 0;
	क्रम (;;) {
		मुक्त(circle[शीर्ष++]);
		अगर (शीर्ष == गिनती)
			अवरोध;
	}
	मुक्त(circle);
}
#अन्यथा
/* Must never be called */
व्योम print_except_N_last_bytes(खाता *fp, अचिन्हित गिनती);
व्योम print_except_N_last_पंक्तियाँ(खाता *fp, अचिन्हित गिनती);
#पूर्ण_अगर

#अगर !ENABLE_FEATURE_FANCY_शीर्ष
# define eat_अंक(negative_N,p) eat_अंक(p)
#पूर्ण_अगर
अटल अचिन्हित दीर्घ
eat_अंक(bool *negative_N, स्थिर अक्षर *p)
{
#अगर ENABLE_FEATURE_FANCY_शीर्ष
	अगर (*p == '-') {
		*negative_N = 1;
		p++;
	}
#पूर्ण_अगर
	वापस xatoul_sfx(p, bkm_suffixes);
}

अटल स्थिर अक्षर शीर्ष_opts[] ALIGN1 =
	"n:"
#अगर ENABLE_FEATURE_FANCY_शीर्ष
	"c:qv"
#पूर्ण_अगर
	;

#घोषणा header_fmt_str "\n==> %s <==\n"

पूर्णांक शीर्ष_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक शीर्ष_main(पूर्णांक argc, अक्षर **argv)
{
	अचिन्हित दीर्घ गिनती = 10;
#अगर ENABLE_FEATURE_FANCY_शीर्ष
	पूर्णांक header_threshhold = 1;
	bool गिनती_bytes = 0;
	bool negative_N = 0;
#अन्यथा
# define header_threshhold 1
# define गिनती_bytes       0
# define negative_N        0
#पूर्ण_अगर
	खाता *fp;
	स्थिर अक्षर *fmt;
	अक्षर *p;
	पूर्णांक opt;
	पूर्णांक retval = निकास_सफल;

#अगर ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_शीर्ष
	/* Allow legacy syntax of an initial numeric option without -n. */
	अगर (argv[1] && argv[1][0] == '-'
	 && है_अंक(argv[1][1])
	) {
		--argc;
		++argv;
		p = argv[0] + 1;
		जाओ GET_गिनती;
	}
#पूर्ण_अगर

	/* No size benefit in converting this to getopt32 */
	जबतक ((opt = getopt(argc, argv, शीर्ष_opts)) > 0) {
		चयन (opt) {
#अगर ENABLE_FEATURE_FANCY_शीर्ष
		हाल 'q':
			header_threshhold = पूर्णांक_उच्च;
			अवरोध;
		हाल 'v':
			header_threshhold = -1;
			अवरोध;
		हाल 'c':
			गिनती_bytes = 1;
			/* fall through */
#पूर्ण_अगर
		हाल 'n':
			p = optarg;
#अगर ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_शीर्ष
 GET_गिनती:
#पूर्ण_अगर
			गिनती = eat_अंक(&negative_N, p);
			अवरोध;
		शेष:
			bb_show_usage();
		}
	}

	argc -= optind;
	argv += optind;
	अगर (!*argv)
		*--argv = (अक्षर*)"-";

	fmt = header_fmt_str + 1;
	अगर (argc <= header_threshhold) {
#अगर ENABLE_FEATURE_FANCY_शीर्ष
		header_threshhold = 0;
#अन्यथा
		fmt += 11; /* "" */
#पूर्ण_अगर
	}
	अगर (negative_N) {
		अगर (गिनती >= पूर्णांक_उच्च / माप(अक्षर*))
			bb_error_संदेश("count is too big: %lu", गिनती);
	}

	करो {
		fp = fopen_or_warn_stdin(*argv);
		अगर (fp) {
			अगर (fp == मानक_निवेश) {
				*argv = (अक्षर *) bb_संदेश_standard_निवेश;
			}
			अगर (header_threshhold) {
				म_लिखो(fmt, *argv);
			}
			अगर (negative_N) {
				अगर (गिनती_bytes) {
					print_except_N_last_bytes(fp, गिनती);
				} अन्यथा {
					print_except_N_last_पंक्तियाँ(fp, गिनती);
				}
			} अन्यथा {
				print_first_N(fp, गिनती, गिनती_bytes);
			}
			die_if_ferror_stdout();
			अगर (fclose_if_not_stdin(fp)) {
				bb_simple_perror_संदेश(*argv);
				retval = निकास_त्रुटि;
			}
		} अन्यथा {
			retval = निकास_त्रुटि;
		}
		fmt = header_fmt_str;
	} जबतक (*++argv);

	fflush_stdout_and_exit(retval);
}
