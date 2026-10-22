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
# undef isspace
# define isprint(c) ((unsigned)((c) - 0x20) <= (0x7e - 0x20))
# define isspace(c) ((c) == ' ')
#endif

#اگر ENABLE_FEATURE_WC_LARGE
# define شمار_T unsigned long long
# define شمار_FMT "llu"
#وگرنه
# define شمار_T unsigned
# define شمار_FMT "u"
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
شمارش {
	WC_سطرها    = 0, /* -l */
	WC_واژگان    = 1, /* -w */
	WC_UNICHARS = 2, /* -m */
	WC_BYTES    = 3, /* -c */
	WC_LENGTH   = 4, /* -L */
	رقم_WCS     = 5,
};

صحیح wc_main(صحیح argc, نویسه **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح wc_main(صحیح argc UNUSED_PARAM, نویسه **argv)
{
	ثابت نویسه *arg;
	ثابت نویسه *start_fmt = " %9"شمار_FMT + 1;
	ثابت نویسه *fname_fmt = " %s\n";
	شمار_T *pcounts;
	شمار_T counts[رقم_WCS];
	شمار_T totals[رقم_WCS];
	صحیح رقم_files;
	smallint status = EXIT_SUCCESS;
	unsigned print_type;

	init_unicode();

	print_type = getopt32(argv, "lwmcL");

	اگر (print_type == 0) {
		print_type = (1 << WC_سطرها) | (1 << WC_واژگان) | (1 << WC_BYTES);
	}

	argv += optind;
	اگر (!argv[0]) {
		*--argv = (نویسه *) bb_پیام_standard_درون_داد;
		fname_fmt = "\n";
	}
	اگر (!argv[1]) { /* zero or one filename? */
		اگر (!((print_type-1) & print_type)) /* exactly one option? */
			start_fmt = "%"شمار_FMT;
	}

	memset(totals, 0, اندازه(totals));

	pcounts = counts;

	رقم_files = 0;
	وقتی ((arg = *argv++) != پوچ) {
		FILE *fp;
		ثابت نویسه *s;
		unsigned u;
		unsigned linepos;
		smallint in_word;

		++رقم_files;
		fp = fopen_or_warn_stdin(arg);
		اگر (!fp) {
			status = EXIT_FAILURE;
			ادامه;
		}

		memset(counts, 0, اندازه(counts));
		linepos = 0;
		in_word = 0;

		وقتی (1) {
			صحیح c;
			/* Our -w doesn't match GNU wc exactly... oh well */

			c = getc(fp);
			اگر (c == EOF) {
				اگر (ferror(fp)) {
					bb_simple_perror_پیام(arg);
					status = EXIT_FAILURE;
				}
				برو DO_EOF;  /* Treat an EOF as '\r'. */
			}

			/* Cater for -c and -m */
			++counts[WC_BYTES];
			اگر (unicode_status != UNICODE_ON /* every byte is a new char */
			 || (c & 0xc0) != 0x80 /* it isn't a 2nd+ byte of a Unicode char */
			) {
				++counts[WC_UNICHARS];
			}

			اگر (isprint_asciionly(c)) { /* FIXME: not unicode-aware */
				++linepos;
				اگر (!isspace(c)) {
					in_word = 1;
					ادامه;
				}
			} وگرنه اگر ((unsigned)(c - 9) <= 4) {
				/* \t  9
				 * \n 10
				 * \v 11
				 * \f 12
				 * \r 13
				 */
				اگر (c == '\t') {
					linepos = (linepos | 7) + 1;
				} وگرنه {  /* '\n', '\r', '\f', or '\v' */
 DO_EOF:
					اگر (linepos > counts[WC_LENGTH]) {
						counts[WC_LENGTH] = linepos;
					}
					اگر (c == '\n') {
						++counts[WC_سطرها];
					}
					اگر (c != '\v') {
						linepos = 0;
					}
				}
			} وگرنه {
				ادامه;
			}

			counts[WC_واژگان] += in_word;
			in_word = 0;
			اگر (c == EOF) {
				بشکن;
			}
		}

		fclose_if_not_stdin(fp);

		اگر (totals[WC_LENGTH] < counts[WC_LENGTH]) {
			totals[WC_LENGTH] = counts[WC_LENGTH];
		}
		totals[WC_LENGTH] -= counts[WC_LENGTH];

 OUTPUT:
		/* coreutils wc tries hard to print pretty columns
		 * (saves results for all files, finds max col len etc...)
		 * we won't try that hard, it will bloat us too much */
		s = start_fmt;
		u = 0;
		انجامبده {
			اگر (print_type & (1 << u)) {
				printf(s, pcounts[u]);
				s = " %9"شمار_FMT; /* Ok... restore the leading space. */
			}
			totals[u] += pcounts[u];
		} وقتی (++u < رقم_WCS);
		printf(fname_fmt, arg);
	}

	/* If more than one file was processed, we want the totals.  To save some
	 * space, we set the pcounts ptr to the totals array.  This has the side
	 * effect of trashing the totals array after outputting it, but that's
	 * irrelavent since we no longer need it. */
	اگر (رقم_files > 1) {
		رقم_files = 0;  /* Make sure we don't get here again. */
		arg = "total";
		pcounts = totals;
		--argv;
		برو OUTPUT;
	}

	fflush_stdout_and_exit(status);
}
