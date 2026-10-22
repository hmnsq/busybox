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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"

/* These are sort types */
ਗਿਣਤੀ {
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
	FLAG_r  = 1 << 10,      /*ਉਲਟ*/
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

ਸਥਿਰ ਅਚਲ ਅੱਖਰ ਛਾਂਟੋ_opt_str[] ALIGN1 = "^"
			"nghMVucszbrdfimS:T:o:k:*t:"
			"\0" "o--o:t--t"/*-t, -o: at most one of each*/;
/*
 * OPT_STR must not be string literal, needs to have stable address:
 * code uses "strchr(OPT_STR,c) - OPT_STR" idiom.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPT_STR (ਛਾਂਟੋ_opt_str + 1)

#ਜੇ ENABLE_FEATURE_ਛਾਂਟੋ_BIG
ਸਥਿਰ ਅੱਖਰ ਕੁੰਜੀ_separator;

ਸਥਿਰ ਬਣਤਰ ਛਾਂਟੋ_ਕੁੰਜੀ {
	ਬਣਤਰ ਛਾਂਟੋ_ਕੁੰਜੀ *ਅਗਲਾ_ਕੁੰਜੀ;  /* linked list */
	ਬਿਨਾਨਿਸ਼ਾਨੀ range[4];          /* start word, start char, end word, end char */
	ਬਿਨਾਨਿਸ਼ਾਨੀ flags;
} *ਕੁੰਜੀ_list;


/* This is a NOEXEC applet. Be very careful! */


