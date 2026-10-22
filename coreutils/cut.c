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

#ଅନ୍ତର୍ଭାଵ "libbb.h"

#ଯଦି ENABLE_FEATURE_CUT_REGEX
#ଅନ୍ତର୍ଭାଵ "xregex.h"
#ଅନ୍ୟଥା
#define regex_t ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା { ପୂର୍ଣ୍ଣସଂଖ୍ୟା rm_eo, rm_so; } regmatch_t;
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

ସଂରଚନା cut_list {
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା startpos;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା endpos;
};

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cmpfunc(ଅଚଳ ଶୂନ୍ୟ *a, ଅଚଳ ଶୂନ୍ୟ *b)
{
	ଫେରନ୍ତୁ (((ସଂରଚନା cut_list *) a)->startpos -
			((ସଂରଚନା cut_list *) b)->startpos);
}

ସ୍ଥିର ଶୂନ୍ୟ cut_file(FILE *file, ଅଚଳ ଅକ୍ଷର *delim, ଅଚଳ ଅକ୍ଷର *odelim,
		ଅଚଳ ସଂରଚନା cut_list *cut_lists, unsigned nlists)
{
	ଅକ୍ଷର *ଧାଡ଼ି;
	unsigned linenum = 0;	/* keep these zero-based to be consistent */
	regex_t reg;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା spos, shoe = option_mask32 & CUT_OPT_REGEX_FLGS;

	ଯଦି (shoe) xregcomp(&reg, delim, REG_EXTENDED);

	/* go through every line in the file */
	ଯେତେବେଳେ ((ଧାଡ଼ି = xmalloc_fgetline(file)) != ଖାଲି) {

		/* set up a list so we can keep track of what's been printed */
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା linelen = strlen(ଧାଡ଼ି);
		ଅକ୍ଷର *printed = xzalloc(linelen + 1);
		ଅକ୍ଷର *orig_ଧାଡ଼ି = ଧାଡ଼ି;
		unsigned cl_pos = 0;

		/* cut based on chars/bytes XXX: only works when sizeof(char) == byte */
		ଯଦି (option_mask32 & (CUT_OPT_CHAR_FLGS | CUT_OPT_BYTE_FLGS)) {
			/* print the chars specified in each cut list */
			ପାଇଁ (; cl_pos < nlists; cl_pos++) {
				ପାଇଁ (spos = cut_lists[cl_pos].startpos; spos < linelen;) {
					ଯଦି (!printed[spos]) {
						printed[spos] = 'X';
						putchar(ଧାଡ଼ି[spos]);
					}
					ଯଦି (++spos > cut_lists[cl_pos].endpos) {
						ଭାଙ୍ଗ;
					}
				}
			}
		} ଅନ୍ୟଥା ଯଦି (*delim == '\n') {	/* cut by lines */
			spos = cut_lists[cl_pos].startpos;

			/* get out if we have no more lists to process or if the lines
			 * are lower than what we're interested in */
			ଯଦି (((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)linenum < spos) || (cl_pos >= nlists))
				ଯାଅ next_ଧାଡ଼ି;

			/* if the line we're looking for is lower than the one we were
			 * passed, it means we displayed it already, so move on */
			ଯେତେବେଳେ (spos < (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)linenum) {
				spos++;
				/* go to the next list if we're at the end of this one */
				ଯଦି (spos > cut_lists[cl_pos].endpos) {
					cl_pos++;
					/* get out if there's no more lists to process */
					ଯଦି (cl_pos >= nlists)
						ଯାଅ next_ଧାଡ଼ି;
					spos = cut_lists[cl_pos].startpos;
					/* get out if the current line is lower than the one
					 * we just became interested in */
					ଯଦି ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)linenum < spos)
						ଯାଅ next_ଧାଡ଼ି;
				}
			}

			/* If we made it here, it means we've found the line we're
			 * looking for, so print it */
			puts(ଧାଡ଼ି);
			ଯାଅ next_ଧାଡ଼ି;
		} ଅନ୍ୟଥା {		/* cut by fields */
			unsigned uu = 0, start = 0, end = 0, out = 0;
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା dcount = 0;

			/* Loop through bytes, finding next delimiter */
			ପାଇଁ (;;) {
				/* End of current range? */
				ଯଦି (end == linelen || dcount > cut_lists[cl_pos].endpos) {
					ଯଦି (++cl_pos >= nlists) ଭାଙ୍ଗ;
					ଯଦି (option_mask32 & CUT_OPT_NOSORT_FLGS)
						start = dcount = uu = 0;
					end = 0;
				}
				/* End of current line? */
				ଯଦି (uu == linelen) {
					/* If we've seen no delimiters, check -s */
					ଯଦି (!cl_pos && !dcount && !shoe) {
						ଯଦି (option_mask32 & CUT_OPT_SUPPRESS_FLGS)
							ଯାଅ next_ଧାଡ଼ି;
					} ଅନ୍ୟଥା ଯଦି (dcount < cut_lists[cl_pos].startpos)
						start = linelen;
					end = linelen;
				} ଅନ୍ୟଥା {
					/* Find next delimiter */
					ଯଦି (shoe) {
						regmatch_t rr = {-1, -1};

						ଯଦି (!regexec(&reg, ଧାଡ଼ି+uu, 1, &rr, REG_NOTBOL|REG_NOTEOL)) {
							end = uu + rr.rm_so;
							uu += rr.rm_eo;
						} ଅନ୍ୟଥା {
							uu = linelen;
							ଜାରି;
						}
					} ଅନ୍ୟଥା ଯଦି (ଧାଡ଼ି[end = uu++] != *delim)
						ଜାରି;

					/* Got delimiter. Loop if not yet within range. */
					ଯଦି (dcount++ < cut_lists[cl_pos].startpos) {
						start = uu;
						ଜାରି;
					}
				}
				ଯଦି (end != start || !shoe)
					printf("%s%.*s", out++ ? odelim : "", end-start, ଧାଡ଼ି + start);
				start = uu;
				ଯଦି (!dcount)
					ଭାଙ୍ଗ;
			}
		}
		/* if we printed anything, finish with newline */
		putchar('\n');
 next_ଧାଡ଼ି:
		linenum++;
		ମୁକ୍ତ(printed);
		ମୁକ୍ତ(orig_ଧାଡ଼ି);
	}
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା cut_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv) MAIN_EXTERNALLY_VISIBLE;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା cut_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc UNUSED_PARAM, ଅକ୍ଷର **argv)
{
	/* growable array holding a series of lists */
	ସଂରଚନା cut_list *cut_lists = ଖାଲି;
	unsigned nlists = 0;	/* number of elements in above list */
	ଅକ୍ଷର *sopt, *ltok;
	ଅଚଳ ଅକ୍ଷର *delim = ଖାଲି;
	ଅଚଳ ଅକ୍ଷର *odelim = ଖାଲି;
	unsigned opt;

#define ARG "bcf"IF_FEATURE_CUT_REGEX("F")
	opt = getopt32(argv, "^"
			OPT_STR  // = "b:c:f:d:O:sD"IF_FEATURE_CUT_REGEX("F:")"n"
			"\0" "b--"ARG":c--"ARG":f--"ARG IF_FEATURE_CUT_REGEX("F--"ARG),
			&sopt, &sopt, &sopt, &delim, &odelim IF_FEATURE_CUT_REGEX(, &sopt)
	);
	ଯଦି (!delim || !*delim)
		delim = (opt & CUT_OPT_REGEX_FLGS) ? "[[:space:]]+" : "\t";
	ଯଦି (!odelim) odelim = (opt & CUT_OPT_REGEX_FLGS) ? " " : delim;

//	argc -= optind;
	argv += optind;
	ଯଦି (!(opt & (CUT_OPT_BYTE_FLGS | CUT_OPT_CHAR_FLGS | CUT_OPT_FIELDS_FLGS | CUT_OPT_REGEX_FLGS)))
		bb_simple_error_ସନ୍ଦେଶ_and_die("expected a list of bytes, characters, or fields");

	/*  non-field (char or byte) cutting has some special handling */
	ଯଦି (!(opt & (CUT_OPT_FIELDS_FLGS|CUT_OPT_REGEX_FLGS))) {
		ସ୍ଥିର ଅଚଳ ଅକ୍ଷର _op_on_field[] ALIGN1 = " only when operating on fields";

		ଯଦି (opt & CUT_OPT_SUPPRESS_FLGS) {
			bb_error_ସନ୍ଦେଶ_and_die
				("suppressing non-delimited lines makes sense%s", _op_on_field);
		}
		ଯଦି (opt & CUT_OPT_DELIM_FLGS) {
			bb_error_ସନ୍ଦେଶ_and_die
				("a delimiter may be specified%s", _op_on_field);
		}
	}

	/*
	 * parse list and put values into startpos and endpos.
	 * valid list formats: N, N-, N-M, -M
	 * more than one list can be separated by commas
	 */
	{
		ଅକ୍ଷର *ntok;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା s = 0, e = 0;

		/* take apart the lists, one by one (they are separated with commas) */
		ଯେତେବେଳେ ((ltok = strsep(&sopt, ",")) != ଖାଲି) {

			/* it's actually legal to pass an empty list */
			ଯଦି (!ltok[0])
				ଜାରି;

			/* get the start pos */
			ntok = strsep(&ltok, "-");
			ଯଦି (!ntok[0]) {
				s = 0;
			} ଅନ୍ୟଥା {
				s = xatoi_positive(ntok);
				/* account for the fact that arrays are zero based, while
				 * the user expects the first char on the line to be char #1 */
				ଯଦି (s != 0)
					s--;
			}

			/* get the end pos */
			ଯଦି (ltok == ଖାଲି) {
				e = s;
			} ଅନ୍ୟଥା ଯଦି (!ltok[0]) {
				e = INT_MAX;
			} ଅନ୍ୟଥା {
				e = xatoi_positive(ltok);
				/* if the user specified and end position of 0,
				 * that means "til the end of the line" */
				ଯଦି (!*ltok)
					e = INT_MAX;
				ଅନ୍ୟଥା ଯଦି (e < s)
					bb_error_ସନ୍ଦେଶ_and_die("%d<%d", e, s);
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
		ଯଦି (nlists == 0)
			bb_simple_error_ସନ୍ଦେଶ_and_die("missing list of positions");

		/* now that the lists are parsed, we need to sort them to make life
		 * easier on us when it comes time to print the chars / fields / lines
		 */
		ଯଦି (!(opt & CUT_OPT_NOSORT_FLGS))
			qsort(cut_lists, nlists, ଆକାର(cut_lists[0]), cmpfunc);
	}

	{
		exitcode_t retval = EXIT_SUCCESS;

		ଯଦି (!*argv)
			*--argv = (ଅକ୍ଷର *)"-";

		କର {
			FILE *file = fopen_or_warn_stdin(*argv);
			ଯଦି (!file) {
				retval = EXIT_FAILURE;
				ଜାରି;
			}
			cut_file(file, delim, odelim, cut_lists, nlists);
			fclose_if_not_stdin(file);
		} ଯେତେବେଳେ (*++argv);

		ଯଦି (ENABLE_FEATURE_CLEAN_UP)
			ମୁକ୍ତ(cut_lists);
		fflush_stdout_and_exit(retval);
	}
}
