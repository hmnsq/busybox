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

स्थिर स्थिरांक अक्षर sort_opt_str[] ALIGN1 = "^"
			"nghMVucszbrdfimS:T:o:k:*t:"
			"\0" "o--o:t--t"/*-t, -o: at most one of each*/;
/*
 * OPT_STR must not be string literal, needs to have stable address:
 * code uses "strchr(OPT_STR,c) - OPT_STR" idiom.
 */
#define OPT_STR (sort_opt_str + 1)

#जर ENABLE_FEATURE_SORT_BIG
स्थिर अक्षर किल्ली_separator;

स्थिर संरचना sort_किल्ली {
	संरचना sort_किल्ली *next_किल्ली;  /* linked list */
	अचिन्हित range[4];          /* start word, start char, end word, end char */
	अचिन्हित flags;
} *किल्ली_list;


/* This is a NOEXEC applet. Be very careful! */


स्थिर अक्षर *get_किल्ली(अक्षर *str, संरचना sort_किल्ली *किल्ली, पूर्णांक flags)
{
	पूर्णांक start = start; /* for compiler */
	पूर्णांक end;
	पूर्णांक len, j;
	अचिन्हित i;

	/* Special case whole string, so we don't have to make a copy */
	जर (किल्ली->range[0] == 1 && !किल्ली->range[1] && !किल्ली->range[2] && !किल्ली->range[3]
	 && !(flags & (FLAG_b | FLAG_d | FLAG_f | FLAG_i | FLAG_bb))
	) {
		परत str;
	}

	/* Find start of key on first pass, end on second pass */
	len = म_माप(str);
	साठी (j = 0; j < 2; j++) {
		जर (!किल्ली->range[2*j])
			end = len;
		/* Loop through fields */
		अन्यथा {
			अचिन्हित अक्षर ch = 0;

			end = 0;
			साठी (i = 1; i < किल्ली->range[2*j] + j; i++) {
				जर (किल्ली_separator) {
					/* Skip body of key and separator */
					पर्यंत ((ch = str[end]) != '\0') {
							end++;
						जर (ch == किल्ली_separator)
							थांबा;
					}
				} अन्यथा {
					/* Skip leading blanks */
					पर्यंत (है_खाली(str[end]))
						end++;
					/* Skip body of key */
					पर्यंत (str[end] != '\0') {
						जर (है_खाली(str[end]))
							थांबा;
						end++;
					}
				}
			}
			/* Remove last delim: "abc:def:" => "abc:def" */
			जर (j && ch) {
				//if (str[end-1] != key_separator)
				//  bb_error_msg(_and_die("BUG! "
				//  "str[start:%d,end:%d]:'%.*s'",
				//  start, end, (int)(end-start), &str[start]);
				end--;
			}
		}
		जर (!j) start = end;
	}
	/* Strip leading whitespace if necessary */
	जर (flags & FLAG_b)
		/* not using skip_whitespace() for speed */
		पर्यंत (है_खाली(str[start])) start++;
	/* Strip trailing whitespace if necessary */
	जर (flags & FLAG_bb)
		पर्यंत (end > start && है_खाली(str[end-1])) end--;
	/* -kSTART,N.ENDCHAR: honor ENDCHAR (1-based) */
	जर (किल्ली->range[3]) {
		end = किल्ली->range[3];
		जर (end > len) end = len;
	}
	/* -kN.STARTCHAR[,...]: honor STARTCHAR (1-based) */
	जर (किल्ली->range[1]) {
		start += किल्ली->range[1] - 1;
		जर (start > len) start = len;
	}
	/* Make the copy */
	जर (end < start)
		end = start;
	str = xstrndup(str+start, end-start);
	/* Handle -d */
	जर (flags & FLAG_d) {
		साठी (start = end = 0; str[end]; end++)
			जर (है_खाली(str[end]) || है_अक्षर_अंक(str[end]))
				str[start++] = str[end];
		str[start] = '\0';
	}
	/* Handle -i */
	जर (flags & FLAG_i) {
		साठी (start = end = 0; str[end]; end++)
			जर (isprint_asciionly(str[end]))
				str[start++] = str[end];
		str[start] = '\0';
	}
	/* Handle -f */
	जर (flags & FLAG_f)
		साठी (i = 0; str[i]; i++)
			str[i] = बड़े(str[i]);

	परत str;
}

