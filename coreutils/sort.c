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

#અન્તર્ભાવ "libbb.h"

/* These are sort types */
ગણના {
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

સ્થિર અચલ અક્ષર sort_opt_str[] ALIGN1 = "^"
			"nghMVucszbrdfimS:T:o:k:*t:"
			"\0" "o--o:t--t"/*-t, -o: at most one of each*/;
/*
 * OPT_STR must not be string literal, needs to have stable address:
 * code uses "strchr(OPT_STR,c) - OPT_STR" idiom.
 */
#define OPT_STR (sort_opt_str + 1)

#જો ENABLE_FEATURE_SORT_BIG
સ્થિર અક્ષર ચાવી_separator;

સ્થિર સંરચના sort_ચાવી {
	સંરચના sort_ચાવી *next_ચાવી;  /* linked list */
	unsigned range[4];          /* start word, start char, end word, end char */
	unsigned flags;
} *ચાવી_list;


/* This is a NOEXEC applet. Be very careful! */


સ્થિર અક્ષર *get_ચાવી(અક્ષર *str, સંરચના sort_ચાવી *ચાવી, પૂર્ણાંક flags)
{
	પૂર્ણાંક start = start; /* for compiler */
	પૂર્ણાંક end;
	પૂર્ણાંક len, j;
	unsigned i;

	/* Special case whole string, so we don't have to make a copy */
	જો (ચાવી->range[0] == 1 && !ચાવી->range[1] && !ચાવી->range[2] && !ચાવી->range[3]
	 && !(flags & (FLAG_b | FLAG_d | FLAG_f | FLAG_i | FLAG_bb))
	) {
		પાછા str;
	}

	/* Find start of key on first pass, end on second pass */
	len = strlen(str);
	માટે (j = 0; j < 2; j++) {
		જો (!ચાવી->range[2*j])
			end = len;
		/* Loop through fields */
		નહીંતર {
			unsigned અક્ષર ch = 0;

			end = 0;
			માટે (i = 1; i < ચાવી->range[2*j] + j; i++) {
				જો (ચાવી_separator) {
					/* Skip body of key and separator */
					જ્યારે ((ch = str[end]) != '\0') {
							end++;
						જો (ch == ચાવી_separator)
							તોડો;
					}
				} નહીંતર {
					/* Skip leading blanks */
					જ્યારે (isspace(str[end]))
						end++;
					/* Skip body of key */
					જ્યારે (str[end] != '\0') {
						જો (isspace(str[end]))
							તોડો;
						end++;
					}
				}
			}
			/* Remove last delim: "abc:def:" => "abc:def" */
			જો (j && ch) {
				//if (str[end-1] != key_separator)
				//  bb_error_msg(_and_die("BUG! "
				//  "str[start:%d,end:%d]:'%.*s'",
				//  start, end, (int)(end-start), &str[start]);
				end--;
			}
		}
		જો (!j) start = end;
	}
	/* Strip leading whitespace if necessary */
	જો (flags & FLAG_b)
		/* not using skip_whitespace() for speed */
		જ્યારે (isspace(str[start])) start++;
	/* Strip trailing whitespace if necessary */
	જો (flags & FLAG_bb)
		જ્યારે (end > start && isspace(str[end-1])) end--;
	/* -kSTART,N.ENDCHAR: honor ENDCHAR (1-based) */
	જો (ચાવી->range[3]) {
		end = ચાવી->range[3];
		જો (end > len) end = len;
	}
	/* -kN.STARTCHAR[,...]: honor STARTCHAR (1-based) */
	જો (ચાવી->range[1]) {
		start += ચાવી->range[1] - 1;
		જો (start > len) start = len;
	}
	/* Make the copy */
	જો (end < start)
		end = start;
	str = xstrndup(str+start, end-start);
	/* Handle -d */
	જો (flags & FLAG_d) {
		માટે (start = end = 0; str[end]; end++)
			જો (isspace(str[end]) || isalnum(str[end]))
				str[start++] = str[end];
		str[start] = '\0';
	}
	/* Handle -i */
	જો (flags & FLAG_i) {
		માટે (start = end = 0; str[end]; end++)
			જો (isprint_asciionly(str[end]))
				str[start++] = str[end];
		str[start] = '\0';
	}
	/* Handle -f */
	જો (flags & FLAG_f)
		માટે (i = 0; str[i]; i++)
			str[i] = toupper(str[i]);

	પાછા str;
}

સ્થિર સંરચના sort_ચાવી *add_ચાવી(શૂન્ય)
{
	સંરચના sort_ચાવી **pkey = &ચાવી_list;
	જ્યારે (*pkey)
		pkey = &((*pkey)->next_ચાવી);
	પાછા *pkey = xzalloc(કદ(સંરચના sort_ચાવી));
}

#define GET_લીટી(fp) \
	((option_mask32 & FLAG_z) \
	? bb_get_chunk_from_file(fp, ખાલી) \
	: xmalloc_fgetline(fp))
#નહીંતર
#define GET_લીટી(fp) xmalloc_fgetline(fp)
#endif

#જો ENABLE_FEATURE_SORT_BIG
સ્થિર પૂર્ણાંક scale_suffix(અચલ અક્ષર *tail)
{
	સ્થિર અચલ અક્ષર suffix[] ALIGN1 = "kmgtpezy";
	અચલ અક્ષર *s;
	પૂર્ણાંક n;

	જો (!tail[0])
		પાછા -1;
	s = strchr(suffix, tail[0] | 0x20);
	જો (!s)
		પાછા -1;
	n = s - suffix;
	જો (n != 0 && tail[0] >= 'a')
		પાછા -1; /* mg... not accepted, only MG... */
	પાછા n;
}
#endif

/* Iterate through keys list and perform comparisons */
સ્થિર પૂર્ણાંક compare_ચાવીઓ(અચલ શૂન્ય *xarg, અચલ શૂન્ય *yarg)
{
	પૂર્ણાંક flags = option_mask32, retval = 0;
	અક્ષર *x, *y;

#જો ENABLE_FEATURE_SORT_BIG
	સંરચના sort_ચાવી *ચાવી;

	માટે (ચાવી = ચાવી_list; !retval && ચાવી; ચાવી = ચાવી->next_ચાવી) {
		flags = ચાવી->flags ? ચાવી->flags : option_mask32;
		/* Chop out and modify key chunks, handling -dfib */
		x = get_ચાવી(*(અક્ષર **)xarg, ચાવી, flags);
		y = get_ચાવી(*(અક્ષર **)yarg, ચાવી, flags);
#નહીંતર
	/* This curly bracket serves no purpose but to match the nesting
	 * level of the for () loop we're not using */
	{
		x = *(અક્ષર **)xarg;
		y = *(અક્ષર **)yarg;
#endif
		/* Perform actual comparison */
		પસંદગી (flags & (FLAG_n | FLAG_g | FLAG_h | FLAG_M | FLAG_V)) {
		મૂળભૂત:
			bb_simple_error_સંદેશ_and_die("unknown sort type");
			તોડો;
#જો defined(HAVE_STRVERSCMP) && HAVE_STRVERSCMP == 1
		સ્થિતિ FLAG_V:
			retval = strverscmp(x, y);
			તોડો;
#endif
		/* Ascii sort */
		સ્થિતિ 0:
#જો ENABLE_LOCALE_SUPPORT
			retval = strcoll(x, y);
#નહીંતર
			retval = strcmp(x, y);
#endif
			તોડો;
#જો ENABLE_FEATURE_SORT_BIG
		સ્થિતિ FLAG_g:
		સ્થિતિ FLAG_h: {
			અક્ષર *xx, *yy;
//TODO: needs setlocale(LC_NUMERIC, "C")?
			દ્વિગુણ dx = strtod(x, &xx);
			દ્વિગુણ dy = strtod(y, &yy);
			/* not numbers < NaN < -infinity < numbers < +infinity) */
			જો (x == xx)
				retval = (y == yy ? 0 : -1);
			નહીંતર જો (y == yy)
				retval = 1;
			/* Check for isnan */
			નહીંતર જો (dx != dx)
				retval = (dy != dy) ? 0 : -1;
			નહીંતર જો (dy != dy)
				retval = 1;
			નહીંતર {
				જો (flags & FLAG_h) {
					પૂર્ણાંક xs = scale_suffix(xx);
					પૂર્ણાંક ys = scale_suffix(yy);
					જો (xs != ys) {
						retval = xs - ys;
						તોડો;
					}
				}
				/* Check for infinity.  Could underflow, but it avoids libm. */
				જો (1.0 / dx == 0.0) {
					જો (dx < 0)
						retval = (1.0 / dy == 0.0 && dy < 0) ? 0 : -1;
					નહીંતર
						retval = (1.0 / dy == 0.0 && dy > 0) ? 0 : 1;
				} નહીંતર જો (1.0 / dy == 0.0)
					retval = (dy < 0) ? 1 : -1;
				નહીંતર
					retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			}
			તોડો;
		}
		સ્થિતિ FLAG_M: {
			સંરચના tm thyme;
			પૂર્ણાંક dx;
			અક્ષર *xx, *yy;

			xx = strptime(skip_whitespace(x), "%b", &thyme);
			dx = thyme.tm_mon;
			yy = strptime(skip_whitespace(y), "%b", &thyme);
			જો (!xx)
				retval = (!yy) ? 0 : -1;
			નહીંતર જો (!yy)
				retval = 1;
			નહીંતર
				retval = dx - thyme.tm_mon;
			તોડો;
		}
		/* Full floating point version of -n */
		સ્થિતિ FLAG_n: {
			દ્વિગુણ dx = atof(x);
			દ્વિગુણ dy = atof(y);
			retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			તોડો;
		}
		} /* switch */
		/* Free key copies. */
		જો (x != *(અક્ષર **)xarg) મુક્ત(x);
		જો (y != *(અક્ષર **)yarg) મુક્ત(y);
		/* if (retval) break; - done by for () anyway */
#નહીંતર
		/* Integer version of -n for tiny systems */
		સ્થિતિ FLAG_n:
			retval = atoi(x) - atoi(y);
			તોડો;
		} /* switch */
#endif
	} /* for */

	જો (retval == 0) {
		/* So far lines are "the same" */

		જો (option_mask32 & FLAG_s) {
			/* "Stable sort": later line is "greater than",
			 * IOW: do not allow qsort() to swap equal lines.
			 */
			uint32_t *p32;
			uint32_t x32, y32;
			અક્ષર *લીટી;
			unsigned len;

			લીટી = *(અક્ષર**)xarg;
			len = (strlen(લીટી) + 4) & (~3u);
			p32 = (શૂન્ય*)(લીટી + len);
			x32 = *p32;
			લીટી = *(અક્ષર**)yarg;
			len = (strlen(લીટી) + 4) & (~3u);
			p32 = (શૂન્ય*)(લીટી + len);
			y32 = *p32;

			/* If x > y, 1, else -1 */
			retval = (x32 > y32) * 2 - 1;
			/* Here, -r has no effect! */
			પાછા retval;
		}
		જો (!(option_mask32 & FLAG_no_tie_break)) {
			/* fallback sort */
			flags = option_mask32;
			retval = strcmp(*(અક્ષર **)xarg, *(અક્ષર **)yarg);
		}
	}

	જો (flags & FLAG_r)
		પાછા -retval;

	પાછા retval;
}

#જો ENABLE_FEATURE_SORT_BIG
સ્થિર unsigned str2u(અક્ષર **str)
{
	unsigned long lu;
	જો (!isdigit((*str)[0]))
		bb_simple_error_સંદેશ_and_die("bad field specification");
	lu = strtoul(*str, str, 10);
	જો ((કદ(long) > કદ(પૂર્ણાંક) && lu > INT_MAX) || !lu)
		bb_simple_error_સંદેશ_and_die("bad field specification");
	પાછા lu;
}
#endif

પૂર્ણાંક sort_main(પૂર્ણાંક argc, અક્ષર **argv) MAIN_EXTERNALLY_VISIBLE;
પૂર્ણાંક sort_main(પૂર્ણાંક argc UNUSED_PARAM, અક્ષર **argv)
{
	અક્ષર **લીટીઓ;
	અક્ષર *str_ignored, *str_o, *str_t;
	llist_t *lst_k = ખાલી;
	પૂર્ણાંક i;
	પૂર્ણાંક linecount;
	unsigned opts;
#જો ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
	bool can_drop_dups;
	size_t prev_len = 0;
	અક્ષર *prev_લીટી = (અક્ષર*) "";
	/* Postpone optimizing if the input is small, < 16k lines:
	 * even just free()ing duplicate lines takes time.
	 */
	size_t ગણતરી_to_optimize_dups = 0x3fff;
#endif

	xfunc_error_retval = 2;

	/* Parse command line options */
	opts = getopt32(argv,
			sort_opt_str,
			&str_ignored, &str_ignored, &str_o, &lst_k, &str_t
	);
#જો ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
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
	જો (opts & FLAG_s)
		ગણતરી_to_optimize_dups = (size_t)-1L;
#endif
	/* global b strips leading and trailing spaces */
	જો (opts & FLAG_b)
		option_mask32 |= FLAG_bb;
#જો ENABLE_FEATURE_SORT_BIG
	જો (opts & FLAG_t) {
		જો (!str_t[0] || str_t[1])
			bb_simple_error_સંદેશ_and_die("bad -t parameter");
		ચાવી_separator = str_t[0];
	}
	/* note: below this point we use option_mask32, not opts,
	 * since that reduces register pressure and makes code smaller */

	/* Parse sort key */
	જ્યારે (lst_k) {
		ગણના {
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
		સંરચના sort_ચાવી *ચાવી = add_ચાવી();
		અક્ષર *str_k = llist_કાઢો(&lst_k);

		i = 0; /* i==0 before comma, 1 after (-k3,6) */
		જ્યારે (*str_k) {
			/* Start of range */
			/* Cannot use bb_strtou - suffix can be a letter */
			ચાવી->range[2*i] = str2u(&str_k);
			જો (*str_k == '.') {
				str_k++;
				ચાવી->range[2*i+1] = str2u(&str_k);
			}
			જ્યારે (*str_k) {
				પૂર્ણાંક flag;
				અચલ અક્ષર *idx;

				જો (*str_k == ',' && !i++) {
					str_k++;
					તોડો;
				} /* no else needed: fall through to syntax error
					because comma isn't in OPT_STR */
				idx = strchr(OPT_STR, *str_k);
				જો (!idx)
					bb_simple_error_સંદેશ_and_die("unknown key option");
				flag = 1 << (idx - OPT_STR);
				જો (flag & ~FLAG_allowed_for_k)
					bb_simple_error_સંદેશ_and_die("unknown sort type");
				/* b after ',' means strip _trailing_ space */
				જો (i && flag == FLAG_b)
					flag = FLAG_bb;
				ચાવી->flags |= flag;
				str_k++;
			}
		}
	}
#endif

	/* Open input files and read data */
	argv += optind;
	જો (!*argv)
		*--argv = (અક્ષર*)"-";
	linecount = 0;
	લીટીઓ = ખાલી;
	કરો {
		/* coreutils 6.9 compat: abort on first open error,
		 * do not continue to next file: */
		FILE *fp = xfopen_stdin(*argv);
		માટે (;;) {
			અક્ષર *લીટી = GET_લીટી(fp);
			જો (!લીટી)
				તોડો;

#જો ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
			જો (ગણતરી_to_optimize_dups != 0)
				ગણતરી_to_optimize_dups--;
			જો (ગણતરી_to_optimize_dups == 0) {
				size_t len;
				અક્ષર *new_લીટી;

				/* On kernel/linux/arch/ *.[ch] files,
				 * this reduces memory usage by 6%.
				 *  yes | head -99999999 | sort
				 * goes down from 1900Mb to 380 Mb.
				 */
				len = strlen(લીટી);
				જો (len <= prev_len) {
					new_લીટી = prev_લીટી + (prev_len - len);
					જો (strcmp(લીટી, new_લીટી) == 0) {
						/* it's a tail of the prev line */
						જો (can_drop_dups && prev_len == len) {
							/* it's identical to prev line */
							મુક્ત(લીટી);
							ચાલુ;
						}
						મુક્ત(લીટી);
						લીટી = new_લીટી;
						/* continue using longer prev_line
						 * for future tail tests.
						 */
						જાઓ skip;
					}
				}
				prev_len = len;
				prev_લીટી = લીટી;
 skip: ;
			}
#નહીંતર
//TODO: lighter version which only drops total dups if can_drop_dups == true
#endif
			લીટીઓ = xrealloc_vector(લીટીઓ, 6, linecount);
			લીટીઓ[linecount++] = લીટી;
		}
		fclose_if_not_stdin(fp);
	} જ્યારે (*++argv);

#જો ENABLE_FEATURE_SORT_BIG
	/* If no key, perform alphabetic sort */
	જો (!ચાવી_list)
		add_ચાવી()->range[0] = 1;
	/* Handle -c */
	જો (option_mask32 & FLAG_c) {
		પૂર્ણાંક j = (option_mask32 & FLAG_u) ? -1 : 0;
		માટે (i = 1; i < linecount; i++) {
			જો (compare_ચાવીઓ(&લીટીઓ[i-1], &લીટીઓ[i]) > j) {
				fprintf(stderr, "Check line %u\n", i);
				પાછા EXIT_FAILURE;
			}
		}
		પાછા EXIT_SUCCESS;
	}
#endif

	/* For stable sort, store original line position beyond terminating NUL */
	જો (option_mask32 & FLAG_s) {
		માટે (i = 0; i < linecount; i++) {
			uint32_t *p32;
			અક્ષર *લીટી;
			unsigned len;

			લીટી = લીટીઓ[i];
			len = (strlen(લીટી) + 4) & (~3u);
			લીટીઓ[i] = લીટી = xrealloc(લીટી, len + 4);
			p32 = (શૂન્ય*)(લીટી + len);
			*p32 = i;
		}
		/*option_mask32 |= FLAG_no_tie_break;*/
		/* ^^^redundant: if FLAG_s, compare_keys() does no tie break */
	}

	/* Perform the actual sort */
	qsort(લીટીઓ, linecount, કદ(લીટીઓ[0]), compare_ચાવીઓ);

	/* Handle -u */
	જો (option_mask32 & FLAG_u) {
		પૂર્ણાંક j = 0;
		/* coreutils 6.3 drop lines for which only key is the same:
		 * - disabling last-resort compare, or else compare_keys()
		 * will be the same only for completely identical lines
		 * - disabling -s (same reasons)
		 */
		option_mask32 = (option_mask32 | FLAG_no_tie_break) & (~FLAG_s);
		માટે (i = 1; i < linecount; i++) {
			જો (compare_ચાવીઓ(&લીટીઓ[j], &લીટીઓ[i]) == 0)
				મુક્ત(લીટીઓ[i]);
			નહીંતર
				લીટીઓ[++j] = લીટીઓ[i];
		}
		જો (linecount)
			linecount = j+1;
	}

	/* Print it */
#જો ENABLE_FEATURE_SORT_BIG
	/* Open output file _after_ we read all input ones */
	જો (option_mask32 & FLAG_o)
		xmove_fd(xopen(str_o, O_WRONLY|O_CREAT|O_TRUNC), STDOUT_FILENO);
#endif
	{
		પૂર્ણાંક ch = (option_mask32 & FLAG_z) ? '\0' : '\n';
		માટે (i = 0; i < linecount; i++)
			printf("%s%c", લીટીઓ[i], ch);
	}

	fflush_stdout_and_exit_SUCCESS();
}
