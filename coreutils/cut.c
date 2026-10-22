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

#شامل "libbb.h"

#اگر ENABLE_FEATURE_CUT_REGEX
#شامل "xregex.h"
#وگرنه
#define regex_t صحیح
تعریفنوع ساختار { صحیح rm_eo, rm_so; } regmatch_t;
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

ساختار cut_list {
	صحیح startpos;
	صحیح endpos;
};

ایستا صحیح cmpfunc(ثابت تهی *a, ثابت تهی *b)
{
	بازگشت (((ساختار cut_list *) a)->startpos -
			((ساختار cut_list *) b)->startpos);
}

ایستا تهی cut_file(FILE *file, ثابت نویسه *delim, ثابت نویسه *odelim,
		ثابت ساختار cut_list *cut_lists, unsigned nlists)
{
	نویسه *سطر;
	unsigned linenum = 0;	/* keep these zero-based to be consistent */
	regex_t reg;
	صحیح spos, shoe = option_mask32 & CUT_OPT_REGEX_FLGS;

	اگر (shoe) xregcomp(&reg, delim, REG_EXTENDED);

	/* go through every line in the file */
	وقتی ((سطر = xmalloc_fgetline(file)) != پوچ) {

		/* set up a list so we can keep track of what's been printed */
		صحیح linelen = strlen(سطر);
		نویسه *printed = xzalloc(linelen + 1);
		نویسه *orig_سطر = سطر;
		unsigned cl_pos = 0;

		/* cut based on chars/bytes XXX: only works when sizeof(char) == byte */
		اگر (option_mask32 & (CUT_OPT_CHAR_FLGS | CUT_OPT_BYTE_FLGS)) {
			/* print the chars specified in each cut list */
			برای (; cl_pos < nlists; cl_pos++) {
				برای (spos = cut_lists[cl_pos].startpos; spos < linelen;) {
					اگر (!printed[spos]) {
						printed[spos] = 'X';
						putchar(سطر[spos]);
					}
					اگر (++spos > cut_lists[cl_pos].endpos) {
						بشکن;
					}
				}
			}
		} وگرنه اگر (*delim == '\n') {	/* cut by lines */
			spos = cut_lists[cl_pos].startpos;

			/* get out if we have no more lists to process or if the lines
			 * are lower than what we're interested in */
			اگر (((صحیح)linenum < spos) || (cl_pos >= nlists))
				برو next_سطر;

			/* if the line we're looking for is lower than the one we were
			 * passed, it means we displayed it already, so move on */
			وقتی (spos < (صحیح)linenum) {
				spos++;
				/* go to the next list if we're at the end of this one */
				اگر (spos > cut_lists[cl_pos].endpos) {
					cl_pos++;
					/* get out if there's no more lists to process */
					اگر (cl_pos >= nlists)
						برو next_سطر;
					spos = cut_lists[cl_pos].startpos;
					/* get out if the current line is lower than the one
					 * we just became interested in */
					اگر ((صحیح)linenum < spos)
						برو next_سطر;
				}
			}

			/* If we made it here, it means we've found the line we're
			 * looking for, so print it */
			puts(سطر);
			برو next_سطر;
		} وگرنه {		/* cut by fields */
			unsigned uu = 0, start = 0, end = 0, out = 0;
			صحیح dcount = 0;

			/* Loop through bytes, finding next delimiter */
			برای (;;) {
				/* End of current range? */
				اگر (end == linelen || dcount > cut_lists[cl_pos].endpos) {
					اگر (++cl_pos >= nlists) بشکن;
					اگر (option_mask32 & CUT_OPT_NOSORT_FLGS)
						start = dcount = uu = 0;
					end = 0;
				}
				/* End of current line? */
				اگر (uu == linelen) {
					/* If we've seen no delimiters, check -s */
					اگر (!cl_pos && !dcount && !shoe) {
						اگر (option_mask32 & CUT_OPT_SUPPRESS_FLGS)
							برو next_سطر;
					} وگرنه اگر (dcount < cut_lists[cl_pos].startpos)
						start = linelen;
					end = linelen;
				} وگرنه {
					/* Find next delimiter */
					اگر (shoe) {
						regmatch_t rr = {-1, -1};

						اگر (!regexec(&reg, سطر+uu, 1, &rr, REG_NOTBOL|REG_NOTEOL)) {
							end = uu + rr.rm_so;
							uu += rr.rm_eo;
						} وگرنه {
							uu = linelen;
							ادامه;
						}
					} وگرنه اگر (سطر[end = uu++] != *delim)
						ادامه;

					/* Got delimiter. Loop if not yet within range. */
					اگر (dcount++ < cut_lists[cl_pos].startpos) {
						start = uu;
						ادامه;
					}
				}
				اگر (end != start || !shoe)
					printf("%s%.*s", out++ ? odelim : "", end-start, سطر + start);
				start = uu;
				اگر (!dcount)
					بشکن;
			}
		}
		/* if we printed anything, finish with newline */
		putchar('\n');
 next_سطر:
		linenum++;
		آزاد(printed);
		آزاد(orig_سطر);
	}
}

