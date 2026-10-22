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
ಎಣಿಕೆ {
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
	FLAG_r  = 1 << 10,      /*ಹಿಮ್ಮುಖ*/
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

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ ವಿಂಗಡಿಸು_opt_str[] ALIGN1 = "^"
			"nghMVucszbrdfimS:T:o:k:*t:"
			"\0" "o--o:t--t"/*-t, -o: at most one of each*/;
/*
 * OPT_STR must not be string literal, needs to have stable address:
 * code uses "strchr(OPT_STR,c) - OPT_STR" idiom.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು OPT_STR (ವಿಂಗಡಿಸು_opt_str + 1)

#ಆದರೆ ENABLE_FEATURE_ವಿಂಗಡಿಸು_BIG
ಸ್ಥಿರ ಅಕ್ಷರ ಕೀಲಿ_separator;

ಸ್ಥಿರ ರಚನೆ ವಿಂಗಡಿಸು_ಕೀಲಿ {
	ರಚನೆ ವಿಂಗಡಿಸು_ಕೀಲಿ *ಮುಂದಿನ_ಕೀಲಿ;  /* linked list */
	ಚಿಹ್ನೆರಹಿತ range[4];          /* start word, start char, end word, end char */
	ಚಿಹ್ನೆರಹಿತ flags;
} *ಕೀಲಿ_list;


/* This is a NOEXEC applet. Be very careful! */


