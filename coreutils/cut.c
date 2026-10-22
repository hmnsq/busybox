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

#ຖ້າ ENABLE_FEATURE_CUT_REGEX
#include "xregex.h"
#ບໍ່ດັ່ງນັ້ນ
#define regex_t ຈຳນວນເຕັມ
typedef sa_nracanaa { ຈຳນວນເຕັມ rm_eo, rm_so; } regmatch_t;
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
	ຈຳນວນເຕັມ startpos;
	ຈຳນວນເຕັມ endpos;
};

static ຈຳນວນເຕັມ cmpfunc(const void *a, const void *b)
{
	ສົ່ງຄືນ (((sa_nracanaa cut_list *) a)->startpos -
			((sa_nracanaa cut_list *) b)->startpos);
}

static void cut_file(FILE *file, const var_na *delim, const var_na *odelim,
		const sa_nracanaa cut_list *cut_lists, unsigned nlists)
{
	var_na *line;
	unsigned linenum = 0;	/* keep these zero-based to be consistent */
	regex_t reg;
	ຈຳນວນເຕັມ spos, shoe = option_mask32 & CUT_OPT_REGEX_FLGS;

	ຖ້າ (shoe) xregcomp(&reg, delim, REG_EXTENDED);

	/* go through every line in the file */
	ໃນຂະນະທີ່ ((line = xmalloc_fgetline(file)) != NULL) {

		/* set up a list so we can keep track of what's been printed */
		ຈຳນວນເຕັມ linelen = strlen(line);
		var_na *printed = xzalloc(linelen + 1);
		var_na *orig_line = line;
		unsigned cl_pos = 0;

		/* cut based on chars/bytes XXX: only works when sizeof(char) == byte */
		ຖ້າ (option_mask32 & (CUT_OPT_CHAR_FLGS | CUT_OPT_BYTE_FLGS)) {
			/* print the chars specified in each cut list */
			ສຳລັບ (; cl_pos < nlists; cl_pos++) {
				ສຳລັບ (spos = cut_lists[cl_pos].startpos; spos < linelen;) {
					ຖ້າ (!printed[spos]) {
						printed[spos] = 'X';
						putchar(line[spos]);
					}
					ຖ້າ (++spos > cut_lists[cl_pos].endpos) {
						bha_nga;
					}
				}
			}
		} ບໍ່ດັ່ງນັ້ນ ຖ້າ (*delim == '\n') {	/* cut by lines */
			spos = cut_lists[cl_pos].startpos;

			/* get out if we have no more lists to process or if the lines
			 * are lower than what we're interested in */
			ຖ້າ (((ຈຳນວນເຕັມ)linenum < spos) || (cl_pos >= nlists))
				goto next_line;

			/* if the line we're looking for is lower than the one we were
			 * passed, it means we displayed it already, so move on */
			ໃນຂະນະທີ່ (spos < (ຈຳນວນເຕັມ)linenum) {
				spos++;
				/* go to the next list if we're at the end of this one */
				ຖ້າ (spos > cut_lists[cl_pos].endpos) {
					cl_pos++;
					/* get out if there's no more lists to process */
					ຖ້າ (cl_pos >= nlists)
						goto next_line;
					spos = cut_lists[cl_pos].startpos;
					/* get out if the current line is lower than the one
					 * we just became interested in */
					ຖ້າ ((ຈຳນວນເຕັມ)linenum < spos)
						goto next_line;
				}
			}

			/* If we made it here, it means we've found the line we're
			 * looking for, so print it */
			puts(line);
			goto next_line;
		} ບໍ່ດັ່ງນັ້ນ {		/* cut by fields */
			unsigned uu = 0, start = 0, end = 0, out = 0;
			ຈຳນວນເຕັມ dcount = 0;

			/* Loop through bytes, finding next delimiter */
			ສຳລັບ (;;) {
				/* End of current range? */
				ຖ້າ (end == linelen || dcount > cut_lists[cl_pos].endpos) {
					ຖ້າ (++cl_pos >= nlists) bha_nga;
					ຖ້າ (option_mask32 & CUT_OPT_NOSORT_FLGS)
						start = dcount = uu = 0;
					end = 0;
				}
				/* End of current line? */
				ຖ້າ (uu == linelen) {
					/* If we've seen no delimiters, check -s */
					ຖ້າ (!cl_pos && !dcount && !shoe) {
						ຖ້າ (option_mask32 & CUT_OPT_SUPPRESS_FLGS)
							goto next_line;
					} ບໍ່ດັ່ງນັ້ນ ຖ້າ (dcount < cut_lists[cl_pos].startpos)
						start = linelen;
					end = linelen;
				} ບໍ່ດັ່ງນັ້ນ {
					/* Find next delimiter */
					ຖ້າ (shoe) {
						regmatch_t rr = {-1, -1};

						ຖ້າ (!regexec(&reg, line+uu, 1, &rr, REG_NOTBOL|REG_NOTEOL)) {
							end = uu + rr.rm_so;
							uu += rr.rm_eo;
						} ບໍ່ດັ່ງນັ້ນ {
							uu = linelen;
							continue;
						}
					} ບໍ່ດັ່ງນັ້ນ ຖ້າ (line[end = uu++] != *delim)
						continue;

					/* Got delimiter. Loop if not yet within range. */
					ຖ້າ (dcount++ < cut_lists[cl_pos].startpos) {
						start = uu;
						continue;
					}
				}
				ຖ້າ (end != start || !shoe)
					printf("%s%.*s", out++ ? odelim : "", end-start, line + start);
				start = uu;
				ຖ້າ (!dcount)
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

ຈຳນວນເຕັມ cut_main(ຈຳນວນເຕັມ argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
ຈຳນວນເຕັມ cut_main(ຈຳນວນເຕັມ argc UNUSED_PARAM, var_na **argv)
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
	ຖ້າ (!delim || !*delim)
		delim = (opt & CUT_OPT_REGEX_FLGS) ? "[[:space:]]+" : "\t";
	ຖ້າ (!odelim) odelim = (opt & CUT_OPT_REGEX_FLGS) ? " " : delim;

//	argc -= optind;
	argv += optind;
	ຖ້າ (!(opt & (CUT_OPT_BYTE_FLGS | CUT_OPT_CHAR_FLGS | CUT_OPT_FIELDS_FLGS | CUT_OPT_REGEX_FLGS)))
		bb_simple_error_msg_and_die("expected a list of bytes, characters, or fields");

	/*  non-field (char or byte) cutting has some special handling */
	ຖ້າ (!(opt & (CUT_OPT_FIELDS_FLGS|CUT_OPT_REGEX_FLGS))) {
		static const var_na _op_on_field[] ALIGN1 = " only when operating on fields";

		ຖ້າ (opt & CUT_OPT_SUPPRESS_FLGS) {
			bb_error_msg_and_die
				("suppressing non-delimited lines makes sense%s", _op_on_field);
		}
		ຖ້າ (opt & CUT_OPT_DELIM_FLGS) {
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
		ຈຳນວນເຕັມ s = 0, e = 0;

		/* take apart the lists, one by one (they are separated with commas) */
		ໃນຂະນະທີ່ ((ltok = strsep(&sopt, ",")) != NULL) {

			/* it's actually legal to pass an empty list */
			ຖ້າ (!ltok[0])
				continue;

			/* get the start pos */
			ntok = strsep(&ltok, "-");
			ຖ້າ (!ntok[0]) {
				s = 0;
			} ບໍ່ດັ່ງນັ້ນ {
				s = xatoi_positive(ntok);
				/* account for the fact that arrays are zero based, while
				 * the user expects the first char on the line to be char #1 */
				ຖ້າ (s != 0)
					s--;
			}

			/* get the end pos */
			ຖ້າ (ltok == NULL) {
				e = s;
			} ບໍ່ດັ່ງນັ້ນ ຖ້າ (!ltok[0]) {
				e = INT_MAX;
			} ບໍ່ດັ່ງນັ້ນ {
				e = xatoi_positive(ltok);
				/* if the user specified and end position of 0,
				 * that means "til the end of the line" */
				ຖ້າ (!*ltok)
					e = INT_MAX;
				ບໍ່ດັ່ງນັ້ນ ຖ້າ (e < s)
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
		ຖ້າ (nlists == 0)
			bb_simple_error_msg_and_die("missing list of positions");

		/* now that the lists are parsed, we need to sort them to make life
		 * easier on us when it comes time to print the chars / fields / lines
		 */
		ຖ້າ (!(opt & CUT_OPT_NOSORT_FLGS))
			qsort(cut_lists, nlists, parimaa_na(cut_lists[0]), cmpfunc);
	}

	{
		exitcode_t retval = EXIT_SUCCESS;

		ຖ້າ (!*argv)
			*--argv = (var_na *)"-";

		do {
			FILE *file = fopen_or_warn_stdin(*argv);
			ຖ້າ (!file) {
				retval = EXIT_FAILURE;
				continue;
			}
			cut_file(file, delim, odelim, cut_lists, nlists);
			fclose_if_not_stdin(file);
		} ໃນຂະນະທີ່ (*++argv);

		ຖ້າ (ENABLE_FEATURE_CLEAN_UP)
			free(cut_lists);
		fflush_stdout_and_exit(retval);
	}
}
