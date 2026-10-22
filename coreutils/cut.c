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

#অন্তর্ভাৱ "libbb.h"

#যদি ENABLE_FEATURE_CUT_REGEX
#অন্তর্ভাৱ "xregex.h"
#অন্যথা
#define regex_t পূর্ণাঙ্ক
প্রকারনির্দেশ সংরচনা { পূর্ণাঙ্ক rm_eo, rm_so; } regmatch_t;
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

সংরচনা cut_list {
	পূর্ণাঙ্ক startpos;
	পূর্ণাঙ্ক endpos;
};

স্থির পূর্ণাঙ্ক cmpfunc(ধ্রুৱ শূন্য *a, ধ্রুৱ শূন্য *b)
{
	ঘূৰাই (((সংরচনা cut_list *) a)->startpos -
			((সংরচনা cut_list *) b)->startpos);
}

স্থির শূন্য cut_file(FILE *file, ধ্রুৱ ৱর্ণ *delim, ধ্রুৱ ৱর্ণ *odelim,
		ধ্রুৱ সংরচনা cut_list *cut_lists, unsigned nlists)
{
	ৱর্ণ *সাৰি;
	unsigned linenum = 0;	/* keep these zero-based to be consistent */
	regex_t reg;
	পূর্ণাঙ্ক spos, shoe = option_mask32 & CUT_OPT_REGEX_FLGS;

	যদি (shoe) xregcomp(&reg, delim, REG_EXTENDED);

	/* go through every line in the file */
	যেতিয়ালৈ ((সাৰি = xmalloc_fgetline(file)) != রিক্ত) {

		/* set up a list so we can keep track of what's been printed */
		পূর্ণাঙ্ক linelen = strlen(সাৰি);
		ৱর্ণ *printed = xzalloc(linelen + 1);
		ৱর্ণ *orig_সাৰি = সাৰি;
		unsigned cl_pos = 0;

		/* cut based on chars/bytes XXX: only works when sizeof(char) == byte */
		যদি (option_mask32 & (CUT_OPT_CHAR_FLGS | CUT_OPT_BYTE_FLGS)) {
			/* print the chars specified in each cut list */
			বাবে (; cl_pos < nlists; cl_pos++) {
				বাবে (spos = cut_lists[cl_pos].startpos; spos < linelen;) {
					যদি (!printed[spos]) {
						printed[spos] = 'X';
						putchar(সাৰি[spos]);
					}
					যদি (++spos > cut_lists[cl_pos].endpos) {
						ভঙ্গ;
					}
				}
			}
		} অন্যথা যদি (*delim == '\n') {	/* cut by lines */
			spos = cut_lists[cl_pos].startpos;

			/* get out if we have no more lists to process or if the lines
			 * are lower than what we're interested in */
			যদি (((পূর্ণাঙ্ক)linenum < spos) || (cl_pos >= nlists))
				গচ্ছ next_সাৰি;

			/* if the line we're looking for is lower than the one we were
			 * passed, it means we displayed it already, so move on */
			যেতিয়ালৈ (spos < (পূর্ণাঙ্ক)linenum) {
				spos++;
				/* go to the next list if we're at the end of this one */
				যদি (spos > cut_lists[cl_pos].endpos) {
					cl_pos++;
					/* get out if there's no more lists to process */
					যদি (cl_pos >= nlists)
						গচ্ছ next_সাৰি;
					spos = cut_lists[cl_pos].startpos;
					/* get out if the current line is lower than the one
					 * we just became interested in */
					যদি ((পূর্ণাঙ্ক)linenum < spos)
						গচ্ছ next_সাৰি;
				}
			}

			/* If we made it here, it means we've found the line we're
			 * looking for, so print it */
			puts(সাৰি);
			গচ্ছ next_সাৰি;
		} অন্যথা {		/* cut by fields */
			unsigned uu = 0, start = 0, end = 0, out = 0;
			পূর্ণাঙ্ক dcount = 0;

			/* Loop through bytes, finding next delimiter */
			বাবে (;;) {
				/* End of current range? */
				যদি (end == linelen || dcount > cut_lists[cl_pos].endpos) {
					যদি (++cl_pos >= nlists) ভঙ্গ;
					যদি (option_mask32 & CUT_OPT_NOSORT_FLGS)
						start = dcount = uu = 0;
					end = 0;
				}
				/* End of current line? */
				যদি (uu == linelen) {
					/* If we've seen no delimiters, check -s */
					যদি (!cl_pos && !dcount && !shoe) {
						যদি (option_mask32 & CUT_OPT_SUPPRESS_FLGS)
							গচ্ছ next_সাৰি;
					} অন্যথা যদি (dcount < cut_lists[cl_pos].startpos)
						start = linelen;
					end = linelen;
				} অন্যথা {
					/* Find next delimiter */
					যদি (shoe) {
						regmatch_t rr = {-1, -1};

						যদি (!regexec(&reg, সাৰি+uu, 1, &rr, REG_NOTBOL|REG_NOTEOL)) {
							end = uu + rr.rm_so;
							uu += rr.rm_eo;
						} অন্যথা {
							uu = linelen;
							অনুৱর্ত;
						}
					} অন্যথা যদি (সাৰি[end = uu++] != *delim)
						অনুৱর্ত;

					/* Got delimiter. Loop if not yet within range. */
					যদি (dcount++ < cut_lists[cl_pos].startpos) {
						start = uu;
						অনুৱর্ত;
					}
				}
				যদি (end != start || !shoe)
					printf("%s%.*s", out++ ? odelim : "", end-start, সাৰি + start);
				start = uu;
				যদি (!dcount)
					ভঙ্গ;
			}
		}
		/* if we printed anything, finish with newline */
		putchar('\n');
 next_সাৰি:
		linenum++;
		মুক্ত(printed);
		মুক্ত(orig_সাৰি);
	}
}

