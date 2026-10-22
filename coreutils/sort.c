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

स्थिर ध्रुव वर्ण sort_opt_str[] ALIGN1 = "^"
			"nghMVucszbrdfimS:T:o:k:*t:"
			"\0" "o--o:t--t"/*-t, -o: at most one of each*/;
/*
 * OPT_STR must not be string literal, needs to have stable address:
 * code uses "strchr(OPT_STR,c) - OPT_STR" idiom.
 */
#define OPT_STR (sort_opt_str + 1)

#सचे ENABLE_FEATURE_SORT_BIG
स्थिर वर्ण कुञ्चिका_separator;

स्थिर संरचना sort_कुञ्चिका {
	संरचना sort_कुञ्चिका *next_कुञ्चिका;  /* linked list */
	unsigned range[4];          /* start word, start char, end word, end char */
	unsigned flags;
} *कुञ्चिका_list;


/* This is a NOEXEC applet. Be very careful! */


स्थिर वर्ण *get_कुञ्चिका(वर्ण *str, संरचना sort_कुञ्चिका *कुञ्चिका, पूर्णाङ्क flags)
{
	पूर्णाङ्क start = start; /* for compiler */
	पूर्णाङ्क end;
	पूर्णाङ्क len, j;
	unsigned i;

	/* Special case whole string, so we don't have to make a copy */
	सचे (कुञ्चिका->range[0] == 1 && !कुञ्चिका->range[1] && !कुञ्चिका->range[2] && !कुञ्चिका->range[3]
	 && !(flags & (FLAG_b | FLAG_d | FLAG_f | FLAG_i | FLAG_bb))
	) {
		पटिनिवत्त str;
	}

	/* Find start of key on first pass, end on second pass */
	len = strlen(str);
	अत्थाय (j = 0; j < 2; j++) {
		सचे (!कुञ्चिका->range[2*j])
			end = len;
		/* Loop through fields */
		अञ्ञथा {
			unsigned वर्ण ch = 0;

			end = 0;
			अत्थाय (i = 1; i < कुञ्चिका->range[2*j] + j; i++) {
				सचे (कुञ्चिका_separator) {
					/* Skip body of key and separator */
					याव ((ch = str[end]) != '\0') {
							end++;
						सचे (ch == कुञ्चिका_separator)
							भङ्ग;
					}
				} अञ्ञथा {
					/* Skip leading blanks */
					याव (isspace(str[end]))
						end++;
					/* Skip body of key */
					याव (str[end] != '\0') {
						सचे (isspace(str[end]))
							भङ्ग;
						end++;
					}
				}
			}
			/* Remove last delim: "abc:def:" => "abc:def" */
			सचे (j && ch) {
				//if (str[end-1] != key_separator)
				//  bb_error_msg(_and_die("BUG! "
				//  "str[start:%d,end:%d]:'%.*s'",
				//  start, end, (int)(end-start), &str[start]);
				end--;
			}
		}
		सचे (!j) start = end;
	}
	/* Strip leading whitespace if necessary */
	सचे (flags & FLAG_b)
		/* not using skip_whitespace() for speed */
		याव (isspace(str[start])) start++;
	/* Strip trailing whitespace if necessary */
	सचे (flags & FLAG_bb)
		याव (end > start && isspace(str[end-1])) end--;
	/* -kSTART,N.ENDCHAR: honor ENDCHAR (1-based) */
	सचे (कुञ्चिका->range[3]) {
		end = कुञ्चिका->range[3];
		सचे (end > len) end = len;
	}
	/* -kN.STARTCHAR[,...]: honor STARTCHAR (1-based) */
	सचे (कुञ्चिका->range[1]) {
		start += कुञ्चिका->range[1] - 1;
		सचे (start > len) start = len;
	}
	/* Make the copy */
	सचे (end < start)
		end = start;
	str = xstrndup(str+start, end-start);
	/* Handle -d */
	सचे (flags & FLAG_d) {
		अत्थाय (start = end = 0; str[end]; end++)
			सचे (isspace(str[end]) || isalnum(str[end]))
				str[start++] = str[end];
		str[start] = '\0';
	}
	/* Handle -i */
	सचे (flags & FLAG_i) {
		अत्थाय (start = end = 0; str[end]; end++)
			सचे (isprint_asciionly(str[end]))
				str[start++] = str[end];
		str[start] = '\0';
	}
	/* Handle -f */
	सचे (flags & FLAG_f)
		अत्थाय (i = 0; str[i]; i++)
			str[i] = toupper(str[i]);

	पटिनिवत्त str;
}

