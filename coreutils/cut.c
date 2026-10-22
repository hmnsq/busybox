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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"

#ਜੇ ENABLE_FEATURE_CUT_REGEX
#ਅਨ੍ਤਰ੍ਭਾਵ "xregex.h"
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ regex_t ਪੂਰਨ
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ { ਪੂਰਨ rm_eo, rm_so; } regmatch_t;
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ xregcomp(x, ...) *(x) = 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ regexec(...)     0
#ਜੇ_ਅੰਤ

/* This is a NOEXEC applet. Be very careful! */


/* option vars */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPT_STR "b:c:f:d:O:sD"IF_FEATURE_CUT_REGEX("F:")"n"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CUT_OPT_BYTE_FLGS     (1 << 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CUT_OPT_CHAR_FLGS     (1 << 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CUT_OPT_FIELDS_FLGS   (1 << 2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CUT_OPT_DELIM_FLGS    (1 << 3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CUT_OPT_ODELIM_FLGS   (1 << 4)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CUT_OPT_SUPPRESS_FLGS (1 << 5)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CUT_OPT_NOSORT_FLGS   (1 << 6)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CUT_OPT_REGEX_FLGS    ((1 << 7) * ENABLE_FEATURE_CUT_REGEX)

ਬਣਤਰ cut_list {
	ਪੂਰਨ startpos;
	ਪੂਰਨ endpos;
};

ਸਥਿਰ ਪੂਰਨ cmpfunc(ਅਚਲ ਖਾਲੀ *a, ਅਚਲ ਖਾਲੀ *b)
{
	ਵਾਪਸ (((ਬਣਤਰ cut_list *) a)->startpos -
			((ਬਣਤਰ cut_list *) b)->startpos);
}

ਸਥਿਰ ਖਾਲੀ cut_file(FILE *file, ਅਚਲ ਅੱਖਰ *delim, ਅਚਲ ਅੱਖਰ *odelim,
		ਅਚਲ ਬਣਤਰ cut_list *cut_lists, ਬਿਨਾਨਿਸ਼ਾਨੀ nlists)
{
	ਅੱਖਰ *ਸਤਰ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ linenum = 0;	/* keep these zero-based to be consistent */
	regex_t reg;
	ਪੂਰਨ spos, shoe = option_mask32 & CUT_OPT_REGEX_FLGS;

	ਜੇ (shoe) xregcomp(&reg, delim, REG_EXTENDED);

	/* go through every line in the file */
	ਜਦੋਂ ((ਸਤਰ = xmalloc_fgetline(file)) != ਨਿਰਾ) {

		/* set up a list so we can keep track of what's been printed */
		ਪੂਰਨ linelen = ਲੜੀ_ਲੰਬਾਈ(ਸਤਰ);
		ਅੱਖਰ *printed = xzalloc(linelen + 1);
		ਅੱਖਰ *orig_ਸਤਰ = ਸਤਰ;
		ਬਿਨਾਨਿਸ਼ਾਨੀ cl_pos = 0;

		/* cut based on chars/bytes XXX: only works when sizeof(char) == byte */
		ਜੇ (option_mask32 & (CUT_OPT_CHAR_FLGS | CUT_OPT_BYTE_FLGS)) {
			/* print the chars specified in each cut list */
			ਲਈ (; cl_pos < nlists; cl_pos++) {
				ਲਈ (spos = cut_lists[cl_pos].startpos; spos < linelen;) {
					ਜੇ (!printed[spos]) {
						printed[spos] = 'X';
						ਅੱਖਰ_ਲਿਖੋ(ਸਤਰ[spos]);
					}
					ਜੇ (++spos > cut_lists[cl_pos].endpos) {
						ਰੋਕੋ;
					}
				}
			}
		} ਨਹੀਂਤਾਂ ਜੇ (*delim == '\n') {	/* cut by lines */
			spos = cut_lists[cl_pos].startpos;

			/* get out if we have no more lists to process or if the lines
			 * are lower than what we're interested in */
			ਜੇ (((ਪੂਰਨ)linenum < spos) || (cl_pos >= nlists))
				ਜਾਓ ਅਗਲਾ_ਸਤਰ;

			/* if the line we're looking for is lower than the one we were
			 * passed, it means we displayed it already, so move on */
			ਜਦੋਂ (spos < (ਪੂਰਨ)linenum) {
				spos++;
				/* go to the next list if we're at the end of this one */
				ਜੇ (spos > cut_lists[cl_pos].endpos) {
					cl_pos++;
					/* get out if there's no more lists to process */
					ਜੇ (cl_pos >= nlists)
						ਜਾਓ ਅਗਲਾ_ਸਤਰ;
					spos = cut_lists[cl_pos].startpos;
					/* get out if the current line is lower than the one
					 * we just became interested in */
					ਜੇ ((ਪੂਰਨ)linenum < spos)
						ਜਾਓ ਅਗਲਾ_ਸਤਰ;
				}
			}

			/* If we made it here, it means we've found the line we're
			 * looking for, so print it */
			puts(ਸਤਰ);
			ਜਾਓ ਅਗਲਾ_ਸਤਰ;
		} ਨਹੀਂਤਾਂ {		/* cut by fields */
			ਬਿਨਾਨਿਸ਼ਾਨੀ uu = 0, ਸ਼ੁਰੂ = 0, ਸਿਰਾ = 0, ਬਾਹਰਲਾ = 0;
			ਪੂਰਨ dcount = 0;

			/* Loop through bytes, finding next delimiter */
			ਲਈ (;;) {
				/* End of current range? */
				ਜੇ (ਸਿਰਾ == linelen || dcount > cut_lists[cl_pos].endpos) {
					ਜੇ (++cl_pos >= nlists) ਰੋਕੋ;
					ਜੇ (option_mask32 & CUT_OPT_NOSORT_FLGS)
						ਸ਼ੁਰੂ = dcount = uu = 0;
					ਸਿਰਾ = 0;
				}
				/* End of current line? */
				ਜੇ (uu == linelen) {
					/* If we've seen no delimiters, check -s */
					ਜੇ (!cl_pos && !dcount && !shoe) {
						ਜੇ (option_mask32 & CUT_OPT_SUPPRESS_FLGS)
							ਜਾਓ ਅਗਲਾ_ਸਤਰ;
					} ਨਹੀਂਤਾਂ ਜੇ (dcount < cut_lists[cl_pos].startpos)
						ਸ਼ੁਰੂ = linelen;
					ਸਿਰਾ = linelen;
				} ਨਹੀਂਤਾਂ {
					/* Find next delimiter */
					ਜੇ (shoe) {
						regmatch_t rr = {-1, -1};

						ਜੇ (!regexec(&reg, ਸਤਰ+uu, 1, &rr, REG_NOTBOL|REG_NOTEOL)) {
							ਸਿਰਾ = uu + rr.rm_so;
							uu += rr.rm_eo;
						} ਨਹੀਂਤਾਂ {
							uu = linelen;
							ਜਾਰੀ;
						}
					} ਨਹੀਂਤਾਂ ਜੇ (ਸਤਰ[ਸਿਰਾ = uu++] != *delim)
						ਜਾਰੀ;

					/* Got delimiter. Loop if not yet within range. */
					ਜੇ (dcount++ < cut_lists[cl_pos].startpos) {
						ਸ਼ੁਰੂ = uu;
						ਜਾਰੀ;
					}
				}
				ਜੇ (ਸਿਰਾ != ਸ਼ੁਰੂ || !shoe)
					ਲਿਖੋ("%s%.*s", ਬਾਹਰਲਾ++ ? odelim : "", ਸਿਰਾ-ਸ਼ੁਰੂ, ਸਤਰ + ਸ਼ੁਰੂ);
				ਸ਼ੁਰੂ = uu;
				ਜੇ (!dcount)
					ਰੋਕੋ;
			}
		}
		/* if we printed anything, finish with newline */
		ਅੱਖਰ_ਲਿਖੋ('\n');
 ਅਗਲਾ_ਸਤਰ:
		linenum++;
		ਮੁਕਤ(printed);
		ਮੁਕਤ(orig_ਸਤਰ);
	}
}

ਪੂਰਨ cut_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ cut_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
	/* growable array holding a series of lists */
	ਬਣਤਰ cut_list *cut_lists = ਨਿਰਾ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ nlists = 0;	/* number of elements in above list */
	ਅੱਖਰ *sopt, *ltok;
	ਅਚਲ ਅੱਖਰ *delim = ਨਿਰਾ;
	ਅਚਲ ਅੱਖਰ *odelim = ਨਿਰਾ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ opt;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ARG "bcf"IF_FEATURE_CUT_REGEX("F")
	opt = getopt32(argv, "^"
			OPT_STR  // = "b:c:f:d:O:sD"IF_FEATURE_CUT_REGEX("F:")"n"
			"\0" "b--"ARG":c--"ARG":f--"ARG IF_FEATURE_CUT_REGEX("F--"ARG),
			&sopt, &sopt, &sopt, &delim, &odelim IF_FEATURE_CUT_REGEX(, &sopt)
	);
	ਜੇ (!delim || !*delim)
		delim = (opt & CUT_OPT_REGEX_FLGS) ? "[[:space:]]+" : "\t";
	ਜੇ (!odelim) odelim = (opt & CUT_OPT_REGEX_FLGS) ? " " : delim;

//	argc -= optind;
	argv += optind;
	ਜੇ (!(opt & (CUT_OPT_BYTE_FLGS | CUT_OPT_CHAR_FLGS | CUT_OPT_FIELDS_FLGS | CUT_OPT_REGEX_FLGS)))
		bb_simple_error_ਸੁਨੇਹਾ_and_die("expected a list of bytes, characters, or fields");

	/*  non-field (char or byte) cutting has some special handling */
	ਜੇ (!(opt & (CUT_OPT_FIELDS_FLGS|CUT_OPT_REGEX_FLGS))) {
		ਸਥਿਰ ਅਚਲ ਅੱਖਰ op_on_ਖੇਤਰ[] ALIGN1 = " only when operating on fields";

		ਜੇ (opt & CUT_OPT_SUPPRESS_FLGS) {
			bb_error_ਸੁਨੇਹਾ_and_die
				("suppressing non-delimited lines makes sense%s", op_on_ਖੇਤਰ);
		}
		ਜੇ (opt & CUT_OPT_DELIM_FLGS) {
			bb_error_ਸੁਨੇਹਾ_and_die
				("a delimiter may be specified%s", op_on_ਖੇਤਰ);
		}
	}

	/*
	 * parse list and put values into startpos and endpos.
	 * valid list formats: N, N-, N-M, -M
	 * more than one list can be separated by commas
	 */
	{
		ਅੱਖਰ *ntok;
		ਪੂਰਨ s = 0, e = 0;

		/* take apart the lists, one by one (they are separated with commas) */
		ਜਦੋਂ ((ltok = strsep(&sopt, ",")) != ਨਿਰਾ) {

			/* it's actually legal to pass an empty list */
			ਜੇ (!ltok[0])
				ਜਾਰੀ;

			/* get the start pos */
			ntok = strsep(&ltok, "-");
			ਜੇ (!ntok[0]) {
				s = 0;
			} ਨਹੀਂਤਾਂ {
				s = xatoi_ਸਕਾਰਾਤਮਕ(ntok);
				/* account for the fact that arrays are zero based, while
				 * the user expects the first char on the line to be char #1 */
				ਜੇ (s != 0)
					s--;
			}

			/* get the end pos */
			ਜੇ (ltok == ਨਿਰਾ) {
				e = s;
			} ਨਹੀਂਤਾਂ ਜੇ (!ltok[0]) {
				e = ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ;
			} ਨਹੀਂਤਾਂ {
				e = xatoi_ਸਕਾਰਾਤਮਕ(ltok);
				/* if the user specified and end position of 0,
				 * that means "til the end of the line" */
				ਜੇ (!*ltok)
					e = ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ;
				ਨਹੀਂਤਾਂ ਜੇ (e < s)
					bb_error_ਸੁਨੇਹਾ_and_die("%d<%d", e, s);
				e--;	/* again, arrays are zero based, lines are 1 based */
			}

			/* add the new list */
			cut_lists = xrealloc_ਸਦਿਸ਼(cut_lists, 4, nlists);
			/* NB: startpos is always >= 0 */
			cut_lists[nlists].startpos = s;
			cut_lists[nlists].endpos = e;
			nlists++;
		}

		/* make sure we got some cut positions out of all that */
		ਜੇ (nlists == 0)
			bb_simple_error_ਸੁਨੇਹਾ_and_die("missing list of positions");

		/* now that the lists are parsed, we need to sort them to make life
		 * easier on us when it comes time to print the chars / fields / lines
		 */
		ਜੇ (!(opt & CUT_OPT_NOSORT_FLGS))
			qsort(cut_lists, nlists, ਆਕਾਰ(cut_lists[0]), cmpfunc);
	}

	{
		exitcode_t retval = EXIT_SUCCESS;

		ਜੇ (!*argv)
			*--argv = (ਅੱਖਰ *)"-";

		ਕਰੋ {
			FILE *file = fopen_or_warn_stdin(*argv);
			ਜੇ (!file) {
				retval = EXIT_FAILURE;
				ਜਾਰੀ;
			}
			cut_file(file, delim, odelim, cut_lists, nlists);
			fclose_if_not_stdin(file);
		} ਜਦੋਂ (*++argv);

		ਜੇ (ENABLE_FEATURE_CLEAN_ਉੱਪਰ)
			ਮੁਕਤ(cut_lists);
		fflush_stdout_and_exit(retval);
	}
}
