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
#परिभाषय FLAG_गणनम्_अष्टकानि        1
#परिभाषय FLAG_BREAK_SPACES       2
#परिभाषय FLAG_विस्तारः              4

/* Assuming the current column is COLUMN, return the column that
   printing C will move the cursor to.
   The first column is 0. */
स्थिर पूर्णाङ्क adjust_column(अचिह्नित column, वर्ण c)
{
	यदि (option_mask32 & FLAG_गणनम्_अष्टकानि)
		प्रत्यागम ++column;

	यदि (c == '\t')
		प्रत्यागम column + 8 - column % 8;

	यदि (c == '\b') {
		यदि ((पूर्णाङ्क)--column < 0)
			column = 0;
	}
	अन्यथा यदि (c == '\r')
		column = 0;
	अन्यथा { /* just a printable char */
		यदि (unicode_स्थितिः != UNICODE_ON /* every byte is a new char */
		 || (c & 0xc0) != 0x80 /* it isn't a 2nd+ byte of a Unicode char */
		) {
			column++;
		}
	}
	प्रत्यागम column;
}

/* Note that this function can write NULs, unlike fputs etc. */
स्थिर शून्य write2stdout(ध्रुव शून्य *buf, अचिह्नित आकार)
{
	fwrite(buf, 1, आकार, stdout);
}

पूर्णाङ्क fold_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क fold_main(पूर्णाङ्क argc अप्रयुक्तम्_PARAM, वर्ण **argv)
{
	वर्ण *पङ्क्तिः_बहिर्गमनम् = रिक्त;
	ध्रुव वर्ण *w_opt = "80";
	अचिह्नित विस्तारः;
	exitcode_t exitcode = EXIT_SUCCESS;

	init_unicode();

	यदि (ENABLE_INCLUDE_SUSv2) {
		/* Turn any numeric options into -w options.  */
		पूर्णाङ्क i;
		प्रति (i = 1; argv[i]; i++) {
			ध्रुव वर्ण *a = argv[i];
			यदि (*a == '-') {
				a++;
				यदि (*a == '-' && !a[1]) /* "--" */
					भङ्ग;
				यदि (अङ्कः_वा(*a))
					argv[i] = xasprintf("-w%s", a);
			}
		}
	}

	getopt32(argv, "bsw:", &w_opt);
	विस्तारः = xatou_range(w_opt, 1, 10000);

	argv += optind;
	यदि (!*argv)
		*--argv = (वर्ण*)"-";

	कुरु {
		FILE *istream = fopen_or_warn_stdin(*argv);
		पूर्णाङ्क c;
		अचिह्नित column = 0;     /* Screen column where next char will go */
		अचिह्नित विस्थापनम्_बहिर्गमनम् = 0; /* Index in 'line_out' for next char */

		यदि (istream == रिक्त) {
			exitcode = EXIT_FAILURE;
			अनुवर्त;
		}

		यावत् ((c = getc(istream)) != EOF) {
			/* We grow line_out in chunks of 0x1000 bytes */
			यदि ((विस्थापनम्_बहिर्गमनम् & 0xfff) == 0) {
				पङ्क्तिः_बहिर्गमनम् = xrealloc(पङ्क्तिः_बहिर्गमनम्, विस्थापनम्_बहिर्गमनम् + 0x1000);
			}
 rescan:
			पङ्क्तिः_बहिर्गमनम्[विस्थापनम्_बहिर्गमनम्] = c;
			यदि (c == '\n') {
				write2stdout(पङ्क्तिः_बहिर्गमनम्, विस्थापनम्_बहिर्गमनम् + 1);
				column = विस्थापनम्_बहिर्गमनम् = 0;
				अनुवर्त;
			}
			column = adjust_column(column, c);
			यदि (column <= विस्तारः || विस्थापनम्_बहिर्गमनम् == 0) {
				/* offset_out == 0 case happens
				 * with small width (say, 1) and tabs.
				 * The very first tab already goes to column 8,
				 * but we must not wrap it */
				विस्थापनम्_बहिर्गमनम्++;
				अनुवर्त;
			}

			/* This character would make the line too long.
			 * Print the line plus a newline, and make this character
			 * start the next line */
			यदि (option_mask32 & FLAG_BREAK_SPACES) {
				अचिह्नित i;
				अचिह्नित logical_अन्तः;

				/* Look for the last blank. */
				प्रति (logical_अन्तः = विस्थापनम्_बहिर्गमनम् - 1; (पूर्णाङ्क)logical_अन्तः >= 0; logical_अन्तः--) {
					यदि (!isblank(पङ्क्तिः_बहिर्गमनम्[logical_अन्तः]))
						अनुवर्त;

					/* Found a space or tab.
					 * Output up to and including it, and start a new line */
					logical_अन्तः++;
					/*line_out[logical_end] = '\n'; - NO! this nukes one buffered character */
					write2stdout(पङ्क्तिः_बहिर्गमनम्, logical_अन्तः);
					अक्षरं_लिख('\n');
					/* Move the remainder to the beginning of the next line.
					 * The areas being copied here might overlap. */
					memmove(पङ्क्तिः_बहिर्गमनम्, पङ्क्तिः_बहिर्गमनम् + logical_अन्तः, विस्थापनम्_बहिर्गमनम् - logical_अन्तः);
					विस्थापनम्_बहिर्गमनम् -= logical_अन्तः;
					प्रति (column = i = 0; i < विस्थापनम्_बहिर्गमनम्; i++) {
						column = adjust_column(column, पङ्क्तिः_बहिर्गमनम्[i]);
					}
					गच्छ rescan;
				}
				/* No blank found, wrap will split the overlong word */
			}
			/* Output what we accumulated up to now, and start a new line */
			पङ्क्तिः_बहिर्गमनम्[विस्थापनम्_बहिर्गमनम्] = '\n';
			write2stdout(पङ्क्तिः_बहिर्गमनम्, विस्थापनम्_बहिर्गमनम् + 1);
			column = विस्थापनम्_बहिर्गमनम् = 0;
			गच्छ rescan;
		} /* while (not EOF) */

		यदि (विस्थापनम्_बहिर्गमनम्) {
			write2stdout(पङ्क्तिः_बहिर्गमनम्, विस्थापनम्_बहिर्गमनम्);
		}

		यदि (fclose_if_not_stdin(istream)) {
			bb_simple_perror_सन्देशः(*argv);
			exitcode = EXIT_FAILURE;
		}
	} यावत् (*++argv);

	fflush_stdout_and_exit(exitcode);
}
