/* vi: set sw=4 ts=4: */
/*
 * SuS3 compliant sort implementation for busybox
 *
 * Copyright (C) 2004 by Rob Landley <rob@landley.net>
 *
 * MAINTAINER: Rob Landley <rob@landley.net>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 *
 * See SuS3 sort standard at:
 * http://www.opengroup.org/onlinepubs/007904975/utilities/sort.html
 */
//config:config SORT
//config:	bool "sort (8.1 kb)"
//config:	default y
//config:	help
//config:	sort is used to sort lines of text in specified files.
//config:
//config:config FEATURE_SORT_BIG
//config:	bool "Full SuSv3 compliant sort (support -ktcbdfioghM)"
//config:	default y
//config:	depends on SORT
//config:	help
//config:	Without this, sort only supports -rusz, and an integer version
//config:	of -n. Selecting this adds sort keys, floating point support, and
//config:	more. This adds a little over 3k to a nonstatic build on x86.
//config:
//config:	The SuSv3 sort standard is available at:
//config:	http://www.opengroup.org/onlinepubs/007904975/utilities/sort.html
//config:
//config:config FEATURE_SORT_OPTIMIZE_MEMORY
//config:	bool "Use less memory (but might be slower)"
//config:	default n   # defaults to N since we are size-paranoid tribe
//config:	depends on SORT
//config:	help
//config:	Attempt to use less memory (by storing only one copy
//config:	of duplicated lines, and such). Useful if you work on huge files.

//applet:IF_SORT(APPLET_NOEXEC(sort, sort, BB_DIR_USR_BIN, BB_SUID_DROP, sort))

//kbuild:lib-$(CONFIG_SORT) += sort.o

//usage:#define sort_trivial_usage
//usage:       "[-nru"
//usage:	IF_FEATURE_SORT_BIG("ghMcszbdfiokt] [-o FILE] [-k START[.OFS][OPTS][,END[.OFS][OPTS]] [-t CHAR")
//usage:       "] [FILE]..."
//usage:#define sort_full_usage "\n\n"
//usage:       "Sort lines of text\n"
//usage:	IF_FEATURE_SORT_BIG(
//usage:     "\n	-o FILE	Output to FILE"
//usage:     "\n	-c	Check whether input is sorted"
//usage:     "\n	-b	Ignore leading blanks"
//usage:     "\n	-f	Ignore case"
//usage:     "\n	-i	Ignore unprintable characters"
//usage:     "\n	-d	Dictionary order (blank or alphanumeric only)"
//usage:	)
//-h, --human-numeric-sort: compare human readable numbers (e.g., 2K 1G)
//usage:     "\n	-n	Sort numbers"
//usage:	IF_FEATURE_SORT_BIG(
//usage:     "\n	-g	General numerical sort"
//usage:     "\n	-h	Sort human readable numbers (2K 1G)"
//usage:     "\n	-M	Sort month"
//usage:     "\n	-V	Sort version"
//usage:     "\n	-t CHAR	Field separator"
//usage:     "\n	-k N[,M] Sort by Nth field"
//usage:	)
//usage:     "\n	-r	Reverse sort order"
//usage:     "\n	-s	Stable (don't sort ties alphabetically)"
//usage:     "\n	-u	Suppress duplicate lines"
//usage:     "\n	-z	NUL terminated input and output"
///////:     "\n	-m	Ignored for GNU compatibility"
///////:     "\n	-S BUFSZ Ignored for GNU compatibility"
///////:     "\n	-T TMPDIR Ignored for GNU compatibility"
//usage:
//usage:#define sort_example_usage
//usage:       "$ echo -e \"e\\nf\\nb\\nd\\nc\\na\" | sort\n"
//usage:       "a\n"
//usage:       "b\n"
//usage:       "c\n"
//usage:       "d\n"
//usage:       "e\n"
//usage:       "f\n"
//usage:	IF_FEATURE_SORT_BIG(
//usage:		"$ echo -e \"c 3\\nb 2\\nd 2\" | $SORT -k 2,2n -k 1,1r\n"
//usage:		"d 2\n"
//usage:		"b 2\n"
//usage:		"c 3\n"
//usage:	)
//usage:       ""

#అన్తర్భావ "libbb.h"

