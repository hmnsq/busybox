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

#ಅನ್ತರ್ಭಾವ "libbb.h"

#ಯದಿ ENABLE_FEATURE_CUT_REGEX
#ಅನ್ತರ್ಭಾವ "xregex.h"
#ಅನ್ಯಥಾ
#define regex_t ಪೂರ್ಣಾಙ್ಕ
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ { ಪೂರ್ಣಾಙ್ಕ rm_eo, rm_so; } regmatch_t;
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

ಸಂರಚನಾ cut_list {
	ಪೂರ್ಣಾಙ್ಕ startpos;
	ಪೂರ್ಣಾಙ್ಕ endpos;
};

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cmpfunc(ಧ್ರುವ ಶೂನ್ಯ *a, ಧ್ರುವ ಶೂನ್ಯ *b)
{
	ಪ್ರತ್ಯಾಗಮ (((ಸಂರಚನಾ cut_list *) a)->startpos -
			((ಸಂರಚನಾ cut_list *) b)->startpos);
}

ಸ್ಥಿರ ಶೂನ್ಯ cut_file(FILE *file, ಧ್ರುವ ವರ್ಣ *delim, ಧ್ರುವ ವರ್ಣ *odelim,
		ಧ್ರುವ ಸಂರಚನಾ cut_list *cut_lists, unsigned nlists)
{
	ವರ್ಣ *ಸಾಲು;
	unsigned linenum = 0;	/* keep these zero-based to be consistent */
	regex_t reg;
	ಪೂರ್ಣಾಙ್ಕ spos, shoe = option_mask32 & CUT_OPT_REGEX_FLGS;

	ಯದಿ (shoe) xregcomp(&reg, delim, REG_EXTENDED);

	/* go through every line in the file */
	ಯಾವತ್ ((ಸಾಲು = xmalloc_fgetline(file)) != ರಿಕ್ತ) {

		/* set up a list so we can keep track of what's been printed */
		ಪೂರ್ಣಾಙ್ಕ linelen = strlen(ಸಾಲು);
		ವರ್ಣ *printed = xzalloc(linelen + 1);
		ವರ್ಣ *orig_ಸಾಲು = ಸಾಲು;
		unsigned cl_pos = 0;

		/* cut based on chars/bytes XXX: only works when sizeof(char) == byte */
		ಯದಿ (option_mask32 & (CUT_OPT_CHAR_FLGS | CUT_OPT_BYTE_FLGS)) {
			/* print the chars specified in each cut list */
			ಪ್ರತಿ (; cl_pos < nlists; cl_pos++) {
				ಪ್ರತಿ (spos = cut_lists[cl_pos].startpos; spos < linelen;) {
					ಯದಿ (!printed[spos]) {
						printed[spos] = 'X';
						putchar(ಸಾಲು[spos]);
					}
					ಯದಿ (++spos > cut_lists[cl_pos].endpos) {
						ಭಙ್ಗ;
					}
				}
			}
		} ಅನ್ಯಥಾ ಯದಿ (*delim == '\n') {	/* cut by lines */
			spos = cut_lists[cl_pos].startpos;

			/* get out if we have no more lists to process or if the lines
			 * are lower than what we're interested in */
			ಯದಿ (((ಪೂರ್ಣಾಙ್ಕ)linenum < spos) || (cl_pos >= nlists))
				ಗಚ್ಛ next_ಸಾಲು;

			/* if the line we're looking for is lower than the one we were
			 * passed, it means we displayed it already, so move on */
			ಯಾವತ್ (spos < (ಪೂರ್ಣಾಙ್ಕ)linenum) {
				spos++;
				/* go to the next list if we're at the end of this one */
				ಯದಿ (spos > cut_lists[cl_pos].endpos) {
					cl_pos++;
					/* get out if there's no more lists to process */
					ಯದಿ (cl_pos >= nlists)
						ಗಚ್ಛ next_ಸಾಲು;
					spos = cut_lists[cl_pos].startpos;
					/* get out if the current line is lower than the one
					 * we just became interested in */
					ಯದಿ ((ಪೂರ್ಣಾಙ್ಕ)linenum < spos)
						ಗಚ್ಛ next_ಸಾಲು;
				}
			}

			/* If we made it here, it means we've found the line we're
			 * looking for, so print it */
			puts(ಸಾಲು);
			ಗಚ್ಛ next_ಸಾಲು;
		} ಅನ್ಯಥಾ {		/* cut by fields */
			unsigned uu = 0, start = 0, end = 0, out = 0;
			ಪೂರ್ಣಾಙ್ಕ dcount = 0;

			/* Loop through bytes, finding next delimiter */
			ಪ್ರತಿ (;;) {
				/* End of current range? */
				ಯದಿ (end == linelen || dcount > cut_lists[cl_pos].endpos) {
					ಯದಿ (++cl_pos >= nlists) ಭಙ್ಗ;
					ಯದಿ (option_mask32 & CUT_OPT_NOSORT_FLGS)
						start = dcount = uu = 0;
					end = 0;
				}
				/* End of current line? */
				ಯದಿ (uu == linelen) {
					/* If we've seen no delimiters, check -s */
					ಯದಿ (!cl_pos && !dcount && !shoe) {
						ಯದಿ (option_mask32 & CUT_OPT_SUPPRESS_FLGS)
							ಗಚ್ಛ next_ಸಾಲು;
					} ಅನ್ಯಥಾ ಯದಿ (dcount < cut_lists[cl_pos].startpos)
						start = linelen;
					end = linelen;
				} ಅನ್ಯಥಾ {
					/* Find next delimiter */
					ಯದಿ (shoe) {
						regmatch_t rr = {-1, -1};

						ಯದಿ (!regexec(&reg, ಸಾಲು+uu, 1, &rr, REG_NOTBOL|REG_NOTEOL)) {
							end = uu + rr.rm_so;
							uu += rr.rm_eo;
						} ಅನ್ಯಥಾ {
							uu = linelen;
							ಅನುವರ್ತ;
						}
					} ಅನ್ಯಥಾ ಯದಿ (ಸಾಲು[end = uu++] != *delim)
						ಅನುವರ್ತ;

					/* Got delimiter. Loop if not yet within range. */
					ಯದಿ (dcount++ < cut_lists[cl_pos].startpos) {
						start = uu;
						ಅನುವರ್ತ;
					}
				}
				ಯದಿ (end != start || !shoe)
					printf("%s%.*s", out++ ? odelim : "", end-start, ಸಾಲು + start);
				start = uu;
				ಯದಿ (!dcount)
					ಭಙ್ಗ;
			}
		}
		/* if we printed anything, finish with newline */
		putchar('\n');
 next_ಸಾಲು:
		linenum++;
		ಮುಕ್ತ(printed);
		ಮುಕ್ತ(orig_ಸಾಲು);
	}
}