स्थिर संरचना sort_कुञ्चिका *add_कुञ्चिका(शून्य)
{
	संरचना sort_कुञ्चिका **pkey = &कुञ्चिका_list;
	याव (*pkey)
		pkey = &((*pkey)->next_कुञ्चिका);
	पटिनिवत्त *pkey = xzalloc(परिमाण(संरचना sort_कुञ्चिका));
}

#define GET_पङ्क्तिः(fp) \
	((option_mask32 & FLAG_z) \
	? bb_get_chunk_from_file(fp, रिक्त) \
	: xmalloc_fgetline(fp))
#अञ्ञथा
#define GET_पङ्क्तिः(fp) xmalloc_fgetline(fp)
#endif

#सचे ENABLE_FEATURE_SORT_BIG
स्थिर पूर्णाङ्क scale_suffix(ध्रुव वर्ण *tail)
{
	स्थिर ध्रुव वर्ण suffix[] ALIGN1 = "kmgtpezy";
	ध्रुव वर्ण *s;
	पूर्णाङ्क n;

	सचे (!tail[0])
		पटिनिवत्त -1;
	s = strchr(suffix, tail[0] | 0x20);
	सचे (!s)
		पटिनिवत्त -1;
	n = s - suffix;
	सचे (n != 0 && tail[0] >= 'a')
		पटिनिवत्त -1; /* mg... not accepted, only MG... */
	पटिनिवत्त n;
}
#endif