/* These are sort types */
గణన {
	FLAG_n  = 1 << 0,       /* Numeric sort */
	FLAG_g  = 1 << 1,       /* Sort using strtod() */
	FLAG_h  = 1 << 2,       /* Sort using strtod(), plus KMGT suffixes */
	FLAG_M  = 1 << 3,       /* Sort date */
	FLAG_V  = 1 << 4,       /* Sort version */
/* ucsz apply to root level only, not keys.  b at root level implies bb */
	FLAG_u  = 1 << 5,       /* Unique */
	FLAG_c  = 1 << 6,       /* Check: no output, exit(!ordered) */
	FLAG_s  = 1 << 7,       /* Stable sort, no ascii fallback at end */
	FLAG_z  = 1 << 8,       /* Input and output is NUL terminated, not \n */
/* These can be applied to search keys, the previous four can't */
	FLAG_b  = 1 << 9,       /* Ignore leading blanks */
	FLAG_r  = 1 << 10,      /*తిరగేయి*/
	FLAG_d  = 1 << 11,      /* Ignore !(isalnum()|isspace()) */
	FLAG_f  = 1 << 12,      /* Force uppercase */
	FLAG_i  = 1 << 13,      /* Ignore !isprint() */
	FLAG_m  = 1 << 14,      /* ignored: merge already sorted files; do not sort */
	FLAG_S  = 1 << 15,      /* ignored: -S, --buffer-size=SIZE */
	FLAG_T  = 1 << 16,      /* ignored: -T, --temporary-directory=DIR */
	FLAG_o  = 1 << 17,
	FLAG_k  = 1 << 18,
	FLAG_t  = 1 << 19,
	FLAG_bb = 0x80000000,   /* Ignore trailing blanks  */
	FLAG_no_tie_break = 0x40000000,
};

స్థిర స్థిరరాశి అక్షరం క్రమీకరించు_opt_str[] ALIGN1 = "^"
			"nghMVucszbrdfimS:T:o:k:*t:"
			"\0" "o--o:t--t"/*-t, -o: at most one of each*/;
/*
 * OPT_STR must not be string literal, needs to have stable address:
 * code uses "strchr(OPT_STR,c) - OPT_STR" idiom.
 */
#నిర్వచించు OPT_STR (క్రమీకరించు_opt_str + 1)

#ఒకవేళ ENABLE_FEATURE_క్రమీకరించు_BIG
స్థిర అక్షరం కీలకం_separator;

స్థిర నిర్మాణం క్రమీకరించు_కీలకం {
	నిర్మాణం క్రమీకరించు_కీలకం *తదుపరి_కీలకం;  /* linked list */
	గుర్తులేని range[4];          /* start word, start char, end word, end char */
	గుర్తులేని flags;
} *కీలకం_list;


/* This is a NOEXEC applet. Be very careful! */


