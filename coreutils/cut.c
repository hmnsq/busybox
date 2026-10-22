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

#समावेश "libbb.h"

#अगर ENABLE_FEATURE_CUT_REGEX
#समावेश "xregex.h"
#अन्यथा
#घोषणा regex_t पूर्णांक
प्रकार काष्ठा { पूर्णांक rm_eo, rm_so; } regmatch_t;
#घोषणा xregcomp(x, ...) *(x) = 0
#घोषणा regexec(...)     0
#पूर्ण_अगर

/* This is a NOEXEC applet. Be very careful! */


/* option vars */
#घोषणा OPT_STR "b:c:f:d:O:sD"IF_FEATURE_CUT_REGEX("F:")"n"
#घोषणा CUT_OPT_BYTE_FLGS     (1 << 0)
#घोषणा CUT_OPT_CHAR_FLGS     (1 << 1)
#घोषणा CUT_OPT_FIELDS_FLGS   (1 << 2)
#घोषणा CUT_OPT_DELIM_FLGS    (1 << 3)
#घोषणा CUT_OPT_ODELIM_FLGS   (1 << 4)
#घोषणा CUT_OPT_SUPPRESS_FLGS (1 << 5)
#घोषणा CUT_OPT_NOSORT_FLGS   (1 << 6)
#घोषणा CUT_OPT_REGEX_FLGS    ((1 << 7) * ENABLE_FEATURE_CUT_REGEX)

काष्ठा cut_list {
	पूर्णांक startpos;
	पूर्णांक endpos;
};

अटल पूर्णांक cmpfunc(स्थिर व्योम *a, स्थिर व्योम *b)
{
	वापस (((काष्ठा cut_list *) a)->startpos -
			((काष्ठा cut_list *) b)->startpos);
}

अटल व्योम cut_file(खाता *file, स्थिर अक्षर *delim, स्थिर अक्षर *odelim,
		स्थिर काष्ठा cut_list *cut_lists, अचिन्हित nlists)
{
	अक्षर *पंक्ति;
	अचिन्हित linenum = 0;	/* keep these zero-based to be consistent */
	regex_t reg;
	पूर्णांक spos, shoe = option_mask32 & CUT_OPT_REGEX_FLGS;

	अगर (shoe) xregcomp(&reg, delim, REG_EXTENDED);

	/* go through every line in the file */
	जबतक ((पंक्ति = xmalloc_fgetline(file)) != शून्य) {

		/* set up a list so we can keep track of what's been printed */
		पूर्णांक linelen = म_माप(पंक्ति);
		अक्षर *printed = xzalloc(linelen + 1);
		अक्षर *orig_पंक्ति = पंक्ति;
		अचिन्हित cl_pos = 0;

		/* cut based on chars/bytes XXX: only works when sizeof(char) == byte */
		अगर (option_mask32 & (CUT_OPT_CHAR_FLGS | CUT_OPT_BYTE_FLGS)) {
			/* print the chars specified in each cut list */
			क्रम (; cl_pos < nlists; cl_pos++) {
				क्रम (spos = cut_lists[cl_pos].startpos; spos < linelen;) {
					अगर (!printed[spos]) {
						printed[spos] = 'X';
						अक्षर_दो(पंक्ति[spos]);
					}
					अगर (++spos > cut_lists[cl_pos].endpos) {
						अवरोध;
					}
				}
			}
		} अन्यथा अगर (*delim == '\n') {	/* cut by lines */
			spos = cut_lists[cl_pos].startpos;

			/* get out if we have no more lists to process or if the lines
			 * are lower than what we're interested in */
			अगर (((पूर्णांक)linenum < spos) || (cl_pos >= nlists))
				जाओ अग्र_पंक्ति;

			/* if the line we're looking for is lower than the one we were
			 * passed, it means we displayed it already, so move on */
			जबतक (spos < (पूर्णांक)linenum) {
				spos++;
				/* go to the next list if we're at the end of this one */
				अगर (spos > cut_lists[cl_pos].endpos) {
					cl_pos++;
					/* get out if there's no more lists to process */
					अगर (cl_pos >= nlists)
						जाओ अग्र_पंक्ति;
					spos = cut_lists[cl_pos].startpos;
					/* get out if the current line is lower than the one
					 * we just became interested in */
					अगर ((पूर्णांक)linenum < spos)
						जाओ अग्र_पंक्ति;
				}
			}

			/* If we made it here, it means we've found the line we're
			 * looking for, so print it */
			माला_दो(पंक्ति);
			जाओ अग्र_पंक्ति;
		} अन्यथा {		/* cut by fields */
			अचिन्हित uu = 0, start = 0, end = 0, out = 0;
			पूर्णांक dcount = 0;

			/* Loop through bytes, finding next delimiter */
			क्रम (;;) {
				/* End of current range? */
				अगर (end == linelen || dcount > cut_lists[cl_pos].endpos) {
					अगर (++cl_pos >= nlists) अवरोध;
					अगर (option_mask32 & CUT_OPT_NOSORT_FLGS)
						start = dcount = uu = 0;
					end = 0;
				}
				/* End of current line? */
				अगर (uu == linelen) {
					/* If we've seen no delimiters, check -s */
					अगर (!cl_pos && !dcount && !shoe) {
						अगर (option_mask32 & CUT_OPT_SUPPRESS_FLGS)
							जाओ अग्र_पंक्ति;
					} अन्यथा अगर (dcount < cut_lists[cl_pos].startpos)
						start = linelen;
					end = linelen;
				} अन्यथा {
					/* Find next delimiter */
					अगर (shoe) {
						regmatch_t rr = {-1, -1};

						अगर (!regexec(&reg, पंक्ति+uu, 1, &rr, REG_NOTBOL|REG_NOTEOL)) {
							end = uu + rr.rm_so;
							uu += rr.rm_eo;
						} अन्यथा {
							uu = linelen;
							जारी;
						}
					} अन्यथा अगर (पंक्ति[end = uu++] != *delim)
						जारी;

					/* Got delimiter. Loop if not yet within range. */
					अगर (dcount++ < cut_lists[cl_pos].startpos) {
						start = uu;
						जारी;
					}
				}
				अगर (end != start || !shoe)
					म_लिखो("%s%.*s", out++ ? odelim : "", end-start, पंक्ति + start);
				start = uu;
				अगर (!dcount)
					अवरोध;
			}
		}
		/* if we printed anything, finish with newline */
		अक्षर_दो('\n');
 अग्र_पंक्ति:
		linenum++;
		मुक्त(printed);
		मुक्त(orig_पंक्ति);
	}
}

