/* vi: set sw=4 ts=4: */
/*
 * cat implementation for busybox
 *
 * Copyright (C) 2003  Manuel Novoa III  <mjn3@codepoet.org>
 *
 * Licensed under GPLv2, see file LICENSE in this source tree.
 */
//config:config CAT
//config:	bool "cat (5.8 kb)"
//config:	default y
//config:	help
//config:	cat is used to concatenate files and print them to the standard
//config:	output. Enable this option if you wish to enable the 'cat' utility.
//config:
//config:config FEATURE_CATN
//config:	bool "Enable -n and -b options"
//config:	default y
//config:	depends on CAT
//config:	help
//config:	-n numbers all output lines while -b numbers nonempty output lines.
//config:
//config:config FEATURE_CATV
//config:	bool "cat -v[etA]"
//config:	default y
//config:	depends on CAT
//config:	help
//config:	Display nonprinting characters as escape sequences

//applet:IF_CAT(APPLET(cat, BB_DIR_BIN, BB_SUID_DROP))

//kbuild:lib-$(CONFIG_CAT) += cat.o

/* BB_AUDIT SUSv3 compliant */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/cat.html */

//usage:#if ENABLE_FEATURE_CATN || ENABLE_FEATURE_CATV
//usage:#define cat_trivial_usage
//usage:       "[-" IF_FEATURE_CATN("nb") IF_FEATURE_CATV("vteA") "] [FILE]..."
//usage:#else
//usage:#define cat_trivial_usage
//usage:       "[FILE]..."
//usage:#endif
//usage:#define cat_full_usage "\n\n"
//usage:       "Print FILEs to stdout\n"
//usage:	IF_FEATURE_CATN(
//usage:     "\n	-n	Number output lines"
//usage:     "\n	-b	Number nonempty lines"
//usage:	)
//usage:	IF_FEATURE_CATV(
//usage:     "\n	-v	Show nonprinting characters as ^x or M-x"
//usage:     "\n	-t	...and tabs as ^I"
//usage:     "\n	-e	...and end lines with $"
//usage:     "\n	-A	Same as -vte"
//usage:	)
/*
  Longopts not implemented yet:
      --number-nonblank    number nonempty output lines, overrides -n
      --number             number all output lines
      --show-nonprinting   use ^ and M- notation, except for LFD and TAB
      --show-all           equivalent to -vet
  Not implemented yet:
  -E, --show-ends          display $ at end of each line (-e sans -v)
  -T, --show-tabs          display TAB characters as ^I (-t sans -v)
  -s, --squeeze-blank      suppress repeated empty output lines
*/
//usage:
//usage:#define cat_example_usage
//usage:       "$ cat /proc/uptime\n"
//usage:       "110716.72 17.67"

#అన్తర్భావ "libbb.h"
#అన్తర్భావ "common_bufsiz.h"

#ఒకవేళ ENABLE_FEATURE_CATV
/*
 * cat -v implementation for busybox
 *
 * Copyright (C) 2006 Rob Landley <rob@landley.net>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
/* Rob had "cat -v" implemented as a separate applet, catv.
 * See "cat -v considered harmful" at
 * http://cm.bell-labs.com/cm/cs/doc/84/kp.ps.gz
 * From USENIX Summer Conference Proceedings, 1983
 * """
 * The talk reviews reasons for UNIX's popularity and shows, using UCB cat
 * as a primary example, how UNIX has grown fat. cat isn't for printing
 * files with line numbers, it isn't for compressing multiple blank lines,
 * it's not for looking at non-printing ASCII characters, it's for
 * concatenating files.
 * We are reminded that ls isn't the place for code to break a single column
 * into multiple ones, and that mailnews shouldn't have its own more
 * processing or joke encryption code.
 * """
 *
 * I agree with the argument. Unfortunately, this ship has sailed (1983...).
 * There are dozens of Linux distros and each of them has "cat" which supports -v.
 * It's unrealistic for us to "reeducate" them to use our, incompatible way
 * to achieve "cat -v" effect. The actual effect would be "users pissed off
 * by gratuitous incompatibility".
 */
