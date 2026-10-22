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

#ಆದರೆ ENABLE_FEATURE_CUT_REGEX
#ಅನ್ತರ್ಭಾವ "xregex.h"
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು regex_t ಪೂರ್ಣಾಂಕ
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ { ಪೂರ್ಣಾಂಕ rm_eo, rm_so; } regmatch_t;
#ವ್ಯಾಖ್ಯಾನಿಸು xregcomp(x, ...) *(x) = 0
#ವ್ಯಾಖ್ಯಾನಿಸು regexec(...)     0
#ಆದರೆ_ಅಂತ್ಯ

/* This is a NOEXEC applet. Be very careful! */


/* option vars */
#ವ್ಯಾಖ್ಯಾನಿಸು OPT_STR "b:c:f:d:O:sD"IF_FEATURE_CUT_REGEX("F:")"n"
#ವ್ಯಾಖ್ಯಾನಿಸು CUT_OPT_BYTE_FLGS     (1 << 0)
#ವ್ಯಾಖ್ಯಾನಿಸು CUT_OPT_CHAR_FLGS     (1 << 1)
#ವ್ಯಾಖ್ಯಾನಿಸು CUT_OPT_FIELDS_FLGS   (1 << 2)
#ವ್ಯಾಖ್ಯಾನಿಸು CUT_OPT_DELIM_FLGS    (1 << 3)
#ವ್ಯಾಖ್ಯಾನಿಸು CUT_OPT_ODELIM_FLGS   (1 << 4)
#ವ್ಯಾಖ್ಯಾನಿಸು CUT_OPT_SUPPRESS_FLGS (1 << 5)
#ವ್ಯಾಖ್ಯಾನಿಸು CUT_OPT_NOSORT_FLGS   (1 << 6)
#ವ್ಯಾಖ್ಯಾನಿಸು CUT_OPT_REGEX_FLGS    ((1 << 7) * ENABLE_FEATURE_CUT_REGEX)

ರಚನೆ cut_list {
	ಪೂರ್ಣಾಂಕ startpos;
	ಪೂರ್ಣಾಂಕ endpos;
};

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cmpfunc(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *a, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *b)
{
	ಹಿಂತಿರುಗಿ (((ರಚನೆ cut_list *) a)->startpos -
			((ರಚನೆ cut_list *) b)->startpos);
}

