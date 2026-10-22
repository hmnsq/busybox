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

#അന്തര്ഭാവ "libbb.h"

#എങ്കിൽ ENABLE_FEATURE_CUT_REGEX
#അന്തര്ഭാവ "xregex.h"
#അല്ലെങ്കിൽ
#define regex_t പൂർണ്ണസംഖ്യ
തരനിർവചനം ഘടന { പൂർണ്ണസംഖ്യ rm_eo, rm_so; } regmatch_t;
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

ഘടന cut_list {
	പൂർണ്ണസംഖ്യ startpos;
	പൂർണ്ണസംഖ്യ endpos;
};

സ്ഥിതം പൂർണ്ണസംഖ്യ cmpfunc(സ്ഥിരം ശൂന്യം *a, സ്ഥിരം ശൂന്യം *b)
{
	തിരികെ (((ഘടന cut_list *) a)->startpos -
			((ഘടന cut_list *) b)->startpos);
}

സ്ഥിതം ശൂന്യം cut_file(FILE *file, സ്ഥിരം അക്ഷരം *delim, സ്ഥിരം അക്ഷരം *odelim,
		സ്ഥിരം ഘടന cut_list *cut_lists, unsigned nlists)
{
	അക്ഷരം *വരി;
	unsigned linenum = 0;	/* keep these zero-based to be consistent */
	regex_t reg;
	പൂർണ്ണസംഖ്യ spos, shoe = option_mask32 & CUT_OPT_REGEX_FLGS;

	എങ്കിൽ (shoe) xregcomp(&reg, delim, REG_EXTENDED);

	/* go through every line in the file */
	ആകുമ്പോൾ ((വരി = xmalloc_fgetline(file)) != ശൂന്യ) {

		/* set up a list so we can keep track of what's been printed */
		പൂർണ്ണസംഖ്യ linelen = strlen(വരി);
		അക്ഷരം *printed = xzalloc(linelen + 1);
		അക്ഷരം *orig_വരി = വരി;
		unsigned cl_pos = 0;

		/* cut based on chars/bytes XXX: only works when sizeof(char) == byte */
		എങ്കിൽ (option_mask32 & (CUT_OPT_CHAR_FLGS | CUT_OPT_BYTE_FLGS)) {
			/* print the chars specified in each cut list */
			വേണ്ടി (; cl_pos < nlists; cl_pos++) {
				വേണ്ടി (spos = cut_lists[cl_pos].startpos; spos < linelen;) {
					എങ്കിൽ (!printed[spos]) {
						printed[spos] = 'X';
						putchar(വരി[spos]);
					}
					എങ്കിൽ (++spos > cut_lists[cl_pos].endpos) {
						നിർത്തുക;
					}
				}
			}
		} അല്ലെങ്കിൽ എങ്കിൽ (*delim == '\n') {	/* cut by lines */
			spos = cut_lists[cl_pos].startpos;

			/* get out if we have no more lists to process or if the lines
			 * are lower than what we're interested in */
			എങ്കിൽ (((പൂർണ്ണസംഖ്യ)linenum < spos) || (cl_pos >= nlists))
				പോകുക next_വരി;

			/* if the line we're looking for is lower than the one we were
			 * passed, it means we displayed it already, so move on */
			ആകുമ്പോൾ (spos < (പൂർണ്ണസംഖ്യ)linenum) {
				spos++;
				/* go to the next list if we're at the end of this one */
				എങ്കിൽ (spos > cut_lists[cl_pos].endpos) {
					cl_pos++;
					/* get out if there's no more lists to process */
					എങ്കിൽ (cl_pos >= nlists)
						പോകുക next_വരി;
					spos = cut_lists[cl_pos].startpos;
					/* get out if the current line is lower than the one
					 * we just became interested in */
					എങ്കിൽ ((പൂർണ്ണസംഖ്യ)linenum < spos)
						പോകുക next_വരി;
				}
			}

			/* If we made it here, it means we've found the line we're
			 * looking for, so print it */
			puts(വരി);
			പോകുക next_വരി;
		} അല്ലെങ്കിൽ {		/* cut by fields */
			unsigned uu = 0, start = 0, end = 0, out = 0;
			പൂർണ്ണസംഖ്യ dcount = 0;

			/* Loop through bytes, finding next delimiter */
			വേണ്ടി (;;) {
				/* End of current range? */
				എങ്കിൽ (end == linelen || dcount > cut_lists[cl_pos].endpos) {
					എങ്കിൽ (++cl_pos >= nlists) നിർത്തുക;
					എങ്കിൽ (option_mask32 & CUT_OPT_NOSORT_FLGS)
						start = dcount = uu = 0;
					end = 0;
				}
				/* End of current line? */
				എങ്കിൽ (uu == linelen) {
					/* If we've seen no delimiters, check -s */
					എങ്കിൽ (!cl_pos && !dcount && !shoe) {
						എങ്കിൽ (option_mask32 & CUT_OPT_SUPPRESS_FLGS)
							പോകുക next_വരി;
					} അല്ലെങ്കിൽ എങ്കിൽ (dcount < cut_lists[cl_pos].startpos)
						start = linelen;
					end = linelen;
				} അല്ലെങ്കിൽ {
					/* Find next delimiter */
					എങ്കിൽ (shoe) {
						regmatch_t rr = {-1, -1};

						എങ്കിൽ (!regexec(&reg, വരി+uu, 1, &rr, REG_NOTBOL|REG_NOTEOL)) {
							end = uu + rr.rm_so;
							uu += rr.rm_eo;
						} അല്ലെങ്കിൽ {
							uu = linelen;
							തുടരുക;
						}
					} അല്ലെങ്കിൽ എങ്കിൽ (വരി[end = uu++] != *delim)
						തുടരുക;

					/* Got delimiter. Loop if not yet within range. */
					എങ്കിൽ (dcount++ < cut_lists[cl_pos].startpos) {
						start = uu;
						തുടരുക;
					}
				}
				എങ്കിൽ (end != start || !shoe)
					printf("%s%.*s", out++ ? odelim : "", end-start, വരി + start);
				start = uu;
				എങ്കിൽ (!dcount)
					നിർത്തുക;
			}
		}
		/* if we printed anything, finish with newline */
		putchar('\n');
 next_വരി:
		linenum++;
		സ്വതന്ത്രം(printed);
		സ്വതന്ത്രം(orig_വരി);
	}
}

