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

#ಅನ್ತರ್ಭಾವ "libbb.h"
#ಅನ್ತರ್ಭಾವ "unicode.h"

/* This is a NOEXEC applet. Be very careful! */

/* Must match getopt32 call */
#define FLAG_ಎಣಿಕೆ_BYTES        1
#define FLAG_BREAK_SPACES       2
#define FLAG_WIDTH              4

/* Assuming the current column is COLUMN, return the column that
   printing C will move the cursor to.
   The first column is 0. */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ adjust_column(unsigned column, ವರ್ಣ c)
{
	ಯದಿ (option_mask32 & FLAG_ಎಣಿಕೆ_BYTES)
		ಪ್ರತ್ಯಾಗಮ ++column;

	ಯದಿ (c == '\t')
		ಪ್ರತ್ಯಾಗಮ column + 8 - column % 8;

	ಯದಿ (c == '\b') {
		ಯದಿ ((ಪೂರ್ಣಾಙ್ಕ)--column < 0)
			column = 0;
	}
	ಅನ್ಯಥಾ ಯದಿ (c == '\r')
		column = 0;
	ಅನ್ಯಥಾ { /* just a printable char */
		ಯದಿ (unicode_status != UNICODE_ON /* every byte is a new char */
		 || (c & 0xc0) != 0x80 /* it isn't a 2nd+ byte of a Unicode char */
		) {
			column++;
		}
	}
	ಪ್ರತ್ಯಾಗಮ column;
}

/* Note that this function can write NULs, unlike fputs etc. */
ಸ್ಥಿರ ಶೂನ್ಯ write2stdout(ಧ್ರುವ ಶೂನ್ಯ *buf, unsigned size)
{
	fwrite(buf, 1, size, stdout);
}

ಪೂರ್ಣಾಙ್ಕ fold_main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಙ್ಕ fold_main(ಪೂರ್ಣಾಙ್ಕ argc UNUSED_PARAM, ವರ್ಣ **argv)
{
	ವರ್ಣ *ಸಾಲು_out = ರಿಕ್ತ;
	ಧ್ರುವ ವರ್ಣ *w_opt = "80";
	unsigned width;
	exitcode_t exitcode = EXIT_SUCCESS;

	init_unicode();

	ಯದಿ (ENABLE_INCLUDE_SUSv2) {
		/* Turn any numeric options into -w options.  */
		ಪೂರ್ಣಾಙ್ಕ i;
		ಪ್ರತಿ (i = 1; argv[i]; i++) {
			ಧ್ರುವ ವರ್ಣ *a = argv[i];
			ಯದಿ (*a == '-') {
				a++;
				ಯದಿ (*a == '-' && !a[1]) /* "--" */
					ಭಙ್ಗ;
				ಯದಿ (isdigit(*a))
					argv[i] = xasprintf("-w%s", a);
			}
		}
	}

	getopt32(argv, "bsw:", &w_opt);
	width = xatou_range(w_opt, 1, 10000);

	argv += optind;
	ಯದಿ (!*argv)
		*--argv = (ವರ್ಣ*)"-";

	ಕುರು {
		FILE *istream = fopen_or_warn_stdin(*argv);
		ಪೂರ್ಣಾಙ್ಕ c;
		unsigned column = 0;     /* Screen column where next char will go */
		unsigned offset_out = 0; /* Index in 'line_out' for next char */

		ಯದಿ (istream == ರಿಕ್ತ) {
			exitcode = EXIT_FAILURE;
			ಅನುವರ್ತ;
		}

		ಯಾವತ್ ((c = getc(istream)) != EOF) {
			/* We grow line_out in chunks of 0x1000 bytes */
			ಯದಿ ((offset_out & 0xfff) == 0) {
				ಸಾಲು_out = xrealloc(ಸಾಲು_out, offset_out + 0x1000);
			}
 rescan:
			ಸಾಲು_out[offset_out] = c;
			ಯದಿ (c == '\n') {
				write2stdout(ಸಾಲು_out, offset_out + 1);
				column = offset_out = 0;
				ಅನುವರ್ತ;
			}
			column = adjust_column(column, c);
			ಯದಿ (column <= width || offset_out == 0) {
				/* offset_out == 0 case happens
				 * with small width (say, 1) and tabs.
				 * The very first tab already goes to column 8,
				 * but we must not wrap it */
				offset_out++;
				ಅನುವರ್ತ;
			}

			/* This character would make the line too long.
			 * Print the line plus a newline, and make this character
			 * start the next line */
			ಯದಿ (option_mask32 & FLAG_BREAK_SPACES) {
				unsigned i;
				unsigned logical_end;

				/* Look for the last blank. */
				ಪ್ರತಿ (logical_end = offset_out - 1; (ಪೂರ್ಣಾಙ್ಕ)logical_end >= 0; logical_end--) {
					ಯದಿ (!isblank(ಸಾಲು_out[logical_end]))
						ಅನುವರ್ತ;

					/* Found a space or tab.
					 * Output up to and including it, and start a new line */
					logical_end++;
					/*line_out[logical_end] = '\n'; - NO! this nukes one buffered character */
					write2stdout(ಸಾಲು_out, logical_end);
					putchar('\n');
					/* Move the remainder to the beginning of the next line.
					 * The areas being copied here might overlap. */
					memmove(ಸಾಲು_out, ಸಾಲು_out + logical_end, offset_out - logical_end);
					offset_out -= logical_end;
					ಪ್ರತಿ (column = i = 0; i < offset_out; i++) {
						column = adjust_column(column, ಸಾಲು_out[i]);
					}
					ಗಚ್ಛ rescan;
				}
				/* No blank found, wrap will split the overlong word */
			}
			/* Output what we accumulated up to now, and start a new line */
			ಸಾಲು_out[offset_out] = '\n';
			write2stdout(ಸಾಲು_out, offset_out + 1);
			column = offset_out = 0;
			ಗಚ್ಛ rescan;
		} /* while (not EOF) */

		ಯದಿ (offset_out) {
			write2stdout(ಸಾಲು_out, offset_out);
		}

		ಯದಿ (fclose_if_not_stdin(istream)) {
			bb_simple_perror_ಸಂದೇಶ(*argv);
			exitcode = EXIT_FAILURE;
		}
	} ಯಾವತ್ (*++argv);

	fflush_stdout_and_exit(exitcode);
}