ಸ್ಥಿರ ಶೂನ್ಯ cut_file(FILE *file, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *delim, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *odelim,
		ಸ್ಥಿರಾಂಕ ರಚನೆ cut_list *cut_lists, ಚಿಹ್ನೆರಹಿತ nlists)
{
	ಅಕ್ಷರ *ಸಾಲು;
	ಚಿಹ್ನೆರಹಿತ linenum = 0;	/* keep these zero-based to be consistent */
	regex_t reg;
	ಪೂರ್ಣಾಂಕ spos, shoe = option_mask32 & CUT_OPT_REGEX_FLGS;

	ಆದರೆ (shoe) xregcomp(&reg, delim, REG_EXTENDED);

	/* go through every line in the file */
	ಆಗ ((ಸಾಲು = xmalloc_fgetline(file)) != ಖಾಲಿ) {

		/* set up a list so we can keep track of what's been printed */
		ಪೂರ್ಣಾಂಕ linelen = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಸಾಲು);
		ಅಕ್ಷರ *printed = xzalloc(linelen + 1);
		ಅಕ್ಷರ *orig_ಸಾಲು = ಸಾಲು;
		ಚಿಹ್ನೆರಹಿತ cl_pos = 0;

		/* cut based on chars/bytes XXX: only works when sizeof(char) == byte */
		ಆದರೆ (option_mask32 & (CUT_OPT_CHAR_FLGS | CUT_OPT_BYTE_FLGS)) {
			/* print the chars specified in each cut list */
			ಗಾಗಿ (; cl_pos < nlists; cl_pos++) {
				ಗಾಗಿ (spos = cut_lists[cl_pos].startpos; spos < linelen;) {
					ಆದರೆ (!printed[spos]) {
						printed[spos] = 'X';
						ಅಕ್ಷರ_ಬರೆ(ಸಾಲು[spos]);
					}
					ಆದರೆ (++spos > cut_lists[cl_pos].endpos) {
						ನಿಲ್ಲಿಸು;
					}
				}
			}
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*delim == '\n') {	/* cut by lines */
			spos = cut_lists[cl_pos].startpos;

			/* get out if we have no more lists to process or if the lines
			 * are lower than what we're interested in */
			ಆದರೆ (((ಪೂರ್ಣಾಂಕ)linenum < spos) || (cl_pos >= nlists))
				ಹೋಗು ಮುಂದಿನ_ಸಾಲು;

			/* if the line we're looking for is lower than the one we were
			 * passed, it means we displayed it already, so move on */
			ಆಗ (spos < (ಪೂರ್ಣಾಂಕ)linenum) {
				spos++;
				/* go to the next list if we're at the end of this one */
				ಆದರೆ (spos > cut_lists[cl_pos].endpos) {
					cl_pos++;
					/* get out if there's no more lists to process */
					ಆದರೆ (cl_pos >= nlists)
						ಹೋಗು ಮುಂದಿನ_ಸಾಲು;
					spos = cut_lists[cl_pos].startpos;
					/* get out if the current line is lower than the one
					 * we just became interested in */
					ಆದರೆ ((ಪೂರ್ಣಾಂಕ)linenum < spos)
						ಹೋಗು ಮುಂದಿನ_ಸಾಲು;
				}
			}

			/* If we made it here, it means we've found the line we're
			 * looking for, so print it */
			puts(ಸಾಲು);
			ಹೋಗು ಮುಂದಿನ_ಸಾಲು;
		} ಇಲ್ಲದಿದ್ದರೆ {		/* cut by fields */
			ಚಿಹ್ನೆರಹಿತ uu = 0, ಆರಂಭ = 0, ಕೊನೆ = 0, ಹೊರಹೋಗು = 0;
			ಪೂರ್ಣಾಂಕ dcount = 0;

			/* Loop through bytes, finding next delimiter */
			ಗಾಗಿ (;;) {
				/* End of current range? */
				ಆದರೆ (ಕೊನೆ == linelen || dcount > cut_lists[cl_pos].endpos) {
					ಆದರೆ (++cl_pos >= nlists) ನಿಲ್ಲಿಸು;
					ಆದರೆ (option_mask32 & CUT_OPT_NOSORT_FLGS)
						ಆರಂಭ = dcount = uu = 0;
					ಕೊನೆ = 0;
				}
				/* End of current line? */
				ಆದರೆ (uu == linelen) {
					/* If we've seen no delimiters, check -s */
					ಆದರೆ (!cl_pos && !dcount && !shoe) {
						ಆದರೆ (option_mask32 & CUT_OPT_SUPPRESS_FLGS)
							ಹೋಗು ಮುಂದಿನ_ಸಾಲು;
					} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dcount < cut_lists[cl_pos].startpos)
						ಆರಂಭ = linelen;
					ಕೊನೆ = linelen;
				} ಇಲ್ಲದಿದ್ದರೆ {
					/* Find next delimiter */
					ಆದರೆ (shoe) {
						regmatch_t rr = {-1, -1};

						ಆದರೆ (!regexec(&reg, ಸಾಲು+uu, 1, &rr, REG_NOTBOL|REG_NOTEOL)) {
							ಕೊನೆ = uu + rr.rm_so;
							uu += rr.rm_eo;
						} ಇಲ್ಲದಿದ್ದರೆ {
							uu = linelen;
							ಮುಂದುವರಿ;
						}
					} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಸಾಲು[ಕೊನೆ = uu++] != *delim)
						ಮುಂದುವರಿ;

					/* Got delimiter. Loop if not yet within range. */
					ಆದರೆ (dcount++ < cut_lists[cl_pos].startpos) {
						ಆರಂಭ = uu;
						ಮುಂದುವರಿ;
					}
				}
				ಆದರೆ (ಕೊನೆ != ಆರಂಭ || !shoe)
					ಬರೆ("%s%.*s", ಹೊರಹೋಗು++ ? odelim : "", ಕೊನೆ-ಆರಂಭ, ಸಾಲು + ಆರಂಭ);
				ಆರಂಭ = uu;
				ಆದರೆ (!dcount)
					ನಿಲ್ಲಿಸು;
			}
		}
		/* if we printed anything, finish with newline */
		ಅಕ್ಷರ_ಬರೆ('\n');
 ಮುಂದಿನ_ಸಾಲು:
		linenum++;
		ಬಿಡುಗಡೆ(printed);
		ಬಿಡುಗಡೆ(orig_ಸಾಲು);
	}
}

ಪೂರ್ಣಾಂಕ cut_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ cut_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	/* growable array holding a series of lists */
	ರಚನೆ cut_list *cut_lists = ಖಾಲಿ;
	ಚಿಹ್ನೆರಹಿತ nlists = 0;	/* number of elements in above list */
	ಅಕ್ಷರ *sopt, *ltok;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *delim = ಖಾಲಿ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *odelim = ಖಾಲಿ;
	ಚಿಹ್ನೆರಹಿತ opt;

