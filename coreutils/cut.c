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

#అన్తర్భావ "libbb.h"

#ఒకవేళ ENABLE_FEATURE_CUT_REGEX
#అన్తర్భావ "xregex.h"
#లేకపోతే
#నిర్వచించు regex_t పూర్ణాంకం
రకనిర్వచనం నిర్మాణం { పూర్ణాంకం rm_eo, rm_so; } regmatch_t;
#నిర్వచించు xregcomp(x, ...) *(x) = 0
#నిర్వచించు regexec(...)     0
#అయితే_ముగింపు

/* This is a NOEXEC applet. Be very careful! */


/* option vars */
#నిర్వచించు OPT_STR "b:c:f:d:O:sD"IF_FEATURE_CUT_REGEX("F:")"n"
#నిర్వచించు CUT_OPT_BYTE_FLGS     (1 << 0)
#నిర్వచించు CUT_OPT_CHAR_FLGS     (1 << 1)
#నిర్వచించు CUT_OPT_FIELDS_FLGS   (1 << 2)
#నిర్వచించు CUT_OPT_DELIM_FLGS    (1 << 3)
#నిర్వచించు CUT_OPT_ODELIM_FLGS   (1 << 4)
#నిర్వచించు CUT_OPT_SUPPRESS_FLGS (1 << 5)
#నిర్వచించు CUT_OPT_NOSORT_FLGS   (1 << 6)
#నిర్వచించు CUT_OPT_REGEX_FLGS    ((1 << 7) * ENABLE_FEATURE_CUT_REGEX)

నిర్మాణం cut_list {
	పూర్ణాంకం startpos;
	పూర్ణాంకం endpos;
};

స్థిర పూర్ణాంకం cmpfunc(స్థిరరాశి శూన్య *a, స్థిరరాశి శూన్య *b)
{
	తిరిగి (((నిర్మాణం cut_list *) a)->startpos -
			((నిర్మాణం cut_list *) b)->startpos);
}