পূর্ণাঙ্ক cut_main(পূর্ণাঙ্ক argc, ৱর্ণ **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণাঙ্ক cut_main(পূর্ণাঙ্ক argc UNUSED_PARAM, ৱর্ণ **argv)
{
	/* growable array holding a series of lists */
	সংরচনা cut_list *cut_lists = রিক্ত;
	unsigned nlists = 0;	/* number of elements in above list */
	ৱর্ণ *sopt, *ltok;
	ধ্রুৱ ৱর্ণ *delim = রিক্ত;
	ধ্রুৱ ৱর্ণ *odelim = রিক্ত;
	unsigned opt;

#define ARG "bcf"IF_FEATURE_CUT_REGEX("F")
	opt = getopt32(argv, "^"
			OPT_STR  // = "b:c:f:d:O:sD"IF_FEATURE_CUT_REGEX("F:")"n"
			"\0" "b--"ARG":c--"ARG":f--"ARG IF_FEATURE_CUT_REGEX("F--"ARG),
			&sopt, &sopt, &sopt, &delim, &odelim IF_FEATURE_CUT_REGEX(, &sopt)
	);
	যদি (!delim || !*delim)
		delim = (opt & CUT_OPT_REGEX_FLGS) ? "[[:space:]]+" : "\t";
	যদি (!odelim) odelim = (opt & CUT_OPT_REGEX_FLGS) ? " " : delim;

//	argc -= optind;
	argv += optind;
	যদি (!(opt & (CUT_OPT_BYTE_FLGS | CUT_OPT_CHAR_FLGS | CUT_OPT_FIELDS_FLGS | CUT_OPT_REGEX_FLGS)))
		bb_simple_error_বাৰ্তা_and_die("expected a list of bytes, characters, or fields");

	/*  non-field (char or byte) cutting has some special handling */
	যদি (!(opt & (CUT_OPT_FIELDS_FLGS|CUT_OPT_REGEX_FLGS))) {
		স্থির ধ্রুৱ ৱর্ণ _op_on_field[] ALIGN1 = " only when operating on fields";

		যদি (opt & CUT_OPT_SUPPRESS_FLGS) {
			bb_error_বাৰ্তা_and_die
				("suppressing non-delimited lines makes sense%s", _op_on_field);
		}
		যদি (opt & CUT_OPT_DELIM_FLGS) {
			bb_error_বাৰ্তা_and_die
				("a delimiter may be specified%s", _op_on_field);
		}
	}

	/*
	 * parse list and put values into startpos and endpos.
	 * valid list formats: N, N-, N-M, -M
	 * more than one list can be separated by commas
	 */
	{
		ৱর্ণ *ntok;
		পূর্ণাঙ্ক s = 0, e = 0;

		/* take apart the lists, one by one (they are separated with commas) */
		যেতিয়ালৈ ((ltok = strsep(&sopt, ",")) != রিক্ত) {

			/* it's actually legal to pass an empty list */
			যদি (!ltok[0])
				অনুৱর্ত;

			/* get the start pos */
			ntok = strsep(&ltok, "-");
			যদি (!ntok[0]) {
				s = 0;
			} অন্যথা {
				s = xatoi_positive(ntok);
				/* account for the fact that arrays are zero based, while
				 * the user expects the first char on the line to be char #1 */
				যদি (s != 0)
					s--;
			}

			/* get the end pos */
			যদি (ltok == রিক্ত) {
				e = s;
			} অন্যথা যদি (!ltok[0]) {
				e = INT_MAX;
			} অন্যথা {
				e = xatoi_positive(ltok);
				/* if the user specified and end position of 0,
				 * that means "til the end of the line" */
				যদি (!*ltok)
					e = INT_MAX;
				অন্যথা যদি (e < s)
					bb_error_বাৰ্তা_and_die("%d<%d", e, s);
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
		যদি (nlists == 0)
			bb_simple_error_বাৰ্তা_and_die("missing list of positions");

		/* now that the lists are parsed, we need to sort them to make life
		 * easier on us when it comes time to print the chars / fields / lines
		 */
		যদি (!(opt & CUT_OPT_NOSORT_FLGS))
			qsort(cut_lists, nlists, পরিমাণ(cut_lists[0]), cmpfunc);
	}

	{
		exitcode_t retval = EXIT_SUCCESS;

		যদি (!*argv)
			*--argv = (ৱর্ণ *)"-";

		কৰা {
			FILE *file = fopen_or_warn_stdin(*argv);
			যদি (!file) {
				retval = EXIT_FAILURE;
				অনুৱর্ত;
			}
			cut_file(file, delim, odelim, cut_lists, nlists);
			fclose_if_not_stdin(file);
		} যেতিয়ালৈ (*++argv);

		যদি (ENABLE_FEATURE_CLEAN_UP)
			মুক্ত(cut_lists);
		fflush_stdout_and_exit(retval);
	}
}
