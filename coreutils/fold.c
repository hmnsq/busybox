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
#ವ್ಯಾಖ್ಯಾನಿಸು FLAG_ಎಣಿಕೆ_ಬೈಟುಗಳು        1
#ವ್ಯಾಖ್ಯಾನಿಸು FLAG_BREAK_SPACES       2
#ವ್ಯಾಖ್ಯಾನಿಸು FLAG_ಅಗಲ              4

/* Assuming the current column is COLUMN, return the column that
   printing C will move the cursor to.
   The first column is 0. */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ adjust_ಸ್ತಂಭ(ಚಿಹ್ನೆರಹಿತ ಸ್ತಂಭ, ಅಕ್ಷರ c)
{
	ಆದರೆ (option_mask32 & FLAG_ಎಣಿಕೆ_ಬೈಟುಗಳು)
		ಹಿಂತಿರುಗಿ ++ಸ್ತಂಭ;

	ಆದರೆ (c == '\t')
		ಹಿಂತಿರುಗಿ ಸ್ತಂಭ + 8 - ಸ್ತಂಭ % 8;

	ಆದರೆ (c == '\b') {
		ಆದರೆ ((ಪೂರ್ಣಾಂಕ)--ಸ್ತಂಭ < 0)
			ಸ್ತಂಭ = 0;
	}
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (c == '\r')
		ಸ್ತಂಭ = 0;
	ಇಲ್ಲದಿದ್ದರೆ { /* just a printable char */
		ಆದರೆ (unicode_ಸ್ಥಿತಿ != UNICODE_ON /* every byte is a new char */
		 || (c & 0xc0) != 0x80 /* it isn't a 2nd+ byte of a Unicode char */
		) {
			ಸ್ತಂಭ++;
		}
	}
	ಹಿಂತಿರುಗಿ ಸ್ತಂಭ;
}

/* Note that this function can write NULs, unlike fputs etc. */
ಸ್ಥಿರ ಶೂನ್ಯ write2stdout(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *buf, ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_2)
{
	fwrite(buf, 1, ಗಾತ್ರ_2, stdout);
}

