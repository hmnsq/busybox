/* vi: set sw=4 ts=4: */
/*
 * wc implementation for busybox
 *
 * Copyright (C) 2003  Manuel Novoa III  <mjn3@codepoet.org>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
/* Mar 16, 2003      Manuel Novoa III   (mjn3@codepoet.org)
 *
 * Rewritten to fix a number of problems and do some size optimizations.
 * Problems in the previous busybox implementation (besides bloat) included:
 *  1) broken 'wc -c' optimization (read note below)
 *  2) broken handling of '-' args
 *  3) no checking of ferror on EOF returns
 *  4) isprint() wasn't considered when word counting.
 *
 * NOTES:
 *
 * The previous busybox wc attempted an optimization using stat for the
 * case of counting chars only.  I omitted that because it was broken.
 * It didn't take into account the possibility of input coming from a
 * pipe, or input from a file with file pointer not at the beginning.
 *
 * To implement such a speed optimization correctly, not only do you
 * need the size, but also the file position.  Note also that the
 * file position may be past the end of file.  Consider the example
 * (adapted from example in gnu wc.c)
 *
 *      echo hello > /tmp/testfile &&
 *      (dd ibs=1k skip=1 count=0 &> /dev/null; wc -c) < /tmp/testfile
 *
 * for which 'wc -c' should output '0'.
 */
//config:config WC
//config:	bool "wc (4.7 kb)"
//config:	default y
//config:	help
//config:	wc is used to print the number of bytes, words, and lines,
//config:	in specified files.
//config:
//config:config FEATURE_WC_LARGE
//config:	bool "Support very large counts"
//config:	default y
//config:	depends on WC
//config:	help
//config:	Use "unsigned long long" for counter variables.

//applet:IF_WC(APPLET(wc, BB_DIR_USR_BIN, BB_SUID_DROP))

//kbuild:lib-$(CONFIG_WC) += wc.o

/* BB_AUDIT SUSv3 compliant. */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/wc.html */

#అన్తర్భావ "libbb.h"
#అన్తర్భావ "unicode.h"

#ఒకవేళ !ENABLE_LOCALE_SUPPORT
# undef isprint
# undef ఖాళీనా
# define isprint(c) ((గుర్తులేని)((c) - 0x20) <= (0x7e - 0x20))
# define ఖాళీనా(c) ((c) == ' ')
#అయితే_ముగింపు

#ఒకవేళ ENABLE_FEATURE_WC_LARGE
# define లెక్క_T గుర్తులేని దీర్ఘ దీర్ఘ
# define లెక్క_FMT "llu"
#లేకపోతే
# define లెక్క_T గుర్తులేని
# define లెక్క_FMT "u"
#అయితే_ముగింపు

/* We support -m even when UNICODE_SUPPORT is off,
 * we just don't advertise it in help text,
 * since it is the same as -c in this case.
 */

//usage:#define wc_trivial_usage
//usage:       "[-c"IF_UNICODE_SUPPORT("m")"lwL] [FILE]..."
//usage:
//usage:#define wc_full_usage "\n\n"
//usage:       "Count lines, words, and bytes for FILEs (or stdin)\n"
//usage:     "\n	-c	Count bytes"
//usage:	IF_UNICODE_SUPPORT(
//usage:     "\n	-m	Count characters"
//usage:	)
//usage:     "\n	-l	Count newlines"
//usage:     "\n	-w	Count words"
//usage:     "\n	-L	Print longest line length"
//usage:
//usage:#define wc_example_usage
//usage:       "$ wc /etc/passwd\n"
//usage:       "     31      46    1365 /etc/passwd\n"

/* Order is important if we want to be compatible with
 * column order in "wc -cmlwL" output:
 */
గణన {
	WC_పంక్తులు    = 0, /* -l */
	WC_పదాలు    = 1, /* -w */
	WC_UNICHARS = 2, /* -m */
	WC_బైట్లు    = 3, /* -c */
	WC_నిడివి   = 4, /* -L */
	అంకె_WCS     = 5,
};

