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

#ಅನ್ತರ್ಭಾವ "libbb.h"

/* These are sort types */
ಗಣನಾ {
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

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ sort_opt_str[] ALIGN1 = "^"
			"nghMVucszbrdfimS:T:o:k:*t:"
			"\0" "o--o:t--t"/*-t, -o: at most one of each*/;
/*
 * OPT_STR must not be string literal, needs to have stable address:
 * code uses "strchr(OPT_STR,c) - OPT_STR" idiom.
 */
#define OPT_STR (sort_opt_str + 1)

#ಯದಿ ENABLE_FEATURE_SORT_BIG
ಸ್ಥಿರ ವರ್ಣ ಕೀಲಿ_separator;

ಸ್ಥಿರ ಸಂರಚನಾ sort_ಕೀಲಿ {
	ಸಂರಚನಾ sort_ಕೀಲಿ *next_ಕೀಲಿ;  /* linked list */
	unsigned range[4];          /* start word, start char, end word, end char */
	unsigned flags;
} *ಕೀಲಿ_list;


/* This is a NOEXEC applet. Be very careful! */


ಸ್ಥಿರ ವರ್ಣ *get_ಕೀಲಿ(ವರ್ಣ *str, ಸಂರಚನಾ sort_ಕೀಲಿ *ಕೀಲಿ, ಪೂರ್ಣಾಙ್ಕ flags)
{
	ಪೂರ್ಣಾಙ್ಕ start = start; /* for compiler */
	ಪೂರ್ಣಾಙ್ಕ end;
	ಪೂರ್ಣಾಙ್ಕ len, j;
	unsigned i;

	/* Special case whole string, so we don't have to make a copy */
	ಯದಿ (ಕೀಲಿ->range[0] == 1 && !ಕೀಲಿ->range[1] && !ಕೀಲಿ->range[2] && !ಕೀಲಿ->range[3]
	 && !(flags & (FLAG_b | FLAG_d | FLAG_f | FLAG_i | FLAG_bb))
	) {
		ಪ್ರತ್ಯಾಗಮ str;
	}

	/* Find start of key on first pass, end on second pass */
	len = strlen(str);
	ಪ್ರತಿ (j = 0; j < 2; j++) {
		ಯದಿ (!ಕೀಲಿ->range[2*j])
			end = len;
		/* Loop through fields */
		ಅನ್ಯಥಾ {
			unsigned ವರ್ಣ ch = 0;

			end = 0;
			ಪ್ರತಿ (i = 1; i < ಕೀಲಿ->range[2*j] + j; i++) {
				ಯದಿ (ಕೀಲಿ_separator) {
					/* Skip body of key and separator */
					ಯಾವತ್ ((ch = str[end]) != '\0') {
							end++;
						ಯದಿ (ch == ಕೀಲಿ_separator)
							ಭಙ್ಗ;
					}
				} ಅನ್ಯಥಾ {
					/* Skip leading blanks */
					ಯಾವತ್ (isspace(str[end]))
						end++;
					/* Skip body of key */
					ಯಾವತ್ (str[end] != '\0') {
						ಯದಿ (isspace(str[end]))
							ಭಙ್ಗ;
						end++;
					}
				}
			}
			/* Remove last delim: "abc:def:" => "abc:def" */
			ಯದಿ (j && ch) {
				//if (str[end-1] != key_separator)
				//  bb_error_msg(_and_die("BUG! "
				//  "str[start:%d,end:%d]:'%.*s'",
				//  start, end, (int)(end-start), &str[start]);
				end--;
			}
		}
		ಯದಿ (!j) start = end;
	}
	/* Strip leading whitespace if necessary */
	ಯದಿ (flags & FLAG_b)
		/* not using skip_whitespace() for speed */
		ಯಾವತ್ (isspace(str[start])) start++;
	/* Strip trailing whitespace if necessary */
	ಯದಿ (flags & FLAG_bb)
		ಯಾವತ್ (end > start && isspace(str[end-1])) end--;
	/* -kSTART,N.ENDCHAR: honor ENDCHAR (1-based) */
	ಯದಿ (ಕೀಲಿ->range[3]) {
		end = ಕೀಲಿ->range[3];
		ಯದಿ (end > len) end = len;
	}
	/* -kN.STARTCHAR[,...]: honor STARTCHAR (1-based) */
	ಯದಿ (ಕೀಲಿ->range[1]) {
		start += ಕೀಲಿ->range[1] - 1;
		ಯದಿ (start > len) start = len;
	}
	/* Make the copy */
	ಯದಿ (end < start)
		end = start;
	str = xstrndup(str+start, end-start);
	/* Handle -d */
	ಯದಿ (flags & FLAG_d) {
		ಪ್ರತಿ (start = end = 0; str[end]; end++)
			ಯದಿ (isspace(str[end]) || isalnum(str[end]))
				str[start++] = str[end];
		str[start] = '\0';
	}
	/* Handle -i */
	ಯದಿ (flags & FLAG_i) {
		ಪ್ರತಿ (start = end = 0; str[end]; end++)
			ಯದಿ (isprint_asciionly(str[end]))
				str[start++] = str[end];
		str[start] = '\0';
	}
	/* Handle -f */
	ಯದಿ (flags & FLAG_f)
		ಪ್ರತಿ (i = 0; str[i]; i++)
			str[i] = toupper(str[i]);

	ಪ್ರತ್ಯಾಗಮ str;
}

ಸ್ಥಿರ ಸಂರಚನಾ sort_ಕೀಲಿ *add_ಕೀಲಿ(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ sort_ಕೀಲಿ **pkey = &ಕೀಲಿ_list;
	ಯಾವತ್ (*pkey)
		pkey = &((*pkey)->next_ಕೀಲಿ);
	ಪ್ರತ್ಯಾಗಮ *pkey = xzalloc(ಪರಿಮಾಣ(ಸಂರಚನಾ sort_ಕೀಲಿ));
}

#define GET_ಸಾಲು(fp) \
	((option_mask32 & FLAG_z) \
	? bb_get_chunk_from_file(fp, ರಿಕ್ತ) \
	: xmalloc_fgetline(fp))
#ಅನ್ಯಥಾ
#define GET_ಸಾಲು(fp) xmalloc_fgetline(fp)
#endif

#ಯದಿ ENABLE_FEATURE_SORT_BIG
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ scale_suffix(ಧ್ರುವ ವರ್ಣ *tail)
{
	ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ suffix[] ALIGN1 = "kmgtpezy";
	ಧ್ರುವ ವರ್ಣ *s;
	ಪೂರ್ಣಾಙ್ಕ n;

	ಯದಿ (!tail[0])
		ಪ್ರತ್ಯಾಗಮ -1;
	s = strchr(suffix, tail[0] | 0x20);
	ಯದಿ (!s)
		ಪ್ರತ್ಯಾಗಮ -1;
	n = s - suffix;
	ಯದಿ (n != 0 && tail[0] >= 'a')
		ಪ್ರತ್ಯಾಗಮ -1; /* mg... not accepted, only MG... */
	ಪ್ರತ್ಯಾಗಮ n;
}
#endif

/* Iterate through keys list and perform comparisons */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ compare_ಕೀಲಿಗಳು(ಧ್ರುವ ಶೂನ್ಯ *xarg, ಧ್ರುವ ಶೂನ್ಯ *yarg)
{
	ಪೂರ್ಣಾಙ್ಕ flags = option_mask32, retval = 0;
	ವರ್ಣ *x, *y;

#ಯದಿ ENABLE_FEATURE_SORT_BIG
	ಸಂರಚನಾ sort_ಕೀಲಿ *ಕೀಲಿ;

	ಪ್ರತಿ (ಕೀಲಿ = ಕೀಲಿ_list; !retval && ಕೀಲಿ; ಕೀಲಿ = ಕೀಲಿ->next_ಕೀಲಿ) {
		flags = ಕೀಲಿ->flags ? ಕೀಲಿ->flags : option_mask32;
		/* Chop out and modify key chunks, handling -dfib */
		x = get_ಕೀಲಿ(*(ವರ್ಣ **)xarg, ಕೀಲಿ, flags);
		y = get_ಕೀಲಿ(*(ವರ್ಣ **)yarg, ಕೀಲಿ, flags);
#ಅನ್ಯಥಾ
	/* This curly bracket serves no purpose but to match the nesting
	 * level of the for () loop we're not using */
	{
		x = *(ವರ್ಣ **)xarg;
		y = *(ವರ್ಣ **)yarg;
#endif
		/* Perform actual comparison */
		ವಿಕಲ್ಪ (flags & (FLAG_n | FLAG_g | FLAG_h | FLAG_M | FLAG_V)) {
		ಸ್ವಭಾವ:
			bb_simple_error_ಸಂದೇಶ_and_die("unknown sort type");
			ಭಙ್ಗ;
#ಯದಿ defined(HAVE_STRVERSCMP) && HAVE_STRVERSCMP == 1
		ಪಕ್ಷ FLAG_V:
			retval = strverscmp(x, y);
			ಭಙ್ಗ;
#endif
		/* Ascii sort */
		ಪಕ್ಷ 0:
#ಯದಿ ENABLE_LOCALE_SUPPORT
			retval = strcoll(x, y);
#ಅನ್ಯಥಾ
			retval = strcmp(x, y);
#endif
			ಭಙ್ಗ;
#ಯದಿ ENABLE_FEATURE_SORT_BIG
		ಪಕ್ಷ FLAG_g:
		ಪಕ್ಷ FLAG_h: {
			ವರ್ಣ *xx, *yy;
//TODO: needs setlocale(LC_NUMERIC, "C")?
			ದ್ವಿಗುಣ dx = strtod(x, &xx);
			ದ್ವಿಗುಣ dy = strtod(y, &yy);
			/* not numbers < NaN < -infinity < numbers < +infinity) */
			ಯದಿ (x == xx)
				retval = (y == yy ? 0 : -1);
			ಅನ್ಯಥಾ ಯದಿ (y == yy)
				retval = 1;
			/* Check for isnan */
			ಅನ್ಯಥಾ ಯದಿ (dx != dx)
				retval = (dy != dy) ? 0 : -1;
			ಅನ್ಯಥಾ ಯದಿ (dy != dy)
				retval = 1;
			ಅನ್ಯಥಾ {
				ಯದಿ (flags & FLAG_h) {
					ಪೂರ್ಣಾಙ್ಕ xs = scale_suffix(xx);
					ಪೂರ್ಣಾಙ್ಕ ys = scale_suffix(yy);
					ಯದಿ (xs != ys) {
						retval = xs - ys;
						ಭಙ್ಗ;
					}
				}
				/* Check for infinity.  Could underflow, but it avoids libm. */
				ಯದಿ (1.0 / dx == 0.0) {
					ಯದಿ (dx < 0)
						retval = (1.0 / dy == 0.0 && dy < 0) ? 0 : -1;
					ಅನ್ಯಥಾ
						retval = (1.0 / dy == 0.0 && dy > 0) ? 0 : 1;
				} ಅನ್ಯಥಾ ಯದಿ (1.0 / dy == 0.0)
					retval = (dy < 0) ? 1 : -1;
				ಅನ್ಯಥಾ
					retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			}
			ಭಙ್ಗ;
		}
		ಪಕ್ಷ FLAG_M: {
			ಸಂರಚನಾ tm thyme;
			ಪೂರ್ಣಾಙ್ಕ dx;
			ವರ್ಣ *xx, *yy;

			xx = strptime(skip_whitespace(x), "%b", &thyme);
			dx = thyme.tm_mon;
			yy = strptime(skip_whitespace(y), "%b", &thyme);
			ಯದಿ (!xx)
				retval = (!yy) ? 0 : -1;
			ಅನ್ಯಥಾ ಯದಿ (!yy)
				retval = 1;
			ಅನ್ಯಥಾ
				retval = dx - thyme.tm_mon;
			ಭಙ್ಗ;
		}
		/* Full floating point version of -n */
		ಪಕ್ಷ FLAG_n: {
			ದ್ವಿಗುಣ dx = atof(x);
			ದ್ವಿಗುಣ dy = atof(y);
			retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			ಭಙ್ಗ;
		}
		} /* switch */
		/* Free key copies. */
		ಯದಿ (x != *(ವರ್ಣ **)xarg) ಮುಕ್ತ(x);
		ಯದಿ (y != *(ವರ್ಣ **)yarg) ಮುಕ್ತ(y);
		/* if (retval) break; - done by for () anyway */
#ಅನ್ಯಥಾ
		/* Integer version of -n for tiny systems */
		ಪಕ್ಷ FLAG_n:
			retval = atoi(x) - atoi(y);
			ಭಙ್ಗ;
		} /* switch */
#endif
	} /* for */

	ಯದಿ (retval == 0) {
		/* So far lines are "the same" */

		ಯದಿ (option_mask32 & FLAG_s) {
			/* "Stable sort": later line is "greater than",
			 * IOW: do not allow qsort() to swap equal lines.
			 */
			uint32_t *p32;
			uint32_t x32, y32;
			ವರ್ಣ *ಸಾಲು;
			unsigned len;

			ಸಾಲು = *(ವರ್ಣ**)xarg;
			len = (strlen(ಸಾಲು) + 4) & (~3u);
			p32 = (ಶೂನ್ಯ*)(ಸಾಲು + len);
			x32 = *p32;
			ಸಾಲು = *(ವರ್ಣ**)yarg;
			len = (strlen(ಸಾಲು) + 4) & (~3u);
			p32 = (ಶೂನ್ಯ*)(ಸಾಲು + len);
			y32 = *p32;

			/* If x > y, 1, else -1 */
			retval = (x32 > y32) * 2 - 1;
			/* Here, -r has no effect! */
			ಪ್ರತ್ಯಾಗಮ retval;
		}
		ಯದಿ (!(option_mask32 & FLAG_no_tie_break)) {
			/* fallback sort */
			flags = option_mask32;
			retval = strcmp(*(ವರ್ಣ **)xarg, *(ವರ್ಣ **)yarg);
		}
	}

	ಯದಿ (flags & FLAG_r)
		ಪ್ರತ್ಯಾಗಮ -retval;

	ಪ್ರತ್ಯಾಗಮ retval;
}

#ಯದಿ ENABLE_FEATURE_SORT_BIG
ಸ್ಥಿರ unsigned str2u(ವರ್ಣ **str)
{
	unsigned long lu;
	ಯದಿ (!isdigit((*str)[0]))
		bb_simple_error_ಸಂದೇಶ_and_die("bad field specification");
	lu = strtoul(*str, str, 10);
	ಯದಿ ((ಪರಿಮಾಣ(long) > ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ) && lu > INT_MAX) || !lu)
		bb_simple_error_ಸಂದೇಶ_and_die("bad field specification");
	ಪ್ರತ್ಯಾಗಮ lu;
}
#endif

