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

#ако ENABLE_FEATURE_CUT_REGEX
#include "xregex.h"
#инако
#define regex_t цео
typedef struct { цео rm_eo, rm_so; } regmatch_t;
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

struct cut_list {
	цео startpos;
	цео endpos;
};

static цео cmpfunc(const void *a, const void *b)
{
	врати (((struct cut_list *) a)->startpos -
			((struct cut_list *) b)->startpos);
}

static void cut_file(FILE *file, const char *delim, const char *odelim,
		const struct cut_list *cut_lists, unsigned nlists)
{
	char *line;
	unsigned linenum = 0;	/* keep these zero-based to be consistent */
	regex_t reg;
	цео spos, shoe = option_mask32 & CUT_OPT_REGEX_FLGS;

	ако (shoe) xregcomp(&reg, delim, REG_EXTENDED);

	/* go through every line in the file */
	док ((line = xmalloc_fgetline(file)) != NULL) {

		/* set up a list so we can keep track of what's been printed */
		цео linelen = strlen(line);
		char *printed = xzalloc(linelen + 1);
		char *orig_line = line;
		unsigned cl_pos = 0;

		/* cut based on chars/bytes XXX: only works when sizeof(char) == byte */
		ако (option_mask32 & (CUT_OPT_CHAR_FLGS | CUT_OPT_BYTE_FLGS)) {
			/* print the chars specified in each cut list */
			за (; cl_pos < nlists; cl_pos++) {
				за (spos = cut_lists[cl_pos].startpos; spos < linelen;) {
					ако (!printed[spos]) {
						printed[spos] = 'X';
						putchar(line[spos]);
					}
					ако (++spos > cut_lists[cl_pos].endpos) {
						break;
					}
				}
			}
		} инако ако (*delim == '\n') {	/* cut by lines */
			spos = cut_lists[cl_pos].startpos;

			/* get out if we have no more lists to process or if the lines
			 * are lower than what we're interested in */
			ако (((цео)linenum < spos) || (cl_pos >= nlists))
				goto next_line;

			/* if the line we're looking for is lower than the one we were
			 * passed, it means we displayed it already, so move on */
			док (spos < (цео)linenum) {
				spos++;
				/* go to the next list if we're at the end of this one */
				ако (spos > cut_lists[cl_pos].endpos) {
					cl_pos++;
					/* get out if there's no more lists to process */
					ако (cl_pos >= nlists)
						goto next_line;
					spos = cut_lists[cl_pos].startpos;
					/* get out if the current line is lower than the one
					 * we just became interested in */
					ако ((цео)linenum < spos)
						goto next_line;
				}
			}

			/* If we made it here, it means we've found the line we're
			 * looking for, so print it */
			puts(line);
			goto next_line;
		} инако {		/* cut by fields */
			unsigned uu = 0, start = 0, end = 0, out = 0;
			цео dcount = 0;

			/* Loop through bytes, finding next delimiter */
			за (;;) {
				/* End of current range? */
				ако (end == linelen || dcount > cut_lists[cl_pos].endpos) {
					ако (++cl_pos >= nlists) break;
					ако (option_mask32 & CUT_OPT_NOSORT_FLGS)
						start = dcount = uu = 0;
					end = 0;
				}
				/* End of current line? */
				ако (uu == linelen) {
					/* If we've seen no delimiters, check -s */
					ако (!cl_pos && !dcount && !shoe) {
						ако (option_mask32 & CUT_OPT_SUPPRESS_FLGS)
							goto next_line;
					} инако ако (dcount < cut_lists[cl_pos].startpos)
						start = linelen;
					end = linelen;
				} инако {
					/* Find next delimiter */
					ако (shoe) {
						regmatch_t rr = {-1, -1};

						ако (!regexec(&reg, line+uu, 1, &rr, REG_NOTBOL|REG_NOTEOL)) {
							end = uu + rr.rm_so;
							uu += rr.rm_eo;
						} инако {
							uu = linelen;
							continue;
						}
					} инако ако (line[end = uu++] != *delim)
						continue;

					/* Got delimiter. Loop if not yet within range. */
					ако (dcount++ < cut_lists[cl_pos].startpos) {
						start = uu;
						continue;
					}
				}
				ако (end != start || !shoe)
					printf("%s%.*s", out++ ? odelim : "", end-start, line + start);
				start = uu;
				ако (!dcount)
					break;
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

цео cut_main(цео argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
цео cut_main(цео argc UNUSED_PARAM, char **argv)
{
	/* growable array holding a series of lists */
	struct cut_list *cut_lists = NULL;
	unsigned nlists = 0;	/* number of elements in above list */
	char *sopt, *ltok;
	const char *delim = NULL;
	const char *odelim = NULL;
	unsigned opt;

#define ARG "bcf"IF_FEATURE_CUT_REGEX("F")
	opt = getopt32(argv, "^"
			OPT_STR  // = "b:c:f:d:O:sD"IF_FEATURE_CUT_REGEX("F:")"n"
			"\0" "b--"ARG":c--"ARG":f--"ARG IF_FEATURE_CUT_REGEX("F--"ARG),
			&sopt, &sopt, &sopt, &delim, &odelim IF_FEATURE_CUT_REGEX(, &sopt)
	);
	ако (!delim || !*delim)
		delim = (opt & CUT_OPT_REGEX_FLGS) ? "[[:space:]]+" : "\t";
	ако (!odelim) odelim = (opt & CUT_OPT_REGEX_FLGS) ? " " : delim;

//	argc -= optind;
	argv += optind;
	ако (!(opt & (CUT_OPT_BYTE_FLGS | CUT_OPT_CHAR_FLGS | CUT_OPT_FIELDS_FLGS | CUT_OPT_REGEX_FLGS)))
		bb_simple_error_msg_and_die("expected a list of bytes, characters, or fields");

	/*  non-field (char or byte) cutting has some special handling */
	ако (!(opt & (CUT_OPT_FIELDS_FLGS|CUT_OPT_REGEX_FLGS))) {
		static const char _op_on_field[] ALIGN1 = " only when operating on fields";

		ако (opt & CUT_OPT_SUPPRESS_FLGS) {
			bb_error_msg_and_die
				("suppressing non-delimited lines makes sense%s", _op_on_field);
		}
		ако (opt & CUT_OPT_DELIM_FLGS) {
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
		char *ntok;
		цео s = 0, e = 0;

		/* take apart the lists, one by one (they are separated with commas) */
		док ((ltok = strsep(&sopt, ",")) != NULL) {

			/* it's actually legal to pass an empty list */
			ако (!ltok[0])
				continue;

			/* get the start pos */
			ntok = strsep(&ltok, "-");
			ако (!ntok[0]) {
				s = 0;
			} инако {
				s = xatoi_positive(ntok);
				/* account for the fact that arrays are zero based, while
				 * the user expects the first char on the line to be char #1 */
				ако (s != 0)
					s--;
			}

			/* get the end pos */
			ако (ltok == NULL) {
				e = s;
			} инако ако (!ltok[0]) {
				e = INT_MAX;
			} инако {
				e = xatoi_positive(ltok);
				/* if the user specified and end position of 0,
				 * that means "til the end of the line" */
				ако (!*ltok)
					e = INT_MAX;
				инако ако (e < s)
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
		ако (nlists == 0)
			bb_simple_error_msg_and_die("missing list of positions");

		/* now that the lists are parsed, we need to sort them to make life
		 * easier on us when it comes time to print the chars / fields / lines
		 */
		ако (!(opt & CUT_OPT_NOSORT_FLGS))
			qsort(cut_lists, nlists, sizeof(cut_lists[0]), cmpfunc);
	}

	{
		exitcode_t retval = EXIT_SUCCESS;

		ако (!*argv)
			*--argv = (char *)"-";

		do {
			FILE *file = fopen_or_warn_stdin(*argv);
			ако (!file) {
				retval = EXIT_FAILURE;
				continue;
			}
			cut_file(file, delim, odelim, cut_lists, nlists);
			fclose_if_not_stdin(file);
		} док (*++argv);

		ако (ENABLE_FEATURE_CLEAN_UP)
			free(cut_lists);
		fflush_stdout_and_exit(retval);
	}
}