ಸ್ಥಿರ ಅಕ್ಷರ *get_ಕೀಲಿ(ಅಕ್ಷರ *str, ರಚನೆ ವಿಂಗಡಿಸು_ಕೀಲಿ *ಕೀಲಿ, ಪೂರ್ಣಾಂಕ flags)
{
	ಪೂರ್ಣಾಂಕ ಆರಂಭ = ಆರಂಭ; /* for compiler */
	ಪೂರ್ಣಾಂಕ ಕೊನೆ;
	ಪೂರ್ಣಾಂಕ len, j;
	ಚಿಹ್ನೆರಹಿತ i;

	/* Special case whole string, so we don't have to make a copy */
	ಆದರೆ (ಕೀಲಿ->range[0] == 1 && !ಕೀಲಿ->range[1] && !ಕೀಲಿ->range[2] && !ಕೀಲಿ->range[3]
	 && !(flags & (FLAG_b | FLAG_d | FLAG_f | FLAG_i | FLAG_bb))
	) {
		ಹಿಂತಿರುಗಿ str;
	}

	/* Find start of key on first pass, end on second pass */
	len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(str);
	ಗಾಗಿ (j = 0; j < 2; j++) {
		ಆದರೆ (!ಕೀಲಿ->range[2*j])
			ಕೊನೆ = len;
		/* Loop through fields */
		ಇಲ್ಲದಿದ್ದರೆ {
			ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ch = 0;

			ಕೊನೆ = 0;
			ಗಾಗಿ (i = 1; i < ಕೀಲಿ->range[2*j] + j; i++) {
				ಆದರೆ (ಕೀಲಿ_separator) {
					/* Skip body of key and separator */
					ಆಗ ((ch = str[ಕೊನೆ]) != '\0') {
							ಕೊನೆ++;
						ಆದರೆ (ch == ಕೀಲಿ_separator)
							ನಿಲ್ಲಿಸು;
					}
				} ಇಲ್ಲದಿದ್ದರೆ {
					/* Skip leading blanks */
					ಆಗ (ಖಾಲಿಯೇ(str[ಕೊನೆ]))
						ಕೊನೆ++;
					/* Skip body of key */
					ಆಗ (str[ಕೊನೆ] != '\0') {
						ಆದರೆ (ಖಾಲಿಯೇ(str[ಕೊನೆ]))
							ನಿಲ್ಲಿಸು;
						ಕೊನೆ++;
					}
				}
			}
			/* Remove last delim: "abc:def:" => "abc:def" */
			ಆದರೆ (j && ch) {
				//if (str[end-1] != key_separator)
				//  bb_error_msg(_and_die("BUG! "
				//  "str[start:%d,end:%d]:'%.*s'",
				//  start, end, (int)(end-start), &str[start]);
				ಕೊನೆ--;
			}
		}
		ಆದರೆ (!j) ಆರಂಭ = ಕೊನೆ;
	}
	/* Strip leading whitespace if necessary */
	ಆದರೆ (flags & FLAG_b)
		/* not using skip_whitespace() for speed */
		ಆಗ (ಖಾಲಿಯೇ(str[ಆರಂಭ])) ಆರಂಭ++;
	/* Strip trailing whitespace if necessary */
	ಆದರೆ (flags & FLAG_bb)
		ಆಗ (ಕೊನೆ > ಆರಂಭ && ಖಾಲಿಯೇ(str[ಕೊನೆ-1])) ಕೊನೆ--;
	/* -kSTART,N.ENDCHAR: honor ENDCHAR (1-based) */
	ಆದರೆ (ಕೀಲಿ->range[3]) {
		ಕೊನೆ = ಕೀಲಿ->range[3];
		ಆದರೆ (ಕೊನೆ > len) ಕೊನೆ = len;
	}
	/* -kN.STARTCHAR[,...]: honor STARTCHAR (1-based) */
	ಆದರೆ (ಕೀಲಿ->range[1]) {
		ಆರಂಭ += ಕೀಲಿ->range[1] - 1;
		ಆದರೆ (ಆರಂಭ > len) ಆರಂಭ = len;
	}
	/* Make the copy */
	ಆದರೆ (ಕೊನೆ < ಆರಂಭ)
		ಕೊನೆ = ಆರಂಭ;
	str = xstrndup(str+ಆರಂಭ, ಕೊನೆ-ಆರಂಭ);
	/* Handle -d */
	ಆದರೆ (flags & FLAG_d) {
		ಗಾಗಿ (ಆರಂಭ = ಕೊನೆ = 0; str[ಕೊನೆ]; ಕೊನೆ++)
			ಆದರೆ (ಖಾಲಿಯೇ(str[ಕೊನೆ]) || ಅಕ್ಷರವೇ_ಅಂಕಿಯೇ(str[ಕೊನೆ]))
				str[ಆರಂಭ++] = str[ಕೊನೆ];
		str[ಆರಂಭ] = '\0';
	}
	/* Handle -i */
	ಆದರೆ (flags & FLAG_i) {
		ಗಾಗಿ (ಆರಂಭ = ಕೊನೆ = 0; str[ಕೊನೆ]; ಕೊನೆ++)
			ಆದರೆ (isprint_asciionly(str[ಕೊನೆ]))
				str[ಆರಂಭ++] = str[ಕೊನೆ];
		str[ಆರಂಭ] = '\0';
	}
	/* Handle -f */
	ಆದರೆ (flags & FLAG_f)
		ಗಾಗಿ (i = 0; str[i]; i++)
			str[i] = ದೊಡ್ಡ_ಅಕ್ಷರ(str[i]);

	ಹಿಂತಿರುಗಿ str;
}

ಸ್ಥಿರ ರಚನೆ ವಿಂಗಡಿಸು_ಕೀಲಿ *ಕೂಡಿಸು_ಕೀಲಿ(ಶೂನ್ಯ)
{
	ರಚನೆ ವಿಂಗಡಿಸು_ಕೀಲಿ **pkey = &ಕೀಲಿ_list;
	ಆಗ (*pkey)
		pkey = &((*pkey)->ಮುಂದಿನ_ಕೀಲಿ);
	ಹಿಂತಿರುಗಿ *pkey = xzalloc(ಗಾತ್ರ(ರಚನೆ ವಿಂಗಡಿಸು_ಕೀಲಿ));
}