స్థిర అక్షరం *get_కీలకం(అక్షరం *str, నిర్మాణం క్రమీకరించు_కీలకం *కీలకం, పూర్ణాంకం flags)
{
	పూర్ణాంకం ప్రారంభం = ప్రారంభం; /* for compiler */
	పూర్ణాంకం చివర;
	పూర్ణాంకం len, j;
	గుర్తులేని i;

	/* Special case whole string, so we don't have to make a copy */
	ఒకవేళ (కీలకం->range[0] == 1 && !కీలకం->range[1] && !కీలకం->range[2] && !కీలకం->range[3]
	 && !(flags & (FLAG_b | FLAG_d | FLAG_f | FLAG_i | FLAG_bb))
	) {
		తిరిగి str;
	}

	/* Find start of key on first pass, end on second pass */
	len = అక్షరశ్రేణి_పొడవు(str);
	కొరకు (j = 0; j < 2; j++) {
		ఒకవేళ (!కీలకం->range[2*j])
			చివర = len;
		/* Loop through fields */
		లేకపోతే {
			గుర్తులేని అక్షరం ch = 0;

			చివర = 0;
			కొరకు (i = 1; i < కీలకం->range[2*j] + j; i++) {
				ఒకవేళ (కీలకం_separator) {
					/* Skip body of key and separator */
					అయితే ((ch = str[చివర]) != '\0') {
							చివర++;
						ఒకవేళ (ch == కీలకం_separator)
							ఆపు;
					}
				} లేకపోతే {
					/* Skip leading blanks */
					అయితే (ఖాళీనా(str[చివర]))
						చివర++;
					/* Skip body of key */
					అయితే (str[చివర] != '\0') {
						ఒకవేళ (ఖాళీనా(str[చివర]))
							ఆపు;
						చివర++;
					}
				}
			}
			/* Remove last delim: "abc:def:" => "abc:def" */
			ఒకవేళ (j && ch) {
				//if (str[end-1] != key_separator)
				//  bb_error_msg(_and_die("BUG! "
				//  "str[start:%d,end:%d]:'%.*s'",
				//  start, end, (int)(end-start), &str[start]);
				చివర--;
			}
		}
		ఒకవేళ (!j) ప్రారంభం = చివర;
	}
	/* Strip leading whitespace if necessary */
	ఒకవేళ (flags & FLAG_b)
		/* not using skip_whitespace() for speed */
		అయితే (ఖాళీనా(str[ప్రారంభం])) ప్రారంభం++;
	/* Strip trailing whitespace if necessary */
	ఒకవేళ (flags & FLAG_bb)
		అయితే (చివర > ప్రారంభం && ఖాళీనా(str[చివర-1])) చివర--;
	/* -kSTART,N.ENDCHAR: honor ENDCHAR (1-based) */
	ఒకవేళ (కీలకం->range[3]) {
		చివర = కీలకం->range[3];
		ఒకవేళ (చివర > len) చివర = len;
	}
	/* -kN.STARTCHAR[,...]: honor STARTCHAR (1-based) */
	ఒకవేళ (కీలకం->range[1]) {
		ప్రారంభం += కీలకం->range[1] - 1;
		ఒకవేళ (ప్రారంభం > len) ప్రారంభం = len;
	}
	/* Make the copy */
	ఒకవేళ (చివర < ప్రారంభం)
		చివర = ప్రారంభం;
	str = xstrndup(str+ప్రారంభం, చివర-ప్రారంభం);
	/* Handle -d */
	ఒకవేళ (flags & FLAG_d) {
		కొరకు (ప్రారంభం = చివర = 0; str[చివర]; చివర++)
			ఒకవేళ (ఖాళీనా(str[చివర]) || అక్షరమా_అంకెనా(str[చివర]))
				str[ప్రారంభం++] = str[చివర];
		str[ప్రారంభం] = '\0';
	}
	/* Handle -i */
	ఒకవేళ (flags & FLAG_i) {
		కొరకు (ప్రారంభం = చివర = 0; str[చివర]; చివర++)
			ఒకవేళ (isprint_asciionly(str[చివర]))
				str[ప్రారంభం++] = str[చివర];
		str[ప్రారంభం] = '\0';
	}
	/* Handle -f */
	ఒకవేళ (flags & FLAG_f)
		కొరకు (i = 0; str[i]; i++)
			str[i] = పెద్ద_అక్షరం(str[i]);

	తిరిగి str;
}

స్థిర నిర్మాణం క్రమీకరించు_కీలకం *కూడు_కీలకం(శూన్య)
{
	నిర్మాణం క్రమీకరించు_కీలకం **pkey = &కీలకం_list;
	అయితే (*pkey)
		pkey = &((*pkey)->తదుపరి_కీలకం);
	తిరిగి *pkey = xzalloc(పరిమాణం(నిర్మాణం క్రమీకరించు_కీలకం));
}

#నిర్వచించు GET_పంక్తి(fp) \
	((option_mask32 & FLAG_z) \
	? bb_get_chunk_from_file(fp, ఖాళీ) \
	: xmalloc_fgetline(fp))
#లేకపోతే
#నిర్వచించు GET_పంక్తి(fp) xmalloc_fgetline(fp)
#అయితే_ముగింపు

#ఒకవేళ ENABLE_FEATURE_క్రమీకరించు_BIG
స్థిర పూర్ణాంకం scale_suffix(స్థిరరాశి అక్షరం *తోక)
{
	స్థిర స్థిరరాశి అక్షరం suffix[] ALIGN1 = "kmgtpezy";
	స్థిరరాశి అక్షరం *s;
	పూర్ణాంకం n;

	ఒకవేళ (!తోక[0])
		తిరిగి -1;
	s = strchr(suffix, తోక[0] | 0x20);
	ఒకవేళ (!s)
		తిరిగి -1;
	n = s - suffix;
	ఒకవేళ (n != 0 && తోక[0] >= 'a')
		తిరిగి -1; /* mg... not accepted, only MG... */
	తిరిగి n;
}
#అయితే_ముగింపు

