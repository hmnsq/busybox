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

#ضمن "libbb.h"
#ضمن "unicode.h"

#إذا !ENABLE_LOCALE_SUPPORT
# undef isprint
# undef خال_آهي
# define isprint(c) ((بنانشان)((c) - 0x20) <= (0x7e - 0x20))
# define خال_آهي(c) ((c) == ' ')
#جيڪڏهن_پورو

#إذا ENABLE_FEATURE_WC_LARGE
# define ڳڻپ_T بنانشان ڊگهو ڊگهو
# define ڳڻپ_FMT "llu"
#وإلا
# define ڳڻپ_T بنانشان
# define ڳڻپ_FMT "u"
#جيڪڏهن_پورو

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
تعداد {
	WC_سٽون    = 0, /* -l */
	WC_لفظ    = 1, /* -w */
	WC_UNICHARS = 2, /* -m */
	WC_بائيٽون    = 3, /* -c */
	WC_ڊيگهائي   = 4, /* -L */
	انگ_WCS     = 5,
};

صحيح wc_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح wc_main(صحيح argc اڻ_استعمال_ٿيل_PARAM, حرف **argv)
{
	ثابت حرف *arg;
	ثابت حرف *شروعات_fmt = " %9"ڳڻپ_FMT + 1;
	ثابت حرف *fname_fmt = " %s\n";
	ڳڻپ_T *pcounts;
	ڳڻپ_T counts[انگ_WCS];
	ڳڻپ_T totals[انگ_WCS];
	صحيح انگ_files;
	smallint حالت = EXIT_SUCCESS;
	بنانشان print_type;

	init_unicode();

	print_type = getopt32(argv, "lwmcL");

	إذا (print_type == 0) {
		print_type = (1 << WC_سٽون) | (1 << WC_لفظ) | (1 << WC_بائيٽون);
	}

	argv += optind;
	إذا (!argv[0]) {
		*--argv = (حرف *) bb_نياپو_standard_داخلا;
		fname_fmt = "\n";
	}
	إذا (!argv[1]) { /* zero or one filename? */
		إذا (!((print_type-1) & print_type)) /* exactly one option? */
			شروعات_fmt = "%"ڳڻپ_FMT;
	}

	memset(totals, 0, حجم(totals));

	pcounts = counts;

	انگ_files = 0;
	طالما ((arg = *argv++) != عدم) {
		FILE *fp;
		ثابت حرف *s;
		بنانشان u;
		بنانشان linepos;
		smallint اندريون_لفظ;

		++انگ_files;
		fp = fopen_or_warn_stdin(arg);
		إذا (!fp) {
			حالت = EXIT_FAILURE;
			استمر;
		}

		memset(counts, 0, حجم(counts));
		linepos = 0;
		اندريون_لفظ = 0;

		طالما (1) {
			صحيح c;
			/* Our -w doesn't match GNU wc exactly... oh well */

			c = getc(fp);
			إذا (c == EOF) {
				إذا (ferror(fp)) {
					bb_simple_perror_نياپو(arg);
					حالت = EXIT_FAILURE;
				}
				اذهب DO_EOF;  /* Treat an EOF as '\r'. */
			}

			/* Cater for -c and -m */
			++counts[WC_بائيٽون];
			إذا (unicode_حالت != UNICODE_ON /* every byte is a new char */
			 || (c & 0xc0) != 0x80 /* it isn't a 2nd+ byte of a Unicode char */
			) {
				++counts[WC_UNICHARS];
			}

			إذا (isprint_asciionly(c)) { /* FIXME: not unicode-aware */
				++linepos;
				إذا (!خال_آهي(c)) {
					اندريون_لفظ = 1;
					استمر;
				}
			} وإلا إذا ((بنانشان)(c - 9) <= 4) {
				/* \t  9
				 * \n 10
				 * \v 11
				 * \f 12
				 * \r 13
				 */
				إذا (c == '\t') {
					linepos = (linepos | 7) + 1;
				} وإلا {  /* '\n', '\r', '\f', or '\v' */
 DO_EOF:
					إذا (linepos > counts[WC_ڊيگهائي]) {
						counts[WC_ڊيگهائي] = linepos;
					}
					إذا (c == '\n') {
						++counts[WC_سٽون];
					}
					إذا (c != '\v') {
						linepos = 0;
					}
				}
			} وإلا {
				استمر;
			}

			counts[WC_لفظ] += اندريون_لفظ;
			اندريون_لفظ = 0;
			إذا (c == EOF) {
				اكسر;
			}
		}

		fclose_if_not_stdin(fp);

		إذا (totals[WC_ڊيگهائي] < counts[WC_ڊيگهائي]) {
			totals[WC_ڊيگهائي] = counts[WC_ڊيگهائي];
		}
		totals[WC_ڊيگهائي] -= counts[WC_ڊيگهائي];

 OUTPUT:
		/* coreutils wc tries hard to print pretty columns
		 * (saves results for all files, finds max col len etc...)
		 * we won't try that hard, it will bloat us too much */
		s = شروعات_fmt;
		u = 0;
		افعل {
			إذا (print_type & (1 << u)) {
				لکو(s, pcounts[u]);
				s = " %9"ڳڻپ_FMT; /* Ok... restore the leading space. */
			}
			totals[u] += pcounts[u];
		} طالما (++u < انگ_WCS);
		لکو(fname_fmt, arg);
	}

	/* If more than one file was processed, we want the totals.  To save some
	 * space, we set the pcounts ptr to the totals array.  This has the side
	 * effect of trashing the totals array after outputting it, but that's
	 * irrelavent since we no longer need it. */
	إذا (انگ_files > 1) {
		انگ_files = 0;  /* Make sure we don't get here again. */
		arg = "total";
		pcounts = totals;
		--argv;
		اذهب OUTPUT;
	}

	fflush_stdout_and_exit(حالت);
}