ਸਥਿਰ ਅੱਖਰ *get_ਕੁੰਜੀ(ਅੱਖਰ *str, ਬਣਤਰ ਛਾਂਟੋ_ਕੁੰਜੀ *ਕੁੰਜੀ, ਪੂਰਨ flags)
{
	ਪੂਰਨ ਸ਼ੁਰੂ = ਸ਼ੁਰੂ; /* for compiler */
	ਪੂਰਨ ਸਿਰਾ;
	ਪੂਰਨ len, j;
	ਬਿਨਾਨਿਸ਼ਾਨੀ i;

	/* Special case whole string, so we don't have to make a copy */
	ਜੇ (ਕੁੰਜੀ->range[0] == 1 && !ਕੁੰਜੀ->range[1] && !ਕੁੰਜੀ->range[2] && !ਕੁੰਜੀ->range[3]
	 && !(flags & (FLAG_b | FLAG_d | FLAG_f | FLAG_i | FLAG_bb))
	) {
		ਵਾਪਸ str;
	}

	/* Find start of key on first pass, end on second pass */
	len = ਲੜੀ_ਲੰਬਾਈ(str);
	ਲਈ (j = 0; j < 2; j++) {
		ਜੇ (!ਕੁੰਜੀ->range[2*j])
			ਸਿਰਾ = len;
		/* Loop through fields */
		ਨਹੀਂਤਾਂ {
			ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ch = 0;

			ਸਿਰਾ = 0;
			ਲਈ (i = 1; i < ਕੁੰਜੀ->range[2*j] + j; i++) {
				ਜੇ (ਕੁੰਜੀ_separator) {
					/* Skip body of key and separator */
					ਜਦੋਂ ((ch = str[ਸਿਰਾ]) != '\0') {
							ਸਿਰਾ++;
						ਜੇ (ch == ਕੁੰਜੀ_separator)
							ਰੋਕੋ;
					}
				} ਨਹੀਂਤਾਂ {
					/* Skip leading blanks */
					ਜਦੋਂ (ਖਾਲੀ_ਥਾਂ_ਹੈ(str[ਸਿਰਾ]))
						ਸਿਰਾ++;
					/* Skip body of key */
					ਜਦੋਂ (str[ਸਿਰਾ] != '\0') {
						ਜੇ (ਖਾਲੀ_ਥਾਂ_ਹੈ(str[ਸਿਰਾ]))
							ਰੋਕੋ;
						ਸਿਰਾ++;
					}
				}
			}
			/* Remove last delim: "abc:def:" => "abc:def" */
			ਜੇ (j && ch) {
				//if (str[end-1] != key_separator)
				//  bb_error_msg(_and_die("BUG! "
				//  "str[start:%d,end:%d]:'%.*s'",
				//  start, end, (int)(end-start), &str[start]);
				ਸਿਰਾ--;
			}
		}
		ਜੇ (!j) ਸ਼ੁਰੂ = ਸਿਰਾ;
	}
	/* Strip leading whitespace if necessary */
	ਜੇ (flags & FLAG_b)
		/* not using skip_whitespace() for speed */
		ਜਦੋਂ (ਖਾਲੀ_ਥਾਂ_ਹੈ(str[ਸ਼ੁਰੂ])) ਸ਼ੁਰੂ++;
	/* Strip trailing whitespace if necessary */
	ਜੇ (flags & FLAG_bb)
		ਜਦੋਂ (ਸਿਰਾ > ਸ਼ੁਰੂ && ਖਾਲੀ_ਥਾਂ_ਹੈ(str[ਸਿਰਾ-1])) ਸਿਰਾ--;
	/* -kSTART,N.ENDCHAR: honor ENDCHAR (1-based) */
	ਜੇ (ਕੁੰਜੀ->range[3]) {
		ਸਿਰਾ = ਕੁੰਜੀ->range[3];
		ਜੇ (ਸਿਰਾ > len) ਸਿਰਾ = len;
	}
	/* -kN.STARTCHAR[,...]: honor STARTCHAR (1-based) */
	ਜੇ (ਕੁੰਜੀ->range[1]) {
		ਸ਼ੁਰੂ += ਕੁੰਜੀ->range[1] - 1;
		ਜੇ (ਸ਼ੁਰੂ > len) ਸ਼ੁਰੂ = len;
	}
	/* Make the copy */
	ਜੇ (ਸਿਰਾ < ਸ਼ੁਰੂ)
		ਸਿਰਾ = ਸ਼ੁਰੂ;
	str = xstrndup(str+ਸ਼ੁਰੂ, ਸਿਰਾ-ਸ਼ੁਰੂ);
	/* Handle -d */
	ਜੇ (flags & FLAG_d) {
		ਲਈ (ਸ਼ੁਰੂ = ਸਿਰਾ = 0; str[ਸਿਰਾ]; ਸਿਰਾ++)
			ਜੇ (ਖਾਲੀ_ਥਾਂ_ਹੈ(str[ਸਿਰਾ]) || ਅੱਖਰ_ਜਾਂ_ਅੰਕ(str[ਸਿਰਾ]))
				str[ਸ਼ੁਰੂ++] = str[ਸਿਰਾ];
		str[ਸ਼ੁਰੂ] = '\0';
	}
	/* Handle -i */
	ਜੇ (flags & FLAG_i) {
		ਲਈ (ਸ਼ੁਰੂ = ਸਿਰਾ = 0; str[ਸਿਰਾ]; ਸਿਰਾ++)
			ਜੇ (isprint_asciionly(str[ਸਿਰਾ]))
				str[ਸ਼ੁਰੂ++] = str[ਸਿਰਾ];
		str[ਸ਼ੁਰੂ] = '\0';
	}
	/* Handle -f */
	ਜੇ (flags & FLAG_f)
		ਲਈ (i = 0; str[i]; i++)
			str[i] = ਵੱਡਾ_ਅੱਖਰ(str[i]);

	ਵਾਪਸ str;
}

