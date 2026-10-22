/* vi: set sw=4 ts=4: */
/*
 * wc implementation for busybox
 *
 * Copyright (C) 2003  Manuel Novoa III  <mjn3@codepoet.org>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
/* Mar 16, 2003      Manuel Novoa III   (mjn3@codepoet.org)
 *
 * Rewritten to fix a number of problems and do some size optimizations.
 * Problems in the previous busybox implementation (besides bloat) included:
 *  1) broken 'wc -c' optimization (read note below)
 *  2) broken handling of '-' args
 *  3) no checking of ferror on EOF returns
 *  4) isprint() wasn't considered when word counting.
 *
 * NOTES:
 *
 * The previous busybox wc attempted an optimization using stat for the
 * case of counting chars only.  I omitted that because it was broken.
 * It didn't take into account the possibility of input coming from a
 * pipe, or input from a file with file pointer not at the beginning.
 *
 * To implement such a speed optimization correctly, not only do you
 * need the size, but also the file position.  Note also that the
 * file position may be past the end of file.  Consider the example
 * (adapted from example in gnu wc.c)
 *
 *      echo hello > /tmp/testfile &&
 *      (dd ibs=1k skip=1 count=0 &> /dev/null; wc -c) < /tmp/testfile
 *
 * for which 'wc -c' should output '0'.
 */
//config:config WC
//config:	bool "wc (4.7 kb)"
//config:	default y
//config:	help
//config:	wc is used to print the number of bytes, words, and lines,
//config:	in specified files.
//config:
//config:config FEATURE_WC_LARGE
//config:	bool "Support very large counts"
//config:	default y
//config:	depends on WC
//config:	help
//config:	Use "unsigned long long" for counter variables.

//applet:IF_WC(APPLET(wc, BB_DIR_USR_BIN, BB_SUID_DROP))

//kbuild:lib-$(CONFIG_WC) += wc.o

/* BB_AUDIT SUSv3 compliant. */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/wc.html */

#समावेश "libbb.h"
#समावेश "unicode.h"

#अगर !ENABLE_LOCALE_SUPPORT
# undef है_छाप
# undef है_खाली
# define है_छाप(c) ((अचिन्हित)((c) - 0x20) <= (0x7e - 0x20))
# define है_खाली(c) ((c) == ' ')
#पूर्ण_अगर

#अगर ENABLE_FEATURE_WC_LARGE
# define गिनती_T अचिन्हित दीर्घ दीर्घ
# define गिनती_FMT "llu"
#अन्यथा
# define गिनती_T अचिन्हित
# define गिनती_FMT "u"
#पूर्ण_अगर

/* We support -m even when UNICODE_SUPPORT is off,
 * we just don't advertise it in help text,
 * since it is the same as -c in this case.
 */

//usage:#define wc_trivial_usage
//usage:       "[-c"IF_UNICODE_SUPPORT("m")"lwL] [FILE]..."
//usage:
//usage:#define wc_full_usage "\n\n"
//usage:       "Count lines, words, and bytes for FILEs (or stdin)\n"
//usage:     "\n	-c	Count bytes"
//usage:	IF_UNICODE_SUPPORT(
//usage:     "\n	-m	Count characters"
//usage:	)
//usage:     "\n	-l	Count newlines"
//usage:     "\n	-w	Count words"
//usage:     "\n	-L	Print longest line length"
//usage:
//usage:#define wc_example_usage
//usage:       "$ wc /etc/passwd\n"
//usage:       "     31      46    1365 /etc/passwd\n"

/* Order is important if we want to be compatible with
 * column order in "wc -cmlwL" output:
 */
क्रमागत {
	WC_पंक्तियाँ    = 0, /* -l */
	WC_शब्दगण    = 1, /* -w */
	WC_UNICHARS = 2, /* -m */
	WC_BYTES    = 3, /* -c */
	WC_लंबाई   = 4, /* -L */
	अंक_WCS     = 5,
};

