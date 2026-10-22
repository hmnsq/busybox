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
#تعریف_کړه FLAG_شمېر_بایټونه        1
#تعریف_کړه FLAG_BREAK_SPACES       2
#تعریف_کړه FLAG_پلنوالی              4

/* Assuming the current column is COLUMN, return the column that
   printing C will move the cursor to.
   The first column is 0. */
ساكن صحيح adjust_ستنه(بینښه ستنه, حرف c)
{
	إذا (option_mask32 & FLAG_شمېر_بایټونه)
		أرجع ++ستنه;

	إذا (c == '\t')
		أرجع ستنه + 8 - ستنه % 8;

	إذا (c == '\b') {
		إذا ((صحيح)--ستنه < 0)
			ستنه = 0;
	}
	وإلا إذا (c == '\r')
		ستنه = 0;
	وإلا { /* just a printable char */
		إذا (unicode_حالت != UNICODE_ON /* every byte is a new char */
		 || (c & 0xc0) != 0x80 /* it isn't a 2nd+ byte of a Unicode char */
		) {
			ستنه++;
		}
	}
	أرجع ستنه;
}

/* Note that this function can write NULs, unlike fputs etc. */
ساكن فراغ write2stdout(ثابت فراغ *buf, بینښه اندازه)
{
	fwrite(buf, 1, اندازه, stdout);
}

صحيح fold_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح fold_main(صحيح argc ناکارول_شوی_PARAM, حرف **argv)
{
	حرف *کرښه_بهر = عدم;
	ثابت حرف *w_opt = "80";
	بینښه پلنوالی;
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
				إذا (که_عدد(*a))
					argv[i] = xasprintf("-w%s", a);
			}
		}
	}

	getopt32(argv, "bsw:", &w_opt);
	پلنوالی = xatou_range(w_opt, 1, 10000);

	argv += optind;
	إذا (!*argv)
		*--argv = (حرف*)"-";

	افعل {
		FILE *istream = fopen_or_warn_stdin(*argv);
		صحيح c;
		بینښه ستنه = 0;     /* Screen column where next char will go */
		بینښه دننه_واټن_بهر = 0; /* Index in 'line_out' for next char */

		إذا (istream == عدم) {
			exitcode = EXIT_FAILURE;
			استمر;
		}

		طالما ((c = getc(istream)) != EOF) {
			/* We grow line_out in chunks of 0x1000 bytes */
			إذا ((دننه_واټن_بهر & 0xfff) == 0) {
				کرښه_بهر = xrealloc(کرښه_بهر, دننه_واټن_بهر + 0x1000);
			}
 rescan:
			کرښه_بهر[دننه_واټن_بهر] = c;
			إذا (c == '\n') {
				write2stdout(کرښه_بهر, دننه_واټن_بهر + 1);
				ستنه = دننه_واټن_بهر = 0;
				استمر;
			}
			ستنه = adjust_ستنه(ستنه, c);
			إذا (ستنه <= پلنوالی || دننه_واټن_بهر == 0) {
				/* offset_out == 0 case happens
				 * with small width (say, 1) and tabs.
				 * The very first tab already goes to column 8,
				 * but we must not wrap it */
				دننه_واټن_بهر++;
				استمر;
			}

			/* This character would make the line too long.
			 * Print the line plus a newline, and make this character
			 * start the next line */
			إذا (option_mask32 & FLAG_BREAK_SPACES) {
				بینښه i;
				بینښه logical_پای_ټکی;

				/* Look for the last blank. */
				لكل (logical_پای_ټکی = دننه_واټن_بهر - 1; (صحيح)logical_پای_ټکی >= 0; logical_پای_ټکی--) {
					إذا (!isblank(کرښه_بهر[logical_پای_ټکی]))
						استمر;

					/* Found a space or tab.
					 * Output up to and including it, and start a new line */
					logical_پای_ټکی++;
					/*line_out[logical_end] = '\n'; - NO! this nukes one buffered character */
					write2stdout(کرښه_بهر, logical_پای_ټکی);
					توری_چاپ_کړه('\n');
					/* Move the remainder to the beginning of the next line.
					 * The areas being copied here might overlap. */
					memmove(کرښه_بهر, کرښه_بهر + logical_پای_ټکی, دننه_واټن_بهر - logical_پای_ټکی);
					دننه_واټن_بهر -= logical_پای_ټکی;
					لكل (ستنه = i = 0; i < دننه_واټن_بهر; i++) {
						ستنه = adjust_ستنه(ستنه, کرښه_بهر[i]);
					}
					اذهب rescan;
				}
				/* No blank found, wrap will split the overlong word */
			}
			/* Output what we accumulated up to now, and start a new line */
			کرښه_بهر[دننه_واټن_بهر] = '\n';
			write2stdout(کرښه_بهر, دننه_واټن_بهر + 1);
			ستنه = دننه_واټن_بهر = 0;
			اذهب rescan;
		} /* while (not EOF) */

		إذا (دننه_واټن_بهر) {
			write2stdout(کرښه_بهر, دننه_واټن_بهر);
		}

		إذا (fclose_if_not_stdin(istream)) {
			bb_simple_perror_پیغام(*argv);
			exitcode = EXIT_FAILURE;
		}
	} طالما (*++argv);

	fflush_stdout_and_exit(exitcode);
}
