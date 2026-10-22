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
	FLAG_r  = 1 << 10,      /*سرچپه*/
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

ساكن ثابت حرف ترتیبول_opt_str[] ALIGN1 = "^"
			"nghMVucszbrdfimS:T:o:k:*t:"
			"\0" "o--o:t--t"/*-t, -o: at most one of each*/;
/*
 * OPT_STR must not be string literal, needs to have stable address:
 * code uses "strchr(OPT_STR,c) - OPT_STR" idiom.
 */
#تعریف_کړه OPT_STR (ترتیبول_opt_str + 1)

#إذا ENABLE_FEATURE_ترتیبول_BIG
ساكن حرف کلی_separator;

ساكن بنية ترتیبول_کلی {
	بنية ترتیبول_کلی *بل_کلی;  /* linked list */
	بینښه range[4];          /* start word, start char, end word, end char */
	بینښه flags;
} *کلی_list;


/* This is a NOEXEC applet. Be very careful! */


ساكن حرف *get_کلی(حرف *str, بنية ترتیبول_کلی *کلی, صحيح flags)
{
	صحيح پیل = پیل; /* for compiler */
	صحيح پای_ټکی;
	صحيح len, j;
	بینښه i;

	/* Special case whole string, so we don't have to make a copy */
	إذا (کلی->range[0] == 1 && !کلی->range[1] && !کلی->range[2] && !کلی->range[3]
	 && !(flags & (FLAG_b | FLAG_d | FLAG_f | FLAG_i | FLAG_bb))
	) {
		أرجع str;
	}

	/* Find start of key on first pass, end on second pass */
	len = مزي_اوږدوالی(str);
	لكل (j = 0; j < 2; j++) {
		إذا (!کلی->range[2*j])
			پای_ټکی = len;
		/* Loop through fields */
		وإلا {
			بینښه حرف ch = 0;

			پای_ټکی = 0;
			لكل (i = 1; i < کلی->range[2*j] + j; i++) {
				إذا (کلی_separator) {
					/* Skip body of key and separator */
					طالما ((ch = str[پای_ټکی]) != '\0') {
							پای_ټکی++;
						إذا (ch == کلی_separator)
							اكسر;
					}
				} وإلا {
					/* Skip leading blanks */
					طالما (که_تشه(str[پای_ټکی]))
						پای_ټکی++;
					/* Skip body of key */
					طالما (str[پای_ټکی] != '\0') {
						إذا (که_تشه(str[پای_ټکی]))
							اكسر;
						پای_ټکی++;
					}
				}
			}
			/* Remove last delim: "abc:def:" => "abc:def" */
			إذا (j && ch) {
				//if (str[end-1] != key_separator)
				//  bb_error_msg(_and_die("BUG! "
				//  "str[start:%d,end:%d]:'%.*s'",
				//  start, end, (int)(end-start), &str[start]);
				پای_ټکی--;
			}
		}
		إذا (!j) پیل = پای_ټکی;
	}
	/* Strip leading whitespace if necessary */
	إذا (flags & FLAG_b)
		/* not using skip_whitespace() for speed */
		طالما (که_تشه(str[پیل])) پیل++;
	/* Strip trailing whitespace if necessary */
	إذا (flags & FLAG_bb)
		طالما (پای_ټکی > پیل && که_تشه(str[پای_ټکی-1])) پای_ټکی--;
	/* -kSTART,N.ENDCHAR: honor ENDCHAR (1-based) */
	إذا (کلی->range[3]) {
		پای_ټکی = کلی->range[3];
		إذا (پای_ټکی > len) پای_ټکی = len;
	}
	/* -kN.STARTCHAR[,...]: honor STARTCHAR (1-based) */
	إذا (کلی->range[1]) {
		پیل += کلی->range[1] - 1;
		إذا (پیل > len) پیل = len;
	}
	/* Make the copy */
	إذا (پای_ټکی < پیل)
		پای_ټکی = پیل;
	str = xstrndup(str+پیل, پای_ټکی-پیل);
	/* Handle -d */
	إذا (flags & FLAG_d) {
		لكل (پیل = پای_ټکی = 0; str[پای_ټکی]; پای_ټکی++)
			إذا (که_تشه(str[پای_ټکی]) || که_توری_یا_عدد(str[پای_ټکی]))
				str[پیل++] = str[پای_ټکی];
		str[پیل] = '\0';
	}
	/* Handle -i */
	إذا (flags & FLAG_i) {
		لكل (پیل = پای_ټکی = 0; str[پای_ټکی]; پای_ټکی++)
			إذا (isprint_asciionly(str[پای_ټکی]))
				str[پیل++] = str[پای_ټکی];
		str[پیل] = '\0';
	}
	/* Handle -f */
	إذا (flags & FLAG_f)
		لكل (i = 0; str[i]; i++)
			str[i] = لوی_توری(str[i]);

	أرجع str;
}