स्थिर संरचना sort_किल्ली *add_किल्ली(शून्य)
{
	संरचना sort_किल्ली **pkey = &किल्ली_list;
	पर्यंत (*pkey)
		pkey = &((*pkey)->next_किल्ली);
	परत *pkey = xzalloc(आकार(संरचना sort_किल्ली));
}

#define GET_ओळ(fp) \
	((option_mask32 & FLAG_z) \
	? bb_get_chunk_from_file(fp, रिक्त) \
	: xmalloc_fgetline(fp))
#अन्यथा
#define GET_ओळ(fp) xmalloc_fgetline(fp)
#endif

#जर ENABLE_FEATURE_SORT_BIG
स्थिर पूर्णांक scale_suffix(स्थिरांक अक्षर *tail)
{
	स्थिर स्थिरांक अक्षर suffix[] ALIGN1 = "kmgtpezy";
	स्थिरांक अक्षर *s;
	पूर्णांक n;

	जर (!tail[0])
		परत -1;
	s = म_अक्षर(suffix, tail[0] | 0x20);
	जर (!s)
		परत -1;
	n = s - suffix;
	जर (n != 0 && tail[0] >= 'a')
		परत -1; /* mg... not accepted, only MG... */
	परत n;
}
#endif

/* Iterate through keys list and perform comparisons */
स्थिर पूर्णांक compare_किल्ल्या(स्थिरांक शून्य *xarg, स्थिरांक शून्य *yarg)
{
	पूर्णांक flags = option_mask32, retval = 0;
	अक्षर *x, *y;

#जर ENABLE_FEATURE_SORT_BIG
	संरचना sort_किल्ली *किल्ली;

	साठी (किल्ली = किल्ली_list; !retval && किल्ली; किल्ली = किल्ली->next_किल्ली) {
		flags = किल्ली->flags ? किल्ली->flags : option_mask32;
		/* Chop out and modify key chunks, handling -dfib */
		x = get_किल्ली(*(अक्षर **)xarg, किल्ली, flags);
		y = get_किल्ली(*(अक्षर **)yarg, किल्ली, flags);
#अन्यथा
	/* This curly bracket serves no purpose but to match the nesting
	 * level of the for () loop we're not using */
	{
		x = *(अक्षर **)xarg;
		y = *(अक्षर **)yarg;
#endif
		/* Perform actual comparison */
		निवड (flags & (FLAG_n | FLAG_g | FLAG_h | FLAG_M | FLAG_V)) {
		पूर्वनिर्धारित:
			bb_simple_error_संदेश_and_die("unknown sort type");
			थांबा;
#जर defined(HAVE_STRVERSCMP) && HAVE_STRVERSCMP == 1
		स्थिती FLAG_V:
			retval = strverscmp(x, y);
			थांबा;
#endif
		/* Ascii sort */
		स्थिती 0:
#जर ENABLE_LOCALE_SUPPORT
			retval = strcoll(x, y);
#अन्यथा
			retval = म_भेद(x, y);
#endif
			थांबा;
#जर ENABLE_FEATURE_SORT_BIG
		स्थिती FLAG_g:
		स्थिती FLAG_h: {
			अक्षर *xx, *yy;
//TODO: needs setlocale(LC_NUMERIC, "C")?
			द्विगुण dx = म_से_भग्न(x, &xx);
			द्विगुण dy = म_से_भग्न(y, &yy);
			/* not numbers < NaN < -infinity < numbers < +infinity) */
			जर (x == xx)
				retval = (y == yy ? 0 : -1);
			अन्यथा जर (y == yy)
				retval = 1;
			/* Check for isnan */
			अन्यथा जर (dx != dx)
				retval = (dy != dy) ? 0 : -1;
			अन्यथा जर (dy != dy)
				retval = 1;
			अन्यथा {
				जर (flags & FLAG_h) {
					पूर्णांक xs = scale_suffix(xx);
					पूर्णांक ys = scale_suffix(yy);
					जर (xs != ys) {
						retval = xs - ys;
						थांबा;
					}
				}
				/* Check for infinity.  Could underflow, but it avoids libm. */
				जर (1.0 / dx == 0.0) {
					जर (dx < 0)
						retval = (1.0 / dy == 0.0 && dy < 0) ? 0 : -1;
					अन्यथा
						retval = (1.0 / dy == 0.0 && dy > 0) ? 0 : 1;
				} अन्यथा जर (1.0 / dy == 0.0)
					retval = (dy < 0) ? 1 : -1;
				अन्यथा
					retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			}
			थांबा;
		}
		स्थिती FLAG_M: {
			संरचना पंचांग thyme;
			पूर्णांक dx;
			अक्षर *xx, *yy;

			xx = strptime(skip_whitespace(x), "%b", &thyme);
			dx = thyme.tm_mon;
			yy = strptime(skip_whitespace(y), "%b", &thyme);
			जर (!xx)
				retval = (!yy) ? 0 : -1;
			अन्यथा जर (!yy)
				retval = 1;
			अन्यथा
				retval = dx - thyme.tm_mon;
			थांबा;
		}
		/* Full floating point version of -n */
		स्थिती FLAG_n: {
			द्विगुण dx = म_से_भ(x);
			द्विगुण dy = म_से_भ(y);
			retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			थांबा;
		}
		} /* switch */
		/* Free key copies. */
		जर (x != *(अक्षर **)xarg) मुक्त(x);
		जर (y != *(अक्षर **)yarg) मुक्त(y);
		/* if (retval) break; - done by for () anyway */
#अन्यथा
		/* Integer version of -n for tiny systems */
		स्थिती FLAG_n:
			retval = म_से_प(x) - म_से_प(y);
			थांबा;
		} /* switch */
#endif
	} /* for */

	जर (retval == 0) {
		/* So far lines are "the same" */

		जर (option_mask32 & FLAG_s) {
			/* "Stable sort": later line is "greater than",
			 * IOW: do not allow qsort() to swap equal lines.
			 */
			uint32_t *p32;
			uint32_t x32, y32;
			अक्षर *ओळ;
			अचिन्हित len;

			ओळ = *(अक्षर**)xarg;
			len = (म_माप(ओळ) + 4) & (~3u);
			p32 = (शून्य*)(ओळ + len);
			x32 = *p32;
			ओळ = *(अक्षर**)yarg;
			len = (म_माप(ओळ) + 4) & (~3u);
			p32 = (शून्य*)(ओळ + len);
			y32 = *p32;

			/* If x > y, 1, else -1 */
			retval = (x32 > y32) * 2 - 1;
			/* Here, -r has no effect! */
			परत retval;
		}
		जर (!(option_mask32 & FLAG_no_tie_break)) {
			/* fallback sort */
			flags = option_mask32;
			retval = म_भेद(*(अक्षर **)xarg, *(अक्षर **)yarg);
		}
	}

	जर (flags & FLAG_r)
		परत -retval;

	परत retval;
}