ਸਥਿਰ ਬਣਤਰ ਛਾਂਟੋ_ਕੁੰਜੀ *ਜੋੜੋ_ਕੁੰਜੀ(ਖਾਲੀ)
{
	ਬਣਤਰ ਛਾਂਟੋ_ਕੁੰਜੀ **pkey = &ਕੁੰਜੀ_list;
	ਜਦੋਂ (*pkey)
		pkey = &((*pkey)->ਅਗਲਾ_ਕੁੰਜੀ);
	ਵਾਪਸ *pkey = xzalloc(ਆਕਾਰ(ਬਣਤਰ ਛਾਂਟੋ_ਕੁੰਜੀ));
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GET_ਸਤਰ(fp) \
	((option_mask32 & FLAG_z) \
	? bb_get_chunk_from_file(fp, ਨਿਰਾ) \
	: xmalloc_fgetline(fp))
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GET_ਸਤਰ(fp) xmalloc_fgetline(fp)
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_FEATURE_ਛਾਂਟੋ_BIG
ਸਥਿਰ ਪੂਰਨ scale_suffix(ਅਚਲ ਅੱਖਰ *ਪੂਛ)
{
	ਸਥਿਰ ਅਚਲ ਅੱਖਰ suffix[] ALIGN1 = "kmgtpezy";
	ਅਚਲ ਅੱਖਰ *s;
	ਪੂਰਨ n;

	ਜੇ (!ਪੂਛ[0])
		ਵਾਪਸ -1;
	s = strchr(suffix, ਪੂਛ[0] | 0x20);
	ਜੇ (!s)
		ਵਾਪਸ -1;
	n = s - suffix;
	ਜੇ (n != 0 && ਪੂਛ[0] >= 'a')
		ਵਾਪਸ -1; /* mg... not accepted, only MG... */
	ਵਾਪਸ n;
}
#ਜੇ_ਅੰਤ

/* Iterate through keys list and perform comparisons */
ਸਥਿਰ ਪੂਰਨ compare_ਕੁੰਜੀਆਂ(ਅਚਲ ਖਾਲੀ *xarg, ਅਚਲ ਖਾਲੀ *yarg)
{
	ਪੂਰਨ flags = option_mask32, retval = 0;
	ਅੱਖਰ *x, *y;

#ਜੇ ENABLE_FEATURE_ਛਾਂਟੋ_BIG
	ਬਣਤਰ ਛਾਂਟੋ_ਕੁੰਜੀ *ਕੁੰਜੀ;

	ਲਈ (ਕੁੰਜੀ = ਕੁੰਜੀ_list; !retval && ਕੁੰਜੀ; ਕੁੰਜੀ = ਕੁੰਜੀ->ਅਗਲਾ_ਕੁੰਜੀ) {
		flags = ਕੁੰਜੀ->flags ? ਕੁੰਜੀ->flags : option_mask32;
		/* Chop out and modify key chunks, handling -dfib */
		x = get_ਕੁੰਜੀ(*(ਅੱਖਰ **)xarg, ਕੁੰਜੀ, flags);
		y = get_ਕੁੰਜੀ(*(ਅੱਖਰ **)yarg, ਕੁੰਜੀ, flags);
#ਨਹੀਂਤਾਂ
	/* This curly bracket serves no purpose but to match the nesting
	 * level of the for () loop we're not using */
	{
		x = *(ਅੱਖਰ **)xarg;
		y = *(ਅੱਖਰ **)yarg;
#ਜੇ_ਅੰਤ
		/* Perform actual comparison */
		ਚੋਣ (flags & (FLAG_n | FLAG_g | FLAG_h | FLAG_M | FLAG_V)) {
		ਮੂਲ:
			bb_simple_error_ਸੁਨੇਹਾ_and_die("unknown sort type");
			ਰੋਕੋ;
#ਜੇ defined(HAVE_STRVERSCMP) && HAVE_STRVERSCMP == 1
		ਹਾਲਤ FLAG_V:
			retval = strverscmp(x, y);
			ਰੋਕੋ;
#ਜੇ_ਅੰਤ
		/* Ascii sort */
		ਹਾਲਤ 0:
#ਜੇ ENABLE_LOCALE_SUPPORT
			retval = strcoll(x, y);
#ਨਹੀਂਤਾਂ
			retval = strcmp(x, y);
#ਜੇ_ਅੰਤ
			ਰੋਕੋ;
#ਜੇ ENABLE_FEATURE_ਛਾਂਟੋ_BIG
		ਹਾਲਤ FLAG_g:
		ਹਾਲਤ FLAG_h: {
			ਅੱਖਰ *xx, *yy;
//TODO: needs setlocale(LC_NUMERIC, "C")?
			ਦੁੱਗਣਾ dx = strtod(x, &xx);
			ਦੁੱਗਣਾ dy = strtod(y, &yy);
			/* not numbers < NaN < -infinity < numbers < +infinity) */
			ਜੇ (x == xx)
				retval = (y == yy ? 0 : -1);
			ਨਹੀਂਤਾਂ ਜੇ (y == yy)
				retval = 1;
			/* Check for isnan */
			ਨਹੀਂਤਾਂ ਜੇ (dx != dx)
				retval = (dy != dy) ? 0 : -1;
			ਨਹੀਂਤਾਂ ਜੇ (dy != dy)
				retval = 1;
			ਨਹੀਂਤਾਂ {
				ਜੇ (flags & FLAG_h) {
					ਪੂਰਨ xs = scale_suffix(xx);
					ਪੂਰਨ ys = scale_suffix(yy);
					ਜੇ (xs != ys) {
						retval = xs - ys;
						ਰੋਕੋ;
					}
				}
				/* Check for infinity.  Could underflow, but it avoids libm. */
				ਜੇ (1.0 / dx == 0.0) {
					ਜੇ (dx < 0)
						retval = (1.0 / dy == 0.0 && dy < 0) ? 0 : -1;
					ਨਹੀਂਤਾਂ
						retval = (1.0 / dy == 0.0 && dy > 0) ? 0 : 1;
				} ਨਹੀਂਤਾਂ ਜੇ (1.0 / dy == 0.0)
					retval = (dy < 0) ? 1 : -1;
				ਨਹੀਂਤਾਂ
					retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			}
			ਰੋਕੋ;
		}
		ਹਾਲਤ FLAG_M: {
			ਬਣਤਰ tm thyme;
			ਪੂਰਨ dx;
			ਅੱਖਰ *xx, *yy;

			xx = strptime(skip_whitespace(x), "%b", &thyme);
			dx = thyme.tm_mon;
			yy = strptime(skip_whitespace(y), "%b", &thyme);
			ਜੇ (!xx)
				retval = (!yy) ? 0 : -1;
			ਨਹੀਂਤਾਂ ਜੇ (!yy)
				retval = 1;
			ਨਹੀਂਤਾਂ
				retval = dx - thyme.tm_mon;
			ਰੋਕੋ;
		}
		/* Full floating point version of -n */
		ਹਾਲਤ FLAG_n: {
			ਦੁੱਗਣਾ dx = atof(x);
			ਦੁੱਗਣਾ dy = atof(y);
			retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			ਰੋਕੋ;
		}
		} /* switch */
		/* Free key copies. */
		ਜੇ (x != *(ਅੱਖਰ **)xarg) ਮੁਕਤ(x);
		ਜੇ (y != *(ਅੱਖਰ **)yarg) ਮੁਕਤ(y);
		/* if (retval) break; - done by for () anyway */
#ਨਹੀਂਤਾਂ
		/* Integer version of -n for tiny systems */
		ਹਾਲਤ FLAG_n:
			retval = atoi(x) - atoi(y);
			ਰੋਕੋ;
		} /* switch */
#ਜੇ_ਅੰਤ
	} /* for */

	ਜੇ (retval == 0) {
		/* So far lines are "the same" */

		ਜੇ (option_mask32 & FLAG_s) {
			/* "Stable sort": later line is "greater than",
			 * IOW: do not allow qsort() to swap equal lines.
			 */
			uint32_t *p32;
			uint32_t x32, y32;
			ਅੱਖਰ *ਸਤਰ;
			ਬਿਨਾਨਿਸ਼ਾਨੀ len;

			ਸਤਰ = *(ਅੱਖਰ**)xarg;
			len = (ਲੜੀ_ਲੰਬਾਈ(ਸਤਰ) + 4) & (~3u);
			p32 = (ਖਾਲੀ*)(ਸਤਰ + len);
			x32 = *p32;
			ਸਤਰ = *(ਅੱਖਰ**)yarg;
			len = (ਲੜੀ_ਲੰਬਾਈ(ਸਤਰ) + 4) & (~3u);
			p32 = (ਖਾਲੀ*)(ਸਤਰ + len);
			y32 = *p32;

			/* If x > y, 1, else -1 */
			retval = (x32 > y32) * 2 - 1;
			/* Here, -r has no effect! */
			ਵਾਪਸ retval;
		}
		ਜੇ (!(option_mask32 & FLAG_no_tie_break)) {
			/* fallback sort */
			flags = option_mask32;
			retval = strcmp(*(ਅੱਖਰ **)xarg, *(ਅੱਖਰ **)yarg);
		}
	}

	ਜੇ (flags & FLAG_r)
		ਵਾਪਸ -retval;

	ਵਾਪਸ retval;
}

#ਜੇ ENABLE_FEATURE_ਛਾਂਟੋ_BIG
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ str2u(ਅੱਖਰ **str)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ lu;
	ਜੇ (!ਅੰਕ_ਹੈ((*str)[0]))
		bb_simple_error_ਸੁਨੇਹਾ_and_die("bad field specification");
	lu = strtoul(*str, str, 10);
	ਜੇ ((ਆਕਾਰ(ਲੰਮਾ) > ਆਕਾਰ(ਪੂਰਨ) && lu > ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ) || !lu)
		bb_simple_error_ਸੁਨੇਹਾ_and_die("bad field specification");
	ਵਾਪਸ lu;
}
#ਜੇ_ਅੰਤ

