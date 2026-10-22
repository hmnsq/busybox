/* vi: set sw=4 ts=4: */
/*
 * tac implementation for busybox
 * tac - concatenate and print files in reverse
 *
 * Copyright (C) 2003  Yang Xiaopeng  <yxp at hanwang.com.cn>
 * Copyright (C) 2007  Natanael Copa  <natanael.copa@gmail.com>
 * Copyright (C) 2007  Tito Ragusa    <farmatito@tiscali.it>
 *
 * Licensed under GPLv2, see file LICENSE in this source tree.
 */
/* Based on Yang Xiaopeng's (yxp at hanwang.com.cn) patch
 * http://www.uclibc.org/lists/busybox/2003-July/008813.html
 */
//config:config TAC
//config:	bool "tac (4.1 kb)"
//config:	default y
//config:	help
//config:	tac is used to concatenate and print files in reverse.

//applet:IF_TAC(APPLET_NOEXEC(tac, tac, BB_DIR_USR_BIN, BB_SUID_DROP, tac))

//kbuild:lib-$(CONFIG_TAC) += tac.o

//usage:#define tac_trivial_usage
//usage:	"[FILE]..."
//usage:#define tac_full_usage "\n\n"
//usage:	"Concatenate FILEs and print them in reverse"

#అన్తర్భావ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

నిర్మాణం lstring {
	పూర్ణాంకం కొలత;
	అక్షరం buf[1];
};

పూర్ణాంకం tac_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం tac_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	అక్షరం **name;
	FILE *f;
	నిర్మాణం lstring *పంక్తి = ఖాళీ;
	llist_t *list = ఖాళీ;
	పూర్ణాంకం retval = EXIT_SUCCESS;

#ఒకవేళ ENABLE_DESKTOP
/* tac from coreutils 6.9 supports:
       -b, --before
              attach the separator before instead of after
       -r, --regex
              interpret the separator as a regular expression
       -s, --separator=STRING
              use STRING as the separator instead of newline
We support none, but at least we will complain or handle "--":
*/
	getopt32(argv, "");
	argv += optind;
#లేకపోతే
	argv++;
#అయితే_ముగింపు
	ఒకవేళ (!*argv)
		*--argv = (అక్షరం *)"-";
	/* We will read from last file to first */
	name = argv;
	అయితే (*name)
		name++;

	చేయి {
		పూర్ణాంకం ch, i;

		name--;
		f = fopen_or_warn_stdin(*name);
		ఒకవేళ (f == ఖాళీ) {
			/* error message is printed by fopen_or_warn_stdin */
			retval = EXIT_FAILURE;
			కొనసాగు;
		}

		errno = i = 0;
		చేయి {
			ch = fgetc(f);
			ఒకవేళ (ch != EOF) {
				ఒకవేళ (!(i & 0x7f))
					/* Grow on every 128th char */
					పంక్తి = xrealloc(పంక్తి, i + 0x7f + పరిమాణం(పూర్ణాంకం) + 1);
				పంక్తి->buf[i++] = ch;
			}
			ఒకవేళ (ch == '\n' || (ch == EOF && i != 0)) {
				పంక్తి = xrealloc(పంక్తి, i + పరిమాణం(పూర్ణాంకం));
				పంక్తి->కొలత = i;
				llist_కూడు_గమ్యం(&list, పంక్తి);
				పంక్తి = ఖాళీ;
				i = 0;
			}
		} అయితే (ch != EOF);
		/* fgetc sets errno to ENOENT on EOF, we don't want
		 * to warn on this non-error! */
		ఒకవేళ (errno && errno != ENOENT) {
			bb_simple_perror_సందేశం(*name);
			retval = EXIT_FAILURE;
		}
	} అయితే (name != argv);

	అయితే (list) {
		పంక్తి = (నిర్మాణం lstring *)list->దత్తాంశం;
		xwrite(STDOUT_FILENO, పంక్తి->buf, పంక్తి->కొలత);
		ఒకవేళ (ENABLE_FEATURE_CLEAN_పైకి) {
			విడుదల(llist_తీయి(&list));
		} లేకపోతే {
			list = list->link;
		}
	}

	తిరిగి retval;
}
