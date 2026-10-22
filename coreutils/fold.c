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

#അന്തര്ഭാവ "libbb.h"
#അന്തര്ഭാവ "unicode.h"

/* This is a NOEXEC applet. Be very careful! */

/* Must match getopt32 call */
#define FLAG_എണ്ണം_BYTES        1
#define FLAG_BREAK_SPACES       2
#define FLAG_WIDTH              4

/* Assuming the current column is COLUMN, return the column that
   printing C will move the cursor to.
   The first column is 0. */
സ്ഥിതം പൂർണ്ണസംഖ്യ adjust_column(unsigned column, അക്ഷരം c)
{
	എങ്കിൽ (option_mask32 & FLAG_എണ്ണം_BYTES)
		തിരികെ ++column;

	എങ്കിൽ (c == '\t')
		തിരികെ column + 8 - column % 8;

	എങ്കിൽ (c == '\b') {
		എങ്കിൽ ((പൂർണ്ണസംഖ്യ)--column < 0)
			column = 0;
	}
	അല്ലെങ്കിൽ എങ്കിൽ (c == '\r')
		column = 0;
	അല്ലെങ്കിൽ { /* just a printable char */
		എങ്കിൽ (unicode_status != UNICODE_ON /* every byte is a new char */
		 || (c & 0xc0) != 0x80 /* it isn't a 2nd+ byte of a Unicode char */
		) {
			column++;
		}
	}
	തിരികെ column;
}

/* Note that this function can write NULs, unlike fputs etc. */
സ്ഥിതം ശൂന്യം write2stdout(സ്ഥിരം ശൂന്യം *buf, unsigned size)
{
	fwrite(buf, 1, size, stdout);
}

പൂർണ്ണസംഖ്യ fold_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) MAIN_EXTERNALLY_VISIBLE;
പൂർണ്ണസംഖ്യ fold_main(പൂർണ്ണസംഖ്യ argc UNUSED_PARAM, അക്ഷരം **argv)
{
	അക്ഷരം *വരി_out = ശൂന്യ;
	സ്ഥിരം അക്ഷരം *w_opt = "80";
	unsigned width;
	exitcode_t exitcode = EXIT_SUCCESS;

	init_unicode();

	എങ്കിൽ (ENABLE_INCLUDE_SUSv2) {
		/* Turn any numeric options into -w options.  */
		പൂർണ്ണസംഖ്യ i;
		വേണ്ടി (i = 1; argv[i]; i++) {
			സ്ഥിരം അക്ഷരം *a = argv[i];
			എങ്കിൽ (*a == '-') {
				a++;
				എങ്കിൽ (*a == '-' && !a[1]) /* "--" */
					നിർത്തുക;
				എങ്കിൽ (isdigit(*a))
					argv[i] = xasprintf("-w%s", a);
			}
		}
	}

	getopt32(argv, "bsw:", &w_opt);
	width = xatou_range(w_opt, 1, 10000);

	argv += optind;
	എങ്കിൽ (!*argv)
		*--argv = (അക്ഷരം*)"-";

	ചെയ്യുക {
		FILE *istream = fopen_or_warn_stdin(*argv);
		പൂർണ്ണസംഖ്യ c;
		unsigned column = 0;     /* Screen column where next char will go */
		unsigned offset_out = 0; /* Index in 'line_out' for next char */

		എങ്കിൽ (istream == ശൂന്യ) {
			exitcode = EXIT_FAILURE;
			തുടരുക;
		}

		ആകുമ്പോൾ ((c = getc(istream)) != EOF) {
			/* We grow line_out in chunks of 0x1000 bytes */
			എങ്കിൽ ((offset_out & 0xfff) == 0) {
				വരി_out = xrealloc(വരി_out, offset_out + 0x1000);
			}
 rescan:
			വരി_out[offset_out] = c;
			എങ്കിൽ (c == '\n') {
				write2stdout(വരി_out, offset_out + 1);
				column = offset_out = 0;
				തുടരുക;
			}
			column = adjust_column(column, c);
			എങ്കിൽ (column <= width || offset_out == 0) {
				/* offset_out == 0 case happens
				 * with small width (say, 1) and tabs.
				 * The very first tab already goes to column 8,
				 * but we must not wrap it */
				offset_out++;
				തുടരുക;
			}

			/* This character would make the line too long.
			 * Print the line plus a newline, and make this character
			 * start the next line */
			എങ്കിൽ (option_mask32 & FLAG_BREAK_SPACES) {
				unsigned i;
				unsigned logical_end;

				/* Look for the last blank. */
				വേണ്ടി (logical_end = offset_out - 1; (പൂർണ്ണസംഖ്യ)logical_end >= 0; logical_end--) {
					എങ്കിൽ (!isblank(വരി_out[logical_end]))
						തുടരുക;

					/* Found a space or tab.
					 * Output up to and including it, and start a new line */
					logical_end++;
					/*line_out[logical_end] = '\n'; - NO! this nukes one buffered character */
					write2stdout(വരി_out, logical_end);
					putchar('\n');
					/* Move the remainder to the beginning of the next line.
					 * The areas being copied here might overlap. */
					memmove(വരി_out, വരി_out + logical_end, offset_out - logical_end);
					offset_out -= logical_end;
					വേണ്ടി (column = i = 0; i < offset_out; i++) {
						column = adjust_column(column, വരി_out[i]);
					}
					പോകുക rescan;
				}
				/* No blank found, wrap will split the overlong word */
			}
			/* Output what we accumulated up to now, and start a new line */
			വരി_out[offset_out] = '\n';
			write2stdout(വരി_out, offset_out + 1);
			column = offset_out = 0;
			പോകുക rescan;
		} /* while (not EOF) */

		എങ്കിൽ (offset_out) {
			write2stdout(വരി_out, offset_out);
		}

		എങ്കിൽ (fclose_if_not_stdin(istream)) {
			bb_simple_perror_സന്ദേശം(*argv);
			exitcode = EXIT_FAILURE;
		}
	} ആകുമ്പോൾ (*++argv);

	fflush_stdout_and_exit(exitcode);
}
