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

#जर ENABLE_FEATURE_CUT_REGEX
#अन्तर्भाव "xregex.h"
#अन्यथा
#define regex_t पूर्णांक
प्रकारव्याख्या संरचना { पूर्णांक rm_eo, rm_so; } regmatch_t;
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
	पूर्णांक startpos;
	पूर्णांक endpos;
};

स्थिर पूर्णांक cmpfunc(स्थिरांक शून्य *a, स्थिरांक शून्य *b)
{
	परत (((संरचना cut_list *) a)->startpos -
			((संरचना cut_list *) b)->startpos);
}

स्थिर शून्य cut_file(खाता *file, स्थिरांक अक्षर *delim, स्थिरांक अक्षर *odelim,
		स्थिरांक संरचना cut_list *cut_lists, अचिन्हित nlists)
{
	अक्षर *ओळ;
	अचिन्हित linenum = 0;	/* keep these zero-based to be consistent */
	regex_t reg;
	पूर्णांक spos, shoe = option_mask32 & CUT_OPT_REGEX_FLGS;

	जर (shoe) xregcomp(&reg, delim, REG_EXTENDED);

	/* go through every line in the file */
	पर्यंत ((ओळ = xmalloc_fgetline(file)) != रिक्त) {

		/* set up a list so we can keep track of what's been printed */
		पूर्णांक linelen = म_माप(ओळ);
		अक्षर *printed = xzalloc(linelen + 1);
		अक्षर *orig_ओळ = ओळ;
		अचिन्हित cl_pos = 0;

		/* cut based on chars/bytes XXX: only works when sizeof(char) == byte */
		जर (option_mask32 & (CUT_OPT_CHAR_FLGS | CUT_OPT_BYTE_FLGS)) {
			/* print the chars specified in each cut list */
			साठी (; cl_pos < nlists; cl_pos++) {
				साठी (spos = cut_lists[cl_pos].startpos; spos < linelen;) {
					जर (!printed[spos]) {
						printed[spos] = 'X';
						अक्षर_दो(ओळ[spos]);
					}
					जर (++spos > cut_lists[cl_pos].endpos) {
						थांबा;
					}
				}
			}
		} अन्यथा जर (*delim == '\n') {	/* cut by lines */
			spos = cut_lists[cl_pos].startpos;

			/* get out if we have no more lists to process or if the lines
			 * are lower than what we're interested in */
			जर (((पूर्णांक)linenum < spos) || (cl_pos >= nlists))
				जा next_ओळ;

			/* if the line we're looking for is lower than the one we were
			 * passed, it means we displayed it already, so move on */
			पर्यंत (spos < (पूर्णांक)linenum) {
				spos++;
				/* go to the next list if we're at the end of this one */
				जर (spos > cut_lists[cl_pos].endpos) {
					cl_pos++;
					/* get out if there's no more lists to process */
					जर (cl_pos >= nlists)
						जा next_ओळ;
					spos = cut_lists[cl_pos].startpos;
					/* get out if the current line is lower than the one
					 * we just became interested in */
					जर ((पूर्णांक)linenum < spos)
						जा next_ओळ;
				}
			}

			/* If we made it here, it means we've found the line we're
			 * looking for, so print it */
			माला_दो(ओळ);
			जा next_ओळ;
		} अन्यथा {		/* cut by fields */
			अचिन्हित uu = 0, start = 0, end = 0, out = 0;
			पूर्णांक dcount = 0;

			/* Loop through bytes, finding next delimiter */
			साठी (;;) {
				/* End of current range? */
				जर (end == linelen || dcount > cut_lists[cl_pos].endpos) {
					जर (++cl_pos >= nlists) थांबा;
					जर (option_mask32 & CUT_OPT_NOSORT_FLGS)
						start = dcount = uu = 0;
					end = 0;
				}
				/* End of current line? */
				जर (uu == linelen) {
					/* If we've seen no delimiters, check -s */
					जर (!cl_pos && !dcount && !shoe) {
						जर (option_mask32 & CUT_OPT_SUPPRESS_FLGS)
							जा next_ओळ;
					} अन्यथा जर (dcount < cut_lists[cl_pos].startpos)
						start = linelen;
					end = linelen;
				} अन्यथा {
					/* Find next delimiter */
					जर (shoe) {
						regmatch_t rr = {-1, -1};

						जर (!regexec(&reg, ओळ+uu, 1, &rr, REG_NOTBOL|REG_NOTEOL)) {
							end = uu + rr.rm_so;
							uu += rr.rm_eo;
						} अन्यथा {
							uu = linelen;
							सुरूठेवा;
						}
					} अन्यथा जर (ओळ[end = uu++] != *delim)
						सुरूठेवा;

					/* Got delimiter. Loop if not yet within range. */
					जर (dcount++ < cut_lists[cl_pos].startpos) {
						start = uu;
						सुरूठेवा;
					}
				}
				जर (end != start || !shoe)
					म_लिखो("%s%.*s", out++ ? odelim : "", end-start, ओळ + start);
				start = uu;
				जर (!dcount)
					थांबा;
			}
		}
		/* if we printed anything, finish with newline */
		अक्षर_दो('\n');
 next_ओळ:
		linenum++;
		मुक्त(printed);
		मुक्त(orig_ओळ);
	}
}