/* Iterate through keys list and perform comparisons */
స్థిర పూర్ణాంకం compare_కీలకాలు(స్థిరరాశి శూన్య *xarg, స్థిరరాశి శూన్య *yarg)
{
	పూర్ణాంకం flags = option_mask32, retval = 0;
	అక్షరం *x, *y;

#ఒకవేళ ENABLE_FEATURE_క్రమీకరించు_BIG
	నిర్మాణం క్రమీకరించు_కీలకం *కీలకం;

	కొరకు (కీలకం = కీలకం_list; !retval && కీలకం; కీలకం = కీలకం->తదుపరి_కీలకం) {
		flags = కీలకం->flags ? కీలకం->flags : option_mask32;
		/* Chop out and modify key chunks, handling -dfib */
		x = get_కీలకం(*(అక్షరం **)xarg, కీలకం, flags);
		y = get_కీలకం(*(అక్షరం **)yarg, కీలకం, flags);
#లేకపోతే
	/* This curly bracket serves no purpose but to match the nesting
	 * level of the for () loop we're not using */
	{
		x = *(అక్షరం **)xarg;
		y = *(అక్షరం **)yarg;
#అయితే_ముగింపు
		/* Perform actual comparison */
		ఎంపిక (flags & (FLAG_n | FLAG_g | FLAG_h | FLAG_M | FLAG_V)) {
		అప్రమేయ:
			bb_simple_error_సందేశం_and_die("unknown sort type");
			ఆపు;
#ఒకవేళ defined(HAVE_STRVERSCMP) && HAVE_STRVERSCMP == 1
		స్థితి FLAG_V:
			retval = strverscmp(x, y);
			ఆపు;
#అయితే_ముగింపు
		/* Ascii sort */
		స్థితి 0:
#ఒకవేళ ENABLE_LOCALE_SUPPORT
			retval = strcoll(x, y);
#లేకపోతే
			retval = strcmp(x, y);
#అయితే_ముగింపు
			ఆపు;
#ఒకవేళ ENABLE_FEATURE_క్రమీకరించు_BIG
		స్థితి FLAG_g:
		స్థితి FLAG_h: {
			అక్షరం *xx, *yy;
//TODO: needs setlocale(LC_NUMERIC, "C")?
			ద్విగుణ dx = strtod(x, &xx);
			ద్విగుణ dy = strtod(y, &yy);
			/* not numbers < NaN < -infinity < numbers < +infinity) */
			ఒకవేళ (x == xx)
				retval = (y == yy ? 0 : -1);
			లేకపోతే ఒకవేళ (y == yy)
				retval = 1;
			/* Check for isnan */
			లేకపోతే ఒకవేళ (dx != dx)
				retval = (dy != dy) ? 0 : -1;
			లేకపోతే ఒకవేళ (dy != dy)
				retval = 1;
			లేకపోతే {
				ఒకవేళ (flags & FLAG_h) {
					పూర్ణాంకం xs = scale_suffix(xx);
					పూర్ణాంకం ys = scale_suffix(yy);
					ఒకవేళ (xs != ys) {
						retval = xs - ys;
						ఆపు;
					}
				}
				/* Check for infinity.  Could underflow, but it avoids libm. */
				ఒకవేళ (1.0 / dx == 0.0) {
					ఒకవేళ (dx < 0)
						retval = (1.0 / dy == 0.0 && dy < 0) ? 0 : -1;
					లేకపోతే
						retval = (1.0 / dy == 0.0 && dy > 0) ? 0 : 1;
				} లేకపోతే ఒకవేళ (1.0 / dy == 0.0)
					retval = (dy < 0) ? 1 : -1;
				లేకపోతే
					retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			}
			ఆపు;
		}
		స్థితి FLAG_M: {
			నిర్మాణం tm thyme;
			పూర్ణాంకం dx;
			అక్షరం *xx, *yy;

			xx = strptime(skip_whitespace(x), "%b", &thyme);
			dx = thyme.tm_mon;
			yy = strptime(skip_whitespace(y), "%b", &thyme);
			ఒకవేళ (!xx)
				retval = (!yy) ? 0 : -1;
			లేకపోతే ఒకవేళ (!yy)
				retval = 1;
			లేకపోతే
				retval = dx - thyme.tm_mon;
			ఆపు;
		}
		/* Full floating point version of -n */
		స్థితి FLAG_n: {
			ద్విగుణ dx = atof(x);
			ద్విగుణ dy = atof(y);
			retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			ఆపు;
		}
		} /* switch */
		/* Free key copies. */
		ఒకవేళ (x != *(అక్షరం **)xarg) విడుదల(x);
		ఒకవేళ (y != *(అక్షరం **)yarg) విడుదల(y);
		/* if (retval) break; - done by for () anyway */
#లేకపోతే
		/* Integer version of -n for tiny systems */
		స్థితి FLAG_n:
			retval = atoi(x) - atoi(y);
			ఆపు;
		} /* switch */
#అయితే_ముగింపు
	} /* for */

	ఒకవేళ (retval == 0) {
		/* So far lines are "the same" */

		ఒకవేళ (option_mask32 & FLAG_s) {
			/* "Stable sort": later line is "greater than",
			 * IOW: do not allow qsort() to swap equal lines.
			 */
			uint32_t *p32;
			uint32_t x32, y32;
			అక్షరం *పంక్తి;
			గుర్తులేని len;

			పంక్తి = *(అక్షరం**)xarg;
			len = (అక్షరశ్రేణి_పొడవు(పంక్తి) + 4) & (~3u);
			p32 = (శూన్య*)(పంక్తి + len);
			x32 = *p32;
			పంక్తి = *(అక్షరం**)yarg;
			len = (అక్షరశ్రేణి_పొడవు(పంక్తి) + 4) & (~3u);
			p32 = (శూన్య*)(పంక్తి + len);
			y32 = *p32;

			/* If x > y, 1, else -1 */
			retval = (x32 > y32) * 2 - 1;
			/* Here, -r has no effect! */
			తిరిగి retval;
		}
		ఒకవేళ (!(option_mask32 & FLAG_no_tie_break)) {
			/* fallback sort */
			flags = option_mask32;
			retval = strcmp(*(అక్షరం **)xarg, *(అక్షరం **)yarg);
		}
	}

	ఒకవేళ (flags & FLAG_r)
		తిరిగి -retval;

	తిరిగి retval;
}

