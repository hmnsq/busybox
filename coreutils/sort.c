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

#അന്തര്ഭാവ "libbb.h"

/* These are sort types */
എണ്ണൽ {
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

സ്ഥിതം സ്ഥിരം അക്ഷരം sort_opt_str[] ALIGN1 = "^"
			"nghMVucszbrdfimS:T:o:k:*t:"
			"\0" "o--o:t--t"/*-t, -o: at most one of each*/;
/*
 * OPT_STR must not be string literal, needs to have stable address:
 * code uses "strchr(OPT_STR,c) - OPT_STR" idiom.
 */
#define OPT_STR (sort_opt_str + 1)

#എങ്കിൽ ENABLE_FEATURE_SORT_BIG
സ്ഥിതം അക്ഷരം താക്കോൽ_separator;

സ്ഥിതം ഘടന sort_താക്കോൽ {
	ഘടന sort_താക്കോൽ *next_താക്കോൽ;  /* linked list */
	unsigned range[4];          /* start word, start char, end word, end char */
	unsigned flags;
} *താക്കോൽ_list;


/* This is a NOEXEC applet. Be very careful! */


സ്ഥിതം അക്ഷരം *get_താക്കോൽ(അക്ഷരം *str, ഘടന sort_താക്കോൽ *താക്കോൽ, പൂർണ്ണസംഖ്യ flags)
{
	പൂർണ്ണസംഖ്യ start = start; /* for compiler */
	പൂർണ്ണസംഖ്യ end;
	പൂർണ്ണസംഖ്യ len, j;
	unsigned i;

	/* Special case whole string, so we don't have to make a copy */
	എങ്കിൽ (താക്കോൽ->range[0] == 1 && !താക്കോൽ->range[1] && !താക്കോൽ->range[2] && !താക്കോൽ->range[3]
	 && !(flags & (FLAG_b | FLAG_d | FLAG_f | FLAG_i | FLAG_bb))
	) {
		തിരികെ str;
	}

	/* Find start of key on first pass, end on second pass */
	len = strlen(str);
	വേണ്ടി (j = 0; j < 2; j++) {
		എങ്കിൽ (!താക്കോൽ->range[2*j])
			end = len;
		/* Loop through fields */
		അല്ലെങ്കിൽ {
			unsigned അക്ഷരം ch = 0;

			end = 0;
			വേണ്ടി (i = 1; i < താക്കോൽ->range[2*j] + j; i++) {
				എങ്കിൽ (താക്കോൽ_separator) {
					/* Skip body of key and separator */
					ആകുമ്പോൾ ((ch = str[end]) != '\0') {
							end++;
						എങ്കിൽ (ch == താക്കോൽ_separator)
							നിർത്തുക;
					}
				} അല്ലെങ്കിൽ {
					/* Skip leading blanks */
					ആകുമ്പോൾ (isspace(str[end]))
						end++;
					/* Skip body of key */
					ആകുമ്പോൾ (str[end] != '\0') {
						എങ്കിൽ (isspace(str[end]))
							നിർത്തുക;
						end++;
					}
				}
			}
			/* Remove last delim: "abc:def:" => "abc:def" */
			എങ്കിൽ (j && ch) {
				//if (str[end-1] != key_separator)
				//  bb_error_msg(_and_die("BUG! "
				//  "str[start:%d,end:%d]:'%.*s'",
				//  start, end, (int)(end-start), &str[start]);
				end--;
			}
		}
		എങ്കിൽ (!j) start = end;
	}
	/* Strip leading whitespace if necessary */
	എങ്കിൽ (flags & FLAG_b)
		/* not using skip_whitespace() for speed */
		ആകുമ്പോൾ (isspace(str[start])) start++;
	/* Strip trailing whitespace if necessary */
	എങ്കിൽ (flags & FLAG_bb)
		ആകുമ്പോൾ (end > start && isspace(str[end-1])) end--;
	/* -kSTART,N.ENDCHAR: honor ENDCHAR (1-based) */
	എങ്കിൽ (താക്കോൽ->range[3]) {
		end = താക്കോൽ->range[3];
		എങ്കിൽ (end > len) end = len;
	}
	/* -kN.STARTCHAR[,...]: honor STARTCHAR (1-based) */
	എങ്കിൽ (താക്കോൽ->range[1]) {
		start += താക്കോൽ->range[1] - 1;
		എങ്കിൽ (start > len) start = len;
	}
	/* Make the copy */
	എങ്കിൽ (end < start)
		end = start;
	str = xstrndup(str+start, end-start);
	/* Handle -d */
	എങ്കിൽ (flags & FLAG_d) {
		വേണ്ടി (start = end = 0; str[end]; end++)
			എങ്കിൽ (isspace(str[end]) || isalnum(str[end]))
				str[start++] = str[end];
		str[start] = '\0';
	}
	/* Handle -i */
	എങ്കിൽ (flags & FLAG_i) {
		വേണ്ടി (start = end = 0; str[end]; end++)
			എങ്കിൽ (isprint_asciionly(str[end]))
				str[start++] = str[end];
		str[start] = '\0';
	}
	/* Handle -f */
	എങ്കിൽ (flags & FLAG_f)
		വേണ്ടി (i = 0; str[i]; i++)
			str[i] = toupper(str[i]);

	തിരികെ str;
}

സ്ഥിതം ഘടന sort_താക്കോൽ *add_താക്കോൽ(ശൂന്യം)
{
	ഘടന sort_താക്കോൽ **pkey = &താക്കോൽ_list;
	ആകുമ്പോൾ (*pkey)
		pkey = &((*pkey)->next_താക്കോൽ);
	തിരികെ *pkey = xzalloc(വലുപ്പം(ഘടന sort_താക്കോൽ));
}

#define GET_വരി(fp) \
	((option_mask32 & FLAG_z) \
	? bb_get_chunk_from_file(fp, ശൂന്യ) \
	: xmalloc_fgetline(fp))
#അല്ലെങ്കിൽ
#define GET_വരി(fp) xmalloc_fgetline(fp)
#endif

#എങ്കിൽ ENABLE_FEATURE_SORT_BIG
സ്ഥിതം പൂർണ്ണസംഖ്യ scale_suffix(സ്ഥിരം അക്ഷരം *tail)
{
	സ്ഥിതം സ്ഥിരം അക്ഷരം suffix[] ALIGN1 = "kmgtpezy";
	സ്ഥിരം അക്ഷരം *s;
	പൂർണ്ണസംഖ്യ n;

	എങ്കിൽ (!tail[0])
		തിരികെ -1;
	s = strchr(suffix, tail[0] | 0x20);
	എങ്കിൽ (!s)
		തിരികെ -1;
	n = s - suffix;
	എങ്കിൽ (n != 0 && tail[0] >= 'a')
		തിരികെ -1; /* mg... not accepted, only MG... */
	തിരികെ n;
}
#endif

/* Iterate through keys list and perform comparisons */
സ്ഥിതം പൂർണ്ണസംഖ്യ compare_താക്കോലുകൾ(സ്ഥിരം ശൂന്യം *xarg, സ്ഥിരം ശൂന്യം *yarg)
{
	പൂർണ്ണസംഖ്യ flags = option_mask32, retval = 0;
	അക്ഷരം *x, *y;

#എങ്കിൽ ENABLE_FEATURE_SORT_BIG
	ഘടന sort_താക്കോൽ *താക്കോൽ;

	വേണ്ടി (താക്കോൽ = താക്കോൽ_list; !retval && താക്കോൽ; താക്കോൽ = താക്കോൽ->next_താക്കോൽ) {
		flags = താക്കോൽ->flags ? താക്കോൽ->flags : option_mask32;
		/* Chop out and modify key chunks, handling -dfib */
		x = get_താക്കോൽ(*(അക്ഷരം **)xarg, താക്കോൽ, flags);
		y = get_താക്കോൽ(*(അക്ഷരം **)yarg, താക്കോൽ, flags);
#അല്ലെങ്കിൽ
	/* This curly bracket serves no purpose but to match the nesting
	 * level of the for () loop we're not using */
	{
		x = *(അക്ഷരം **)xarg;
		y = *(അക്ഷരം **)yarg;
#endif
		/* Perform actual comparison */
		തിരഞ്ഞെടുപ്പ് (flags & (FLAG_n | FLAG_g | FLAG_h | FLAG_M | FLAG_V)) {
		സ്ഥിരസ്ഥിതി:
			bb_simple_error_സന്ദേശം_and_die("unknown sort type");
			നിർത്തുക;
#എങ്കിൽ defined(HAVE_STRVERSCMP) && HAVE_STRVERSCMP == 1
		അവസ്ഥ FLAG_V:
			retval = strverscmp(x, y);
			നിർത്തുക;
#endif
		/* Ascii sort */
		അവസ്ഥ 0:
#എങ്കിൽ ENABLE_LOCALE_SUPPORT
			retval = strcoll(x, y);
#അല്ലെങ്കിൽ
			retval = strcmp(x, y);
#endif
			നിർത്തുക;
#എങ്കിൽ ENABLE_FEATURE_SORT_BIG
		അവസ്ഥ FLAG_g:
		അവസ്ഥ FLAG_h: {
			അക്ഷരം *xx, *yy;
//TODO: needs setlocale(LC_NUMERIC, "C")?
			ഇരട്ട dx = strtod(x, &xx);
			ഇരട്ട dy = strtod(y, &yy);
			/* not numbers < NaN < -infinity < numbers < +infinity) */
			എങ്കിൽ (x == xx)
				retval = (y == yy ? 0 : -1);
			അല്ലെങ്കിൽ എങ്കിൽ (y == yy)
				retval = 1;
			/* Check for isnan */
			അല്ലെങ്കിൽ എങ്കിൽ (dx != dx)
				retval = (dy != dy) ? 0 : -1;
			അല്ലെങ്കിൽ എങ്കിൽ (dy != dy)
				retval = 1;
			അല്ലെങ്കിൽ {
				എങ്കിൽ (flags & FLAG_h) {
					പൂർണ്ണസംഖ്യ xs = scale_suffix(xx);
					പൂർണ്ണസംഖ്യ ys = scale_suffix(yy);
					എങ്കിൽ (xs != ys) {
						retval = xs - ys;
						നിർത്തുക;
					}
				}
				/* Check for infinity.  Could underflow, but it avoids libm. */
				എങ്കിൽ (1.0 / dx == 0.0) {
					എങ്കിൽ (dx < 0)
						retval = (1.0 / dy == 0.0 && dy < 0) ? 0 : -1;
					അല്ലെങ്കിൽ
						retval = (1.0 / dy == 0.0 && dy > 0) ? 0 : 1;
				} അല്ലെങ്കിൽ എങ്കിൽ (1.0 / dy == 0.0)
					retval = (dy < 0) ? 1 : -1;
				അല്ലെങ്കിൽ
					retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			}
			നിർത്തുക;
		}
		അവസ്ഥ FLAG_M: {
			ഘടന tm thyme;
			പൂർണ്ണസംഖ്യ dx;
			അക്ഷരം *xx, *yy;

			xx = strptime(skip_whitespace(x), "%b", &thyme);
			dx = thyme.tm_mon;
			yy = strptime(skip_whitespace(y), "%b", &thyme);
			എങ്കിൽ (!xx)
				retval = (!yy) ? 0 : -1;
			അല്ലെങ്കിൽ എങ്കിൽ (!yy)
				retval = 1;
			അല്ലെങ്കിൽ
				retval = dx - thyme.tm_mon;
			നിർത്തുക;
		}
		/* Full floating point version of -n */
		അവസ്ഥ FLAG_n: {
			ഇരട്ട dx = atof(x);
			ഇരട്ട dy = atof(y);
			retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			നിർത്തുക;
		}
		} /* switch */
		/* Free key copies. */
		എങ്കിൽ (x != *(അക്ഷരം **)xarg) സ്വതന്ത്രം(x);
		എങ്കിൽ (y != *(അക്ഷരം **)yarg) സ്വതന്ത്രം(y);
		/* if (retval) break; - done by for () anyway */
#അല്ലെങ്കിൽ
		/* Integer version of -n for tiny systems */
		അവസ്ഥ FLAG_n:
			retval = atoi(x) - atoi(y);
			നിർത്തുക;
		} /* switch */
#endif
	} /* for */

	എങ്കിൽ (retval == 0) {
		/* So far lines are "the same" */

		എങ്കിൽ (option_mask32 & FLAG_s) {
			/* "Stable sort": later line is "greater than",
			 * IOW: do not allow qsort() to swap equal lines.
			 */
			uint32_t *p32;
			uint32_t x32, y32;
			അക്ഷരം *വരി;
			unsigned len;

			വരി = *(അക്ഷരം**)xarg;
			len = (strlen(വരി) + 4) & (~3u);
			p32 = (ശൂന്യം*)(വരി + len);
			x32 = *p32;
			വരി = *(അക്ഷരം**)yarg;
			len = (strlen(വരി) + 4) & (~3u);
			p32 = (ശൂന്യം*)(വരി + len);
			y32 = *p32;

			/* If x > y, 1, else -1 */
			retval = (x32 > y32) * 2 - 1;
			/* Here, -r has no effect! */
			തിരികെ retval;
		}
		എങ്കിൽ (!(option_mask32 & FLAG_no_tie_break)) {
			/* fallback sort */
			flags = option_mask32;
			retval = strcmp(*(അക്ഷരം **)xarg, *(അക്ഷരം **)yarg);
		}
	}

	എങ്കിൽ (flags & FLAG_r)
		തിരികെ -retval;

	തിരികെ retval;
}

#എങ്കിൽ ENABLE_FEATURE_SORT_BIG
സ്ഥിതം unsigned str2u(അക്ഷരം **str)
{
	unsigned long lu;
	എങ്കിൽ (!isdigit((*str)[0]))
		bb_simple_error_സന്ദേശം_and_die("bad field specification");
	lu = strtoul(*str, str, 10);
	എങ്കിൽ ((വലുപ്പം(long) > വലുപ്പം(പൂർണ്ണസംഖ്യ) && lu > INT_MAX) || !lu)
		bb_simple_error_സന്ദേശം_and_die("bad field specification");
	തിരികെ lu;
}
#endif

പൂർണ്ണസംഖ്യ sort_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) MAIN_EXTERNALLY_VISIBLE;
പൂർണ്ണസംഖ്യ sort_main(പൂർണ്ണസംഖ്യ argc UNUSED_PARAM, അക്ഷരം **argv)
{
	അക്ഷരം **വരികൾ;
	അക്ഷരം *str_ignored, *str_o, *str_t;
	llist_t *lst_k = ശൂന്യ;
	പൂർണ്ണസംഖ്യ i;
	പൂർണ്ണസംഖ്യ linecount;
	unsigned opts;
#എങ്കിൽ ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
	bool can_drop_dups;
	size_t prev_len = 0;
	അക്ഷരം *prev_വരി = (അക്ഷരം*) "";
	/* Postpone optimizing if the input is small, < 16k lines:
	 * even just free()ing duplicate lines takes time.
	 */
	size_t എണ്ണം_to_optimize_dups = 0x3fff;
#endif

	xfunc_error_retval = 2;

	/* Parse command line options */
	opts = getopt32(argv,
			sort_opt_str,
			&str_ignored, &str_ignored, &str_o, &lst_k, &str_t
	);
#എങ്കിൽ ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
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
	എങ്കിൽ (opts & FLAG_s)
		എണ്ണം_to_optimize_dups = (size_t)-1L;
#endif
	/* global b strips leading and trailing spaces */
	എങ്കിൽ (opts & FLAG_b)
		option_mask32 |= FLAG_bb;
#എങ്കിൽ ENABLE_FEATURE_SORT_BIG
	എങ്കിൽ (opts & FLAG_t) {
		എങ്കിൽ (!str_t[0] || str_t[1])
			bb_simple_error_സന്ദേശം_and_die("bad -t parameter");
		താക്കോൽ_separator = str_t[0];
	}
	/* note: below this point we use option_mask32, not opts,
	 * since that reduces register pressure and makes code smaller */

	/* Parse sort key */
	ആകുമ്പോൾ (lst_k) {
		എണ്ണൽ {
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
		ഘടന sort_താക്കോൽ *താക്കോൽ = add_താക്കോൽ();
		അക്ഷരം *str_k = llist_എടുക്കുക(&lst_k);

		i = 0; /* i==0 before comma, 1 after (-k3,6) */
		ആകുമ്പോൾ (*str_k) {
			/* Start of range */
			/* Cannot use bb_strtou - suffix can be a letter */
			താക്കോൽ->range[2*i] = str2u(&str_k);
			എങ്കിൽ (*str_k == '.') {
				str_k++;
				താക്കോൽ->range[2*i+1] = str2u(&str_k);
			}
			ആകുമ്പോൾ (*str_k) {
				പൂർണ്ണസംഖ്യ flag;
				സ്ഥിരം അക്ഷരം *idx;

				എങ്കിൽ (*str_k == ',' && !i++) {
					str_k++;
					നിർത്തുക;
				} /* no else needed: fall through to syntax error
					because comma isn't in OPT_STR */
				idx = strchr(OPT_STR, *str_k);
				എങ്കിൽ (!idx)
					bb_simple_error_സന്ദേശം_and_die("unknown key option");
				flag = 1 << (idx - OPT_STR);
				എങ്കിൽ (flag & ~FLAG_allowed_for_k)
					bb_simple_error_സന്ദേശം_and_die("unknown sort type");
				/* b after ',' means strip _trailing_ space */
				എങ്കിൽ (i && flag == FLAG_b)
					flag = FLAG_bb;
				താക്കോൽ->flags |= flag;
				str_k++;
			}
		}
	}
#endif

	/* Open input files and read data */
	argv += optind;
	എങ്കിൽ (!*argv)
		*--argv = (അക്ഷരം*)"-";
	linecount = 0;
	വരികൾ = ശൂന്യ;
	ചെയ്യുക {
		/* coreutils 6.9 compat: abort on first open error,
		 * do not continue to next file: */
		FILE *fp = xfopen_stdin(*argv);
		വേണ്ടി (;;) {
			അക്ഷരം *വരി = GET_വരി(fp);
			എങ്കിൽ (!വരി)
				നിർത്തുക;

#എങ്കിൽ ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
			എങ്കിൽ (എണ്ണം_to_optimize_dups != 0)
				എണ്ണം_to_optimize_dups--;
			എങ്കിൽ (എണ്ണം_to_optimize_dups == 0) {
				size_t len;
				അക്ഷരം *new_വരി;

				/* On kernel/linux/arch/ *.[ch] files,
				 * this reduces memory usage by 6%.
				 *  yes | head -99999999 | sort
				 * goes down from 1900Mb to 380 Mb.
				 */
				len = strlen(വരി);
				എങ്കിൽ (len <= prev_len) {
					new_വരി = prev_വരി + (prev_len - len);
					എങ്കിൽ (strcmp(വരി, new_വരി) == 0) {
						/* it's a tail of the prev line */
						എങ്കിൽ (can_drop_dups && prev_len == len) {
							/* it's identical to prev line */
							സ്വതന്ത്രം(വരി);
							തുടരുക;
						}
						സ്വതന്ത്രം(വരി);
						വരി = new_വരി;
						/* continue using longer prev_line
						 * for future tail tests.
						 */
						പോകുക skip;
					}
				}
				prev_len = len;
				prev_വരി = വരി;
 skip: ;
			}
#അല്ലെങ്കിൽ
//TODO: lighter version which only drops total dups if can_drop_dups == true
#endif
			വരികൾ = xrealloc_vector(വരികൾ, 6, linecount);
			വരികൾ[linecount++] = വരി;
		}
		fclose_if_not_stdin(fp);
	} ആകുമ്പോൾ (*++argv);

#എങ്കിൽ ENABLE_FEATURE_SORT_BIG
	/* If no key, perform alphabetic sort */
	എങ്കിൽ (!താക്കോൽ_list)
		add_താക്കോൽ()->range[0] = 1;
	/* Handle -c */
	എങ്കിൽ (option_mask32 & FLAG_c) {
		പൂർണ്ണസംഖ്യ j = (option_mask32 & FLAG_u) ? -1 : 0;
		വേണ്ടി (i = 1; i < linecount; i++) {
			എങ്കിൽ (compare_താക്കോലുകൾ(&വരികൾ[i-1], &വരികൾ[i]) > j) {
				fprintf(stderr, "Check line %u\n", i);
				തിരികെ EXIT_FAILURE;
			}
		}
		തിരികെ EXIT_SUCCESS;
	}
#endif

	/* For stable sort, store original line position beyond terminating NUL */
	എങ്കിൽ (option_mask32 & FLAG_s) {
		വേണ്ടി (i = 0; i < linecount; i++) {
			uint32_t *p32;
			അക്ഷരം *വരി;
			unsigned len;

			വരി = വരികൾ[i];
			len = (strlen(വരി) + 4) & (~3u);
			വരികൾ[i] = വരി = xrealloc(വരി, len + 4);
			p32 = (ശൂന്യം*)(വരി + len);
			*p32 = i;
		}
		/*option_mask32 |= FLAG_no_tie_break;*/
		/* ^^^redundant: if FLAG_s, compare_keys() does no tie break */
	}

	/* Perform the actual sort */
	qsort(വരികൾ, linecount, വലുപ്പം(വരികൾ[0]), compare_താക്കോലുകൾ);

	/* Handle -u */
	എങ്കിൽ (option_mask32 & FLAG_u) {
		പൂർണ്ണസംഖ്യ j = 0;
		/* coreutils 6.3 drop lines for which only key is the same:
		 * - disabling last-resort compare, or else compare_keys()
		 * will be the same only for completely identical lines
		 * - disabling -s (same reasons)
		 */
		option_mask32 = (option_mask32 | FLAG_no_tie_break) & (~FLAG_s);
		വേണ്ടി (i = 1; i < linecount; i++) {
			എങ്കിൽ (compare_താക്കോലുകൾ(&വരികൾ[j], &വരികൾ[i]) == 0)
				സ്വതന്ത്രം(വരികൾ[i]);
			അല്ലെങ്കിൽ
				വരികൾ[++j] = വരികൾ[i];
		}
		എങ്കിൽ (linecount)
			linecount = j+1;
	}

	/* Print it */
#എങ്കിൽ ENABLE_FEATURE_SORT_BIG
	/* Open output file _after_ we read all input ones */
	എങ്കിൽ (option_mask32 & FLAG_o)
		xmove_fd(xopen(str_o, O_WRONLY|O_CREAT|O_TRUNC), STDOUT_FILENO);
#endif
	{
		പൂർണ്ണസംഖ്യ ch = (option_mask32 & FLAG_z) ? '\0' : '\n';
		വേണ്ടി (i = 0; i < linecount; i++)
			printf("%s%c", വരികൾ[i], ch);
	}

	fflush_stdout_and_exit_SUCCESS();
}
