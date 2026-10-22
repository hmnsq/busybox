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

#شامل "libbb.h"
#شامل "unicode.h"

#اگر !ENABLE_LOCALE_SUPPORT
# undef isprint
# undef خلا_ہے
# define isprint(c) ((غیرعلامتی)((c) - 0x20) <= (0x7e - 0x20))
# define خلا_ہے(c) ((c) == ' ')
#اگر_ختم

#اگر ENABLE_FEATURE_WC_LARGE
# define گنتی_T غیرعلامتی طویل طویل
# define گنتی_FMT "llu"
#ورنہ
# define گنتی_T غیرعلامتی
# define گنتی_FMT "u"
#اگر_ختم

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
شمار {
	WC_سطریں    = 0, /* -l */
	WC_الفاظ    = 1, /* -w */
	WC_UNICHARS = 2, /* -m */
	WC_بائٹیں    = 3, /* -c */
	WC_لمبائی   = 4, /* -L */
	عدد_WCS     = 5,
};

صحیح wc_main(صحیح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح wc_main(صحیح argc غیر_مستعمل_PARAM, حرف **argv)
{
	ثابت حرف *arg;
	ثابت حرف *آغاز_fmt = " %9"گنتی_FMT + 1;
	ثابت حرف *fname_fmt = " %s\n";
	گنتی_T *pcounts;
	گنتی_T counts[عدد_WCS];
	گنتی_T totals[عدد_WCS];
	صحیح عدد_files;
	smallint حالت_2 = EXIT_SUCCESS;
	غیرعلامتی print_type;

	init_unicode();

	print_type = getopt32(argv, "lwmcL");

	اگر (print_type == 0) {
		print_type = (1 << WC_سطریں) | (1 << WC_الفاظ) | (1 << WC_بائٹیں);
	}

	argv += optind;
	اگر (!argv[0]) {
		*--argv = (حرف *) bb_پیغام_standard_داخلہ;
		fname_fmt = "\n";
	}
	اگر (!argv[1]) { /* zero or one filename? */
		اگر (!((print_type-1) & print_type)) /* exactly one option? */
			آغاز_fmt = "%"گنتی_FMT;
	}

	memset(totals, 0, حجم(totals));

	pcounts = counts;

	عدد_files = 0;
	جب ((arg = *argv++) != تہی) {
		FILE *fp;
		ثابت حرف *s;
		غیرعلامتی u;
		غیرعلامتی linepos;
		smallint اندر_لفظ;

		++عدد_files;
		fp = fopen_or_warn_stdin(arg);
		اگر (!fp) {
			حالت_2 = EXIT_FAILURE;
			جاری;
		}

		memset(counts, 0, حجم(counts));
		linepos = 0;
		اندر_لفظ = 0;

		جب (1) {
			صحیح c;
			/* Our -w doesn't match GNU wc exactly... oh well */

			c = getc(fp);
			اگر (c == EOF) {
				اگر (ferror(fp)) {
					bb_simple_perror_پیغام(arg);
					حالت_2 = EXIT_FAILURE;
				}
				جاؤ DO_EOF;  /* Treat an EOF as '\r'. */
			}

			/* Cater for -c and -m */
			++counts[WC_بائٹیں];
			اگر (unicode_حالت != UNICODE_ON /* every byte is a new char */
			 || (c & 0xc0) != 0x80 /* it isn't a 2nd+ byte of a Unicode char */
			) {
				++counts[WC_UNICHARS];
			}

			اگر (isprint_asciionly(c)) { /* FIXME: not unicode-aware */
				++linepos;
				اگر (!خلا_ہے(c)) {
					اندر_لفظ = 1;
					جاری;
				}
			} ورنہ اگر ((غیرعلامتی)(c - 9) <= 4) {
				/* \t  9
				 * \n 10
				 * \v 11
				 * \f 12
				 * \r 13
				 */
				اگر (c == '\t') {
					linepos = (linepos | 7) + 1;
				} ورنہ {  /* '\n', '\r', '\f', or '\v' */
 DO_EOF:
					اگر (linepos > counts[WC_لمبائی]) {
						counts[WC_لمبائی] = linepos;
					}
					اگر (c == '\n') {
						++counts[WC_سطریں];
					}
					اگر (c != '\v') {
						linepos = 0;
					}
				}
			} ورنہ {
				جاری;
			}

			counts[WC_الفاظ] += اندر_لفظ;
			اندر_لفظ = 0;
			اگر (c == EOF) {
				توڑو;
			}
		}

		fclose_if_not_stdin(fp);

		اگر (totals[WC_لمبائی] < counts[WC_لمبائی]) {
			totals[WC_لمبائی] = counts[WC_لمبائی];
		}
		totals[WC_لمبائی] -= counts[WC_لمبائی];

 OUTPUT:
		/* coreutils wc tries hard to print pretty columns
		 * (saves results for all files, finds max col len etc...)
		 * we won't try that hard, it will bloat us too much */
		s = آغاز_fmt;
		u = 0;
		کرو {
			اگر (print_type & (1 << u)) {
				لکھو(s, pcounts[u]);
				s = " %9"گنتی_FMT; /* Ok... restore the leading space. */
			}
			totals[u] += pcounts[u];
		} جب (++u < عدد_WCS);
		لکھو(fname_fmt, arg);
	}

	/* If more than one file was processed, we want the totals.  To save some
	 * space, we set the pcounts ptr to the totals array.  This has the side
	 * effect of trashing the totals array after outputting it, but that's
	 * irrelavent since we no longer need it. */
	اگر (عدد_files > 1) {
		عدد_files = 0;  /* Make sure we don't get here again. */
		arg = "total";
		pcounts = totals;
		--argv;
		جاؤ OUTPUT;
	}

	fflush_stdout_and_exit(حالت_2);
}