#ವ್ಯಾಖ್ಯಾನಿಸು GET_ಸಾಲು(fp) \
	((option_mask32 & FLAG_z) \
	? bb_get_chunk_from_file(fp, ಖಾಲಿ) \
	: xmalloc_fgetline(fp))
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು GET_ಸಾಲು(fp) xmalloc_fgetline(fp)
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_FEATURE_ವಿಂಗಡಿಸು_BIG
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ scale_suffix(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಾಲ)
{
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ suffix[] ALIGN1 = "kmgtpezy";
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s;
	ಪೂರ್ಣಾಂಕ n;

	ಆದರೆ (!ಬಾಲ[0])
		ಹಿಂತಿರುಗಿ -1;
	s = strchr(suffix, ಬಾಲ[0] | 0x20);
	ಆದರೆ (!s)
		ಹಿಂತಿರುಗಿ -1;
	n = s - suffix;
	ಆದರೆ (n != 0 && ಬಾಲ[0] >= 'a')
		ಹಿಂತಿರುಗಿ -1; /* mg... not accepted, only MG... */
	ಹಿಂತಿರುಗಿ n;
}
#ಆದರೆ_ಅಂತ್ಯ

/* Iterate through keys list and perform comparisons */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ compare_ಕೀಲಿಗಳು(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *xarg, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *yarg)
{
	ಪೂರ್ಣಾಂಕ flags = option_mask32, retval = 0;
	ಅಕ್ಷರ *x, *y;

#ಆದರೆ ENABLE_FEATURE_ವಿಂಗಡಿಸು_BIG
	ರಚನೆ ವಿಂಗಡಿಸು_ಕೀಲಿ *ಕೀಲಿ;

	ಗಾಗಿ (ಕೀಲಿ = ಕೀಲಿ_list; !retval && ಕೀಲಿ; ಕೀಲಿ = ಕೀಲಿ->ಮುಂದಿನ_ಕೀಲಿ) {
		flags = ಕೀಲಿ->flags ? ಕೀಲಿ->flags : option_mask32;
		/* Chop out and modify key chunks, handling -dfib */
		x = get_ಕೀಲಿ(*(ಅಕ್ಷರ **)xarg, ಕೀಲಿ, flags);
		y = get_ಕೀಲಿ(*(ಅಕ್ಷರ **)yarg, ಕೀಲಿ, flags);
#ಇಲ್ಲದಿದ್ದರೆ
	/* This curly bracket serves no purpose but to match the nesting
	 * level of the for () loop we're not using */
	{
		x = *(ಅಕ್ಷರ **)xarg;
		y = *(ಅಕ್ಷರ **)yarg;
#ಆದರೆ_ಅಂತ್ಯ
		/* Perform actual comparison */
		ಆಯ್ಕೆ (flags & (FLAG_n | FLAG_g | FLAG_h | FLAG_M | FLAG_V)) {
		ಪೂರ್ವನಿಯೋಜಿತ:
			bb_simple_error_ಸಂದೇಶ_and_die("unknown sort type");
			ನಿಲ್ಲಿಸು;
#ಆದರೆ defined(HAVE_STRVERSCMP) && HAVE_STRVERSCMP == 1
		ಸ್ಥಿತಿ FLAG_V:
			retval = strverscmp(x, y);
			ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
		/* Ascii sort */
		ಸ್ಥಿತಿ 0:
#ಆದರೆ ENABLE_LOCALE_SUPPORT
			retval = strcoll(x, y);
#ಇಲ್ಲದಿದ್ದರೆ
			retval = strcmp(x, y);
#ಆದರೆ_ಅಂತ್ಯ
			ನಿಲ್ಲಿಸು;
#ಆದರೆ ENABLE_FEATURE_ವಿಂಗಡಿಸು_BIG
		ಸ್ಥಿತಿ FLAG_g:
		ಸ್ಥಿತಿ FLAG_h: {
			ಅಕ್ಷರ *xx, *yy;
//TODO: needs setlocale(LC_NUMERIC, "C")?
			ದ್ವಿಗುಣ dx = strtod(x, &xx);
			ದ್ವಿಗುಣ dy = strtod(y, &yy);
			/* not numbers < NaN < -infinity < numbers < +infinity) */
			ಆದರೆ (x == xx)
				retval = (y == yy ? 0 : -1);
			ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (y == yy)
				retval = 1;
			/* Check for isnan */
			ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dx != dx)
				retval = (dy != dy) ? 0 : -1;
			ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dy != dy)
				retval = 1;
			ಇಲ್ಲದಿದ್ದರೆ {
				ಆದರೆ (flags & FLAG_h) {
					ಪೂರ್ಣಾಂಕ xs = scale_suffix(xx);
					ಪೂರ್ಣಾಂಕ ys = scale_suffix(yy);
					ಆದರೆ (xs != ys) {
						retval = xs - ys;
						ನಿಲ್ಲಿಸು;
					}
				}
				/* Check for infinity.  Could underflow, but it avoids libm. */
				ಆದರೆ (1.0 / dx == 0.0) {
					ಆದರೆ (dx < 0)
						retval = (1.0 / dy == 0.0 && dy < 0) ? 0 : -1;
					ಇಲ್ಲದಿದ್ದರೆ
						retval = (1.0 / dy == 0.0 && dy > 0) ? 0 : 1;
				} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (1.0 / dy == 0.0)
					retval = (dy < 0) ? 1 : -1;
				ಇಲ್ಲದಿದ್ದರೆ
					retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			}
			ನಿಲ್ಲಿಸು;
		}
		ಸ್ಥಿತಿ FLAG_M: {
			ರಚನೆ tm thyme;
			ಪೂರ್ಣಾಂಕ dx;
			ಅಕ್ಷರ *xx, *yy;

			xx = strptime(skip_whitespace(x), "%b", &thyme);
			dx = thyme.tm_mon;
			yy = strptime(skip_whitespace(y), "%b", &thyme);
			ಆದರೆ (!xx)
				retval = (!yy) ? 0 : -1;
			ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!yy)
				retval = 1;
			ಇಲ್ಲದಿದ್ದರೆ
				retval = dx - thyme.tm_mon;
			ನಿಲ್ಲಿಸು;
		}
		/* Full floating point version of -n */
		ಸ್ಥಿತಿ FLAG_n: {
			ದ್ವಿಗುಣ dx = atof(x);
			ದ್ವಿಗುಣ dy = atof(y);
			retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			ನಿಲ್ಲಿಸು;
		}
		} /* switch */
		/* Free key copies. */
		ಆದರೆ (x != *(ಅಕ್ಷರ **)xarg) ಬಿಡುಗಡೆ(x);
		ಆದರೆ (y != *(ಅಕ್ಷರ **)yarg) ಬಿಡುಗಡೆ(y);
		/* if (retval) break; - done by for () anyway */