#ఒకవేళ ENABLE_FEATURE_క్రమీకరించు_BIG
స్థిర గుర్తులేని str2u(అక్షరం **str)
{
	గుర్తులేని దీర్ఘ lu;
	ఒకవేళ (!అంకెనా((*str)[0]))
		bb_simple_error_సందేశం_and_die("bad field specification");
	lu = strtoul(*str, str, 10);
	ఒకవేళ ((పరిమాణం(దీర్ఘ) > పరిమాణం(పూర్ణాంకం) && lu > గరిష్ఠ_పూర్ణాంకం) || !lu)
		bb_simple_error_సందేశం_and_die("bad field specification");
	తిరిగి lu;
}
#అయితే_ముగింపు

పూర్ణాంకం క్రమీకరించు_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం క్రమీకరించు_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	అక్షరం **పంక్తులు;
	అక్షరం *str_ignored, *str_o, *str_t;
	llist_t *lst_k = ఖాళీ;
	పూర్ణాంకం i;
	పూర్ణాంకం linecount;
	గుర్తులేని opts;
#ఒకవేళ ENABLE_FEATURE_క్రమీకరించు_OPTIMIZE_జ్ఞాపకం
	bool can_drop_dups;
	కొలత_t prev_len = 0;
	అక్షరం *prev_పంక్తి = (అక్షరం*) "";
	/* Postpone optimizing if the input is small, < 16k lines:
	 * even just free()ing duplicate lines takes time.
	 */
	కొలత_t లెక్క_గమ్యం_optimize_dups = 0x3fff;
#అయితే_ముగింపు

	xfunc_error_retval = 2;

	/* Parse command line options */
	opts = getopt32(argv,
			క్రమీకరించు_opt_str,
			&str_ignored, &str_ignored, &str_o, &lst_k, &str_t
	);
#ఒకవేళ ENABLE_FEATURE_క్రమీకరించు_OPTIMIZE_జ్ఞాపకం
	/* Can drop dups only if -u but no "complicating" options,
	 * IOW: if we do a full line compares. Safe options:
	 * -o FILE Output to FILE
	 * -z	   Lines are terminated by NUL, not newline
	 * -r	   Reverse sort order
	 * -s      Stable (don't sort ties alphabetically)
	 * Not sure about these:
	 * -b      Ignore leading blanks
	 * -f	   Ignore case
	 * -i	   Ignore unprintable characters
	 * -d	   Dictionary order (blank or alphanumeric only)
	 * -n	   Sort numbers
	 * -g	   General numerical sort
	 * -M	   Sort month
	 */
	can_drop_dups = ((opts & ~(FLAG_o|FLAG_z|FLAG_r|FLAG_s)) == FLAG_u);
	/* Stable sort needs every line to be uniquely allocated,
	 * disable optimization to reuse strings:
	 */
	ఒకవేళ (opts & FLAG_s)
		లెక్క_గమ్యం_optimize_dups = (కొలత_t)-1L;
