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
#define FLAG_मोजणी_BYTES        1
#define FLAG_BREAK_SPACES       2
#define FLAG_WIDTH              4

/* Assuming the current column is COLUMN, return the column that
   printing C will move the cursor to.
   The first column is 0. */
स्थिर पूर्णांक adjust_column(अचिन्हित column, अक्षर c)
{
	जर (option_mask32 & FLAG_मोजणी_BYTES)
		परत ++column;

	जर (c == '\t')
		परत column + 8 - column % 8;

	जर (c == '\b') {
		जर ((पूर्णांक)--column < 0)
			column = 0;
	}
	अन्यथा जर (c == '\r')
		column = 0;
	अन्यथा { /* just a printable char */
		जर (unicode_status != UNICODE_ON /* every byte is a new char */
		 || (c & 0xc0) != 0x80 /* it isn't a 2nd+ byte of a Unicode char */
		) {
			column++;
		}
	}
	परत column;
}

/* Note that this function can write NULs, unlike fputs etc. */
स्थिर शून्य write2stdout(स्थिरांक शून्य *buf, अचिन्हित size)
{
	ख_डालो(buf, 1, size, मानक_निकास);
}

पूर्णांक fold_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक fold_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	अक्षर *ओळ_out = रिक्त;
	स्थिरांक अक्षर *w_opt = "80";
	अचिन्हित width;
	exitcode_t exitcode = निकास_सफल;

	init_unicode();

	जर (ENABLE_INCLUDE_SUSv2) {
		/* Turn any numeric options into -w options.  */
		पूर्णांक i;
		साठी (i = 1; argv[i]; i++) {
			स्थिरांक अक्षर *a = argv[i];
			जर (*a == '-') {
				a++;
				जर (*a == '-' && !a[1]) /* "--" */
					थांबा;
				जर (है_अंक(*a))
					argv[i] = xasprintf("-w%s", a);
			}
		}
	}

	getopt32(argv, "bsw:", &w_opt);
	width = xatou_range(w_opt, 1, 10000);

	argv += optind;
	जर (!*argv)
		*--argv = (अक्षर*)"-";

	करा {
		खाता *istream = fopen_or_warn_stdin(*argv);
		पूर्णांक c;
		अचिन्हित column = 0;     /* Screen column where next char will go */
		अचिन्हित offset_out = 0; /* Index in 'line_out' for next char */

		जर (istream == रिक्त) {
			exitcode = निकास_त्रुटि;
			सुरूठेवा;
		}

		पर्यंत ((c = अ_लो(istream)) != खातापूर्ण) {
			/* We grow line_out in chunks of 0x1000 bytes */
			जर ((offset_out & 0xfff) == 0) {
				ओळ_out = xrealloc(ओळ_out, offset_out + 0x1000);
			}
 rescan:
			ओळ_out[offset_out] = c;
			जर (c == '\n') {
				write2stdout(ओळ_out, offset_out + 1);
				column = offset_out = 0;
				सुरूठेवा;
			}
			column = adjust_column(column, c);
			जर (column <= width || offset_out == 0) {
				/* offset_out == 0 case happens
				 * with small width (say, 1) and tabs.
				 * The very first tab already goes to column 8,
				 * but we must not wrap it */
				offset_out++;
				सुरूठेवा;
			}

			/* This character would make the line too long.
			 * Print the line plus a newline, and make this character
			 * start the next line */
			जर (option_mask32 & FLAG_BREAK_SPACES) {
				अचिन्हित i;
				अचिन्हित logical_end;

				/* Look for the last blank. */
				साठी (logical_end = offset_out - 1; (पूर्णांक)logical_end >= 0; logical_end--) {
					जर (!isblank(ओळ_out[logical_end]))
						सुरूठेवा;

					/* Found a space or tab.
					 * Output up to and including it, and start a new line */
					logical_end++;
					/*line_out[logical_end] = '\n'; - NO! this nukes one buffered character */
					write2stdout(ओळ_out, logical_end);
					अक्षर_दो('\n');
					/* Move the remainder to the beginning of the next line.
					 * The areas being copied here might overlap. */
					स_हटाओ(ओळ_out, ओळ_out + logical_end, offset_out - logical_end);
					offset_out -= logical_end;
					साठी (column = i = 0; i < offset_out; i++) {
						column = adjust_column(column, ओळ_out[i]);
					}
					जा rescan;
				}
				/* No blank found, wrap will split the overlong word */
			}
			/* Output what we accumulated up to now, and start a new line */
			ओळ_out[offset_out] = '\n';
			write2stdout(ओळ_out, offset_out + 1);
			column = offset_out = 0;
			जा rescan;
		} /* while (not EOF) */

		जर (offset_out) {
			write2stdout(ओळ_out, offset_out);
		}

		जर (fclose_if_not_stdin(istream)) {
			bb_simple_perror_संदेश(*argv);
			exitcode = निकास_त्रुटि;
		}
	} पर्यंत (*++argv);

	fflush_stdout_and_exit(exitcode);
}
