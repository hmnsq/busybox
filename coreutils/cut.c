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

#include "libbb.h"

#ပေႃး ENABLE_FEATURE_CUT_REGEX
#include "xregex.h"
#ပေႃးဢမ်ႇ
#define regex_t puur_naa_nka
typedef sa_nracanaa { puur_naa_nka rm_eo, rm_so; } regmatch_t;
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

sa_nracanaa cut_list {
	puur_naa_nka startpos;
	puur_naa_nka endpos;
};

static puur_naa_nka cmpfunc(const void *a, const void *b)
{
	ပွၵ်ႈ (((sa_nracanaa cut_list *) a)->startpos -
			((sa_nracanaa cut_list *) b)->startpos);
}

static void cut_file(FILE *file, const var_na *delim, const var_na *odelim,
		const sa_nracanaa cut_list *cut_lists, unsigned nlists)
{
	var_na *line;
	unsigned linenum = 0;	/* keep these zero-based to be consistent */
	regex_t reg;
	puur_naa_nka spos, shoe = option_mask32 & CUT_OPT_REGEX_FLGS;

	ပေႃး (shoe) xregcomp(&reg, delim, REG_EXTENDED);

	/* go through every line in the file */
	ၶၢဝ်း ((line = xmalloc_fgetline(file)) != NULL) {

		/* set up a list so we can keep track of what's been printed */
		puur_naa_nka linelen = strlen(line);
		var_na *printed = xzalloc(linelen + 1);
		var_na *orig_line = line;
		unsigned cl_pos = 0;

		/* cut based on chars/bytes XXX: only works when sizeof(char) == byte */
		ပေႃး (option_mask32 & (CUT_OPT_CHAR_FLGS | CUT_OPT_BYTE_FLGS)) {
			/* print the chars specified in each cut list */
			for (; cl_pos < nlists; cl_pos++) {
				for (spos = cut_lists[cl_pos].startpos; spos < linelen;) {
					ပေႃး (!printed[spos]) {
						printed[spos] = 'X';
						putchar(line[spos]);
					}
					ပေႃး (++spos > cut_lists[cl_pos].endpos) {
						bha_nga;
					}
				}
			}
		} ပေႃးဢမ်ႇ ပေႃး (*delim == '\n') {	/* cut by lines */
			spos = cut_lists[cl_pos].startpos;

			/* get out if we have no more lists to process or if the lines
			 * are lower than what we're interested in */
			ပေႃး (((puur_naa_nka)linenum < spos) || (cl_pos >= nlists))
				goto next_line;

			/* if the line we're looking for is lower than the one we were
			 * passed, it means we displayed it already, so move on */
			ၶၢဝ်း (spos < (puur_naa_nka)linenum) {
				spos++;
				/* go to the next list if we're at the end of this one */
				ပေႃး (spos > cut_lists[cl_pos].endpos) {
					cl_pos++;
					/* get out if there's no more lists to process */
					ပေႃး (cl_pos >= nlists)
						goto next_line;
					spos = cut_lists[cl_pos].startpos;
					/* get out if the current line is lower than the one
					 * we just became interested in */
					ပေႃး ((puur_naa_nka)linenum < spos)
						goto next_line;
				}
			}

			/* If we made it here, it means we've found the line we're
			 * looking for, so print it */
			puts(line);
			goto next_line;
		} ပေႃးဢမ်ႇ {		/* cut by fields */
			unsigned uu = 0, start = 0, end = 0, out = 0;
			puur_naa_nka dcount = 0;

			/* Loop through bytes, finding next delimiter */
			for (;;) {
				/* End of current range? */
				ပေႃး (end == linelen || dcount > cut_lists[cl_pos].endpos) {
					ပေႃး (++cl_pos >= nlists) bha_nga;
					ပေႃး (option_mask32 & CUT_OPT_NOSORT_FLGS)
						start = dcount = uu = 0;
					end = 0;
				}
				/* End of current line? */
				ပေႃး (uu == linelen) {
					/* If we've seen no delimiters, check -s */
					ပေႃး (!cl_pos && !dcount && !shoe) {
						ပေႃး (option_mask32 & CUT_OPT_SUPPRESS_FLGS)
							goto next_line;
					} ပေႃးဢမ်ႇ ပေႃး (dcount < cut_lists[cl_pos].startpos)
						start = linelen;
					end = linelen;
				} ပေႃးဢမ်ႇ {
					/* Find next delimiter */
					ပေႃး (shoe) {
						regmatch_t rr = {-1, -1};

						ပေႃး (!regexec(&reg, line+uu, 1, &rr, REG_NOTBOL|REG_NOTEOL)) {
							end = uu + rr.rm_so;
							uu += rr.rm_eo;
						} ပေႃးဢမ်ႇ {
							uu = linelen;
							continue;
						}
					} ပေႃးဢမ်ႇ ပေႃး (line[end = uu++] != *delim)
						continue;

					/* Got delimiter. Loop if not yet within range. */
					ပေႃး (dcount++ < cut_lists[cl_pos].startpos) {
						start = uu;
						continue;
					}
				}
				ပေႃး (end != start || !shoe)
					printf("%s%.*s", out++ ? odelim : "", end-start, line + start);
				start = uu;
				ပေႃး (!dcount)
					bha_nga;
			}
		}
		/* if we printed anything, finish with newline */
		putchar('\n');
 next_line:
		linenum++;
		free(printed);
		free(orig_line);
	}
}

