/* vi: set sw=4 ts=4: */
/*
 * cut.c - minimalist version of cut
 *
 * Copyright (C) 1999,2000,2001 by Lineo, inc.
 * Written by Mark Whitley <markw@codepoet.org>
 * debloated by Bernhard Reutner-Fischer
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config CUT
//config:	bool "cut (6.7 kb)"
//config:	default y
//config:	help
//config:	cut is used to print selected parts of lines from
//config:	each file to stdout.
//config:
//config:config FEATURE_CUT_REGEX
//config:	bool "cut -F"
//config:	default y
//config:	depends on CUT
//config:	help
//config:	Allow regex based delimiters.

//applet:IF_CUT(APPLET_NOEXEC(cut, cut, BB_DIR_USR_BIN, BB_SUID_DROP, cut))

//kbuild:lib-$(CONFIG_CUT) += cut.o

//usage:#define cut_trivial_usage
//usage:       "[OPTIONS] [FILE]..."
//usage:#define cut_full_usage "\n\n"
//usage:       "Print selected fields from FILEs to stdout\n"
//usage:     "\n	-b LIST	Output only bytes from LIST"
//usage:     "\n	-c LIST	Output only characters from LIST"
//usage:     "\n	-d SEP	Field delimiter for input (default -f TAB, -F run of whitespace)"
//usage:     "\n	-O SEP	Field delimeter for output (default = -d for -f, one space for -F)"
//usage:     "\n	-D	Don't sort/collate sections or match -fF lines without delimeter"
//usage:     "\n	-f LIST	Print only these fields (-d is single char)"
//usage:     IF_FEATURE_CUT_REGEX(
//usage:     "\n	-F LIST	Print only these fields (-d is regex)"
//usage:     )
//usage:     "\n	-s	Output only lines containing delimiter"
//usage:     "\n	-n	Ignored"
//(manpage:-n	with -b: don't split multibyte characters)
//usage:
//usage:#define cut_example_usage
//usage:       "$ echo \"Hello world\" | cut -f 1 -d ' '\n"
//usage:       "Hello\n"
//usage:       "$ echo \"Hello world\" | cut -f 2 -d ' '\n"
//usage:       "world\n"

#அந்தர்பாவ "libbb.h"

#எனில் ENABLE_FEATURE_CUT_REGEX
#அந்தர்பாவ "xregex.h"
#இல்லையெனில்
#define regex_t முழு
வகைவரை கட்டமைப்பு { முழு rm_eo, rm_so; } regmatch_t;
#define xregcomp(x, ...) *(x) = 0
#define regexec(...)     0
#endif

/* This is a NOEXEC applet. Be very careful! */


/* option vars */
#define OPT_STR "b:c:f:d:O:sD"IF_FEATURE_CUT_REGEX("F:")"n"
#define CUT_OPT_BYTE_FLGS     (1 << 0)
#define CUT_OPT_CHAR_FLGS     (1 << 1)
#define CUT_OPT_FIELDS_FLGS   (1 << 2)
#define CUT_OPT_DELIM_FLGS    (1 << 3)
#define CUT_OPT_ODELIM_FLGS   (1 << 4)
#define CUT_OPT_SUPPRESS_FLGS (1 << 5)
#define CUT_OPT_NOSORT_FLGS   (1 << 6)
#define CUT_OPT_REGEX_FLGS    ((1 << 7) * ENABLE_FEATURE_CUT_REGEX)

கட்டமைப்பு cut_list {
	முழு startpos;
	முழு endpos;
};

நிலையான முழு cmpfunc(மாறிலி வெறுமை *a, மாறிலி வெறுமை *b)
{
	திரும்பு (((கட்டமைப்பு cut_list *) a)->startpos -
			((கட்டமைப்பு cut_list *) b)->startpos);
}

