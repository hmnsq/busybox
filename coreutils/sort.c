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

#அந்தர்பாவ "libbb.h"

/* These are sort types */
எண்ணிக்கை {
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

நிலையான மாறிலி எழுத்து sort_opt_str[] ALIGN1 = "^"
			"nghMVucszbrdfimS:T:o:k:*t:"
			"\0" "o--o:t--t"/*-t, -o: at most one of each*/;
/*
 * OPT_STR must not be string literal, needs to have stable address:
 * code uses "strchr(OPT_STR,c) - OPT_STR" idiom.
 */
#define OPT_STR (sort_opt_str + 1)

#எனில் ENABLE_FEATURE_SORT_BIG
நிலையான எழுத்து சாவி_separator;

நிலையான கட்டமைப்பு sort_சாவி {
	கட்டமைப்பு sort_சாவி *next_சாவி;  /* linked list */
	unsigned range[4];          /* start word, start char, end word, end char */
	unsigned flags;
} *சாவி_list;


/* This is a NOEXEC applet. Be very careful! */


நிலையான எழுத்து *get_சாவி(எழுத்து *str, கட்டமைப்பு sort_சாவி *சாவி, முழு flags)
{
	முழு start = start; /* for compiler */
	முழு end;
	முழு len, j;
	unsigned i;

	/* Special case whole string, so we don't have to make a copy */
	எனில் (சாவி->range[0] == 1 && !சாவி->range[1] && !சாவி->range[2] && !சாவி->range[3]
	 && !(flags & (FLAG_b | FLAG_d | FLAG_f | FLAG_i | FLAG_bb))
	) {
		திரும்பு str;
	}

	/* Find start of key on first pass, end on second pass */
	len = strlen(str);
	க்கு (j = 0; j < 2; j++) {
		எனில் (!சாவி->range[2*j])
			end = len;
		/* Loop through fields */
		இல்லையெனில் {
			unsigned எழுத்து ch = 0;

			end = 0;
			க்கு (i = 1; i < சாவி->range[2*j] + j; i++) {
				எனில் (சாவி_separator) {
					/* Skip body of key and separator */
					போது ((ch = str[end]) != '\0') {
							end++;
						எனில் (ch == சாவி_separator)
							நிறுத்து;
					}
				} இல்லையெனில் {
					/* Skip leading blanks */
					போது (isspace(str[end]))
						end++;
					/* Skip body of key */
					போது (str[end] != '\0') {
						எனில் (isspace(str[end]))
							நிறுத்து;
						end++;
					}
				}
			}
			/* Remove last delim: "abc:def:" => "abc:def" */
			எனில் (j && ch) {
				//if (str[end-1] != key_separator)
				//  bb_error_msg(_and_die("BUG! "
				//  "str[start:%d,end:%d]:'%.*s'",
				//  start, end, (int)(end-start), &str[start]);
				end--;
			}
		}
		எனில் (!j) start = end;
	}
	/* Strip leading whitespace if necessary */
	எனில் (flags & FLAG_b)
		/* not using skip_whitespace() for speed */
		போது (isspace(str[start])) start++;
	/* Strip trailing whitespace if necessary */
	எனில் (flags & FLAG_bb)
		போது (end > start && isspace(str[end-1])) end--;
	/* -kSTART,N.ENDCHAR: honor ENDCHAR (1-based) */
	எனில் (சாவி->range[3]) {
		end = சாவி->range[3];
		எனில் (end > len) end = len;
	}
	/* -kN.STARTCHAR[,...]: honor STARTCHAR (1-based) */
	எனில் (சாவி->range[1]) {
		start += சாவி->range[1] - 1;
		எனில் (start > len) start = len;
	}
	/* Make the copy */
	எனில் (end < start)
		end = start;
	str = xstrndup(str+start, end-start);
	/* Handle -d */
	எனில் (flags & FLAG_d) {
		க்கு (start = end = 0; str[end]; end++)
			எனில் (isspace(str[end]) || isalnum(str[end]))
				str[start++] = str[end];
		str[start] = '\0';
	}
	/* Handle -i */
	எனில் (flags & FLAG_i) {
		க்கு (start = end = 0; str[end]; end++)
			எனில் (isprint_asciionly(str[end]))
				str[start++] = str[end];
		str[start] = '\0';
	}
	/* Handle -f */
	எனில் (flags & FLAG_f)
		க்கு (i = 0; str[i]; i++)
			str[i] = toupper(str[i]);

	திரும்பு str;
}

நிலையான கட்டமைப்பு sort_சாவி *add_சாவி(வெறுமை)
{
	கட்டமைப்பு sort_சாவி **pkey = &சாவி_list;
	போது (*pkey)
		pkey = &((*pkey)->next_சாவி);
	திரும்பு *pkey = xzalloc(அளவு(கட்டமைப்பு sort_சாவி));
}

#define GET_வரி(fp) \
	((option_mask32 & FLAG_z) \
	? bb_get_chunk_from_file(fp, வெற்று) \
	: xmalloc_fgetline(fp))
#இல்லையெனில்
#define GET_வரி(fp) xmalloc_fgetline(fp)
#endif

#எனில் ENABLE_FEATURE_SORT_BIG
நிலையான முழு scale_suffix(மாறிலி எழுத்து *tail)
{
	நிலையான மாறிலி எழுத்து suffix[] ALIGN1 = "kmgtpezy";
	மாறிலி எழுத்து *s;
	முழு n;

	எனில் (!tail[0])
		திரும்பு -1;
	s = strchr(suffix, tail[0] | 0x20);
	எனில் (!s)
		திரும்பு -1;
	n = s - suffix;
	எனில் (n != 0 && tail[0] >= 'a')
		திரும்பு -1; /* mg... not accepted, only MG... */
	திரும்பு n;
}
#endif

/* Iterate through keys list and perform comparisons */
நிலையான முழு compare_சாவிகள்(மாறிலி வெறுமை *xarg, மாறிலி வெறுமை *yarg)
{
	முழு flags = option_mask32, retval = 0;
	எழுத்து *x, *y;

#எனில் ENABLE_FEATURE_SORT_BIG
	கட்டமைப்பு sort_சாவி *சாவி;

	க்கு (சாவி = சாவி_list; !retval && சாவி; சாவி = சாவி->next_சாவி) {
		flags = சாவி->flags ? சாவி->flags : option_mask32;
		/* Chop out and modify key chunks, handling -dfib */
		x = get_சாவி(*(எழுத்து **)xarg, சாவி, flags);
		y = get_சாவி(*(எழுத்து **)yarg, சாவி, flags);
#இல்லையெனில்
	/* This curly bracket serves no purpose but to match the nesting
	 * level of the for () loop we're not using */
	{
		x = *(எழுத்து **)xarg;
		y = *(எழுத்து **)yarg;
#endif
		/* Perform actual comparison */
		தேர்வு (flags & (FLAG_n | FLAG_g | FLAG_h | FLAG_M | FLAG_V)) {
		இயல்பு:
			bb_simple_error_செய்தி_and_die("unknown sort type");
			நிறுத்து;
#எனில் defined(HAVE_STRVERSCMP) && HAVE_STRVERSCMP == 1
		நிலை FLAG_V:
			retval = strverscmp(x, y);
			நிறுத்து;
#endif
		/* Ascii sort */
		நிலை 0:
#எனில் ENABLE_LOCALE_SUPPORT
			retval = strcoll(x, y);
#இல்லையெனில்
			retval = strcmp(x, y);
#endif
			நிறுத்து;
#எனில் ENABLE_FEATURE_SORT_BIG
		நிலை FLAG_g:
		நிலை FLAG_h: {
			எழுத்து *xx, *yy;
//TODO: needs setlocale(LC_NUMERIC, "C")?
			இரட்டை dx = strtod(x, &xx);
			இரட்டை dy = strtod(y, &yy);
			/* not numbers < NaN < -infinity < numbers < +infinity) */
			எனில் (x == xx)
				retval = (y == yy ? 0 : -1);
			இல்லையெனில் எனில் (y == yy)
				retval = 1;
			/* Check for isnan */
			இல்லையெனில் எனில் (dx != dx)
				retval = (dy != dy) ? 0 : -1;
			இல்லையெனில் எனில் (dy != dy)
				retval = 1;
			இல்லையெனில் {
				எனில் (flags & FLAG_h) {
					முழு xs = scale_suffix(xx);
					முழு ys = scale_suffix(yy);
					எனில் (xs != ys) {
						retval = xs - ys;
						நிறுத்து;
					}
				}
				/* Check for infinity.  Could underflow, but it avoids libm. */
				எனில் (1.0 / dx == 0.0) {
					எனில் (dx < 0)
						retval = (1.0 / dy == 0.0 && dy < 0) ? 0 : -1;
					இல்லையெனில்
						retval = (1.0 / dy == 0.0 && dy > 0) ? 0 : 1;
				} இல்லையெனில் எனில் (1.0 / dy == 0.0)
					retval = (dy < 0) ? 1 : -1;
				இல்லையெனில்
					retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			}
			நிறுத்து;
		}
		நிலை FLAG_M: {
			கட்டமைப்பு tm thyme;
			முழு dx;
			எழுத்து *xx, *yy;

			xx = strptime(skip_whitespace(x), "%b", &thyme);
			dx = thyme.tm_mon;
			yy = strptime(skip_whitespace(y), "%b", &thyme);
			எனில் (!xx)
				retval = (!yy) ? 0 : -1;
			இல்லையெனில் எனில் (!yy)
				retval = 1;
			இல்லையெனில்
				retval = dx - thyme.tm_mon;
			நிறுத்து;
		}
		/* Full floating point version of -n */
		நிலை FLAG_n: {
			இரட்டை dx = atof(x);
			இரட்டை dy = atof(y);
			retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			நிறுத்து;
		}
		} /* switch */
		/* Free key copies. */
		எனில் (x != *(எழுத்து **)xarg) விடுவி(x);
		எனில் (y != *(எழுத்து **)yarg) விடுவி(y);
		/* if (retval) break; - done by for () anyway */
#இல்லையெனில்
		/* Integer version of -n for tiny systems */
		நிலை FLAG_n:
			retval = atoi(x) - atoi(y);
			நிறுத்து;
		} /* switch */
#endif
	} /* for */

	எனில் (retval == 0) {
		/* So far lines are "the same" */

		எனில் (option_mask32 & FLAG_s) {
			/* "Stable sort": later line is "greater than",
			 * IOW: do not allow qsort() to swap equal lines.
			 */
			uint32_t *p32;
			uint32_t x32, y32;
			எழுத்து *வரி;
			unsigned len;

			வரி = *(எழுத்து**)xarg;
			len = (strlen(வரி) + 4) & (~3u);
			p32 = (வெறுமை*)(வரி + len);
			x32 = *p32;
			வரி = *(எழுத்து**)yarg;
			len = (strlen(வரி) + 4) & (~3u);
			p32 = (வெறுமை*)(வரி + len);
			y32 = *p32;

			/* If x > y, 1, else -1 */
			retval = (x32 > y32) * 2 - 1;
			/* Here, -r has no effect! */
			திரும்பு retval;
		}
		எனில் (!(option_mask32 & FLAG_no_tie_break)) {
			/* fallback sort */
			flags = option_mask32;
			retval = strcmp(*(எழுத்து **)xarg, *(எழுத்து **)yarg);
		}
	}

	எனில் (flags & FLAG_r)
		திரும்பு -retval;

	திரும்பு retval;
}

#எனில் ENABLE_FEATURE_SORT_BIG
நிலையான unsigned str2u(எழுத்து **str)
{
	unsigned long lu;
	எனில் (!isdigit((*str)[0]))
		bb_simple_error_செய்தி_and_die("bad field specification");
	lu = strtoul(*str, str, 10);
	எனில் ((அளவு(long) > அளவு(முழு) && lu > INT_MAX) || !lu)
		bb_simple_error_செய்தி_and_die("bad field specification");
	திரும்பு lu;
}
#endif

முழு sort_main(முழு argc, எழுத்து **argv) MAIN_EXTERNALLY_VISIBLE;
முழு sort_main(முழு argc UNUSED_PARAM, எழுத்து **argv)
{
	எழுத்து **வரிகள்;
	எழுத்து *str_ignored, *str_o, *str_t;
	llist_t *lst_k = வெற்று;
	முழு i;
	முழு linecount;
	unsigned opts;
#எனில் ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
	bool can_drop_dups;
	size_t prev_len = 0;
	எழுத்து *prev_வரி = (எழுத்து*) "";
	/* Postpone optimizing if the input is small, < 16k lines:
	 * even just free()ing duplicate lines takes time.
	 */
	size_t மொத்தம்_to_optimize_dups = 0x3fff;
#endif

	xfunc_error_retval = 2;

	/* Parse command line options */
	opts = getopt32(argv,
			sort_opt_str,
			&str_ignored, &str_ignored, &str_o, &lst_k, &str_t
	);
#எனில் ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
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
	எனில் (opts & FLAG_s)
		மொத்தம்_to_optimize_dups = (size_t)-1L;
#endif
	/* global b strips leading and trailing spaces */
	எனில் (opts & FLAG_b)
		option_mask32 |= FLAG_bb;
#எனில் ENABLE_FEATURE_SORT_BIG
	எனில் (opts & FLAG_t) {
		எனில் (!str_t[0] || str_t[1])
			bb_simple_error_செய்தி_and_die("bad -t parameter");
		சாவி_separator = str_t[0];
	}
	/* note: below this point we use option_mask32, not opts,
	 * since that reduces register pressure and makes code smaller */

	/* Parse sort key */
	போது (lst_k) {
		எண்ணிக்கை {
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
		கட்டமைப்பு sort_சாவி *சாவி = add_சாவி();
		எழுத்து *str_k = llist_எடு(&lst_k);

		i = 0; /* i==0 before comma, 1 after (-k3,6) */
		போது (*str_k) {
			/* Start of range */
			/* Cannot use bb_strtou - suffix can be a letter */
			சாவி->range[2*i] = str2u(&str_k);
			எனில் (*str_k == '.') {
				str_k++;
				சாவி->range[2*i+1] = str2u(&str_k);
			}
			போது (*str_k) {
				முழு flag;
				மாறிலி எழுத்து *idx;

				எனில் (*str_k == ',' && !i++) {
					str_k++;
					நிறுத்து;
				} /* no else needed: fall through to syntax error
					because comma isn't in OPT_STR */
				idx = strchr(OPT_STR, *str_k);
				எனில் (!idx)
					bb_simple_error_செய்தி_and_die("unknown key option");
				flag = 1 << (idx - OPT_STR);
				எனில் (flag & ~FLAG_allowed_for_k)
					bb_simple_error_செய்தி_and_die("unknown sort type");
				/* b after ',' means strip _trailing_ space */
				எனில் (i && flag == FLAG_b)
					flag = FLAG_bb;
				சாவி->flags |= flag;
				str_k++;
			}
		}
	}
#endif

	/* Open input files and read data */
	argv += optind;
	எனில் (!*argv)
		*--argv = (எழுத்து*)"-";
	linecount = 0;
	வரிகள் = வெற்று;
	செய் {
		/* coreutils 6.9 compat: abort on first open error,
		 * do not continue to next file: */
		FILE *fp = xfopen_stdin(*argv);
		க்கு (;;) {
			எழுத்து *வரி = GET_வரி(fp);
			எனில் (!வரி)
				நிறுத்து;

#எனில் ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
			எனில் (மொத்தம்_to_optimize_dups != 0)
				மொத்தம்_to_optimize_dups--;
			எனில் (மொத்தம்_to_optimize_dups == 0) {
				size_t len;
				எழுத்து *new_வரி;

				/* On kernel/linux/arch/ *.[ch] files,
				 * this reduces memory usage by 6%.
				 *  yes | head -99999999 | sort
				 * goes down from 1900Mb to 380 Mb.
				 */
				len = strlen(வரி);
				எனில் (len <= prev_len) {
					new_வரி = prev_வரி + (prev_len - len);
					எனில் (strcmp(வரி, new_வரி) == 0) {
						/* it's a tail of the prev line */
						எனில் (can_drop_dups && prev_len == len) {
							/* it's identical to prev line */
							விடுவி(வரி);
							தொடர்;
						}
						விடுவி(வரி);
						வரி = new_வரி;
						/* continue using longer prev_line
						 * for future tail tests.
						 */
						செல் skip;
					}
				}
				prev_len = len;
				prev_வரி = வரி;
 skip: ;
			}
#இல்லையெனில்
//TODO: lighter version which only drops total dups if can_drop_dups == true
#endif
			வரிகள் = xrealloc_vector(வரிகள், 6, linecount);
			வரிகள்[linecount++] = வரி;
		}
		fclose_if_not_stdin(fp);
	} போது (*++argv);

#எனில் ENABLE_FEATURE_SORT_BIG
	/* If no key, perform alphabetic sort */
	எனில் (!சாவி_list)
		add_சாவி()->range[0] = 1;
	/* Handle -c */
	எனில் (option_mask32 & FLAG_c) {
		முழு j = (option_mask32 & FLAG_u) ? -1 : 0;
		க்கு (i = 1; i < linecount; i++) {
			எனில் (compare_சாவிகள்(&வரிகள்[i-1], &வரிகள்[i]) > j) {
				fprintf(stderr, "Check line %u\n", i);
				திரும்பு EXIT_FAILURE;
			}
		}
		திரும்பு EXIT_SUCCESS;
	}
#endif

	/* For stable sort, store original line position beyond terminating NUL */
	எனில் (option_mask32 & FLAG_s) {
		க்கு (i = 0; i < linecount; i++) {
			uint32_t *p32;
			எழுத்து *வரி;
			unsigned len;

			வரி = வரிகள்[i];
			len = (strlen(வரி) + 4) & (~3u);
			வரிகள்[i] = வரி = xrealloc(வரி, len + 4);
			p32 = (வெறுமை*)(வரி + len);
			*p32 = i;
		}
		/*option_mask32 |= FLAG_no_tie_break;*/
		/* ^^^redundant: if FLAG_s, compare_keys() does no tie break */
	}

	/* Perform the actual sort */
	qsort(வரிகள், linecount, அளவு(வரிகள்[0]), compare_சாவிகள்);

	/* Handle -u */
	எனில் (option_mask32 & FLAG_u) {
		முழு j = 0;
		/* coreutils 6.3 drop lines for which only key is the same:
		 * - disabling last-resort compare, or else compare_keys()
		 * will be the same only for completely identical lines
		 * - disabling -s (same reasons)
		 */
		option_mask32 = (option_mask32 | FLAG_no_tie_break) & (~FLAG_s);
		க்கு (i = 1; i < linecount; i++) {
			எனில் (compare_சாவிகள்(&வரிகள்[j], &வரிகள்[i]) == 0)
				விடுவி(வரிகள்[i]);
			இல்லையெனில்
				வரிகள்[++j] = வரிகள்[i];
		}
		எனில் (linecount)
			linecount = j+1;
	}

	/* Print it */
#எனில் ENABLE_FEATURE_SORT_BIG
	/* Open output file _after_ we read all input ones */
	எனில் (option_mask32 & FLAG_o)
		xmove_fd(xopen(str_o, O_WRONLY|O_CREAT|O_TRUNC), STDOUT_FILENO);
#endif
	{
		முழு ch = (option_mask32 & FLAG_z) ? '\0' : '\n';
		க்கு (i = 0; i < linecount; i++)
			printf("%s%c", வரிகள்[i], ch);
	}

	fflush_stdout_and_exit_SUCCESS();
}
