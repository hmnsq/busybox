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

#ضمن "libbb.h"

#إذا ENABLE_FEATURE_CUT_REGEX
#ضمن "xregex.h"
#وإلا
#وصف_ڪريو regex_t صحيح
تعريفنوع بنية { صحيح rm_eo, rm_so; } regmatch_t;
#وصف_ڪريو xregcomp(x, ...) *(x) = 0
#وصف_ڪريو regexec(...)     0
#جيڪڏهن_پورو

/* This is a NOEXEC applet. Be very careful! */


/* option vars */
#وصف_ڪريو OPT_STR "b:c:f:d:O:sD"IF_FEATURE_CUT_REGEX("F:")"n"
#وصف_ڪريو CUT_OPT_BYTE_FLGS     (1 << 0)
#وصف_ڪريو CUT_OPT_CHAR_FLGS     (1 << 1)
#وصف_ڪريو CUT_OPT_FIELDS_FLGS   (1 << 2)
#وصف_ڪريو CUT_OPT_DELIM_FLGS    (1 << 3)
#وصف_ڪريو CUT_OPT_ODELIM_FLGS   (1 << 4)
#وصف_ڪريو CUT_OPT_SUPPRESS_FLGS (1 << 5)
#وصف_ڪريو CUT_OPT_NOSORT_FLGS   (1 << 6)
#وصف_ڪريو CUT_OPT_REGEX_FLGS    ((1 << 7) * ENABLE_FEATURE_CUT_REGEX)

بنية cut_list {
	صحيح startpos;
	صحيح endpos;
};

ساكن صحيح cmpfunc(ثابت فراغ *a, ثابت فراغ *b)
{
	أرجع (((بنية cut_list *) a)->startpos -
			((بنية cut_list *) b)->startpos);
}

ساكن فراغ cut_file(FILE *file, ثابت حرف *delim, ثابت حرف *odelim,
		ثابت بنية cut_list *cut_lists, بنانشان nlists)
{
	حرف *سٽ;
	بنانشان linenum = 0;	/* keep these zero-based to be consistent */
	regex_t reg;
	صحيح spos, shoe = option_mask32 & CUT_OPT_REGEX_FLGS;

	إذا (shoe) xregcomp(&reg, delim, REG_EXTENDED);

	/* go through every line in the file */
	طالما ((سٽ = xmalloc_fgetline(file)) != عدم) {

		/* set up a list so we can keep track of what's been printed */
		صحيح linelen = لڙي_ڊيگهه(سٽ);
		حرف *printed = xzalloc(linelen + 1);
		حرف *orig_سٽ = سٽ;
		بنانشان cl_pos = 0;

		/* cut based on chars/bytes XXX: only works when sizeof(char) == byte */
		إذا (option_mask32 & (CUT_OPT_CHAR_FLGS | CUT_OPT_BYTE_FLGS)) {
			/* print the chars specified in each cut list */
			لكل (; cl_pos < nlists; cl_pos++) {
				لكل (spos = cut_lists[cl_pos].startpos; spos < linelen;) {
					إذا (!printed[spos]) {
						printed[spos] = 'X';
						اکر_لکو(سٽ[spos]);
					}
					إذا (++spos > cut_lists[cl_pos].endpos) {
						اكسر;
					}
				}
			}
		} وإلا إذا (*delim == '\n') {	/* cut by lines */
			spos = cut_lists[cl_pos].startpos;

			/* get out if we have no more lists to process or if the lines
			 * are lower than what we're interested in */
			إذا (((صحيح)linenum < spos) || (cl_pos >= nlists))
				اذهب ايندڙ_سٽ;

			/* if the line we're looking for is lower than the one we were
			 * passed, it means we displayed it already, so move on */
			طالما (spos < (صحيح)linenum) {
				spos++;
				/* go to the next list if we're at the end of this one */
				إذا (spos > cut_lists[cl_pos].endpos) {
					cl_pos++;
					/* get out if there's no more lists to process */
					إذا (cl_pos >= nlists)
						اذهب ايندڙ_سٽ;
					spos = cut_lists[cl_pos].startpos;
					/* get out if the current line is lower than the one
					 * we just became interested in */
					إذا ((صحيح)linenum < spos)
						اذهب ايندڙ_سٽ;
				}
			}

			/* If we made it here, it means we've found the line we're
			 * looking for, so print it */
			puts(سٽ);
			اذهب ايندڙ_سٽ;
		} وإلا {		/* cut by fields */
			بنانشان uu = 0, شروعات = 0, پڇاڙي = 0, ٻاهريون = 0;
			صحيح dcount = 0;

			/* Loop through bytes, finding next delimiter */
			لكل (;;) {
				/* End of current range? */
				إذا (پڇاڙي == linelen || dcount > cut_lists[cl_pos].endpos) {
					إذا (++cl_pos >= nlists) اكسر;
					إذا (option_mask32 & CUT_OPT_NOSORT_FLGS)
						شروعات = dcount = uu = 0;
					پڇاڙي = 0;
				}
				/* End of current line? */
				إذا (uu == linelen) {
					/* If we've seen no delimiters, check -s */
					إذا (!cl_pos && !dcount && !shoe) {
						إذا (option_mask32 & CUT_OPT_SUPPRESS_FLGS)
							اذهب ايندڙ_سٽ;
					} وإلا إذا (dcount < cut_lists[cl_pos].startpos)
						شروعات = linelen;
					پڇاڙي = linelen;
				} وإلا {
					/* Find next delimiter */
					إذا (shoe) {
						regmatch_t rr = {-1, -1};

						إذا (!regexec(&reg, سٽ+uu, 1, &rr, REG_NOTBOL|REG_NOTEOL)) {
							پڇاڙي = uu + rr.rm_so;
							uu += rr.rm_eo;
						} وإلا {
							uu = linelen;
							استمر;
						}
					} وإلا إذا (سٽ[پڇاڙي = uu++] != *delim)
						استمر;

					/* Got delimiter. Loop if not yet within range. */
					إذا (dcount++ < cut_lists[cl_pos].startpos) {
						شروعات = uu;
						استمر;
					}
				}
				إذا (پڇاڙي != شروعات || !shoe)
					لکو("%s%.*s", ٻاهريون++ ? odelim : "", پڇاڙي-شروعات, سٽ + شروعات);
				شروعات = uu;
				إذا (!dcount)
					اكسر;
			}
		}
		/* if we printed anything, finish with newline */
		اکر_لکو('\n');
 ايندڙ_سٽ:
		linenum++;
		حرر(printed);
		حرر(orig_سٽ);
	}
}

