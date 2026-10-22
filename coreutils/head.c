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

#అన్తర్భావ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

#ఒకవేళ !ENABLE_FEATURE_FANCY_తల
# define print_మొదటి_N(fp,లెక్క,బైట్లు) print_మొదటి_N(fp,లెక్క)
#అయితే_ముగింపు
స్థిర శూన్య
print_మొదటి_N(FILE *fp, గుర్తులేని దీర్ఘ లెక్క, bool లెక్క_బైట్లు)
{
#ఒకవేళ !ENABLE_FEATURE_FANCY_తల
	స్థిరరాశి పూర్ణాంకం లెక్క_బైట్లు = 0;
#అయితే_ముగింపు
	అయితే (లెక్క) {
		పూర్ణాంకం c = getc(fp);
		ఒకవేళ (c == EOF)
			ఆపు;
		ఒకవేళ (లెక్క_బైట్లు || (c == '\n'))
			--లెక్క;
		అక్షరం_రాయి(c);
	}
}

#ఒకవేళ ENABLE_FEATURE_FANCY_తల
స్థిర శూన్య
print_except_N_చివరి_బైట్లు(FILE *fp, గుర్తులేని లెక్క)
{
	గుర్తులేని అక్షరం *circle = xmalloc(++లెక్క);
	గుర్తులేని తల = 0;
	కొరకు (;;) {
		పూర్ణాంకం c;
		c = getc(fp);
		ఒకవేళ (c == EOF)
			వెళ్ళు ret;
		circle[తల++] = c;
		ఒకవేళ (తల == లెక్క)
			ఆపు;
	}
	కొరకు (;;) {
		పూర్ణాంకం c;
		ఒకవేళ (తల == లెక్క)
			తల = 0;
		అక్షరం_రాయి(circle[తల]);
		c = getc(fp);
		ఒకవేళ (c == EOF)
			వెళ్ళు ret;
		circle[తల] = c;
		తల++;
	}
 ret:
	విడుదల(circle);
}

స్థిర శూన్య
print_except_N_చివరి_పంక్తులు(FILE *fp, గుర్తులేని లెక్క)
{
	అక్షరం **circle = xzalloc((++లెక్క) * పరిమాణం(circle[0]));
	గుర్తులేని తల = 0;
	కొరకు (;;) {
		అక్షరం *c;
		c = xmalloc_fgets(fp);
		ఒకవేళ (!c)
			వెళ్ళు ret;
		circle[తల++] = c;
		ఒకవేళ (తల == లెక్క)
			ఆపు;
	}
	కొరకు (;;) {
		అక్షరం *c;
		ఒకవేళ (తల == లెక్క)
			తల = 0;
		fputs_stdout(circle[తల]);
		c = xmalloc_fgets(fp);
		ఒకవేళ (!c)
			వెళ్ళు ret;
		విడుదల(circle[తల]);
		circle[తల++] = c;
	}
 ret:
	తల = 0;
	కొరకు (;;) {
		విడుదల(circle[తల++]);
		ఒకవేళ (తల == లెక్క)
			ఆపు;
	}
	విడుదల(circle);
}
#లేకపోతే
/* Must never be called */
శూన్య print_except_N_చివరి_బైట్లు(FILE *fp, గుర్తులేని లెక్క);
శూన్య print_except_N_చివరి_పంక్తులు(FILE *fp, గుర్తులేని లెక్క);
#అయితే_ముగింపు

#ఒకవేళ !ENABLE_FEATURE_FANCY_తల
# define eat_అంకె(negative_N,p) eat_అంకె(p)
#అయితే_ముగింపు
స్థిర గుర్తులేని దీర్ఘ
eat_అంకె(bool *negative_N, స్థిరరాశి అక్షరం *p)
{
#ఒకవేళ ENABLE_FEATURE_FANCY_తల
	ఒకవేళ (*p == '-') {
		*negative_N = 1;
		p++;
	}
#అయితే_ముగింపు
	తిరిగి xatoul_sfx(p, bkm_suffixes);
}

స్థిర స్థిరరాశి అక్షరం తల_opts[] ALIGN1 =
	"n:"
