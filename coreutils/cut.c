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

#अन्तर्भाव "libbb.h"

#यदि ENABLE_FEATURE_CUT_REGEX
#अन्तर्भाव "xregex.h"
#अन्यथा
#define regex_t पूर्णाङ्क
प्रकारनिर्देश संरचना { पूर्णाङ्क rm_eo, rm_so; } regmatch_t;
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

संरचना cut_list {
	पूर्णाङ्क startpos;
	पूर्णाङ्क endpos;
};

स्थिर पूर्णाङ्क cmpfunc(ध्रुव शून्य *a, ध्रुव शून्य *b)
{
	प्रत्यागम (((संरचना cut_list *) a)->startpos -
			((संरचना cut_list *) b)->startpos);
}

स्थिर शून्य cut_file(खाता *file, ध्रुव वर्ण *delim, ध्रुव वर्ण *odelim,
		ध्रुव संरचना cut_list *cut_lists, अचिन्हित nlists)
{
	वर्ण *पंक्ति;
	अचिन्हित linenum = 0;	/* keep these zero-based to be consistent */
	regex_t reg;
	पूर्णाङ्क spos, shoe = option_mask32 & CUT_OPT_REGEX_FLGS;

	यदि (shoe) xregcomp(&reg, delim, REG_EXTENDED);

	/* go through every line in the file */
	यावत् ((पंक्ति = xmalloc_fgetline(file)) != रिक्त) {

		/* set up a list so we can keep track of what's been printed */
		पूर्णाङ्क linelen = म_माप(पंक्ति);
		वर्ण *printed = xzalloc(linelen + 1);
		वर्ण *orig_पंक्ति = पंक्ति;
		अचिन्हित cl_pos = 0;

		/* cut based on chars/bytes XXX: only works when sizeof(char) == byte */
		यदि (option_mask32 & (CUT_OPT_CHAR_FLGS | CUT_OPT_BYTE_FLGS)) {
			/* print the chars specified in each cut list */
			प्रति (; cl_pos < nlists; cl_pos++) {
				प्रति (spos = cut_lists[cl_pos].startpos; spos < linelen;) {
					यदि (!printed[spos]) {
						printed[spos] = 'X';
						अक्षर_दो(पंक्ति[spos]);
					}
					यदि (++spos > cut_lists[cl_pos].endpos) {
						भङ्ग;
					}
				}
			}
		} अन्यथा यदि (*delim == '\n') {	/* cut by lines */
			spos = cut_lists[cl_pos].startpos;

			/* get out if we have no more lists to process or if the lines
			 * are lower than what we're interested in */
			यदि (((पूर्णाङ्क)linenum < spos) || (cl_pos >= nlists))
				गच्छ next_पंक्ति;

			/* if the line we're looking for is lower than the one we were
			 * passed, it means we displayed it already, so move on */
			यावत् (spos < (पूर्णाङ्क)linenum) {
				spos++;
				/* go to the next list if we're at the end of this one */
				यदि (spos > cut_lists[cl_pos].endpos) {
					cl_pos++;
					/* get out if there's no more lists to process */
					यदि (cl_pos >= nlists)
						गच्छ next_पंक्ति;
					spos = cut_lists[cl_pos].startpos;
					/* get out if the current line is lower than the one
					 * we just became interested in */
					यदि ((पूर्णाङ्क)linenum < spos)
						गच्छ next_पंक्ति;
				}
			}

			/* If we made it here, it means we've found the line we're
			 * looking for, so print it */
			माला_दो(पंक्ति);
			गच्छ next_पंक्ति;
		} अन्यथा {		/* cut by fields */
			अचिन्हित uu = 0, start = 0, end = 0, out = 0;
			पूर्णाङ्क dcount = 0;

			/* Loop through bytes, finding next delimiter */
			प्रति (;;) {
				/* End of current range? */
				यदि (end == linelen || dcount > cut_lists[cl_pos].endpos) {
					यदि (++cl_pos >= nlists) भङ्ग;
					यदि (option_mask32 & CUT_OPT_NOSORT_FLGS)
						start = dcount = uu = 0;
					end = 0;
				}
				/* End of current line? */
				यदि (uu == linelen) {
					/* If we've seen no delimiters, check -s */
					यदि (!cl_pos && !dcount && !shoe) {
						यदि (option_mask32 & CUT_OPT_SUPPRESS_FLGS)
							गच्छ next_पंक्ति;
					} अन्यथा यदि (dcount < cut_lists[cl_pos].startpos)
						start = linelen;
					end = linelen;
				} अन्यथा {
					/* Find next delimiter */
					यदि (shoe) {
						regmatch_t rr = {-1, -1};

						यदि (!regexec(&reg, पंक्ति+uu, 1, &rr, REG_NOTBOL|REG_NOTEOL)) {
							end = uu + rr.rm_so;
							uu += rr.rm_eo;
						} अन्यथा {
							uu = linelen;
							अनुवर्त;
						}
					} अन्यथा यदि (पंक्ति[end = uu++] != *delim)
						अनुवर्त;

					/* Got delimiter. Loop if not yet within range. */
					यदि (dcount++ < cut_lists[cl_pos].startpos) {
						start = uu;
						अनुवर्त;
					}
				}
				यदि (end != start || !shoe)
					म_लिखो("%s%.*s", out++ ? odelim : "", end-start, पंक्ति + start);
				start = uu;
				यदि (!dcount)
					भङ्ग;
			}
		}
		/* if we printed anything, finish with newline */
		अक्षर_दो('\n');
 next_पंक्ति:
		linenum++;
		मुक्त(printed);
		मुक्त(orig_पंक्ति);
	}
}