స్థిర శూన్య cut_file(FILE *file, స్థిరరాశి అక్షరం *delim, స్థిరరాశి అక్షరం *odelim,
		స్థిరరాశి నిర్మాణం cut_list *cut_lists, గుర్తులేని nlists)
{
	అక్షరం *పంక్తి;
	గుర్తులేని linenum = 0;	/* keep these zero-based to be consistent */
	regex_t reg;
	పూర్ణాంకం spos, shoe = option_mask32 & CUT_OPT_REGEX_FLGS;

	ఒకవేళ (shoe) xregcomp(&reg, delim, REG_EXTENDED);

	/* go through every line in the file */
	అయితే ((పంక్తి = xmalloc_fgetline(file)) != ఖాళీ) {

		/* set up a list so we can keep track of what's been printed */
		పూర్ణాంకం linelen = అక్షరశ్రేణి_పొడవు(పంక్తి);
		అక్షరం *printed = xzalloc(linelen + 1);
		అక్షరం *orig_పంక్తి = పంక్తి;
		గుర్తులేని cl_pos = 0;

		/* cut based on chars/bytes XXX: only works when sizeof(char) == byte */
		ఒకవేళ (option_mask32 & (CUT_OPT_CHAR_FLGS | CUT_OPT_BYTE_FLGS)) {
			/* print the chars specified in each cut list */
			కొరకు (; cl_pos < nlists; cl_pos++) {
				కొరకు (spos = cut_lists[cl_pos].startpos; spos < linelen;) {
					ఒకవేళ (!printed[spos]) {
						printed[spos] = 'X';
						అక్షరం_రాయి(పంక్తి[spos]);
					}
					ఒకవేళ (++spos > cut_lists[cl_pos].endpos) {
						ఆపు;
					}
				}
			}
		} లేకపోతే ఒకవేళ (*delim == '\n') {	/* cut by lines */
			spos = cut_lists[cl_pos].startpos;

			/* get out if we have no more lists to process or if the lines
			 * are lower than what we're interested in */
			ఒకవేళ (((పూర్ణాంకం)linenum < spos) || (cl_pos >= nlists))
				వెళ్ళు తదుపరి_పంక్తి;

			/* if the line we're looking for is lower than the one we were
			 * passed, it means we displayed it already, so move on */
			అయితే (spos < (పూర్ణాంకం)linenum) {
				spos++;
				/* go to the next list if we're at the end of this one */
				ఒకవేళ (spos > cut_lists[cl_pos].endpos) {
					cl_pos++;
					/* get out if there's no more lists to process */
					ఒకవేళ (cl_pos >= nlists)
						వెళ్ళు తదుపరి_పంక్తి;
					spos = cut_lists[cl_pos].startpos;
					/* get out if the current line is lower than the one
					 * we just became interested in */
					ఒకవేళ ((పూర్ణాంకం)linenum < spos)
						వెళ్ళు తదుపరి_పంక్తి;
				}
			}

			/* If we made it here, it means we've found the line we're
			 * looking for, so print it */
			puts(పంక్తి);
			వెళ్ళు తదుపరి_పంక్తి;
		} లేకపోతే {		/* cut by fields */
			గుర్తులేని uu = 0, ప్రారంభం = 0, చివర = 0, బహిర్గతం = 0;
			పూర్ణాంకం dcount = 0;

			/* Loop through bytes, finding next delimiter */
			కొరకు (;;) {
				/* End of current range? */
				ఒకవేళ (చివర == linelen || dcount > cut_lists[cl_pos].endpos) {
					ఒకవేళ (++cl_pos >= nlists) ఆపు;
					ఒకవేళ (option_mask32 & CUT_OPT_NOSORT_FLGS)
						ప్రారంభం = dcount = uu = 0;
					చివర = 0;
				}
				/* End of current line? */
				ఒకవేళ (uu == linelen) {
					/* If we've seen no delimiters, check -s */
					ఒకవేళ (!cl_pos && !dcount && !shoe) {
						ఒకవేళ (option_mask32 & CUT_OPT_SUPPRESS_FLGS)
							వెళ్ళు తదుపరి_పంక్తి;
					} లేకపోతే ఒకవేళ (dcount < cut_lists[cl_pos].startpos)
						ప్రారంభం = linelen;
					చివర = linelen;
				} లేకపోతే {
					/* Find next delimiter */
					ఒకవేళ (shoe) {
						regmatch_t rr = {-1, -1};

						ఒకవేళ (!regexec(&reg, పంక్తి+uu, 1, &rr, REG_NOTBOL|REG_NOTEOL)) {
							చివర = uu + rr.rm_so;
							uu += rr.rm_eo;
						} లేకపోతే {
							uu = linelen;
							కొనసాగు;
						}
					} లేకపోతే ఒకవేళ (పంక్తి[చివర = uu++] != *delim)
						కొనసాగు;

					/* Got delimiter. Loop if not yet within range. */
					ఒకవేళ (dcount++ < cut_lists[cl_pos].startpos) {
						ప్రారంభం = uu;
						కొనసాగు;
					}
				}
				ఒకవేళ (చివర != ప్రారంభం || !shoe)
					రాయి("%s%.*s", బహిర్గతం++ ? odelim : "", చివర-ప్రారంభం, పంక్తి + ప్రారంభం);
				ప్రారంభం = uu;
				ఒకవేళ (!dcount)
					ఆపు;
			}
		}
		/* if we printed anything, finish with newline */
		అక్షరం_రాయి('\n');
 తదుపరి_పంక్తి:
		linenum++;
		విడుదల(printed);
		విడుదల(orig_పంక్తి);
	}
}

పూర్ణాంకం cut_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం cut_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	/* growable array holding a series of lists */
	నిర్మాణం cut_list *cut_lists = ఖాళీ;
	గుర్తులేని nlists = 0;	/* number of elements in above list */
	అక్షరం *sopt, *ltok;
	స్థిరరాశి అక్షరం *delim = ఖాళీ;
	స్థిరరాశి అక్షరం *odelim = ఖాళీ;
	గుర్తులేని opt;

#నిర్వచించు ARG "bcf"IF_FEATURE_CUT_REGEX("F")
	opt = getopt32(argv, "^"
			OPT_STR  // = "b:c:f:d:O:sD"IF_FEATURE_CUT_REGEX("F:")"n"
			"\0" "b--"ARG":c--"ARG":f--"ARG IF_FEATURE_CUT_REGEX("F--"ARG),
			&sopt, &sopt, &sopt, &delim, &odelim IF_FEATURE_CUT_REGEX(, &sopt)
	);
	ఒకవేళ (!delim || !*delim)
		delim = (opt & CUT_OPT_REGEX_FLGS) ? "[[:space:]]+" : "\t";
	ఒకవేళ (!odelim) odelim = (opt & CUT_OPT_REGEX_FLGS) ? " " : delim;

