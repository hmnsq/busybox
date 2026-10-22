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

#ଅନ୍ତର୍ଭାଵ "libbb.h"

/* These are sort types */
ଗଣନା {
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

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର sort_opt_str[] ALIGN1 = "^"
			"nghMVucszbrdfimS:T:o:k:*t:"
			"\0" "o--o:t--t"/*-t, -o: at most one of each*/;
/*
 * OPT_STR must not be string literal, needs to have stable address:
 * code uses "strchr(OPT_STR,c) - OPT_STR" idiom.
 */
#define OPT_STR (sort_opt_str + 1)

#ଯଦି ENABLE_FEATURE_SORT_BIG
ସ୍ଥିର ଅକ୍ଷର ଚାବି_separator;

ସ୍ଥିର ସଂରଚନା sort_ଚାବି {
	ସଂରଚନା sort_ଚାବି *next_ଚାବି;  /* linked list */
	unsigned range[4];          /* start word, start char, end word, end char */
	unsigned flags;
} *ଚାବି_list;


/* This is a NOEXEC applet. Be very careful! */


ସ୍ଥିର ଅକ୍ଷର *get_ଚାବି(ଅକ୍ଷର *str, ସଂରଚନା sort_ଚାବି *ଚାବି, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା start = start; /* for compiler */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା end;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len, j;
	unsigned i;

	/* Special case whole string, so we don't have to make a copy */
	ଯଦି (ଚାବି->range[0] == 1 && !ଚାବି->range[1] && !ଚାବି->range[2] && !ଚାବି->range[3]
	 && !(flags & (FLAG_b | FLAG_d | FLAG_f | FLAG_i | FLAG_bb))
	) {
		ଫେରନ୍ତୁ str;
	}

	/* Find start of key on first pass, end on second pass */
	len = strlen(str);
	ପାଇଁ (j = 0; j < 2; j++) {
		ଯଦି (!ଚାବି->range[2*j])
			end = len;
		/* Loop through fields */
		ଅନ୍ୟଥା {
			unsigned ଅକ୍ଷର ch = 0;

			end = 0;
			ପାଇଁ (i = 1; i < ଚାବି->range[2*j] + j; i++) {
				ଯଦି (ଚାବି_separator) {
					/* Skip body of key and separator */
					ଯେତେବେଳେ ((ch = str[end]) != '\0') {
							end++;
						ଯଦି (ch == ଚାବି_separator)
							ଭାଙ୍ଗ;
					}
				} ଅନ୍ୟଥା {
					/* Skip leading blanks */
					ଯେତେବେଳେ (isspace(str[end]))
						end++;
					/* Skip body of key */
					ଯେତେବେଳେ (str[end] != '\0') {
						ଯଦି (isspace(str[end]))
							ଭାଙ୍ଗ;
						end++;
					}
				}
			}
			/* Remove last delim: "abc:def:" => "abc:def" */
			ଯଦି (j && ch) {
				//if (str[end-1] != key_separator)
				//  bb_error_msg(_and_die("BUG! "
				//  "str[start:%d,end:%d]:'%.*s'",
				//  start, end, (int)(end-start), &str[start]);
				end--;
			}
		}
		ଯଦି (!j) start = end;
	}
	/* Strip leading whitespace if necessary */
	ଯଦି (flags & FLAG_b)
		/* not using skip_whitespace() for speed */
		ଯେତେବେଳେ (isspace(str[start])) start++;
	/* Strip trailing whitespace if necessary */
	ଯଦି (flags & FLAG_bb)
		ଯେତେବେଳେ (end > start && isspace(str[end-1])) end--;
	/* -kSTART,N.ENDCHAR: honor ENDCHAR (1-based) */
	ଯଦି (ଚାବି->range[3]) {
		end = ଚାବି->range[3];
		ଯଦି (end > len) end = len;
	}
	/* -kN.STARTCHAR[,...]: honor STARTCHAR (1-based) */
	ଯଦି (ଚାବି->range[1]) {
		start += ଚାବି->range[1] - 1;
		ଯଦି (start > len) start = len;
	}
	/* Make the copy */
	ଯଦି (end < start)
		end = start;
	str = xstrndup(str+start, end-start);
	/* Handle -d */
	ଯଦି (flags & FLAG_d) {
		ପାଇଁ (start = end = 0; str[end]; end++)
			ଯଦି (isspace(str[end]) || isalnum(str[end]))
				str[start++] = str[end];
		str[start] = '\0';
	}
	/* Handle -i */
	ଯଦି (flags & FLAG_i) {
		ପାଇଁ (start = end = 0; str[end]; end++)
			ଯଦି (isprint_asciionly(str[end]))
				str[start++] = str[end];
		str[start] = '\0';
	}
	/* Handle -f */
	ଯଦି (flags & FLAG_f)
		ପାଇଁ (i = 0; str[i]; i++)
			str[i] = toupper(str[i]);

	ଫେରନ୍ତୁ str;
}

ସ୍ଥିର ସଂରଚନା sort_ଚାବି *add_ଚାବି(ଶୂନ୍ୟ)
{
	ସଂରଚନା sort_ଚାବି **pkey = &ଚାବି_list;
	ଯେତେବେଳେ (*pkey)
		pkey = &((*pkey)->next_ଚାବି);
	ଫେରନ୍ତୁ *pkey = xzalloc(ଆକାର(ସଂରଚନା sort_ଚାବି));
}

#define GET_ଧାଡ଼ି(fp) \
	((option_mask32 & FLAG_z) \
	? bb_get_chunk_from_file(fp, ଖାଲି) \
	: xmalloc_fgetline(fp))
#ଅନ୍ୟଥା
#define GET_ଧାଡ଼ି(fp) xmalloc_fgetline(fp)
#endif

#ଯଦି ENABLE_FEATURE_SORT_BIG
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା scale_suffix(ଅଚଳ ଅକ୍ଷର *tail)
{
	ସ୍ଥିର ଅଚଳ ଅକ୍ଷର suffix[] ALIGN1 = "kmgtpezy";
	ଅଚଳ ଅକ୍ଷର *s;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;

	ଯଦି (!tail[0])
		ଫେରନ୍ତୁ -1;
	s = strchr(suffix, tail[0] | 0x20);
	ଯଦି (!s)
		ଫେରନ୍ତୁ -1;
	n = s - suffix;
	ଯଦି (n != 0 && tail[0] >= 'a')
		ଫେରନ୍ତୁ -1; /* mg... not accepted, only MG... */
	ଫେରନ୍ତୁ n;
}
#endif

/* Iterate through keys list and perform comparisons */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା compare_ଚାବିଗୁଡ଼ିକ(ଅଚଳ ଶୂନ୍ୟ *xarg, ଅଚଳ ଶୂନ୍ୟ *yarg)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = option_mask32, retval = 0;
	ଅକ୍ଷର *x, *y;

#ଯଦି ENABLE_FEATURE_SORT_BIG
	ସଂରଚନା sort_ଚାବି *ଚାବି;

	ପାଇଁ (ଚାବି = ଚାବି_list; !retval && ଚାବି; ଚାବି = ଚାବି->next_ଚାବି) {
		flags = ଚାବି->flags ? ଚାବି->flags : option_mask32;
		/* Chop out and modify key chunks, handling -dfib */
		x = get_ଚାବି(*(ଅକ୍ଷର **)xarg, ଚାବି, flags);
		y = get_ଚାବି(*(ଅକ୍ଷର **)yarg, ଚାବି, flags);
#ଅନ୍ୟଥା
	/* This curly bracket serves no purpose but to match the nesting
	 * level of the for () loop we're not using */
	{
		x = *(ଅକ୍ଷର **)xarg;
		y = *(ଅକ୍ଷର **)yarg;
#endif
		/* Perform actual comparison */
		ବାଛ (flags & (FLAG_n | FLAG_g | FLAG_h | FLAG_M | FLAG_V)) {
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			bb_simple_error_ସନ୍ଦେଶ_and_die("unknown sort type");
			ଭାଙ୍ଗ;
#ଯଦି defined(HAVE_STRVERSCMP) && HAVE_STRVERSCMP == 1
		ସ୍ଥିତି FLAG_V:
			retval = strverscmp(x, y);
			ଭାଙ୍ଗ;
#endif
		/* Ascii sort */
		ସ୍ଥିତି 0:
#ଯଦି ENABLE_LOCALE_SUPPORT
			retval = strcoll(x, y);
#ଅନ୍ୟଥା
			retval = strcmp(x, y);
#endif
			ଭାଙ୍ଗ;
#ଯଦି ENABLE_FEATURE_SORT_BIG
		ସ୍ଥିତି FLAG_g:
		ସ୍ଥିତି FLAG_h: {
			ଅକ୍ଷର *xx, *yy;
//TODO: needs setlocale(LC_NUMERIC, "C")?
			ଦ୍ୱିଗୁଣ dx = strtod(x, &xx);
			ଦ୍ୱିଗୁଣ dy = strtod(y, &yy);
			/* not numbers < NaN < -infinity < numbers < +infinity) */
			ଯଦି (x == xx)
				retval = (y == yy ? 0 : -1);
			ଅନ୍ୟଥା ଯଦି (y == yy)
				retval = 1;
			/* Check for isnan */
			ଅନ୍ୟଥା ଯଦି (dx != dx)
				retval = (dy != dy) ? 0 : -1;
			ଅନ୍ୟଥା ଯଦି (dy != dy)
				retval = 1;
			ଅନ୍ୟଥା {
				ଯଦି (flags & FLAG_h) {
					ପୂର୍ଣ୍ଣସଂଖ୍ୟା xs = scale_suffix(xx);
					ପୂର୍ଣ୍ଣସଂଖ୍ୟା ys = scale_suffix(yy);
					ଯଦି (xs != ys) {
						retval = xs - ys;
						ଭାଙ୍ଗ;
					}
				}
				/* Check for infinity.  Could underflow, but it avoids libm. */
				ଯଦି (1.0 / dx == 0.0) {
					ଯଦି (dx < 0)
						retval = (1.0 / dy == 0.0 && dy < 0) ? 0 : -1;
					ଅନ୍ୟଥା
						retval = (1.0 / dy == 0.0 && dy > 0) ? 0 : 1;
				} ଅନ୍ୟଥା ଯଦି (1.0 / dy == 0.0)
					retval = (dy < 0) ? 1 : -1;
				ଅନ୍ୟଥା
					retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			}
			ଭାଙ୍ଗ;
		}
		ସ୍ଥିତି FLAG_M: {
			ସଂରଚନା tm thyme;
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା dx;
			ଅକ୍ଷର *xx, *yy;

			xx = strptime(skip_whitespace(x), "%b", &thyme);
			dx = thyme.tm_mon;
			yy = strptime(skip_whitespace(y), "%b", &thyme);
			ଯଦି (!xx)
				retval = (!yy) ? 0 : -1;
			ଅନ୍ୟଥା ଯଦି (!yy)
				retval = 1;
			ଅନ୍ୟଥା
				retval = dx - thyme.tm_mon;
			ଭାଙ୍ଗ;
		}
		/* Full floating point version of -n */
		ସ୍ଥିତି FLAG_n: {
			ଦ୍ୱିଗୁଣ dx = atof(x);
			ଦ୍ୱିଗୁଣ dy = atof(y);
			retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			ଭାଙ୍ଗ;
		}
		} /* switch */
		/* Free key copies. */
		ଯଦି (x != *(ଅକ୍ଷର **)xarg) ମୁକ୍ତ(x);
		ଯଦି (y != *(ଅକ୍ଷର **)yarg) ମୁକ୍ତ(y);
		/* if (retval) break; - done by for () anyway */
#ଅନ୍ୟଥା
		/* Integer version of -n for tiny systems */
		ସ୍ଥିତି FLAG_n:
			retval = atoi(x) - atoi(y);
			ଭାଙ୍ଗ;
		} /* switch */
#endif
	} /* for */

	ଯଦି (retval == 0) {
		/* So far lines are "the same" */

		ଯଦି (option_mask32 & FLAG_s) {
			/* "Stable sort": later line is "greater than",
			 * IOW: do not allow qsort() to swap equal lines.
			 */
			uint32_t *p32;
			uint32_t x32, y32;
			ଅକ୍ଷର *ଧାଡ଼ି;
			unsigned len;

			ଧାଡ଼ି = *(ଅକ୍ଷର**)xarg;
			len = (strlen(ଧାଡ଼ି) + 4) & (~3u);
			p32 = (ଶୂନ୍ୟ*)(ଧାଡ଼ି + len);
			x32 = *p32;
			ଧାଡ଼ି = *(ଅକ୍ଷର**)yarg;
			len = (strlen(ଧାଡ଼ି) + 4) & (~3u);
			p32 = (ଶୂନ୍ୟ*)(ଧାଡ଼ି + len);
			y32 = *p32;

			/* If x > y, 1, else -1 */
			retval = (x32 > y32) * 2 - 1;
			/* Here, -r has no effect! */
			ଫେରନ୍ତୁ retval;
		}
		ଯଦି (!(option_mask32 & FLAG_no_tie_break)) {
			/* fallback sort */
			flags = option_mask32;
			retval = strcmp(*(ଅକ୍ଷର **)xarg, *(ଅକ୍ଷର **)yarg);
		}
	}

	ଯଦି (flags & FLAG_r)
		ଫେରନ୍ତୁ -retval;

	ଫେରନ୍ତୁ retval;
}