ਪੂਰਨ ਛਾਂਟੋ_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ ਛਾਂਟੋ_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
	ਅੱਖਰ **ਸਤਰਾਂ;
	ਅੱਖਰ *str_ignored, *str_o, *str_t;
	llist_t *lst_k = ਨਿਰਾ;
	ਪੂਰਨ i;
	ਪੂਰਨ linecount;
	ਬਿਨਾਨਿਸ਼ਾਨੀ opts;
#ਜੇ ENABLE_FEATURE_ਛਾਂਟੋ_OPTIMIZE_ਸਮ੍ਰਿਤੀ
	bool can_drop_dups;
	ਮਾਪ_t prev_len = 0;
	ਅੱਖਰ *prev_ਸਤਰ = (ਅੱਖਰ*) "";
	/* Postpone optimizing if the input is small, < 16k lines:
	 * even just free()ing duplicate lines takes time.
	 */
	ਮਾਪ_t ਗਿਣਤੀ_ਮੰਜ਼ਿਲ_optimize_dups = 0x3fff;
#ਜੇ_ਅੰਤ

	xfunc_error_retval = 2;

	/* Parse command line options */
	opts = getopt32(argv,
			ਛਾਂਟੋ_opt_str,
			&str_ignored, &str_ignored, &str_o, &lst_k, &str_t
	);