పూర్ణాంకం wc_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం wc_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	స్థిరరాశి అక్షరం *arg;
	స్థిరరాశి అక్షరం *ప్రారంభం_fmt = " %9"లెక్క_FMT + 1;
	స్థిరరాశి అక్షరం *fname_fmt = " %s\n";
	లెక్క_T *pcounts;
	లెక్క_T counts[అంకె_WCS];
	లెక్క_T totals[అంకె_WCS];
	పూర్ణాంకం అంకె_files;
	smallint స్థితి_2 = EXIT_SUCCESS;
	గుర్తులేని print_type;

	init_unicode();

	print_type = getopt32(argv, "lwmcL");

	ఒకవేళ (print_type == 0) {
		print_type = (1 << WC_పంక్తులు) | (1 << WC_పదాలు) | (1 << WC_బైట్లు);
	}

	argv += optind;
	ఒకవేళ (!argv[0]) {
		*--argv = (అక్షరం *) bb_సందేశం_standard_ఆదానం;
		fname_fmt = "\n";
	}
	ఒకవేళ (!argv[1]) { /* zero or one filename? */
		ఒకవేళ (!((print_type-1) & print_type)) /* exactly one option? */
			ప్రారంభం_fmt = "%"లెక్క_FMT;
	}

	memset(totals, 0, పరిమాణం(totals));

	pcounts = counts;

	అంకె_files = 0;
	అయితే ((arg = *argv++) != ఖాళీ) {
		FILE *fp;
		స్థిరరాశి అక్షరం *s;
		గుర్తులేని u;
		గుర్తులేని linepos;
		smallint లోపల_మాట;

		++అంకె_files;
		fp = fopen_or_warn_stdin(arg);
		ఒకవేళ (!fp) {
			స్థితి_2 = EXIT_FAILURE;
			కొనసాగు;
		}

		memset(counts, 0, పరిమాణం(counts));
		linepos = 0;
		లోపల_మాట = 0;

		అయితే (1) {
			పూర్ణాంకం c;
			/* Our -w doesn't match GNU wc exactly... oh well */

			c = getc(fp);
			ఒకవేళ (c == EOF) {
				ఒకవేళ (ferror(fp)) {
					bb_simple_perror_సందేశం(arg);
					స్థితి_2 = EXIT_FAILURE;
				}
				వెళ్ళు DO_EOF;  /* Treat an EOF as '\r'. */
			}

			/* Cater for -c and -m */
			++counts[WC_బైట్లు];
			ఒకవేళ (unicode_స్థితి != UNICODE_ON /* every byte is a new char */
			 || (c & 0xc0) != 0x80 /* it isn't a 2nd+ byte of a Unicode char */
			) {
				++counts[WC_UNICHARS];
			}

			ఒకవేళ (isprint_asciionly(c)) { /* FIXME: not unicode-aware */
				++linepos;
				ఒకవేళ (!ఖాళీనా(c)) {
					లోపల_మాట = 1;
					కొనసాగు;
				}
			} లేకపోతే ఒకవేళ ((గుర్తులేని)(c - 9) <= 4) {
				/* \t  9
				 * \n 10
				 * \v 11
				 * \f 12
				 * \r 13
				 */
				ఒకవేళ (c == '\t') {
					linepos = (linepos | 7) + 1;
				} లేకపోతే {  /* '\n', '\r', '\f', or '\v' */
 DO_EOF:
					ఒకవేళ (linepos > counts[WC_నిడివి]) {
						counts[WC_నిడివి] = linepos;
					}
					ఒకవేళ (c == '\n') {
						++counts[WC_పంక్తులు];
					}
					ఒకవేళ (c != '\v') {
						linepos = 0;
					}
				}
			} లేకపోతే {
				కొనసాగు;
			}

			counts[WC_పదాలు] += లోపల_మాట;
			లోపల_మాట = 0;
			ఒకవేళ (c == EOF) {
				ఆపు;
			}
		}

		fclose_if_not_stdin(fp);

		ఒకవేళ (totals[WC_నిడివి] < counts[WC_నిడివి]) {
			totals[WC_నిడివి] = counts[WC_నిడివి];
		}
		totals[WC_నిడివి] -= counts[WC_నిడివి];

 OUTPUT:
		/* coreutils wc tries hard to print pretty columns
		 * (saves results for all files, finds max col len etc...)
		 * we won't try that hard, it will bloat us too much */
		s = ప్రారంభం_fmt;
		u = 0;
		చేయి {
			ఒకవేళ (print_type & (1 << u)) {
				రాయి(s, pcounts[u]);
				s = " %9"లెక్క_FMT; /* Ok... restore the leading space. */
			}
			totals[u] += pcounts[u];
		} అయితే (++u < అంకె_WCS);
		రాయి(fname_fmt, arg);
	}

	/* If more than one file was processed, we want the totals.  To save some
	 * space, we set the pcounts ptr to the totals array.  This has the side
	 * effect of trashing the totals array after outputting it, but that's
	 * irrelavent since we no longer need it. */
	ఒకవేళ (అంకె_files > 1) {
		అంకె_files = 0;  /* Make sure we don't get here again. */
		arg = "total";
		pcounts = totals;
		--argv;
		వెళ్ళు OUTPUT;
	}

	fflush_stdout_and_exit(స్థితి_2);
}