#ಇಲ್ಲದಿದ್ದರೆ
		/* Integer version of -n for tiny systems */
		ಸ್ಥಿತಿ FLAG_n:
			retval = atoi(x) - atoi(y);
			ನಿಲ್ಲಿಸು;
		} /* switch */
#ಆದರೆ_ಅಂತ್ಯ
	} /* for */

	ಆದರೆ (retval == 0) {
		/* So far lines are "the same" */

		ಆದರೆ (option_mask32 & FLAG_s) {
			/* "Stable sort": later line is "greater than",
			 * IOW: do not allow qsort() to swap equal lines.
			 */
			uint32_t *p32;
			uint32_t x32, y32;
			ಅಕ್ಷರ *ಸಾಲು;
			ಚಿಹ್ನೆರಹಿತ len;

			ಸಾಲು = *(ಅಕ್ಷರ**)xarg;
			len = (ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಸಾಲು) + 4) & (~3u);
			p32 = (ಶೂನ್ಯ*)(ಸಾಲು + len);
			x32 = *p32;
			ಸಾಲು = *(ಅಕ್ಷರ**)yarg;
			len = (ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಸಾಲು) + 4) & (~3u);
			p32 = (ಶೂನ್ಯ*)(ಸಾಲು + len);
			y32 = *p32;

			/* If x > y, 1, else -1 */
			retval = (x32 > y32) * 2 - 1;
			/* Here, -r has no effect! */
			ಹಿಂತಿರುಗಿ retval;
		}
		ಆದರೆ (!(option_mask32 & FLAG_no_tie_break)) {
			/* fallback sort */
			flags = option_mask32;
			retval = strcmp(*(ಅಕ್ಷರ **)xarg, *(ಅಕ್ಷರ **)yarg);
		}
	}

	ಆದರೆ (flags & FLAG_r)
		ಹಿಂತಿರುಗಿ -retval;

	ಹಿಂತಿರುಗಿ retval;
}

