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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "unicode.h"

#ਜੇ !ENABLE_LOCALE_SUPPORT
# undef isprint
# undef ਖਾਲੀ_ਥਾਂ_ਹੈ
# define isprint(c) ((ਬਿਨਾਨਿਸ਼ਾਨੀ)((c) - 0x20) <= (0x7e - 0x20))
# define ਖਾਲੀ_ਥਾਂ_ਹੈ(c) ((c) == ' ')
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_FEATURE_WC_LARGE
# define ਗਿਣਤੀ_T ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ
# define ਗਿਣਤੀ_FMT "llu"
#ਨਹੀਂਤਾਂ
# define ਗਿਣਤੀ_T ਬਿਨਾਨਿਸ਼ਾਨੀ
# define ਗਿਣਤੀ_FMT "u"
#ਜੇ_ਅੰਤ

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
ਗਿਣਤੀ {
	WC_ਸਤਰਾਂ    = 0, /* -l */
	WC_ਸ਼ਬਦ    = 1, /* -w */
	WC_UNICHARS = 2, /* -m */
	WC_ਬਾਈਟਾਂ    = 3, /* -c */
	WC_ਲੰਮਾਈ   = 4, /* -L */
	ਅੰਕ_WCS     = 5,
};

ਪੂਰਨ wc_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ wc_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
	ਅਚਲ ਅੱਖਰ *arg;
	ਅਚਲ ਅੱਖਰ *ਸ਼ੁਰੂ_fmt = " %9"ਗਿਣਤੀ_FMT + 1;
	ਅਚਲ ਅੱਖਰ *fname_fmt = " %s\n";
	ਗਿਣਤੀ_T *pcounts;
	ਗਿਣਤੀ_T counts[ਅੰਕ_WCS];
	ਗਿਣਤੀ_T totals[ਅੰਕ_WCS];
	ਪੂਰਨ ਅੰਕ_files;
	smallint ਸਥਿਤੀ = EXIT_SUCCESS;
	ਬਿਨਾਨਿਸ਼ਾਨੀ print_type;

	init_unicode();

	print_type = getopt32(argv, "lwmcL");

	ਜੇ (print_type == 0) {
		print_type = (1 << WC_ਸਤਰਾਂ) | (1 << WC_ਸ਼ਬਦ) | (1 << WC_ਬਾਈਟਾਂ);
	}

	argv += optind;
	ਜੇ (!argv[0]) {
		*--argv = (ਅੱਖਰ *) bb_ਸੁਨੇਹਾ_standard_ਨਿਵੇਸ਼;
		fname_fmt = "\n";
	}
	ਜੇ (!argv[1]) { /* zero or one filename? */
		ਜੇ (!((print_type-1) & print_type)) /* exactly one option? */
			ਸ਼ੁਰੂ_fmt = "%"ਗਿਣਤੀ_FMT;
	}

	memset(totals, 0, ਆਕਾਰ(totals));

	pcounts = counts;

	ਅੰਕ_files = 0;
	ਜਦੋਂ ((arg = *argv++) != ਨਿਰਾ) {
		FILE *fp;
		ਅਚਲ ਅੱਖਰ *s;
		ਬਿਨਾਨਿਸ਼ਾਨੀ u;
		ਬਿਨਾਨਿਸ਼ਾਨੀ linepos;
		smallint ਵਿੱਚ_ਸ਼ਬਦ;

		++ਅੰਕ_files;
		fp = fopen_or_warn_stdin(arg);
		ਜੇ (!fp) {
			ਸਥਿਤੀ = EXIT_FAILURE;
			ਜਾਰੀ;
		}

		memset(counts, 0, ਆਕਾਰ(counts));
		linepos = 0;
		ਵਿੱਚ_ਸ਼ਬਦ = 0;

		ਜਦੋਂ (1) {
			ਪੂਰਨ c;
			/* Our -w doesn't match GNU wc exactly... oh well */

			c = getc(fp);
			ਜੇ (c == EOF) {
				ਜੇ (ferror(fp)) {
					bb_simple_perror_ਸੁਨੇਹਾ(arg);
					ਸਥਿਤੀ = EXIT_FAILURE;
				}
				ਜਾਓ DO_EOF;  /* Treat an EOF as '\r'. */
			}

			/* Cater for -c and -m */
			++counts[WC_ਬਾਈਟਾਂ];
			ਜੇ (unicode_ਸਥਿਤੀ != UNICODE_ON /* every byte is a new char */
			 || (c & 0xc0) != 0x80 /* it isn't a 2nd+ byte of a Unicode char */
			) {
				++counts[WC_UNICHARS];
			}

			ਜੇ (isprint_asciionly(c)) { /* FIXME: not unicode-aware */
				++linepos;
				ਜੇ (!ਖਾਲੀ_ਥਾਂ_ਹੈ(c)) {
					ਵਿੱਚ_ਸ਼ਬਦ = 1;
					ਜਾਰੀ;
				}
			} ਨਹੀਂਤਾਂ ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ)(c - 9) <= 4) {
				/* \t  9
				 * \n 10
				 * \v 11
				 * \f 12
				 * \r 13
				 */
				ਜੇ (c == '\t') {
					linepos = (linepos | 7) + 1;
				} ਨਹੀਂਤਾਂ {  /* '\n', '\r', '\f', or '\v' */
 DO_EOF:
					ਜੇ (linepos > counts[WC_ਲੰਮਾਈ]) {
						counts[WC_ਲੰਮਾਈ] = linepos;
					}
					ਜੇ (c == '\n') {
						++counts[WC_ਸਤਰਾਂ];
					}
					ਜੇ (c != '\v') {
						linepos = 0;
					}
				}
			} ਨਹੀਂਤਾਂ {
				ਜਾਰੀ;
			}

			counts[WC_ਸ਼ਬਦ] += ਵਿੱਚ_ਸ਼ਬਦ;
			ਵਿੱਚ_ਸ਼ਬਦ = 0;
			ਜੇ (c == EOF) {
				ਰੋਕੋ;
			}
		}

		fclose_if_not_stdin(fp);

		ਜੇ (totals[WC_ਲੰਮਾਈ] < counts[WC_ਲੰਮਾਈ]) {
			totals[WC_ਲੰਮਾਈ] = counts[WC_ਲੰਮਾਈ];
		}
		totals[WC_ਲੰਮਾਈ] -= counts[WC_ਲੰਮਾਈ];

 OUTPUT:
		/* coreutils wc tries hard to print pretty columns
		 * (saves results for all files, finds max col len etc...)
		 * we won't try that hard, it will bloat us too much */
		s = ਸ਼ੁਰੂ_fmt;
		u = 0;
		ਕਰੋ {
			ਜੇ (print_type & (1 << u)) {
				ਲਿਖੋ(s, pcounts[u]);
				s = " %9"ਗਿਣਤੀ_FMT; /* Ok... restore the leading space. */
			}
			totals[u] += pcounts[u];
		} ਜਦੋਂ (++u < ਅੰਕ_WCS);
		ਲਿਖੋ(fname_fmt, arg);
	}

	/* If more than one file was processed, we want the totals.  To save some
	 * space, we set the pcounts ptr to the totals array.  This has the side
	 * effect of trashing the totals array after outputting it, but that's
	 * irrelavent since we no longer need it. */
	ਜੇ (ਅੰਕ_files > 1) {
		ਅੰਕ_files = 0;  /* Make sure we don't get here again. */
		arg = "total";
		pcounts = totals;
		--argv;
		ਜਾਓ OUTPUT;
	}

	fflush_stdout_and_exit(ਸਥਿਤੀ);
}