صحیح cut_main(صحیح argc, نویسه **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح cut_main(صحیح argc UNUSED_PARAM, نویسه **argv)
{
	/* growable array holding a series of lists */
	ساختار cut_list *cut_lists = پوچ;
	unsigned nlists = 0;	/* number of elements in above list */
	نویسه *sopt, *ltok;
	ثابت نویسه *delim = پوچ;
	ثابت نویسه *odelim = پوچ;
	unsigned opt;

#define ARG "bcf"IF_FEATURE_CUT_REGEX("F")
	opt = getopt32(argv, "^"
			OPT_STR  // = "b:c:f:d:O:sD"IF_FEATURE_CUT_REGEX("F:")"n"
			"\0" "b--"ARG":c--"ARG":f--"ARG IF_FEATURE_CUT_REGEX("F--"ARG),
			&sopt, &sopt, &sopt, &delim, &odelim IF_FEATURE_CUT_REGEX(, &sopt)
	);
	اگر (!delim || !*delim)
		delim = (opt & CUT_OPT_REGEX_FLGS) ? "[[:space:]]+" : "\t";
	اگر (!odelim) odelim = (opt & CUT_OPT_REGEX_FLGS) ? " " : delim;

//	argc -= optind;
	argv += optind;
	اگر (!(opt & (CUT_OPT_BYTE_FLGS | CUT_OPT_CHAR_FLGS | CUT_OPT_FIELDS_FLGS | CUT_OPT_REGEX_FLGS)))
		bb_simple_error_پیام_and_die("expected a list of bytes, characters, or fields");

	/*  non-field (char or byte) cutting has some special handling */
	اگر (!(opt & (CUT_OPT_FIELDS_FLGS|CUT_OPT_REGEX_FLGS))) {
		ایستا ثابت نویسه _op_on_field[] ALIGN1 = " only when operating on fields";

		اگر (opt & CUT_OPT_SUPPRESS_FLGS) {
			bb_error_پیام_and_die
				("suppressing non-delimited lines makes sense%s", _op_on_field);
		}
		اگر (opt & CUT_OPT_DELIM_FLGS) {
			bb_error_پیام_and_die
				("a delimiter may be specified%s", _op_on_field);
		}
	}

	/*
	 * parse list and put values into startpos and endpos.
	 * valid list formats: N, N-, N-M, -M
	 * more than one list can be separated by commas
	 */
	{
		نویسه *ntok;
		صحیح s = 0, e = 0;

		/* take apart the lists, one by one (they are separated with commas) */
		وقتی ((ltok = strsep(&sopt, ",")) != پوچ) {

			/* it's actually legal to pass an empty list */
			اگر (!ltok[0])
				ادامه;

			/* get the start pos */
			ntok = strsep(&ltok, "-");
			اگر (!ntok[0]) {
				s = 0;
			} وگرنه {
				s = xatoi_positive(ntok);
				/* account for the fact that arrays are zero based, while
				 * the user expects the first char on the line to be char #1 */
				اگر (s != 0)
					s--;
			}

			/* get the end pos */
			اگر (ltok == پوچ) {
				e = s;
			} وگرنه اگر (!ltok[0]) {
				e = INT_MAX;
			} وگرنه {
				e = xatoi_positive(ltok);
				/* if the user specified and end position of 0,
				 * that means "til the end of the line" */
				اگر (!*ltok)
					e = INT_MAX;
				وگرنه اگر (e < s)
					bb_error_پیام_and_die("%d<%d", e, s);
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
		اگر (nlists == 0)
			bb_simple_error_پیام_and_die("missing list of positions");

		/* now that the lists are parsed, we need to sort them to make life
		 * easier on us when it comes time to print the chars / fields / lines
		 */
		اگر (!(opt & CUT_OPT_NOSORT_FLGS))
			qsort(cut_lists, nlists, اندازه(cut_lists[0]), cmpfunc);
	}

	{
		exitcode_t retval = EXIT_SUCCESS;

		اگر (!*argv)
			*--argv = (نویسه *)"-";

		انجامبده {
			FILE *file = fopen_or_warn_stdin(*argv);
			اگر (!file) {
				retval = EXIT_FAILURE;
				ادامه;
			}
			cut_file(file, delim, odelim, cut_lists, nlists);
			fclose_if_not_stdin(file);
		} وقتی (*++argv);

		اگر (ENABLE_FEATURE_CLEAN_UP)
			آزاد(cut_lists);
		fflush_stdout_and_exit(retval);
	}
}