पूर्णाङ्क cut_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क cut_main(पूर्णाङ्क argc UNUSED_PARAM, वर्ण **argv)
{
	/* growable array holding a series of lists */
	संरचना cut_list *cut_lists = रिक्त;
	अचिन्हित nlists = 0;	/* number of elements in above list */
	वर्ण *sopt, *ltok;
	ध्रुव वर्ण *delim = रिक्त;
	ध्रुव वर्ण *odelim = रिक्त;
	अचिन्हित opt;

#define ARG "bcf"IF_FEATURE_CUT_REGEX("F")
	opt = getopt32(argv, "^"
			OPT_STR  // = "b:c:f:d:O:sD"IF_FEATURE_CUT_REGEX("F:")"n"
			"\0" "b--"ARG":c--"ARG":f--"ARG IF_FEATURE_CUT_REGEX("F--"ARG),
			&sopt, &sopt, &sopt, &delim, &odelim IF_FEATURE_CUT_REGEX(, &sopt)
	);
	यदि (!delim || !*delim)
		delim = (opt & CUT_OPT_REGEX_FLGS) ? "[[:space:]]+" : "\t";
	यदि (!odelim) odelim = (opt & CUT_OPT_REGEX_FLGS) ? " " : delim;

//	argc -= optind;
	argv += optind;
	यदि (!(opt & (CUT_OPT_BYTE_FLGS | CUT_OPT_CHAR_FLGS | CUT_OPT_FIELDS_FLGS | CUT_OPT_REGEX_FLGS)))
		bb_simple_error_संदेश_and_die("expected a list of bytes, characters, or fields");

	/*  non-field (char or byte) cutting has some special handling */
	यदि (!(opt & (CUT_OPT_FIELDS_FLGS|CUT_OPT_REGEX_FLGS))) {
		स्थिर ध्रुव वर्ण _op_on_field[] ALIGN1 = " only when operating on fields";

		यदि (opt & CUT_OPT_SUPPRESS_FLGS) {
			bb_error_संदेश_and_die
				("suppressing non-delimited lines makes sense%s", _op_on_field);
		}
		यदि (opt & CUT_OPT_DELIM_FLGS) {
			bb_error_संदेश_and_die
				("a delimiter may be specified%s", _op_on_field);
		}
	}

	/*
	 * parse list and put values into startpos and endpos.
	 * valid list formats: N, N-, N-M, -M
	 * more than one list can be separated by commas
	 */
	{
		वर्ण *ntok;
		पूर्णाङ्क s = 0, e = 0;

		/* take apart the lists, one by one (they are separated with commas) */
		यावत् ((ltok = strsep(&sopt, ",")) != रिक्त) {

			/* it's actually legal to pass an empty list */
			यदि (!ltok[0])
				अनुवर्त;

			/* get the start pos */
			ntok = strsep(&ltok, "-");
			यदि (!ntok[0]) {
				s = 0;
			} अन्यथा {
				s = xatoi_positive(ntok);
				/* account for the fact that arrays are zero based, while
				 * the user expects the first char on the line to be char #1 */
				यदि (s != 0)
					s--;
			}

			/* get the end pos */
			यदि (ltok == रिक्त) {
				e = s;
			} अन्यथा यदि (!ltok[0]) {
				e = पूर्णांक_उच्च;
			} अन्यथा {
				e = xatoi_positive(ltok);
				/* if the user specified and end position of 0,
				 * that means "til the end of the line" */
				यदि (!*ltok)
					e = पूर्णांक_उच्च;
				अन्यथा यदि (e < s)
					bb_error_संदेश_and_die("%d<%d", e, s);
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
		यदि (nlists == 0)
			bb_simple_error_संदेश_and_die("missing list of positions");

		/* now that the lists are parsed, we need to sort them to make life
		 * easier on us when it comes time to print the chars / fields / lines
		 */
		यदि (!(opt & CUT_OPT_NOSORT_FLGS))
			क्विक(cut_lists, nlists, परिमाण(cut_lists[0]), cmpfunc);
	}

	{
		exitcode_t retval = निकास_सफल;

		यदि (!*argv)
			*--argv = (वर्ण *)"-";

		कुरु {
			खाता *file = fopen_or_warn_stdin(*argv);
			यदि (!file) {
				retval = निकास_त्रुटि;
				अनुवर्त;
			}
			cut_file(file, delim, odelim, cut_lists, nlists);
			fclose_if_not_stdin(file);
		} यावत् (*++argv);

		यदि (ENABLE_FEATURE_CLEAN_UP)
			मुक्त(cut_lists);
		fflush_stdout_and_exit(retval);
	}
}