#जर ENABLE_FEATURE_SORT_BIG
स्थिर अचिन्हित str2u(अक्षर **str)
{
	अचिन्हित दीर्घ lu;
	जर (!है_अंक((*str)[0]))
		bb_simple_error_संदेश_and_die("bad field specification");
	lu = म_से_अदीर्घ(*str, str, 10);
	जर ((आकार(दीर्घ) > आकार(पूर्णांक) && lu > पूर्णांक_उच्च) || !lu)
		bb_simple_error_संदेश_and_die("bad field specification");
	परत lu;
}
#endif

पूर्णांक sort_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक sort_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	अक्षर **ओळी;
	अक्षर *str_ignored, *str_o, *str_t;
	llist_t *lst_k = रिक्त;
	पूर्णांक i;
	पूर्णांक linecount;
	अचिन्हित opts;
#जर ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
	bool can_drop_dups;
	माप_प्रकार prev_len = 0;
	अक्षर *prev_ओळ = (अक्षर*) "";
	/* Postpone optimizing if the input is small, < 16k lines:
	 * even just free()ing duplicate lines takes time.
	 */
	माप_प्रकार मोजणी_to_optimize_dups = 0x3fff;
#endif

	xfunc_error_retval = 2;

	/* Parse command line options */
	opts = getopt32(argv,
			sort_opt_str,
			&str_ignored, &str_ignored, &str_o, &lst_k, &str_t
	);