//	argc -= optind;
	argv += optind;
	ఒకవేళ (!(opt & (CUT_OPT_BYTE_FLGS | CUT_OPT_CHAR_FLGS | CUT_OPT_FIELDS_FLGS | CUT_OPT_REGEX_FLGS)))
		bb_simple_error_సందేశం_and_die("expected a list of bytes, characters, or fields");

	/*  non-field (char or byte) cutting has some special handling */
	ఒకవేళ (!(opt & (CUT_OPT_FIELDS_FLGS|CUT_OPT_REGEX_FLGS))) {
		స్థిర స్థిరరాశి అక్షరం op_on_క్షేత్రం[] ALIGN1 = " only when operating on fields";

		ఒకవేళ (opt & CUT_OPT_SUPPRESS_FLGS) {
			bb_error_సందేశం_and_die
				("suppressing non-delimited lines makes sense%s", op_on_క్షేత్రం);
		}
		ఒకవేళ (opt & CUT_OPT_DELIM_FLGS) {
			bb_error_సందేశం_and_die
				("a delimiter may be specified%s", op_on_క్షేత్రం);
		}
	}

	/*
	 * parse list and put values into startpos and endpos.
	 * valid list formats: N, N-, N-M, -M
	 * more than one list can be separated by commas
	 */
	{
		అక్షరం *ntok;
		పూర్ణాంకం s = 0, e = 0;

		/* take apart the lists, one by one (they are separated with commas) */
		అయితే ((ltok = strsep(&sopt, ",")) != ఖాళీ) {

			/* it's actually legal to pass an empty list */
			ఒకవేళ (!ltok[0])
				కొనసాగు;

			/* get the start pos */
			ntok = strsep(&ltok, "-");
			ఒకవేళ (!ntok[0]) {
				s = 0;
			} లేకపోతే {
				s = xatoi_ధనాత్మక(ntok);
				/* account for the fact that arrays are zero based, while
				 * the user expects the first char on the line to be char #1 */
				ఒకవేళ (s != 0)
					s--;
			}

			/* get the end pos */
			ఒకవేళ (ltok == ఖాళీ) {
				e = s;
			} లేకపోతే ఒకవేళ (!ltok[0]) {
				e = గరిష్ఠ_పూర్ణాంకం;
			} లేకపోతే {
				e = xatoi_ధనాత్మక(ltok);
				/* if the user specified and end position of 0,
				 * that means "til the end of the line" */
				ఒకవేళ (!*ltok)
					e = గరిష్ఠ_పూర్ణాంకం;
				లేకపోతే ఒకవేళ (e < s)
					bb_error_సందేశం_and_die("%d<%d", e, s);
				e--;	/* again, arrays are zero based, lines are 1 based */
			}

			/* add the new list */
			cut_lists = xrealloc_సదిశ(cut_lists, 4, nlists);
			/* NB: startpos is always >= 0 */
			cut_lists[nlists].startpos = s;
			cut_lists[nlists].endpos = e;
			nlists++;
		}

		/* make sure we got some cut positions out of all that */
		ఒకవేళ (nlists == 0)
			bb_simple_error_సందేశం_and_die("missing list of positions");

		/* now that the lists are parsed, we need to sort them to make life
		 * easier on us when it comes time to print the chars / fields / lines
		 */
		ఒకవేళ (!(opt & CUT_OPT_NOSORT_FLGS))
			qsort(cut_lists, nlists, పరిమాణం(cut_lists[0]), cmpfunc);
	}

	{
		exitcode_t retval = EXIT_SUCCESS;

		ఒకవేళ (!*argv)
			*--argv = (అక్షరం *)"-";

		చేయి {
			FILE *file = fopen_or_warn_stdin(*argv);
			ఒకవేళ (!file) {
				retval = EXIT_FAILURE;
				కొనసాగు;
			}
			cut_file(file, delim, odelim, cut_lists, nlists);
			fclose_if_not_stdin(file);
		} అయితే (*++argv);

		ఒకవేళ (ENABLE_FEATURE_CLEAN_పైకి)
			విడుదల(cut_lists);
		fflush_stdout_and_exit(retval);
	}
}
