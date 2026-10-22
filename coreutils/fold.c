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

#अन्तर्भाव "libbb.h"
#अन्तर्भाव "unicode.h"

/* This is a NOEXEC applet. Be very careful! */

/* Must match getopt32 call */
#define FLAG_गणनम्_BYTES        1
#define FLAG_BREAK_SPACES       2
#define FLAG_WIDTH              4

/* Assuming the current column is COLUMN, return the column that
   printing C will move the cursor to.
   The first column is 0. */
स्थिर पूर्णाङ्क adjust_column(unsigned column, वर्ण c)
{
	जइ (option_mask32 & FLAG_गणनम्_BYTES)
		प्रत्यागम ++column;

	जइ (c == '\t')
		प्रत्यागम column + 8 - column % 8;

	जइ (c == '\b') {
		जइ ((पूर्णाङ्क)--column < 0)
			column = 0;
	}
	अण्णहा जइ (c == '\r')
		column = 0;
	अण्णहा { /* just a printable char */
		जइ (unicode_status != UNICODE_ON /* every byte is a new char */
		 || (c & 0xc0) != 0x80 /* it isn't a 2nd+ byte of a Unicode char */
		) {
			column++;
		}
	}
	प्रत्यागम column;
}

/* Note that this function can write NULs, unlike fputs etc. */
स्थिर शून्य write2stdout(ध्रुव शून्य *buf, unsigned size)
{
	fwrite(buf, 1, size, stdout);
}

पूर्णाङ्क fold_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क fold_main(पूर्णाङ्क argc UNUSED_PARAM, वर्ण **argv)
{
	वर्ण *पङ्क्तिः_out = रिक्त;
	ध्रुव वर्ण *w_opt = "80";
	unsigned width;
	exitcode_t exitcode = EXIT_SUCCESS;

	init_unicode();

	जइ (ENABLE_INCLUDE_SUSv2) {
		/* Turn any numeric options into -w options.  */
		पूर्णाङ्क i;
		प्रति (i = 1; argv[i]; i++) {
			ध्रुव वर्ण *a = argv[i];
			जइ (*a == '-') {
				a++;
				जइ (*a == '-' && !a[1]) /* "--" */
					भङ्ग;
				जइ (isdigit(*a))
					argv[i] = xasprintf("-w%s", a);
			}
		}
	}

	getopt32(argv, "bsw:", &w_opt);
	width = xatou_range(w_opt, 1, 10000);

	argv += optind;
	जइ (!*argv)
		*--argv = (वर्ण*)"-";

	करेहि {
		FILE *istream = fopen_or_warn_stdin(*argv);
		पूर्णाङ्क c;
		unsigned column = 0;     /* Screen column where next char will go */
		unsigned offset_out = 0; /* Index in 'line_out' for next char */

		जइ (istream == रिक्त) {
			exitcode = EXIT_FAILURE;
			अनुवर्त;
		}

		जाव ((c = getc(istream)) != EOF) {
			/* We grow line_out in chunks of 0x1000 bytes */
			जइ ((offset_out & 0xfff) == 0) {
				पङ्क्तिः_out = xrealloc(पङ्क्तिः_out, offset_out + 0x1000);
			}
 rescan:
			पङ्क्तिः_out[offset_out] = c;
			जइ (c == '\n') {
				write2stdout(पङ्क्तिः_out, offset_out + 1);
				column = offset_out = 0;
				अनुवर्त;
			}
			column = adjust_column(column, c);
			जइ (column <= width || offset_out == 0) {
				/* offset_out == 0 case happens
				 * with small width (say, 1) and tabs.
				 * The very first tab already goes to column 8,
				 * but we must not wrap it */
				offset_out++;
				अनुवर्त;
			}

			/* This character would make the line too long.
			 * Print the line plus a newline, and make this character
			 * start the next line */
			जइ (option_mask32 & FLAG_BREAK_SPACES) {
				unsigned i;
				unsigned logical_end;

				/* Look for the last blank. */
				प्रति (logical_end = offset_out - 1; (पूर्णाङ्क)logical_end >= 0; logical_end--) {
					जइ (!isblank(पङ्क्तिः_out[logical_end]))
						अनुवर्त;

					/* Found a space or tab.
					 * Output up to and including it, and start a new line */
					logical_end++;
					/*line_out[logical_end] = '\n'; - NO! this nukes one buffered character */
					write2stdout(पङ्क्तिः_out, logical_end);
					putchar('\n');
					/* Move the remainder to the beginning of the next line.
					 * The areas being copied here might overlap. */
					memmove(पङ्क्तिः_out, पङ्क्तिः_out + logical_end, offset_out - logical_end);
					offset_out -= logical_end;
					प्रति (column = i = 0; i < offset_out; i++) {
						column = adjust_column(column, पङ्क्तिः_out[i]);
					}
					गच्छ rescan;
				}
				/* No blank found, wrap will split the overlong word */
			}
			/* Output what we accumulated up to now, and start a new line */
			पङ्क्तिः_out[offset_out] = '\n';
			write2stdout(पङ्क्तिः_out, offset_out + 1);
			column = offset_out = 0;
			गच्छ rescan;
		} /* while (not EOF) */

		जइ (offset_out) {
			write2stdout(पङ्क्तिः_out, offset_out);
		}

		जइ (fclose_if_not_stdin(istream)) {
			bb_simple_perror_सन्देशः(*argv);
			exitcode = EXIT_FAILURE;
		}
	} जाव (*++argv);

	fflush_stdout_and_exit(exitcode);
}
