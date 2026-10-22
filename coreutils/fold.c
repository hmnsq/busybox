/* vi: set sw=4 ts=4: */
/*
 * fold -- wrap each input line to fit in specified width.
 *
 * Written by David MacKenzie, djm@gnu.ai.mit.edu.
 * Copyright (C) 91, 1995-2002 Free Software Foundation, Inc.
 *
 * Modified for busybox based on coreutils v 5.0
 * Copyright (C) 2003 Glenn McGrath
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config FOLD
//config:	bool "fold (4.8 kb)"
//config:	default y
//config:	help
//config:	Wrap text to fit a specific width.

//applet:IF_FOLD(APPLET_NOEXEC(fold, fold, BB_DIR_USR_BIN, BB_SUID_DROP, fold))

//kbuild:lib-$(CONFIG_FOLD) += fold.o

//usage:#define fold_trivial_usage
//usage:       "[-bs] [-w WIDTH] [FILE]..."
//usage:#define fold_full_usage "\n\n"
//usage:       "Wrap input lines in FILEs (or stdin), writing to stdout\n"
//usage:     "\n	-b	Count bytes rather than columns"
//usage:     "\n	-s	Break at spaces"
//usage:     "\n	-w	Use WIDTH columns instead of 80"

#అన్తర్భావ "libbb.h"
#అన్తర్భావ "unicode.h"

/* This is a NOEXEC applet. Be very careful! */

/* Must match getopt32 call */
#నిర్వచించు FLAG_లెక్క_బైట్లు        1
#నిర్వచించు FLAG_BREAK_SPACES       2
#నిర్వచించు FLAG_వెడల్పు              4

/* Assuming the current column is COLUMN, return the column that
   printing C will move the cursor to.
   The first column is 0. */
స్థిర పూర్ణాంకం adjust_స్తంభం(గుర్తులేని స్తంభం, అక్షరం c)
{
	ఒకవేళ (option_mask32 & FLAG_లెక్క_బైట్లు)
		తిరిగి ++స్తంభం;

	ఒకవేళ (c == '\t')
		తిరిగి స్తంభం + 8 - స్తంభం % 8;

	ఒకవేళ (c == '\b') {
		ఒకవేళ ((పూర్ణాంకం)--స్తంభం < 0)
			స్తంభం = 0;
	}
	లేకపోతే ఒకవేళ (c == '\r')
		స్తంభం = 0;
	లేకపోతే { /* just a printable char */
		ఒకవేళ (unicode_స్థితి != UNICODE_ON /* every byte is a new char */
		 || (c & 0xc0) != 0x80 /* it isn't a 2nd+ byte of a Unicode char */
		) {
			స్తంభం++;
		}
	}
	తిరిగి స్తంభం;
}

/* Note that this function can write NULs, unlike fputs etc. */
స్థిర శూన్య write2stdout(స్థిరరాశి శూన్య *buf, గుర్తులేని కొలత)
{
	fwrite(buf, 1, కొలత, stdout);
}

