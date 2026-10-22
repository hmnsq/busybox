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

#شامل "libbb.h"

/* These are sort types */
شمارش {
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

ایستا ثابت نویسه sort_opt_str[] ALIGN1 = "^"
			"nghMVucszbrdfimS:T:o:k:*t:"
			"\0" "o--o:t--t"/*-t, -o: at most one of each*/;
/*
 * OPT_STR must not be string literal, needs to have stable address:
 * code uses "strchr(OPT_STR,c) - OPT_STR" idiom.
 */
#define OPT_STR (sort_opt_str + 1)

#اگر ENABLE_FEATURE_SORT_BIG
ایستا نویسه کلید_separator;

ایستا ساختار sort_کلید {
	ساختار sort_کلید *next_کلید;  /* linked list */
	unsigned range[4];          /* start word, start char, end word, end char */
	unsigned flags;
} *کلید_list;


/* This is a NOEXEC applet. Be very careful! */


ایستا نویسه *get_کلید(نویسه *str, ساختار sort_کلید *کلید, صحیح flags)
{
	صحیح start = start; /* for compiler */
	صحیح end;
	صحیح len, j;
	unsigned i;

	/* Special case whole string, so we don't have to make a copy */
	اگر (کلید->range[0] == 1 && !کلید->range[1] && !کلید->range[2] && !کلید->range[3]
	 && !(flags & (FLAG_b | FLAG_d | FLAG_f | FLAG_i | FLAG_bb))
	) {
		بازگشت str;
	}

	/* Find start of key on first pass, end on second pass */
	len = strlen(str);
	برای (j = 0; j < 2; j++) {
		اگر (!کلید->range[2*j])
			end = len;
		/* Loop through fields */
		وگرنه {
			unsigned نویسه ch = 0;

			end = 0;
			برای (i = 1; i < کلید->range[2*j] + j; i++) {
				اگر (کلید_separator) {
					/* Skip body of key and separator */
					وقتی ((ch = str[end]) != '\0') {
							end++;
						اگر (ch == کلید_separator)
							بشکن;
					}
				} وگرنه {
					/* Skip leading blanks */
					وقتی (isspace(str[end]))
						end++;
					/* Skip body of key */
					وقتی (str[end] != '\0') {
						اگر (isspace(str[end]))
							بشکن;
						end++;
					}
				}
			}
			/* Remove last delim: "abc:def:" => "abc:def" */
			اگر (j && ch) {
				//if (str[end-1] != key_separator)
				//  bb_error_msg(_and_die("BUG! "
				//  "str[start:%d,end:%d]:'%.*s'",
				//  start, end, (int)(end-start), &str[start]);
				end--;
			}
		}
		اگر (!j) start = end;
	}
	/* Strip leading whitespace if necessary */
	اگر (flags & FLAG_b)
		/* not using skip_whitespace() for speed */
		وقتی (isspace(str[start])) start++;
	/* Strip trailing whitespace if necessary */
	اگر (flags & FLAG_bb)
		وقتی (end > start && isspace(str[end-1])) end--;
	/* -kSTART,N.ENDCHAR: honor ENDCHAR (1-based) */
	اگر (کلید->range[3]) {
		end = کلید->range[3];
		اگر (end > len) end = len;
	}
	/* -kN.STARTCHAR[,...]: honor STARTCHAR (1-based) */
	اگر (کلید->range[1]) {
		start += کلید->range[1] - 1;
		اگر (start > len) start = len;
	}
	/* Make the copy */
	اگر (end < start)
		end = start;
	str = xstrndup(str+start, end-start);
	/* Handle -d */
	اگر (flags & FLAG_d) {
		برای (start = end = 0; str[end]; end++)
			اگر (isspace(str[end]) || isalnum(str[end]))
				str[start++] = str[end];
		str[start] = '\0';
	}
	/* Handle -i */
	اگر (flags & FLAG_i) {
		برای (start = end = 0; str[end]; end++)
			اگر (isprint_asciionly(str[end]))
				str[start++] = str[end];
		str[start] = '\0';
	}
	/* Handle -f */
	اگر (flags & FLAG_f)
		برای (i = 0; str[i]; i++)
			str[i] = toupper(str[i]);

	بازگشت str;
}

ایستا ساختار sort_کلید *add_کلید(تهی)
{
	ساختار sort_کلید **pkey = &کلید_list;
	وقتی (*pkey)
		pkey = &((*pkey)->next_کلید);
	بازگشت *pkey = xzalloc(اندازه(ساختار sort_کلید));
}

#define GET_سطر(fp) \
	((option_mask32 & FLAG_z) \
	? bb_get_chunk_from_file(fp, پوچ) \
	: xmalloc_fgetline(fp))
#وگرنه
#define GET_سطر(fp) xmalloc_fgetline(fp)
#endif

#اگر ENABLE_FEATURE_SORT_BIG
ایستا صحیح scale_suffix(ثابت نویسه *tail)
{
	ایستا ثابت نویسه suffix[] ALIGN1 = "kmgtpezy";
	ثابت نویسه *s;
	صحیح n;

	اگر (!tail[0])
		بازگشت -1;
	s = strchr(suffix, tail[0] | 0x20);
	اگر (!s)
		بازگشت -1;
	n = s - suffix;
	اگر (n != 0 && tail[0] >= 'a')
		بازگشت -1; /* mg... not accepted, only MG... */
	بازگشت n;
}
#endif

/* Iterate through keys list and perform comparisons */
ایستا صحیح compare_کلیدها(ثابت تهی *xarg, ثابت تهی *yarg)
{
	صحیح flags = option_mask32, retval = 0;
	نویسه *x, *y;

#اگر ENABLE_FEATURE_SORT_BIG
	ساختار sort_کلید *کلید;

	برای (کلید = کلید_list; !retval && کلید; کلید = کلید->next_کلید) {
		flags = کلید->flags ? کلید->flags : option_mask32;
		/* Chop out and modify key chunks, handling -dfib */
		x = get_کلید(*(نویسه **)xarg, کلید, flags);
		y = get_کلید(*(نویسه **)yarg, کلید, flags);
#وگرنه
	/* This curly bracket serves no purpose but to match the nesting
	 * level of the for () loop we're not using */
	{
		x = *(نویسه **)xarg;
		y = *(نویسه **)yarg;
#endif
		/* Perform actual comparison */
		انتخاب (flags & (FLAG_n | FLAG_g | FLAG_h | FLAG_M | FLAG_V)) {
		پیشفرض:
			bb_simple_error_پیام_and_die("unknown sort type");
			بشکن;
#اگر defined(HAVE_STRVERSCMP) && HAVE_STRVERSCMP == 1
		حالت FLAG_V:
			retval = strverscmp(x, y);
			بشکن;
#endif
		/* Ascii sort */
		حالت 0:
#اگر ENABLE_LOCALE_SUPPORT
			retval = strcoll(x, y);
#وگرنه
			retval = strcmp(x, y);
#endif
			بشکن;
#اگر ENABLE_FEATURE_SORT_BIG
		حالت FLAG_g:
		حالت FLAG_h: {
			نویسه *xx, *yy;
//TODO: needs setlocale(LC_NUMERIC, "C")?
			دوگانه dx = strtod(x, &xx);
			دوگانه dy = strtod(y, &yy);
			/* not numbers < NaN < -infinity < numbers < +infinity) */
			اگر (x == xx)
				retval = (y == yy ? 0 : -1);
			وگرنه اگر (y == yy)
				retval = 1;
			/* Check for isnan */
			وگرنه اگر (dx != dx)
				retval = (dy != dy) ? 0 : -1;
			وگرنه اگر (dy != dy)
				retval = 1;
			وگرنه {
				اگر (flags & FLAG_h) {
					صحیح xs = scale_suffix(xx);
					صحیح ys = scale_suffix(yy);
					اگر (xs != ys) {
						retval = xs - ys;
						بشکن;
					}
				}
				/* Check for infinity.  Could underflow, but it avoids libm. */
				اگر (1.0 / dx == 0.0) {
					اگر (dx < 0)
						retval = (1.0 / dy == 0.0 && dy < 0) ? 0 : -1;
					وگرنه
						retval = (1.0 / dy == 0.0 && dy > 0) ? 0 : 1;
				} وگرنه اگر (1.0 / dy == 0.0)
					retval = (dy < 0) ? 1 : -1;
				وگرنه
					retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			}
			بشکن;
		}
		حالت FLAG_M: {
			ساختار tm thyme;
			صحیح dx;
			نویسه *xx, *yy;

			xx = strptime(skip_whitespace(x), "%b", &thyme);
			dx = thyme.tm_mon;
			yy = strptime(skip_whitespace(y), "%b", &thyme);
			اگر (!xx)
				retval = (!yy) ? 0 : -1;
			وگرنه اگر (!yy)
				retval = 1;
			وگرنه
				retval = dx - thyme.tm_mon;
			بشکن;
		}
		/* Full floating point version of -n */
		حالت FLAG_n: {
			دوگانه dx = atof(x);
			دوگانه dy = atof(y);
			retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			بشکن;
		}
		} /* switch */
		/* Free key copies. */
		اگر (x != *(نویسه **)xarg) آزاد(x);
		اگر (y != *(نویسه **)yarg) آزاد(y);
		/* if (retval) break; - done by for () anyway */
#وگرنه
		/* Integer version of -n for tiny systems */
		حالت FLAG_n:
			retval = atoi(x) - atoi(y);
			بشکن;
		} /* switch */
#endif
	} /* for */

	اگر (retval == 0) {
		/* So far lines are "the same" */

		اگر (option_mask32 & FLAG_s) {
			/* "Stable sort": later line is "greater than",
			 * IOW: do not allow qsort() to swap equal lines.
			 */
			uint32_t *p32;
			uint32_t x32, y32;
			نویسه *سطر;
			unsigned len;

			سطر = *(نویسه**)xarg;
			len = (strlen(سطر) + 4) & (~3u);
			p32 = (تهی*)(سطر + len);
			x32 = *p32;
			سطر = *(نویسه**)yarg;
			len = (strlen(سطر) + 4) & (~3u);
			p32 = (تهی*)(سطر + len);
			y32 = *p32;

			/* If x > y, 1, else -1 */
			retval = (x32 > y32) * 2 - 1;
			/* Here, -r has no effect! */
			بازگشت retval;
		}
		اگر (!(option_mask32 & FLAG_no_tie_break)) {
			/* fallback sort */
			flags = option_mask32;
			retval = strcmp(*(نویسه **)xarg, *(نویسه **)yarg);
		}
	}

	اگر (flags & FLAG_r)
		بازگشت -retval;

	بازگشت retval;
}

#اگر ENABLE_FEATURE_SORT_BIG
ایستا unsigned str2u(نویسه **str)
{
	unsigned long lu;
	اگر (!isdigit((*str)[0]))
		bb_simple_error_پیام_and_die("bad field specification");
	lu = strtoul(*str, str, 10);
	اگر ((اندازه(long) > اندازه(صحیح) && lu > INT_MAX) || !lu)
		bb_simple_error_پیام_and_die("bad field specification");
	بازگشت lu;
}
#endif

صحیح sort_main(صحیح argc, نویسه **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح sort_main(صحیح argc UNUSED_PARAM, نویسه **argv)
{
	نویسه **سطرها;
	نویسه *str_ignored, *str_o, *str_t;
	llist_t *lst_k = پوچ;
	صحیح i;
	صحیح linecount;
	unsigned opts;
#اگر ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
	bool can_drop_dups;
	size_t prev_len = 0;
	نویسه *prev_سطر = (نویسه*) "";
	/* Postpone optimizing if the input is small, < 16k lines:
	 * even just free()ing duplicate lines takes time.
	 */
	size_t شمار_to_optimize_dups = 0x3fff;
#endif

	xfunc_error_retval = 2;

	/* Parse command line options */
	opts = getopt32(argv,
			sort_opt_str,
			&str_ignored, &str_ignored, &str_o, &lst_k, &str_t
	);
#اگر ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
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
	اگر (opts & FLAG_s)
		شمار_to_optimize_dups = (size_t)-1L;
#endif
	/* global b strips leading and trailing spaces */
	اگر (opts & FLAG_b)
		option_mask32 |= FLAG_bb;
#اگر ENABLE_FEATURE_SORT_BIG
	اگر (opts & FLAG_t) {
		اگر (!str_t[0] || str_t[1])
			bb_simple_error_پیام_and_die("bad -t parameter");
		کلید_separator = str_t[0];
	}
	/* note: below this point we use option_mask32, not opts,
	 * since that reduces register pressure and makes code smaller */

	/* Parse sort key */
	وقتی (lst_k) {
		شمارش {
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
		ساختار sort_کلید *کلید = add_کلید();
		نویسه *str_k = llist_برداشت(&lst_k);

		i = 0; /* i==0 before comma, 1 after (-k3,6) */
		وقتی (*str_k) {
			/* Start of range */
			/* Cannot use bb_strtou - suffix can be a letter */
			کلید->range[2*i] = str2u(&str_k);
			اگر (*str_k == '.') {
				str_k++;
				کلید->range[2*i+1] = str2u(&str_k);
			}
			وقتی (*str_k) {
				صحیح flag;
				ثابت نویسه *idx;

				اگر (*str_k == ',' && !i++) {
					str_k++;
					بشکن;
				} /* no else needed: fall through to syntax error
					because comma isn't in OPT_STR */
				idx = strchr(OPT_STR, *str_k);
				اگر (!idx)
					bb_simple_error_پیام_and_die("unknown key option");
				flag = 1 << (idx - OPT_STR);
				اگر (flag & ~FLAG_allowed_for_k)
					bb_simple_error_پیام_and_die("unknown sort type");
				/* b after ',' means strip _trailing_ space */
				اگر (i && flag == FLAG_b)
					flag = FLAG_bb;
				کلید->flags |= flag;
				str_k++;
			}
		}
	}
#endif

	/* Open input files and read data */
	argv += optind;
	اگر (!*argv)
		*--argv = (نویسه*)"-";
	linecount = 0;
	سطرها = پوچ;
	انجامبده {
		/* coreutils 6.9 compat: abort on first open error,
		 * do not continue to next file: */
		FILE *fp = xfopen_stdin(*argv);
		برای (;;) {
			نویسه *سطر = GET_سطر(fp);
			اگر (!سطر)
				بشکن;

#اگر ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
			اگر (شمار_to_optimize_dups != 0)
				شمار_to_optimize_dups--;
			اگر (شمار_to_optimize_dups == 0) {
				size_t len;
				نویسه *new_سطر;

				/* On kernel/linux/arch/ *.[ch] files,
				 * this reduces memory usage by 6%.
				 *  yes | head -99999999 | sort
				 * goes down from 1900Mb to 380 Mb.
				 */
				len = strlen(سطر);
				اگر (len <= prev_len) {
					new_سطر = prev_سطر + (prev_len - len);
					اگر (strcmp(سطر, new_سطر) == 0) {
						/* it's a tail of the prev line */
						اگر (can_drop_dups && prev_len == len) {
							/* it's identical to prev line */
							آزاد(سطر);
							ادامه;
						}
						آزاد(سطر);
						سطر = new_سطر;
						/* continue using longer prev_line
						 * for future tail tests.
						 */
						برو skip;
					}
				}
				prev_len = len;
				prev_سطر = سطر;
 skip: ;
			}
#وگرنه
//TODO: lighter version which only drops total dups if can_drop_dups == true
#endif
			سطرها = xrealloc_vector(سطرها, 6, linecount);
			سطرها[linecount++] = سطر;
		}
		fclose_if_not_stdin(fp);
	} وقتی (*++argv);

#اگر ENABLE_FEATURE_SORT_BIG
	/* If no key, perform alphabetic sort */
	اگر (!کلید_list)
		add_کلید()->range[0] = 1;
	/* Handle -c */
	اگر (option_mask32 & FLAG_c) {
		صحیح j = (option_mask32 & FLAG_u) ? -1 : 0;
		برای (i = 1; i < linecount; i++) {
			اگر (compare_کلیدها(&سطرها[i-1], &سطرها[i]) > j) {
				fprintf(stderr, "Check line %u\n", i);
				بازگشت EXIT_FAILURE;
			}
		}
		بازگشت EXIT_SUCCESS;
	}
#endif

	/* For stable sort, store original line position beyond terminating NUL */
	اگر (option_mask32 & FLAG_s) {
		برای (i = 0; i < linecount; i++) {
			uint32_t *p32;
			نویسه *سطر;
			unsigned len;

			سطر = سطرها[i];
			len = (strlen(سطر) + 4) & (~3u);
			سطرها[i] = سطر = xrealloc(سطر, len + 4);
			p32 = (تهی*)(سطر + len);
			*p32 = i;
		}
		/*option_mask32 |= FLAG_no_tie_break;*/
		/* ^^^redundant: if FLAG_s, compare_keys() does no tie break */
	}

	/* Perform the actual sort */
	qsort(سطرها, linecount, اندازه(سطرها[0]), compare_کلیدها);

	/* Handle -u */
	اگر (option_mask32 & FLAG_u) {
		صحیح j = 0;
		/* coreutils 6.3 drop lines for which only key is the same:
		 * - disabling last-resort compare, or else compare_keys()
		 * will be the same only for completely identical lines
		 * - disabling -s (same reasons)
		 */
		option_mask32 = (option_mask32 | FLAG_no_tie_break) & (~FLAG_s);
		برای (i = 1; i < linecount; i++) {
			اگر (compare_کلیدها(&سطرها[j], &سطرها[i]) == 0)
				آزاد(سطرها[i]);
			وگرنه
				سطرها[++j] = سطرها[i];
		}
		اگر (linecount)
			linecount = j+1;
	}

	/* Print it */
#اگر ENABLE_FEATURE_SORT_BIG
	/* Open output file _after_ we read all input ones */
	اگر (option_mask32 & FLAG_o)
		xmove_fd(xopen(str_o, O_WRONLY|O_CREAT|O_TRUNC), STDOUT_FILENO);
#endif
	{
		صحیح ch = (option_mask32 & FLAG_z) ? '\0' : '\n';
		برای (i = 0; i < linecount; i++)
			printf("%s%c", سطرها[i], ch);
	}

	fflush_stdout_and_exit_SUCCESS();
}