ಪೂರ್ಣಾಙ್ಕ sort_main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಙ್ಕ sort_main(ಪೂರ್ಣಾಙ್ಕ argc UNUSED_PARAM, ವರ್ಣ **argv)
{
	ವರ್ಣ **ಸಾಲುಗಳು;
	ವರ್ಣ *str_ignored, *str_o, *str_t;
	llist_t *lst_k = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ i;
	ಪೂರ್ಣಾಙ್ಕ linecount;
	unsigned opts;
#ಯದಿ ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
	bool can_drop_dups;
	size_t prev_len = 0;
	ವರ್ಣ *prev_ಸಾಲು = (ವರ್ಣ*) "";
	/* Postpone optimizing if the input is small, < 16k lines:
	 * even just free()ing duplicate lines takes time.
	 */
	size_t ಎಣಿಕೆ_to_optimize_dups = 0x3fff;
#endif

	xfunc_error_retval = 2;

	/* Parse command line options */
	opts = getopt32(argv,
			sort_opt_str,
			&str_ignored, &str_ignored, &str_o, &lst_k, &str_t
	);
#ಯದಿ ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
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
	ಯದಿ (opts & FLAG_s)
		ಎಣಿಕೆ_to_optimize_dups = (size_t)-1L;
#endif
	/* global b strips leading and trailing spaces */
	ಯದಿ (opts & FLAG_b)
		option_mask32 |= FLAG_bb;
#ಯದಿ ENABLE_FEATURE_SORT_BIG
	ಯದಿ (opts & FLAG_t) {
		ಯದಿ (!str_t[0] || str_t[1])
			bb_simple_error_ಸಂದೇಶ_and_die("bad -t parameter");
		ಕೀಲಿ_separator = str_t[0];
	}
	/* note: below this point we use option_mask32, not opts,
	 * since that reduces register pressure and makes code smaller */

	/* Parse sort key */
	ಯಾವತ್ (lst_k) {
		ಗಣನಾ {
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
		ಸಂರಚನಾ sort_ಕೀಲಿ *ಕೀಲಿ = add_ಕೀಲಿ();
		ವರ್ಣ *str_k = llist_ತೆಗೆ(&lst_k);

		i = 0; /* i==0 before comma, 1 after (-k3,6) */
		ಯಾವತ್ (*str_k) {
			/* Start of range */
			/* Cannot use bb_strtou - suffix can be a letter */
			ಕೀಲಿ->range[2*i] = str2u(&str_k);
			ಯದಿ (*str_k == '.') {
				str_k++;
				ಕೀಲಿ->range[2*i+1] = str2u(&str_k);
			}
			ಯಾವತ್ (*str_k) {
				ಪೂರ್ಣಾಙ್ಕ flag;
				ಧ್ರುವ ವರ್ಣ *idx;

				ಯದಿ (*str_k == ',' && !i++) {
					str_k++;
					ಭಙ್ಗ;
				} /* no else needed: fall through to syntax error
					because comma isn't in OPT_STR */
				idx = strchr(OPT_STR, *str_k);
				ಯದಿ (!idx)
					bb_simple_error_ಸಂದೇಶ_and_die("unknown key option");
				flag = 1 << (idx - OPT_STR);
				ಯದಿ (flag & ~FLAG_allowed_for_k)
					bb_simple_error_ಸಂದೇಶ_and_die("unknown sort type");
				/* b after ',' means strip _trailing_ space */
				ಯದಿ (i && flag == FLAG_b)
					flag = FLAG_bb;
				ಕೀಲಿ->flags |= flag;
				str_k++;
			}
		}
	}
#endif

	/* Open input files and read data */
	argv += optind;
	ಯದಿ (!*argv)
		*--argv = (ವರ್ಣ*)"-";
	linecount = 0;
	ಸಾಲುಗಳು = ರಿಕ್ತ;
	ಕುರು {
		/* coreutils 6.9 compat: abort on first open error,
		 * do not continue to next file: */
		FILE *fp = xfopen_stdin(*argv);
		ಪ್ರತಿ (;;) {
			ವರ್ಣ *ಸಾಲು = GET_ಸಾಲು(fp);
			ಯದಿ (!ಸಾಲು)
				ಭಙ್ಗ;

#ಯದಿ ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
			ಯದಿ (ಎಣಿಕೆ_to_optimize_dups != 0)
				ಎಣಿಕೆ_to_optimize_dups--;
			ಯದಿ (ಎಣಿಕೆ_to_optimize_dups == 0) {
				size_t len;
				ವರ್ಣ *new_ಸಾಲು;

				/* On kernel/linux/arch/ *.[ch] files,
				 * this reduces memory usage by 6%.
				 *  yes | head -99999999 | sort
				 * goes down from 1900Mb to 380 Mb.
				 */
				len = strlen(ಸಾಲು);
				ಯದಿ (len <= prev_len) {
					new_ಸಾಲು = prev_ಸಾಲು + (prev_len - len);
					ಯದಿ (strcmp(ಸಾಲು, new_ಸಾಲು) == 0) {
						/* it's a tail of the prev line */
						ಯದಿ (can_drop_dups && prev_len == len) {
							/* it's identical to prev line */
							ಮುಕ್ತ(ಸಾಲು);
							ಅನುವರ್ತ;
						}
						ಮುಕ್ತ(ಸಾಲು);
						ಸಾಲು = new_ಸಾಲು;
						/* continue using longer prev_line
						 * for future tail tests.
						 */
						ಗಚ್ಛ skip;
					}
				}
				prev_len = len;
				prev_ಸಾಲು = ಸಾಲು;
 skip: ;
			}
#ಅನ್ಯಥಾ
//TODO: lighter version which only drops total dups if can_drop_dups == true
#endif
			ಸಾಲುಗಳು = xrealloc_vector(ಸಾಲುಗಳು, 6, linecount);
			ಸಾಲುಗಳು[linecount++] = ಸಾಲು;
		}
		fclose_if_not_stdin(fp);
	} ಯಾವತ್ (*++argv);

#ಯದಿ ENABLE_FEATURE_SORT_BIG
	/* If no key, perform alphabetic sort */
	ಯದಿ (!ಕೀಲಿ_list)
		add_ಕೀಲಿ()->range[0] = 1;
	/* Handle -c */
	ಯದಿ (option_mask32 & FLAG_c) {
		ಪೂರ್ಣಾಙ್ಕ j = (option_mask32 & FLAG_u) ? -1 : 0;
		ಪ್ರತಿ (i = 1; i < linecount; i++) {
			ಯದಿ (compare_ಕೀಲಿಗಳು(&ಸಾಲುಗಳು[i-1], &ಸಾಲುಗಳು[i]) > j) {
				fprintf(stderr, "Check line %u\n", i);
				ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;
			}
		}
		ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
	}
#endif

	/* For stable sort, store original line position beyond terminating NUL */
	ಯದಿ (option_mask32 & FLAG_s) {
		ಪ್ರತಿ (i = 0; i < linecount; i++) {
			uint32_t *p32;
			ವರ್ಣ *ಸಾಲು;
			unsigned len;

			ಸಾಲು = ಸಾಲುಗಳು[i];
			len = (strlen(ಸಾಲು) + 4) & (~3u);
			ಸಾಲುಗಳು[i] = ಸಾಲು = xrealloc(ಸಾಲು, len + 4);
			p32 = (ಶೂನ್ಯ*)(ಸಾಲು + len);
			*p32 = i;
		}
		/*option_mask32 |= FLAG_no_tie_break;*/
		/* ^^^redundant: if FLAG_s, compare_keys() does no tie break */
	}

	/* Perform the actual sort */
	qsort(ಸಾಲುಗಳು, linecount, ಪರಿಮಾಣ(ಸಾಲುಗಳು[0]), compare_ಕೀಲಿಗಳು);

	/* Handle -u */
	ಯದಿ (option_mask32 & FLAG_u) {
		ಪೂರ್ಣಾಙ್ಕ j = 0;
		/* coreutils 6.3 drop lines for which only key is the same:
		 * - disabling last-resort compare, or else compare_keys()
		 * will be the same only for completely identical lines
		 * - disabling -s (same reasons)
		 */
		option_mask32 = (option_mask32 | FLAG_no_tie_break) & (~FLAG_s);
		ಪ್ರತಿ (i = 1; i < linecount; i++) {
			ಯದಿ (compare_ಕೀಲಿಗಳು(&ಸಾಲುಗಳು[j], &ಸಾಲುಗಳು[i]) == 0)
				ಮುಕ್ತ(ಸಾಲುಗಳು[i]);
			ಅನ್ಯಥಾ
				ಸಾಲುಗಳು[++j] = ಸಾಲುಗಳು[i];
		}
		ಯದಿ (linecount)
			linecount = j+1;
	}

	/* Print it */
#ಯದಿ ENABLE_FEATURE_SORT_BIG
	/* Open output file _after_ we read all input ones */
	ಯದಿ (option_mask32 & FLAG_o)
		xmove_fd(xopen(str_o, O_WRONLY|O_CREAT|O_TRUNC), STDOUT_FILENO);
#endif
	{
		ಪೂರ್ಣಾಙ್ಕ ch = (option_mask32 & FLAG_z) ? '\0' : '\n';
		ಪ್ರತಿ (i = 0; i < linecount; i++)
			printf("%s%c", ಸಾಲುಗಳು[i], ch);
	}

	fflush_stdout_and_exit_SUCCESS();
}
