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

#ضمن "libbb.h"
#ضمن "unicode.h"

/* This is a NOEXEC applet. Be very careful! */

/* Must match getopt32 call */
#define FLAG_تعداد_BYTES        1
#define FLAG_BREAK_SPACES       2
#define FLAG_WIDTH              4

/* Assuming the current column is COLUMN, return the column that
   printing C will move the cursor to.
   The first column is 0. */
ساكن صحيح adjust_column(unsigned column, حرف c)
{
	إذا (option_mask32 & FLAG_تعداد_BYTES)
		إرجاع ++column;

	إذا (c == '\t')
		إرجاع column + 8 - column % 8;

	إذا (c == '\b') {
		إذا ((صحيح)--column < 0)
			column = 0;
	}
	وإلا إذا (c == '\r')
		column = 0;
	وإلا { /* just a printable char */
		إذا (unicode_status != UNICODE_ON /* every byte is a new char */
		 || (c & 0xc0) != 0x80 /* it isn't a 2nd+ byte of a Unicode char */
		) {
			column++;
		}
	}
	إرجاع column;
}

/* Note that this function can write NULs, unlike fputs etc. */
ساكن فراغ write2stdout(ثابت فراغ *buf, unsigned size)
{
	fwrite(buf, 1, size, stdout);
}

صحيح fold_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح fold_main(صحيح argc UNUSED_PARAM, حرف **argv)
{
	حرف *سطر_out = عدم;
	ثابت حرف *w_opt = "80";
	unsigned width;
	exitcode_t exitcode = EXIT_SUCCESS;

	init_unicode();

	إذا (ENABLE_INCLUDE_SUSv2) {
		/* Turn any numeric options into -w options.  */
		صحيح i;
		لكل (i = 1; argv[i]; i++) {
			ثابت حرف *a = argv[i];
			إذا (*a == '-') {
				a++;
				إذا (*a == '-' && !a[1]) /* "--" */
					اكسر;
				إذا (isdigit(*a))
					argv[i] = xasprintf("-w%s", a);
			}
		}
	}

	getopt32(argv, "bsw:", &w_opt);
	width = xatou_range(w_opt, 1, 10000);

	argv += optind;
	إذا (!*argv)
		*--argv = (حرف*)"-";

	افعل {
		FILE *istream = fopen_or_warn_stdin(*argv);
		صحيح c;
		unsigned column = 0;     /* Screen column where next char will go */
		unsigned offset_out = 0; /* Index in 'line_out' for next char */

		إذا (istream == عدم) {
			exitcode = EXIT_FAILURE;
			استمر;
		}

		بينما ((c = getc(istream)) != EOF) {
			/* We grow line_out in chunks of 0x1000 bytes */
			إذا ((offset_out & 0xfff) == 0) {
				سطر_out = xrealloc(سطر_out, offset_out + 0x1000);
			}
 rescan:
			سطر_out[offset_out] = c;
			إذا (c == '\n') {
				write2stdout(سطر_out, offset_out + 1);
				column = offset_out = 0;
				استمر;
			}
			column = adjust_column(column, c);
			إذا (column <= width || offset_out == 0) {
				/* offset_out == 0 case happens
				 * with small width (say, 1) and tabs.
				 * The very first tab already goes to column 8,
				 * but we must not wrap it */
				offset_out++;
				استمر;
			}

			/* This character would make the line too long.
			 * Print the line plus a newline, and make this character
			 * start the next line */
			إذا (option_mask32 & FLAG_BREAK_SPACES) {
				unsigned i;
				unsigned logical_end;

				/* Look for the last blank. */
				لكل (logical_end = offset_out - 1; (صحيح)logical_end >= 0; logical_end--) {
					إذا (!isblank(سطر_out[logical_end]))
						استمر;

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
					لكل (column = i = 0; i < offset_out; i++) {
						column = adjust_column(column, سطر_out[i]);
					}
					اذهب rescan;
				}
				/* No blank found, wrap will split the overlong word */
			}
			/* Output what we accumulated up to now, and start a new line */
			سطر_out[offset_out] = '\n';
			write2stdout(سطر_out, offset_out + 1);
			column = offset_out = 0;
			اذهب rescan;
		} /* while (not EOF) */

		إذا (offset_out) {
			write2stdout(سطر_out, offset_out);
		}

		إذا (fclose_if_not_stdin(istream)) {
			bb_simple_perror_رسالة(*argv);
			exitcode = EXIT_FAILURE;
		}
	} بينما (*++argv);

	fflush_stdout_and_exit(exitcode);
}