ساكن بنية ترتیبول_کلی *جمع_کړه_کلی(فراغ)
{
	بنية ترتیبول_کلی **pkey = &کلی_list;
	طالما (*pkey)
		pkey = &((*pkey)->بل_کلی);
	أرجع *pkey = xzalloc(حجم(بنية ترتیبول_کلی));
}

#تعریف_کړه GET_کرښه(fp) \
	((option_mask32 & FLAG_z) \
	? bb_get_chunk_from_file(fp, عدم) \
	: xmalloc_fgetline(fp))
#وإلا
#تعریف_کړه GET_کرښه(fp) xmalloc_fgetline(fp)
#د_که_پای

#إذا ENABLE_FEATURE_ترتیبول_BIG
ساكن صحيح scale_suffix(ثابت حرف *لکۍ)
{
	ساكن ثابت حرف suffix[] ALIGN1 = "kmgtpezy";
	ثابت حرف *s;
	صحيح n;

	إذا (!لکۍ[0])
		أرجع -1;
	s = strchr(suffix, لکۍ[0] | 0x20);
	إذا (!s)
		أرجع -1;
	n = s - suffix;
	إذا (n != 0 && لکۍ[0] >= 'a')
		أرجع -1; /* mg... not accepted, only MG... */
	أرجع n;
}
#د_که_پای

