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

#شامل "libbb.h"
#شامل "unicode.h"

/* This is a NOEXEC applet. Be very careful! */

/* Must match getopt32 call */
#تعریف_کرو FLAG_گنتی_بائٹیں        1
#تعریف_کرو FLAG_BREAK_SPACES       2
#تعریف_کرو FLAG_چوڑائی              4

/* Assuming the current column is COLUMN, return the column that
   printing C will move the cursor to.
   The first column is 0. */
ساکن صحیح adjust_عمود(غیرعلامتی عمود, حرف c)
{
	اگر (option_mask32 & FLAG_گنتی_بائٹیں)
		واپس ++عمود;

	اگر (c == '\t')
		واپس عمود + 8 - عمود % 8;

	اگر (c == '\b') {
		اگر ((صحیح)--عمود < 0)
			عمود = 0;
	}
	ورنہ اگر (c == '\r')
		عمود = 0;
	ورنہ { /* just a printable char */
		اگر (unicode_حالت != UNICODE_ON /* every byte is a new char */
		 || (c & 0xc0) != 0x80 /* it isn't a 2nd+ byte of a Unicode char */
		) {
			عمود++;
		}
	}
	واپس عمود;
}

/* Note that this function can write NULs, unlike fputs etc. */
ساکن خالی write2stdout(ثابت خالی *buf, غیرعلامتی سائز)
{
	fwrite(buf, 1, سائز, stdout);
}

صحیح fold_main(صحیح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح fold_main(صحیح argc غیر_مستعمل_PARAM, حرف **argv)
{
	حرف *سطر_برآمد = تہی;
	ثابت حرف *w_opt = "80";
	غیرعلامتی چوڑائی;
	exitcode_t exitcode = EXIT_SUCCESS;

	init_unicode();

	اگر (ENABLE_INCLUDE_SUSv2) {
		/* Turn any numeric options into -w options.  */
		صحیح i;
		برائے (i = 1; argv[i]; i++) {
			ثابت حرف *a = argv[i];
			اگر (*a == '-') {
				a++;
				اگر (*a == '-' && !a[1]) /* "--" */
					توڑو;
				اگر (ہندسہ_ہے(*a))
					argv[i] = xasprintf("-w%s", a);
			}
		}
	}

	getopt32(argv, "bsw:", &w_opt);
	چوڑائی = xatou_range(w_opt, 1, 10000);

	argv += optind;
	اگر (!*argv)
		*--argv = (حرف*)"-";

	کرو {
		FILE *istream = fopen_or_warn_stdin(*argv);
		صحیح c;
		غیرعلامتی عمود = 0;     /* Screen column where next char will go */
		غیرعلامتی انحراف_برآمد = 0; /* Index in 'line_out' for next char */

		اگر (istream == تہی) {
			exitcode = EXIT_FAILURE;
			جاری;
		}

		جب ((c = getc(istream)) != EOF) {
			/* We grow line_out in chunks of 0x1000 bytes */
			اگر ((انحراف_برآمد & 0xfff) == 0) {
				سطر_برآمد = xrealloc(سطر_برآمد, انحراف_برآمد + 0x1000);
			}
 rescan:
			سطر_برآمد[انحراف_برآمد] = c;
			اگر (c == '\n') {
				write2stdout(سطر_برآمد, انحراف_برآمد + 1);
				عمود = انحراف_برآمد = 0;
				جاری;
			}
			عمود = adjust_عمود(عمود, c);
			اگر (عمود <= چوڑائی || انحراف_برآمد == 0) {
				/* offset_out == 0 case happens
				 * with small width (say, 1) and tabs.
				 * The very first tab already goes to column 8,
				 * but we must not wrap it */
				انحراف_برآمد++;
				جاری;
			}

			/* This character would make the line too long.
			 * Print the line plus a newline, and make this character
			 * start the next line */
			اگر (option_mask32 & FLAG_BREAK_SPACES) {
				غیرعلامتی i;
				غیرعلامتی logical_انتہا;

				/* Look for the last blank. */
				برائے (logical_انتہا = انحراف_برآمد - 1; (صحیح)logical_انتہا >= 0; logical_انتہا--) {
					اگر (!isblank(سطر_برآمد[logical_انتہا]))
						جاری;

					/* Found a space or tab.
					 * Output up to and including it, and start a new line */
					logical_انتہا++;
					/*line_out[logical_end] = '\n'; - NO! this nukes one buffered character */
					write2stdout(سطر_برآمد, logical_انتہا);
					حرف_لکھو('\n');
					/* Move the remainder to the beginning of the next line.
					 * The areas being copied here might overlap. */
					memmove(سطر_برآمد, سطر_برآمد + logical_انتہا, انحراف_برآمد - logical_انتہا);
					انحراف_برآمد -= logical_انتہا;
					برائے (عمود = i = 0; i < انحراف_برآمد; i++) {
						عمود = adjust_عمود(عمود, سطر_برآمد[i]);
					}
					جاؤ rescan;
				}
				/* No blank found, wrap will split the overlong word */
			}
			/* Output what we accumulated up to now, and start a new line */
			سطر_برآمد[انحراف_برآمد] = '\n';
			write2stdout(سطر_برآمد, انحراف_برآمد + 1);
			عمود = انحراف_برآمد = 0;
			جاؤ rescan;
		} /* while (not EOF) */

		اگر (انحراف_برآمد) {
			write2stdout(سطر_برآمد, انحراف_برآمد);
		}

		اگر (fclose_if_not_stdin(istream)) {
			bb_simple_perror_پیغام(*argv);
			exitcode = EXIT_FAILURE;
		}
	} جب (*++argv);

	fflush_stdout_and_exit(exitcode);
}
