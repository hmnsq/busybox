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

#समावेश "libbb.h"

/* These are sort types */
क्रमागत {
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

अटल स्थिर अक्षर sort_opt_str[] ALIGN1 = "^"
			"nghMVucszbrdfimS:T:o:k:*t:"
			"\0" "o--o:t--t"/*-t, -o: at most one of each*/;
/*
 * OPT_STR must not be string literal, needs to have stable address:
 * code uses "strchr(OPT_STR,c) - OPT_STR" idiom.
 */
#घोषणा OPT_STR (sort_opt_str + 1)

#अगर ENABLE_FEATURE_SORT_BIG
अटल अक्षर कुंजी_separator;

अटल काष्ठा sort_कुंजी {
	काष्ठा sort_कुंजी *अग्र_कुंजी;  /* linked list */
	अचिन्हित range[4];          /* start word, start char, end word, end char */
	अचिन्हित flags;
} *कुंजी_list;


/* This is a NOEXEC applet. Be very careful! */


अटल अक्षर *get_कुंजी(अक्षर *str, काष्ठा sort_कुंजी *कुंजी, पूर्णांक flags)
{
	पूर्णांक start = start; /* for compiler */
	पूर्णांक end;
	पूर्णांक len, j;
	अचिन्हित i;

	/* Special case whole string, so we don't have to make a copy */
	अगर (कुंजी->range[0] == 1 && !कुंजी->range[1] && !कुंजी->range[2] && !कुंजी->range[3]
	 && !(flags & (FLAG_b | FLAG_d | FLAG_f | FLAG_i | FLAG_bb))
	) {
		वापस str;
	}

	/* Find start of key on first pass, end on second pass */
	len = म_माप(str);
	क्रम (j = 0; j < 2; j++) {
		अगर (!कुंजी->range[2*j])
			end = len;
		/* Loop through fields */
		अन्यथा {
			अचिन्हित अक्षर ch = 0;

			end = 0;
			क्रम (i = 1; i < कुंजी->range[2*j] + j; i++) {
				अगर (कुंजी_separator) {
					/* Skip body of key and separator */
					जबतक ((ch = str[end]) != '\0') {
							end++;
						अगर (ch == कुंजी_separator)
							अवरोध;
					}
				} अन्यथा {
					/* Skip leading blanks */
					जबतक (है_खाली(str[end]))
						end++;
					/* Skip body of key */
					जबतक (str[end] != '\0') {
						अगर (है_खाली(str[end]))
							अवरोध;
						end++;
					}
				}
			}
			/* Remove last delim: "abc:def:" => "abc:def" */
			अगर (j && ch) {
				//if (str[end-1] != key_separator)
				//  bb_error_msg(_and_die("BUG! "
				//  "str[start:%d,end:%d]:'%.*s'",
				//  start, end, (int)(end-start), &str[start]);
				end--;
			}
		}
		अगर (!j) start = end;
	}
	/* Strip leading whitespace if necessary */
	अगर (flags & FLAG_b)
		/* not using skip_whitespace() for speed */
		जबतक (है_खाली(str[start])) start++;
	/* Strip trailing whitespace if necessary */
	अगर (flags & FLAG_bb)
		जबतक (end > start && है_खाली(str[end-1])) end--;
	/* -kSTART,N.ENDCHAR: honor ENDCHAR (1-based) */
	अगर (कुंजी->range[3]) {
		end = कुंजी->range[3];
		अगर (end > len) end = len;
	}
	/* -kN.STARTCHAR[,...]: honor STARTCHAR (1-based) */
	अगर (कुंजी->range[1]) {
		start += कुंजी->range[1] - 1;
		अगर (start > len) start = len;
	}
	/* Make the copy */
	अगर (end < start)
		end = start;
	str = xstrndup(str+start, end-start);
	/* Handle -d */
	अगर (flags & FLAG_d) {
		क्रम (start = end = 0; str[end]; end++)
			अगर (है_खाली(str[end]) || है_अक्षर_अंक(str[end]))
				str[start++] = str[end];
		str[start] = '\0';
	}
	/* Handle -i */
	अगर (flags & FLAG_i) {
		क्रम (start = end = 0; str[end]; end++)
			अगर (isprint_asciionly(str[end]))
				str[start++] = str[end];
		str[start] = '\0';
	}
	/* Handle -f */
	अगर (flags & FLAG_f)
		क्रम (i = 0; str[i]; i++)
			str[i] = बड़े(str[i]);

	वापस str;
}

अटल काष्ठा sort_कुंजी *add_कुंजी(व्योम)
{
	काष्ठा sort_कुंजी **pkey = &कुंजी_list;
	जबतक (*pkey)
		pkey = &((*pkey)->अग्र_कुंजी);
	वापस *pkey = xzalloc(माप(काष्ठा sort_कुंजी));
}

#घोषणा GET_पंक्ति(fp) \
	((option_mask32 & FLAG_z) \
	? bb_get_chunk_from_file(fp, शून्य) \
	: xmalloc_fgetline(fp))
#अन्यथा
#घोषणा GET_पंक्ति(fp) xmalloc_fgetline(fp)
#पूर्ण_अगर

#अगर ENABLE_FEATURE_SORT_BIG
अटल पूर्णांक scale_suffix(स्थिर अक्षर *पुच्छ)
{
	अटल स्थिर अक्षर suffix[] ALIGN1 = "kmgtpezy";
	स्थिर अक्षर *s;
	पूर्णांक n;

	अगर (!पुच्छ[0])
		वापस -1;
	s = म_अक्षर(suffix, पुच्छ[0] | 0x20);
	अगर (!s)
		वापस -1;
	n = s - suffix;
	अगर (n != 0 && पुच्छ[0] >= 'a')
		वापस -1; /* mg... not accepted, only MG... */
	वापस n;
}
#पूर्ण_अगर

/* Iterate through keys list and perform comparisons */
अटल पूर्णांक compare_कुंजियाँ(स्थिर व्योम *xarg, स्थिर व्योम *yarg)
{
	पूर्णांक flags = option_mask32, retval = 0;
	अक्षर *x, *y;

#अगर ENABLE_FEATURE_SORT_BIG
	काष्ठा sort_कुंजी *कुंजी;

	क्रम (कुंजी = कुंजी_list; !retval && कुंजी; कुंजी = कुंजी->अग्र_कुंजी) {
		flags = कुंजी->flags ? कुंजी->flags : option_mask32;
		/* Chop out and modify key chunks, handling -dfib */
		x = get_कुंजी(*(अक्षर **)xarg, कुंजी, flags);
		y = get_कुंजी(*(अक्षर **)yarg, कुंजी, flags);
#अन्यथा
	/* This curly bracket serves no purpose but to match the nesting
	 * level of the for () loop we're not using */
	{
		x = *(अक्षर **)xarg;
		y = *(अक्षर **)yarg;
#पूर्ण_अगर
		/* Perform actual comparison */
		चयन (flags & (FLAG_n | FLAG_g | FLAG_h | FLAG_M | FLAG_V)) {
		शेष:
			bb_simple_error_संदेश_and_die("unknown sort type");
			अवरोध;
#अगर defined(HAVE_STRVERSCMP) && HAVE_STRVERSCMP == 1
		हाल FLAG_V:
			retval = strverscmp(x, y);
			अवरोध;
#पूर्ण_अगर
		/* Ascii sort */
		हाल 0:
#अगर ENABLE_LOCALE_SUPPORT
			retval = strcoll(x, y);
#अन्यथा
			retval = म_भेद(x, y);
#पूर्ण_अगर
			अवरोध;
#अगर ENABLE_FEATURE_SORT_BIG
		हाल FLAG_g:
		हाल FLAG_h: {
			अक्षर *xx, *yy;
//TODO: needs setlocale(LC_NUMERIC, "C")?
			द्विगुन dx = म_से_भग्न(x, &xx);
			द्विगुन dy = म_से_भग्न(y, &yy);
			/* not numbers < NaN < -infinity < numbers < +infinity) */
			अगर (x == xx)
				retval = (y == yy ? 0 : -1);
			अन्यथा अगर (y == yy)
				retval = 1;
			/* Check for isnan */
			अन्यथा अगर (dx != dx)
				retval = (dy != dy) ? 0 : -1;
			अन्यथा अगर (dy != dy)
				retval = 1;
			अन्यथा {
				अगर (flags & FLAG_h) {
					पूर्णांक xs = scale_suffix(xx);
					पूर्णांक ys = scale_suffix(yy);
					अगर (xs != ys) {
						retval = xs - ys;
						अवरोध;
					}
				}
				/* Check for infinity.  Could underflow, but it avoids libm. */
				अगर (1.0 / dx == 0.0) {
					अगर (dx < 0)
						retval = (1.0 / dy == 0.0 && dy < 0) ? 0 : -1;
					अन्यथा
						retval = (1.0 / dy == 0.0 && dy > 0) ? 0 : 1;
				} अन्यथा अगर (1.0 / dy == 0.0)
					retval = (dy < 0) ? 1 : -1;
				अन्यथा
					retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			}
			अवरोध;
		}
		हाल FLAG_M: {
			काष्ठा पंचांग thyme;
			पूर्णांक dx;
			अक्षर *xx, *yy;

			xx = strptime(skip_whitespace(x), "%b", &thyme);
			dx = thyme.tm_mon;
			yy = strptime(skip_whitespace(y), "%b", &thyme);
			अगर (!xx)
				retval = (!yy) ? 0 : -1;
			अन्यथा अगर (!yy)
				retval = 1;
			अन्यथा
				retval = dx - thyme.tm_mon;
			अवरोध;
		}
		/* Full floating point version of -n */
		हाल FLAG_n: {
			द्विगुन dx = म_से_भ(x);
			द्विगुन dy = म_से_भ(y);
			retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			अवरोध;
		}
		} /* switch */
		/* Free key copies. */
		अगर (x != *(अक्षर **)xarg) मुक्त(x);
		अगर (y != *(अक्षर **)yarg) मुक्त(y);
		/* if (retval) break; - done by for () anyway */
#अन्यथा
		/* Integer version of -n for tiny systems */
		हाल FLAG_n:
			retval = म_से_प(x) - म_से_प(y);
			अवरोध;
		} /* switch */
#पूर्ण_अगर
	} /* for */

	अगर (retval == 0) {
		/* So far lines are "the same" */

		अगर (option_mask32 & FLAG_s) {
			/* "Stable sort": later line is "greater than",
			 * IOW: do not allow qsort() to swap equal lines.
			 */
			uint32_t *p32;
			uint32_t x32, y32;
			अक्षर *पंक्ति;
			अचिन्हित len;

			पंक्ति = *(अक्षर**)xarg;
			len = (म_माप(पंक्ति) + 4) & (~3u);
			p32 = (व्योम*)(पंक्ति + len);
			x32 = *p32;
			पंक्ति = *(अक्षर**)yarg;
			len = (म_माप(पंक्ति) + 4) & (~3u);
			p32 = (व्योम*)(पंक्ति + len);
			y32 = *p32;

			/* If x > y, 1, else -1 */
			retval = (x32 > y32) * 2 - 1;
			/* Here, -r has no effect! */
			वापस retval;
		}
		अगर (!(option_mask32 & FLAG_no_tie_break)) {
			/* fallback sort */
			flags = option_mask32;
			retval = म_भेद(*(अक्षर **)xarg, *(अक्षर **)yarg);
		}
	}

	अगर (flags & FLAG_r)
		वापस -retval;

	वापस retval;
}

#अगर ENABLE_FEATURE_SORT_BIG
अटल अचिन्हित str2u(अक्षर **str)
{
	अचिन्हित दीर्घ lu;
	अगर (!है_अंक((*str)[0]))
		bb_simple_error_संदेश_and_die("bad field specification");
	lu = म_से_अदीर्घ(*str, str, 10);
	अगर ((माप(दीर्घ) > माप(पूर्णांक) && lu > पूर्णांक_उच्च) || !lu)
		bb_simple_error_संदेश_and_die("bad field specification");
	वापस lu;
}
#पूर्ण_अगर

पूर्णांक sort_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक sort_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	अक्षर **पंक्तियाँ;
	अक्षर *str_ignored, *str_o, *str_t;
	llist_t *lst_k = शून्य;
	पूर्णांक i;
	पूर्णांक linecount;
	अचिन्हित opts;
#अगर ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
	bool can_drop_dups;
	माप_प्रकार पूर्व_len = 0;
	अक्षर *पूर्व_पंक्ति = (अक्षर*) "";
	/* Postpone optimizing if the input is small, < 16k lines:
	 * even just free()ing duplicate lines takes time.
	 */
	माप_प्रकार गिनती_to_optimize_dups = 0x3fff;
#पूर्ण_अगर

	xfunc_error_retval = 2;

	/* Parse command line options */
	opts = getopt32(argv,
			sort_opt_str,
			&str_ignored, &str_ignored, &str_o, &lst_k, &str_t
	);
#अगर ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
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
	अगर (opts & FLAG_s)
		गिनती_to_optimize_dups = (माप_प्रकार)-1L;
#पूर्ण_अगर
	/* global b strips leading and trailing spaces */
	अगर (opts & FLAG_b)
		option_mask32 |= FLAG_bb;
#अगर ENABLE_FEATURE_SORT_BIG
	अगर (opts & FLAG_t) {
		अगर (!str_t[0] || str_t[1])
			bb_simple_error_संदेश_and_die("bad -t parameter");
		कुंजी_separator = str_t[0];
	}
	/* note: below this point we use option_mask32, not opts,
	 * since that reduces register pressure and makes code smaller */

	/* Parse sort key */
	जबतक (lst_k) {
		क्रमागत {
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
		काष्ठा sort_कुंजी *कुंजी = add_कुंजी();
		अक्षर *str_k = llist_निकालो(&lst_k);

		i = 0; /* i==0 before comma, 1 after (-k3,6) */
		जबतक (*str_k) {
			/* Start of range */
			/* Cannot use bb_strtou - suffix can be a letter */
			कुंजी->range[2*i] = str2u(&str_k);
			अगर (*str_k == '.') {
				str_k++;
				कुंजी->range[2*i+1] = str2u(&str_k);
			}
			जबतक (*str_k) {
				पूर्णांक flag;
				स्थिर अक्षर *idx;

				अगर (*str_k == ',' && !i++) {
					str_k++;
					अवरोध;
				} /* no else needed: fall through to syntax error
					because comma isn't in OPT_STR */
				idx = म_अक्षर(OPT_STR, *str_k);
				अगर (!idx)
					bb_simple_error_संदेश_and_die("unknown key option");
				flag = 1 << (idx - OPT_STR);
				अगर (flag & ~FLAG_allowed_for_k)
					bb_simple_error_संदेश_and_die("unknown sort type");
				/* b after ',' means strip _trailing_ space */
				अगर (i && flag == FLAG_b)
					flag = FLAG_bb;
				कुंजी->flags |= flag;
				str_k++;
			}
		}
	}
#पूर्ण_अगर

	/* Open input files and read data */
	argv += optind;
	अगर (!*argv)
		*--argv = (अक्षर*)"-";
	linecount = 0;
	पंक्तियाँ = शून्य;
	करो {
		/* coreutils 6.9 compat: abort on first open error,
		 * do not continue to next file: */
		खाता *fp = xfopen_stdin(*argv);
		क्रम (;;) {
			अक्षर *पंक्ति = GET_पंक्ति(fp);
			अगर (!पंक्ति)
				अवरोध;

#अगर ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
			अगर (गिनती_to_optimize_dups != 0)
				गिनती_to_optimize_dups--;
			अगर (गिनती_to_optimize_dups == 0) {
				माप_प्रकार len;
				अक्षर *new_पंक्ति;

				/* On kernel/linux/arch/ *.[ch] files,
				 * this reduces memory usage by 6%.
				 *  yes | head -99999999 | sort
				 * goes down from 1900Mb to 380 Mb.
				 */
				len = म_माप(पंक्ति);
				अगर (len <= पूर्व_len) {
					new_पंक्ति = पूर्व_पंक्ति + (पूर्व_len - len);
					अगर (म_भेद(पंक्ति, new_पंक्ति) == 0) {
						/* it's a tail of the prev line */
						अगर (can_drop_dups && पूर्व_len == len) {
							/* it's identical to prev line */
							मुक्त(पंक्ति);
							जारी;
						}
						मुक्त(पंक्ति);
						पंक्ति = new_पंक्ति;
						/* continue using longer prev_line
						 * for future tail tests.
						 */
						जाओ skip;
					}
				}
				पूर्व_len = len;
				पूर्व_पंक्ति = पंक्ति;
 skip: ;
			}
#अन्यथा
//TODO: lighter version which only drops total dups if can_drop_dups == true
#पूर्ण_अगर
			पंक्तियाँ = xrealloc_vector(पंक्तियाँ, 6, linecount);
			पंक्तियाँ[linecount++] = पंक्ति;
		}
		fclose_if_not_stdin(fp);
	} जबतक (*++argv);

#अगर ENABLE_FEATURE_SORT_BIG
	/* If no key, perform alphabetic sort */
	अगर (!कुंजी_list)
		add_कुंजी()->range[0] = 1;
	/* Handle -c */
	अगर (option_mask32 & FLAG_c) {
		पूर्णांक j = (option_mask32 & FLAG_u) ? -1 : 0;
		क्रम (i = 1; i < linecount; i++) {
			अगर (compare_कुंजियाँ(&पंक्तियाँ[i-1], &पंक्तियाँ[i]) > j) {
				ख_लिखो(मानक_त्रुटि, "Check line %u\n", i);
				वापस निकास_त्रुटि;
			}
		}
		वापस निकास_सफल;
	}
#पूर्ण_अगर

	/* For stable sort, store original line position beyond terminating NUL */
	अगर (option_mask32 & FLAG_s) {
		क्रम (i = 0; i < linecount; i++) {
			uint32_t *p32;
			अक्षर *पंक्ति;
			अचिन्हित len;

			पंक्ति = पंक्तियाँ[i];
			len = (म_माप(पंक्ति) + 4) & (~3u);
			पंक्तियाँ[i] = पंक्ति = xrealloc(पंक्ति, len + 4);
			p32 = (व्योम*)(पंक्ति + len);
			*p32 = i;
		}
		/*option_mask32 |= FLAG_no_tie_break;*/
		/* ^^^redundant: if FLAG_s, compare_keys() does no tie break */
	}

	/* Perform the actual sort */
	क्विक(पंक्तियाँ, linecount, माप(पंक्तियाँ[0]), compare_कुंजियाँ);

	/* Handle -u */
	अगर (option_mask32 & FLAG_u) {
		पूर्णांक j = 0;
		/* coreutils 6.3 drop lines for which only key is the same:
		 * - disabling last-resort compare, or else compare_keys()
		 * will be the same only for completely identical lines
		 * - disabling -s (same reasons)
		 */
		option_mask32 = (option_mask32 | FLAG_no_tie_break) & (~FLAG_s);
		क्रम (i = 1; i < linecount; i++) {
			अगर (compare_कुंजियाँ(&पंक्तियाँ[j], &पंक्तियाँ[i]) == 0)
				मुक्त(पंक्तियाँ[i]);
			अन्यथा
				पंक्तियाँ[++j] = पंक्तियाँ[i];
		}
		अगर (linecount)
			linecount = j+1;
	}

	/* Print it */
#अगर ENABLE_FEATURE_SORT_BIG
	/* Open output file _after_ we read all input ones */
	अगर (option_mask32 & FLAG_o)
		xmove_fd(xopen(str_o, O_WRONLY|O_CREAT|O_TRUNC), STDOUT_FILENO);
#पूर्ण_अगर
	{
		पूर्णांक ch = (option_mask32 & FLAG_z) ? '\0' : '\n';
		क्रम (i = 0; i < linecount; i++)
			म_लिखो("%s%c", पंक्तियाँ[i], ch);
	}

	fflush_stdout_and_exit_SUCCESS();
}