#ಆದರೆ ENABLE_FEATURE_ವಿಂಗಡಿಸು_BIG
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ str2u(ಅಕ್ಷರ **str)
{
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ lu;
	ಆದರೆ (!ಅಂಕಿಯೇ((*str)[0]))
		bb_simple_error_ಸಂದೇಶ_and_die("bad field specification");
	lu = strtoul(*str, str, 10);
	ಆದರೆ ((ಗಾತ್ರ(ದೀರ್ಘ) > ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ) && lu > ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ) || !lu)
		bb_simple_error_ಸಂದೇಶ_and_die("bad field specification");
	ಹಿಂತಿರುಗಿ lu;
}
#ಆದರೆ_ಅಂತ್ಯ

ಪೂರ್ಣಾಂಕ ವಿಂಗಡಿಸು_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ ವಿಂಗಡಿಸು_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	ಅಕ್ಷರ **ಸಾಲುಗಳು;
	ಅಕ್ಷರ *str_ignored, *str_o, *str_t;
	llist_t *lst_k = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ i;
	ಪೂರ್ಣಾಂಕ linecount;
	ಚಿಹ್ನೆರಹಿತ opts;
#ಆದರೆ ENABLE_FEATURE_ವಿಂಗಡಿಸು_OPTIMIZE_ಸ್ಮರಣೆ
	bool can_drop_dups;
	ಗಾತ್ರ_t prev_len = 0;
	ಅಕ್ಷರ *prev_ಸಾಲು = (ಅಕ್ಷರ*) "";
	/* Postpone optimizing if the input is small, < 16k lines:
	 * even just free()ing duplicate lines takes time.
	 */
	ಗಾತ್ರ_t ಎಣಿಕೆ_ಗಮ್ಯ_optimize_dups = 0x3fff;
#ಆದರೆ_ಅಂತ್ಯ

	xfunc_error_retval = 2;

	/* Parse command line options */
	opts = getopt32(argv,
			ವಿಂಗಡಿಸು_opt_str,
			&str_ignored, &str_ignored, &str_o, &lst_k, &str_t
	);
#ಆದರೆ ENABLE_FEATURE_ವಿಂಗಡಿಸು_OPTIMIZE_ಸ್ಮರಣೆ
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
	ಆದರೆ (opts & FLAG_s)
		ಎಣಿಕೆ_ಗಮ್ಯ_optimize_dups = (ಗಾತ್ರ_t)-1L;
#ಆದರೆ_ಅಂತ್ಯ
	/* global b strips leading and trailing spaces */
	ಆದರೆ (opts & FLAG_b)
		option_mask32 |= FLAG_bb;
