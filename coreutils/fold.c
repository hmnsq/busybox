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
#وصف_ڪريو FLAG_ڳڻپ_بائيٽون        1
#وصف_ڪريو FLAG_BREAK_SPACES       2
#وصف_ڪريو FLAG_ويڪر              4

/* Assuming the current column is COLUMN, return the column that
   printing C will move the cursor to.
   The first column is 0. */
ساكن صحيح adjust_ڪالم(بنانشان ڪالم, حرف c)
{
	إذا (option_mask32 & FLAG_ڳڻپ_بائيٽون)
		أرجع ++ڪالم;

	إذا (c == '\t')
		أرجع ڪالم + 8 - ڪالم % 8;

	إذا (c == '\b') {
		إذا ((صحيح)--ڪالم < 0)
			ڪالم = 0;
	}
	وإلا إذا (c == '\r')
		ڪالم = 0;
	وإلا { /* just a printable char */
		إذا (unicode_حالت != UNICODE_ON /* every byte is a new char */
		 || (c & 0xc0) != 0x80 /* it isn't a 2nd+ byte of a Unicode char */
		) {
			ڪالم++;
		}
	}
	أرجع ڪالم;
}

/* Note that this function can write NULs, unlike fputs etc. */
ساكن فراغ write2stdout(ثابت فراغ *buf, بنانشان سائيز)
{
	fwrite(buf, 1, سائيز, stdout);
}

صحيح fold_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح fold_main(صحيح argc اڻ_استعمال_ٿيل_PARAM, حرف **argv)
{
	حرف *سٽ_ٻاهريون = عدم;
	ثابت حرف *w_opt = "80";
	بنانشان ويڪر;
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
				إذا (انگ_آهي(*a))
					argv[i] = xasprintf("-w%s", a);
			}
		}
	}

	getopt32(argv, "bsw:", &w_opt);
	ويڪر = xatou_range(w_opt, 1, 10000);

	argv += optind;
	إذا (!*argv)
		*--argv = (حرف*)"-";

	افعل {
		FILE *istream = fopen_or_warn_stdin(*argv);
		صحيح c;
		بنانشان ڪالم = 0;     /* Screen column where next char will go */
		بنانشان وٿي_ٻاهريون = 0; /* Index in 'line_out' for next char */

		إذا (istream == عدم) {
			exitcode = EXIT_FAILURE;
			استمر;
		}

		طالما ((c = getc(istream)) != EOF) {
			/* We grow line_out in chunks of 0x1000 bytes */
			إذا ((وٿي_ٻاهريون & 0xfff) == 0) {
				سٽ_ٻاهريون = xrealloc(سٽ_ٻاهريون, وٿي_ٻاهريون + 0x1000);
			}
 rescan:
			سٽ_ٻاهريون[وٿي_ٻاهريون] = c;
			إذا (c == '\n') {
				write2stdout(سٽ_ٻاهريون, وٿي_ٻاهريون + 1);
				ڪالم = وٿي_ٻاهريون = 0;
				استمر;
			}
			ڪالم = adjust_ڪالم(ڪالم, c);
			إذا (ڪالم <= ويڪر || وٿي_ٻاهريون == 0) {
				/* offset_out == 0 case happens
				 * with small width (say, 1) and tabs.
				 * The very first tab already goes to column 8,
				 * but we must not wrap it */
				وٿي_ٻاهريون++;
				استمر;
			}

			/* This character would make the line too long.
			 * Print the line plus a newline, and make this character
			 * start the next line */
			إذا (option_mask32 & FLAG_BREAK_SPACES) {
				بنانشان i;
				بنانشان logical_پڇاڙي;

				/* Look for the last blank. */
				لكل (logical_پڇاڙي = وٿي_ٻاهريون - 1; (صحيح)logical_پڇاڙي >= 0; logical_پڇاڙي--) {
					إذا (!isblank(سٽ_ٻاهريون[logical_پڇاڙي]))
						استمر;

					/* Found a space or tab.
					 * Output up to and including it, and start a new line */
					logical_پڇاڙي++;
					/*line_out[logical_end] = '\n'; - NO! this nukes one buffered character */
					write2stdout(سٽ_ٻاهريون, logical_پڇاڙي);
					اکر_لکو('\n');
					/* Move the remainder to the beginning of the next line.
					 * The areas being copied here might overlap. */
					memmove(سٽ_ٻاهريون, سٽ_ٻاهريون + logical_پڇاڙي, وٿي_ٻاهريون - logical_پڇاڙي);
					وٿي_ٻاهريون -= logical_پڇاڙي;
					لكل (ڪالم = i = 0; i < وٿي_ٻاهريون; i++) {
						ڪالم = adjust_ڪالم(ڪالم, سٽ_ٻاهريون[i]);
					}
					اذهب rescan;
				}
				/* No blank found, wrap will split the overlong word */
			}
			/* Output what we accumulated up to now, and start a new line */
			سٽ_ٻاهريون[وٿي_ٻاهريون] = '\n';
			write2stdout(سٽ_ٻاهريون, وٿي_ٻاهريون + 1);
			ڪالم = وٿي_ٻاهريون = 0;
			اذهب rescan;
		} /* while (not EOF) */

		إذا (وٿي_ٻاهريون) {
			write2stdout(سٽ_ٻاهريون, وٿي_ٻاهريون);
		}

		إذا (fclose_if_not_stdin(istream)) {
			bb_simple_perror_نياپو(*argv);
			exitcode = EXIT_FAILURE;
		}
	} طالما (*++argv);

	fflush_stdout_and_exit(exitcode);
}
