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
# undef isspace
# define isprint(c) ((unsigned)((c) - 0x20) <= (0x7e - 0x20))
# define isspace(c) ((c) == ' ')
#endif

#إذا ENABLE_FEATURE_WC_LARGE
# define تعداد_T unsigned long long
# define تعداد_FMT "llu"
#وإلا
# define تعداد_T unsigned
# define تعداد_FMT "u"
#endif

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
	WC_أسطر    = 0, /* -l */
	WC_كلمات    = 1, /* -w */
	WC_UNICHARS = 2, /* -m */
	WC_BYTES    = 3, /* -c */
	WC_LENGTH   = 4, /* -L */
	رقم_WCS     = 5,
};

صحيح wc_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح wc_main(صحيح argc UNUSED_PARAM, حرف **argv)
{
	ثابت حرف *arg;
	ثابت حرف *start_fmt = " %9"تعداد_FMT + 1;
	ثابت حرف *fname_fmt = " %s\n";
	تعداد_T *pcounts;
	تعداد_T counts[رقم_WCS];
	تعداد_T totals[رقم_WCS];
	صحيح رقم_files;
	smallint status = EXIT_SUCCESS;
	unsigned print_type;

	init_unicode();

	print_type = getopt32(argv, "lwmcL");

	إذا (print_type == 0) {
		print_type = (1 << WC_أسطر) | (1 << WC_كلمات) | (1 << WC_BYTES);
	}

	argv += optind;
	إذا (!argv[0]) {
		*--argv = (حرف *) bb_رسالة_standard_إدخال;
		fname_fmt = "\n";
	}
	إذا (!argv[1]) { /* zero or one filename? */
		إذا (!((print_type-1) & print_type)) /* exactly one option? */
			start_fmt = "%"تعداد_FMT;
	}

	memset(totals, 0, حجم(totals));

	pcounts = counts;

	رقم_files = 0;
	بينما ((arg = *argv++) != عدم) {
		FILE *fp;
		ثابت حرف *s;
		unsigned u;
		unsigned linepos;
		smallint in_word;

		++رقم_files;
		fp = fopen_or_warn_stdin(arg);
		إذا (!fp) {
			status = EXIT_FAILURE;
			استمر;
		}

		memset(counts, 0, حجم(counts));
		linepos = 0;
		in_word = 0;

		بينما (1) {
			صحيح c;
			/* Our -w doesn't match GNU wc exactly... oh well */

			c = getc(fp);
			إذا (c == EOF) {
				إذا (ferror(fp)) {
					bb_simple_perror_رسالة(arg);
					status = EXIT_FAILURE;
				}
				اذهب DO_EOF;  /* Treat an EOF as '\r'. */
			}

			/* Cater for -c and -m */
			++counts[WC_BYTES];
			إذا (unicode_status != UNICODE_ON /* every byte is a new char */
			 || (c & 0xc0) != 0x80 /* it isn't a 2nd+ byte of a Unicode char */
			) {
				++counts[WC_UNICHARS];
			}

			إذا (isprint_asciionly(c)) { /* FIXME: not unicode-aware */
				++linepos;
				إذا (!isspace(c)) {
					in_word = 1;
					استمر;
				}
			} وإلا إذا ((unsigned)(c - 9) <= 4) {
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
					إذا (linepos > counts[WC_LENGTH]) {
						counts[WC_LENGTH] = linepos;
					}
					إذا (c == '\n') {
						++counts[WC_أسطر];
					}
					إذا (c != '\v') {
						linepos = 0;
					}
				}
			} وإلا {
				استمر;
			}

			counts[WC_كلمات] += in_word;
			in_word = 0;
			إذا (c == EOF) {
				اكسر;
			}
		}

		fclose_if_not_stdin(fp);

		إذا (totals[WC_LENGTH] < counts[WC_LENGTH]) {
			totals[WC_LENGTH] = counts[WC_LENGTH];
		}
		totals[WC_LENGTH] -= counts[WC_LENGTH];

 OUTPUT:
		/* coreutils wc tries hard to print pretty columns
		 * (saves results for all files, finds max col len etc...)
		 * we won't try that hard, it will bloat us too much */
		s = start_fmt;
		u = 0;
		افعل {
			إذا (print_type & (1 << u)) {
				printf(s, pcounts[u]);
				s = " %9"تعداد_FMT; /* Ok... restore the leading space. */
			}
			totals[u] += pcounts[u];
		} بينما (++u < رقم_WCS);
		printf(fname_fmt, arg);
	}

	/* If more than one file was processed, we want the totals.  To save some
	 * space, we set the pcounts ptr to the totals array.  This has the side
	 * effect of trashing the totals array after outputting it, but that's
	 * irrelavent since we no longer need it. */
	إذا (رقم_files > 1) {
		رقم_files = 0;  /* Make sure we don't get here again. */
		arg = "total";
		pcounts = totals;
		--argv;
		اذهب OUTPUT;
	}

	fflush_stdout_and_exit(status);
}