ಪೂರ್ಣಾಙ್ಕ cut_main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಙ್ಕ cut_main(ಪೂರ್ಣಾಙ್ಕ argc UNUSED_PARAM, ವರ್ಣ **argv)
{
	/* growable array holding a series of lists */
	ಸಂರಚನಾ cut_list *cut_lists = ರಿಕ್ತ;
	unsigned nlists = 0;	/* number of elements in above list */
	ವರ್ಣ *sopt, *ltok;
	ಧ್ರುವ ವರ್ಣ *delim = ರಿಕ್ತ;
	ಧ್ರುವ ವರ್ಣ *odelim = ರಿಕ್ತ;
	unsigned opt;

#define ARG "bcf"IF_FEATURE_CUT_REGEX("F")
	opt = getopt32(argv, "^"
			OPT_STR  // = "b:c:f:d:O:sD"IF_FEATURE_CUT_REGEX("F:")"n"
			"\0" "b--"ARG":c--"ARG":f--"ARG IF_FEATURE_CUT_REGEX("F--"ARG),
			&sopt, &sopt, &sopt, &delim, &odelim IF_FEATURE_CUT_REGEX(, &sopt)
	);
	ಯದಿ (!delim || !*delim)
		delim = (opt & CUT_OPT_REGEX_FLGS) ? "[[:space:]]+" : "\t";
	ಯದಿ (!odelim) odelim = (opt & CUT_OPT_REGEX_FLGS) ? " " : delim;

//	argc -= optind;
	argv += optind;
	ಯದಿ (!(opt & (CUT_OPT_BYTE_FLGS | CUT_OPT_CHAR_FLGS | CUT_OPT_FIELDS_FLGS | CUT_OPT_REGEX_FLGS)))
		bb_simple_error_ಸಂದೇಶ_and_die("expected a list of bytes, characters, or fields");

	/*  non-field (char or byte) cutting has some special handling */
	ಯದಿ (!(opt & (CUT_OPT_FIELDS_FLGS|CUT_OPT_REGEX_FLGS))) {
		ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ _op_on_field[] ALIGN1 = " only when operating on fields";

		ಯದಿ (opt & CUT_OPT_SUPPRESS_FLGS) {
			bb_error_ಸಂದೇಶ_and_die
				("suppressing non-delimited lines makes sense%s", _op_on_field);
		}
		ಯದಿ (opt & CUT_OPT_DELIM_FLGS) {
			bb_error_ಸಂದೇಶ_and_die
				("a delimiter may be specified%s", _op_on_field);
		}
	}

	/*
	 * parse list and put values into startpos and endpos.
	 * valid list formats: N, N-, N-M, -M
	 * more than one list can be separated by commas
	 */
	{
		ವರ್ಣ *ntok;
		ಪೂರ್ಣಾಙ್ಕ s = 0, e = 0;

		/* take apart the lists, one by one (they are separated with commas) */
		ಯಾವತ್ ((ltok = strsep(&sopt, ",")) != ರಿಕ್ತ) {

			/* it's actually legal to pass an empty list */
			ಯದಿ (!ltok[0])
				ಅನುವರ್ತ;

			/* get the start pos */
			ntok = strsep(&ltok, "-");
			ಯದಿ (!ntok[0]) {
				s = 0;
			} ಅನ್ಯಥಾ {
				s = xatoi_positive(ntok);
				/* account for the fact that arrays are zero based, while
				 * the user expects the first char on the line to be char #1 */
				ಯದಿ (s != 0)
					s--;
			}

			/* get the end pos */
			ಯದಿ (ltok == ರಿಕ್ತ) {
				e = s;
			} ಅನ್ಯಥಾ ಯದಿ (!ltok[0]) {
				e = INT_MAX;
			} ಅನ್ಯಥಾ {
				e = xatoi_positive(ltok);
				/* if the user specified and end position of 0,
				 * that means "til the end of the line" */
				ಯದಿ (!*ltok)
					e = INT_MAX;
				ಅನ್ಯಥಾ ಯದಿ (e < s)
					bb_error_ಸಂದೇಶ_and_die("%d<%d", e, s);
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
		ಯದಿ (nlists == 0)
			bb_simple_error_ಸಂದೇಶ_and_die("missing list of positions");

		/* now that the lists are parsed, we need to sort them to make life
		 * easier on us when it comes time to print the chars / fields / lines
		 */
		ಯದಿ (!(opt & CUT_OPT_NOSORT_FLGS))
			qsort(cut_lists, nlists, ಪರಿಮಾಣ(cut_lists[0]), cmpfunc);
	}

	{
		exitcode_t retval = EXIT_SUCCESS;

		ಯದಿ (!*argv)
			*--argv = (ವರ್ಣ *)"-";

		ಕುರು {
			FILE *file = fopen_or_warn_stdin(*argv);
			ಯದಿ (!file) {
				retval = EXIT_FAILURE;
				ಅನುವರ್ತ;
			}
			cut_file(file, delim, odelim, cut_lists, nlists);
			fclose_if_not_stdin(file);
		} ಯಾವತ್ (*++argv);

		ಯದಿ (ENABLE_FEATURE_CLEAN_UP)
			ಮುಕ್ತ(cut_lists);
		fflush_stdout_and_exit(retval);
	}
}