पूर्णांक cut_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक cut_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	/* growable array holding a series of lists */
	संरचना cut_list *cut_lists = रिक्त;
	अचिन्हित nlists = 0;	/* number of elements in above list */
	अक्षर *sopt, *ltok;
	स्थिरांक अक्षर *delim = रिक्त;
	स्थिरांक अक्षर *odelim = रिक्त;
	अचिन्हित opt;

#define ARG "bcf"IF_FEATURE_CUT_REGEX("F")
	opt = getopt32(argv, "^"
			OPT_STR  // = "b:c:f:d:O:sD"IF_FEATURE_CUT_REGEX("F:")"n"
			"\0" "b--"ARG":c--"ARG":f--"ARG IF_FEATURE_CUT_REGEX("F--"ARG),
			&sopt, &sopt, &sopt, &delim, &odelim IF_FEATURE_CUT_REGEX(, &sopt)
	);
	जर (!delim || !*delim)
		delim = (opt & CUT_OPT_REGEX_FLGS) ? "[[:space:]]+" : "\t";
	जर (!odelim) odelim = (opt & CUT_OPT_REGEX_FLGS) ? " " : delim;

//	argc -= optind;
	argv += optind;
	जर (!(opt & (CUT_OPT_BYTE_FLGS | CUT_OPT_CHAR_FLGS | CUT_OPT_FIELDS_FLGS | CUT_OPT_REGEX_FLGS)))
		bb_simple_error_संदेश_and_die("expected a list of bytes, characters, or fields");

	/*  non-field (char or byte) cutting has some special handling */
	जर (!(opt & (CUT_OPT_FIELDS_FLGS|CUT_OPT_REGEX_FLGS))) {
		स्थिर स्थिरांक अक्षर _op_on_field[] ALIGN1 = " only when operating on fields";

		जर (opt & CUT_OPT_SUPPRESS_FLGS) {
			bb_error_संदेश_and_die
				("suppressing non-delimited lines makes sense%s", _op_on_field);
		}
		जर (opt & CUT_OPT_DELIM_FLGS) {
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
		अक्षर *ntok;
		पूर्णांक s = 0, e = 0;

		/* take apart the lists, one by one (they are separated with commas) */
		पर्यंत ((ltok = strsep(&sopt, ",")) != रिक्त) {

			/* it's actually legal to pass an empty list */
			जर (!ltok[0])
				सुरूठेवा;

			/* get the start pos */
			ntok = strsep(&ltok, "-");
			जर (!ntok[0]) {
				s = 0;
			} अन्यथा {
				s = xatoi_positive(ntok);
				/* account for the fact that arrays are zero based, while
				 * the user expects the first char on the line to be char #1 */
				जर (s != 0)
					s--;
			}

			/* get the end pos */
			जर (ltok == रिक्त) {
				e = s;
			} अन्यथा जर (!ltok[0]) {
				e = पूर्णांक_उच्च;
			} अन्यथा {
				e = xatoi_positive(ltok);
				/* if the user specified and end position of 0,
				 * that means "til the end of the line" */
				जर (!*ltok)
					e = पूर्णांक_उच्च;
				अन्यथा जर (e < s)
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
		जर (nlists == 0)
			bb_simple_error_संदेश_and_die("missing list of positions");

		/* now that the lists are parsed, we need to sort them to make life
		 * easier on us when it comes time to print the chars / fields / lines
		 */
		जर (!(opt & CUT_OPT_NOSORT_FLGS))
			क्विक(cut_lists, nlists, आकार(cut_lists[0]), cmpfunc);
	}

	{
		exitcode_t retval = निकास_सफल;

		जर (!*argv)
			*--argv = (अक्षर *)"-";

		करा {
			खाता *file = fopen_or_warn_stdin(*argv);
			जर (!file) {
				retval = निकास_त्रुटि;
				सुरूठेवा;
			}
			cut_file(file, delim, odelim, cut_lists, nlists);
			fclose_if_not_stdin(file);
		} पर्यंत (*++argv);

		जर (ENABLE_FEATURE_CLEAN_UP)
			मुक्त(cut_lists);
		fflush_stdout_and_exit(retval);
	}
}
