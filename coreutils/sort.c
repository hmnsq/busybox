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

स्थिर अचर वर्ण sort_opt_str[] ALIGN1 = "^"
			"nghMVucszbrdfimS:T:o:k:*t:"
			"\0" "o--o:t--t"/*-t, -o: at most one of each*/;
/*
 * OPT_STR must not be string literal, needs to have stable address:
 * code uses "strchr(OPT_STR,c) - OPT_STR" idiom.
 */
#define OPT_STR (sort_opt_str + 1)

#यदि ENABLE_FEATURE_SORT_BIG
स्थिर वर्ण साँचो_separator;

स्थिर संरचना sort_साँचो {
	संरचना sort_साँचो *next_साँचो;  /* linked list */
	अचिन्हित range[4];          /* start word, start char, end word, end char */
	अचिन्हित flags;
} *साँचो_list;


/* This is a NOEXEC applet. Be very careful! */


स्थिर वर्ण *get_साँचो(वर्ण *str, संरचना sort_साँचो *साँचो, पूर्णाङ्क flags)
{
	पूर्णाङ्क start = start; /* for compiler */
	पूर्णाङ्क end;
	पूर्णाङ्क len, j;
	अचिन्हित i;

	/* Special case whole string, so we don't have to make a copy */
	यदि (साँचो->range[0] == 1 && !साँचो->range[1] && !साँचो->range[2] && !साँचो->range[3]
	 && !(flags & (FLAG_b | FLAG_d | FLAG_f | FLAG_i | FLAG_bb))
	) {
		फर्काऊ str;
	}

	/* Find start of key on first pass, end on second pass */
	len = म_माप(str);
	लागि (j = 0; j < 2; j++) {
		यदि (!साँचो->range[2*j])
			end = len;
		/* Loop through fields */
		अन्यथा {
			अचिन्हित वर्ण ch = 0;

			end = 0;
			लागि (i = 1; i < साँचो->range[2*j] + j; i++) {
				यदि (साँचो_separator) {
					/* Skip body of key and separator */
					जबसम्म ((ch = str[end]) != '\0') {
							end++;
						यदि (ch == साँचो_separator)
							रोक;
					}
				} अन्यथा {
					/* Skip leading blanks */
					जबसम्म (है_खाली(str[end]))
						end++;
					/* Skip body of key */
					जबसम्म (str[end] != '\0') {
						यदि (है_खाली(str[end]))
							रोक;
						end++;
					}
				}
			}
			/* Remove last delim: "abc:def:" => "abc:def" */
			यदि (j && ch) {
				//if (str[end-1] != key_separator)
				//  bb_error_msg(_and_die("BUG! "
				//  "str[start:%d,end:%d]:'%.*s'",
				//  start, end, (int)(end-start), &str[start]);
				end--;
			}
		}
		यदि (!j) start = end;
	}
	/* Strip leading whitespace if necessary */
	यदि (flags & FLAG_b)
		/* not using skip_whitespace() for speed */
		जबसम्म (है_खाली(str[start])) start++;
	/* Strip trailing whitespace if necessary */
	यदि (flags & FLAG_bb)
		जबसम्म (end > start && है_खाली(str[end-1])) end--;
	/* -kSTART,N.ENDCHAR: honor ENDCHAR (1-based) */
	यदि (साँचो->range[3]) {
		end = साँचो->range[3];
		यदि (end > len) end = len;
	}
	/* -kN.STARTCHAR[,...]: honor STARTCHAR (1-based) */
	यदि (साँचो->range[1]) {
		start += साँचो->range[1] - 1;
		यदि (start > len) start = len;
	}
	/* Make the copy */
	यदि (end < start)
		end = start;
	str = xstrndup(str+start, end-start);
	/* Handle -d */
	यदि (flags & FLAG_d) {
		लागि (start = end = 0; str[end]; end++)
			यदि (है_खाली(str[end]) || है_अक्षर_अंक(str[end]))
				str[start++] = str[end];
		str[start] = '\0';
	}
	/* Handle -i */
	यदि (flags & FLAG_i) {
		लागि (start = end = 0; str[end]; end++)
			यदि (isprint_asciionly(str[end]))
				str[start++] = str[end];
		str[start] = '\0';
	}
	/* Handle -f */
	यदि (flags & FLAG_f)
		लागि (i = 0; str[i]; i++)
			str[i] = बड़े(str[i]);

	फर्काऊ str;
}

