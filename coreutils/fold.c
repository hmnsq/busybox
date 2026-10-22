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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "unicode.h"

/* This is a NOEXEC applet. Be very careful! */

/* Must match getopt32 call */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLAG_ਗਿਣਤੀ_ਬਾਈਟਾਂ        1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLAG_BREAK_SPACES       2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLAG_ਚੌੜਾਈ              4

/* Assuming the current column is COLUMN, return the column that
   printing C will move the cursor to.
   The first column is 0. */
ਸਥਿਰ ਪੂਰਨ adjust_ਕਾਲਮ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਕਾਲਮ, ਅੱਖਰ c)
{
	ਜੇ (option_mask32 & FLAG_ਗਿਣਤੀ_ਬਾਈਟਾਂ)
		ਵਾਪਸ ++ਕਾਲਮ;

	ਜੇ (c == '\t')
		ਵਾਪਸ ਕਾਲਮ + 8 - ਕਾਲਮ % 8;

	ਜੇ (c == '\b') {
		ਜੇ ((ਪੂਰਨ)--ਕਾਲਮ < 0)
			ਕਾਲਮ = 0;
	}
	ਨਹੀਂਤਾਂ ਜੇ (c == '\r')
		ਕਾਲਮ = 0;
	ਨਹੀਂਤਾਂ { /* just a printable char */
		ਜੇ (unicode_ਸਥਿਤੀ != UNICODE_ON /* every byte is a new char */
		 || (c & 0xc0) != 0x80 /* it isn't a 2nd+ byte of a Unicode char */
		) {
			ਕਾਲਮ++;
		}
	}
	ਵਾਪਸ ਕਾਲਮ;
}

/* Note that this function can write NULs, unlike fputs etc. */
ਸਥਿਰ ਖਾਲੀ write2stdout(ਅਚਲ ਖਾਲੀ *buf, ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ)
{
	fwrite(buf, 1, ਮਾਪ, stdout);
}