#ਜੇ ENABLE_FEATURE_ਛਾਂਟੋ_OPTIMIZE_ਸਮ੍ਰਿਤੀ
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
	ਜੇ (opts & FLAG_s)
		ਗਿਣਤੀ_ਮੰਜ਼ਿਲ_optimize_dups = (ਮਾਪ_t)-1L;
#ਜੇ_ਅੰਤ
	/* global b strips leading and trailing spaces */
	ਜੇ (opts & FLAG_b)
		option_mask32 |= FLAG_bb;
#ਜੇ ENABLE_FEATURE_ਛਾਂਟੋ_BIG
	ਜੇ (opts & FLAG_t) {
		ਜੇ (!str_t[0] || str_t[1])
			bb_simple_error_ਸੁਨੇਹਾ_and_die("bad -t parameter");
		ਕੁੰਜੀ_separator = str_t[0];
	}
	/* note: below this point we use option_mask32, not opts,
	 * since that reduces register pressure and makes code smaller */

	/* Parse sort key */
	ਜਦੋਂ (lst_k) {
		ਗਿਣਤੀ {
			FLAG_allowed_for_k =
				FLAG_n | /* Numeric sort */
				FLAG_g | /* Sort using strtod() */
				FLAG_h | /* Sort using strtod(), plus KMGT suffixes */
				FLAG_M | /* Sort date */
				FLAG_b | /* Ignore leading blanks */
				FLAG_r | /*ਉਲਟ*/
				FLAG_d | /* Ignore !(isalnum()|isspace()) */
				FLAG_f | /* Force uppercase */
				FLAG_i | /* Ignore !isprint() */
			0
		};
		ਬਣਤਰ ਛਾਂਟੋ_ਕੁੰਜੀ *ਕੁੰਜੀ = ਜੋੜੋ_ਕੁੰਜੀ();
		ਅੱਖਰ *str_k = llist_ਕੱਢੋ(&lst_k);

		i = 0; /* i==0 before comma, 1 after (-k3,6) */
		ਜਦੋਂ (*str_k) {
			/* Start of range */
			/* Cannot use bb_strtou - suffix can be a letter */
			ਕੁੰਜੀ->range[2*i] = str2u(&str_k);
			ਜੇ (*str_k == '.') {
				str_k++;
				ਕੁੰਜੀ->range[2*i+1] = str2u(&str_k);
			}
			ਜਦੋਂ (*str_k) {
				ਪੂਰਨ flag;
				ਅਚਲ ਅੱਖਰ *idx;

				ਜੇ (*str_k == ',' && !i++) {
					str_k++;
					ਰੋਕੋ;
				} /* no else needed: fall through to syntax error
					because comma isn't in OPT_STR */
				idx = strchr(OPT_STR, *str_k);
				ਜੇ (!idx)
					bb_simple_error_ਸੁਨੇਹਾ_and_die("unknown key option");
				flag = 1 << (idx - OPT_STR);
				ਜੇ (flag & ~FLAG_allowed_for_k)
					bb_simple_error_ਸੁਨੇਹਾ_and_die("unknown sort type");
				/* b after ',' means strip _trailing_ space */
				ਜੇ (i && flag == FLAG_b)
					flag = FLAG_bb;
				ਕੁੰਜੀ->flags |= flag;
				str_k++;
			}
		}
	}