स्थिर संरचना sort_साँचो *add_साँचो(शून्य)
{
	संरचना sort_साँचो **pkey = &साँचो_list;
	जबसम्म (*pkey)
		pkey = &((*pkey)->next_साँचो);
	फर्काऊ *pkey = xzalloc(परिमाण(संरचना sort_साँचो));
}

#define GET_पङ्क्ति(fp) \
	((option_mask32 & FLAG_z) \
	? bb_get_chunk_from_file(fp, रिक्त) \
	: xmalloc_fgetline(fp))
#अन्यथा
#define GET_पङ्क्ति(fp) xmalloc_fgetline(fp)
#endif

#यदि ENABLE_FEATURE_SORT_BIG
स्थिर पूर्णाङ्क scale_suffix(अचर वर्ण *tail)
{
	स्थिर अचर वर्ण suffix[] ALIGN1 = "kmgtpezy";
	अचर वर्ण *s;
	पूर्णाङ्क n;

	यदि (!tail[0])
		फर्काऊ -1;
	s = म_अक्षर(suffix, tail[0] | 0x20);
	यदि (!s)
		फर्काऊ -1;
	n = s - suffix;
	यदि (n != 0 && tail[0] >= 'a')
		फर्काऊ -1; /* mg... not accepted, only MG... */
	फर्काऊ n;
}
#endif

