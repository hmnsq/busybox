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
	FLAG_r  = 1 << 10,      /*معکوس*/
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

ساكن ثابت حرف مرتب_سازی_opt_str[] ALIGN1 = "^"
			"nghMVucszbrdfimS:T:o:k:*t:"
			"\0" "o--o:t--t"/*-t, -o: at most one of each*/;
/*
 * OPT_STR must not be string literal, needs to have stable address:
 * code uses "strchr(OPT_STR,c) - OPT_STR" idiom.
 */
#تعریف_کن OPT_STR (مرتب_سازی_opt_str + 1)

#إذا ENABLE_FEATURE_مرتب_سازی_BIG
ساكن حرف کلید_separator;

ساكن بنية مرتب_سازی_کلید {
	بنية مرتب_سازی_کلید *بعدی_کلید;  /* linked list */
	بیعلامت range[4];          /* start word, start char, end word, end char */
	بیعلامت flags;
} *کلید_list;


/* This is a NOEXEC applet. Be very careful! */


ساكن حرف *get_کلید(حرف *str, بنية مرتب_سازی_کلید *کلید, صحيح flags)
{
	صحيح شروع = شروع; /* for compiler */
	صحيح انتها;
	صحيح len, j;
	بیعلامت i;

	/* Special case whole string, so we don't have to make a copy */
	إذا (کلید->range[0] == 1 && !کلید->range[1] && !کلید->range[2] && !کلید->range[3]
	 && !(flags & (FLAG_b | FLAG_d | FLAG_f | FLAG_i | FLAG_bb))
	) {
		أرجع str;
	}

	/* Find start of key on first pass, end on second pass */
	len = طول_رشته(str);
	لكل (j = 0; j < 2; j++) {
		إذا (!کلید->range[2*j])
			انتها = len;
		/* Loop through fields */
		وإلا {
			بیعلامت حرف ch = 0;

			انتها = 0;
			لكل (i = 1; i < کلید->range[2*j] + j; i++) {
				إذا (کلید_separator) {
					/* Skip body of key and separator */
					طالما ((ch = str[انتها]) != '\0') {
							انتها++;
						إذا (ch == کلید_separator)
							اكسر;
					}
				} وإلا {
					/* Skip leading blanks */
					طالما (آیا_فاصله(str[انتها]))
						انتها++;
					/* Skip body of key */
					طالما (str[انتها] != '\0') {
						إذا (آیا_فاصله(str[انتها]))
							اكسر;
						انتها++;
					}
				}
			}
			/* Remove last delim: "abc:def:" => "abc:def" */
			إذا (j && ch) {
				//if (str[end-1] != key_separator)
				//  bb_error_msg(_and_die("BUG! "
				//  "str[start:%d,end:%d]:'%.*s'",
				//  start, end, (int)(end-start), &str[start]);
				انتها--;
			}
		}
		إذا (!j) شروع = انتها;
	}
	/* Strip leading whitespace if necessary */
	إذا (flags & FLAG_b)
		/* not using skip_whitespace() for speed */
		طالما (آیا_فاصله(str[شروع])) شروع++;
	/* Strip trailing whitespace if necessary */
	إذا (flags & FLAG_bb)
		طالما (انتها > شروع && آیا_فاصله(str[انتها-1])) انتها--;
	/* -kSTART,N.ENDCHAR: honor ENDCHAR (1-based) */
	إذا (کلید->range[3]) {
		انتها = کلید->range[3];
		إذا (انتها > len) انتها = len;
	}
	/* -kN.STARTCHAR[,...]: honor STARTCHAR (1-based) */
	إذا (کلید->range[1]) {
		شروع += کلید->range[1] - 1;
		إذا (شروع > len) شروع = len;
	}
	/* Make the copy */
	إذا (انتها < شروع)
		انتها = شروع;
	str = xstrndup(str+شروع, انتها-شروع);
	/* Handle -d */
	إذا (flags & FLAG_d) {
		لكل (شروع = انتها = 0; str[انتها]; انتها++)
			إذا (آیا_فاصله(str[انتها]) || آیا_حرف_یا_رقم(str[انتها]))
				str[شروع++] = str[انتها];
		str[شروع] = '\0';
	}
	/* Handle -i */
	إذا (flags & FLAG_i) {
		لكل (شروع = انتها = 0; str[انتها]; انتها++)
			إذا (isprint_asciionly(str[انتها]))
				str[شروع++] = str[انتها];
		str[شروع] = '\0';
	}
	/* Handle -f */
	إذا (flags & FLAG_f)
		لكل (i = 0; str[i]; i++)
			str[i] = به_بزرگ(str[i]);

	أرجع str;
}