పూర్ణాంకం fold_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం fold_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	అక్షరం *పంక్తి_బహిర్గతం = ఖాళీ;
	స్థిరరాశి అక్షరం *w_opt = "80";
	గుర్తులేని వెడల్పు;
	exitcode_t exitcode = EXIT_SUCCESS;

	init_unicode();

	ఒకవేళ (ENABLE_INCLUDE_SUSv2) {
		/* Turn any numeric options into -w options.  */
		పూర్ణాంకం i;
		కొరకు (i = 1; argv[i]; i++) {
			స్థిరరాశి అక్షరం *a = argv[i];
			ఒకవేళ (*a == '-') {
				a++;
				ఒకవేళ (*a == '-' && !a[1]) /* "--" */
					ఆపు;
				ఒకవేళ (అంకెనా(*a))
					argv[i] = xasprintf("-w%s", a);
			}
		}
	}

	getopt32(argv, "bsw:", &w_opt);
	వెడల్పు = xatou_range(w_opt, 1, 10000);

	argv += optind;
	ఒకవేళ (!*argv)
		*--argv = (అక్షరం*)"-";

	చేయి {
		FILE *istream = fopen_or_warn_stdin(*argv);
		పూర్ణాంకం c;
		గుర్తులేని స్తంభం = 0;     /* Screen column where next char will go */
		గుర్తులేని స్థానభ్రంశం_బహిర్గతం = 0; /* Index in 'line_out' for next char */

		ఒకవేళ (istream == ఖాళీ) {
			exitcode = EXIT_FAILURE;
			కొనసాగు;
		}

		అయితే ((c = getc(istream)) != EOF) {
			/* We grow line_out in chunks of 0x1000 bytes */
			ఒకవేళ ((స్థానభ్రంశం_బహిర్గతం & 0xfff) == 0) {
				పంక్తి_బహిర్గతం = xrealloc(పంక్తి_బహిర్గతం, స్థానభ్రంశం_బహిర్గతం + 0x1000);
			}
 rescan:
			పంక్తి_బహిర్గతం[స్థానభ్రంశం_బహిర్గతం] = c;
			ఒకవేళ (c == '\n') {
				write2stdout(పంక్తి_బహిర్గతం, స్థానభ్రంశం_బహిర్గతం + 1);
				స్తంభం = స్థానభ్రంశం_బహిర్గతం = 0;
				కొనసాగు;
			}
			స్తంభం = adjust_స్తంభం(స్తంభం, c);
			ఒకవేళ (స్తంభం <= వెడల్పు || స్థానభ్రంశం_బహిర్గతం == 0) {
				/* offset_out == 0 case happens
				 * with small width (say, 1) and tabs.
				 * The very first tab already goes to column 8,
				 * but we must not wrap it */
				స్థానభ్రంశం_బహిర్గతం++;
				కొనసాగు;
			}

			/* This character would make the line too long.
			 * Print the line plus a newline, and make this character
			 * start the next line */
			ఒకవేళ (option_mask32 & FLAG_BREAK_SPACES) {
				గుర్తులేని i;
				గుర్తులేని logical_చివర;

				/* Look for the last blank. */
				కొరకు (logical_చివర = స్థానభ్రంశం_బహిర్గతం - 1; (పూర్ణాంకం)logical_చివర >= 0; logical_చివర--) {
					ఒకవేళ (!isblank(పంక్తి_బహిర్గతం[logical_చివర]))
						కొనసాగు;

					/* Found a space or tab.
					 * Output up to and including it, and start a new line */
					logical_చివర++;
					/*line_out[logical_end] = '\n'; - NO! this nukes one buffered character */
					write2stdout(పంక్తి_బహిర్గతం, logical_చివర);
					అక్షరం_రాయి('\n');
					/* Move the remainder to the beginning of the next line.
					 * The areas being copied here might overlap. */
					memmove(పంక్తి_బహిర్గతం, పంక్తి_బహిర్గతం + logical_చివర, స్థానభ్రంశం_బహిర్గతం - logical_చివర);
					స్థానభ్రంశం_బహిర్గతం -= logical_చివర;
					కొరకు (స్తంభం = i = 0; i < స్థానభ్రంశం_బహిర్గతం; i++) {
						స్తంభం = adjust_స్తంభం(స్తంభం, పంక్తి_బహిర్గతం[i]);
					}
					వెళ్ళు rescan;
				}
				/* No blank found, wrap will split the overlong word */
			}
			/* Output what we accumulated up to now, and start a new line */
			పంక్తి_బహిర్గతం[స్థానభ్రంశం_బహిర్గతం] = '\n';
			write2stdout(పంక్తి_బహిర్గతం, స్థానభ్రంశం_బహిర్గతం + 1);
			స్తంభం = స్థానభ్రంశం_బహిర్గతం = 0;
			వెళ్ళు rescan;
		} /* while (not EOF) */

		ఒకవేళ (స్థానభ్రంశం_బహిర్గతం) {
			write2stdout(పంక్తి_బహిర్గతం, స్థానభ్రంశం_బహిర్గతం);
		}

		ఒకవేళ (fclose_if_not_stdin(istream)) {
			bb_simple_perror_సందేశం(*argv);
			exitcode = EXIT_FAILURE;
		}
	} అయితే (*++argv);

	fflush_stdout_and_exit(exitcode);
}