நிலையான வெறுமை cut_file(FILE *file, மாறிலி எழுத்து *delim, மாறிலி எழுத்து *odelim,
		மாறிலி கட்டமைப்பு cut_list *cut_lists, unsigned nlists)
{
	எழுத்து *வரி;
	unsigned linenum = 0;	/* keep these zero-based to be consistent */
	regex_t reg;
	முழு spos, shoe = option_mask32 & CUT_OPT_REGEX_FLGS;

	எனில் (shoe) xregcomp(&reg, delim, REG_EXTENDED);

	/* go through every line in the file */
	போது ((வரி = xmalloc_fgetline(file)) != வெற்று) {

		/* set up a list so we can keep track of what's been printed */
		முழு linelen = strlen(வரி);
		எழுத்து *printed = xzalloc(linelen + 1);
		எழுத்து *orig_வரி = வரி;
		unsigned cl_pos = 0;

		/* cut based on chars/bytes XXX: only works when sizeof(char) == byte */
		எனில் (option_mask32 & (CUT_OPT_CHAR_FLGS | CUT_OPT_BYTE_FLGS)) {
			/* print the chars specified in each cut list */
			க்கு (; cl_pos < nlists; cl_pos++) {
				க்கு (spos = cut_lists[cl_pos].startpos; spos < linelen;) {
					எனில் (!printed[spos]) {
						printed[spos] = 'X';
						putchar(வரி[spos]);
					}
					எனில் (++spos > cut_lists[cl_pos].endpos) {
						நிறுத்து;
					}
				}
			}
		} இல்லையெனில் எனில் (*delim == '\n') {	/* cut by lines */
			spos = cut_lists[cl_pos].startpos;

			/* get out if we have no more lists to process or if the lines
			 * are lower than what we're interested in */
			எனில் (((முழு)linenum < spos) || (cl_pos >= nlists))
				செல் next_வரி;

			/* if the line we're looking for is lower than the one we were
			 * passed, it means we displayed it already, so move on */
			போது (spos < (முழு)linenum) {
				spos++;
				/* go to the next list if we're at the end of this one */
				எனில் (spos > cut_lists[cl_pos].endpos) {
					cl_pos++;
					/* get out if there's no more lists to process */
					எனில் (cl_pos >= nlists)
						செல் next_வரி;
					spos = cut_lists[cl_pos].startpos;
					/* get out if the current line is lower than the one
					 * we just became interested in */
					எனில் ((முழு)linenum < spos)
						செல் next_வரி;
				}
			}

			/* If we made it here, it means we've found the line we're
			 * looking for, so print it */
			puts(வரி);
			செல் next_வரி;
		} இல்லையெனில் {		/* cut by fields */
			unsigned uu = 0, start = 0, end = 0, out = 0;
			முழு dcount = 0;

			/* Loop through bytes, finding next delimiter */
			க்கு (;;) {
				/* End of current range? */
				எனில் (end == linelen || dcount > cut_lists[cl_pos].endpos) {
					எனில் (++cl_pos >= nlists) நிறுத்து;
					எனில் (option_mask32 & CUT_OPT_NOSORT_FLGS)
						start = dcount = uu = 0;
					end = 0;
				}
				/* End of current line? */
				எனில் (uu == linelen) {
					/* If we've seen no delimiters, check -s */
					எனில் (!cl_pos && !dcount && !shoe) {
						எனில் (option_mask32 & CUT_OPT_SUPPRESS_FLGS)
							செல் next_வரி;
					} இல்லையெனில் எனில் (dcount < cut_lists[cl_pos].startpos)
						start = linelen;
					end = linelen;
				} இல்லையெனில் {
					/* Find next delimiter */
					எனில் (shoe) {
						regmatch_t rr = {-1, -1};

						எனில் (!regexec(&reg, வரி+uu, 1, &rr, REG_NOTBOL|REG_NOTEOL)) {
							end = uu + rr.rm_so;
							uu += rr.rm_eo;
						} இல்லையெனில் {
							uu = linelen;
							தொடர்;
						}
					} இல்லையெனில் எனில் (வரி[end = uu++] != *delim)
						தொடர்;

					/* Got delimiter. Loop if not yet within range. */
					எனில் (dcount++ < cut_lists[cl_pos].startpos) {
						start = uu;
						தொடர்;
					}
				}
				எனில் (end != start || !shoe)
					printf("%s%.*s", out++ ? odelim : "", end-start, வரி + start);
				start = uu;
				எனில் (!dcount)
					நிறுத்து;
			}
		}
		/* if we printed anything, finish with newline */
		putchar('\n');
 next_வரி:
		linenum++;
		விடுவி(printed);
		விடுவி(orig_வரி);
	}
}