ساكن بنية مرتب_سازی_کلید *جمع_کن_کلید(فراغ)
{
	بنية مرتب_سازی_کلید **pkey = &کلید_list;
	طالما (*pkey)
		pkey = &((*pkey)->بعدی_کلید);
	أرجع *pkey = xzalloc(حجم(بنية مرتب_سازی_کلید));
}

#تعریف_کن GET_سطر(fp) \
	((option_mask32 & FLAG_z) \
	? bb_get_chunk_from_file(fp, عدم) \
	: xmalloc_fgetline(fp))
#وإلا
#تعریف_کن GET_سطر(fp) xmalloc_fgetline(fp)
#پایان_اگر

#إذا ENABLE_FEATURE_مرتب_سازی_BIG
ساكن صحيح scale_suffix(ثابت حرف *دم)
{
	ساكن ثابت حرف suffix[] ALIGN1 = "kmgtpezy";
	ثابت حرف *s;
	صحيح n;

	إذا (!دم[0])
		أرجع -1;
	s = strchr(suffix, دم[0] | 0x20);
	إذا (!s)
		أرجع -1;
	n = s - suffix;
	إذا (n != 0 && دم[0] >= 'a')
		أرجع -1; /* mg... not accepted, only MG... */
	أرجع n;
}
#پایان_اگر

