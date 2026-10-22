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
#تعریف_کن FLAG_شمار_بایتها        1
#تعریف_کن FLAG_BREAK_SPACES       2
#تعریف_کن FLAG_عرض              4

/* Assuming the current column is COLUMN, return the column that
   printing C will move the cursor to.
   The first column is 0. */
ساكن صحيح adjust_ستونی(بیعلامت ستونی, حرف c)
{
	إذا (option_mask32 & FLAG_شمار_بایتها)
		أرجع ++ستونی;

	إذا (c == '\t')
		أرجع ستونی + 8 - ستونی % 8;

	إذا (c == '\b') {
		إذا ((صحيح)--ستونی < 0)
			ستونی = 0;
	}
	وإلا إذا (c == '\r')
		ستونی = 0;
	وإلا { /* just a printable char */
		إذا (unicode_وضعیت != UNICODE_ON /* every byte is a new char */
		 || (c & 0xc0) != 0x80 /* it isn't a 2nd+ byte of a Unicode char */
		) {
			ستونی++;
		}
	}
	أرجع ستونی;
}

/* Note that this function can write NULs, unlike fputs etc. */
ساكن فراغ write2stdout(ثابت فراغ *buf, بیعلامت اندازه)
{
	fwrite(buf, 1, اندازه, stdout);
}

صحيح fold_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح fold_main(صحيح argc استفادهنشده_PARAM, حرف **argv)
{
	حرف *سطر_بیرون = عدم;
	ثابت حرف *w_opt = "80";
	بیعلامت عرض;
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
				إذا (آیا_رقم(*a))
					argv[i] = xasprintf("-w%s", a);
			}
		}
	}

	getopt32(argv, "bsw:", &w_opt);
	عرض = xatou_range(w_opt, 1, 10000);

	argv += optind;
	إذا (!*argv)
		*--argv = (حرف*)"-";

	افعل {
		FILE *istream = fopen_or_warn_stdin(*argv);
		صحيح c;
		بیعلامت ستونی = 0;     /* Screen column where next char will go */
		بیعلامت جابه_جایی_بیرون = 0; /* Index in 'line_out' for next char */

		إذا (istream == عدم) {
			exitcode = EXIT_FAILURE;
			استمر;
		}

		طالما ((c = getc(istream)) != EOF) {
			/* We grow line_out in chunks of 0x1000 bytes */
			إذا ((جابه_جایی_بیرون & 0xfff) == 0) {
				سطر_بیرون = xrealloc(سطر_بیرون, جابه_جایی_بیرون + 0x1000);
			}
 rescan:
			سطر_بیرون[جابه_جایی_بیرون] = c;
			إذا (c == '\n') {
				write2stdout(سطر_بیرون, جابه_جایی_بیرون + 1);
				ستونی = جابه_جایی_بیرون = 0;
				استمر;
			}
			ستونی = adjust_ستونی(ستونی, c);
			إذا (ستونی <= عرض || جابه_جایی_بیرون == 0) {
				/* offset_out == 0 case happens
				 * with small width (say, 1) and tabs.
				 * The very first tab already goes to column 8,
				 * but we must not wrap it */
				جابه_جایی_بیرون++;
				استمر;
			}

			/* This character would make the line too long.
			 * Print the line plus a newline, and make this character
			 * start the next line */
			إذا (option_mask32 & FLAG_BREAK_SPACES) {
				بیعلامت i;
				بیعلامت logical_انتها;

				/* Look for the last blank. */
				لكل (logical_انتها = جابه_جایی_بیرون - 1; (صحيح)logical_انتها >= 0; logical_انتها--) {
					إذا (!isblank(سطر_بیرون[logical_انتها]))
						استمر;

					/* Found a space or tab.
					 * Output up to and including it, and start a new line */
					logical_انتها++;
					/*line_out[logical_end] = '\n'; - NO! this nukes one buffered character */
					write2stdout(سطر_بیرون, logical_انتها);
					حرف_چاپ_کن('\n');
					/* Move the remainder to the beginning of the next line.
					 * The areas being copied here might overlap. */
					memmove(سطر_بیرون, سطر_بیرون + logical_انتها, جابه_جایی_بیرون - logical_انتها);
					جابه_جایی_بیرون -= logical_انتها;
					لكل (ستونی = i = 0; i < جابه_جایی_بیرون; i++) {
						ستونی = adjust_ستونی(ستونی, سطر_بیرون[i]);
					}
					اذهب rescan;
				}
				/* No blank found, wrap will split the overlong word */
			}
			/* Output what we accumulated up to now, and start a new line */
			سطر_بیرون[جابه_جایی_بیرون] = '\n';
			write2stdout(سطر_بیرون, جابه_جایی_بیرون + 1);
			ستونی = جابه_جایی_بیرون = 0;
			اذهب rescan;
		} /* while (not EOF) */

		إذا (جابه_جایی_بیرون) {
			write2stdout(سطر_بیرون, جابه_جایی_بیرون);
		}

		إذا (fclose_if_not_stdin(istream)) {
			bb_simple_perror_پیام(*argv);
			exitcode = EXIT_FAILURE;
		}
	} طالما (*++argv);

	fflush_stdout_and_exit(exitcode);
}
