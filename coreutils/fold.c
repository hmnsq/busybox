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

#અન્તર્ભાવ "libbb.h"
#અન્તર્ભાવ "unicode.h"

/* This is a NOEXEC applet. Be very careful! */

/* Must match getopt32 call */
#define FLAG_ગણતરી_BYTES        1
#define FLAG_BREAK_SPACES       2
#define FLAG_WIDTH              4

/* Assuming the current column is COLUMN, return the column that
   printing C will move the cursor to.
   The first column is 0. */
સ્થિર પૂર્ણાંક adjust_column(unsigned column, અક્ષર c)
{
	જો (option_mask32 & FLAG_ગણતરી_BYTES)
		પાછા ++column;

	જો (c == '\t')
		પાછા column + 8 - column % 8;

	જો (c == '\b') {
		જો ((પૂર્ણાંક)--column < 0)
			column = 0;
	}
	નહીંતર જો (c == '\r')
		column = 0;
	નહીંતર { /* just a printable char */
		જો (unicode_status != UNICODE_ON /* every byte is a new char */
		 || (c & 0xc0) != 0x80 /* it isn't a 2nd+ byte of a Unicode char */
		) {
			column++;
		}
	}
	પાછા column;
}

/* Note that this function can write NULs, unlike fputs etc. */
સ્થિર શૂન્ય write2stdout(અચલ શૂન્ય *buf, unsigned size)
{
	fwrite(buf, 1, size, stdout);
}

પૂર્ણાંક fold_main(પૂર્ણાંક argc, અક્ષર **argv) MAIN_EXTERNALLY_VISIBLE;
પૂર્ણાંક fold_main(પૂર્ણાંક argc UNUSED_PARAM, અક્ષર **argv)
{
	અક્ષર *લીટી_out = ખાલી;
	અચલ અક્ષર *w_opt = "80";
	unsigned width;
	exitcode_t exitcode = EXIT_SUCCESS;

	init_unicode();

	જો (ENABLE_INCLUDE_SUSv2) {
		/* Turn any numeric options into -w options.  */
		પૂર્ણાંક i;
		માટે (i = 1; argv[i]; i++) {
			અચલ અક્ષર *a = argv[i];
			જો (*a == '-') {
				a++;
				જો (*a == '-' && !a[1]) /* "--" */
					તોડો;
				જો (isdigit(*a))
					argv[i] = xasprintf("-w%s", a);
			}
		}
	}

	getopt32(argv, "bsw:", &w_opt);
	width = xatou_range(w_opt, 1, 10000);

	argv += optind;
	જો (!*argv)
		*--argv = (અક્ષર*)"-";

	કરો {
		FILE *istream = fopen_or_warn_stdin(*argv);
		પૂર્ણાંક c;
		unsigned column = 0;     /* Screen column where next char will go */
		unsigned offset_out = 0; /* Index in 'line_out' for next char */

		જો (istream == ખાલી) {
			exitcode = EXIT_FAILURE;
			ચાલુ;
		}

		જ્યારે ((c = getc(istream)) != EOF) {
			/* We grow line_out in chunks of 0x1000 bytes */
			જો ((offset_out & 0xfff) == 0) {
				લીટી_out = xrealloc(લીટી_out, offset_out + 0x1000);
			}
 rescan:
			લીટી_out[offset_out] = c;
			જો (c == '\n') {
				write2stdout(લીટી_out, offset_out + 1);
				column = offset_out = 0;
				ચાલુ;
			}
			column = adjust_column(column, c);
			જો (column <= width || offset_out == 0) {
				/* offset_out == 0 case happens
				 * with small width (say, 1) and tabs.
				 * The very first tab already goes to column 8,
				 * but we must not wrap it */
				offset_out++;
				ચાલુ;
			}

			/* This character would make the line too long.
			 * Print the line plus a newline, and make this character
			 * start the next line */
			જો (option_mask32 & FLAG_BREAK_SPACES) {
				unsigned i;
				unsigned logical_end;

				/* Look for the last blank. */
				માટે (logical_end = offset_out - 1; (પૂર્ણાંક)logical_end >= 0; logical_end--) {
					જો (!isblank(લીટી_out[logical_end]))
						ચાલુ;

					/* Found a space or tab.
					 * Output up to and including it, and start a new line */
					logical_end++;
					/*line_out[logical_end] = '\n'; - NO! this nukes one buffered character */
					write2stdout(લીટી_out, logical_end);
					putchar('\n');
					/* Move the remainder to the beginning of the next line.
					 * The areas being copied here might overlap. */
					memmove(લીટી_out, લીટી_out + logical_end, offset_out - logical_end);
					offset_out -= logical_end;
					માટે (column = i = 0; i < offset_out; i++) {
						column = adjust_column(column, લીટી_out[i]);
					}
					જાઓ rescan;
				}
				/* No blank found, wrap will split the overlong word */
			}
			/* Output what we accumulated up to now, and start a new line */
			લીટી_out[offset_out] = '\n';
			write2stdout(લીટી_out, offset_out + 1);
			column = offset_out = 0;
			જાઓ rescan;
		} /* while (not EOF) */

		જો (offset_out) {
			write2stdout(લીટી_out, offset_out);
		}

		જો (fclose_if_not_stdin(istream)) {
			bb_simple_perror_સંદેશ(*argv);
			exitcode = EXIT_FAILURE;
		}
	} જ્યારે (*++argv);

	fflush_stdout_and_exit(exitcode);
}
