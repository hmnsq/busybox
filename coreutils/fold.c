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

#شامل "libbb.h"
#شامل "unicode.h"

/* This is a NOEXEC applet. Be very careful! */

/* Must match getopt32 call */
#define FLAG_شمار_BYTES        1
#define FLAG_BREAK_SPACES       2
#define FLAG_WIDTH              4

/* Assuming the current column is COLUMN, return the column that
   printing C will move the cursor to.
   The first column is 0. */
ایستا صحیح adjust_column(unsigned column, نویسه c)
{
	اگر (option_mask32 & FLAG_شمار_BYTES)
		بازگشت ++column;

	اگر (c == '\t')
		بازگشت column + 8 - column % 8;

	اگر (c == '\b') {
		اگر ((صحیح)--column < 0)
			column = 0;
	}
	وگرنه اگر (c == '\r')
		column = 0;
	وگرنه { /* just a printable char */
		اگر (unicode_status != UNICODE_ON /* every byte is a new char */
		 || (c & 0xc0) != 0x80 /* it isn't a 2nd+ byte of a Unicode char */
		) {
			column++;
		}
	}
	بازگشت column;
}

/* Note that this function can write NULs, unlike fputs etc. */
ایستا تهی write2stdout(ثابت تهی *buf, unsigned size)
{
	fwrite(buf, 1, size, stdout);
}

صحیح fold_main(صحیح argc, نویسه **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح fold_main(صحیح argc UNUSED_PARAM, نویسه **argv)
{
	نویسه *سطر_out = پوچ;
	ثابت نویسه *w_opt = "80";
	unsigned width;
	exitcode_t exitcode = EXIT_SUCCESS;

	init_unicode();

	اگر (ENABLE_INCLUDE_SUSv2) {
		/* Turn any numeric options into -w options.  */
		صحیح i;
		برای (i = 1; argv[i]; i++) {
			ثابت نویسه *a = argv[i];
			اگر (*a == '-') {
				a++;
				اگر (*a == '-' && !a[1]) /* "--" */
					بشکن;
				اگر (isdigit(*a))
					argv[i] = xasprintf("-w%s", a);
			}
		}
	}

	getopt32(argv, "bsw:", &w_opt);
	width = xatou_range(w_opt, 1, 10000);

	argv += optind;
	اگر (!*argv)
		*--argv = (نویسه*)"-";

	انجامبده {
		FILE *istream = fopen_or_warn_stdin(*argv);
		صحیح c;
		unsigned column = 0;     /* Screen column where next char will go */
		unsigned offset_out = 0; /* Index in 'line_out' for next char */

		اگر (istream == پوچ) {
			exitcode = EXIT_FAILURE;
			ادامه;
		}

		وقتی ((c = getc(istream)) != EOF) {
			/* We grow line_out in chunks of 0x1000 bytes */
			اگر ((offset_out & 0xfff) == 0) {
				سطر_out = xrealloc(سطر_out, offset_out + 0x1000);
			}
 rescan:
			سطر_out[offset_out] = c;
			اگر (c == '\n') {
				write2stdout(سطر_out, offset_out + 1);
				column = offset_out = 0;
				ادامه;
			}
			column = adjust_column(column, c);
			اگر (column <= width || offset_out == 0) {
				/* offset_out == 0 case happens
				 * with small width (say, 1) and tabs.
				 * The very first tab already goes to column 8,
				 * but we must not wrap it */
				offset_out++;
				ادامه;
			}

			/* This character would make the line too long.
			 * Print the line plus a newline, and make this character
			 * start the next line */
			اگر (option_mask32 & FLAG_BREAK_SPACES) {
				unsigned i;
				unsigned logical_end;

				/* Look for the last blank. */
				برای (logical_end = offset_out - 1; (صحیح)logical_end >= 0; logical_end--) {
					اگر (!isblank(سطر_out[logical_end]))
						ادامه;

					/* Found a space or tab.
					 * Output up to and including it, and start a new line */
					logical_end++;
					/*line_out[logical_end] = '\n'; - NO! this nukes one buffered character */
					write2stdout(سطر_out, logical_end);
					putchar('\n');
					/* Move the remainder to the beginning of the next line.
					 * The areas being copied here might overlap. */
					memmove(سطر_out, سطر_out + logical_end, offset_out - logical_end);
					offset_out -= logical_end;
					برای (column = i = 0; i < offset_out; i++) {
						column = adjust_column(column, سطر_out[i]);
					}
					برو rescan;
				}
				/* No blank found, wrap will split the overlong word */
			}
			/* Output what we accumulated up to now, and start a new line */
			سطر_out[offset_out] = '\n';
			write2stdout(سطر_out, offset_out + 1);
			column = offset_out = 0;
			برو rescan;
		} /* while (not EOF) */

		اگر (offset_out) {
			write2stdout(سطر_out, offset_out);
		}

		اگر (fclose_if_not_stdin(istream)) {
			bb_simple_perror_پیام(*argv);
			exitcode = EXIT_FAILURE;
		}
	} وقتی (*++argv);

	fflush_stdout_and_exit(exitcode);
}
