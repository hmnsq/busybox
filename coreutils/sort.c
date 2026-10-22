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

#অন্তর্ভাৱ "libbb.h"

/* These are sort types */
গণনা {
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

স্থির ধ্রুৱ ৱর্ণ sort_opt_str[] ALIGN1 = "^"
			"nghMVucszbrdfimS:T:o:k:*t:"
			"\0" "o--o:t--t"/*-t, -o: at most one of each*/;
/*
 * OPT_STR must not be string literal, needs to have stable address:
 * code uses "strchr(OPT_STR,c) - OPT_STR" idiom.
 */
#define OPT_STR (sort_opt_str + 1)

#যদি ENABLE_FEATURE_SORT_BIG
স্থির ৱর্ণ চাবি_separator;

স্থির সংরচনা sort_চাবি {
	সংরচনা sort_চাবি *next_চাবি;  /* linked list */
	unsigned range[4];          /* start word, start char, end word, end char */
	unsigned flags;
} *চাবি_list;


/* This is a NOEXEC applet. Be very careful! */


স্থির ৱর্ণ *get_চাবি(ৱর্ণ *str, সংরচনা sort_চাবি *চাবি, পূর্ণাঙ্ক flags)
{
	পূর্ণাঙ্ক start = start; /* for compiler */
	পূর্ণাঙ্ক end;
	পূর্ণাঙ্ক len, j;
	unsigned i;

	/* Special case whole string, so we don't have to make a copy */
	যদি (চাবি->range[0] == 1 && !চাবি->range[1] && !চাবি->range[2] && !চাবি->range[3]
	 && !(flags & (FLAG_b | FLAG_d | FLAG_f | FLAG_i | FLAG_bb))
	) {
		ঘূৰাই str;
	}

	/* Find start of key on first pass, end on second pass */
	len = strlen(str);
	বাবে (j = 0; j < 2; j++) {
		যদি (!চাবি->range[2*j])
			end = len;
		/* Loop through fields */
		অন্যথা {
			unsigned ৱর্ণ ch = 0;

			end = 0;
			বাবে (i = 1; i < চাবি->range[2*j] + j; i++) {
				যদি (চাবি_separator) {
					/* Skip body of key and separator */
					যেতিয়ালৈ ((ch = str[end]) != '\0') {
							end++;
						যদি (ch == চাবি_separator)
							ভঙ্গ;
					}
				} অন্যথা {
					/* Skip leading blanks */
					যেতিয়ালৈ (isspace(str[end]))
						end++;
					/* Skip body of key */
					যেতিয়ালৈ (str[end] != '\0') {
						যদি (isspace(str[end]))
							ভঙ্গ;
						end++;
					}
				}
			}
			/* Remove last delim: "abc:def:" => "abc:def" */
			যদি (j && ch) {
				//if (str[end-1] != key_separator)
				//  bb_error_msg(_and_die("BUG! "
				//  "str[start:%d,end:%d]:'%.*s'",
				//  start, end, (int)(end-start), &str[start]);
				end--;
			}
		}
		যদি (!j) start = end;
	}
	/* Strip leading whitespace if necessary */
	যদি (flags & FLAG_b)
		/* not using skip_whitespace() for speed */
		যেতিয়ালৈ (isspace(str[start])) start++;
	/* Strip trailing whitespace if necessary */
	যদি (flags & FLAG_bb)
		যেতিয়ালৈ (end > start && isspace(str[end-1])) end--;
	/* -kSTART,N.ENDCHAR: honor ENDCHAR (1-based) */
	যদি (চাবি->range[3]) {
		end = চাবি->range[3];
		যদি (end > len) end = len;
	}
	/* -kN.STARTCHAR[,...]: honor STARTCHAR (1-based) */
	যদি (চাবি->range[1]) {
		start += চাবি->range[1] - 1;
		যদি (start > len) start = len;
	}
	/* Make the copy */
	যদি (end < start)
		end = start;
	str = xstrndup(str+start, end-start);
	/* Handle -d */
	যদি (flags & FLAG_d) {
		বাবে (start = end = 0; str[end]; end++)
			যদি (isspace(str[end]) || isalnum(str[end]))
				str[start++] = str[end];
		str[start] = '\0';
	}
	/* Handle -i */
	যদি (flags & FLAG_i) {
		বাবে (start = end = 0; str[end]; end++)
			যদি (isprint_asciionly(str[end]))
				str[start++] = str[end];
		str[start] = '\0';
	}
	/* Handle -f */
	যদি (flags & FLAG_f)
		বাবে (i = 0; str[i]; i++)
			str[i] = toupper(str[i]);

	ঘূৰাই str;
}

স্থির সংরচনা sort_চাবি *add_চাবি(শূন্য)
{
	সংরচনা sort_চাবি **pkey = &চাবি_list;
	যেতিয়ালৈ (*pkey)
		pkey = &((*pkey)->next_চাবি);
	ঘূৰাই *pkey = xzalloc(পরিমাণ(সংরচনা sort_চাবি));
}

#define GET_সাৰি(fp) \
	((option_mask32 & FLAG_z) \
	? bb_get_chunk_from_file(fp, রিক্ত) \
	: xmalloc_fgetline(fp))
#অন্যথা
#define GET_সাৰি(fp) xmalloc_fgetline(fp)
#endif

#যদি ENABLE_FEATURE_SORT_BIG
স্থির পূর্ণাঙ্ক scale_suffix(ধ্রুৱ ৱর্ণ *tail)
{
	স্থির ধ্রুৱ ৱর্ণ suffix[] ALIGN1 = "kmgtpezy";
	ধ্রুৱ ৱর্ণ *s;
	পূর্ণাঙ্ক n;

	যদি (!tail[0])
		ঘূৰাই -1;
	s = strchr(suffix, tail[0] | 0x20);
	যদি (!s)
		ঘূৰাই -1;
	n = s - suffix;
	যদি (n != 0 && tail[0] >= 'a')
		ঘূৰাই -1; /* mg... not accepted, only MG... */
	ঘূৰাই n;
}
#endif

/* Iterate through keys list and perform comparisons */
স্থির পূর্ণাঙ্ক compare_চাবিগুলি(ধ্রুৱ শূন্য *xarg, ধ্রুৱ শূন্য *yarg)
{
	পূর্ণাঙ্ক flags = option_mask32, retval = 0;
	ৱর্ণ *x, *y;

#যদি ENABLE_FEATURE_SORT_BIG
	সংরচনা sort_চাবি *চাবি;

	বাবে (চাবি = চাবি_list; !retval && চাবি; চাবি = চাবি->next_চাবি) {
		flags = চাবি->flags ? চাবি->flags : option_mask32;
		/* Chop out and modify key chunks, handling -dfib */
		x = get_চাবি(*(ৱর্ণ **)xarg, চাবি, flags);
		y = get_চাবি(*(ৱর্ণ **)yarg, চাবি, flags);
#অন্যথা
	/* This curly bracket serves no purpose but to match the nesting
	 * level of the for () loop we're not using */
	{
		x = *(ৱর্ণ **)xarg;
		y = *(ৱর্ণ **)yarg;
#endif
		/* Perform actual comparison */
		ৱিকল্প (flags & (FLAG_n | FLAG_g | FLAG_h | FLAG_M | FLAG_V)) {
		স্ৱভাৱ:
			bb_simple_error_বাৰ্তা_and_die("unknown sort type");
			ভঙ্গ;
#যদি defined(HAVE_STRVERSCMP) && HAVE_STRVERSCMP == 1
		পক্ষ FLAG_V:
			retval = strverscmp(x, y);
			ভঙ্গ;
#endif
		/* Ascii sort */
		পক্ষ 0:
#যদি ENABLE_LOCALE_SUPPORT
			retval = strcoll(x, y);
#অন্যথা
			retval = strcmp(x, y);
#endif
			ভঙ্গ;
#যদি ENABLE_FEATURE_SORT_BIG
		পক্ষ FLAG_g:
		পক্ষ FLAG_h: {
			ৱর্ণ *xx, *yy;
//TODO: needs setlocale(LC_NUMERIC, "C")?
			দ্ৱিগুণ dx = strtod(x, &xx);
			দ্ৱিগুণ dy = strtod(y, &yy);
			/* not numbers < NaN < -infinity < numbers < +infinity) */
			যদি (x == xx)
				retval = (y == yy ? 0 : -1);
			অন্যথা যদি (y == yy)
				retval = 1;
			/* Check for isnan */
			অন্যথা যদি (dx != dx)
				retval = (dy != dy) ? 0 : -1;
			অন্যথা যদি (dy != dy)
				retval = 1;
			অন্যথা {
				যদি (flags & FLAG_h) {
					পূর্ণাঙ্ক xs = scale_suffix(xx);
					পূর্ণাঙ্ক ys = scale_suffix(yy);
					যদি (xs != ys) {
						retval = xs - ys;
						ভঙ্গ;
					}
				}
				/* Check for infinity.  Could underflow, but it avoids libm. */
				যদি (1.0 / dx == 0.0) {
					যদি (dx < 0)
						retval = (1.0 / dy == 0.0 && dy < 0) ? 0 : -1;
					অন্যথা
						retval = (1.0 / dy == 0.0 && dy > 0) ? 0 : 1;
				} অন্যথা যদি (1.0 / dy == 0.0)
					retval = (dy < 0) ? 1 : -1;
				অন্যথা
					retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			}
			ভঙ্গ;
		}
		পক্ষ FLAG_M: {
			সংরচনা tm thyme;
			পূর্ণাঙ্ক dx;
			ৱর্ণ *xx, *yy;

			xx = strptime(skip_whitespace(x), "%b", &thyme);
			dx = thyme.tm_mon;
			yy = strptime(skip_whitespace(y), "%b", &thyme);
			যদি (!xx)
				retval = (!yy) ? 0 : -1;
			অন্যথা যদি (!yy)
				retval = 1;
			অন্যথা
				retval = dx - thyme.tm_mon;
			ভঙ্গ;
		}
		/* Full floating point version of -n */
		পক্ষ FLAG_n: {
			দ্ৱিগুণ dx = atof(x);
			দ্ৱিগুণ dy = atof(y);
			retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			ভঙ্গ;
		}
		} /* switch */
		/* Free key copies. */
		যদি (x != *(ৱর্ণ **)xarg) মুক্ত(x);
		যদি (y != *(ৱর্ণ **)yarg) মুক্ত(y);
		/* if (retval) break; - done by for () anyway */
#অন্যথা
		/* Integer version of -n for tiny systems */
		পক্ষ FLAG_n:
			retval = atoi(x) - atoi(y);
			ভঙ্গ;
		} /* switch */
#endif
	} /* for */

	যদি (retval == 0) {
		/* So far lines are "the same" */

		যদি (option_mask32 & FLAG_s) {
			/* "Stable sort": later line is "greater than",
			 * IOW: do not allow qsort() to swap equal lines.
			 */
			uint32_t *p32;
			uint32_t x32, y32;
			ৱর্ণ *সাৰি;
			unsigned len;

			সাৰি = *(ৱর্ণ**)xarg;
			len = (strlen(সাৰি) + 4) & (~3u);
			p32 = (শূন্য*)(সাৰি + len);
			x32 = *p32;
			সাৰি = *(ৱর্ণ**)yarg;
			len = (strlen(সাৰি) + 4) & (~3u);
			p32 = (শূন্য*)(সাৰি + len);
			y32 = *p32;

			/* If x > y, 1, else -1 */
			retval = (x32 > y32) * 2 - 1;
			/* Here, -r has no effect! */
			ঘূৰাই retval;
		}
		যদি (!(option_mask32 & FLAG_no_tie_break)) {
			/* fallback sort */
			flags = option_mask32;
			retval = strcmp(*(ৱর্ণ **)xarg, *(ৱর্ণ **)yarg);
		}
	}

	যদি (flags & FLAG_r)
		ঘূৰাই -retval;

	ঘূৰাই retval;
}

#যদি ENABLE_FEATURE_SORT_BIG
স্থির unsigned str2u(ৱর্ণ **str)
{
	unsigned long lu;
	যদি (!isdigit((*str)[0]))
		bb_simple_error_বাৰ্তা_and_die("bad field specification");
	lu = strtoul(*str, str, 10);
	যদি ((পরিমাণ(long) > পরিমাণ(পূর্ণাঙ্ক) && lu > INT_MAX) || !lu)
		bb_simple_error_বাৰ্তা_and_die("bad field specification");
	ঘূৰাই lu;
}
#endif

পূর্ণাঙ্ক sort_main(পূর্ণাঙ্ক argc, ৱর্ণ **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণাঙ্ক sort_main(পূর্ণাঙ্ক argc UNUSED_PARAM, ৱর্ণ **argv)
{
	ৱর্ণ **সাৰিগুলি;
	ৱর্ণ *str_ignored, *str_o, *str_t;
	llist_t *lst_k = রিক্ত;
	পূর্ণাঙ্ক i;
	পূর্ণাঙ্ক linecount;
	unsigned opts;
#যদি ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
	bool can_drop_dups;
	size_t prev_len = 0;
	ৱর্ণ *prev_সাৰি = (ৱর্ণ*) "";
	/* Postpone optimizing if the input is small, < 16k lines:
	 * even just free()ing duplicate lines takes time.
	 */
	size_t গুনতি_to_optimize_dups = 0x3fff;
#endif

	xfunc_error_retval = 2;

	/* Parse command line options */
	opts = getopt32(argv,
			sort_opt_str,
			&str_ignored, &str_ignored, &str_o, &lst_k, &str_t
	);
#যদি ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
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
	যদি (opts & FLAG_s)
		গুনতি_to_optimize_dups = (size_t)-1L;
#endif
	/* global b strips leading and trailing spaces */
	যদি (opts & FLAG_b)
		option_mask32 |= FLAG_bb;
#যদি ENABLE_FEATURE_SORT_BIG
	যদি (opts & FLAG_t) {
		যদি (!str_t[0] || str_t[1])
			bb_simple_error_বাৰ্তা_and_die("bad -t parameter");
		চাবি_separator = str_t[0];
	}
	/* note: below this point we use option_mask32, not opts,
	 * since that reduces register pressure and makes code smaller */

	/* Parse sort key */
	যেতিয়ালৈ (lst_k) {
		গণনা {
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
		সংরচনা sort_চাবি *চাবি = add_চাবি();
		ৱর্ণ *str_k = llist_বেৰ_কৰো(&lst_k);

		i = 0; /* i==0 before comma, 1 after (-k3,6) */
		যেতিয়ালৈ (*str_k) {
			/* Start of range */
			/* Cannot use bb_strtou - suffix can be a letter */
			চাবি->range[2*i] = str2u(&str_k);
			যদি (*str_k == '.') {
				str_k++;
				চাবি->range[2*i+1] = str2u(&str_k);
			}
			যেতিয়ালৈ (*str_k) {
				পূর্ণাঙ্ক flag;
				ধ্রুৱ ৱর্ণ *idx;

				যদি (*str_k == ',' && !i++) {
					str_k++;
					ভঙ্গ;
				} /* no else needed: fall through to syntax error
					because comma isn't in OPT_STR */
				idx = strchr(OPT_STR, *str_k);
				যদি (!idx)
					bb_simple_error_বাৰ্তা_and_die("unknown key option");
				flag = 1 << (idx - OPT_STR);
				যদি (flag & ~FLAG_allowed_for_k)
					bb_simple_error_বাৰ্তা_and_die("unknown sort type");
				/* b after ',' means strip _trailing_ space */
				যদি (i && flag == FLAG_b)
					flag = FLAG_bb;
				চাবি->flags |= flag;
				str_k++;
			}
		}
	}
#endif

	/* Open input files and read data */
	argv += optind;
	যদি (!*argv)
		*--argv = (ৱর্ণ*)"-";
	linecount = 0;
	সাৰিগুলি = রিক্ত;
	কৰা {
		/* coreutils 6.9 compat: abort on first open error,
		 * do not continue to next file: */
		FILE *fp = xfopen_stdin(*argv);
		বাবে (;;) {
			ৱর্ণ *সাৰি = GET_সাৰি(fp);
			যদি (!সাৰি)
				ভঙ্গ;

#যদি ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
			যদি (গুনতি_to_optimize_dups != 0)
				গুনতি_to_optimize_dups--;
			যদি (গুনতি_to_optimize_dups == 0) {
				size_t len;
				ৱর্ণ *new_সাৰি;

				/* On kernel/linux/arch/ *.[ch] files,
				 * this reduces memory usage by 6%.
				 *  yes | head -99999999 | sort
				 * goes down from 1900Mb to 380 Mb.
				 */
				len = strlen(সাৰি);
				যদি (len <= prev_len) {
					new_সাৰি = prev_সাৰি + (prev_len - len);
					যদি (strcmp(সাৰি, new_সাৰি) == 0) {
						/* it's a tail of the prev line */
						যদি (can_drop_dups && prev_len == len) {
							/* it's identical to prev line */
							মুক্ত(সাৰি);
							অনুৱর্ত;
						}
						মুক্ত(সাৰি);
						সাৰি = new_সাৰি;
						/* continue using longer prev_line
						 * for future tail tests.
						 */
						গচ্ছ skip;
					}
				}
				prev_len = len;
				prev_সাৰি = সাৰি;
 skip: ;
			}
#অন্যথা
//TODO: lighter version which only drops total dups if can_drop_dups == true
#endif
			সাৰিগুলি = xrealloc_vector(সাৰিগুলি, 6, linecount);
			সাৰিগুলি[linecount++] = সাৰি;
		}
		fclose_if_not_stdin(fp);
	} যেতিয়ালৈ (*++argv);

#যদি ENABLE_FEATURE_SORT_BIG
	/* If no key, perform alphabetic sort */
	যদি (!চাবি_list)
		add_চাবি()->range[0] = 1;
	/* Handle -c */
	যদি (option_mask32 & FLAG_c) {
		পূর্ণাঙ্ক j = (option_mask32 & FLAG_u) ? -1 : 0;
		বাবে (i = 1; i < linecount; i++) {
			যদি (compare_চাবিগুলি(&সাৰিগুলি[i-1], &সাৰিগুলি[i]) > j) {
				fprintf(stderr, "Check line %u\n", i);
				ঘূৰাই EXIT_FAILURE;
			}
		}
		ঘূৰাই EXIT_SUCCESS;
	}
#endif

	/* For stable sort, store original line position beyond terminating NUL */
	যদি (option_mask32 & FLAG_s) {
		বাবে (i = 0; i < linecount; i++) {
			uint32_t *p32;
			ৱর্ণ *সাৰি;
			unsigned len;

			সাৰি = সাৰিগুলি[i];
			len = (strlen(সাৰি) + 4) & (~3u);
			সাৰিগুলি[i] = সাৰি = xrealloc(সাৰি, len + 4);
			p32 = (শূন্য*)(সাৰি + len);
			*p32 = i;
		}
		/*option_mask32 |= FLAG_no_tie_break;*/
		/* ^^^redundant: if FLAG_s, compare_keys() does no tie break */
	}

	/* Perform the actual sort */
	qsort(সাৰিগুলি, linecount, পরিমাণ(সাৰিগুলি[0]), compare_চাবিগুলি);

	/* Handle -u */
	যদি (option_mask32 & FLAG_u) {
		পূর্ণাঙ্ক j = 0;
		/* coreutils 6.3 drop lines for which only key is the same:
		 * - disabling last-resort compare, or else compare_keys()
		 * will be the same only for completely identical lines
		 * - disabling -s (same reasons)
		 */
		option_mask32 = (option_mask32 | FLAG_no_tie_break) & (~FLAG_s);
		বাবে (i = 1; i < linecount; i++) {
			যদি (compare_চাবিগুলি(&সাৰিগুলি[j], &সাৰিগুলি[i]) == 0)
				মুক্ত(সাৰিগুলি[i]);
			অন্যথা
				সাৰিগুলি[++j] = সাৰিগুলি[i];
		}
		যদি (linecount)
			linecount = j+1;
	}

	/* Print it */
#যদি ENABLE_FEATURE_SORT_BIG
	/* Open output file _after_ we read all input ones */
	যদি (option_mask32 & FLAG_o)
		xmove_fd(xopen(str_o, O_WRONLY|O_CREAT|O_TRUNC), STDOUT_FILENO);
#endif
	{
		পূর্ণাঙ্ক ch = (option_mask32 & FLAG_z) ? '\0' : '\n';
		বাবে (i = 0; i < linecount; i++)
			printf("%s%c", সাৰিগুলি[i], ch);
	}

	fflush_stdout_and_exit_SUCCESS();
}