puur_naa_nka cut_main(puur_naa_nka argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
puur_naa_nka cut_main(puur_naa_nka argc UNUSED_PARAM, var_na **argv)
{
	/* growable array holding a series of lists */
	sa_nracanaa cut_list *cut_lists = NULL;
	unsigned nlists = 0;	/* number of elements in above list */
	var_na *sopt, *ltok;
	const var_na *delim = NULL;
	const var_na *odelim = NULL;
	unsigned opt;

#define ARG "bcf"IF_FEATURE_CUT_REGEX("F")
	opt = getopt32(argv, "^"
			OPT_STR  // = "b:c:f:d:O:sD"IF_FEATURE_CUT_REGEX("F:")"n"
			"\0" "b--"ARG":c--"ARG":f--"ARG IF_FEATURE_CUT_REGEX("F--"ARG),
			&sopt, &sopt, &sopt, &delim, &odelim IF_FEATURE_CUT_REGEX(, &sopt)
	);
	ပေႃး (!delim || !*delim)
		delim = (opt & CUT_OPT_REGEX_FLGS) ? "[[:space:]]+" : "\t";
	ပေႃး (!odelim) odelim = (opt & CUT_OPT_REGEX_FLGS) ? " " : delim;

//	argc -= optind;
	argv += optind;
	ပေႃး (!(opt & (CUT_OPT_BYTE_FLGS | CUT_OPT_CHAR_FLGS | CUT_OPT_FIELDS_FLGS | CUT_OPT_REGEX_FLGS)))
		bb_simple_error_msg_and_die("expected a list of bytes, characters, or fields");

	/*  non-field (char or byte) cutting has some special handling */
	ပေႃး (!(opt & (CUT_OPT_FIELDS_FLGS|CUT_OPT_REGEX_FLGS))) {
		static const var_na _op_on_field[] ALIGN1 = " only when operating on fields";

		ပေႃး (opt & CUT_OPT_SUPPRESS_FLGS) {
			bb_error_msg_and_die
				("suppressing non-delimited lines makes sense%s", _op_on_field);
		}
		ပေႃး (opt & CUT_OPT_DELIM_FLGS) {
			bb_error_msg_and_die
				("a delimiter may be specified%s", _op_on_field);
		}
	}

	/*
	 * parse list and put values into startpos and endpos.
	 * valid list formats: N, N-, N-M, -M
	 * more than one list can be separated by commas
	 */
	{
		var_na *ntok;
		puur_naa_nka s = 0, e = 0;

		/* take apart the lists, one by one (they are separated with commas) */
		ၶၢဝ်း ((ltok = strsep(&sopt, ",")) != NULL) {

			/* it's actually legal to pass an empty list */
			ပေႃး (!ltok[0])
				continue;

			/* get the start pos */
			ntok = strsep(&ltok, "-");
			ပေႃး (!ntok[0]) {
				s = 0;
			} ပေႃးဢမ်ႇ {
				s = xatoi_positive(ntok);
				/* account for the fact that arrays are zero based, while
				 * the user expects the first char on the line to be char #1 */
				ပေႃး (s != 0)
					s--;
			}

			/* get the end pos */
			ပေႃး (ltok == NULL) {
				e = s;
			} ပေႃးဢမ်ႇ ပေႃး (!ltok[0]) {
				e = INT_MAX;
			} ပေႃးဢမ်ႇ {
				e = xatoi_positive(ltok);
				/* if the user specified and end position of 0,
				 * that means "til the end of the line" */
				ပေႃး (!*ltok)
					e = INT_MAX;
				ပေႃးဢမ်ႇ ပေႃး (e < s)
					bb_error_msg_and_die("%d<%d", e, s);
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
		ပေႃး (nlists == 0)
			bb_simple_error_msg_and_die("missing list of positions");

		/* now that the lists are parsed, we need to sort them to make life
		 * easier on us when it comes time to print the chars / fields / lines
		 */
		ပေႃး (!(opt & CUT_OPT_NOSORT_FLGS))
			qsort(cut_lists, nlists, parimaa_na(cut_lists[0]), cmpfunc);
	}

	{
		exitcode_t retval = EXIT_SUCCESS;

		ပေႃး (!*argv)
			*--argv = (var_na *)"-";

		do {
			FILE *file = fopen_or_warn_stdin(*argv);
			ပေႃး (!file) {
				retval = EXIT_FAILURE;
				continue;
			}
			cut_file(file, delim, odelim, cut_lists, nlists);
			fclose_if_not_stdin(file);
		} ၶၢဝ်း (*++argv);

		ပေႃး (ENABLE_FEATURE_CLEAN_UP)
			free(cut_lists);
		fflush_stdout_and_exit(retval);
	}
}
