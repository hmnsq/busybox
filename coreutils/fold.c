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

#அந்தர்பாவ "libbb.h"
#அந்தர்பாவ "unicode.h"

/* This is a NOEXEC applet. Be very careful! */

/* Must match getopt32 call */
#define FLAG_மொத்தம்_BYTES        1
#define FLAG_BREAK_SPACES       2
#define FLAG_WIDTH              4

/* Assuming the current column is COLUMN, return the column that
   printing C will move the cursor to.
   The first column is 0. */
நிலையான முழு adjust_column(unsigned column, எழுத்து c)
{
	எனில் (option_mask32 & FLAG_மொத்தம்_BYTES)
		திரும்பு ++column;

	எனில் (c == '\t')
		திரும்பு column + 8 - column % 8;

	எனில் (c == '\b') {
		எனில் ((முழு)--column < 0)
			column = 0;
	}
	இல்லையெனில் எனில் (c == '\r')
		column = 0;
	இல்லையெனில் { /* just a printable char */
		எனில் (unicode_status != UNICODE_ON /* every byte is a new char */
		 || (c & 0xc0) != 0x80 /* it isn't a 2nd+ byte of a Unicode char */
		) {
			column++;
		}
	}
	திரும்பு column;
}

/* Note that this function can write NULs, unlike fputs etc. */
நிலையான வெறுமை write2stdout(மாறிலி வெறுமை *buf, unsigned size)
{
	fwrite(buf, 1, size, stdout);
}

முழு fold_main(முழு argc, எழுத்து **argv) MAIN_EXTERNALLY_VISIBLE;
முழு fold_main(முழு argc UNUSED_PARAM, எழுத்து **argv)
{
	எழுத்து *வரி_out = வெற்று;
	மாறிலி எழுத்து *w_opt = "80";
	unsigned width;
	exitcode_t exitcode = EXIT_SUCCESS;

	init_unicode();

	எனில் (ENABLE_INCLUDE_SUSv2) {
		/* Turn any numeric options into -w options.  */
		முழு i;
		க்கு (i = 1; argv[i]; i++) {
			மாறிலி எழுத்து *a = argv[i];
			எனில் (*a == '-') {
				a++;
				எனில் (*a == '-' && !a[1]) /* "--" */
					நிறுத்து;
				எனில் (isdigit(*a))
					argv[i] = xasprintf("-w%s", a);
			}
		}
	}

	getopt32(argv, "bsw:", &w_opt);
	width = xatou_range(w_opt, 1, 10000);

	argv += optind;
	எனில் (!*argv)
		*--argv = (எழுத்து*)"-";

	செய் {
		FILE *istream = fopen_or_warn_stdin(*argv);
		முழு c;
		unsigned column = 0;     /* Screen column where next char will go */
		unsigned offset_out = 0; /* Index in 'line_out' for next char */

		எனில் (istream == வெற்று) {
			exitcode = EXIT_FAILURE;
			தொடர்;
		}

		போது ((c = getc(istream)) != EOF) {
			/* We grow line_out in chunks of 0x1000 bytes */
			எனில் ((offset_out & 0xfff) == 0) {
				வரி_out = xrealloc(வரி_out, offset_out + 0x1000);
			}
 rescan:
			வரி_out[offset_out] = c;
			எனில் (c == '\n') {
				write2stdout(வரி_out, offset_out + 1);
				column = offset_out = 0;
				தொடர்;
			}
			column = adjust_column(column, c);
			எனில் (column <= width || offset_out == 0) {
				/* offset_out == 0 case happens
				 * with small width (say, 1) and tabs.
				 * The very first tab already goes to column 8,
				 * but we must not wrap it */
				offset_out++;
				தொடர்;
			}

			/* This character would make the line too long.
			 * Print the line plus a newline, and make this character
			 * start the next line */
			எனில் (option_mask32 & FLAG_BREAK_SPACES) {
				unsigned i;
				unsigned logical_end;

				/* Look for the last blank. */
				க்கு (logical_end = offset_out - 1; (முழு)logical_end >= 0; logical_end--) {
					எனில் (!isblank(வரி_out[logical_end]))
						தொடர்;

					/* Found a space or tab.
					 * Output up to and including it, and start a new line */
					logical_end++;
					/*line_out[logical_end] = '\n'; - NO! this nukes one buffered character */
					write2stdout(வரி_out, logical_end);
					putchar('\n');
					/* Move the remainder to the beginning of the next line.
					 * The areas being copied here might overlap. */
					memmove(வரி_out, வரி_out + logical_end, offset_out - logical_end);
					offset_out -= logical_end;
					க்கு (column = i = 0; i < offset_out; i++) {
						column = adjust_column(column, வரி_out[i]);
					}
					செல் rescan;
				}
				/* No blank found, wrap will split the overlong word */
			}
			/* Output what we accumulated up to now, and start a new line */
			வரி_out[offset_out] = '\n';
			write2stdout(வரி_out, offset_out + 1);
			column = offset_out = 0;
			செல் rescan;
		} /* while (not EOF) */

		எனில் (offset_out) {
			write2stdout(வரி_out, offset_out);
		}

		எனில் (fclose_if_not_stdin(istream)) {
			bb_simple_perror_செய்தி(*argv);
			exitcode = EXIT_FAILURE;
		}
	} போது (*++argv);

	fflush_stdout_and_exit(exitcode);
}