#ଯଦି ENABLE_FEATURE_SORT_BIG
ସ୍ଥିର unsigned str2u(ଅକ୍ଷର **str)
{
	unsigned long lu;
	ଯଦି (!isdigit((*str)[0]))
		bb_simple_error_ସନ୍ଦେଶ_and_die("bad field specification");
	lu = strtoul(*str, str, 10);
	ଯଦି ((ଆକାର(long) > ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା) && lu > INT_MAX) || !lu)
		bb_simple_error_ସନ୍ଦେଶ_and_die("bad field specification");
	ଫେରନ୍ତୁ lu;
}
#endif

ପୂର୍ଣ୍ଣସଂଖ୍ୟା sort_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv) MAIN_EXTERNALLY_VISIBLE;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sort_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc UNUSED_PARAM, ଅକ୍ଷର **argv)
{
	ଅକ୍ଷର **ଧାଡ଼ିଗୁଡ଼ିକ;
	ଅକ୍ଷର *str_ignored, *str_o, *str_t;
	llist_t *lst_k = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା linecount;
	unsigned opts;
#ଯଦି ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
	bool can_drop_dups;
	size_t prev_len = 0;
	ଅକ୍ଷର *prev_ଧାଡ଼ି = (ଅକ୍ଷର*) "";
	/* Postpone optimizing if the input is small, < 16k lines:
	 * even just free()ing duplicate lines takes time.
	 */
	size_t ଗଣନା_to_optimize_dups = 0x3fff;
#endif

	xfunc_error_retval = 2;

	/* Parse command line options */
	opts = getopt32(argv,
			sort_opt_str,
			&str_ignored, &str_ignored, &str_o, &lst_k, &str_t
	);
#ଯଦି ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
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
	ଯଦି (opts & FLAG_s)
		ଗଣନା_to_optimize_dups = (size_t)-1L;
#endif
	/* global b strips leading and trailing spaces */
	ଯଦି (opts & FLAG_b)
		option_mask32 |= FLAG_bb;
#ଯଦି ENABLE_FEATURE_SORT_BIG
	ଯଦି (opts & FLAG_t) {
		ଯଦି (!str_t[0] || str_t[1])
			bb_simple_error_ସନ୍ଦେଶ_and_die("bad -t parameter");
		ଚାବି_separator = str_t[0];
	}
	/* note: below this point we use option_mask32, not opts,
	 * since that reduces register pressure and makes code smaller */

	/* Parse sort key */
	ଯେତେବେଳେ (lst_k) {
		ଗଣନା {
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
		ସଂରଚନା sort_ଚାବି *ଚାବି = add_ଚାବି();
		ଅକ୍ଷର *str_k = llist_କାଢ଼(&lst_k);

		i = 0; /* i==0 before comma, 1 after (-k3,6) */
		ଯେତେବେଳେ (*str_k) {
			/* Start of range */
			/* Cannot use bb_strtou - suffix can be a letter */
			ଚାବି->range[2*i] = str2u(&str_k);
			ଯଦି (*str_k == '.') {
				str_k++;
				ଚାବି->range[2*i+1] = str2u(&str_k);
			}
			ଯେତେବେଳେ (*str_k) {
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା flag;
				ଅଚଳ ଅକ୍ଷର *idx;

				ଯଦି (*str_k == ',' && !i++) {
					str_k++;
					ଭାଙ୍ଗ;
				} /* no else needed: fall through to syntax error
					because comma isn't in OPT_STR */
				idx = strchr(OPT_STR, *str_k);
				ଯଦି (!idx)
					bb_simple_error_ସନ୍ଦେଶ_and_die("unknown key option");
				flag = 1 << (idx - OPT_STR);
				ଯଦି (flag & ~FLAG_allowed_for_k)
					bb_simple_error_ସନ୍ଦେଶ_and_die("unknown sort type");
				/* b after ',' means strip _trailing_ space */
				ଯଦି (i && flag == FLAG_b)
					flag = FLAG_bb;
				ଚାବି->flags |= flag;
				str_k++;
			}
		}
	}
#endif

	/* Open input files and read data */
	argv += optind;
	ଯଦି (!*argv)
		*--argv = (ଅକ୍ଷର*)"-";
	linecount = 0;
	ଧାଡ଼ିଗୁଡ଼ିକ = ଖାଲି;
	କର {
		/* coreutils 6.9 compat: abort on first open error,
		 * do not continue to next file: */
		FILE *fp = xfopen_stdin(*argv);
		ପାଇଁ (;;) {
			ଅକ୍ଷର *ଧାଡ଼ି = GET_ଧାଡ଼ି(fp);
			ଯଦି (!ଧାଡ଼ି)
				ଭାଙ୍ଗ;

#ଯଦି ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
			ଯଦି (ଗଣନା_to_optimize_dups != 0)
				ଗଣନା_to_optimize_dups--;
			ଯଦି (ଗଣନା_to_optimize_dups == 0) {
				size_t len;
				ଅକ୍ଷର *new_ଧାଡ଼ି;

				/* On kernel/linux/arch/ *.[ch] files,
				 * this reduces memory usage by 6%.
				 *  yes | head -99999999 | sort
				 * goes down from 1900Mb to 380 Mb.
				 */
				len = strlen(ଧାଡ଼ି);
				ଯଦି (len <= prev_len) {
					new_ଧାଡ଼ି = prev_ଧାଡ଼ି + (prev_len - len);
					ଯଦି (strcmp(ଧାଡ଼ି, new_ଧାଡ଼ି) == 0) {
						/* it's a tail of the prev line */
						ଯଦି (can_drop_dups && prev_len == len) {
							/* it's identical to prev line */
							ମୁକ୍ତ(ଧାଡ଼ି);
							ଜାରି;
						}
						ମୁକ୍ତ(ଧାଡ଼ି);
						ଧାଡ଼ି = new_ଧାଡ଼ି;
						/* continue using longer prev_line
						 * for future tail tests.
						 */
						ଯାଅ skip;
					}
				}
				prev_len = len;
				prev_ଧାଡ଼ି = ଧାଡ଼ି;
 skip: ;
			}
#ଅନ୍ୟଥା
//TODO: lighter version which only drops total dups if can_drop_dups == true
#endif
			ଧାଡ଼ିଗୁଡ଼ିକ = xrealloc_vector(ଧାଡ଼ିଗୁଡ଼ିକ, 6, linecount);
			ଧାଡ଼ିଗୁଡ଼ିକ[linecount++] = ଧାଡ଼ି;
		}
		fclose_if_not_stdin(fp);
	} ଯେତେବେଳେ (*++argv);

#ଯଦି ENABLE_FEATURE_SORT_BIG
	/* If no key, perform alphabetic sort */
	ଯଦି (!ଚାବି_list)
		add_ଚାବି()->range[0] = 1;
	/* Handle -c */
	ଯଦି (option_mask32 & FLAG_c) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା j = (option_mask32 & FLAG_u) ? -1 : 0;
		ପାଇଁ (i = 1; i < linecount; i++) {
			ଯଦି (compare_ଚାବିଗୁଡ଼ିକ(&ଧାଡ଼ିଗୁଡ଼ିକ[i-1], &ଧାଡ଼ିଗୁଡ଼ିକ[i]) > j) {
				fprintf(stderr, "Check line %u\n", i);
				ଫେରନ୍ତୁ EXIT_FAILURE;
			}
		}
		ଫେରନ୍ତୁ EXIT_SUCCESS;
	}
#endif

	/* For stable sort, store original line position beyond terminating NUL */
	ଯଦି (option_mask32 & FLAG_s) {
		ପାଇଁ (i = 0; i < linecount; i++) {
			uint32_t *p32;
			ଅକ୍ଷର *ଧାଡ଼ି;
			unsigned len;

			ଧାଡ଼ି = ଧାଡ଼ିଗୁଡ଼ିକ[i];
			len = (strlen(ଧାଡ଼ି) + 4) & (~3u);
			ଧାଡ଼ିଗୁଡ଼ିକ[i] = ଧାଡ଼ି = xrealloc(ଧାଡ଼ି, len + 4);
			p32 = (ଶୂନ୍ୟ*)(ଧାଡ଼ି + len);
			*p32 = i;
		}
		/*option_mask32 |= FLAG_no_tie_break;*/
		/* ^^^redundant: if FLAG_s, compare_keys() does no tie break */
	}

	/* Perform the actual sort */
	qsort(ଧାଡ଼ିଗୁଡ଼ିକ, linecount, ଆକାର(ଧାଡ଼ିଗୁଡ଼ିକ[0]), compare_ଚାବିଗୁଡ଼ିକ);

	/* Handle -u */
	ଯଦି (option_mask32 & FLAG_u) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା j = 0;
		/* coreutils 6.3 drop lines for which only key is the same:
		 * - disabling last-resort compare, or else compare_keys()
		 * will be the same only for completely identical lines
		 * - disabling -s (same reasons)
		 */
		option_mask32 = (option_mask32 | FLAG_no_tie_break) & (~FLAG_s);
		ପାଇଁ (i = 1; i < linecount; i++) {
			ଯଦି (compare_ଚାବିଗୁଡ଼ିକ(&ଧାଡ଼ିଗୁଡ଼ିକ[j], &ଧାଡ଼ିଗୁଡ଼ିକ[i]) == 0)
				ମୁକ୍ତ(ଧାଡ଼ିଗୁଡ଼ିକ[i]);
			ଅନ୍ୟଥା
				ଧାଡ଼ିଗୁଡ଼ିକ[++j] = ଧାଡ଼ିଗୁଡ଼ିକ[i];
		}
		ଯଦି (linecount)
			linecount = j+1;
	}

	/* Print it */
#ଯଦି ENABLE_FEATURE_SORT_BIG
	/* Open output file _after_ we read all input ones */
	ଯଦି (option_mask32 & FLAG_o)
		xmove_fd(xopen(str_o, O_WRONLY|O_CREAT|O_TRUNC), STDOUT_FILENO);
#endif
	{
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch = (option_mask32 & FLAG_z) ? '\0' : '\n';
		ପାଇଁ (i = 0; i < linecount; i++)
			printf("%s%c", ଧାଡ଼ିଗୁଡ଼ିକ[i], ch);
	}

	fflush_stdout_and_exit_SUCCESS();
}