#అయితే_ముగింపు
	/* global b strips leading and trailing spaces */
	ఒకవేళ (opts & FLAG_b)
		option_mask32 |= FLAG_bb;
#ఒకవేళ ENABLE_FEATURE_క్రమీకరించు_BIG
	ఒకవేళ (opts & FLAG_t) {
		ఒకవేళ (!str_t[0] || str_t[1])
			bb_simple_error_సందేశం_and_die("bad -t parameter");
		కీలకం_separator = str_t[0];
	}
	/* note: below this point we use option_mask32, not opts,
	 * since that reduces register pressure and makes code smaller */

	/* Parse sort key */
	అయితే (lst_k) {
		గణన {
			FLAG_allowed_for_k =
				FLAG_n | /* Numeric sort */
				FLAG_g | /* Sort using strtod() */
				FLAG_h | /* Sort using strtod(), plus KMGT suffixes */
				FLAG_M | /* Sort date */
				FLAG_b | /* Ignore leading blanks */
				FLAG_r | /*తిరగేయి*/
				FLAG_d | /* Ignore !(isalnum()|isspace()) */
				FLAG_f | /* Force uppercase */
				FLAG_i | /* Ignore !isprint() */
			0
		};
		నిర్మాణం క్రమీకరించు_కీలకం *కీలకం = కూడు_కీలకం();
		అక్షరం *str_k = llist_తీయి(&lst_k);

		i = 0; /* i==0 before comma, 1 after (-k3,6) */
		అయితే (*str_k) {
			/* Start of range */
			/* Cannot use bb_strtou - suffix can be a letter */
			కీలకం->range[2*i] = str2u(&str_k);
			ఒకవేళ (*str_k == '.') {
				str_k++;
				కీలకం->range[2*i+1] = str2u(&str_k);
			}
			అయితే (*str_k) {
				పూర్ణాంకం flag;
				స్థిరరాశి అక్షరం *idx;

				ఒకవేళ (*str_k == ',' && !i++) {
					str_k++;
					ఆపు;
				} /* no else needed: fall through to syntax error
					because comma isn't in OPT_STR */
				idx = strchr(OPT_STR, *str_k);
				ఒకవేళ (!idx)
					bb_simple_error_సందేశం_and_die("unknown key option");
				flag = 1 << (idx - OPT_STR);
				ఒకవేళ (flag & ~FLAG_allowed_for_k)
					bb_simple_error_సందేశం_and_die("unknown sort type");
				/* b after ',' means strip _trailing_ space */
				ఒకవేళ (i && flag == FLAG_b)
					flag = FLAG_bb;
				కీలకం->flags |= flag;
				str_k++;
			}
		}
	}
#అయితే_ముగింపు

	/* Open input files and read data */
	argv += optind;
	ఒకవేళ (!*argv)
		*--argv = (అక్షరం*)"-";
	linecount = 0;
	పంక్తులు = ఖాళీ;
	చేయి {
		/* coreutils 6.9 compat: abort on first open error,
		 * do not continue to next file: */
		FILE *fp = xfopen_stdin(*argv);
		కొరకు (;;) {
			అక్షరం *పంక్తి = GET_పంక్తి(fp);
			ఒకవేళ (!పంక్తి)
				ఆపు;

#ఒకవేళ ENABLE_FEATURE_క్రమీకరించు_OPTIMIZE_జ్ఞాపకం
			ఒకవేళ (లెక్క_గమ్యం_optimize_dups != 0)
				లెక్క_గమ్యం_optimize_dups--;
			ఒకవేళ (లెక్క_గమ్యం_optimize_dups == 0) {
				కొలత_t len;
				అక్షరం *new_పంక్తి;

				/* On kernel/linux/arch/ *.[ch] files,
				 * this reduces memory usage by 6%.
				 *  yes | head -99999999 | sort
				 * goes down from 1900Mb to 380 Mb.
				 */
				len = అక్షరశ్రేణి_పొడవు(పంక్తి);
				ఒకవేళ (len <= prev_len) {
					new_పంక్తి = prev_పంక్తి + (prev_len - len);
					ఒకవేళ (strcmp(పంక్తి, new_పంక్తి) == 0) {
						/* it's a tail of the prev line */
						ఒకవేళ (can_drop_dups && prev_len == len) {
							/* it's identical to prev line */
							విడుదల(పంక్తి);
							కొనసాగు;
						}
						విడుదల(పంక్తి);
						పంక్తి = new_పంక్తి;
						/* continue using longer prev_line
						 * for future tail tests.
						 */
						వెళ్ళు skip;
					}
				}
				prev_len = len;
				prev_పంక్తి = పంక్తి;
 skip: ;
			}
#లేకపోతే
//TODO: lighter version which only drops total dups if can_drop_dups == true
#అయితే_ముగింపు
			పంక్తులు = xrealloc_సదిశ(పంక్తులు, 6, linecount);
			పంక్తులు[linecount++] = పంక్తి;
		}
		fclose_if_not_stdin(fp);
	} అయితే (*++argv);