#నిర్వచించు CAT_OPT_e (1<<0)
#నిర్వచించు CAT_OPT_t (1<<1)
#నిర్వచించు CAT_OPT_v (1<<2)
/* -A occupies bit (1<<3) */
#నిర్వచించు CAT_OPT_n ((1<<4) * ENABLE_FEATURE_CATN)
#నిర్వచించు CAT_OPT_b ((1<<5) * ENABLE_FEATURE_CATN)
స్థిర పూర్ణాంకం catv(గుర్తులేని opts, అక్షరం **argv)
{
	పూర్ణాంకం retval = EXIT_SUCCESS;
	పూర్ణాంకం fd;
#ఒకవేళ ENABLE_FEATURE_CATN
	bool eol_చూసిన = (opts & (CAT_OPT_n|CAT_OPT_b));
	గుర్తులేని eol_char = (eol_చూసిన ? '\n' : 0x100);
	గుర్తులేని skip_అంకె_on = (opts & CAT_OPT_b) ? '\n' : 0x100;
	గుర్తులేని lineno = 0;
#అయితే_ముగింపు

	BUILD_BUG_ON(CAT_OPT_e != VISIBLE_ENDLINE);
	BUILD_BUG_ON(CAT_OPT_t != VISIBLE_SHOW_TABS);
#ఒకవేళ 0 /* These consts match, we can just pass "opts" to visible() */
	ఒకవేళ (opts & CAT_OPT_e)
		flags |= VISIBLE_ENDLINE;
	ఒకవేళ (opts & CAT_OPT_t)
		flags |= VISIBLE_SHOW_TABS;
#అయితే_ముగింపు

#నిర్వచించు read_buf bb_common_bufsiz1
	అమరిక_common_bufsiz();
	చేయి {
		fd = open_or_warn_stdin(*argv);
		ఒకవేళ (fd < 0) {
			retval = EXIT_FAILURE;
			కొనసాగు;
		}
		కొరకు (;;) {
			పూర్ణాంకం i, res;

			res = పఠ(fd, read_buf, COMMON_BUFSIZE);
			ఒకవేళ (res < 0)
				retval = EXIT_FAILURE;
			ఒకవేళ (res <= 0)
				ఆపు;
			కొరకు (i = 0; i < res; i++) {
				గుర్తులేని అక్షరం c = read_buf[i];
				అక్షరం buf[పరిమాణం("M-^c")];
#ఒకవేళ ENABLE_FEATURE_CATN
				ఒకవేళ (eol_చూసిన && c != skip_అంకె_on)
					రాయి("%6u  ", ++lineno);
				eol_చూసిన = (c == eol_char);
#అయితే_ముగింపు
				visible(c, buf, opts);
				fputs_stdout(buf);
			}
		}
		ఒకవేళ (ENABLE_FEATURE_CLEAN_పైకి && fd)
			close(fd);
	} అయితే (*++argv);

	fflush_stdout_and_exit(retval);
}
#నిర్వచనం_రద్దు CAT_OPT_n
#నిర్వచనం_రద్దు CAT_OPT_b
#అయితే_ముగింపు

పూర్ణాంకం cat_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం cat_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
#ఒకవేళ ENABLE_FEATURE_CATV || ENABLE_FEATURE_CATN
	గుర్తులేని opts;

	opts =
#అయితే_ముగింపు
	getopt32(argv, IF_FEATURE_CATV("^")
		/* -u is ignored ("unbuffered") */
		IF_FEATURE_CATV("etvA")IF_FEATURE_CATN("nb")"u"
		IF_FEATURE_CATV("\0" "Aetv" /* -A == -vet */)
	);
	argv += optind;

	/* Read from stdin if there's nothing else to do. */
	ఒకవేళ (!argv[0])
		*--argv = (అక్షరం*)"-";

#ఒకవేళ ENABLE_FEATURE_CATV
	ఒకవేళ (opts & 7)
		తిరిగి catv(opts, argv);
	opts >>= 4;
#అయితే_ముగింపు

#ఒకవేళ ENABLE_FEATURE_CATN
# define CAT_OPT_n (1<<0)
# define CAT_OPT_b (1<<1)
	ఒకవేళ (opts & (CAT_OPT_n|CAT_OPT_b)) { /* -n or -b */
		నిర్మాణం సంఖ్య_state ns;
		పూర్ణాంకం exitcode;

		ns.వెడల్పు = 6;
		ns.ప్రారంభం = 1;
		ns.inc = 1;
		ns.sep = "\t";
		ns.ఖాళీలు_str = ఖాళీ;
		ns.all = !(opts & CAT_OPT_b); /* -n without -b */
		ns.nonempty = (opts & CAT_OPT_b); /* -b (with or without -n) */
		exitcode = EXIT_SUCCESS;
		చేయి {
			exitcode |= print_numbered_పంక్తులు(&ns, *argv);
		} అయితే (*++argv);
		fflush_stdout_and_exit(exitcode);
	}
	/*opts >>= 2;*/
#అయితే_ముగింపు

	తిరిగి bb_cat(argv);
}
