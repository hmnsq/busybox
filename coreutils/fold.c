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

#অন্তর্ভাৱ "libbb.h"
#অন্তর্ভাৱ "unicode.h"

/* This is a NOEXEC applet. Be very careful! */

/* Must match getopt32 call */
#define FLAG_গুনতি_BYTES        1
#define FLAG_BREAK_SPACES       2
#define FLAG_WIDTH              4

/* Assuming the current column is COLUMN, return the column that
   printing C will move the cursor to.
   The first column is 0. */
স্থির পূর্ণাঙ্ক adjust_column(unsigned column, ৱর্ণ c)
{
	যদি (option_mask32 & FLAG_গুনতি_BYTES)
		ঘূৰাই ++column;

	যদি (c == '\t')
		ঘূৰাই column + 8 - column % 8;

	যদি (c == '\b') {
		যদি ((পূর্ণাঙ্ক)--column < 0)
			column = 0;
	}
	অন্যথা যদি (c == '\r')
		column = 0;
	অন্যথা { /* just a printable char */
		যদি (unicode_status != UNICODE_ON /* every byte is a new char */
		 || (c & 0xc0) != 0x80 /* it isn't a 2nd+ byte of a Unicode char */
		) {
			column++;
		}
	}
	ঘূৰাই column;
}

/* Note that this function can write NULs, unlike fputs etc. */
স্থির শূন্য write2stdout(ধ্রুৱ শূন্য *buf, unsigned size)
{
	fwrite(buf, 1, size, stdout);
}

পূর্ণাঙ্ক fold_main(পূর্ণাঙ্ক argc, ৱর্ণ **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণাঙ্ক fold_main(পূর্ণাঙ্ক argc UNUSED_PARAM, ৱর্ণ **argv)
{
	ৱর্ণ *সাৰি_out = রিক্ত;
	ধ্রুৱ ৱর্ণ *w_opt = "80";
	unsigned width;
	exitcode_t exitcode = EXIT_SUCCESS;

	init_unicode();

	যদি (ENABLE_INCLUDE_SUSv2) {
		/* Turn any numeric options into -w options.  */
		পূর্ণাঙ্ক i;
		বাবে (i = 1; argv[i]; i++) {
			ধ্রুৱ ৱর্ণ *a = argv[i];
			যদি (*a == '-') {
				a++;
				যদি (*a == '-' && !a[1]) /* "--" */
					ভঙ্গ;
				যদি (isdigit(*a))
					argv[i] = xasprintf("-w%s", a);
			}
		}
	}

	getopt32(argv, "bsw:", &w_opt);
	width = xatou_range(w_opt, 1, 10000);

	argv += optind;
	যদি (!*argv)
		*--argv = (ৱর্ণ*)"-";

	কৰা {
		FILE *istream = fopen_or_warn_stdin(*argv);
		পূর্ণাঙ্ক c;
		unsigned column = 0;     /* Screen column where next char will go */
		unsigned offset_out = 0; /* Index in 'line_out' for next char */

		যদি (istream == রিক্ত) {
			exitcode = EXIT_FAILURE;
			অনুৱর্ত;
		}

		যেতিয়ালৈ ((c = getc(istream)) != EOF) {
			/* We grow line_out in chunks of 0x1000 bytes */
			যদি ((offset_out & 0xfff) == 0) {
				সাৰি_out = xrealloc(সাৰি_out, offset_out + 0x1000);
			}
 rescan:
			সাৰি_out[offset_out] = c;
			যদি (c == '\n') {
				write2stdout(সাৰি_out, offset_out + 1);
				column = offset_out = 0;
				অনুৱর্ত;
			}
			column = adjust_column(column, c);
			যদি (column <= width || offset_out == 0) {
				/* offset_out == 0 case happens
				 * with small width (say, 1) and tabs.
				 * The very first tab already goes to column 8,
				 * but we must not wrap it */
				offset_out++;
				অনুৱর্ত;
			}

			/* This character would make the line too long.
			 * Print the line plus a newline, and make this character
			 * start the next line */
			যদি (option_mask32 & FLAG_BREAK_SPACES) {
				unsigned i;
				unsigned logical_end;

				/* Look for the last blank. */
				বাবে (logical_end = offset_out - 1; (পূর্ণাঙ্ক)logical_end >= 0; logical_end--) {
					যদি (!isblank(সাৰি_out[logical_end]))
						অনুৱর্ত;

					/* Found a space or tab.
					 * Output up to and including it, and start a new line */
					logical_end++;
					/*line_out[logical_end] = '\n'; - NO! this nukes one buffered character */
					write2stdout(সাৰি_out, logical_end);
					putchar('\n');
					/* Move the remainder to the beginning of the next line.
					 * The areas being copied here might overlap. */
					memmove(সাৰি_out, সাৰি_out + logical_end, offset_out - logical_end);
					offset_out -= logical_end;
					বাবে (column = i = 0; i < offset_out; i++) {
						column = adjust_column(column, সাৰি_out[i]);
					}
					গচ্ছ rescan;
				}
				/* No blank found, wrap will split the overlong word */
			}
			/* Output what we accumulated up to now, and start a new line */
			সাৰি_out[offset_out] = '\n';
			write2stdout(সাৰি_out, offset_out + 1);
			column = offset_out = 0;
			গচ্ছ rescan;
		} /* while (not EOF) */

		যদি (offset_out) {
			write2stdout(সাৰি_out, offset_out);
		}

		যদি (fclose_if_not_stdin(istream)) {
			bb_simple_perror_বাৰ্তা(*argv);
			exitcode = EXIT_FAILURE;
		}
	} যেতিয়ালৈ (*++argv);

	fflush_stdout_and_exit(exitcode);
}
