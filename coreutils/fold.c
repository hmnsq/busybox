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

#include "libbb.h"
#include "unicode.h"

/* This is a NOEXEC applet. Be very careful! */

/* Must match getopt32 call */
#define FLAG_COUNT_BYTES        1
#define FLAG_BREAK_SPACES       2
#define FLAG_WIDTH              4

/* Assuming the current column is COLUMN, return the column that
   printing C will move the cursor to.
   The first column is 0. */
static 정수 adjust_column(unsigned column, char c)
{
	만약 (option_mask32 & FLAG_COUNT_BYTES)
		반환 ++column;

	만약 (c == '\t')
		반환 column + 8 - column % 8;

	만약 (c == '\b') {
		만약 ((정수)--column < 0)
			column = 0;
	}
	아니면 만약 (c == '\r')
		column = 0;
	아니면 { /* just a printable char */
		만약 (unicode_status != UNICODE_ON /* every byte is a new char */
		 || (c & 0xc0) != 0x80 /* it isn't a 2nd+ byte of a Unicode char */
		) {
			column++;
		}
	}
	반환 column;
}

/* Note that this function can write NULs, unlike fputs etc. */
static void write2stdout(const void *buf, unsigned size)
{
	fwrite(buf, 1, size, stdout);
}

정수 fold_main(정수 argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
정수 fold_main(정수 argc UNUSED_PARAM, char **argv)
{
	char *line_out = NULL;
	const char *w_opt = "80";
	unsigned width;
	exitcode_t exitcode = EXIT_SUCCESS;

	init_unicode();

	만약 (ENABLE_INCLUDE_SUSv2) {
		/* Turn any numeric options into -w options.  */
		정수 i;
		위해 (i = 1; argv[i]; i++) {
			const char *a = argv[i];
			만약 (*a == '-') {
				a++;
				만약 (*a == '-' && !a[1]) /* "--" */
					break;
				만약 (isdigit(*a))
					argv[i] = xasprintf("-w%s", a);
			}
		}
	}

	getopt32(argv, "bsw:", &w_opt);
	width = xatou_range(w_opt, 1, 10000);

	argv += optind;
	만약 (!*argv)
		*--argv = (char*)"-";

	do {
		FILE *istream = fopen_or_warn_stdin(*argv);
		정수 c;
		unsigned column = 0;     /* Screen column where next char will go */
		unsigned offset_out = 0; /* Index in 'line_out' for next char */

		만약 (istream == NULL) {
			exitcode = EXIT_FAILURE;
			continue;
		}

		동안 ((c = getc(istream)) != EOF) {
			/* We grow line_out in chunks of 0x1000 bytes */
			만약 ((offset_out & 0xfff) == 0) {
				line_out = xrealloc(line_out, offset_out + 0x1000);
			}
 rescan:
			line_out[offset_out] = c;
			만약 (c == '\n') {
				write2stdout(line_out, offset_out + 1);
				column = offset_out = 0;
				continue;
			}
			column = adjust_column(column, c);
			만약 (column <= width || offset_out == 0) {
				/* offset_out == 0 case happens
				 * with small width (say, 1) and tabs.
				 * The very first tab already goes to column 8,
				 * but we must not wrap it */
				offset_out++;
				continue;
			}

			/* This character would make the line too long.
			 * Print the line plus a newline, and make this character
			 * start the next line */
			만약 (option_mask32 & FLAG_BREAK_SPACES) {
				unsigned i;
				unsigned logical_end;

				/* Look for the last blank. */
				위해 (logical_end = offset_out - 1; (정수)logical_end >= 0; logical_end--) {
					만약 (!isblank(line_out[logical_end]))
						continue;

					/* Found a space or tab.
					 * Output up to and including it, and start a new line */
					logical_end++;
					/*line_out[logical_end] = '\n'; - NO! this nukes one buffered character */
					write2stdout(line_out, logical_end);
					putchar('\n');
					/* Move the remainder to the beginning of the next line.
					 * The areas being copied here might overlap. */
					memmove(line_out, line_out + logical_end, offset_out - logical_end);
					offset_out -= logical_end;
					위해 (column = i = 0; i < offset_out; i++) {
						column = adjust_column(column, line_out[i]);
					}
					goto rescan;
				}
				/* No blank found, wrap will split the overlong word */
			}
			/* Output what we accumulated up to now, and start a new line */
			line_out[offset_out] = '\n';
			write2stdout(line_out, offset_out + 1);
			column = offset_out = 0;
			goto rescan;
		} /* while (not EOF) */

		만약 (offset_out) {
			write2stdout(line_out, offset_out);
		}

		만약 (fclose_if_not_stdin(istream)) {
			bb_simple_perror_msg(*argv);
			exitcode = EXIT_FAILURE;
		}
	} 동안 (*++argv);

	fflush_stdout_and_exit(exitcode);
}