#ਜੇ_ਅੰਤ

	/* Open input files and read data */
	argv += optind;
	ਜੇ (!*argv)
		*--argv = (ਅੱਖਰ*)"-";
	linecount = 0;
	ਸਤਰਾਂ = ਨਿਰਾ;
	ਕਰੋ {
		/* coreutils 6.9 compat: abort on first open error,
		 * do not continue to next file: */
		FILE *fp = xfopen_stdin(*argv);
		ਲਈ (;;) {
			ਅੱਖਰ *ਸਤਰ = GET_ਸਤਰ(fp);
			ਜੇ (!ਸਤਰ)
				ਰੋਕੋ;

#ਜੇ ENABLE_FEATURE_ਛਾਂਟੋ_OPTIMIZE_ਸਮ੍ਰਿਤੀ
			ਜੇ (ਗਿਣਤੀ_ਮੰਜ਼ਿਲ_optimize_dups != 0)
				ਗਿਣਤੀ_ਮੰਜ਼ਿਲ_optimize_dups--;
			ਜੇ (ਗਿਣਤੀ_ਮੰਜ਼ਿਲ_optimize_dups == 0) {
				ਮਾਪ_t len;
				ਅੱਖਰ *new_ਸਤਰ;

				/* On kernel/linux/arch/ *.[ch] files,
				 * this reduces memory usage by 6%.
				 *  yes | head -99999999 | sort
				 * goes down from 1900Mb to 380 Mb.
				 */
				len = ਲੜੀ_ਲੰਬਾਈ(ਸਤਰ);
				ਜੇ (len <= prev_len) {
					new_ਸਤਰ = prev_ਸਤਰ + (prev_len - len);
					ਜੇ (strcmp(ਸਤਰ, new_ਸਤਰ) == 0) {
						/* it's a tail of the prev line */
						ਜੇ (can_drop_dups && prev_len == len) {
							/* it's identical to prev line */
							ਮੁਕਤ(ਸਤਰ);
							ਜਾਰੀ;
						}
						ਮੁਕਤ(ਸਤਰ);
						ਸਤਰ = new_ਸਤਰ;
						/* continue using longer prev_line
						 * for future tail tests.
						 */
						ਜਾਓ skip;
					}
				}
				prev_len = len;
				prev_ਸਤਰ = ਸਤਰ;
 skip: ;
			}
#ਨਹੀਂਤਾਂ
//TODO: lighter version which only drops total dups if can_drop_dups == true
#ਜੇ_ਅੰਤ
			ਸਤਰਾਂ = xrealloc_ਸਦਿਸ਼(ਸਤਰਾਂ, 6, linecount);
			ਸਤਰਾਂ[linecount++] = ਸਤਰ;
		}
		fclose_if_not_stdin(fp);
	} ਜਦੋਂ (*++argv);

