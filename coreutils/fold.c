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
#define FLAG_गन्ती_BYTES        1
#define FLAG_BREAK_SPACES       2
#define FLAG_WIDTH              4

/* Assuming the current column is COLUMN, return the column that
   printing C will move the cursor to.
   The first column is 0. */
स्थिर पूर्णाङ्क adjust_column(अचिन्हित column, वर्ण c)
{
	यदि (option_mask32 & FLAG_गन्ती_BYTES)
		फर्काऊ ++column;

	यदि (c == '\t')
		फर्काऊ column + 8 - column % 8;

	यदि (c == '\b') {
		यदि ((पूर्णाङ्क)--column < 0)
			column = 0;
	}
	अन्यथा यदि (c == '\r')
		column = 0;
	अन्यथा { /* just a printable char */
		यदि (unicode_status != UNICODE_ON /* every byte is a new char */
		 || (c & 0xc0) != 0x80 /* it isn't a 2nd+ byte of a Unicode char */
		) {
			column++;
		}
	}
	फर्काऊ column;
}

/* Note that this function can write NULs, unlike fputs etc. */
स्थिर शून्य write2stdout(अचर शून्य *buf, अचिन्हित size)
{
	ख_डालो(buf, 1, size, मानक_निकास);
}

पूर्णाङ्क fold_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क fold_main(पूर्णाङ्क argc UNUSED_PARAM, वर्ण **argv)
{
	वर्ण *पङ्क्ति_out = रिक्त;
	अचर वर्ण *w_opt = "80";
	अचिन्हित width;
	exitcode_t exitcode = निकास_सफल;

	init_unicode();

	यदि (ENABLE_INCLUDE_SUSv2) {
		/* Turn any numeric options into -w options.  */
		पूर्णाङ्क i;
		लागि (i = 1; argv[i]; i++) {
			अचर वर्ण *a = argv[i];
			यदि (*a == '-') {
				a++;
				यदि (*a == '-' && !a[1]) /* "--" */
					रोक;
				यदि (है_अंक(*a))
					argv[i] = xasprintf("-w%s", a);
			}
		}
	}

	getopt32(argv, "bsw:", &w_opt);
	width = xatou_range(w_opt, 1, 10000);

	argv += optind;
	यदि (!*argv)
		*--argv = (वर्ण*)"-";

	गर {
		खाता *istream = fopen_or_warn_stdin(*argv);
		पूर्णाङ्क c;
		अचिन्हित column = 0;     /* Screen column where next char will go */
		अचिन्हित offset_out = 0; /* Index in 'line_out' for next char */

		यदि (istream == रिक्त) {
			exitcode = निकास_त्रुटि;
			जारी;
		}

		जबसम्म ((c = अ_लो(istream)) != खातापूर्ण) {
			/* We grow line_out in chunks of 0x1000 bytes */
			यदि ((offset_out & 0xfff) == 0) {
				पङ्क्ति_out = xrealloc(पङ्क्ति_out, offset_out + 0x1000);
			}
 rescan:
			पङ्क्ति_out[offset_out] = c;
			यदि (c == '\n') {
				write2stdout(पङ्क्ति_out, offset_out + 1);
				column = offset_out = 0;
				जारी;
			}
			column = adjust_column(column, c);
			यदि (column <= width || offset_out == 0) {
				/* offset_out == 0 case happens
				 * with small width (say, 1) and tabs.
				 * The very first tab already goes to column 8,
				 * but we must not wrap it */
				offset_out++;
				जारी;
			}

			/* This character would make the line too long.
			 * Print the line plus a newline, and make this character
			 * start the next line */
			यदि (option_mask32 & FLAG_BREAK_SPACES) {
				अचिन्हित i;
				अचिन्हित logical_end;

				/* Look for the last blank. */
				लागि (logical_end = offset_out - 1; (पूर्णाङ्क)logical_end >= 0; logical_end--) {
					यदि (!isblank(पङ्क्ति_out[logical_end]))
						जारी;

					/* Found a space or tab.
					 * Output up to and including it, and start a new line */
					logical_end++;
					/*line_out[logical_end] = '\n'; - NO! this nukes one buffered character */
					write2stdout(पङ्क्ति_out, logical_end);
					अक्षर_दो('\n');
					/* Move the remainder to the beginning of the next line.
					 * The areas being copied here might overlap. */
					स_हटाओ(पङ्क्ति_out, पङ्क्ति_out + logical_end, offset_out - logical_end);
					offset_out -= logical_end;
					लागि (column = i = 0; i < offset_out; i++) {
						column = adjust_column(column, पङ्क्ति_out[i]);
					}
					गच्छ rescan;
				}
				/* No blank found, wrap will split the overlong word */
			}
			/* Output what we accumulated up to now, and start a new line */
			पङ्क्ति_out[offset_out] = '\n';
			write2stdout(पङ्क्ति_out, offset_out + 1);
			column = offset_out = 0;
			गच्छ rescan;
		} /* while (not EOF) */

		यदि (offset_out) {
			write2stdout(पङ्क्ति_out, offset_out);
		}

		यदि (fclose_if_not_stdin(istream)) {
			bb_simple_perror_सन्देश(*argv);
			exitcode = निकास_त्रुटि;
		}
	} जबसम्म (*++argv);

	fflush_stdout_and_exit(exitcode);
}