#जर ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
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
	जर (opts & FLAG_s)
		मोजणी_to_optimize_dups = (माप_प्रकार)-1L;
#endif
	/* global b strips leading and trailing spaces */
	जर (opts & FLAG_b)
		option_mask32 |= FLAG_bb;
#जर ENABLE_FEATURE_SORT_BIG
	जर (opts & FLAG_t) {
		जर (!str_t[0] || str_t[1])
			bb_simple_error_संदेश_and_die("bad -t parameter");
		किल्ली_separator = str_t[0];
	}
	/* note: below this point we use option_mask32, not opts,
	 * since that reduces register pressure and makes code smaller */

	/* Parse sort key */
	पर्यंत (lst_k) {
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
		संरचना sort_किल्ली *किल्ली = add_किल्ली();
		अक्षर *str_k = llist_काढा(&lst_k);

		i = 0; /* i==0 before comma, 1 after (-k3,6) */
		पर्यंत (*str_k) {
			/* Start of range */
			/* Cannot use bb_strtou - suffix can be a letter */
			किल्ली->range[2*i] = str2u(&str_k);
			जर (*str_k == '.') {
				str_k++;
				किल्ली->range[2*i+1] = str2u(&str_k);
			}
			पर्यंत (*str_k) {
				पूर्णांक flag;
				स्थिरांक अक्षर *idx;

				जर (*str_k == ',' && !i++) {
					str_k++;
					थांबा;
				} /* no else needed: fall through to syntax error
					because comma isn't in OPT_STR */
				idx = म_अक्षर(OPT_STR, *str_k);
				जर (!idx)
					bb_simple_error_संदेश_and_die("unknown key option");
				flag = 1 << (idx - OPT_STR);
				जर (flag & ~FLAG_allowed_for_k)
					bb_simple_error_संदेश_and_die("unknown sort type");
				/* b after ',' means strip _trailing_ space */
				जर (i && flag == FLAG_b)
					flag = FLAG_bb;
				किल्ली->flags |= flag;
				str_k++;
			}
		}
	}
