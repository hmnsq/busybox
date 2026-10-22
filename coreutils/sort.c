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

#अन्तर्भाव "libbb.h"

/* These are sort types */
गणना {
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
	FLAG_r  = 1 << 10,      /* Reverse */
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

स्थिर ध्रुव वर्ण क्रमय_opt_str[] ALIGN1 = "^"
			"nghMVucszbrdfimS:T:o:k:*t:"
			"\0" "o--o:t--t"/*-t, -o: at most one of each*/;
/*
 * OPT_STR must not be string literal, needs to have stable address:
 * code uses "strchr(OPT_STR,c) - OPT_STR" idiom.
 */
#परिभाषय OPT_STR (क्रमय_opt_str + 1)

#यदि ENABLE_FEATURE_क्रमय_BIG
स्थिर वर्ण कुञ्चिका_separator;

स्थिर संरचना क्रमय_कुञ्चिका {
	संरचना क्रमय_कुञ्चिका *अग्र_कुञ्चिका;  /* linked list */
	अचिह्नित range[4];          /* start word, start char, end word, end char */
	अचिह्नित flags;
} *कुञ्चिका_list;


/* This is a NOEXEC applet. Be very careful! */


स्थिर वर्ण *get_कुञ्चिका(वर्ण *str, संरचना क्रमय_कुञ्चिका *कुञ्चिका, पूर्णाङ्क flags)
{
	पूर्णाङ्क start = start; /* for compiler */
	पूर्णाङ्क अन्तः;
	पूर्णाङ्क len, j;
	अचिह्नित i;

	/* Special case whole string, so we don't have to make a copy */
	यदि (कुञ्चिका->range[0] == 1 && !कुञ्चिका->range[1] && !कुञ्चिका->range[2] && !कुञ्चिका->range[3]
	 && !(flags & (FLAG_b | FLAG_d | FLAG_f | FLAG_i | FLAG_bb))
	) {
		प्रत्यागम str;
	}

	/* Find start of key on first pass, end on second pass */
	len = वाक्यदैर्घ्यम्(str);
	प्रति (j = 0; j < 2; j++) {
		यदि (!कुञ्चिका->range[2*j])
			अन्तः = len;
		/* Loop through fields */
		अन्यथा {
			अचिह्नित वर्ण ch = 0;

			अन्तः = 0;
			प्रति (i = 1; i < कुञ्चिका->range[2*j] + j; i++) {
				यदि (कुञ्चिका_separator) {
					/* Skip body of key and separator */
					यावत् ((ch = str[अन्तः]) != '\0') {
							अन्तः++;
						यदि (ch == कुञ्चिका_separator)
							भङ्ग;
					}
				} अन्यथा {
					/* Skip leading blanks */
					यावत् (रिक्तम्_वा(str[अन्तः]))
						अन्तः++;
					/* Skip body of key */
					यावत् (str[अन्तः] != '\0') {
						यदि (रिक्तम्_वा(str[अन्तः]))
							भङ्ग;
						अन्तः++;
					}
				}
			}
			/* Remove last delim: "abc:def:" => "abc:def" */
			यदि (j && ch) {
				//if (str[end-1] != key_separator)
				//  bb_error_msg(_and_die("BUG! "
				//  "str[start:%d,end:%d]:'%.*s'",
				//  start, end, (int)(end-start), &str[start]);
				अन्तः--;
			}
		}
		यदि (!j) start = अन्तः;
	}
	/* Strip leading whitespace if necessary */
	यदि (flags & FLAG_b)
		/* not using skip_whitespace() for speed */
		यावत् (रिक्तम्_वा(str[start])) start++;
	/* Strip trailing whitespace if necessary */
	यदि (flags & FLAG_bb)
		यावत् (अन्तः > start && रिक्तम्_वा(str[अन्तः-1])) अन्तः--;
	/* -kSTART,N.ENDCHAR: honor ENDCHAR (1-based) */
	यदि (कुञ्चिका->range[3]) {
		अन्तः = कुञ्चिका->range[3];
		यदि (अन्तः > len) अन्तः = len;
	}
	/* -kN.STARTCHAR[,...]: honor STARTCHAR (1-based) */
	यदि (कुञ्चिका->range[1]) {
		start += कुञ्चिका->range[1] - 1;
		यदि (start > len) start = len;
	}
	/* Make the copy */
	यदि (अन्तः < start)
		अन्तः = start;
	str = xstrndup(str+start, अन्तः-start);
	/* Handle -d */
	यदि (flags & FLAG_d) {
		प्रति (start = अन्तः = 0; str[अन्तः]; अन्तः++)
			यदि (रिक्तम्_वा(str[अन्तः]) || अक्षरम्_अङ्कः_वा(str[अन्तः]))
				str[start++] = str[अन्तः];
		str[start] = '\0';
	}
	/* Handle -i */
	यदि (flags & FLAG_i) {
		प्रति (start = अन्तः = 0; str[अन्तः]; अन्तः++)
			यदि (isprint_asciionly(str[अन्तः]))
				str[start++] = str[अन्तः];
		str[start] = '\0';
	}
	/* Handle -f */
	यदि (flags & FLAG_f)
		प्रति (i = 0; str[i]; i++)
			str[i] = महाक्षरम्(str[i]);

	प्रत्यागम str;
}

स्थिर संरचना क्रमय_कुञ्चिका *सङ्कलय_कुञ्चिका(शून्य)
{
	संरचना क्रमय_कुञ्चिका **pkey = &कुञ्चिका_list;
	यावत् (*pkey)
		pkey = &((*pkey)->अग्र_कुञ्चिका);
	प्रत्यागम *pkey = xzalloc(परिमाण(संरचना क्रमय_कुञ्चिका));
}

#परिभाषय GET_पङ्क्तिः(fp) \
	((option_mask32 & FLAG_z) \
	? bb_get_chunk_from_file(fp, रिक्त) \
	: xmalloc_fgetline(fp))
#अन्यथा
#परिभाषय GET_पङ्क्तिः(fp) xmalloc_fgetline(fp)
#यदि_अन्तः

#यदि ENABLE_FEATURE_क्रमय_BIG
स्थिर पूर्णाङ्क scale_suffix(ध्रुव वर्ण *पुच्छ)
{
	स्थिर ध्रुव वर्ण suffix[] ALIGN1 = "kmgtpezy";
	ध्रुव वर्ण *s;
	पूर्णाङ्क n;

	यदि (!पुच्छ[0])
		प्रत्यागम -1;
	s = strchr(suffix, पुच्छ[0] | 0x20);
	यदि (!s)
		प्रत्यागम -1;
	n = s - suffix;
	यदि (n != 0 && पुच्छ[0] >= 'a')
		प्रत्यागम -1; /* mg... not accepted, only MG... */
	प्रत्यागम n;
}
#यदि_अन्तः

/* Iterate through keys list and perform comparisons */
स्थिर पूर्णाङ्क compare_कुञ्चिकाः(ध्रुव शून्य *xarg, ध्रुव शून्य *yarg)
{
	पूर्णाङ्क flags = option_mask32, retval = 0;
	वर्ण *x, *y;

#यदि ENABLE_FEATURE_क्रमय_BIG
	संरचना क्रमय_कुञ्चिका *कुञ्चिका;

	प्रति (कुञ्चिका = कुञ्चिका_list; !retval && कुञ्चिका; कुञ्चिका = कुञ्चिका->अग्र_कुञ्चिका) {
		flags = कुञ्चिका->flags ? कुञ्चिका->flags : option_mask32;
		/* Chop out and modify key chunks, handling -dfib */
		x = get_कुञ्चिका(*(वर्ण **)xarg, कुञ्चिका, flags);
		y = get_कुञ्चिका(*(वर्ण **)yarg, कुञ्चिका, flags);
#अन्यथा
	/* This curly bracket serves no purpose but to match the nesting
	 * level of the for () loop we're not using */
	{
		x = *(वर्ण **)xarg;
		y = *(वर्ण **)yarg;
#यदि_अन्तः
		/* Perform actual comparison */
		विकल्प (flags & (FLAG_n | FLAG_g | FLAG_h | FLAG_M | FLAG_V)) {
		स्वभाव:
			bb_simple_error_सन्देशः_and_die("unknown sort type");
			भङ्ग;
#यदि defined(HAVE_STRVERSCMP) && HAVE_STRVERSCMP == 1
		पक्ष FLAG_V:
			retval = strverscmp(x, y);
			भङ्ग;
#यदि_अन्तः
		/* Ascii sort */
		पक्ष 0:
#यदि ENABLE_LOCALE_SUPPORT
			retval = strcoll(x, y);
#अन्यथा
			retval = वाक्यं_तुलय(x, y);
#यदि_अन्तः
			भङ्ग;
#यदि ENABLE_FEATURE_क्रमय_BIG
		पक्ष FLAG_g:
		पक्ष FLAG_h: {
			वर्ण *xx, *yy;
//TODO: needs setlocale(LC_NUMERIC, "C")?
			द्विगुण dx = strtod(x, &xx);
			द्विगुण dy = strtod(y, &yy);
			/* not numbers < NaN < -infinity < numbers < +infinity) */
			यदि (x == xx)
				retval = (y == yy ? 0 : -1);
			अन्यथा यदि (y == yy)
				retval = 1;
			/* Check for isnan */
			अन्यथा यदि (dx != dx)
				retval = (dy != dy) ? 0 : -1;
			अन्यथा यदि (dy != dy)
				retval = 1;
			अन्यथा {
				यदि (flags & FLAG_h) {
					पूर्णाङ्क xs = scale_suffix(xx);
					पूर्णाङ्क ys = scale_suffix(yy);
					यदि (xs != ys) {
						retval = xs - ys;
						भङ्ग;
					}
				}
				/* Check for infinity.  Could underflow, but it avoids libm. */
				यदि (1.0 / dx == 0.0) {
					यदि (dx < 0)
						retval = (1.0 / dy == 0.0 && dy < 0) ? 0 : -1;
					अन्यथा
						retval = (1.0 / dy == 0.0 && dy > 0) ? 0 : 1;
				} अन्यथा यदि (1.0 / dy == 0.0)
					retval = (dy < 0) ? 1 : -1;
				अन्यथा
					retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			}
			भङ्ग;
		}
		पक्ष FLAG_M: {
			संरचना tm thyme;
			पूर्णाङ्क dx;
			वर्ण *xx, *yy;

			xx = strptime(skip_whitespace(x), "%b", &thyme);
			dx = thyme.tm_mon;
			yy = strptime(skip_whitespace(y), "%b", &thyme);
			यदि (!xx)
				retval = (!yy) ? 0 : -1;
			अन्यथा यदि (!yy)
				retval = 1;
			अन्यथा
				retval = dx - thyme.tm_mon;
			भङ्ग;
		}
		/* Full floating point version of -n */
		पक्ष FLAG_n: {
			द्विगुण dx = atof(x);
			द्विगुण dy = atof(y);
			retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			भङ्ग;
		}
		} /* switch */
		/* Free key copies. */
		यदि (x != *(वर्ण **)xarg) मुक्त(x);
		यदि (y != *(वर्ण **)yarg) मुक्त(y);
		/* if (retval) break; - done by for () anyway */
#अन्यथा
		/* Integer version of -n for tiny systems */
		पक्ष FLAG_n:
			retval = atoi(x) - atoi(y);
			भङ्ग;
		} /* switch */
#यदि_अन्तः
	} /* for */

	यदि (retval == 0) {
		/* So far lines are "the same" */

		यदि (option_mask32 & FLAG_s) {
			/* "Stable sort": later line is "greater than",
			 * IOW: do not allow qsort() to swap equal lines.
			 */
			uint32_t *p32;
			uint32_t x32, y32;
			वर्ण *पङ्क्तिः;
			अचिह्नित len;

			पङ्क्तिः = *(वर्ण**)xarg;
			len = (वाक्यदैर्घ्यम्(पङ्क्तिः) + 4) & (~3u);
			p32 = (शून्य*)(पङ्क्तिः + len);
			x32 = *p32;
			पङ्क्तिः = *(वर्ण**)yarg;
			len = (वाक्यदैर्घ्यम्(पङ्क्तिः) + 4) & (~3u);
			p32 = (शून्य*)(पङ्क्तिः + len);
			y32 = *p32;

			/* If x > y, 1, else -1 */
			retval = (x32 > y32) * 2 - 1;
			/* Here, -r has no effect! */
			प्रत्यागम retval;
		}
		यदि (!(option_mask32 & FLAG_no_tie_break)) {
			/* fallback sort */
			flags = option_mask32;
			retval = वाक्यं_तुलय(*(वर्ण **)xarg, *(वर्ण **)yarg);
		}
	}

	यदि (flags & FLAG_r)
		प्रत्यागम -retval;

	प्रत्यागम retval;
}

#यदि ENABLE_FEATURE_क्रमय_BIG
स्थिर अचिह्नित str2u(वर्ण **str)
{
	अचिह्नित दीर्घ lu;
	यदि (!अङ्कः_वा((*str)[0]))
		bb_simple_error_सन्देशः_and_die("bad field specification");
	lu = strtoul(*str, str, 10);
	यदि ((परिमाण(दीर्घ) > परिमाण(पूर्णाङ्क) && lu > अधिकतम_पूर्णाङ्कः) || !lu)
		bb_simple_error_सन्देशः_and_die("bad field specification");
	प्रत्यागम lu;
}
#यदि_अन्तः

पूर्णाङ्क क्रमय_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क क्रमय_main(पूर्णाङ्क argc अप्रयुक्तम्_PARAM, वर्ण **argv)
{
	वर्ण **पङ्क्तयः;
	वर्ण *str_ignored, *str_o, *str_t;
	llist_t *lst_k = रिक्त;
	पूर्णाङ्क i;
	पूर्णाङ्क linecount;
	अचिह्नित opts;
#यदि ENABLE_FEATURE_क्रमय_OPTIMIZE_स्मृतिः
	bool can_drop_dups;
	आकार_t पूर्व_len = 0;
	वर्ण *पूर्व_पङ्क्तिः = (वर्ण*) "";
	/* Postpone optimizing if the input is small, < 16k lines:
	 * even just free()ing duplicate lines takes time.
	 */
	आकार_t गणनम्_गन्तव्यम्_optimize_dups = 0x3fff;
#यदि_अन्तः

	xfunc_error_retval = 2;

	/* Parse command line options */
	opts = getopt32(argv,
			क्रमय_opt_str,
			&str_ignored, &str_ignored, &str_o, &lst_k, &str_t
	);
#यदि ENABLE_FEATURE_क्रमय_OPTIMIZE_स्मृतिः
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
	यदि (opts & FLAG_s)
		गणनम्_गन्तव्यम्_optimize_dups = (आकार_t)-1L;
#यदि_अन्तः
	/* global b strips leading and trailing spaces */
	यदि (opts & FLAG_b)
		option_mask32 |= FLAG_bb;
#यदि ENABLE_FEATURE_क्रमय_BIG
	यदि (opts & FLAG_t) {
		यदि (!str_t[0] || str_t[1])
			bb_simple_error_सन्देशः_and_die("bad -t parameter");
		कुञ्चिका_separator = str_t[0];
	}
	/* note: below this point we use option_mask32, not opts,
	 * since that reduces register pressure and makes code smaller */

	/* Parse sort key */
	यावत् (lst_k) {
		गणना {
			FLAG_allowed_for_k =
				FLAG_n | /* Numeric sort */
				FLAG_g | /* Sort using strtod() */
				FLAG_h | /* Sort using strtod(), plus KMGT suffixes */
				FLAG_M | /* Sort date */
				FLAG_b | /* Ignore leading blanks */
				FLAG_r | /* Reverse */
				FLAG_d | /* Ignore !(isalnum()|isspace()) */
				FLAG_f | /* Force uppercase */
				FLAG_i | /* Ignore !isprint() */
			0
		};
		संरचना क्रमय_कुञ्चिका *कुञ्चिका = सङ्कलय_कुञ्चिका();
		वर्ण *str_k = llist_निष्कासय(&lst_k);

		i = 0; /* i==0 before comma, 1 after (-k3,6) */
		यावत् (*str_k) {
			/* Start of range */
			/* Cannot use bb_strtou - suffix can be a letter */
			कुञ्चिका->range[2*i] = str2u(&str_k);
			यदि (*str_k == '.') {
				str_k++;
				कुञ्चिका->range[2*i+1] = str2u(&str_k);
			}
			यावत् (*str_k) {
				पूर्णाङ्क flag;
				ध्रुव वर्ण *idx;

				यदि (*str_k == ',' && !i++) {
					str_k++;
					भङ्ग;
				} /* no else needed: fall through to syntax error
					because comma isn't in OPT_STR */
				idx = strchr(OPT_STR, *str_k);
				यदि (!idx)
					bb_simple_error_सन्देशः_and_die("unknown key option");
				flag = 1 << (idx - OPT_STR);
				यदि (flag & ~FLAG_allowed_for_k)
					bb_simple_error_सन्देशः_and_die("unknown sort type");
				/* b after ',' means strip _trailing_ space */
				यदि (i && flag == FLAG_b)
					flag = FLAG_bb;
				कुञ्चिका->flags |= flag;
				str_k++;
			}
		}
	}
#यदि_अन्तः

	/* Open input files and read data */
	argv += optind;
	यदि (!*argv)
		*--argv = (वर्ण*)"-";
	linecount = 0;
	पङ्क्तयः = रिक्त;
	कुरु {
		/* coreutils 6.9 compat: abort on first open error,
		 * do not continue to next file: */
		FILE *fp = xfopen_stdin(*argv);
		प्रति (;;) {
			वर्ण *पङ्क्तिः = GET_पङ्क्तिः(fp);
			यदि (!पङ्क्तिः)
				भङ्ग;

#यदि ENABLE_FEATURE_क्रमय_OPTIMIZE_स्मृतिः
			यदि (गणनम्_गन्तव्यम्_optimize_dups != 0)
				गणनम्_गन्तव्यम्_optimize_dups--;
			यदि (गणनम्_गन्तव्यम्_optimize_dups == 0) {
				आकार_t len;
				वर्ण *new_पङ्क्तिः;

				/* On kernel/linux/arch/ *.[ch] files,
				 * this reduces memory usage by 6%.
				 *  yes | head -99999999 | sort
				 * goes down from 1900Mb to 380 Mb.
				 */
				len = वाक्यदैर्घ्यम्(पङ्क्तिः);
				यदि (len <= पूर्व_len) {
					new_पङ्क्तिः = पूर्व_पङ्क्तिः + (पूर्व_len - len);
					यदि (वाक्यं_तुलय(पङ्क्तिः, new_पङ्क्तिः) == 0) {
						/* it's a tail of the prev line */
						यदि (can_drop_dups && पूर्व_len == len) {
							/* it's identical to prev line */
							मुक्त(पङ्क्तिः);
							अनुवर्त;
						}
						मुक्त(पङ्क्तिः);
						पङ्क्तिः = new_पङ्क्तिः;
						/* continue using longer prev_line
						 * for future tail tests.
						 */
						गच्छ skip;
					}
				}
				पूर्व_len = len;
				पूर्व_पङ्क्तिः = पङ्क्तिः;
 skip: ;
			}
#अन्यथा
//TODO: lighter version which only drops total dups if can_drop_dups == true
#यदि_अन्तः
			पङ्क्तयः = xrealloc_सदिशम्(पङ्क्तयः, 6, linecount);
			पङ्क्तयः[linecount++] = पङ्क्तिः;
		}
		fclose_if_not_stdin(fp);
	} यावत् (*++argv);

#यदि ENABLE_FEATURE_क्रमय_BIG
	/* If no key, perform alphabetic sort */
	यदि (!कुञ्चिका_list)
		सङ्कलय_कुञ्चिका()->range[0] = 1;
	/* Handle -c */
	यदि (option_mask32 & FLAG_c) {
		पूर्णाङ्क j = (option_mask32 & FLAG_u) ? -1 : 0;
		प्रति (i = 1; i < linecount; i++) {
			यदि (compare_कुञ्चिकाः(&पङ्क्तयः[i-1], &पङ्क्तयः[i]) > j) {
				fprintf(stderr, "Check line %u\n", i);
				प्रत्यागम EXIT_FAILURE;
			}
		}
		प्रत्यागम EXIT_SUCCESS;
	}
#यदि_अन्तः

	/* For stable sort, store original line position beyond terminating NUL */
	यदि (option_mask32 & FLAG_s) {
		प्रति (i = 0; i < linecount; i++) {
			uint32_t *p32;
			वर्ण *पङ्क्तिः;
			अचिह्नित len;

			पङ्क्तिः = पङ्क्तयः[i];
			len = (वाक्यदैर्घ्यम्(पङ्क्तिः) + 4) & (~3u);
			पङ्क्तयः[i] = पङ्क्तिः = xrealloc(पङ्क्तिः, len + 4);
			p32 = (शून्य*)(पङ्क्तिः + len);
			*p32 = i;
		}
		/*option_mask32 |= FLAG_no_tie_break;*/
		/* ^^^redundant: if FLAG_s, compare_keys() does no tie break */
	}

	/* Perform the actual sort */
	शीघ्रं_क्रमय(पङ्क्तयः, linecount, परिमाण(पङ्क्तयः[0]), compare_कुञ्चिकाः);

	/* Handle -u */
	यदि (option_mask32 & FLAG_u) {
		पूर्णाङ्क j = 0;
		/* coreutils 6.3 drop lines for which only key is the same:
		 * - disabling last-resort compare, or else compare_keys()
		 * will be the same only for completely identical lines
		 * - disabling -s (same reasons)
		 */
		option_mask32 = (option_mask32 | FLAG_no_tie_break) & (~FLAG_s);
		प्रति (i = 1; i < linecount; i++) {
			यदि (compare_कुञ्चिकाः(&पङ्क्तयः[j], &पङ्क्तयः[i]) == 0)
				मुक्त(पङ्क्तयः[i]);
			अन्यथा
				पङ्क्तयः[++j] = पङ्क्तयः[i];
		}
		यदि (linecount)
			linecount = j+1;
	}

	/* Print it */
#यदि ENABLE_FEATURE_क्रमय_BIG
	/* Open output file _after_ we read all input ones */
	यदि (option_mask32 & FLAG_o)
		xmove_fd(xopen(str_o, O_WRONLY|O_CREAT|O_TRUNC), STDOUT_FILENO);
#यदि_अन्तः
	{
		पूर्णाङ्क ch = (option_mask32 & FLAG_z) ? '\0' : '\n';
		प्रति (i = 0; i < linecount; i++)
			प्रारूप_लिख("%s%c", पङ्क्तयः[i], ch);
	}

	fflush_stdout_and_exit_SUCCESS();
}