#ఒకవేళ ENABLE_FEATURE_FANCY_తల
	"c:qv"
#అయితే_ముగింపు
	;

#నిర్వచించు శీర్షిక_fmt_str "\n==> %s <==\n"

పూర్ణాంకం తల_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం తల_main(పూర్ణాంకం argc, అక్షరం **argv)
{
	గుర్తులేని దీర్ఘ లెక్క = 10;
#ఒకవేళ ENABLE_FEATURE_FANCY_తల
	పూర్ణాంకం శీర్షిక_threshhold = 1;
	bool లెక్క_బైట్లు = 0;
	bool negative_N = 0;
#లేకపోతే
# define శీర్షిక_threshhold 1
# define లెక్క_బైట్లు       0
# define negative_N        0
#అయితే_ముగింపు
	FILE *fp;
	స్థిరరాశి అక్షరం *fmt;
	అక్షరం *p;
	పూర్ణాంకం opt;
	పూర్ణాంకం retval = EXIT_SUCCESS;

#ఒకవేళ ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_తల
	/* Allow legacy syntax of an initial numeric option without -n. */
	ఒకవేళ (argv[1] && argv[1][0] == '-'
	 && అంకెనా(argv[1][1])
	) {
		--argc;
		++argv;
		p = argv[0] + 1;
		వెళ్ళు GET_లెక్క;
	}
#అయితే_ముగింపు

	/* No size benefit in converting this to getopt32 */
	అయితే ((opt = getopt(argc, argv, తల_opts)) > 0) {
		ఎంపిక (opt) {
#ఒకవేళ ENABLE_FEATURE_FANCY_తల
		స్థితి 'q':
			శీర్షిక_threshhold = గరిష్ఠ_పూర్ణాంకం;
			ఆపు;
		స్థితి 'v':
			శీర్షిక_threshhold = -1;
			ఆపు;
		స్థితి 'c':
			లెక్క_బైట్లు = 1;
			/* fall through */
#అయితే_ముగింపు
		స్థితి 'n':
			p = optarg;
#ఒకవేళ ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_తల
 GET_లెక్క:
#అయితే_ముగింపు
			లెక్క = eat_అంకె(&negative_N, p);
			ఆపు;
		అప్రమేయ:
			bb_show_usage();
		}
	}

	argc -= optind;
	argv += optind;
	ఒకవేళ (!*argv)
		*--argv = (అక్షరం*)"-";

	fmt = శీర్షిక_fmt_str + 1;
	ఒకవేళ (argc <= శీర్షిక_threshhold) {
#ఒకవేళ ENABLE_FEATURE_FANCY_తల
		శీర్షిక_threshhold = 0;
#లేకపోతే
		fmt += 11; /* "" */
#అయితే_ముగింపు
	}
	ఒకవేళ (negative_N) {
		ఒకవేళ (లెక్క >= గరిష్ఠ_పూర్ణాంకం / పరిమాణం(అక్షరం*))
			bb_error_సందేశం("count is too big: %lu", లెక్క);
	}

	చేయి {
		fp = fopen_or_warn_stdin(*argv);
		ఒకవేళ (fp) {
			ఒకవేళ (fp == stdin) {
				*argv = (అక్షరం *) bb_సందేశం_standard_ఆదానం;
			}
			ఒకవేళ (శీర్షిక_threshhold) {
				రాయి(fmt, *argv);
			}
			ఒకవేళ (negative_N) {
				ఒకవేళ (లెక్క_బైట్లు) {
					print_except_N_చివరి_బైట్లు(fp, లెక్క);
				} లేకపోతే {
					print_except_N_చివరి_పంక్తులు(fp, లెక్క);
				}
			} లేకపోతే {
				print_మొదటి_N(fp, లెక్క, లెక్క_బైట్లు);
			}
			die_if_ferror_stdout();
			ఒకవేళ (fclose_if_not_stdin(fp)) {
				bb_simple_perror_సందేశం(*argv);
				retval = EXIT_FAILURE;
			}
		} లేకపోతే {
			retval = EXIT_FAILURE;
		}
		fmt = శీర్షిక_fmt_str;
	} అయితే (*++argv);

	fflush_stdout_and_exit(retval);
}