पूर्णांक wc_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक wc_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	स्थिर अक्षर *arg;
	स्थिर अक्षर *start_fmt = " %9"गिनती_FMT + 1;
	स्थिर अक्षर *fname_fmt = " %s\n";
	गिनती_T *pcounts;
	गिनती_T counts[अंक_WCS];
	गिनती_T totals[अंक_WCS];
	पूर्णांक अंक_files;
	smallint status = निकास_सफल;
	अचिन्हित print_type;

	init_unicode();

	print_type = getopt32(argv, "lwmcL");

	अगर (print_type == 0) {
		print_type = (1 << WC_पंक्तियाँ) | (1 << WC_शब्दगण) | (1 << WC_BYTES);
	}

	argv += optind;
	अगर (!argv[0]) {
		*--argv = (अक्षर *) bb_संदेश_standard_निवेश;
		fname_fmt = "\n";
	}
	अगर (!argv[1]) { /* zero or one filename? */
		अगर (!((print_type-1) & print_type)) /* exactly one option? */
			start_fmt = "%"गिनती_FMT;
	}

	स_रखो(totals, 0, माप(totals));

	pcounts = counts;

	अंक_files = 0;
	जबतक ((arg = *argv++) != शून्य) {
		खाता *fp;
		स्थिर अक्षर *s;
		अचिन्हित u;
		अचिन्हित linepos;
		smallint in_word;

		++अंक_files;
		fp = fopen_or_warn_stdin(arg);
		अगर (!fp) {
			status = निकास_त्रुटि;
			जारी;
		}

		स_रखो(counts, 0, माप(counts));
		linepos = 0;
		in_word = 0;

		जबतक (1) {
			पूर्णांक c;
			/* Our -w doesn't match GNU wc exactly... oh well */

			c = अ_लो(fp);
			अगर (c == खातापूर्ण) {
				अगर (ख_त्रुटि(fp)) {
					bb_simple_perror_संदेश(arg);
					status = निकास_त्रुटि;
				}
				जाओ DO_EOF;  /* Treat an EOF as '\r'. */
			}

			/* Cater for -c and -m */
			++counts[WC_BYTES];
			अगर (unicode_status != UNICODE_ON /* every byte is a new char */
			 || (c & 0xc0) != 0x80 /* it isn't a 2nd+ byte of a Unicode char */
			) {
				++counts[WC_UNICHARS];
			}

			अगर (isprint_asciionly(c)) { /* FIXME: not unicode-aware */
				++linepos;
				अगर (!है_खाली(c)) {
					in_word = 1;
					जारी;
				}
			} अन्यथा अगर ((अचिन्हित)(c - 9) <= 4) {
				/* \t  9
				 * \n 10
				 * \v 11
				 * \f 12
				 * \r 13
				 */
				अगर (c == '\t') {
					linepos = (linepos | 7) + 1;
				} अन्यथा {  /* '\n', '\r', '\f', or '\v' */
 DO_EOF:
					अगर (linepos > counts[WC_लंबाई]) {
						counts[WC_लंबाई] = linepos;
					}
					अगर (c == '\n') {
						++counts[WC_पंक्तियाँ];
					}
					अगर (c != '\v') {
						linepos = 0;
					}
				}
			} अन्यथा {
				जारी;
			}

			counts[WC_शब्दगण] += in_word;
			in_word = 0;
			अगर (c == खातापूर्ण) {
				अवरोध;
			}
		}

		fclose_if_not_stdin(fp);

		अगर (totals[WC_लंबाई] < counts[WC_लंबाई]) {
			totals[WC_लंबाई] = counts[WC_लंबाई];
		}
		totals[WC_लंबाई] -= counts[WC_लंबाई];

 OUTPUT:
		/* coreutils wc tries hard to print pretty columns
		 * (saves results for all files, finds max col len etc...)
		 * we won't try that hard, it will bloat us too much */
		s = start_fmt;
		u = 0;
		करो {
			अगर (print_type & (1 << u)) {
				म_लिखो(s, pcounts[u]);
				s = " %9"गिनती_FMT; /* Ok... restore the leading space. */
			}
			totals[u] += pcounts[u];
		} जबतक (++u < अंक_WCS);
		म_लिखो(fname_fmt, arg);
	}

	/* If more than one file was processed, we want the totals.  To save some
	 * space, we set the pcounts ptr to the totals array.  This has the side
	 * effect of trashing the totals array after outputting it, but that's
	 * irrelavent since we no longer need it. */
	अगर (अंक_files > 1) {
		अंक_files = 0;  /* Make sure we don't get here again. */
		arg = "total";
		pcounts = totals;
		--argv;
		जाओ OUTPUT;
	}

	fflush_stdout_and_exit(status);
}