पूर्णांक cut_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक cut_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	/* growable array holding a series of lists */
	काष्ठा cut_list *cut_lists = शून्य;
	अचिन्हित nlists = 0;	/* number of elements in above list */
	अक्षर *sopt, *ltok;
	स्थिर अक्षर *delim = शून्य;
	स्थिर अक्षर *odelim = शून्य;
	अचिन्हित opt;

#घोषणा ARG "bcf"IF_FEATURE_CUT_REGEX("F")
	opt = getopt32(argv, "^"
			OPT_STR  // = "b:c:f:d:O:sD"IF_FEATURE_CUT_REGEX("F:")"n"
			"\0" "b--"ARG":c--"ARG":f--"ARG IF_FEATURE_CUT_REGEX("F--"ARG),
			&sopt, &sopt, &sopt, &delim, &odelim IF_FEATURE_CUT_REGEX(, &sopt)
	);
	अगर (!delim || !*delim)
		delim = (opt & CUT_OPT_REGEX_FLGS) ? "[[:space:]]+" : "\t";
	अगर (!odelim) odelim = (opt & CUT_OPT_REGEX_FLGS) ? " " : delim;

//	argc -= optind;
	argv += optind;
	अगर (!(opt & (CUT_OPT_BYTE_FLGS | CUT_OPT_CHAR_FLGS | CUT_OPT_FIELDS_FLGS | CUT_OPT_REGEX_FLGS)))
		bb_simple_error_संदेश_and_die("expected a list of bytes, characters, or fields");

	/*  non-field (char or byte) cutting has some special handling */
	अगर (!(opt & (CUT_OPT_FIELDS_FLGS|CUT_OPT_REGEX_FLGS))) {
		अटल स्थिर अक्षर _op_on_field[] ALIGN1 = " only when operating on fields";

		अगर (opt & CUT_OPT_SUPPRESS_FLGS) {
			bb_error_संदेश_and_die
				("suppressing non-delimited lines makes sense%s", _op_on_field);
		}
		अगर (opt & CUT_OPT_DELIM_FLGS) {
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
		जबतक ((ltok = strsep(&sopt, ",")) != शून्य) {

			/* it's actually legal to pass an empty list */
			अगर (!ltok[0])
				जारी;

			/* get the start pos */
			ntok = strsep(&ltok, "-");
			अगर (!ntok[0]) {
				s = 0;
			} अन्यथा {
				s = xatoi_positive(ntok);
				/* account for the fact that arrays are zero based, while
				 * the user expects the first char on the line to be char #1 */
				अगर (s != 0)
					s--;
			}

			/* get the end pos */
			अगर (ltok == शून्य) {
				e = s;
			} अन्यथा अगर (!ltok[0]) {
				e = पूर्णांक_उच्च;
			} अन्यथा {
				e = xatoi_positive(ltok);
				/* if the user specified and end position of 0,
				 * that means "til the end of the line" */
				अगर (!*ltok)
					e = पूर्णांक_उच्च;
				अन्यथा अगर (e < s)
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
		अगर (nlists == 0)
			bb_simple_error_संदेश_and_die("missing list of positions");

		/* now that the lists are parsed, we need to sort them to make life
		 * easier on us when it comes time to print the chars / fields / lines
		 */
		अगर (!(opt & CUT_OPT_NOSORT_FLGS))
			क्विक(cut_lists, nlists, माप(cut_lists[0]), cmpfunc);
	}

	{
		exitcode_t retval = निकास_सफल;

		अगर (!*argv)
			*--argv = (अक्षर *)"-";

		करो {
			खाता *file = fopen_or_warn_stdin(*argv);
			अगर (!file) {
				retval = निकास_त्रुटि;
				जारी;
			}
			cut_file(file, delim, odelim, cut_lists, nlists);
			fclose_if_not_stdin(file);
		} जबतक (*++argv);

		अगर (ENABLE_FEATURE_CLEAN_UP)
			मुक्त(cut_lists);
		fflush_stdout_and_exit(retval);
	}
}