#ಆದರೆ ENABLE_FEATURE_ವಿಂಗಡಿಸು_BIG
	ಆದರೆ (opts & FLAG_t) {
		ಆದರೆ (!str_t[0] || str_t[1])
			bb_simple_error_ಸಂದೇಶ_and_die("bad -t parameter");
		ಕೀಲಿ_separator = str_t[0];
	}
	/* note: below this point we use option_mask32, not opts,
	 * since that reduces register pressure and makes code smaller */

	/* Parse sort key */
	ಆಗ (lst_k) {
		ಎಣಿಕೆ {
			FLAG_allowed_for_k =
				FLAG_n | /* Numeric sort */
				FLAG_g | /* Sort using strtod() */
				FLAG_h | /* Sort using strtod(), plus KMGT suffixes */
				FLAG_M | /* Sort date */
				FLAG_b | /* Ignore leading blanks */
				FLAG_r | /*ಹಿಮ್ಮುಖ*/
				FLAG_d | /* Ignore !(isalnum()|isspace()) */
				FLAG_f | /* Force uppercase */
				FLAG_i | /* Ignore !isprint() */
			0
		};
		ರಚನೆ ವಿಂಗಡಿಸು_ಕೀಲಿ *ಕೀಲಿ = ಕೂಡಿಸು_ಕೀಲಿ();
		ಅಕ್ಷರ *str_k = llist_ತೆಗೆ(&lst_k);

		i = 0; /* i==0 before comma, 1 after (-k3,6) */
		ಆಗ (*str_k) {
			/* Start of range */
			/* Cannot use bb_strtou - suffix can be a letter */
			ಕೀಲಿ->range[2*i] = str2u(&str_k);
			ಆದರೆ (*str_k == '.') {
				str_k++;
				ಕೀಲಿ->range[2*i+1] = str2u(&str_k);
			}
			ಆಗ (*str_k) {
				ಪೂರ್ಣಾಂಕ flag;
				ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *idx;

				ಆದರೆ (*str_k == ',' && !i++) {
					str_k++;
					ನಿಲ್ಲಿಸು;
				} /* no else needed: fall through to syntax error
					because comma isn't in OPT_STR */
				idx = strchr(OPT_STR, *str_k);
				ಆದರೆ (!idx)
					bb_simple_error_ಸಂದೇಶ_and_die("unknown key option");
				flag = 1 << (idx - OPT_STR);
				ಆದರೆ (flag & ~FLAG_allowed_for_k)
					bb_simple_error_ಸಂದೇಶ_and_die("unknown sort type");
				/* b after ',' means strip _trailing_ space */
				ಆದರೆ (i && flag == FLAG_b)
					flag = FLAG_bb;
				ಕೀಲಿ->flags |= flag;
				str_k++;
			}
		}
	}
#ಆದರೆ_ಅಂತ್ಯ

	/* Open input files and read data */
	argv += optind;
	ಆದರೆ (!*argv)
		*--argv = (ಅಕ್ಷರ*)"-";
	linecount = 0;
	ಸಾಲುಗಳು = ಖಾಲಿ;
	ಮಾಡು {
		/* coreutils 6.9 compat: abort on first open error,
		 * do not continue to next file: */
		FILE *fp = xfopen_stdin(*argv);
		ಗಾಗಿ (;;) {
			ಅಕ್ಷರ *ಸಾಲು = GET_ಸಾಲು(fp);
			ಆದರೆ (!ಸಾಲು)
				ನಿಲ್ಲಿಸು;

#ಆದರೆ ENABLE_FEATURE_ವಿಂಗಡಿಸು_OPTIMIZE_ಸ್ಮರಣೆ
			ಆದರೆ (ಎಣಿಕೆ_ಗಮ್ಯ_optimize_dups != 0)
				ಎಣಿಕೆ_ಗಮ್ಯ_optimize_dups--;
			ಆದರೆ (ಎಣಿಕೆ_ಗಮ್ಯ_optimize_dups == 0) {
				ಗಾತ್ರ_t len;
				ಅಕ್ಷರ *new_ಸಾಲು;

				/* On kernel/linux/arch/ *.[ch] files,
				 * this reduces memory usage by 6%.
				 *  yes | head -99999999 | sort
				 * goes down from 1900Mb to 380 Mb.
				 */
				len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಸಾಲು);
				ಆದರೆ (len <= prev_len) {
					new_ಸಾಲು = prev_ಸಾಲು + (prev_len - len);
					ಆದರೆ (strcmp(ಸಾಲು, new_ಸಾಲು) == 0) {
						/* it's a tail of the prev line */
						ಆದರೆ (can_drop_dups && prev_len == len) {
							/* it's identical to prev line */
							ಬಿಡುಗಡೆ(ಸಾಲು);
							ಮುಂದುವರಿ;
						}
						ಬಿಡುಗಡೆ(ಸಾಲು);
						ಸಾಲು = new_ಸಾಲು;
						/* continue using longer prev_line
						 * for future tail tests.
						 */
						ಹೋಗು skip;
					}
				}
				prev_len = len;
				prev_ಸಾಲು = ಸಾಲು;
 skip: ;
			}
#ಇಲ್ಲದಿದ್ದರೆ
//TODO: lighter version which only drops total dups if can_drop_dups == true
#ಆದರೆ_ಅಂತ್ಯ
			ಸಾಲುಗಳು = xrealloc_ಸದಿಶ(ಸಾಲುಗಳು, 6, linecount);
			ಸಾಲುಗಳು[linecount++] = ಸಾಲು;
		}
		fclose_if_not_stdin(fp);
	} ಆಗ (*++argv);

