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

#ಅನ್ತರ್ಭಾವ "libbb.h"
#ಅನ್ತರ್ಭಾವ "unicode.h"

#ಆದರೆ !ENABLE_LOCALE_SUPPORT
# undef isprint
# undef ಖಾಲಿಯೇ
# define isprint(c) ((ಚಿಹ್ನೆರಹಿತ)((c) - 0x20) <= (0x7e - 0x20))
# define ಖಾಲಿಯೇ(c) ((c) == ' ')
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_FEATURE_WC_LARGE
# define ಎಣಿಕೆ_T ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ
# define ಎಣಿಕೆ_FMT "llu"
#ಇಲ್ಲದಿದ್ದರೆ
# define ಎಣಿಕೆ_T ಚಿಹ್ನೆರಹಿತ
# define ಎಣಿಕೆ_FMT "u"
#ಆದರೆ_ಅಂತ್ಯ

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
ಎಣಿಕೆ {
	WC_ಸಾಲುಗಳು    = 0, /* -l */
	WC_ಪದಗಳು    = 1, /* -w */
	WC_UNICHARS = 2, /* -m */
	WC_ಬೈಟುಗಳು    = 3, /* -c */
	WC_ಉದ್ದಳತೆ   = 4, /* -L */
	ಅಂಕೆ_WCS     = 5,
};

ಪೂರ್ಣಾಂಕ wc_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ wc_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *arg;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಆರಂಭ_fmt = " %9"ಎಣಿಕೆ_FMT + 1;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname_fmt = " %s\n";
	ಎಣಿಕೆ_T *pcounts;
	ಎಣಿಕೆ_T counts[ಅಂಕೆ_WCS];
	ಎಣಿಕೆ_T totals[ಅಂಕೆ_WCS];
	ಪೂರ್ಣಾಂಕ ಅಂಕೆ_files;
	smallint ಸ್ಥಿತಿ_2 = EXIT_SUCCESS;
	ಚಿಹ್ನೆರಹಿತ print_type;

	init_unicode();

	print_type = getopt32(argv, "lwmcL");

	ಆದರೆ (print_type == 0) {
		print_type = (1 << WC_ಸಾಲುಗಳು) | (1 << WC_ಪದಗಳು) | (1 << WC_ಬೈಟುಗಳು);
	}

	argv += optind;
	ಆದರೆ (!argv[0]) {
		*--argv = (ಅಕ್ಷರ *) bb_ಸಂದೇಶ_standard_ಒಳಹರಿವು;
		fname_fmt = "\n";
	}
	ಆದರೆ (!argv[1]) { /* zero or one filename? */
		ಆದರೆ (!((print_type-1) & print_type)) /* exactly one option? */
			ಆರಂಭ_fmt = "%"ಎಣಿಕೆ_FMT;
	}

	memset(totals, 0, ಗಾತ್ರ(totals));

	pcounts = counts;

	ಅಂಕೆ_files = 0;
	ಆಗ ((arg = *argv++) != ಖಾಲಿ) {
		FILE *fp;
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s;
		ಚಿಹ್ನೆರಹಿತ u;
		ಚಿಹ್ನೆರಹಿತ linepos;
		smallint ಒಳ_ಶಬ್ದ;

		++ಅಂಕೆ_files;
		fp = fopen_or_warn_stdin(arg);
		ಆದರೆ (!fp) {
			ಸ್ಥಿತಿ_2 = EXIT_FAILURE;
			ಮುಂದುವರಿ;
		}

		memset(counts, 0, ಗಾತ್ರ(counts));
		linepos = 0;
		ಒಳ_ಶಬ್ದ = 0;

		ಆಗ (1) {
			ಪೂರ್ಣಾಂಕ c;
			/* Our -w doesn't match GNU wc exactly... oh well */

			c = getc(fp);
			ಆದರೆ (c == EOF) {
				ಆದರೆ (ferror(fp)) {
					bb_simple_perror_ಸಂದೇಶ(arg);
					ಸ್ಥಿತಿ_2 = EXIT_FAILURE;
				}
				ಹೋಗು DO_EOF;  /* Treat an EOF as '\r'. */
			}

			/* Cater for -c and -m */
			++counts[WC_ಬೈಟುಗಳು];
			ಆದರೆ (unicode_ಸ್ಥಿತಿ != UNICODE_ON /* every byte is a new char */
			 || (c & 0xc0) != 0x80 /* it isn't a 2nd+ byte of a Unicode char */
			) {
				++counts[WC_UNICHARS];
			}

			ಆದರೆ (isprint_asciionly(c)) { /* FIXME: not unicode-aware */
				++linepos;
				ಆದರೆ (!ಖಾಲಿಯೇ(c)) {
					ಒಳ_ಶಬ್ದ = 1;
					ಮುಂದುವರಿ;
				}
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ)(c - 9) <= 4) {
				/* \t  9
				 * \n 10
				 * \v 11
				 * \f 12
				 * \r 13
				 */
				ಆದರೆ (c == '\t') {
					linepos = (linepos | 7) + 1;
				} ಇಲ್ಲದಿದ್ದರೆ {  /* '\n', '\r', '\f', or '\v' */
 DO_EOF:
					ಆದರೆ (linepos > counts[WC_ಉದ್ದಳತೆ]) {
						counts[WC_ಉದ್ದಳತೆ] = linepos;
					}
					ಆದರೆ (c == '\n') {
						++counts[WC_ಸಾಲುಗಳು];
					}
					ಆದರೆ (c != '\v') {
						linepos = 0;
					}
				}
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಮುಂದುವರಿ;
			}

			counts[WC_ಪದಗಳು] += ಒಳ_ಶಬ್ದ;
			ಒಳ_ಶಬ್ದ = 0;
			ಆದರೆ (c == EOF) {
				ನಿಲ್ಲಿಸು;
			}
		}

		fclose_if_not_stdin(fp);

		ಆದರೆ (totals[WC_ಉದ್ದಳತೆ] < counts[WC_ಉದ್ದಳತೆ]) {
			totals[WC_ಉದ್ದಳತೆ] = counts[WC_ಉದ್ದಳತೆ];
		}
		totals[WC_ಉದ್ದಳತೆ] -= counts[WC_ಉದ್ದಳತೆ];

 OUTPUT:
		/* coreutils wc tries hard to print pretty columns
		 * (saves results for all files, finds max col len etc...)
		 * we won't try that hard, it will bloat us too much */
		s = ಆರಂಭ_fmt;
		u = 0;
		ಮಾಡು {
			ಆದರೆ (print_type & (1 << u)) {
				ಬರೆ(s, pcounts[u]);
				s = " %9"ಎಣಿಕೆ_FMT; /* Ok... restore the leading space. */
			}
			totals[u] += pcounts[u];
		} ಆಗ (++u < ಅಂಕೆ_WCS);
		ಬರೆ(fname_fmt, arg);
	}

	/* If more than one file was processed, we want the totals.  To save some
	 * space, we set the pcounts ptr to the totals array.  This has the side
	 * effect of trashing the totals array after outputting it, but that's
	 * irrelavent since we no longer need it. */
	ಆದರೆ (ಅಂಕೆ_files > 1) {
		ಅಂಕೆ_files = 0;  /* Make sure we don't get here again. */
		arg = "total";
		pcounts = totals;
		--argv;
		ಹೋಗು OUTPUT;
	}

	fflush_stdout_and_exit(ಸ್ಥಿತಿ_2);
}