#ವ್ಯಾಖ್ಯಾನಿಸು ARG "bcf"IF_FEATURE_CUT_REGEX("F")
	opt = getopt32(argv, "^"
			OPT_STR  // = "b:c:f:d:O:sD"IF_FEATURE_CUT_REGEX("F:")"n"
			"\0" "b--"ARG":c--"ARG":f--"ARG IF_FEATURE_CUT_REGEX("F--"ARG),
			&sopt, &sopt, &sopt, &delim, &odelim IF_FEATURE_CUT_REGEX(, &sopt)
	);
	ಆದರೆ (!delim || !*delim)
		delim = (opt & CUT_OPT_REGEX_FLGS) ? "[[:space:]]+" : "\t";
	ಆದರೆ (!odelim) odelim = (opt & CUT_OPT_REGEX_FLGS) ? " " : delim;

//	argc -= optind;
	argv += optind;
	ಆದರೆ (!(opt & (CUT_OPT_BYTE_FLGS | CUT_OPT_CHAR_FLGS | CUT_OPT_FIELDS_FLGS | CUT_OPT_REGEX_FLGS)))
		bb_simple_error_ಸಂದೇಶ_and_die("expected a list of bytes, characters, or fields");

	/*  non-field (char or byte) cutting has some special handling */
	ಆದರೆ (!(opt & (CUT_OPT_FIELDS_FLGS|CUT_OPT_REGEX_FLGS))) {
		ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ op_on_ಕ್ಷೇತ್ರ[] ALIGN1 = " only when operating on fields";

		ಆದರೆ (opt & CUT_OPT_SUPPRESS_FLGS) {
			bb_error_ಸಂದೇಶ_and_die
				("suppressing non-delimited lines makes sense%s", op_on_ಕ್ಷೇತ್ರ);
		}
		ಆದರೆ (opt & CUT_OPT_DELIM_FLGS) {
			bb_error_ಸಂದೇಶ_and_die
				("a delimiter may be specified%s", op_on_ಕ್ಷೇತ್ರ);
		}
	}

	/*
	 * parse list and put values into startpos and endpos.
	 * valid list formats: N, N-, N-M, -M
	 * more than one list can be separated by commas
	 */
	{
		ಅಕ್ಷರ *ntok;
		ಪೂರ್ಣಾಂಕ s = 0, e = 0;

		/* take apart the lists, one by one (they are separated with commas) */
		ಆಗ ((ltok = strsep(&sopt, ",")) != ಖಾಲಿ) {

			/* it's actually legal to pass an empty list */
			ಆದರೆ (!ltok[0])
				ಮುಂದುವರಿ;

			/* get the start pos */
			ntok = strsep(&ltok, "-");
			ಆದರೆ (!ntok[0]) {
				s = 0;
			} ಇಲ್ಲದಿದ್ದರೆ {
				s = xatoi_ಧನಾತ್ಮಕ(ntok);
				/* account for the fact that arrays are zero based, while
				 * the user expects the first char on the line to be char #1 */
				ಆದರೆ (s != 0)
					s--;
			}

			/* get the end pos */
			ಆದರೆ (ltok == ಖಾಲಿ) {
				e = s;
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!ltok[0]) {
				e = ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ;
			} ಇಲ್ಲದಿದ್ದರೆ {
				e = xatoi_ಧನಾತ್ಮಕ(ltok);
				/* if the user specified and end position of 0,
				 * that means "til the end of the line" */
				ಆದರೆ (!*ltok)
					e = ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ;
				ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (e < s)
					bb_error_ಸಂದೇಶ_and_die("%d<%d", e, s);
				e--;	/* again, arrays are zero based, lines are 1 based */
			}

			/* add the new list */
			cut_lists = xrealloc_ಸದಿಶ(cut_lists, 4, nlists);
			/* NB: startpos is always >= 0 */
			cut_lists[nlists].startpos = s;
			cut_lists[nlists].endpos = e;
			nlists++;
		}

		/* make sure we got some cut positions out of all that */
		ಆದರೆ (nlists == 0)
			bb_simple_error_ಸಂದೇಶ_and_die("missing list of positions");

		/* now that the lists are parsed, we need to sort them to make life
		 * easier on us when it comes time to print the chars / fields / lines
		 */
		ಆದರೆ (!(opt & CUT_OPT_NOSORT_FLGS))
			qsort(cut_lists, nlists, ಗಾತ್ರ(cut_lists[0]), cmpfunc);
	}

	{
		exitcode_t retval = EXIT_SUCCESS;

		ಆದರೆ (!*argv)
			*--argv = (ಅಕ್ಷರ *)"-";

		ಮಾಡು {
			FILE *file = fopen_or_warn_stdin(*argv);
			ಆದರೆ (!file) {
				retval = EXIT_FAILURE;
				ಮುಂದುವರಿ;
			}
			cut_file(file, delim, odelim, cut_lists, nlists);
			fclose_if_not_stdin(file);
		} ಆಗ (*++argv);

		ಆದರೆ (ENABLE_FEATURE_CLEAN_ಮೇಲೆ)
			ಬಿಡುಗಡೆ(cut_lists);
		fflush_stdout_and_exit(retval);
	}
}