#ఒకవేళ ENABLE_FEATURE_క్రమీకరించు_BIG
	/* If no key, perform alphabetic sort */
	ఒకవేళ (!కీలకం_list)
		కూడు_కీలకం()->range[0] = 1;
	/* Handle -c */
	ఒకవేళ (option_mask32 & FLAG_c) {
		పూర్ణాంకం j = (option_mask32 & FLAG_u) ? -1 : 0;
		కొరకు (i = 1; i < linecount; i++) {
			ఒకవేళ (compare_కీలకాలు(&పంక్తులు[i-1], &పంక్తులు[i]) > j) {
				fprintf(stderr, "Check line %u\n", i);
				తిరిగి EXIT_FAILURE;
			}
		}
		తిరిగి EXIT_SUCCESS;
	}
#అయితే_ముగింపు

	/* For stable sort, store original line position beyond terminating NUL */
	ఒకవేళ (option_mask32 & FLAG_s) {
		కొరకు (i = 0; i < linecount; i++) {
			uint32_t *p32;
			అక్షరం *పంక్తి;
			గుర్తులేని len;

			పంక్తి = పంక్తులు[i];
			len = (అక్షరశ్రేణి_పొడవు(పంక్తి) + 4) & (~3u);
			పంక్తులు[i] = పంక్తి = xrealloc(పంక్తి, len + 4);
			p32 = (శూన్య*)(పంక్తి + len);
			*p32 = i;
		}
		/*option_mask32 |= FLAG_no_tie_break;*/
		/* ^^^redundant: if FLAG_s, compare_keys() does no tie break */
	}

	/* Perform the actual sort */
	qsort(పంక్తులు, linecount, పరిమాణం(పంక్తులు[0]), compare_కీలకాలు);

	/* Handle -u */
	ఒకవేళ (option_mask32 & FLAG_u) {
		పూర్ణాంకం j = 0;
		/* coreutils 6.3 drop lines for which only key is the same:
		 * - disabling last-resort compare, or else compare_keys()
		 * will be the same only for completely identical lines
		 * - disabling -s (same reasons)
		 */
		option_mask32 = (option_mask32 | FLAG_no_tie_break) & (~FLAG_s);
		కొరకు (i = 1; i < linecount; i++) {
			ఒకవేళ (compare_కీలకాలు(&పంక్తులు[j], &పంక్తులు[i]) == 0)
				విడుదల(పంక్తులు[i]);
			లేకపోతే
				పంక్తులు[++j] = పంక్తులు[i];
		}
		ఒకవేళ (linecount)
			linecount = j+1;
	}

	/* Print it */
#ఒకవేళ ENABLE_FEATURE_క్రమీకరించు_BIG
	/* Open output file _after_ we read all input ones */
	ఒకవేళ (option_mask32 & FLAG_o)
		xmove_fd(xopen(str_o, O_WRONLY|O_CREAT|O_TRUNC), STDOUT_FILENO);
#అయితే_ముగింపు
	{
		పూర్ణాంకం ch = (option_mask32 & FLAG_z) ? '\0' : '\n';
		కొరకు (i = 0; i < linecount; i++)
			రాయి("%s%c", పంక్తులు[i], ch);
	}

	fflush_stdout_and_exit_SUCCESS();
}