/* Iterate through keys list and perform comparisons */
ساكن صحيح compare_کلیګانې(ثابت فراغ *xarg, ثابت فراغ *yarg)
{
	صحيح flags = option_mask32, retval = 0;
	حرف *x, *y;

#إذا ENABLE_FEATURE_ترتیبول_BIG
	بنية ترتیبول_کلی *کلی;

	لكل (کلی = کلی_list; !retval && کلی; کلی = کلی->بل_کلی) {
		flags = کلی->flags ? کلی->flags : option_mask32;
		/* Chop out and modify key chunks, handling -dfib */
		x = get_کلی(*(حرف **)xarg, کلی, flags);
		y = get_کلی(*(حرف **)yarg, کلی, flags);
#وإلا
	/* This curly bracket serves no purpose but to match the nesting
	 * level of the for () loop we're not using */
	{
		x = *(حرف **)xarg;
		y = *(حرف **)yarg;
#د_که_پای
		/* Perform actual comparison */
		بدل (flags & (FLAG_n | FLAG_g | FLAG_h | FLAG_M | FLAG_V)) {
		افتراضي:
			bb_simple_error_پیغام_and_die("unknown sort type");
			اكسر;
#إذا defined(HAVE_STRVERSCMP) && HAVE_STRVERSCMP == 1
		حالة FLAG_V:
			retval = strverscmp(x, y);
			اكسر;
#د_که_پای
		/* Ascii sort */
		حالة 0:
#إذا ENABLE_LOCALE_SUPPORT
			retval = strcoll(x, y);
#وإلا
			retval = strcmp(x, y);
#د_که_پای
			اكسر;
#إذا ENABLE_FEATURE_ترتیبول_BIG
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
#د_که_پای
	} /* for */

	إذا (retval == 0) {
		/* So far lines are "the same" */

		إذا (option_mask32 & FLAG_s) {
			/* "Stable sort": later line is "greater than",
			 * IOW: do not allow qsort() to swap equal lines.
			 */
			uint32_t *p32;
			uint32_t x32, y32;
			حرف *کرښه;
			بینښه len;

			کرښه = *(حرف**)xarg;
			len = (مزي_اوږدوالی(کرښه) + 4) & (~3u);
			p32 = (فراغ*)(کرښه + len);
			x32 = *p32;
			کرښه = *(حرف**)yarg;
			len = (مزي_اوږدوالی(کرښه) + 4) & (~3u);
			p32 = (فراغ*)(کرښه + len);
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

#إذا ENABLE_FEATURE_ترتیبول_BIG
ساكن بینښه str2u(حرف **str)
{
	بینښه اوږد lu;
	إذا (!که_عدد((*str)[0]))
		bb_simple_error_پیغام_and_die("bad field specification");
	lu = strtoul(*str, str, 10);
	إذا ((حجم(اوږد) > حجم(صحيح) && lu > لوی_عدد) || !lu)
		bb_simple_error_پیغام_and_die("bad field specification");
	أرجع lu;
}
#د_که_پای

صحيح ترتیبول_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح ترتیبول_main(صحيح argc ناکارول_شوی_PARAM, حرف **argv)
{
	حرف **کرښې;
	حرف *str_ignored, *str_o, *str_t;
	llist_t *lst_k = عدم;
	صحيح i;
	صحيح linecount;
	بینښه opts;
#إذا ENABLE_FEATURE_ترتیبول_OPTIMIZE_حافظه
	bool can_drop_dups;
	اندازه_t prev_len = 0;
	حرف *prev_کرښه = (حرف*) "";
	/* Postpone optimizing if the input is small, < 16k lines:
	 * even just free()ing duplicate lines takes time.
	 */
	اندازه_t شمېر_منزل_optimize_dups = 0x3fff;
#د_که_پای

	xfunc_error_retval = 2;

	/* Parse command line options */
	opts = getopt32(argv,
			ترتیبول_opt_str,
			&str_ignored, &str_ignored, &str_o, &lst_k, &str_t
	);
#إذا ENABLE_FEATURE_ترتیبول_OPTIMIZE_حافظه
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
		شمېر_منزل_optimize_dups = (اندازه_t)-1L;
#د_که_پای
	/* global b strips leading and trailing spaces */
	إذا (opts & FLAG_b)
		option_mask32 |= FLAG_bb;
#إذا ENABLE_FEATURE_ترتیبول_BIG
	إذا (opts & FLAG_t) {
		إذا (!str_t[0] || str_t[1])
			bb_simple_error_پیغام_and_die("bad -t parameter");
		کلی_separator = str_t[0];
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
				FLAG_r | /*سرچپه*/
				FLAG_d | /* Ignore !(isalnum()|isspace()) */
				FLAG_f | /* Force uppercase */
				FLAG_i | /* Ignore !isprint() */
			0
		};
		بنية ترتیبول_کلی *کلی = جمع_کړه_کلی();
		حرف *str_k = llist_راوباسه(&lst_k);

		i = 0; /* i==0 before comma, 1 after (-k3,6) */
		طالما (*str_k) {
			/* Start of range */
			/* Cannot use bb_strtou - suffix can be a letter */
			کلی->range[2*i] = str2u(&str_k);
			إذا (*str_k == '.') {
				str_k++;
				کلی->range[2*i+1] = str2u(&str_k);
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
					bb_simple_error_پیغام_and_die("unknown key option");
				flag = 1 << (idx - OPT_STR);
				إذا (flag & ~FLAG_allowed_for_k)
					bb_simple_error_پیغام_and_die("unknown sort type");
				/* b after ',' means strip _trailing_ space */
				إذا (i && flag == FLAG_b)
					flag = FLAG_bb;
				کلی->flags |= flag;
				str_k++;
			}
		}
	}
#د_که_پای

	/* Open input files and read data */
	argv += optind;
	إذا (!*argv)
		*--argv = (حرف*)"-";
	linecount = 0;
	کرښې = عدم;
	افعل {
		/* coreutils 6.9 compat: abort on first open error,
		 * do not continue to next file: */
		FILE *fp = xfopen_stdin(*argv);
		لكل (;;) {
			حرف *کرښه = GET_کرښه(fp);
			إذا (!کرښه)
				اكسر;

#إذا ENABLE_FEATURE_ترتیبول_OPTIMIZE_حافظه
			إذا (شمېر_منزل_optimize_dups != 0)
				شمېر_منزل_optimize_dups--;
			إذا (شمېر_منزل_optimize_dups == 0) {
				اندازه_t len;
				حرف *new_کرښه;

				/* On kernel/linux/arch/ *.[ch] files,
				 * this reduces memory usage by 6%.
				 *  yes | head -99999999 | sort
				 * goes down from 1900Mb to 380 Mb.
				 */
				len = مزي_اوږدوالی(کرښه);
				إذا (len <= prev_len) {
					new_کرښه = prev_کرښه + (prev_len - len);
					إذا (strcmp(کرښه, new_کرښه) == 0) {
						/* it's a tail of the prev line */
						إذا (can_drop_dups && prev_len == len) {
							/* it's identical to prev line */
							حرر(کرښه);
							استمر;
						}
						حرر(کرښه);
						کرښه = new_کرښه;
						/* continue using longer prev_line
						 * for future tail tests.
						 */
						اذهب skip;
					}
				}
				prev_len = len;
				prev_کرښه = کرښه;
 skip: ;
			}
#وإلا
//TODO: lighter version which only drops total dups if can_drop_dups == true
#د_که_پای
			کرښې = xrealloc_ویکټور(کرښې, 6, linecount);
			کرښې[linecount++] = کرښه;
		}
		fclose_if_not_stdin(fp);
	} طالما (*++argv);

#إذا ENABLE_FEATURE_ترتیبول_BIG
	/* If no key, perform alphabetic sort */
	إذا (!کلی_list)
		جمع_کړه_کلی()->range[0] = 1;
	/* Handle -c */
	إذا (option_mask32 & FLAG_c) {
		صحيح j = (option_mask32 & FLAG_u) ? -1 : 0;
		لكل (i = 1; i < linecount; i++) {
			إذا (compare_کلیګانې(&کرښې[i-1], &کرښې[i]) > j) {
				fprintf(stderr, "Check line %u\n", i);
				أرجع EXIT_FAILURE;
			}
		}
		أرجع EXIT_SUCCESS;
	}
#د_که_پای

	/* For stable sort, store original line position beyond terminating NUL */
	إذا (option_mask32 & FLAG_s) {
		لكل (i = 0; i < linecount; i++) {
			uint32_t *p32;
			حرف *کرښه;
			بینښه len;

			کرښه = کرښې[i];
			len = (مزي_اوږدوالی(کرښه) + 4) & (~3u);
			کرښې[i] = کرښه = xrealloc(کرښه, len + 4);
			p32 = (فراغ*)(کرښه + len);
			*p32 = i;
		}
		/*option_mask32 |= FLAG_no_tie_break;*/
		/* ^^^redundant: if FLAG_s, compare_keys() does no tie break */
	}

	/* Perform the actual sort */
	qsort(کرښې, linecount, حجم(کرښې[0]), compare_کلیګانې);

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
			إذا (compare_کلیګانې(&کرښې[j], &کرښې[i]) == 0)
				حرر(کرښې[i]);
			وإلا
				کرښې[++j] = کرښې[i];
		}
		إذا (linecount)
			linecount = j+1;
	}

	/* Print it */
#إذا ENABLE_FEATURE_ترتیبول_BIG
	/* Open output file _after_ we read all input ones */
	إذا (option_mask32 & FLAG_o)
		xmove_fd(xopen(str_o, O_WRONLY|O_CREAT|O_TRUNC), STDOUT_FILENO);
#د_که_پای
	{
		صحيح ch = (option_mask32 & FLAG_z) ? '\0' : '\n';
		لكل (i = 0; i < linecount; i++)
			چاپ_کړه("%s%c", کرښې[i], ch);
	}

	fflush_stdout_and_exit_SUCCESS();
}