പൂർണ്ണസംഖ്യ cut_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) MAIN_EXTERNALLY_VISIBLE;
പൂർണ്ണസംഖ്യ cut_main(പൂർണ്ണസംഖ്യ argc UNUSED_PARAM, അക്ഷരം **argv)
{
	/* growable array holding a series of lists */
	ഘടന cut_list *cut_lists = ശൂന്യ;
	unsigned nlists = 0;	/* number of elements in above list */
	അക്ഷരം *sopt, *ltok;
	സ്ഥിരം അക്ഷരം *delim = ശൂന്യ;
	സ്ഥിരം അക്ഷരം *odelim = ശൂന്യ;
	unsigned opt;

#define ARG "bcf"IF_FEATURE_CUT_REGEX("F")
	opt = getopt32(argv, "^"
			OPT_STR  // = "b:c:f:d:O:sD"IF_FEATURE_CUT_REGEX("F:")"n"
			"\0" "b--"ARG":c--"ARG":f--"ARG IF_FEATURE_CUT_REGEX("F--"ARG),
			&sopt, &sopt, &sopt, &delim, &odelim IF_FEATURE_CUT_REGEX(, &sopt)
	);
	എങ്കിൽ (!delim || !*delim)
		delim = (opt & CUT_OPT_REGEX_FLGS) ? "[[:space:]]+" : "\t";
	എങ്കിൽ (!odelim) odelim = (opt & CUT_OPT_REGEX_FLGS) ? " " : delim;

//	argc -= optind;
	argv += optind;
	എങ്കിൽ (!(opt & (CUT_OPT_BYTE_FLGS | CUT_OPT_CHAR_FLGS | CUT_OPT_FIELDS_FLGS | CUT_OPT_REGEX_FLGS)))
		bb_simple_error_സന്ദേശം_and_die("expected a list of bytes, characters, or fields");

	/*  non-field (char or byte) cutting has some special handling */
	എങ്കിൽ (!(opt & (CUT_OPT_FIELDS_FLGS|CUT_OPT_REGEX_FLGS))) {
		സ്ഥിതം സ്ഥിരം അക്ഷരം _op_on_field[] ALIGN1 = " only when operating on fields";

		എങ്കിൽ (opt & CUT_OPT_SUPPRESS_FLGS) {
			bb_error_സന്ദേശം_and_die
				("suppressing non-delimited lines makes sense%s", _op_on_field);
		}
		എങ്കിൽ (opt & CUT_OPT_DELIM_FLGS) {
			bb_error_സന്ദേശം_and_die
				("a delimiter may be specified%s", _op_on_field);
		}
	}

	/*
	 * parse list and put values into startpos and endpos.
	 * valid list formats: N, N-, N-M, -M
	 * more than one list can be separated by commas
	 */
	{
		അക്ഷരം *ntok;
		പൂർണ്ണസംഖ്യ s = 0, e = 0;

		/* take apart the lists, one by one (they are separated with commas) */
		ആകുമ്പോൾ ((ltok = strsep(&sopt, ",")) != ശൂന്യ) {

			/* it's actually legal to pass an empty list */
			എങ്കിൽ (!ltok[0])
				തുടരുക;

			/* get the start pos */
			ntok = strsep(&ltok, "-");
			എങ്കിൽ (!ntok[0]) {
				s = 0;
			} അല്ലെങ്കിൽ {
				s = xatoi_positive(ntok);
				/* account for the fact that arrays are zero based, while
				 * the user expects the first char on the line to be char #1 */
				എങ്കിൽ (s != 0)
					s--;
			}

			/* get the end pos */
			എങ്കിൽ (ltok == ശൂന്യ) {
				e = s;
			} അല്ലെങ്കിൽ എങ്കിൽ (!ltok[0]) {
				e = INT_MAX;
			} അല്ലെങ്കിൽ {
				e = xatoi_positive(ltok);
				/* if the user specified and end position of 0,
				 * that means "til the end of the line" */
				എങ്കിൽ (!*ltok)
					e = INT_MAX;
				അല്ലെങ്കിൽ എങ്കിൽ (e < s)
					bb_error_സന്ദേശം_and_die("%d<%d", e, s);
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
		എങ്കിൽ (nlists == 0)
			bb_simple_error_സന്ദേശം_and_die("missing list of positions");

		/* now that the lists are parsed, we need to sort them to make life
		 * easier on us when it comes time to print the chars / fields / lines
		 */
		എങ്കിൽ (!(opt & CUT_OPT_NOSORT_FLGS))
			qsort(cut_lists, nlists, വലുപ്പം(cut_lists[0]), cmpfunc);
	}

	{
		exitcode_t retval = EXIT_SUCCESS;

		എങ്കിൽ (!*argv)
			*--argv = (അക്ഷരം *)"-";

		ചെയ്യുക {
			FILE *file = fopen_or_warn_stdin(*argv);
			എങ്കിൽ (!file) {
				retval = EXIT_FAILURE;
				തുടരുക;
			}
			cut_file(file, delim, odelim, cut_lists, nlists);
			fclose_if_not_stdin(file);
		} ആകുമ്പോൾ (*++argv);

		എങ്കിൽ (ENABLE_FEATURE_CLEAN_UP)
			സ്വതന്ത്രം(cut_lists);
		fflush_stdout_and_exit(retval);
	}
}