/* Iterate through keys list and perform comparisons */
ساكن صحيح compare_کلیدها(ثابت فراغ *xarg, ثابت فراغ *yarg)
{
	صحيح flags = option_mask32, retval = 0;
	حرف *x, *y;

#إذا ENABLE_FEATURE_مرتب_سازی_BIG
	بنية مرتب_سازی_کلید *کلید;

	لكل (کلید = کلید_list; !retval && کلید; کلید = کلید->بعدی_کلید) {
		flags = کلید->flags ? کلید->flags : option_mask32;
		/* Chop out and modify key chunks, handling -dfib */
		x = get_کلید(*(حرف **)xarg, کلید, flags);
		y = get_کلید(*(حرف **)yarg, کلید, flags);
#وإلا
	/* This curly bracket serves no purpose but to match the nesting
	 * level of the for () loop we're not using */
	{
		x = *(حرف **)xarg;
		y = *(حرف **)yarg;
#پایان_اگر
		/* Perform actual comparison */
		بدل (flags & (FLAG_n | FLAG_g | FLAG_h | FLAG_M | FLAG_V)) {
		افتراضي:
			bb_simple_error_پیام_and_die("unknown sort type");
			اكسر;
#إذا defined(HAVE_STRVERSCMP) && HAVE_STRVERSCMP == 1
		حالة FLAG_V:
			retval = strverscmp(x, y);
			اكسر;
#پایان_اگر
		/* Ascii sort */
		حالة 0:
#إذا ENABLE_LOCALE_SUPPORT
			retval = strcoll(x, y);
#وإلا
			retval = strcmp(x, y);
#پایان_اگر
			اكسر;
#إذا ENABLE_FEATURE_مرتب_سازی_BIG
		حالة FLAG_g:
		حالة FLAG_h: {
			حرف *xx, *yy;
//TODO: needs setlocale(LC_NUMERIC, "C")?
			مزدوج dx = strtod(x, &xx);
			مزدوج dy = strtod(y, &yy);
			/* not numbers < NaN < -infinity < numbers < +infinity) */
			إذا (x == xx)
				retval = (y == yy ? 0 : -1);
			وإلا إذا (y == yy)
				retval = 1;
			/* Check for isnan */
			وإلا إذا (dx != dx)
				retval = (dy != dy) ? 0 : -1;
			وإلا إذا (dy != dy)
				retval = 1;
			وإلا {
				إذا (flags & FLAG_h) {
					صحيح xs = scale_suffix(xx);
					صحيح ys = scale_suffix(yy);
					إذا (xs != ys) {
						retval = xs - ys;
						اكسر;
					}
				}
				/* Check for infinity.  Could underflow, but it avoids libm. */
				إذا (1.0 / dx == 0.0) {
					إذا (dx < 0)
						retval = (1.0 / dy == 0.0 && dy < 0) ? 0 : -1;
					وإلا
						retval = (1.0 / dy == 0.0 && dy > 0) ? 0 : 1;
				} وإلا إذا (1.0 / dy == 0.0)
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
			إذا (!xx)
				retval = (!yy) ? 0 : -1;
			وإلا إذا (!yy)
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
		إذا (x != *(حرف **)xarg) حرر(x);
		إذا (y != *(حرف **)yarg) حرر(y);
		/* if (retval) break; - done by for () anyway */
#وإلا
		/* Integer version of -n for tiny systems */
		حالة FLAG_n:
			retval = atoi(x) - atoi(y);
			اكسر;
		} /* switch */
#پایان_اگر
	} /* for */

	إذا (retval == 0) {
		/* So far lines are "the same" */

		إذا (option_mask32 & FLAG_s) {
			/* "Stable sort": later line is "greater than",
			 * IOW: do not allow qsort() to swap equal lines.
			 */
			uint32_t *p32;
			uint32_t x32, y32;
			حرف *سطر;
			بیعلامت len;

			سطر = *(حرف**)xarg;
			len = (طول_رشته(سطر) + 4) & (~3u);
			p32 = (فراغ*)(سطر + len);
			x32 = *p32;
			سطر = *(حرف**)yarg;
			len = (طول_رشته(سطر) + 4) & (~3u);
			p32 = (فراغ*)(سطر + len);
			y32 = *p32;

			/* If x > y, 1, else -1 */
			retval = (x32 > y32) * 2 - 1;
			/* Here, -r has no effect! */
			أرجع retval;
		}
		إذا (!(option_mask32 & FLAG_no_tie_break)) {
			/* fallback sort */
			flags = option_mask32;
			retval = strcmp(*(حرف **)xarg, *(حرف **)yarg);
		}
	}

	إذا (flags & FLAG_r)
		أرجع -retval;

	أرجع retval;
}

#إذا ENABLE_FEATURE_مرتب_سازی_BIG
ساكن بیعلامت str2u(حرف **str)
{
	بیعلامت دراز lu;
	إذا (!آیا_رقم((*str)[0]))
		bb_simple_error_پیام_and_die("bad field specification");
	lu = strtoul(*str, str, 10);
	إذا ((حجم(دراز) > حجم(صحيح) && lu > بیشترین_صحیح) || !lu)
		bb_simple_error_پیام_and_die("bad field specification");
	أرجع lu;
}
#پایان_اگر

صحيح مرتب_سازی_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح مرتب_سازی_main(صحيح argc استفادهنشده_PARAM, حرف **argv)
{
	حرف **سطرها;
	حرف *str_ignored, *str_o, *str_t;
	llist_t *lst_k = عدم;
	صحيح i;
	صحيح linecount;
	بیعلامت opts;
#إذا ENABLE_FEATURE_مرتب_سازی_OPTIMIZE_حافظه
	bool can_drop_dups;
	اندازه_t prev_len = 0;
	حرف *prev_سطر = (حرف*) "";
	/* Postpone optimizing if the input is small, < 16k lines:
	 * even just free()ing duplicate lines takes time.
	 */
	اندازه_t شمار_مقصد_optimize_dups = 0x3fff;
#پایان_اگر

	xfunc_error_retval = 2;

	/* Parse command line options */
	opts = getopt32(argv,
			مرتب_سازی_opt_str,
			&str_ignored, &str_ignored, &str_o, &lst_k, &str_t
	);
#إذا ENABLE_FEATURE_مرتب_سازی_OPTIMIZE_حافظه
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
	إذا (opts & FLAG_s)
		شمار_مقصد_optimize_dups = (اندازه_t)-1L;
#پایان_اگر
	/* global b strips leading and trailing spaces */
	إذا (opts & FLAG_b)
		option_mask32 |= FLAG_bb;
#إذا ENABLE_FEATURE_مرتب_سازی_BIG
	إذا (opts & FLAG_t) {
		إذا (!str_t[0] || str_t[1])
			bb_simple_error_پیام_and_die("bad -t parameter");
		کلید_separator = str_t[0];
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
				FLAG_r | /*معکوس*/
				FLAG_d | /* Ignore !(isalnum()|isspace()) */
				FLAG_f | /* Force uppercase */
				FLAG_i | /* Ignore !isprint() */
			0
		};
		بنية مرتب_سازی_کلید *کلید = جمع_کن_کلید();
		حرف *str_k = llist_برداشت(&lst_k);

		i = 0; /* i==0 before comma, 1 after (-k3,6) */
		طالما (*str_k) {
			/* Start of range */
			/* Cannot use bb_strtou - suffix can be a letter */
			کلید->range[2*i] = str2u(&str_k);
			إذا (*str_k == '.') {
				str_k++;
				کلید->range[2*i+1] = str2u(&str_k);
			}
			طالما (*str_k) {
				صحيح flag;
				ثابت حرف *idx;

				إذا (*str_k == ',' && !i++) {
					str_k++;
					اكسر;
				} /* no else needed: fall through to syntax error
					because comma isn't in OPT_STR */
				idx = strchr(OPT_STR, *str_k);
				إذا (!idx)
					bb_simple_error_پیام_and_die("unknown key option");
				flag = 1 << (idx - OPT_STR);
				إذا (flag & ~FLAG_allowed_for_k)
					bb_simple_error_پیام_and_die("unknown sort type");
				/* b after ',' means strip _trailing_ space */
				إذا (i && flag == FLAG_b)
					flag = FLAG_bb;
				کلید->flags |= flag;
				str_k++;
			}
		}
	}
#پایان_اگر

	/* Open input files and read data */
	argv += optind;
	إذا (!*argv)
		*--argv = (حرف*)"-";
	linecount = 0;
	سطرها = عدم;
	افعل {
		/* coreutils 6.9 compat: abort on first open error,
		 * do not continue to next file: */
		FILE *fp = xfopen_stdin(*argv);
		لكل (;;) {
			حرف *سطر = GET_سطر(fp);
			إذا (!سطر)
				اكسر;

#إذا ENABLE_FEATURE_مرتب_سازی_OPTIMIZE_حافظه
			إذا (شمار_مقصد_optimize_dups != 0)
				شمار_مقصد_optimize_dups--;
			إذا (شمار_مقصد_optimize_dups == 0) {
				اندازه_t len;
				حرف *new_سطر;

				/* On kernel/linux/arch/ *.[ch] files,
				 * this reduces memory usage by 6%.
				 *  yes | head -99999999 | sort
				 * goes down from 1900Mb to 380 Mb.
				 */
				len = طول_رشته(سطر);
				إذا (len <= prev_len) {
					new_سطر = prev_سطر + (prev_len - len);
					إذا (strcmp(سطر, new_سطر) == 0) {
						/* it's a tail of the prev line */
						إذا (can_drop_dups && prev_len == len) {
							/* it's identical to prev line */
							حرر(سطر);
							استمر;
						}
						حرر(سطر);
						سطر = new_سطر;
						/* continue using longer prev_line
						 * for future tail tests.
						 */
						اذهب skip;
					}
				}
				prev_len = len;
				prev_سطر = سطر;
 skip: ;
			}
#وإلا
//TODO: lighter version which only drops total dups if can_drop_dups == true
#پایان_اگر
			سطرها = xrealloc_وکتور(سطرها, 6, linecount);
			سطرها[linecount++] = سطر;
		}
		fclose_if_not_stdin(fp);
	} طالما (*++argv);

#إذا ENABLE_FEATURE_مرتب_سازی_BIG
	/* If no key, perform alphabetic sort */
	إذا (!کلید_list)
		جمع_کن_کلید()->range[0] = 1;
	/* Handle -c */
	إذا (option_mask32 & FLAG_c) {
		صحيح j = (option_mask32 & FLAG_u) ? -1 : 0;
		لكل (i = 1; i < linecount; i++) {
			إذا (compare_کلیدها(&سطرها[i-1], &سطرها[i]) > j) {
				fprintf(stderr, "Check line %u\n", i);
				أرجع EXIT_FAILURE;
			}
		}
		أرجع EXIT_SUCCESS;
	}
#پایان_اگر

	/* For stable sort, store original line position beyond terminating NUL */
	إذا (option_mask32 & FLAG_s) {
		لكل (i = 0; i < linecount; i++) {
			uint32_t *p32;
			حرف *سطر;
			بیعلامت len;

			سطر = سطرها[i];
			len = (طول_رشته(سطر) + 4) & (~3u);
			سطرها[i] = سطر = xrealloc(سطر, len + 4);
			p32 = (فراغ*)(سطر + len);
			*p32 = i;
		}
		/*option_mask32 |= FLAG_no_tie_break;*/
		/* ^^^redundant: if FLAG_s, compare_keys() does no tie break */
	}

	/* Perform the actual sort */
	qsort(سطرها, linecount, حجم(سطرها[0]), compare_کلیدها);

	/* Handle -u */
	إذا (option_mask32 & FLAG_u) {
		صحيح j = 0;
		/* coreutils 6.3 drop lines for which only key is the same:
		 * - disabling last-resort compare, or else compare_keys()
		 * will be the same only for completely identical lines
		 * - disabling -s (same reasons)
		 */
		option_mask32 = (option_mask32 | FLAG_no_tie_break) & (~FLAG_s);
		لكل (i = 1; i < linecount; i++) {
			إذا (compare_کلیدها(&سطرها[j], &سطرها[i]) == 0)
				حرر(سطرها[i]);
			وإلا
				سطرها[++j] = سطرها[i];
		}
		إذا (linecount)
			linecount = j+1;
	}

	/* Print it */
#إذا ENABLE_FEATURE_مرتب_سازی_BIG
	/* Open output file _after_ we read all input ones */
	إذا (option_mask32 & FLAG_o)
		xmove_fd(xopen(str_o, O_WRONLY|O_CREAT|O_TRUNC), STDOUT_FILENO);
#پایان_اگر
	{
		صحيح ch = (option_mask32 & FLAG_z) ? '\0' : '\n';
		لكل (i = 0; i < linecount; i++)
			چاپ_کن("%s%c", سطرها[i], ch);
	}

	fflush_stdout_and_exit_SUCCESS();
}
