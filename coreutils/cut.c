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

#અન્તર્ભાવ "libbb.h"

#જો ENABLE_FEATURE_CUT_REGEX
#અન્તર્ભાવ "xregex.h"
#નહીંતર
#define regex_t પૂર્ણાંક
પ્રકારવ્યાખ્યા સંરચના { પૂર્ણાંક rm_eo, rm_so; } regmatch_t;
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

સંરચના cut_list {
	પૂર્ણાંક startpos;
	પૂર્ણાંક endpos;
};

સ્થિર પૂર્ણાંક cmpfunc(અચલ શૂન્ય *a, અચલ શૂન્ય *b)
{
	પાછા (((સંરચના cut_list *) a)->startpos -
			((સંરચના cut_list *) b)->startpos);
}

સ્થિર શૂન્ય cut_file(FILE *file, અચલ અક્ષર *delim, અચલ અક્ષર *odelim,
		અચલ સંરચના cut_list *cut_lists, unsigned nlists)
{
	અક્ષર *લીટી;
	unsigned linenum = 0;	/* keep these zero-based to be consistent */
	regex_t reg;
	પૂર્ણાંક spos, shoe = option_mask32 & CUT_OPT_REGEX_FLGS;

	જો (shoe) xregcomp(&reg, delim, REG_EXTENDED);

	/* go through every line in the file */
	જ્યારે ((લીટી = xmalloc_fgetline(file)) != ખાલી) {

		/* set up a list so we can keep track of what's been printed */
		પૂર્ણાંક linelen = strlen(લીટી);
		અક્ષર *printed = xzalloc(linelen + 1);
		અક્ષર *orig_લીટી = લીટી;
		unsigned cl_pos = 0;

		/* cut based on chars/bytes XXX: only works when sizeof(char) == byte */
		જો (option_mask32 & (CUT_OPT_CHAR_FLGS | CUT_OPT_BYTE_FLGS)) {
			/* print the chars specified in each cut list */
			માટે (; cl_pos < nlists; cl_pos++) {
				માટે (spos = cut_lists[cl_pos].startpos; spos < linelen;) {
					જો (!printed[spos]) {
						printed[spos] = 'X';
						putchar(લીટી[spos]);
					}
					જો (++spos > cut_lists[cl_pos].endpos) {
						તોડો;
					}
				}
			}
		} નહીંતર જો (*delim == '\n') {	/* cut by lines */
			spos = cut_lists[cl_pos].startpos;

			/* get out if we have no more lists to process or if the lines
			 * are lower than what we're interested in */
			જો (((પૂર્ણાંક)linenum < spos) || (cl_pos >= nlists))
				જાઓ next_લીટી;

			/* if the line we're looking for is lower than the one we were
			 * passed, it means we displayed it already, so move on */
			જ્યારે (spos < (પૂર્ણાંક)linenum) {
				spos++;
				/* go to the next list if we're at the end of this one */
				જો (spos > cut_lists[cl_pos].endpos) {
					cl_pos++;
					/* get out if there's no more lists to process */
					જો (cl_pos >= nlists)
						જાઓ next_લીટી;
					spos = cut_lists[cl_pos].startpos;
					/* get out if the current line is lower than the one
					 * we just became interested in */
					જો ((પૂર્ણાંક)linenum < spos)
						જાઓ next_લીટી;
				}
			}

			/* If we made it here, it means we've found the line we're
			 * looking for, so print it */
			puts(લીટી);
			જાઓ next_લીટી;
		} નહીંતર {		/* cut by fields */
			unsigned uu = 0, start = 0, end = 0, out = 0;
			પૂર્ણાંક dcount = 0;

			/* Loop through bytes, finding next delimiter */
			માટે (;;) {
				/* End of current range? */
				જો (end == linelen || dcount > cut_lists[cl_pos].endpos) {
					જો (++cl_pos >= nlists) તોડો;
					જો (option_mask32 & CUT_OPT_NOSORT_FLGS)
						start = dcount = uu = 0;
					end = 0;
				}
				/* End of current line? */
				જો (uu == linelen) {
					/* If we've seen no delimiters, check -s */
					જો (!cl_pos && !dcount && !shoe) {
						જો (option_mask32 & CUT_OPT_SUPPRESS_FLGS)
							જાઓ next_લીટી;
					} નહીંતર જો (dcount < cut_lists[cl_pos].startpos)
						start = linelen;
					end = linelen;
				} નહીંતર {
					/* Find next delimiter */
					જો (shoe) {
						regmatch_t rr = {-1, -1};

						જો (!regexec(&reg, લીટી+uu, 1, &rr, REG_NOTBOL|REG_NOTEOL)) {
							end = uu + rr.rm_so;
							uu += rr.rm_eo;
						} નહીંતર {
							uu = linelen;
							ચાલુ;
						}
					} નહીંતર જો (લીટી[end = uu++] != *delim)
						ચાલુ;

					/* Got delimiter. Loop if not yet within range. */
					જો (dcount++ < cut_lists[cl_pos].startpos) {
						start = uu;
						ચાલુ;
					}
				}
				જો (end != start || !shoe)
					printf("%s%.*s", out++ ? odelim : "", end-start, લીટી + start);
				start = uu;
				જો (!dcount)
					તોડો;
			}
		}
		/* if we printed anything, finish with newline */
		putchar('\n');
 next_લીટી:
		linenum++;
		મુક્ત(printed);
		મુક્ત(orig_લીટી);
	}
}

