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
#تعریف_کرو FLAG_گنتی_بائٹاں        1
#تعریف_کرو FLAG_BREAK_SPACES       2
#تعریف_کرو FLAG_چوڑائی              4

/* Assuming the current column is COLUMN, return the column that
   printing C will move the cursor to.
   The first column is 0. */
ساكن صحيح adjust_کالم(بنانشانی کالم, حرف c)
{
	إذا (option_mask32 & FLAG_گنتی_بائٹاں)
		أرجع ++کالم;

	إذا (c == '\t')
		أرجع کالم + 8 - کالم % 8;

	إذا (c == '\b') {
		إذا ((صحيح)--کالم < 0)
			کالم = 0;
	}
	وإلا إذا (c == '\r')
		کالم = 0;
	وإلا { /* just a printable char */
		إذا (unicode_حالت != UNICODE_ON /* every byte is a new char */
		 || (c & 0xc0) != 0x80 /* it isn't a 2nd+ byte of a Unicode char */
		) {
			کالم++;
		}
	}
	أرجع کالم;
}

/* Note that this function can write NULs, unlike fputs etc. */
ساكن فراغ write2stdout(ثابت فراغ *buf, بنانشانی ناپ)
{
	fwrite(buf, 1, ناپ, stdout);
}

صحيح fold_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح fold_main(صحيح argc بن_ورتیا_PARAM, حرف **argv)
{
	حرف *سطر_باہرلا = عدم;
	ثابت حرف *w_opt = "80";
	بنانشانی چوڑائی;
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
				إذا (ہندسہ_اے(*a))
					argv[i] = xasprintf("-w%s", a);
			}
		}
	}

	getopt32(argv, "bsw:", &w_opt);
	چوڑائی = xatou_range(w_opt, 1, 10000);

	argv += optind;
	إذا (!*argv)
		*--argv = (حرف*)"-";

	افعل {
		FILE *istream = fopen_or_warn_stdin(*argv);
		صحيح c;
		بنانشانی کالم = 0;     /* Screen column where next char will go */
		بنانشانی وتھ_باہرلا = 0; /* Index in 'line_out' for next char */

		إذا (istream == عدم) {
			exitcode = EXIT_FAILURE;
			استمر;
		}

		طالما ((c = getc(istream)) != EOF) {
			/* We grow line_out in chunks of 0x1000 bytes */
			إذا ((وتھ_باہرلا & 0xfff) == 0) {
				سطر_باہرلا = xrealloc(سطر_باہرلا, وتھ_باہرلا + 0x1000);
			}
 rescan:
			سطر_باہرلا[وتھ_باہرلا] = c;
			إذا (c == '\n') {
				write2stdout(سطر_باہرلا, وتھ_باہرلا + 1);
				کالم = وتھ_باہرلا = 0;
				استمر;
			}
			کالم = adjust_کالم(کالم, c);
			إذا (کالم <= چوڑائی || وتھ_باہرلا == 0) {
				/* offset_out == 0 case happens
				 * with small width (say, 1) and tabs.
				 * The very first tab already goes to column 8,
				 * but we must not wrap it */
				وتھ_باہرلا++;
				استمر;
			}

			/* This character would make the line too long.
			 * Print the line plus a newline, and make this character
			 * start the next line */
			إذا (option_mask32 & FLAG_BREAK_SPACES) {
				بنانشانی i;
				بنانشانی logical_سرا;

				/* Look for the last blank. */
				لكل (logical_سرا = وتھ_باہرلا - 1; (صحيح)logical_سرا >= 0; logical_سرا--) {
					إذا (!isblank(سطر_باہرلا[logical_سرا]))
						استمر;

					/* Found a space or tab.
					 * Output up to and including it, and start a new line */
					logical_سرا++;
					/*line_out[logical_end] = '\n'; - NO! this nukes one buffered character */
					write2stdout(سطر_باہرلا, logical_سرا);
					حرف_لکھو('\n');
					/* Move the remainder to the beginning of the next line.
					 * The areas being copied here might overlap. */
					memmove(سطر_باہرلا, سطر_باہرلا + logical_سرا, وتھ_باہرلا - logical_سرا);
					وتھ_باہرلا -= logical_سرا;
					لكل (کالم = i = 0; i < وتھ_باہرلا; i++) {
						کالم = adjust_کالم(کالم, سطر_باہرلا[i]);
					}
					اذهب rescan;
				}
				/* No blank found, wrap will split the overlong word */
			}
			/* Output what we accumulated up to now, and start a new line */
			سطر_باہرلا[وتھ_باہرلا] = '\n';
			write2stdout(سطر_باہرلا, وتھ_باہرلا + 1);
			کالم = وتھ_باہرلا = 0;
			اذهب rescan;
		} /* while (not EOF) */

		إذا (وتھ_باہرلا) {
			write2stdout(سطر_باہرلا, وتھ_باہرلا);
		}

		إذا (fclose_if_not_stdin(istream)) {
			bb_simple_perror_پیغام(*argv);
			exitcode = EXIT_FAILURE;
		}
	} طالما (*++argv);

	fflush_stdout_and_exit(exitcode);
}