صحيح cut_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح cut_main(صحيح argc اڻ_استعمال_ٿيل_PARAM, حرف **argv)
{
	/* growable array holding a series of lists */
	بنية cut_list *cut_lists = عدم;
	بنانشان nlists = 0;	/* number of elements in above list */
	حرف *sopt, *ltok;
	ثابت حرف *delim = عدم;
	ثابت حرف *odelim = عدم;
	بنانشان opt;

#وصف_ڪريو ARG "bcf"IF_FEATURE_CUT_REGEX("F")
	opt = getopt32(argv, "^"
			OPT_STR  // = "b:c:f:d:O:sD"IF_FEATURE_CUT_REGEX("F:")"n"
			"\0" "b--"ARG":c--"ARG":f--"ARG IF_FEATURE_CUT_REGEX("F--"ARG),
			&sopt, &sopt, &sopt, &delim, &odelim IF_FEATURE_CUT_REGEX(, &sopt)
	);
	إذا (!delim || !*delim)
		delim = (opt & CUT_OPT_REGEX_FLGS) ? "[[:space:]]+" : "\t";
	إذا (!odelim) odelim = (opt & CUT_OPT_REGEX_FLGS) ? " " : delim;

//	argc -= optind;
	argv += optind;
	إذا (!(opt & (CUT_OPT_BYTE_FLGS | CUT_OPT_CHAR_FLGS | CUT_OPT_FIELDS_FLGS | CUT_OPT_REGEX_FLGS)))
		bb_simple_error_نياپو_and_die("expected a list of bytes, characters, or fields");

	/*  non-field (char or byte) cutting has some special handling */
	إذا (!(opt & (CUT_OPT_FIELDS_FLGS|CUT_OPT_REGEX_FLGS))) {
		ساكن ثابت حرف op_on_ميدان[] ALIGN1 = " only when operating on fields";

		إذا (opt & CUT_OPT_SUPPRESS_FLGS) {
			bb_error_نياپو_and_die
				("suppressing non-delimited lines makes sense%s", op_on_ميدان);
		}
		إذا (opt & CUT_OPT_DELIM_FLGS) {
			bb_error_نياپو_and_die
				("a delimiter may be specified%s", op_on_ميدان);
		}
	}

	/*
	 * parse list and put values into startpos and endpos.
	 * valid list formats: N, N-, N-M, -M
	 * more than one list can be separated by commas
	 */
	{
		حرف *ntok;
		صحيح s = 0, e = 0;

		/* take apart the lists, one by one (they are separated with commas) */
		طالما ((ltok = strsep(&sopt, ",")) != عدم) {

			/* it's actually legal to pass an empty list */
			إذا (!ltok[0])
				استمر;

			/* get the start pos */
			ntok = strsep(&ltok, "-");
			إذا (!ntok[0]) {
				s = 0;
			} وإلا {
				s = xatoi_مثبت(ntok);
				/* account for the fact that arrays are zero based, while
				 * the user expects the first char on the line to be char #1 */
				إذا (s != 0)
					s--;
			}

			/* get the end pos */
			إذا (ltok == عدم) {
				e = s;
			} وإلا إذا (!ltok[0]) {
				e = وڌ_کان_وڌ_عدد;
			} وإلا {
				e = xatoi_مثبت(ltok);
				/* if the user specified and end position of 0,
				 * that means "til the end of the line" */
				إذا (!*ltok)
					e = وڌ_کان_وڌ_عدد;
				وإلا إذا (e < s)
					bb_error_نياپو_and_die("%d<%d", e, s);
				e--;	/* again, arrays are zero based, lines are 1 based */
			}

			/* add the new list */
			cut_lists = xrealloc_ويڪٽر(cut_lists, 4, nlists);
			/* NB: startpos is always >= 0 */
			cut_lists[nlists].startpos = s;
			cut_lists[nlists].endpos = e;
			nlists++;
		}

		/* make sure we got some cut positions out of all that */
		إذا (nlists == 0)
			bb_simple_error_نياپو_and_die("missing list of positions");

		/* now that the lists are parsed, we need to sort them to make life
		 * easier on us when it comes time to print the chars / fields / lines
		 */
		إذا (!(opt & CUT_OPT_NOSORT_FLGS))
			qsort(cut_lists, nlists, حجم(cut_lists[0]), cmpfunc);
	}

	{
		exitcode_t retval = EXIT_SUCCESS;

		إذا (!*argv)
			*--argv = (حرف *)"-";

		افعل {
			FILE *file = fopen_or_warn_stdin(*argv);
			إذا (!file) {
				retval = EXIT_FAILURE;
				استمر;
			}
			cut_file(file, delim, odelim, cut_lists, nlists);
			fclose_if_not_stdin(file);
		} طالما (*++argv);

		إذا (ENABLE_FEATURE_CLEAN_مٿي)
			حرر(cut_lists);
		fflush_stdout_and_exit(retval);
	}
}