#ಆದರೆ ENABLE_FEATURE_ವಿಂಗಡಿಸು_BIG
	/* If no key, perform alphabetic sort */
	ಆದರೆ (!ಕೀಲಿ_list)
		ಕೂಡಿಸು_ಕೀಲಿ()->range[0] = 1;
	/* Handle -c */
	ಆದರೆ (option_mask32 & FLAG_c) {
		ಪೂರ್ಣಾಂಕ j = (option_mask32 & FLAG_u) ? -1 : 0;
		ಗಾಗಿ (i = 1; i < linecount; i++) {
			ಆದರೆ (compare_ಕೀಲಿಗಳು(&ಸಾಲುಗಳು[i-1], &ಸಾಲುಗಳು[i]) > j) {
				fprintf(stderr, "Check line %u\n", i);
				ಹಿಂತಿರುಗಿ EXIT_FAILURE;
			}
		}
		ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
	}
#ಆದರೆ_ಅಂತ್ಯ

	/* For stable sort, store original line position beyond terminating NUL */
	ಆದರೆ (option_mask32 & FLAG_s) {
		ಗಾಗಿ (i = 0; i < linecount; i++) {
			uint32_t *p32;
			ಅಕ್ಷರ *ಸಾಲು;
			ಚಿಹ್ನೆರಹಿತ len;

			ಸಾಲು = ಸಾಲುಗಳು[i];
			len = (ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಸಾಲು) + 4) & (~3u);
			ಸಾಲುಗಳು[i] = ಸಾಲು = xrealloc(ಸಾಲು, len + 4);
			p32 = (ಶೂನ್ಯ*)(ಸಾಲು + len);
			*p32 = i;
		}
		/*option_mask32 |= FLAG_no_tie_break;*/
		/* ^^^redundant: if FLAG_s, compare_keys() does no tie break */
	}

	/* Perform the actual sort */
	qsort(ಸಾಲುಗಳು, linecount, ಗಾತ್ರ(ಸಾಲುಗಳು[0]), compare_ಕೀಲಿಗಳು);

	/* Handle -u */
	ಆದರೆ (option_mask32 & FLAG_u) {
		ಪೂರ್ಣಾಂಕ j = 0;
		/* coreutils 6.3 drop lines for which only key is the same:
		 * - disabling last-resort compare, or else compare_keys()
		 * will be the same only for completely identical lines
		 * - disabling -s (same reasons)
		 */
		option_mask32 = (option_mask32 | FLAG_no_tie_break) & (~FLAG_s);
		ಗಾಗಿ (i = 1; i < linecount; i++) {
			ಆದರೆ (compare_ಕೀಲಿಗಳು(&ಸಾಲುಗಳು[j], &ಸಾಲುಗಳು[i]) == 0)
				ಬಿಡುಗಡೆ(ಸಾಲುಗಳು[i]);
			ಇಲ್ಲದಿದ್ದರೆ
				ಸಾಲುಗಳು[++j] = ಸಾಲುಗಳು[i];
		}
		ಆದರೆ (linecount)
			linecount = j+1;
	}

	/* Print it */
#ಆದರೆ ENABLE_FEATURE_ವಿಂಗಡಿಸು_BIG
	/* Open output file _after_ we read all input ones */
	ಆದರೆ (option_mask32 & FLAG_o)
		xmove_fd(xopen(str_o, O_WRONLY|O_CREAT|O_TRUNC), STDOUT_FILENO);
#ಆದರೆ_ಅಂತ್ಯ
	{
		ಪೂರ್ಣಾಂಕ ch = (option_mask32 & FLAG_z) ? '\0' : '\n';
		ಗಾಗಿ (i = 0; i < linecount; i++)
			ಬರೆ("%s%c", ಸಾಲುಗಳು[i], ch);
	}

	fflush_stdout_and_exit_SUCCESS();
}