#ਜੇ ENABLE_FEATURE_ਛਾਂਟੋ_BIG
	/* If no key, perform alphabetic sort */
	ਜੇ (!ਕੁੰਜੀ_list)
		ਜੋੜੋ_ਕੁੰਜੀ()->range[0] = 1;
	/* Handle -c */
	ਜੇ (option_mask32 & FLAG_c) {
		ਪੂਰਨ j = (option_mask32 & FLAG_u) ? -1 : 0;
		ਲਈ (i = 1; i < linecount; i++) {
			ਜੇ (compare_ਕੁੰਜੀਆਂ(&ਸਤਰਾਂ[i-1], &ਸਤਰਾਂ[i]) > j) {
				fprintf(stderr, "Check line %u\n", i);
				ਵਾਪਸ EXIT_FAILURE;
			}
		}
		ਵਾਪਸ EXIT_SUCCESS;
	}
#ਜੇ_ਅੰਤ

	/* For stable sort, store original line position beyond terminating NUL */
	ਜੇ (option_mask32 & FLAG_s) {
		ਲਈ (i = 0; i < linecount; i++) {
			uint32_t *p32;
			ਅੱਖਰ *ਸਤਰ;
			ਬਿਨਾਨਿਸ਼ਾਨੀ len;

			ਸਤਰ = ਸਤਰਾਂ[i];
			len = (ਲੜੀ_ਲੰਬਾਈ(ਸਤਰ) + 4) & (~3u);
			ਸਤਰਾਂ[i] = ਸਤਰ = xrealloc(ਸਤਰ, len + 4);
			p32 = (ਖਾਲੀ*)(ਸਤਰ + len);
			*p32 = i;
		}
		/*option_mask32 |= FLAG_no_tie_break;*/
		/* ^^^redundant: if FLAG_s, compare_keys() does no tie break */
	}

	/* Perform the actual sort */
	qsort(ਸਤਰਾਂ, linecount, ਆਕਾਰ(ਸਤਰਾਂ[0]), compare_ਕੁੰਜੀਆਂ);

	/* Handle -u */
	ਜੇ (option_mask32 & FLAG_u) {
		ਪੂਰਨ j = 0;
		/* coreutils 6.3 drop lines for which only key is the same:
		 * - disabling last-resort compare, or else compare_keys()
		 * will be the same only for completely identical lines
		 * - disabling -s (same reasons)
		 */
		option_mask32 = (option_mask32 | FLAG_no_tie_break) & (~FLAG_s);
		ਲਈ (i = 1; i < linecount; i++) {
			ਜੇ (compare_ਕੁੰਜੀਆਂ(&ਸਤਰਾਂ[j], &ਸਤਰਾਂ[i]) == 0)
				ਮੁਕਤ(ਸਤਰਾਂ[i]);
			ਨਹੀਂਤਾਂ
				ਸਤਰਾਂ[++j] = ਸਤਰਾਂ[i];
		}
		ਜੇ (linecount)
			linecount = j+1;
	}

	/* Print it */
#ਜੇ ENABLE_FEATURE_ਛਾਂਟੋ_BIG
	/* Open output file _after_ we read all input ones */
	ਜੇ (option_mask32 & FLAG_o)
		xmove_fd(xopen(str_o, O_WRONLY|O_CREAT|O_TRUNC), STDOUT_FILENO);
#ਜੇ_ਅੰਤ
	{
		ਪੂਰਨ ch = (option_mask32 & FLAG_z) ? '\0' : '\n';
		ਲਈ (i = 0; i < linecount; i++)
			ਲਿਖੋ("%s%c", ਸਤਰਾਂ[i], ch);
	}

	fflush_stdout_and_exit_SUCCESS();
}