પૂર્ણાંક cut_main(પૂર્ણાંક argc, અક્ષર **argv) MAIN_EXTERNALLY_VISIBLE;
પૂર્ણાંક cut_main(પૂર્ણાંક argc UNUSED_PARAM, અક્ષર **argv)
{
	/* growable array holding a series of lists */
	સંરચના cut_list *cut_lists = ખાલી;
	unsigned nlists = 0;	/* number of elements in above list */
	અક્ષર *sopt, *ltok;
	અચલ અક્ષર *delim = ખાલી;
	અચલ અક્ષર *odelim = ખાલી;
	unsigned opt;

#define ARG "bcf"IF_FEATURE_CUT_REGEX("F")
	opt = getopt32(argv, "^"
			OPT_STR  // = "b:c:f:d:O:sD"IF_FEATURE_CUT_REGEX("F:")"n"
			"\0" "b--"ARG":c--"ARG":f--"ARG IF_FEATURE_CUT_REGEX("F--"ARG),
			&sopt, &sopt, &sopt, &delim, &odelim IF_FEATURE_CUT_REGEX(, &sopt)
	);
	જો (!delim || !*delim)
		delim = (opt & CUT_OPT_REGEX_FLGS) ? "[[:space:]]+" : "\t";
	જો (!odelim) odelim = (opt & CUT_OPT_REGEX_FLGS) ? " " : delim;

//	argc -= optind;
	argv += optind;
	જો (!(opt & (CUT_OPT_BYTE_FLGS | CUT_OPT_CHAR_FLGS | CUT_OPT_FIELDS_FLGS | CUT_OPT_REGEX_FLGS)))
		bb_simple_error_સંદેશ_and_die("expected a list of bytes, characters, or fields");

	/*  non-field (char or byte) cutting has some special handling */
	જો (!(opt & (CUT_OPT_FIELDS_FLGS|CUT_OPT_REGEX_FLGS))) {
		સ્થિર અચલ અક્ષર _op_on_field[] ALIGN1 = " only when operating on fields";

		જો (opt & CUT_OPT_SUPPRESS_FLGS) {
			bb_error_સંદેશ_and_die
				("suppressing non-delimited lines makes sense%s", _op_on_field);
		}
		જો (opt & CUT_OPT_DELIM_FLGS) {
			bb_error_સંદેશ_and_die
				("a delimiter may be specified%s", _op_on_field);
		}
	}

	/*
	 * parse list and put values into startpos and endpos.
	 * valid list formats: N, N-, N-M, -M
	 * more than one list can be separated by commas
	 */
	{
		અક્ષર *ntok;
		પૂર્ણાંક s = 0, e = 0;

		/* take apart the lists, one by one (they are separated with commas) */
		જ્યારે ((ltok = strsep(&sopt, ",")) != ખાલી) {

			/* it's actually legal to pass an empty list */
			જો (!ltok[0])
				ચાલુ;

			/* get the start pos */
			ntok = strsep(&ltok, "-");
			જો (!ntok[0]) {
				s = 0;
			} નહીંતર {
				s = xatoi_positive(ntok);
				/* account for the fact that arrays are zero based, while
				 * the user expects the first char on the line to be char #1 */
				જો (s != 0)
					s--;
			}

			/* get the end pos */
			જો (ltok == ખાલી) {
				e = s;
			} નહીંતર જો (!ltok[0]) {
				e = INT_MAX;
			} નહીંતર {
				e = xatoi_positive(ltok);
				/* if the user specified and end position of 0,
				 * that means "til the end of the line" */
				જો (!*ltok)
					e = INT_MAX;
				નહીંતર જો (e < s)
					bb_error_સંદેશ_and_die("%d<%d", e, s);
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
		જો (nlists == 0)
			bb_simple_error_સંદેશ_and_die("missing list of positions");

		/* now that the lists are parsed, we need to sort them to make life
		 * easier on us when it comes time to print the chars / fields / lines
		 */
		જો (!(opt & CUT_OPT_NOSORT_FLGS))
			qsort(cut_lists, nlists, કદ(cut_lists[0]), cmpfunc);
	}

	{
		exitcode_t retval = EXIT_SUCCESS;

		જો (!*argv)
			*--argv = (અક્ષર *)"-";

		કરો {
			FILE *file = fopen_or_warn_stdin(*argv);
			જો (!file) {
				retval = EXIT_FAILURE;
				ચાલુ;
			}
			cut_file(file, delim, odelim, cut_lists, nlists);
			fclose_if_not_stdin(file);
		} જ્યારે (*++argv);

		જો (ENABLE_FEATURE_CLEAN_UP)
			મુક્ત(cut_lists);
		fflush_stdout_and_exit(retval);
	}
}