ಪೂರ್ಣಾಂಕ fold_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ fold_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	ಅಕ್ಷರ *ಸಾಲು_ಹೊರಹೋಗು = ಖಾಲಿ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *w_opt = "80";
	ಚಿಹ್ನೆರಹಿತ ಅಗಲ;
	exitcode_t exitcode = EXIT_SUCCESS;

	init_unicode();

	ಆದರೆ (ENABLE_INCLUDE_SUSv2) {
		/* Turn any numeric options into -w options.  */
		ಪೂರ್ಣಾಂಕ i;
		ಗಾಗಿ (i = 1; argv[i]; i++) {
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *a = argv[i];
			ಆದರೆ (*a == '-') {
				a++;
				ಆದರೆ (*a == '-' && !a[1]) /* "--" */
					ನಿಲ್ಲಿಸು;
				ಆದರೆ (ಅಂಕಿಯೇ(*a))
					argv[i] = xasprintf("-w%s", a);
			}
		}
	}

	getopt32(argv, "bsw:", &w_opt);
	ಅಗಲ = xatou_range(w_opt, 1, 10000);

	argv += optind;
	ಆದರೆ (!*argv)
		*--argv = (ಅಕ್ಷರ*)"-";

	ಮಾಡು {
		FILE *istream = fopen_or_warn_stdin(*argv);
		ಪೂರ್ಣಾಂಕ c;
		ಚಿಹ್ನೆರಹಿತ ಸ್ತಂಭ = 0;     /* Screen column where next char will go */
		ಚಿಹ್ನೆರಹಿತ ಸ್ಥಾನಾಂತರ_ಹೊರಹೋಗು = 0; /* Index in 'line_out' for next char */

		ಆದರೆ (istream == ಖಾಲಿ) {
			exitcode = EXIT_FAILURE;
			ಮುಂದುವರಿ;
		}

		ಆಗ ((c = getc(istream)) != EOF) {
			/* We grow line_out in chunks of 0x1000 bytes */
			ಆದರೆ ((ಸ್ಥಾನಾಂತರ_ಹೊರಹೋಗು & 0xfff) == 0) {
				ಸಾಲು_ಹೊರಹೋಗು = xrealloc(ಸಾಲು_ಹೊರಹೋಗು, ಸ್ಥಾನಾಂತರ_ಹೊರಹೋಗು + 0x1000);
			}
 rescan:
			ಸಾಲು_ಹೊರಹೋಗು[ಸ್ಥಾನಾಂತರ_ಹೊರಹೋಗು] = c;
			ಆದರೆ (c == '\n') {
				write2stdout(ಸಾಲು_ಹೊರಹೋಗು, ಸ್ಥಾನಾಂತರ_ಹೊರಹೋಗು + 1);
				ಸ್ತಂಭ = ಸ್ಥಾನಾಂತರ_ಹೊರಹೋಗು = 0;
				ಮುಂದುವರಿ;
			}
			ಸ್ತಂಭ = adjust_ಸ್ತಂಭ(ಸ್ತಂಭ, c);
			ಆದರೆ (ಸ್ತಂಭ <= ಅಗಲ || ಸ್ಥಾನಾಂತರ_ಹೊರಹೋಗು == 0) {
				/* offset_out == 0 case happens
				 * with small width (say, 1) and tabs.
				 * The very first tab already goes to column 8,
				 * but we must not wrap it */
				ಸ್ಥಾನಾಂತರ_ಹೊರಹೋಗು++;
				ಮುಂದುವರಿ;
			}

			/* This character would make the line too long.
			 * Print the line plus a newline, and make this character
			 * start the next line */
			ಆದರೆ (option_mask32 & FLAG_BREAK_SPACES) {
				ಚಿಹ್ನೆರಹಿತ i;
				ಚಿಹ್ನೆರಹಿತ logical_ಕೊನೆ;

				/* Look for the last blank. */
				ಗಾಗಿ (logical_ಕೊನೆ = ಸ್ಥಾನಾಂತರ_ಹೊರಹೋಗು - 1; (ಪೂರ್ಣಾಂಕ)logical_ಕೊನೆ >= 0; logical_ಕೊನೆ--) {
					ಆದರೆ (!isblank(ಸಾಲು_ಹೊರಹೋಗು[logical_ಕೊನೆ]))
						ಮುಂದುವರಿ;

					/* Found a space or tab.
					 * Output up to and including it, and start a new line */
					logical_ಕೊನೆ++;
					/*line_out[logical_end] = '\n'; - NO! this nukes one buffered character */
					write2stdout(ಸಾಲು_ಹೊರಹೋಗು, logical_ಕೊನೆ);
					ಅಕ್ಷರ_ಬರೆ('\n');
					/* Move the remainder to the beginning of the next line.
					 * The areas being copied here might overlap. */
					memmove(ಸಾಲು_ಹೊರಹೋಗು, ಸಾಲು_ಹೊರಹೋಗು + logical_ಕೊನೆ, ಸ್ಥಾನಾಂತರ_ಹೊರಹೋಗು - logical_ಕೊನೆ);
					ಸ್ಥಾನಾಂತರ_ಹೊರಹೋಗು -= logical_ಕೊನೆ;
					ಗಾಗಿ (ಸ್ತಂಭ = i = 0; i < ಸ್ಥಾನಾಂತರ_ಹೊರಹೋಗು; i++) {
						ಸ್ತಂಭ = adjust_ಸ್ತಂಭ(ಸ್ತಂಭ, ಸಾಲು_ಹೊರಹೋಗು[i]);
					}
					ಹೋಗು rescan;
				}
				/* No blank found, wrap will split the overlong word */
			}
			/* Output what we accumulated up to now, and start a new line */
			ಸಾಲು_ಹೊರಹೋಗು[ಸ್ಥಾನಾಂತರ_ಹೊರಹೋಗು] = '\n';
			write2stdout(ಸಾಲು_ಹೊರಹೋಗು, ಸ್ಥಾನಾಂತರ_ಹೊರಹೋಗು + 1);
			ಸ್ತಂಭ = ಸ್ಥಾನಾಂತರ_ಹೊರಹೋಗು = 0;
			ಹೋಗು rescan;
		} /* while (not EOF) */

		ಆದರೆ (ಸ್ಥಾನಾಂತರ_ಹೊರಹೋಗು) {
			write2stdout(ಸಾಲು_ಹೊರಹೋಗು, ಸ್ಥಾನಾಂತರ_ಹೊರಹೋಗು);
		}

		ಆದರೆ (fclose_if_not_stdin(istream)) {
			bb_simple_perror_ಸಂದೇಶ(*argv);
			exitcode = EXIT_FAILURE;
		}
	} ಆಗ (*++argv);

	fflush_stdout_and_exit(exitcode);
}