/* Iterate through keys list and perform comparisons */
स्थिर पूर्णाङ्क compare_कुञ्चिकाः(ध्रुव शून्य *xarg, ध्रुव शून्य *yarg)
{
	पूर्णाङ्क flags = option_mask32, retval = 0;
	वर्ण *x, *y;

#सचे ENABLE_FEATURE_SORT_BIG
	संरचना sort_कुञ्चिका *कुञ्चिका;

	अत्थाय (कुञ्चिका = कुञ्चिका_list; !retval && कुञ्चिका; कुञ्चिका = कुञ्चिका->next_कुञ्चिका) {
		flags = कुञ्चिका->flags ? कुञ्चिका->flags : option_mask32;
		/* Chop out and modify key chunks, handling -dfib */
		x = get_कुञ्चिका(*(वर्ण **)xarg, कुञ्चिका, flags);
		y = get_कुञ्चिका(*(वर्ण **)yarg, कुञ्चिका, flags);
#अञ्ञथा
	/* This curly bracket serves no purpose but to match the nesting
	 * level of the for () loop we're not using */
	{
		x = *(वर्ण **)xarg;
		y = *(वर्ण **)yarg;
#endif
		/* Perform actual comparison */
		विकल्प (flags & (FLAG_n | FLAG_g | FLAG_h | FLAG_M | FLAG_V)) {
		स्वभाव:
			bb_simple_error_सन्देशः_and_die("unknown sort type");
			भङ्ग;
#सचे defined(HAVE_STRVERSCMP) && HAVE_STRVERSCMP == 1
		पक्ष FLAG_V:
			retval = strverscmp(x, y);
			भङ्ग;
#endif
		/* Ascii sort */
		पक्ष 0:
#सचे ENABLE_LOCALE_SUPPORT
			retval = strcoll(x, y);
#अञ्ञथा
			retval = strcmp(x, y);
#endif
			भङ्ग;
#सचे ENABLE_FEATURE_SORT_BIG
		पक्ष FLAG_g:
		पक्ष FLAG_h: {
			वर्ण *xx, *yy;
//TODO: needs setlocale(LC_NUMERIC, "C")?
			द्विगुण dx = strtod(x, &xx);
			द्विगुण dy = strtod(y, &yy);
			/* not numbers < NaN < -infinity < numbers < +infinity) */
			सचे (x == xx)
				retval = (y == yy ? 0 : -1);
			अञ्ञथा सचे (y == yy)
				retval = 1;
			/* Check for isnan */
			अञ्ञथा सचे (dx != dx)
				retval = (dy != dy) ? 0 : -1;
			अञ्ञथा सचे (dy != dy)
				retval = 1;
			अञ्ञथा {
				सचे (flags & FLAG_h) {
					पूर्णाङ्क xs = scale_suffix(xx);
					पूर्णाङ्क ys = scale_suffix(yy);
					सचे (xs != ys) {
						retval = xs - ys;
						भङ्ग;
					}
				}
				/* Check for infinity.  Could underflow, but it avoids libm. */
				सचे (1.0 / dx == 0.0) {
					सचे (dx < 0)
						retval = (1.0 / dy == 0.0 && dy < 0) ? 0 : -1;
					अञ्ञथा
						retval = (1.0 / dy == 0.0 && dy > 0) ? 0 : 1;
				} अञ्ञथा सचे (1.0 / dy == 0.0)
					retval = (dy < 0) ? 1 : -1;
				अञ्ञथा
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
			सचे (!xx)
				retval = (!yy) ? 0 : -1;
			अञ्ञथा सचे (!yy)
				retval = 1;
			अञ्ञथा
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
		सचे (x != *(वर्ण **)xarg) मुक्त(x);
		सचे (y != *(वर्ण **)yarg) मुक्त(y);
		/* if (retval) break; - done by for () anyway */
#अञ्ञथा
		/* Integer version of -n for tiny systems */
		पक्ष FLAG_n:
			retval = atoi(x) - atoi(y);
			भङ्ग;
		} /* switch */
#endif
	} /* for */

	सचे (retval == 0) {
		/* So far lines are "the same" */

		सचे (option_mask32 & FLAG_s) {
			/* "Stable sort": later line is "greater than",
			 * IOW: do not allow qsort() to swap equal lines.
			 */
			uint32_t *p32;
			uint32_t x32, y32;
			वर्ण *पङ्क्तिः;
			unsigned len;

			पङ्क्तिः = *(वर्ण**)xarg;
			len = (strlen(पङ्क्तिः) + 4) & (~3u);
			p32 = (शून्य*)(पङ्क्तिः + len);
			x32 = *p32;
			पङ्क्तिः = *(वर्ण**)yarg;
			len = (strlen(पङ्क्तिः) + 4) & (~3u);
			p32 = (शून्य*)(पङ्क्तिः + len);
			y32 = *p32;

			/* If x > y, 1, else -1 */
			retval = (x32 > y32) * 2 - 1;
			/* Here, -r has no effect! */
			पटिनिवत्त retval;
		}
		सचे (!(option_mask32 & FLAG_no_tie_break)) {
			/* fallback sort */
			flags = option_mask32;
			retval = strcmp(*(वर्ण **)xarg, *(वर्ण **)yarg);
		}
	}

	सचे (flags & FLAG_r)
		पटिनिवत्त -retval;

	पटिनिवत्त retval;
}

#सचे ENABLE_FEATURE_SORT_BIG
स्थिर unsigned str2u(वर्ण **str)
{
	unsigned long lu;
	सचे (!isdigit((*str)[0]))
		bb_simple_error_सन्देशः_and_die("bad field specification");
	lu = strtoul(*str, str, 10);
	सचे ((परिमाण(long) > परिमाण(पूर्णाङ्क) && lu > INT_MAX) || !lu)
		bb_simple_error_सन्देशः_and_die("bad field specification");
	पटिनिवत्त lu;
}
#endif

पूर्णाङ्क sort_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क sort_main(पूर्णाङ्क argc UNUSED_PARAM, वर्ण **argv)
{
	वर्ण **पङ्क्तयः;
	वर्ण *str_ignored, *str_o, *str_t;
	llist_t *lst_k = रिक्त;
	पूर्णाङ्क i;
	पूर्णाङ्क linecount;
	unsigned opts;
#सचे ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
	bool can_drop_dups;
	size_t prev_len = 0;
	वर्ण *prev_पङ्क्तिः = (वर्ण*) "";
	/* Postpone optimizing if the input is small, < 16k lines:
	 * even just free()ing duplicate lines takes time.
	 */
	size_t गणनम्_to_optimize_dups = 0x3fff;
#endif

	xfunc_error_retval = 2;

	/* Parse command line options */
	opts = getopt32(argv,
			sort_opt_str,
			&str_ignored, &str_ignored, &str_o, &lst_k, &str_t
	);
#सचे ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
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
	सचे (opts & FLAG_s)
		गणनम्_to_optimize_dups = (size_t)-1L;
#endif
	/* global b strips leading and trailing spaces */
	सचे (opts & FLAG_b)
		option_mask32 |= FLAG_bb;
#सचे ENABLE_FEATURE_SORT_BIG
	सचे (opts & FLAG_t) {
		सचे (!str_t[0] || str_t[1])
			bb_simple_error_सन्देशः_and_die("bad -t parameter");
		कुञ्चिका_separator = str_t[0];
	}
	/* note: below this point we use option_mask32, not opts,
	 * since that reduces register pressure and makes code smaller */

	/* Parse sort key */
	याव (lst_k) {
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
		संरचना sort_कुञ्चिका *कुञ्चिका = add_कुञ्चिका();
		वर्ण *str_k = llist_निष्कासय(&lst_k);

		i = 0; /* i==0 before comma, 1 after (-k3,6) */
		याव (*str_k) {
			/* Start of range */
			/* Cannot use bb_strtou - suffix can be a letter */
			कुञ्चिका->range[2*i] = str2u(&str_k);
			सचे (*str_k == '.') {
				str_k++;
				कुञ्चिका->range[2*i+1] = str2u(&str_k);
			}
			याव (*str_k) {
				पूर्णाङ्क flag;
				ध्रुव वर्ण *idx;

				सचे (*str_k == ',' && !i++) {
					str_k++;
					भङ्ग;
				} /* no else needed: fall through to syntax error
					because comma isn't in OPT_STR */
				idx = strchr(OPT_STR, *str_k);
				सचे (!idx)
					bb_simple_error_सन्देशः_and_die("unknown key option");
				flag = 1 << (idx - OPT_STR);
				सचे (flag & ~FLAG_allowed_for_k)
					bb_simple_error_सन्देशः_and_die("unknown sort type");
				/* b after ',' means strip _trailing_ space */
				सचे (i && flag == FLAG_b)
					flag = FLAG_bb;
				कुञ्चिका->flags |= flag;
				str_k++;
			}
		}
	}
#endif

	/* Open input files and read data */
	argv += optind;
	सचे (!*argv)
		*--argv = (वर्ण*)"-";
	linecount = 0;
	पङ्क्तयः = रिक्त;
	करोहि {
		/* coreutils 6.9 compat: abort on first open error,
		 * do not continue to next file: */
		FILE *fp = xfopen_stdin(*argv);
		अत्थाय (;;) {
			वर्ण *पङ्क्तिः = GET_पङ्क्तिः(fp);
			सचे (!पङ्क्तिः)
				भङ्ग;

#सचे ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
			सचे (गणनम्_to_optimize_dups != 0)
				गणनम्_to_optimize_dups--;
			सचे (गणनम्_to_optimize_dups == 0) {
				size_t len;
				वर्ण *new_पङ्क्तिः;

				/* On kernel/linux/arch/ *.[ch] files,
				 * this reduces memory usage by 6%.
				 *  yes | head -99999999 | sort
				 * goes down from 1900Mb to 380 Mb.
				 */
				len = strlen(पङ्क्तिः);
				सचे (len <= prev_len) {
					new_पङ्क्तिः = prev_पङ्क्तिः + (prev_len - len);
					सचे (strcmp(पङ्क्तिः, new_पङ्क्तिः) == 0) {
						/* it's a tail of the prev line */
						सचे (can_drop_dups && prev_len == len) {
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
				prev_len = len;
				prev_पङ्क्तिः = पङ्क्तिः;
 skip: ;
			}
#अञ्ञथा
//TODO: lighter version which only drops total dups if can_drop_dups == true
#endif
			पङ्क्तयः = xrealloc_vector(पङ्क्तयः, 6, linecount);
			पङ्क्तयः[linecount++] = पङ्क्तिः;
		}
		fclose_if_not_stdin(fp);
	} याव (*++argv);

#सचे ENABLE_FEATURE_SORT_BIG
	/* If no key, perform alphabetic sort */
	सचे (!कुञ्चिका_list)
		add_कुञ्चिका()->range[0] = 1;
	/* Handle -c */
	सचे (option_mask32 & FLAG_c) {
		पूर्णाङ्क j = (option_mask32 & FLAG_u) ? -1 : 0;
		अत्थाय (i = 1; i < linecount; i++) {
			सचे (compare_कुञ्चिकाः(&पङ्क्तयः[i-1], &पङ्क्तयः[i]) > j) {
				fprintf(stderr, "Check line %u\n", i);
				पटिनिवत्त EXIT_FAILURE;
			}
		}
		पटिनिवत्त EXIT_SUCCESS;
	}
#endif

	/* For stable sort, store original line position beyond terminating NUL */
	सचे (option_mask32 & FLAG_s) {
		अत्थाय (i = 0; i < linecount; i++) {
			uint32_t *p32;
			वर्ण *पङ्क्तिः;
			unsigned len;

			पङ्क्तिः = पङ्क्तयः[i];
			len = (strlen(पङ्क्तिः) + 4) & (~3u);
			पङ्क्तयः[i] = पङ्क्तिः = xrealloc(पङ्क्तिः, len + 4);
			p32 = (शून्य*)(पङ्क्तिः + len);
			*p32 = i;
		}
		/*option_mask32 |= FLAG_no_tie_break;*/
		/* ^^^redundant: if FLAG_s, compare_keys() does no tie break */
	}

	/* Perform the actual sort */
	qsort(पङ्क्तयः, linecount, परिमाण(पङ्क्तयः[0]), compare_कुञ्चिकाः);

	/* Handle -u */
	सचे (option_mask32 & FLAG_u) {
		पूर्णाङ्क j = 0;
		/* coreutils 6.3 drop lines for which only key is the same:
		 * - disabling last-resort compare, or else compare_keys()
		 * will be the same only for completely identical lines
		 * - disabling -s (same reasons)
		 */
		option_mask32 = (option_mask32 | FLAG_no_tie_break) & (~FLAG_s);
		अत्थाय (i = 1; i < linecount; i++) {
			सचे (compare_कुञ्चिकाः(&पङ्क्तयः[j], &पङ्क्तयः[i]) == 0)
				मुक्त(पङ्क्तयः[i]);
			अञ्ञथा
				पङ्क्तयः[++j] = पङ्क्तयः[i];
		}
		सचे (linecount)
			linecount = j+1;
	}

	/* Print it */
#सचे ENABLE_FEATURE_SORT_BIG
	/* Open output file _after_ we read all input ones */
	सचे (option_mask32 & FLAG_o)
		xmove_fd(xopen(str_o, O_WRONLY|O_CREAT|O_TRUNC), STDOUT_FILENO);
#endif
	{
		पूर्णाङ्क ch = (option_mask32 & FLAG_z) ? '\0' : '\n';
		अत्थाय (i = 0; i < linecount; i++)
			printf("%s%c", पङ्क्तयः[i], ch);
	}

	fflush_stdout_and_exit_SUCCESS();
}