முழு cut_main(முழு argc, எழுத்து **argv) MAIN_EXTERNALLY_VISIBLE;
முழு cut_main(முழு argc UNUSED_PARAM, எழுத்து **argv)
{
	/* growable array holding a series of lists */
	கட்டமைப்பு cut_list *cut_lists = வெற்று;
	unsigned nlists = 0;	/* number of elements in above list */
	எழுத்து *sopt, *ltok;
	மாறிலி எழுத்து *delim = வெற்று;
	மாறிலி எழுத்து *odelim = வெற்று;
	unsigned opt;

#define ARG "bcf"IF_FEATURE_CUT_REGEX("F")
	opt = getopt32(argv, "^"
			OPT_STR  // = "b:c:f:d:O:sD"IF_FEATURE_CUT_REGEX("F:")"n"
			"\0" "b--"ARG":c--"ARG":f--"ARG IF_FEATURE_CUT_REGEX("F--"ARG),
			&sopt, &sopt, &sopt, &delim, &odelim IF_FEATURE_CUT_REGEX(, &sopt)
	);
	எனில் (!delim || !*delim)
		delim = (opt & CUT_OPT_REGEX_FLGS) ? "[[:space:]]+" : "\t";
	எனில் (!odelim) odelim = (opt & CUT_OPT_REGEX_FLGS) ? " " : delim;

//	argc -= optind;
	argv += optind;
	எனில் (!(opt & (CUT_OPT_BYTE_FLGS | CUT_OPT_CHAR_FLGS | CUT_OPT_FIELDS_FLGS | CUT_OPT_REGEX_FLGS)))
		bb_simple_error_செய்தி_and_die("expected a list of bytes, characters, or fields");

	/*  non-field (char or byte) cutting has some special handling */
	எனில் (!(opt & (CUT_OPT_FIELDS_FLGS|CUT_OPT_REGEX_FLGS))) {
		நிலையான மாறிலி எழுத்து _op_on_field[] ALIGN1 = " only when operating on fields";

		எனில் (opt & CUT_OPT_SUPPRESS_FLGS) {
			bb_error_செய்தி_and_die
				("suppressing non-delimited lines makes sense%s", _op_on_field);
		}
		எனில் (opt & CUT_OPT_DELIM_FLGS) {
			bb_error_செய்தி_and_die
				("a delimiter may be specified%s", _op_on_field);
		}
	}

	/*
	 * parse list and put values into startpos and endpos.
	 * valid list formats: N, N-, N-M, -M
	 * more than one list can be separated by commas
	 */
	{
		எழுத்து *ntok;
		முழு s = 0, e = 0;

		/* take apart the lists, one by one (they are separated with commas) */
		போது ((ltok = strsep(&sopt, ",")) != வெற்று) {

			/* it's actually legal to pass an empty list */
			எனில் (!ltok[0])
				தொடர்;

			/* get the start pos */
			ntok = strsep(&ltok, "-");
			எனில் (!ntok[0]) {
				s = 0;
			} இல்லையெனில் {
				s = xatoi_positive(ntok);
				/* account for the fact that arrays are zero based, while
				 * the user expects the first char on the line to be char #1 */
				எனில் (s != 0)
					s--;
			}

			/* get the end pos */
			எனில் (ltok == வெற்று) {
				e = s;
			} இல்லையெனில் எனில் (!ltok[0]) {
				e = INT_MAX;
			} இல்லையெனில் {
				e = xatoi_positive(ltok);
				/* if the user specified and end position of 0,
				 * that means "til the end of the line" */
				எனில் (!*ltok)
					e = INT_MAX;
				இல்லையெனில் எனில் (e < s)
					bb_error_செய்தி_and_die("%d<%d", e, s);
				e--;	/* again, arrays are zero based, lines are 1 based */
			}

			/* add the new list */
			cut_lists = xrealloc_vector(cut_lists, 4, nlists);
			/* NB: startpos is always >= 0 */
			cut_lists[nlists].startpos = s;
			cut_lists[nlists].endpos = e;
			nlists++;
		}

		/* make sure we got some cut positions out of all that */
		எனில் (nlists == 0)
			bb_simple_error_செய்தி_and_die("missing list of positions");

		/* now that the lists are parsed, we need to sort them to make life
		 * easier on us when it comes time to print the chars / fields / lines
		 */
		எனில் (!(opt & CUT_OPT_NOSORT_FLGS))
			qsort(cut_lists, nlists, அளவு(cut_lists[0]), cmpfunc);
	}

	{
		exitcode_t retval = EXIT_SUCCESS;

		எனில் (!*argv)
			*--argv = (எழுத்து *)"-";

		செய் {
			FILE *file = fopen_or_warn_stdin(*argv);
			எனில் (!file) {
				retval = EXIT_FAILURE;
				தொடர்;
			}
			cut_file(file, delim, odelim, cut_lists, nlists);
			fclose_if_not_stdin(file);
		} போது (*++argv);

		எனில் (ENABLE_FEATURE_CLEAN_UP)
			விடுவி(cut_lists);
		fflush_stdout_and_exit(retval);
	}
}
