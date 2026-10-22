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

#समावेश "libbb.h"
#समावेश "unicode.h"

/* This is a NOEXEC applet. Be very careful! */

/* Must match getopt32 call */
#घोषणा FLAG_गिनती_BYTES        1
#घोषणा FLAG_BREAK_SPACES       2
#घोषणा FLAG_WIDTH              4

/* Assuming the current column is COLUMN, return the column that
   printing C will move the cursor to.
   The first column is 0. */
अटल पूर्णांक adjust_column(अचिन्हित column, अक्षर c)
{
	अगर (option_mask32 & FLAG_गिनती_BYTES)
		वापस ++column;

	अगर (c == '\t')
		वापस column + 8 - column % 8;

	अगर (c == '\b') {
		अगर ((पूर्णांक)--column < 0)
			column = 0;
	}
	अन्यथा अगर (c == '\r')
		column = 0;
	अन्यथा { /* just a printable char */
		अगर (unicode_status != UNICODE_ON /* every byte is a new char */
		 || (c & 0xc0) != 0x80 /* it isn't a 2nd+ byte of a Unicode char */
		) {
			column++;
		}
	}
	वापस column;
}

/* Note that this function can write NULs, unlike fputs etc. */
अटल व्योम write2stdout(स्थिर व्योम *buf, अचिन्हित आकार)
{
	ख_डालो(buf, 1, आकार, मानक_निकास);
}

पूर्णांक fold_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक fold_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	अक्षर *पंक्ति_out = शून्य;
	स्थिर अक्षर *w_opt = "80";
	अचिन्हित width;
	exitcode_t exitcode = निकास_सफल;

	init_unicode();

	अगर (ENABLE_INCLUDE_SUSv2) {
		/* Turn any numeric options into -w options.  */
		पूर्णांक i;
		क्रम (i = 1; argv[i]; i++) {
			स्थिर अक्षर *a = argv[i];
			अगर (*a == '-') {
				a++;
				अगर (*a == '-' && !a[1]) /* "--" */
					अवरोध;
				अगर (है_अंक(*a))
					argv[i] = xasprintf("-w%s", a);
			}
		}
	}

	getopt32(argv, "bsw:", &w_opt);
	width = xatou_range(w_opt, 1, 10000);

	argv += optind;
	अगर (!*argv)
		*--argv = (अक्षर*)"-";

	करो {
		खाता *istream = fopen_or_warn_stdin(*argv);
		पूर्णांक c;
		अचिन्हित column = 0;     /* Screen column where next char will go */
		अचिन्हित offset_out = 0; /* Index in 'line_out' for next char */

		अगर (istream == शून्य) {
			exitcode = निकास_त्रुटि;
			जारी;
		}

		जबतक ((c = अ_लो(istream)) != खातापूर्ण) {
			/* We grow line_out in chunks of 0x1000 bytes */
			अगर ((offset_out & 0xfff) == 0) {
				पंक्ति_out = xrealloc(पंक्ति_out, offset_out + 0x1000);
			}
 rescan:
			पंक्ति_out[offset_out] = c;
			अगर (c == '\n') {
				write2stdout(पंक्ति_out, offset_out + 1);
				column = offset_out = 0;
				जारी;
			}
			column = adjust_column(column, c);
			अगर (column <= width || offset_out == 0) {
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
			अगर (option_mask32 & FLAG_BREAK_SPACES) {
				अचिन्हित i;
				अचिन्हित logical_end;

				/* Look for the last blank. */
				क्रम (logical_end = offset_out - 1; (पूर्णांक)logical_end >= 0; logical_end--) {
					अगर (!isblank(पंक्ति_out[logical_end]))
						जारी;

					/* Found a space or tab.
					 * Output up to and including it, and start a new line */
					logical_end++;
					/*line_out[logical_end] = '\n'; - NO! this nukes one buffered character */
					write2stdout(पंक्ति_out, logical_end);
					अक्षर_दो('\n');
					/* Move the remainder to the beginning of the next line.
					 * The areas being copied here might overlap. */
					स_हटाओ(पंक्ति_out, पंक्ति_out + logical_end, offset_out - logical_end);
					offset_out -= logical_end;
					क्रम (column = i = 0; i < offset_out; i++) {
						column = adjust_column(column, पंक्ति_out[i]);
					}
					जाओ rescan;
				}
				/* No blank found, wrap will split the overlong word */
			}
			/* Output what we accumulated up to now, and start a new line */
			पंक्ति_out[offset_out] = '\n';
			write2stdout(पंक्ति_out, offset_out + 1);
			column = offset_out = 0;
			जाओ rescan;
		} /* while (not EOF) */

		अगर (offset_out) {
			write2stdout(पंक्ति_out, offset_out);
		}

		अगर (fclose_if_not_stdin(istream)) {
			bb_simple_perror_संदेश(*argv);
			exitcode = निकास_त्रुटि;
		}
	} जबतक (*++argv);

	fflush_stdout_and_exit(exitcode);
}