#endif

	/* Open input files and read data */
	argv += optind;
	जर (!*argv)
		*--argv = (अक्षर*)"-";
	linecount = 0;
	ओळी = रिक्त;
	करा {
		/* coreutils 6.9 compat: abort on first open error,
		 * do not continue to next file: */
		खाता *fp = xfopen_stdin(*argv);
		साठी (;;) {
			अक्षर *ओळ = GET_ओळ(fp);
			जर (!ओळ)
				थांबा;

#जर ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
			जर (मोजणी_to_optimize_dups != 0)
				मोजणी_to_optimize_dups--;
			जर (मोजणी_to_optimize_dups == 0) {
				माप_प्रकार len;
				अक्षर *new_ओळ;

				/* On kernel/linux/arch/ *.[ch] files,
				 * this reduces memory usage by 6%.
				 *  yes | head -99999999 | sort
				 * goes down from 1900Mb to 380 Mb.
				 */
				len = म_माप(ओळ);
				जर (len <= prev_len) {
					new_ओळ = prev_ओळ + (prev_len - len);
					जर (म_भेद(ओळ, new_ओळ) == 0) {
						/* it's a tail of the prev line */
						जर (can_drop_dups && prev_len == len) {
							/* it's identical to prev line */
							मुक्त(ओळ);
							सुरूठेवा;
						}
						मुक्त(ओळ);
						ओळ = new_ओळ;
						/* continue using longer prev_line
						 * for future tail tests.
						 */
						जा skip;
					}
				}
				prev_len = len;
				prev_ओळ = ओळ;
 skip: ;
			}
#अन्यथा
//TODO: lighter version which only drops total dups if can_drop_dups == true
#endif
			ओळी = xrealloc_vector(ओळी, 6, linecount);
			ओळी[linecount++] = ओळ;
		}
		fclose_if_not_stdin(fp);
	} पर्यंत (*++argv);

#जर ENABLE_FEATURE_SORT_BIG
	/* If no key, perform alphabetic sort */
	जर (!किल्ली_list)
		add_किल्ली()->range[0] = 1;
	/* Handle -c */
	जर (option_mask32 & FLAG_c) {
		पूर्णांक j = (option_mask32 & FLAG_u) ? -1 : 0;
		साठी (i = 1; i < linecount; i++) {
			जर (compare_किल्ल्या(&ओळी[i-1], &ओळी[i]) > j) {
				ख_लिखो(मानक_त्रुटि, "Check line %u\n", i);
				परत निकास_त्रुटि;
			}
		}
		परत निकास_सफल;
	}
#endif

	/* For stable sort, store original line position beyond terminating NUL */
	जर (option_mask32 & FLAG_s) {
		साठी (i = 0; i < linecount; i++) {
			uint32_t *p32;
			अक्षर *ओळ;
			अचिन्हित len;

			ओळ = ओळी[i];
			len = (म_माप(ओळ) + 4) & (~3u);
			ओळी[i] = ओळ = xrealloc(ओळ, len + 4);
			p32 = (शून्य*)(ओळ + len);
			*p32 = i;
		}
		/*option_mask32 |= FLAG_no_tie_break;*/
		/* ^^^redundant: if FLAG_s, compare_keys() does no tie break */
	}

	/* Perform the actual sort */
	क्विक(ओळी, linecount, आकार(ओळी[0]), compare_किल्ल्या);

	/* Handle -u */
	जर (option_mask32 & FLAG_u) {
		पूर्णांक j = 0;
		/* coreutils 6.3 drop lines for which only key is the same:
		 * - disabling last-resort compare, or else compare_keys()
		 * will be the same only for completely identical lines
		 * - disabling -s (same reasons)
		 */
		option_mask32 = (option_mask32 | FLAG_no_tie_break) & (~FLAG_s);
		साठी (i = 1; i < linecount; i++) {
			जर (compare_किल्ल्या(&ओळी[j], &ओळी[i]) == 0)
				मुक्त(ओळी[i]);
			अन्यथा
				ओळी[++j] = ओळी[i];
		}
		जर (linecount)
			linecount = j+1;
	}

	/* Print it */
#जर ENABLE_FEATURE_SORT_BIG
	/* Open output file _after_ we read all input ones */
	जर (option_mask32 & FLAG_o)
		xmove_fd(xopen(str_o, O_WRONLY|O_CREAT|O_TRUNC), STDOUT_FILENO);
#endif
	{
		पूर्णांक ch = (option_mask32 & FLAG_z) ? '\0' : '\n';
		साठी (i = 0; i < linecount; i++)
			म_लिखो("%s%c", ओळी[i], ch);
	}

	fflush_stdout_and_exit_SUCCESS();
}