ਪੂਰਨ fold_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ fold_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
	ਅੱਖਰ *ਸਤਰ_ਬਾਹਰਲਾ = ਨਿਰਾ;
	ਅਚਲ ਅੱਖਰ *w_opt = "80";
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਚੌੜਾਈ;
	exitcode_t exitcode = EXIT_SUCCESS;

	init_unicode();

	ਜੇ (ENABLE_INCLUDE_SUSv2) {
		/* Turn any numeric options into -w options.  */
		ਪੂਰਨ i;
		ਲਈ (i = 1; argv[i]; i++) {
			ਅਚਲ ਅੱਖਰ *a = argv[i];
			ਜੇ (*a == '-') {
				a++;
				ਜੇ (*a == '-' && !a[1]) /* "--" */
					ਰੋਕੋ;
				ਜੇ (ਅੰਕ_ਹੈ(*a))
					argv[i] = xasprintf("-w%s", a);
			}
		}
	}

	getopt32(argv, "bsw:", &w_opt);
	ਚੌੜਾਈ = xatou_range(w_opt, 1, 10000);

	argv += optind;
	ਜੇ (!*argv)
		*--argv = (ਅੱਖਰ*)"-";

	ਕਰੋ {
		FILE *istream = fopen_or_warn_stdin(*argv);
		ਪੂਰਨ c;
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਕਾਲਮ = 0;     /* Screen column where next char will go */
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਿੱਥ_ਬਾਹਰਲਾ = 0; /* Index in 'line_out' for next char */

		ਜੇ (istream == ਨਿਰਾ) {
			exitcode = EXIT_FAILURE;
			ਜਾਰੀ;
		}

		ਜਦੋਂ ((c = getc(istream)) != EOF) {
			/* We grow line_out in chunks of 0x1000 bytes */
			ਜੇ ((ਵਿੱਥ_ਬਾਹਰਲਾ & 0xfff) == 0) {
				ਸਤਰ_ਬਾਹਰਲਾ = xrealloc(ਸਤਰ_ਬਾਹਰਲਾ, ਵਿੱਥ_ਬਾਹਰਲਾ + 0x1000);
			}
 rescan:
			ਸਤਰ_ਬਾਹਰਲਾ[ਵਿੱਥ_ਬਾਹਰਲਾ] = c;
			ਜੇ (c == '\n') {
				write2stdout(ਸਤਰ_ਬਾਹਰਲਾ, ਵਿੱਥ_ਬਾਹਰਲਾ + 1);
				ਕਾਲਮ = ਵਿੱਥ_ਬਾਹਰਲਾ = 0;
				ਜਾਰੀ;
			}
			ਕਾਲਮ = adjust_ਕਾਲਮ(ਕਾਲਮ, c);
			ਜੇ (ਕਾਲਮ <= ਚੌੜਾਈ || ਵਿੱਥ_ਬਾਹਰਲਾ == 0) {
				/* offset_out == 0 case happens
				 * with small width (say, 1) and tabs.
				 * The very first tab already goes to column 8,
				 * but we must not wrap it */
				ਵਿੱਥ_ਬਾਹਰਲਾ++;
				ਜਾਰੀ;
			}

			/* This character would make the line too long.
			 * Print the line plus a newline, and make this character
			 * start the next line */
			ਜੇ (option_mask32 & FLAG_BREAK_SPACES) {
				ਬਿਨਾਨਿਸ਼ਾਨੀ i;
				ਬਿਨਾਨਿਸ਼ਾਨੀ logical_ਸਿਰਾ;

				/* Look for the last blank. */
				ਲਈ (logical_ਸਿਰਾ = ਵਿੱਥ_ਬਾਹਰਲਾ - 1; (ਪੂਰਨ)logical_ਸਿਰਾ >= 0; logical_ਸਿਰਾ--) {
					ਜੇ (!isblank(ਸਤਰ_ਬਾਹਰਲਾ[logical_ਸਿਰਾ]))
						ਜਾਰੀ;

					/* Found a space or tab.
					 * Output up to and including it, and start a new line */
					logical_ਸਿਰਾ++;
					/*line_out[logical_end] = '\n'; - NO! this nukes one buffered character */
					write2stdout(ਸਤਰ_ਬਾਹਰਲਾ, logical_ਸਿਰਾ);
					ਅੱਖਰ_ਲਿਖੋ('\n');
					/* Move the remainder to the beginning of the next line.
					 * The areas being copied here might overlap. */
					memmove(ਸਤਰ_ਬਾਹਰਲਾ, ਸਤਰ_ਬਾਹਰਲਾ + logical_ਸਿਰਾ, ਵਿੱਥ_ਬਾਹਰਲਾ - logical_ਸਿਰਾ);
					ਵਿੱਥ_ਬਾਹਰਲਾ -= logical_ਸਿਰਾ;
					ਲਈ (ਕਾਲਮ = i = 0; i < ਵਿੱਥ_ਬਾਹਰਲਾ; i++) {
						ਕਾਲਮ = adjust_ਕਾਲਮ(ਕਾਲਮ, ਸਤਰ_ਬਾਹਰਲਾ[i]);
					}
					ਜਾਓ rescan;
				}
				/* No blank found, wrap will split the overlong word */
			}
			/* Output what we accumulated up to now, and start a new line */
			ਸਤਰ_ਬਾਹਰਲਾ[ਵਿੱਥ_ਬਾਹਰਲਾ] = '\n';
			write2stdout(ਸਤਰ_ਬਾਹਰਲਾ, ਵਿੱਥ_ਬਾਹਰਲਾ + 1);
			ਕਾਲਮ = ਵਿੱਥ_ਬਾਹਰਲਾ = 0;
			ਜਾਓ rescan;
		} /* while (not EOF) */

		ਜੇ (ਵਿੱਥ_ਬਾਹਰਲਾ) {
			write2stdout(ਸਤਰ_ਬਾਹਰਲਾ, ਵਿੱਥ_ਬਾਹਰਲਾ);
		}

		ਜੇ (fclose_if_not_stdin(istream)) {
			bb_simple_perror_ਸੁਨੇਹਾ(*argv);
			exitcode = EXIT_FAILURE;
		}
	} ਜਦੋਂ (*++argv);

	fflush_stdout_and_exit(exitcode);
}
