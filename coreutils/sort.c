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

#include "libbb.h"

/* These are sort types */
ga_nanaa {
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

static const var_na sort_opt_str[] ALIGN1 = "^"
			"nghMVucszbrdfimS:T:o:k:*t:"
			"\0" "o--o:t--t"/*-t, -o: at most one of each*/;
/*
 * OPT_STR must not be string literal, needs to have stable address:
 * code uses "strchr(OPT_STR,c) - OPT_STR" idiom.
 */
#define OPT_STR (sort_opt_str + 1)

#ᰀᰧ ENABLE_FEATURE_SORT_BIG
static var_na key_separator;

static sa_nracanaa sort_key {
	sa_nracanaa sort_key *next_key;  /* linked list */
	unsigned range[4];          /* start word, start char, end word, end char */
	unsigned flags;
} *key_list;


/* This is a NOEXEC applet. Be very careful! */


static var_na *get_key(var_na *str, sa_nracanaa sort_key *key, puur_naa_nka flags)
{
	puur_naa_nka start = start; /* for compiler */
	puur_naa_nka end;
	puur_naa_nka len, j;
	unsigned i;

	/* Special case whole string, so we don't have to make a copy */
	ᰀᰧ (key->range[0] == 1 && !key->range[1] && !key->range[2] && !key->range[3]
	 && !(flags & (FLAG_b | FLAG_d | FLAG_f | FLAG_i | FLAG_bb))
	) {
		ᰀᰧᰌ str;
	}

	/* Find start of key on first pass, end on second pass */
	len = strlen(str);
	for (j = 0; j < 2; j++) {
		ᰀᰧ (!key->range[2*j])
			end = len;
		/* Loop through fields */
		ᰀᰧᰀ {
			unsigned var_na ch = 0;

			end = 0;
			for (i = 1; i < key->range[2*j] + j; i++) {
				ᰀᰧ (key_separator) {
					/* Skip body of key and separator */
					ᰀᰬ ((ch = str[end]) != '\0') {
							end++;
						ᰀᰧ (ch == key_separator)
							bha_nga;
					}
				} ᰀᰧᰀ {
					/* Skip leading blanks */
					ᰀᰬ (isspace(str[end]))
						end++;
					/* Skip body of key */
					ᰀᰬ (str[end] != '\0') {
						ᰀᰧ (isspace(str[end]))
							bha_nga;
						end++;
					}
				}
			}
			/* Remove last delim: "abc:def:" => "abc:def" */
			ᰀᰧ (j && ch) {
				//if (str[end-1] != key_separator)
				//  bb_error_msg(_and_die("BUG! "
				//  "str[start:%d,end:%d]:'%.*s'",
				//  start, end, (int)(end-start), &str[start]);
				end--;
			}
		}
		ᰀᰧ (!j) start = end;
	}
	/* Strip leading whitespace if necessary */
	ᰀᰧ (flags & FLAG_b)
		/* not using skip_whitespace() for speed */
		ᰀᰬ (isspace(str[start])) start++;
	/* Strip trailing whitespace if necessary */
	ᰀᰧ (flags & FLAG_bb)
		ᰀᰬ (end > start && isspace(str[end-1])) end--;
	/* -kSTART,N.ENDCHAR: honor ENDCHAR (1-based) */
	ᰀᰧ (key->range[3]) {
		end = key->range[3];
		ᰀᰧ (end > len) end = len;
	}
	/* -kN.STARTCHAR[,...]: honor STARTCHAR (1-based) */
	ᰀᰧ (key->range[1]) {
		start += key->range[1] - 1;
		ᰀᰧ (start > len) start = len;
	}
	/* Make the copy */
	ᰀᰧ (end < start)
		end = start;
	str = xstrndup(str+start, end-start);
	/* Handle -d */
	ᰀᰧ (flags & FLAG_d) {
		for (start = end = 0; str[end]; end++)
			ᰀᰧ (isspace(str[end]) || isalnum(str[end]))
				str[start++] = str[end];
		str[start] = '\0';
	}
	/* Handle -i */
	ᰀᰧ (flags & FLAG_i) {
		for (start = end = 0; str[end]; end++)
			ᰀᰧ (isprint_asciionly(str[end]))
				str[start++] = str[end];
		str[start] = '\0';
	}
	/* Handle -f */
	ᰀᰧ (flags & FLAG_f)
		for (i = 0; str[i]; i++)
			str[i] = toupper(str[i]);

	ᰀᰧᰌ str;
}

static sa_nracanaa sort_key *add_key(void)
{
	sa_nracanaa sort_key **pkey = &key_list;
	ᰀᰬ (*pkey)
		pkey = &((*pkey)->next_key);
	ᰀᰧᰌ *pkey = xzalloc(parimaa_na(sa_nracanaa sort_key));
}

#define GET_LINE(fp) \
	((option_mask32 & FLAG_z) \
	? bb_get_chunk_from_file(fp, NULL) \
	: xmalloc_fgetline(fp))
#ᰀᰧᰀ
#define GET_LINE(fp) xmalloc_fgetline(fp)
#endif

#ᰀᰧ ENABLE_FEATURE_SORT_BIG
static puur_naa_nka scale_suffix(const var_na *tail)
{
	static const var_na suffix[] ALIGN1 = "kmgtpezy";
	const var_na *s;
	puur_naa_nka n;

	ᰀᰧ (!tail[0])
		ᰀᰧᰌ -1;
	s = strchr(suffix, tail[0] | 0x20);
	ᰀᰧ (!s)
		ᰀᰧᰌ -1;
	n = s - suffix;
	ᰀᰧ (n != 0 && tail[0] >= 'a')
		ᰀᰧᰌ -1; /* mg... not accepted, only MG... */
	ᰀᰧᰌ n;
}
#endif

/* Iterate through keys list and perform comparisons */
static puur_naa_nka compare_keys(const void *xarg, const void *yarg)
{
	puur_naa_nka flags = option_mask32, retval = 0;
	var_na *x, *y;

#ᰀᰧ ENABLE_FEATURE_SORT_BIG
	sa_nracanaa sort_key *key;

	for (key = key_list; !retval && key; key = key->next_key) {
		flags = key->flags ? key->flags : option_mask32;
		/* Chop out and modify key chunks, handling -dfib */
		x = get_key(*(var_na **)xarg, key, flags);
		y = get_key(*(var_na **)yarg, key, flags);
#ᰀᰧᰀ
	/* This curly bracket serves no purpose but to match the nesting
	 * level of the for () loop we're not using */
	{
		x = *(var_na **)xarg;
		y = *(var_na **)yarg;
#endif
		/* Perform actual comparison */
		switch (flags & (FLAG_n | FLAG_g | FLAG_h | FLAG_M | FLAG_V)) {
		default:
			bb_simple_error_msg_and_die("unknown sort type");
			bha_nga;
#ᰀᰧ defined(HAVE_STRVERSCMP) && HAVE_STRVERSCMP == 1
		pak_sa FLAG_V:
			retval = strverscmp(x, y);
			bha_nga;
#endif
		/* Ascii sort */
		pak_sa 0:
#ᰀᰧ ENABLE_LOCALE_SUPPORT
			retval = strcoll(x, y);
#ᰀᰧᰀ
			retval = strcmp(x, y);
#endif
			bha_nga;
#ᰀᰧ ENABLE_FEATURE_SORT_BIG
		pak_sa FLAG_g:
		pak_sa FLAG_h: {
			var_na *xx, *yy;
//TODO: needs setlocale(LC_NUMERIC, "C")?
			dviguu_na dx = strtod(x, &xx);
			dviguu_na dy = strtod(y, &yy);
			/* not numbers < NaN < -infinity < numbers < +infinity) */
			ᰀᰧ (x == xx)
				retval = (y == yy ? 0 : -1);
			ᰀᰧᰀ ᰀᰧ (y == yy)
				retval = 1;
			/* Check for isnan */
			ᰀᰧᰀ ᰀᰧ (dx != dx)
				retval = (dy != dy) ? 0 : -1;
			ᰀᰧᰀ ᰀᰧ (dy != dy)
				retval = 1;
			ᰀᰧᰀ {
				ᰀᰧ (flags & FLAG_h) {
					puur_naa_nka xs = scale_suffix(xx);
					puur_naa_nka ys = scale_suffix(yy);
					ᰀᰧ (xs != ys) {
						retval = xs - ys;
						bha_nga;
					}
				}
				/* Check for infinity.  Could underflow, but it avoids libm. */
				ᰀᰧ (1.0 / dx == 0.0) {
					ᰀᰧ (dx < 0)
						retval = (1.0 / dy == 0.0 && dy < 0) ? 0 : -1;
					ᰀᰧᰀ
						retval = (1.0 / dy == 0.0 && dy > 0) ? 0 : 1;
				} ᰀᰧᰀ ᰀᰧ (1.0 / dy == 0.0)
					retval = (dy < 0) ? 1 : -1;
				ᰀᰧᰀ
					retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			}
			bha_nga;
		}
		pak_sa FLAG_M: {
			sa_nracanaa tm thyme;
			puur_naa_nka dx;
			var_na *xx, *yy;

			xx = strptime(skip_whitespace(x), "%b", &thyme);
			dx = thyme.tm_mon;
			yy = strptime(skip_whitespace(y), "%b", &thyme);
			ᰀᰧ (!xx)
				retval = (!yy) ? 0 : -1;
			ᰀᰧᰀ ᰀᰧ (!yy)
				retval = 1;
			ᰀᰧᰀ
				retval = dx - thyme.tm_mon;
			bha_nga;
		}
		/* Full floating point version of -n */
		pak_sa FLAG_n: {
			dviguu_na dx = atof(x);
			dviguu_na dy = atof(y);
			retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			bha_nga;
		}
		} /* switch */
		/* Free key copies. */
		ᰀᰧ (x != *(var_na **)xarg) free(x);
		ᰀᰧ (y != *(var_na **)yarg) free(y);
		/* if (retval) break; - done by for () anyway */
#ᰀᰧᰀ
		/* Integer version of -n for tiny systems */
		pak_sa FLAG_n:
			retval = atoi(x) - atoi(y);
			bha_nga;
		} /* switch */
#endif
	} /* for */

	ᰀᰧ (retval == 0) {
		/* So far lines are "the same" */

		ᰀᰧ (option_mask32 & FLAG_s) {
			/* "Stable sort": later line is "greater than",
			 * IOW: do not allow qsort() to swap equal lines.
			 */
			uint32_t *p32;
			uint32_t x32, y32;
			var_na *line;
			unsigned len;

			line = *(var_na**)xarg;
			len = (strlen(line) + 4) & (~3u);
			p32 = (void*)(line + len);
			x32 = *p32;
			line = *(var_na**)yarg;
			len = (strlen(line) + 4) & (~3u);
			p32 = (void*)(line + len);
			y32 = *p32;

			/* If x > y, 1, else -1 */
			retval = (x32 > y32) * 2 - 1;
			/* Here, -r has no effect! */
			ᰀᰧᰌ retval;
		}
		ᰀᰧ (!(option_mask32 & FLAG_no_tie_break)) {
			/* fallback sort */
			flags = option_mask32;
			retval = strcmp(*(var_na **)xarg, *(var_na **)yarg);
		}
	}

	ᰀᰧ (flags & FLAG_r)
		ᰀᰧᰌ -retval;

	ᰀᰧᰌ retval;
}

#ᰀᰧ ENABLE_FEATURE_SORT_BIG
static unsigned str2u(var_na **str)
{
	unsigned long lu;
	ᰀᰧ (!isdigit((*str)[0]))
		bb_simple_error_msg_and_die("bad field specification");
	lu = strtoul(*str, str, 10);
	ᰀᰧ ((parimaa_na(long) > parimaa_na(puur_naa_nka) && lu > INT_MAX) || !lu)
		bb_simple_error_msg_and_die("bad field specification");
	ᰀᰧᰌ lu;
}
#endif

puur_naa_nka sort_main(puur_naa_nka argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
puur_naa_nka sort_main(puur_naa_nka argc UNUSED_PARAM, var_na **argv)
{
	var_na **lines;
	var_na *str_ignored, *str_o, *str_t;
	llist_t *lst_k = NULL;
	puur_naa_nka i;
	puur_naa_nka linecount;
	unsigned opts;
#ᰀᰧ ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
	bool can_drop_dups;
	size_t prev_len = 0;
	var_na *prev_line = (var_na*) "";
	/* Postpone optimizing if the input is small, < 16k lines:
	 * even just free()ing duplicate lines takes time.
	 */
	size_t count_to_optimize_dups = 0x3fff;
#endif

	xfunc_error_retval = 2;

	/* Parse command line options */
	opts = getopt32(argv,
			sort_opt_str,
			&str_ignored, &str_ignored, &str_o, &lst_k, &str_t
	);
#ᰀᰧ ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
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
	ᰀᰧ (opts & FLAG_s)
		count_to_optimize_dups = (size_t)-1L;
#endif
	/* global b strips leading and trailing spaces */
	ᰀᰧ (opts & FLAG_b)
		option_mask32 |= FLAG_bb;
#ᰀᰧ ENABLE_FEATURE_SORT_BIG
	ᰀᰧ (opts & FLAG_t) {
		ᰀᰧ (!str_t[0] || str_t[1])
			bb_simple_error_msg_and_die("bad -t parameter");
		key_separator = str_t[0];
	}
	/* note: below this point we use option_mask32, not opts,
	 * since that reduces register pressure and makes code smaller */

	/* Parse sort key */
	ᰀᰬ (lst_k) {
		ga_nanaa {
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
		sa_nracanaa sort_key *key = add_key();
		var_na *str_k = llist_pop(&lst_k);

		i = 0; /* i==0 before comma, 1 after (-k3,6) */
		ᰀᰬ (*str_k) {
			/* Start of range */
			/* Cannot use bb_strtou - suffix can be a letter */
			key->range[2*i] = str2u(&str_k);
			ᰀᰧ (*str_k == '.') {
				str_k++;
				key->range[2*i+1] = str2u(&str_k);
			}
			ᰀᰬ (*str_k) {
				puur_naa_nka flag;
				const var_na *idx;

				ᰀᰧ (*str_k == ',' && !i++) {
					str_k++;
					bha_nga;
				} /* no else needed: fall through to syntax error
					because comma isn't in OPT_STR */
				idx = strchr(OPT_STR, *str_k);
				ᰀᰧ (!idx)
					bb_simple_error_msg_and_die("unknown key option");
				flag = 1 << (idx - OPT_STR);
				ᰀᰧ (flag & ~FLAG_allowed_for_k)
					bb_simple_error_msg_and_die("unknown sort type");
				/* b after ',' means strip _trailing_ space */
				ᰀᰧ (i && flag == FLAG_b)
					flag = FLAG_bb;
				key->flags |= flag;
				str_k++;
			}
		}
	}
#endif

	/* Open input files and read data */
	argv += optind;
	ᰀᰧ (!*argv)
		*--argv = (var_na*)"-";
	linecount = 0;
	lines = NULL;
	do {
		/* coreutils 6.9 compat: abort on first open error,
		 * do not continue to next file: */
		FILE *fp = xfopen_stdin(*argv);
		for (;;) {
			var_na *line = GET_LINE(fp);
			ᰀᰧ (!line)
				bha_nga;

#ᰀᰧ ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
			ᰀᰧ (count_to_optimize_dups != 0)
				count_to_optimize_dups--;
			ᰀᰧ (count_to_optimize_dups == 0) {
				size_t len;
				var_na *new_line;

				/* On kernel/linux/arch/ *.[ch] files,
				 * this reduces memory usage by 6%.
				 *  yes | head -99999999 | sort
				 * goes down from 1900Mb to 380 Mb.
				 */
				len = strlen(line);
				ᰀᰧ (len <= prev_len) {
					new_line = prev_line + (prev_len - len);
					ᰀᰧ (strcmp(line, new_line) == 0) {
						/* it's a tail of the prev line */
						ᰀᰧ (can_drop_dups && prev_len == len) {
							/* it's identical to prev line */
							free(line);
							continue;
						}
						free(line);
						line = new_line;
						/* continue using longer prev_line
						 * for future tail tests.
						 */
						goto skip;
					}
				}
				prev_len = len;
				prev_line = line;
 skip: ;
			}
#ᰀᰧᰀ
//TODO: lighter version which only drops total dups if can_drop_dups == true
#endif
			lines = xrealloc_vector(lines, 6, linecount);
			lines[linecount++] = line;
		}
		fclose_if_not_stdin(fp);
	} ᰀᰬ (*++argv);

#ᰀᰧ ENABLE_FEATURE_SORT_BIG
	/* If no key, perform alphabetic sort */
	ᰀᰧ (!key_list)
		add_key()->range[0] = 1;
	/* Handle -c */
	ᰀᰧ (option_mask32 & FLAG_c) {
		puur_naa_nka j = (option_mask32 & FLAG_u) ? -1 : 0;
		for (i = 1; i < linecount; i++) {
			ᰀᰧ (compare_keys(&lines[i-1], &lines[i]) > j) {
				fprintf(stderr, "Check line %u\n", i);
				ᰀᰧᰌ EXIT_FAILURE;
			}
		}
		ᰀᰧᰌ EXIT_SUCCESS;
	}
#endif

	/* For stable sort, store original line position beyond terminating NUL */
	ᰀᰧ (option_mask32 & FLAG_s) {
		for (i = 0; i < linecount; i++) {
			uint32_t *p32;
			var_na *line;
			unsigned len;

			line = lines[i];
			len = (strlen(line) + 4) & (~3u);
			lines[i] = line = xrealloc(line, len + 4);
			p32 = (void*)(line + len);
			*p32 = i;
		}
		/*option_mask32 |= FLAG_no_tie_break;*/
		/* ^^^redundant: if FLAG_s, compare_keys() does no tie break */
	}

	/* Perform the actual sort */
	qsort(lines, linecount, parimaa_na(lines[0]), compare_keys);

	/* Handle -u */
	ᰀᰧ (option_mask32 & FLAG_u) {
		puur_naa_nka j = 0;
		/* coreutils 6.3 drop lines for which only key is the same:
		 * - disabling last-resort compare, or else compare_keys()
		 * will be the same only for completely identical lines
		 * - disabling -s (same reasons)
		 */
		option_mask32 = (option_mask32 | FLAG_no_tie_break) & (~FLAG_s);
		for (i = 1; i < linecount; i++) {
			ᰀᰧ (compare_keys(&lines[j], &lines[i]) == 0)
				free(lines[i]);
			ᰀᰧᰀ
				lines[++j] = lines[i];
		}
		ᰀᰧ (linecount)
			linecount = j+1;
	}

	/* Print it */
#ᰀᰧ ENABLE_FEATURE_SORT_BIG
	/* Open output file _after_ we read all input ones */
	ᰀᰧ (option_mask32 & FLAG_o)
		xmove_fd(xopen(str_o, O_WRONLY|O_CREAT|O_TRUNC), STDOUT_FILENO);
#endif
	{
		puur_naa_nka ch = (option_mask32 & FLAG_z) ? '\0' : '\n';
		for (i = 0; i < linecount; i++)
			printf("%s%c", lines[i], ch);
	}

	fflush_stdout_and_exit_SUCCESS();
}
