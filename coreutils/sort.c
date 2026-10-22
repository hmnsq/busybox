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

#ضمن "libbb.h"

/* These are sort types */
تعداد {
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

ساكن ثابت حرف sort_opt_str[] ALIGN1 = "^"
			"nghMVucszbrdfimS:T:o:k:*t:"
			"\0" "o--o:t--t"/*-t, -o: at most one of each*/;
/*
 * OPT_STR must not be string literal, needs to have stable address:
 * code uses "strchr(OPT_STR,c) - OPT_STR" idiom.
 */
#define OPT_STR (sort_opt_str + 1)

#𐡄𐡍 ENABLE_FEATURE_SORT_BIG
ساكن حرف key_separator;

ساكن بنية sort_key {
	بنية sort_key *next_key;  /* linked list */
	unsigned range[4];          /* start word, start char, end word, end char */
	unsigned flags;
} *key_list;


/* This is a NOEXEC applet. Be very careful! */


ساكن حرف *get_key(حرف *str, بنية sort_key *key, صحيح flags)
{
	صحيح start = start; /* for compiler */
	صحيح end;
	صحيح len, j;
	unsigned i;

	/* Special case whole string, so we don't have to make a copy */
	𐡄𐡍 (key->range[0] == 1 && !key->range[1] && !key->range[2] && !key->range[3]
	 && !(flags & (FLAG_b | FLAG_d | FLAG_f | FLAG_i | FLAG_bb))
	) {
		أرجع str;
	}

	/* Find start of key on first pass, end on second pass */
	len = strlen(str);
	لكل (j = 0; j < 2; j++) {
		𐡄𐡍 (!key->range[2*j])
			end = len;
		/* Loop through fields */
		وإلا {
			unsigned حرف ch = 0;

			end = 0;
			لكل (i = 1; i < key->range[2*j] + j; i++) {
				𐡄𐡍 (key_separator) {
					/* Skip body of key and separator */
					طالما ((ch = str[end]) != '\0') {
							end++;
						𐡄𐡍 (ch == key_separator)
							اكسر;
					}
				} وإلا {
					/* Skip leading blanks */
					طالما (isspace(str[end]))
						end++;
					/* Skip body of key */
					طالما (str[end] != '\0') {
						𐡄𐡍 (isspace(str[end]))
							اكسر;
						end++;
					}
				}
			}
			/* Remove last delim: "abc:def:" => "abc:def" */
			𐡄𐡍 (j && ch) {
				//if (str[end-1] != key_separator)
				//  bb_error_msg(_and_die("BUG! "
				//  "str[start:%d,end:%d]:'%.*s'",
				//  start, end, (int)(end-start), &str[start]);
				end--;
			}
		}
		𐡄𐡍 (!j) start = end;
	}
	/* Strip leading whitespace if necessary */
	𐡄𐡍 (flags & FLAG_b)
		/* not using skip_whitespace() for speed */
		طالما (isspace(str[start])) start++;
	/* Strip trailing whitespace if necessary */
	𐡄𐡍 (flags & FLAG_bb)
		طالما (end > start && isspace(str[end-1])) end--;
	/* -kSTART,N.ENDCHAR: honor ENDCHAR (1-based) */
	𐡄𐡍 (key->range[3]) {
		end = key->range[3];
		𐡄𐡍 (end > len) end = len;
	}
	/* -kN.STARTCHAR[,...]: honor STARTCHAR (1-based) */
	𐡄𐡍 (key->range[1]) {
		start += key->range[1] - 1;
		𐡄𐡍 (start > len) start = len;
	}
	/* Make the copy */
	𐡄𐡍 (end < start)
		end = start;
	str = xstrndup(str+start, end-start);
	/* Handle -d */
	𐡄𐡍 (flags & FLAG_d) {
		لكل (start = end = 0; str[end]; end++)
			𐡄𐡍 (isspace(str[end]) || isalnum(str[end]))
				str[start++] = str[end];
		str[start] = '\0';
	}
	/* Handle -i */
	𐡄𐡍 (flags & FLAG_i) {
		لكل (start = end = 0; str[end]; end++)
			𐡄𐡍 (isprint_asciionly(str[end]))
				str[start++] = str[end];
		str[start] = '\0';
	}
	/* Handle -f */
	𐡄𐡍 (flags & FLAG_f)
		لكل (i = 0; str[i]; i++)
			str[i] = toupper(str[i]);

	أرجع str;
}

ساكن بنية sort_key *add_key(فراغ)
{
	بنية sort_key **pkey = &key_list;
	طالما (*pkey)
		pkey = &((*pkey)->next_key);
	أرجع *pkey = xzalloc(حجم(بنية sort_key));
}

#define GET_LINE(fp) \
	((option_mask32 & FLAG_z) \
	? bb_get_chunk_from_file(fp, عدم) \
	: xmalloc_fgetline(fp))
#وإلا
#define GET_LINE(fp) xmalloc_fgetline(fp)
#endif

#𐡄𐡍 ENABLE_FEATURE_SORT_BIG
ساكن صحيح scale_suffix(ثابت حرف *tail)
{
	ساكن ثابت حرف suffix[] ALIGN1 = "kmgtpezy";
	ثابت حرف *s;
	صحيح n;

	𐡄𐡍 (!tail[0])
		أرجع -1;
	s = strchr(suffix, tail[0] | 0x20);
	𐡄𐡍 (!s)
		أرجع -1;
	n = s - suffix;
	𐡄𐡍 (n != 0 && tail[0] >= 'a')
		أرجع -1; /* mg... not accepted, only MG... */
	أرجع n;
}
#endif

/* Iterate through keys list and perform comparisons */
ساكن صحيح compare_keys(ثابت فراغ *xarg, ثابت فراغ *yarg)
{
	صحيح flags = option_mask32, retval = 0;
	حرف *x, *y;

#𐡄𐡍 ENABLE_FEATURE_SORT_BIG
	بنية sort_key *key;

	لكل (key = key_list; !retval && key; key = key->next_key) {
		flags = key->flags ? key->flags : option_mask32;
		/* Chop out and modify key chunks, handling -dfib */
		x = get_key(*(حرف **)xarg, key, flags);
		y = get_key(*(حرف **)yarg, key, flags);
#وإلا
	/* This curly bracket serves no purpose but to match the nesting
	 * level of the for () loop we're not using */
	{
		x = *(حرف **)xarg;
		y = *(حرف **)yarg;
#endif
		/* Perform actual comparison */
		بدل (flags & (FLAG_n | FLAG_g | FLAG_h | FLAG_M | FLAG_V)) {
		افتراضي:
			bb_simple_error_msg_and_die("unknown sort type");
			اكسر;
#𐡄𐡍 defined(HAVE_STRVERSCMP) && HAVE_STRVERSCMP == 1
		حالة FLAG_V:
			retval = strverscmp(x, y);
			اكسر;
#endif
		/* Ascii sort */
		حالة 0:
#𐡄𐡍 ENABLE_LOCALE_SUPPORT
			retval = strcoll(x, y);
#وإلا
			retval = strcmp(x, y);
#endif
			اكسر;
#𐡄𐡍 ENABLE_FEATURE_SORT_BIG
		حالة FLAG_g:
		حالة FLAG_h: {
			حرف *xx, *yy;
//TODO: needs setlocale(LC_NUMERIC, "C")?
			مزدوج dx = strtod(x, &xx);
			مزدوج dy = strtod(y, &yy);
			/* not numbers < NaN < -infinity < numbers < +infinity) */
			𐡄𐡍 (x == xx)
				retval = (y == yy ? 0 : -1);
			وإلا 𐡄𐡍 (y == yy)
				retval = 1;
			/* Check for isnan */
			وإلا 𐡄𐡍 (dx != dx)
				retval = (dy != dy) ? 0 : -1;
			وإلا 𐡄𐡍 (dy != dy)
				retval = 1;
			وإلا {
				𐡄𐡍 (flags & FLAG_h) {
					صحيح xs = scale_suffix(xx);
					صحيح ys = scale_suffix(yy);
					𐡄𐡍 (xs != ys) {
						retval = xs - ys;
						اكسر;
					}
				}
				/* Check for infinity.  Could underflow, but it avoids libm. */
				𐡄𐡍 (1.0 / dx == 0.0) {
					𐡄𐡍 (dx < 0)
						retval = (1.0 / dy == 0.0 && dy < 0) ? 0 : -1;
					وإلا
						retval = (1.0 / dy == 0.0 && dy > 0) ? 0 : 1;
				} وإلا 𐡄𐡍 (1.0 / dy == 0.0)
					retval = (dy < 0) ? 1 : -1;
				وإلا
					retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			}
			اكسر;
		}
		حالة FLAG_M: {
			بنية tm thyme;
			صحيح dx;
			حرف *xx, *yy;

			xx = strptime(skip_whitespace(x), "%b", &thyme);
			dx = thyme.tm_mon;
			yy = strptime(skip_whitespace(y), "%b", &thyme);
			𐡄𐡍 (!xx)
				retval = (!yy) ? 0 : -1;
			وإلا 𐡄𐡍 (!yy)
				retval = 1;
			وإلا
				retval = dx - thyme.tm_mon;
			اكسر;
		}
		/* Full floating point version of -n */
		حالة FLAG_n: {
			مزدوج dx = atof(x);
			مزدوج dy = atof(y);
			retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			اكسر;
		}
		} /* switch */
		/* Free key copies. */
		𐡄𐡍 (x != *(حرف **)xarg) حرر(x);
		𐡄𐡍 (y != *(حرف **)yarg) حرر(y);
		/* if (retval) break; - done by for () anyway */
#وإلا
		/* Integer version of -n for tiny systems */
		حالة FLAG_n:
			retval = atoi(x) - atoi(y);
			اكسر;
		} /* switch */
#endif
	} /* for */

	𐡄𐡍 (retval == 0) {
		/* So far lines are "the same" */

		𐡄𐡍 (option_mask32 & FLAG_s) {
			/* "Stable sort": later line is "greater than",
			 * IOW: do not allow qsort() to swap equal lines.
			 */
			uint32_t *p32;
			uint32_t x32, y32;
			حرف *line;
			unsigned len;

			line = *(حرف**)xarg;
			len = (strlen(line) + 4) & (~3u);
			p32 = (فراغ*)(line + len);
			x32 = *p32;
			line = *(حرف**)yarg;
			len = (strlen(line) + 4) & (~3u);
			p32 = (فراغ*)(line + len);
			y32 = *p32;

			/* If x > y, 1, else -1 */
			retval = (x32 > y32) * 2 - 1;
			/* Here, -r has no effect! */
			أرجع retval;
		}
		𐡄𐡍 (!(option_mask32 & FLAG_no_tie_break)) {
			/* fallback sort */
			flags = option_mask32;
			retval = strcmp(*(حرف **)xarg, *(حرف **)yarg);
		}
	}

	𐡄𐡍 (flags & FLAG_r)
		أرجع -retval;

	أرجع retval;
}

#𐡄𐡍 ENABLE_FEATURE_SORT_BIG
ساكن unsigned str2u(حرف **str)
{
	unsigned long lu;
	𐡄𐡍 (!isdigit((*str)[0]))
		bb_simple_error_msg_and_die("bad field specification");
	lu = strtoul(*str, str, 10);
	𐡄𐡍 ((حجم(long) > حجم(صحيح) && lu > INT_MAX) || !lu)
		bb_simple_error_msg_and_die("bad field specification");
	أرجع lu;
}
#endif

صحيح sort_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح sort_main(صحيح argc UNUSED_PARAM, حرف **argv)
{
	حرف **lines;
	حرف *str_ignored, *str_o, *str_t;
	llist_t *lst_k = عدم;
	صحيح i;
	صحيح linecount;
	unsigned opts;
#𐡄𐡍 ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
	bool can_drop_dups;
	size_t prev_len = 0;
	حرف *prev_line = (حرف*) "";
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
#𐡄𐡍 ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
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
	𐡄𐡍 (opts & FLAG_s)
		count_to_optimize_dups = (size_t)-1L;
#endif
	/* global b strips leading and trailing spaces */
	𐡄𐡍 (opts & FLAG_b)
		option_mask32 |= FLAG_bb;
#𐡄𐡍 ENABLE_FEATURE_SORT_BIG
	𐡄𐡍 (opts & FLAG_t) {
		𐡄𐡍 (!str_t[0] || str_t[1])
			bb_simple_error_msg_and_die("bad -t parameter");
		key_separator = str_t[0];
	}
	/* note: below this point we use option_mask32, not opts,
	 * since that reduces register pressure and makes code smaller */

	/* Parse sort key */
	طالما (lst_k) {
		تعداد {
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
		بنية sort_key *key = add_key();
		حرف *str_k = llist_pop(&lst_k);

		i = 0; /* i==0 before comma, 1 after (-k3,6) */
		طالما (*str_k) {
			/* Start of range */
			/* Cannot use bb_strtou - suffix can be a letter */
			key->range[2*i] = str2u(&str_k);
			𐡄𐡍 (*str_k == '.') {
				str_k++;
				key->range[2*i+1] = str2u(&str_k);
			}
			طالما (*str_k) {
				صحيح flag;
				ثابت حرف *idx;

				𐡄𐡍 (*str_k == ',' && !i++) {
					str_k++;
					اكسر;
				} /* no else needed: fall through to syntax error
					because comma isn't in OPT_STR */
				idx = strchr(OPT_STR, *str_k);
				𐡄𐡍 (!idx)
					bb_simple_error_msg_and_die("unknown key option");
				flag = 1 << (idx - OPT_STR);
				𐡄𐡍 (flag & ~FLAG_allowed_for_k)
					bb_simple_error_msg_and_die("unknown sort type");
				/* b after ',' means strip _trailing_ space */
				𐡄𐡍 (i && flag == FLAG_b)
					flag = FLAG_bb;
				key->flags |= flag;
				str_k++;
			}
		}
	}
#endif

	/* Open input files and read data */
	argv += optind;
	𐡄𐡍 (!*argv)
		*--argv = (حرف*)"-";
	linecount = 0;
	lines = عدم;
	𐡏𐡁𐡃 {
		/* coreutils 6.9 compat: abort on first open error,
		 * do not continue to next file: */
		FILE *fp = xfopen_stdin(*argv);
		لكل (;;) {
			حرف *line = GET_LINE(fp);
			𐡄𐡍 (!line)
				اكسر;

#𐡄𐡍 ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
			𐡄𐡍 (count_to_optimize_dups != 0)
				count_to_optimize_dups--;
			𐡄𐡍 (count_to_optimize_dups == 0) {
				size_t len;
				حرف *new_line;

				/* On kernel/linux/arch/ *.[ch] files,
				 * this reduces memory usage by 6%.
				 *  yes | head -99999999 | sort
				 * goes down from 1900Mb to 380 Mb.
				 */
				len = strlen(line);
				𐡄𐡍 (len <= prev_len) {
					new_line = prev_line + (prev_len - len);
					𐡄𐡍 (strcmp(line, new_line) == 0) {
						/* it's a tail of the prev line */
						𐡄𐡍 (can_drop_dups && prev_len == len) {
							/* it's identical to prev line */
							حرر(line);
							استمر;
						}
						حرر(line);
						line = new_line;
						/* continue using longer prev_line
						 * for future tail tests.
						 */
						اذهب skip;
					}
				}
				prev_len = len;
				prev_line = line;
 skip: ;
			}
#وإلا
//TODO: lighter version which only drops total dups if can_drop_dups == true
#endif
			lines = xrealloc_vector(lines, 6, linecount);
			lines[linecount++] = line;
		}
		fclose_if_not_stdin(fp);
	} طالما (*++argv);

#𐡄𐡍 ENABLE_FEATURE_SORT_BIG
	/* If no key, perform alphabetic sort */
	𐡄𐡍 (!key_list)
		add_key()->range[0] = 1;
	/* Handle -c */
	𐡄𐡍 (option_mask32 & FLAG_c) {
		صحيح j = (option_mask32 & FLAG_u) ? -1 : 0;
		لكل (i = 1; i < linecount; i++) {
			𐡄𐡍 (compare_keys(&lines[i-1], &lines[i]) > j) {
				fprintf(stderr, "Check line %u\n", i);
				أرجع EXIT_FAILURE;
			}
		}
		أرجع EXIT_SUCCESS;
	}
#endif

	/* For stable sort, store original line position beyond terminating NUL */
	𐡄𐡍 (option_mask32 & FLAG_s) {
		لكل (i = 0; i < linecount; i++) {
			uint32_t *p32;
			حرف *line;
			unsigned len;

			line = lines[i];
			len = (strlen(line) + 4) & (~3u);
			lines[i] = line = xrealloc(line, len + 4);
			p32 = (فراغ*)(line + len);
			*p32 = i;
		}
		/*option_mask32 |= FLAG_no_tie_break;*/
		/* ^^^redundant: if FLAG_s, compare_keys() does no tie break */
	}

	/* Perform the actual sort */
	qsort(lines, linecount, حجم(lines[0]), compare_keys);

	/* Handle -u */
	𐡄𐡍 (option_mask32 & FLAG_u) {
		صحيح j = 0;
		/* coreutils 6.3 drop lines for which only key is the same:
		 * - disabling last-resort compare, or else compare_keys()
		 * will be the same only for completely identical lines
		 * - disabling -s (same reasons)
		 */
		option_mask32 = (option_mask32 | FLAG_no_tie_break) & (~FLAG_s);
		لكل (i = 1; i < linecount; i++) {
			𐡄𐡍 (compare_keys(&lines[j], &lines[i]) == 0)
				حرر(lines[i]);
			وإلا
				lines[++j] = lines[i];
		}
		𐡄𐡍 (linecount)
			linecount = j+1;
	}

	/* Print it */
#𐡄𐡍 ENABLE_FEATURE_SORT_BIG
	/* Open output file _after_ we read all input ones */
	𐡄𐡍 (option_mask32 & FLAG_o)
		xmove_fd(xopen(str_o, O_WRONLY|O_CREAT|O_TRUNC), STDOUT_FILENO);
#endif
	{
		صحيح ch = (option_mask32 & FLAG_z) ? '\0' : '\n';
		لكل (i = 0; i < linecount; i++)
			printf("%s%c", lines[i], ch);
	}

	fflush_stdout_and_exit_SUCCESS();
}