/* Iterate through keys list and perform comparisons */
स्थिर पूर्णाङ्क compare_साँचोहरू(अचर शून्य *xarg, अचर शून्य *yarg)
{
	पूर्णाङ्क flags = option_mask32, retval = 0;
	वर्ण *x, *y;

#यदि ENABLE_FEATURE_SORT_BIG
	संरचना sort_साँचो *साँचो;

	लागि (साँचो = साँचो_list; !retval && साँचो; साँचो = साँचो->next_साँचो) {
		flags = साँचो->flags ? साँचो->flags : option_mask32;
		/* Chop out and modify key chunks, handling -dfib */
		x = get_साँचो(*(वर्ण **)xarg, साँचो, flags);
		y = get_साँचो(*(वर्ण **)yarg, साँचो, flags);
#अन्यथा
	/* This curly bracket serves no purpose but to match the nesting
	 * level of the for () loop we're not using */
	{
		x = *(वर्ण **)xarg;
		y = *(वर्ण **)yarg;
#endif
		/* Perform actual comparison */
		विकल्प (flags & (FLAG_n | FLAG_g | FLAG_h | FLAG_M | FLAG_V)) {
		स्वभाव:
			bb_simple_error_सन्देश_and_die("unknown sort type");
			रोक;
#यदि defined(HAVE_STRVERSCMP) && HAVE_STRVERSCMP == 1
		पक्ष FLAG_V:
			retval = strverscmp(x, y);
			रोक;
#endif
		/* Ascii sort */
		पक्ष 0:
#यदि ENABLE_LOCALE_SUPPORT
			retval = strcoll(x, y);
#अन्यथा
			retval = म_भेद(x, y);
#endif
			रोक;
#यदि ENABLE_FEATURE_SORT_BIG
		पक्ष FLAG_g:
		पक्ष FLAG_h: {
			वर्ण *xx, *yy;
//TODO: needs setlocale(LC_NUMERIC, "C")?
			द्विगुण dx = म_से_भग्न(x, &xx);
			द्विगुण dy = म_से_भग्न(y, &yy);
			/* not numbers < NaN < -infinity < numbers < +infinity) */
			यदि (x == xx)
				retval = (y == yy ? 0 : -1);
			अन्यथा यदि (y == yy)
				retval = 1;
			/* Check for isnan */
			अन्यथा यदि (dx != dx)
				retval = (dy != dy) ? 0 : -1;
			अन्यथा यदि (dy != dy)
				retval = 1;
			अन्यथा {
				यदि (flags & FLAG_h) {
					पूर्णाङ्क xs = scale_suffix(xx);
					पूर्णाङ्क ys = scale_suffix(yy);
					यदि (xs != ys) {
						retval = xs - ys;
						रोक;
					}
				}
				/* Check for infinity.  Could underflow, but it avoids libm. */
				यदि (1.0 / dx == 0.0) {
					यदि (dx < 0)
						retval = (1.0 / dy == 0.0 && dy < 0) ? 0 : -1;
					अन्यथा
						retval = (1.0 / dy == 0.0 && dy > 0) ? 0 : 1;
				} अन्यथा यदि (1.0 / dy == 0.0)
					retval = (dy < 0) ? 1 : -1;
				अन्यथा
					retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			}
			रोक;
		}
		पक्ष FLAG_M: {
			संरचना पंचांग thyme;
			पूर्णाङ्क dx;
			वर्ण *xx, *yy;

			xx = strptime(skip_whitespace(x), "%b", &thyme);
			dx = thyme.tm_mon;
			yy = strptime(skip_whitespace(y), "%b", &thyme);
			यदि (!xx)
				retval = (!yy) ? 0 : -1;
			अन्यथा यदि (!yy)
				retval = 1;
			अन्यथा
				retval = dx - thyme.tm_mon;
			रोक;
		}
		/* Full floating point version of -n */
		पक्ष FLAG_n: {
			द्विगुण dx = म_से_भ(x);
			द्विगुण dy = म_से_भ(y);
			retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			रोक;
		}
		} /* switch */
		/* Free key copies. */
		यदि (x != *(वर्ण **)xarg) मुक्त(x);
		यदि (y != *(वर्ण **)yarg) मुक्त(y);
		/* if (retval) break; - done by for () anyway */
#अन्यथा
		/* Integer version of -n for tiny systems */
		पक्ष FLAG_n:
			retval = म_से_प(x) - म_से_प(y);
			रोक;
		} /* switch */
#endif
	} /* for */

	यदि (retval == 0) {
		/* So far lines are "the same" */

		यदि (option_mask32 & FLAG_s) {
			/* "Stable sort": later line is "greater than",
			 * IOW: do not allow qsort() to swap equal lines.
			 */
			uint32_t *p32;
			uint32_t x32, y32;
			वर्ण *पङ्क्ति;
			अचिन्हित len;

			पङ्क्ति = *(वर्ण**)xarg;
			len = (म_माप(पङ्क्ति) + 4) & (~3u);
			p32 = (शून्य*)(पङ्क्ति + len);
			x32 = *p32;
			पङ्क्ति = *(वर्ण**)yarg;
			len = (म_माप(पङ्क्ति) + 4) & (~3u);
			p32 = (शून्य*)(पङ्क्ति + len);
			y32 = *p32;

			/* If x > y, 1, else -1 */
			retval = (x32 > y32) * 2 - 1;
			/* Here, -r has no effect! */
			फर्काऊ retval;
		}
		यदि (!(option_mask32 & FLAG_no_tie_break)) {
			/* fallback sort */
			flags = option_mask32;
			retval = म_भेद(*(वर्ण **)xarg, *(वर्ण **)yarg);
		}
	}

	यदि (flags & FLAG_r)
		फर्काऊ -retval;

	फर्काऊ retval;
}

#यदि ENABLE_FEATURE_SORT_BIG
स्थिर अचिन्हित str2u(वर्ण **str)
{
	अचिन्हित दीर्घ lu;
	यदि (!है_अंक((*str)[0]))
		bb_simple_error_सन्देश_and_die("bad field specification");
	lu = म_से_अदीर्घ(*str, str, 10);
	यदि ((परिमाण(दीर्घ) > परिमाण(पूर्णाङ्क) && lu > पूर्णांक_उच्च) || !lu)
		bb_simple_error_सन्देश_and_die("bad field specification");
	फर्काऊ lu;
}
#endif

