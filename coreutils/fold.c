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

#ଅନ୍ତର୍ଭାଵ "libbb.h"
#ଅନ୍ତର୍ଭାଵ "unicode.h"

/* This is a NOEXEC applet. Be very careful! */

/* Must match getopt32 call */
#define FLAG_ଗଣନା_BYTES        1
#define FLAG_BREAK_SPACES       2
#define FLAG_WIDTH              4

/* Assuming the current column is COLUMN, return the column that
   printing C will move the cursor to.
   The first column is 0. */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା adjust_column(unsigned column, ଅକ୍ଷର c)
{
	ଯଦି (option_mask32 & FLAG_ଗଣନା_BYTES)
		ଫେରନ୍ତୁ ++column;

	ଯଦି (c == '\t')
		ଫେରନ୍ତୁ column + 8 - column % 8;

	ଯଦି (c == '\b') {
		ଯଦି ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)--column < 0)
			column = 0;
	}
	ଅନ୍ୟଥା ଯଦି (c == '\r')
		column = 0;
	ଅନ୍ୟଥା { /* just a printable char */
		ଯଦି (unicode_status != UNICODE_ON /* every byte is a new char */
		 || (c & 0xc0) != 0x80 /* it isn't a 2nd+ byte of a Unicode char */
		) {
			column++;
		}
	}
	ଫେରନ୍ତୁ column;
}

/* Note that this function can write NULs, unlike fputs etc. */
ସ୍ଥିର ଶୂନ୍ୟ write2stdout(ଅଚଳ ଶୂନ୍ୟ *buf, unsigned size)
{
	fwrite(buf, 1, size, stdout);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା fold_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv) MAIN_EXTERNALLY_VISIBLE;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା fold_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc UNUSED_PARAM, ଅକ୍ଷର **argv)
{
	ଅକ୍ଷର *ଧାଡ଼ି_out = ଖାଲି;
	ଅଚଳ ଅକ୍ଷର *w_opt = "80";
	unsigned width;
	exitcode_t exitcode = EXIT_SUCCESS;

	init_unicode();

	ଯଦି (ENABLE_INCLUDE_SUSv2) {
		/* Turn any numeric options into -w options.  */
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
		ପାଇଁ (i = 1; argv[i]; i++) {
			ଅଚଳ ଅକ୍ଷର *a = argv[i];
			ଯଦି (*a == '-') {
				a++;
				ଯଦି (*a == '-' && !a[1]) /* "--" */
					ଭାଙ୍ଗ;
				ଯଦି (isdigit(*a))
					argv[i] = xasprintf("-w%s", a);
			}
		}
	}

	getopt32(argv, "bsw:", &w_opt);
	width = xatou_range(w_opt, 1, 10000);

	argv += optind;
	ଯଦି (!*argv)
		*--argv = (ଅକ୍ଷର*)"-";

	କର {
		FILE *istream = fopen_or_warn_stdin(*argv);
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା c;
		unsigned column = 0;     /* Screen column where next char will go */
		unsigned offset_out = 0; /* Index in 'line_out' for next char */

		ଯଦି (istream == ଖାଲି) {
			exitcode = EXIT_FAILURE;
			ଜାରି;
		}

		ଯେତେବେଳେ ((c = getc(istream)) != EOF) {
			/* We grow line_out in chunks of 0x1000 bytes */
			ଯଦି ((offset_out & 0xfff) == 0) {
				ଧାଡ଼ି_out = xrealloc(ଧାଡ଼ି_out, offset_out + 0x1000);
			}
 rescan:
			ଧାଡ଼ି_out[offset_out] = c;
			ଯଦି (c == '\n') {
				write2stdout(ଧାଡ଼ି_out, offset_out + 1);
				column = offset_out = 0;
				ଜାରି;
			}
			column = adjust_column(column, c);
			ଯଦି (column <= width || offset_out == 0) {
				/* offset_out == 0 case happens
				 * with small width (say, 1) and tabs.
				 * The very first tab already goes to column 8,
				 * but we must not wrap it */
				offset_out++;
				ଜାରି;
			}

			/* This character would make the line too long.
			 * Print the line plus a newline, and make this character
			 * start the next line */
			ଯଦି (option_mask32 & FLAG_BREAK_SPACES) {
				unsigned i;
				unsigned logical_end;

				/* Look for the last blank. */
				ପାଇଁ (logical_end = offset_out - 1; (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)logical_end >= 0; logical_end--) {
					ଯଦି (!isblank(ଧାଡ଼ି_out[logical_end]))
						ଜାରି;

					/* Found a space or tab.
					 * Output up to and including it, and start a new line */
					logical_end++;
					/*line_out[logical_end] = '\n'; - NO! this nukes one buffered character */
					write2stdout(ଧାଡ଼ି_out, logical_end);
					putchar('\n');
					/* Move the remainder to the beginning of the next line.
					 * The areas being copied here might overlap. */
					memmove(ଧାଡ଼ି_out, ଧାଡ଼ି_out + logical_end, offset_out - logical_end);
					offset_out -= logical_end;
					ପାଇଁ (column = i = 0; i < offset_out; i++) {
						column = adjust_column(column, ଧାଡ଼ି_out[i]);
					}
					ଯାଅ rescan;
				}
				/* No blank found, wrap will split the overlong word */
			}
			/* Output what we accumulated up to now, and start a new line */
			ଧାଡ଼ି_out[offset_out] = '\n';
			write2stdout(ଧାଡ଼ି_out, offset_out + 1);
			column = offset_out = 0;
			ଯାଅ rescan;
		} /* while (not EOF) */

		ଯଦି (offset_out) {
			write2stdout(ଧାଡ଼ି_out, offset_out);
		}

		ଯଦି (fclose_if_not_stdin(istream)) {
			bb_simple_perror_ସନ୍ଦେଶ(*argv);
			exitcode = EXIT_FAILURE;
		}
	} ଯେତେବେଳେ (*++argv);

	fflush_stdout_and_exit(exitcode);
}