पूर्णाङ्क sort_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क sort_main(पूर्णाङ्क argc UNUSED_PARAM, वर्ण **argv)
{
	वर्ण **पङ्क्तिहरू;
	वर्ण *str_ignored, *str_o, *str_t;
	llist_t *lst_k = रिक्त;
	पूर्णाङ्क i;
	पूर्णाङ्क linecount;
	अचिन्हित opts;
#यदि ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
	bool can_drop_dups;
	माप_प्रकार prev_len = 0;
	वर्ण *prev_पङ्क्ति = (वर्ण*) "";
	/* Postpone optimizing if the input is small, < 16k lines:
	 * even just free()ing duplicate lines takes time.
	 */
	माप_प्रकार गन्ती_to_optimize_dups = 0x3fff;
#endif

	xfunc_error_retval = 2;

	/* Parse command line options */
	opts = getopt32(argv,
			sort_opt_str,
			&str_ignored, &str_ignored, &str_o, &lst_k, &str_t
	);
#यदि ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
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
	यदि (opts & FLAG_s)
		गन्ती_to_optimize_dups = (माप_प्रकार)-1L;
#endif
	/* global b strips leading and trailing spaces */
	यदि (opts & FLAG_b)
		option_mask32 |= FLAG_bb;
#यदि ENABLE_FEATURE_SORT_BIG
	यदि (opts & FLAG_t) {
		यदि (!str_t[0] || str_t[1])
			bb_simple_error_सन्देश_and_die("bad -t parameter");
		साँचो_separator = str_t[0];
	}
	/* note: below this point we use option_mask32, not opts,
	 * since that reduces register pressure and makes code smaller */

	/* Parse sort key */
	जबसम्म (lst_k) {
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
		संरचना sort_साँचो *साँचो = add_साँचो();
		वर्ण *str_k = llist_झिक(&lst_k);

		i = 0; /* i==0 before comma, 1 after (-k3,6) */
		जबसम्म (*str_k) {
			/* Start of range */
			/* Cannot use bb_strtou - suffix can be a letter */
			साँचो->range[2*i] = str2u(&str_k);
			यदि (*str_k == '.') {
				str_k++;
				साँचो->range[2*i+1] = str2u(&str_k);
			}
			जबसम्म (*str_k) {
				पूर्णाङ्क flag;
				अचर वर्ण *idx;

				यदि (*str_k == ',' && !i++) {
					str_k++;
					रोक;
				} /* no else needed: fall through to syntax error
					because comma isn't in OPT_STR */
				idx = म_अक्षर(OPT_STR, *str_k);
				यदि (!idx)
					bb_simple_error_सन्देश_and_die("unknown key option");
				flag = 1 << (idx - OPT_STR);
				यदि (flag & ~FLAG_allowed_for_k)
					bb_simple_error_सन्देश_and_die("unknown sort type");
				/* b after ',' means strip _trailing_ space */
				यदि (i && flag == FLAG_b)
					flag = FLAG_bb;
				साँचो->flags |= flag;
				str_k++;
			}
		}
	}
#endif

	/* Open input files and read data */
	argv += optind;
	यदि (!*argv)
		*--argv = (वर्ण*)"-";
	linecount = 0;
	पङ्क्तिहरू = रिक्त;
	गर {
		/* coreutils 6.9 compat: abort on first open error,
		 * do not continue to next file: */
		खाता *fp = xfopen_stdin(*argv);
		लागि (;;) {
			वर्ण *पङ्क्ति = GET_पङ्क्ति(fp);
			यदि (!पङ्क्ति)
				रोक;

#यदि ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
			यदि (गन्ती_to_optimize_dups != 0)
				गन्ती_to_optimize_dups--;
			यदि (गन्ती_to_optimize_dups == 0) {
				माप_प्रकार len;
				वर्ण *new_पङ्क्ति;

				/* On kernel/linux/arch/ *.[ch] files,
				 * this reduces memory usage by 6%.
				 *  yes | head -99999999 | sort
				 * goes down from 1900Mb to 380 Mb.
				 */
				len = म_माप(पङ्क्ति);
				यदि (len <= prev_len) {
					new_पङ्क्ति = prev_पङ्क्ति + (prev_len - len);
					यदि (म_भेद(पङ्क्ति, new_पङ्क्ति) == 0) {
						/* it's a tail of the prev line */
						यदि (can_drop_dups && prev_len == len) {
							/* it's identical to prev line */
							मुक्त(पङ्क्ति);
							जारी;
						}
						मुक्त(पङ्क्ति);
						पङ्क्ति = new_पङ्क्ति;
						/* continue using longer prev_line
						 * for future tail tests.
						 */
						गच्छ skip;
					}
				}
				prev_len = len;
				prev_पङ्क्ति = पङ्क्ति;
 skip: ;
			}
#अन्यथा
//TODO: lighter version which only drops total dups if can_drop_dups == true
#endif
			पङ्क्तिहरू = xrealloc_vector(पङ्क्तिहरू, 6, linecount);
			पङ्क्तिहरू[linecount++] = पङ्क्ति;
		}
		fclose_if_not_stdin(fp);
	} जबसम्म (*++argv);

#यदि ENABLE_FEATURE_SORT_BIG
	/* If no key, perform alphabetic sort */
	यदि (!साँचो_list)
		add_साँचो()->range[0] = 1;
	/* Handle -c */
	यदि (option_mask32 & FLAG_c) {
		पूर्णाङ्क j = (option_mask32 & FLAG_u) ? -1 : 0;
		लागि (i = 1; i < linecount; i++) {
			यदि (compare_साँचोहरू(&पङ्क्तिहरू[i-1], &पङ्क्तिहरू[i]) > j) {
				ख_लिखो(मानक_त्रुटि, "Check line %u\n", i);
				फर्काऊ निकास_त्रुटि;
			}
		}
		फर्काऊ निकास_सफल;
	}
#endif

	/* For stable sort, store original line position beyond terminating NUL */
	यदि (option_mask32 & FLAG_s) {
		लागि (i = 0; i < linecount; i++) {
			uint32_t *p32;
			वर्ण *पङ्क्ति;
			अचिन्हित len;

			पङ्क्ति = पङ्क्तिहरू[i];
			len = (म_माप(पङ्क्ति) + 4) & (~3u);
			पङ्क्तिहरू[i] = पङ्क्ति = xrealloc(पङ्क्ति, len + 4);
			p32 = (शून्य*)(पङ्क्ति + len);
			*p32 = i;
		}
		/*option_mask32 |= FLAG_no_tie_break;*/
		/* ^^^redundant: if FLAG_s, compare_keys() does no tie break */
	}

	/* Perform the actual sort */
	क्विक(पङ्क्तिहरू, linecount, परिमाण(पङ्क्तिहरू[0]), compare_साँचोहरू);

	/* Handle -u */
	यदि (option_mask32 & FLAG_u) {
		पूर्णाङ्क j = 0;
		/* coreutils 6.3 drop lines for which only key is the same:
		 * - disabling last-resort compare, or else compare_keys()
		 * will be the same only for completely identical lines
		 * - disabling -s (same reasons)
		 */
		option_mask32 = (option_mask32 | FLAG_no_tie_break) & (~FLAG_s);
		लागि (i = 1; i < linecount; i++) {
			यदि (compare_साँचोहरू(&पङ्क्तिहरू[j], &पङ्क्तिहरू[i]) == 0)
				मुक्त(पङ्क्तिहरू[i]);
			अन्यथा
				पङ्क्तिहरू[++j] = पङ्क्तिहरू[i];
		}
		यदि (linecount)
			linecount = j+1;
	}

	/* Print it */
#यदि ENABLE_FEATURE_SORT_BIG
	/* Open output file _after_ we read all input ones */
	यदि (option_mask32 & FLAG_o)
		xmove_fd(xopen(str_o, O_WRONLY|O_CREAT|O_TRUNC), STDOUT_FILENO);
#endif
	{
		पूर्णाङ्क ch = (option_mask32 & FLAG_z) ? '\0' : '\n';
		लागि (i = 0; i < linecount; i++)
			म_लिखो("%s%c", पङ्क्तिहरू[i], ch);
	}

	fflush_stdout_and_exit_SUCCESS();
}
