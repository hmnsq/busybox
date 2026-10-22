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
شمار {
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
	FLAG_r  = 1 << 10,      /*الٹ*/
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

ساکن ثابت حرف ترتیب_دو_opt_str[] ALIGN1 = "^"
			"nghMVucszbrdfimS:T:o:k:*t:"
			"\0" "o--o:t--t"/*-t, -o: at most one of each*/;
/*
 * OPT_STR must not be string literal, needs to have stable address:
 * code uses "strchr(OPT_STR,c) - OPT_STR" idiom.
 */
#تعریف_کرو OPT_STR (ترتیب_دو_opt_str + 1)

#اگر ENABLE_FEATURE_ترتیب_دو_BIG
ساکن حرف کلید_separator;

ساکن ساخت ترتیب_دو_کلید {
	ساخت ترتیب_دو_کلید *اگلا_کلید;  /* linked list */
	غیرعلامتی range[4];          /* start word, start char, end word, end char */
	غیرعلامتی flags;
} *کلید_list;


/* This is a NOEXEC applet. Be very careful! */


ساکن حرف *get_کلید(حرف *str, ساخت ترتیب_دو_کلید *کلید, صحیح flags)
{
	صحیح آغاز = آغاز; /* for compiler */
	صحیح انتہا;
	صحیح len, j;
	غیرعلامتی i;

	/* Special case whole string, so we don't have to make a copy */
	اگر (کلید->range[0] == 1 && !کلید->range[1] && !کلید->range[2] && !کلید->range[3]
	 && !(flags & (FLAG_b | FLAG_d | FLAG_f | FLAG_i | FLAG_bb))
	) {
		واپس str;
	}

	/* Find start of key on first pass, end on second pass */
	len = لڑی_لمبائی(str);
	برائے (j = 0; j < 2; j++) {
		اگر (!کلید->range[2*j])
			انتہا = len;
		/* Loop through fields */
		ورنہ {
			غیرعلامتی حرف ch = 0;

			انتہا = 0;
			برائے (i = 1; i < کلید->range[2*j] + j; i++) {
				اگر (کلید_separator) {
					/* Skip body of key and separator */
					جب ((ch = str[انتہا]) != '\0') {
							انتہا++;
						اگر (ch == کلید_separator)
							توڑو;
					}
				} ورنہ {
					/* Skip leading blanks */
					جب (خلا_ہے(str[انتہا]))
						انتہا++;
					/* Skip body of key */
					جب (str[انتہا] != '\0') {
						اگر (خلا_ہے(str[انتہا]))
							توڑو;
						انتہا++;
					}
				}
			}
			/* Remove last delim: "abc:def:" => "abc:def" */
			اگر (j && ch) {
				//if (str[end-1] != key_separator)
				//  bb_error_msg(_and_die("BUG! "
				//  "str[start:%d,end:%d]:'%.*s'",
				//  start, end, (int)(end-start), &str[start]);
				انتہا--;
			}
		}
		اگر (!j) آغاز = انتہا;
	}
	/* Strip leading whitespace if necessary */
	اگر (flags & FLAG_b)
		/* not using skip_whitespace() for speed */
		جب (خلا_ہے(str[آغاز])) آغاز++;
	/* Strip trailing whitespace if necessary */
	اگر (flags & FLAG_bb)
		جب (انتہا > آغاز && خلا_ہے(str[انتہا-1])) انتہا--;
	/* -kSTART,N.ENDCHAR: honor ENDCHAR (1-based) */
	اگر (کلید->range[3]) {
		انتہا = کلید->range[3];
		اگر (انتہا > len) انتہا = len;
	}
	/* -kN.STARTCHAR[,...]: honor STARTCHAR (1-based) */
	اگر (کلید->range[1]) {
		آغاز += کلید->range[1] - 1;
		اگر (آغاز > len) آغاز = len;
	}
	/* Make the copy */
	اگر (انتہا < آغاز)
		انتہا = آغاز;
	str = xstrndup(str+آغاز, انتہا-آغاز);
	/* Handle -d */
	اگر (flags & FLAG_d) {
		برائے (آغاز = انتہا = 0; str[انتہا]; انتہا++)
			اگر (خلا_ہے(str[انتہا]) || حرف_یا_ہندسہ(str[انتہا]))
				str[آغاز++] = str[انتہا];
		str[آغاز] = '\0';
	}
	/* Handle -i */
	اگر (flags & FLAG_i) {
		برائے (آغاز = انتہا = 0; str[انتہا]; انتہا++)
			اگر (isprint_asciionly(str[انتہا]))
				str[آغاز++] = str[انتہا];
		str[آغاز] = '\0';
	}
	/* Handle -f */
	اگر (flags & FLAG_f)
		برائے (i = 0; str[i]; i++)
			str[i] = بڑا_حرف(str[i]);

	واپس str;
}

ساکن ساخت ترتیب_دو_کلید *جوڑو_کلید(خالی)
{
	ساخت ترتیب_دو_کلید **pkey = &کلید_list;
	جب (*pkey)
		pkey = &((*pkey)->اگلا_کلید);
	واپس *pkey = xzalloc(حجم(ساخت ترتیب_دو_کلید));
}

#تعریف_کرو GET_سطر(fp) \
	((option_mask32 & FLAG_z) \
	? bb_get_chunk_from_file(fp, تہی) \
	: xmalloc_fgetline(fp))
#ورنہ
#تعریف_کرو GET_سطر(fp) xmalloc_fgetline(fp)
#اگر_ختم

#اگر ENABLE_FEATURE_ترتیب_دو_BIG
ساکن صحیح scale_suffix(ثابت حرف *دم)
{
	ساکن ثابت حرف suffix[] ALIGN1 = "kmgtpezy";
	ثابت حرف *s;
	صحیح n;

	اگر (!دم[0])
		واپس -1;
	s = strchr(suffix, دم[0] | 0x20);
	اگر (!s)
		واپس -1;
	n = s - suffix;
	اگر (n != 0 && دم[0] >= 'a')
		واپس -1; /* mg... not accepted, only MG... */
	واپس n;
}
#اگر_ختم

/* Iterate through keys list and perform comparisons */
ساکن صحیح compare_کلیدیں(ثابت خالی *xarg, ثابت خالی *yarg)
{
	صحیح flags = option_mask32, retval = 0;
	حرف *x, *y;

#اگر ENABLE_FEATURE_ترتیب_دو_BIG
	ساخت ترتیب_دو_کلید *کلید;

	برائے (کلید = کلید_list; !retval && کلید; کلید = کلید->اگلا_کلید) {
		flags = کلید->flags ? کلید->flags : option_mask32;
		/* Chop out and modify key chunks, handling -dfib */
		x = get_کلید(*(حرف **)xarg, کلید, flags);
		y = get_کلید(*(حرف **)yarg, کلید, flags);
#ورنہ
	/* This curly bracket serves no purpose but to match the nesting
	 * level of the for () loop we're not using */
	{
		x = *(حرف **)xarg;
		y = *(حرف **)yarg;
#اگر_ختم
		/* Perform actual comparison */
		انتخاب (flags & (FLAG_n | FLAG_g | FLAG_h | FLAG_M | FLAG_V)) {
		طےشدہ:
			bb_simple_error_پیغام_and_die("unknown sort type");
			توڑو;
#اگر defined(HAVE_STRVERSCMP) && HAVE_STRVERSCMP == 1
		حالت FLAG_V:
			retval = strverscmp(x, y);
			توڑو;
#اگر_ختم
		/* Ascii sort */
		حالت 0:
#اگر ENABLE_LOCALE_SUPPORT
			retval = strcoll(x, y);
#ورنہ
			retval = strcmp(x, y);
#اگر_ختم
			توڑو;
#اگر ENABLE_FEATURE_ترتیب_دو_BIG
		حالت FLAG_g:
		حالت FLAG_h: {
			حرف *xx, *yy;
//TODO: needs setlocale(LC_NUMERIC, "C")?
			دوہرا dx = strtod(x, &xx);
			دوہرا dy = strtod(y, &yy);
			/* not numbers < NaN < -infinity < numbers < +infinity) */
			اگر (x == xx)
				retval = (y == yy ? 0 : -1);
			ورنہ اگر (y == yy)
				retval = 1;
			/* Check for isnan */
			ورنہ اگر (dx != dx)
				retval = (dy != dy) ? 0 : -1;
			ورنہ اگر (dy != dy)
				retval = 1;
			ورنہ {
				اگر (flags & FLAG_h) {
					صحیح xs = scale_suffix(xx);
					صحیح ys = scale_suffix(yy);
					اگر (xs != ys) {
						retval = xs - ys;
						توڑو;
					}
				}
				/* Check for infinity.  Could underflow, but it avoids libm. */
				اگر (1.0 / dx == 0.0) {
					اگر (dx < 0)
						retval = (1.0 / dy == 0.0 && dy < 0) ? 0 : -1;
					ورنہ
						retval = (1.0 / dy == 0.0 && dy > 0) ? 0 : 1;
				} ورنہ اگر (1.0 / dy == 0.0)
					retval = (dy < 0) ? 1 : -1;
				ورنہ
					retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			}
			توڑو;
		}
		حالت FLAG_M: {
			ساخت tm thyme;
			صحیح dx;
			حرف *xx, *yy;

			xx = strptime(skip_whitespace(x), "%b", &thyme);
			dx = thyme.tm_mon;
			yy = strptime(skip_whitespace(y), "%b", &thyme);
			اگر (!xx)
				retval = (!yy) ? 0 : -1;
			ورنہ اگر (!yy)
				retval = 1;
			ورنہ
				retval = dx - thyme.tm_mon;
			توڑو;
		}
		/* Full floating point version of -n */
		حالت FLAG_n: {
			دوہرا dx = atof(x);
			دوہرا dy = atof(y);
			retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			توڑو;
		}
		} /* switch */
		/* Free key copies. */
		اگر (x != *(حرف **)xarg) آزاد(x);
		اگر (y != *(حرف **)yarg) آزاد(y);
		/* if (retval) break; - done by for () anyway */
#ورنہ
		/* Integer version of -n for tiny systems */
		حالت FLAG_n:
			retval = atoi(x) - atoi(y);
			توڑو;
		} /* switch */
#اگر_ختم
	} /* for */

	اگر (retval == 0) {
		/* So far lines are "the same" */

		اگر (option_mask32 & FLAG_s) {
			/* "Stable sort": later line is "greater than",
			 * IOW: do not allow qsort() to swap equal lines.
			 */
			uint32_t *p32;
			uint32_t x32, y32;
			حرف *سطر;
			غیرعلامتی len;

			سطر = *(حرف**)xarg;
			len = (لڑی_لمبائی(سطر) + 4) & (~3u);
			p32 = (خالی*)(سطر + len);
			x32 = *p32;
			سطر = *(حرف**)yarg;
			len = (لڑی_لمبائی(سطر) + 4) & (~3u);
			p32 = (خالی*)(سطر + len);
			y32 = *p32;

			/* If x > y, 1, else -1 */
			retval = (x32 > y32) * 2 - 1;
			/* Here, -r has no effect! */
			واپس retval;
		}
		اگر (!(option_mask32 & FLAG_no_tie_break)) {
			/* fallback sort */
			flags = option_mask32;
			retval = strcmp(*(حرف **)xarg, *(حرف **)yarg);
		}
	}

	اگر (flags & FLAG_r)
		واپس -retval;

	واپس retval;
}

#اگر ENABLE_FEATURE_ترتیب_دو_BIG
ساکن غیرعلامتی str2u(حرف **str)
{
	غیرعلامتی طویل lu;
	اگر (!ہندسہ_ہے((*str)[0]))
		bb_simple_error_پیغام_and_die("bad field specification");
	lu = strtoul(*str, str, 10);
	اگر ((حجم(طویل) > حجم(صحیح) && lu > زیادہ_سے_زیادہ_عدد) || !lu)
		bb_simple_error_پیغام_and_die("bad field specification");
	واپس lu;
}
#اگر_ختم

صحیح ترتیب_دو_main(صحیح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح ترتیب_دو_main(صحیح argc غیر_مستعمل_PARAM, حرف **argv)
{
	حرف **سطریں;
	حرف *str_ignored, *str_o, *str_t;
	llist_t *lst_k = تہی;
	صحیح i;
	صحیح linecount;
	غیرعلامتی opts;
#اگر ENABLE_FEATURE_ترتیب_دو_OPTIMIZE_حافظہ
	bool can_drop_dups;
	سائز_t پچھلا_len = 0;
	حرف *پچھلا_سطر = (حرف*) "";
	/* Postpone optimizing if the input is small, < 16k lines:
	 * even just free()ing duplicate lines takes time.
	 */
	سائز_t گنتی_مقصد_optimize_dups = 0x3fff;
#اگر_ختم

	xfunc_error_retval = 2;

	/* Parse command line options */
	opts = getopt32(argv,
			ترتیب_دو_opt_str,
			&str_ignored, &str_ignored, &str_o, &lst_k, &str_t
	);
#اگر ENABLE_FEATURE_ترتیب_دو_OPTIMIZE_حافظہ
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
		گنتی_مقصد_optimize_dups = (سائز_t)-1L;
#اگر_ختم
	/* global b strips leading and trailing spaces */
	اگر (opts & FLAG_b)
		option_mask32 |= FLAG_bb;
#اگر ENABLE_FEATURE_ترتیب_دو_BIG
	اگر (opts & FLAG_t) {
		اگر (!str_t[0] || str_t[1])
			bb_simple_error_پیغام_and_die("bad -t parameter");
		کلید_separator = str_t[0];
	}
	/* note: below this point we use option_mask32, not opts,
	 * since that reduces register pressure and makes code smaller */

	/* Parse sort key */
	جب (lst_k) {
		شمار {
			FLAG_allowed_for_k =
				FLAG_n | /* Numeric sort */
				FLAG_g | /* Sort using strtod() */
				FLAG_h | /* Sort using strtod(), plus KMGT suffixes */
				FLAG_M | /* Sort date */
				FLAG_b | /* Ignore leading blanks */
				FLAG_r | /*الٹ*/
				FLAG_d | /* Ignore !(isalnum()|isspace()) */
				FLAG_f | /* Force uppercase */
				FLAG_i | /* Ignore !isprint() */
			0
		};
		ساخت ترتیب_دو_کلید *کلید = جوڑو_کلید();
		حرف *str_k = llist_نکالو(&lst_k);

		i = 0; /* i==0 before comma, 1 after (-k3,6) */
		جب (*str_k) {
			/* Start of range */
			/* Cannot use bb_strtou - suffix can be a letter */
			کلید->range[2*i] = str2u(&str_k);
			اگر (*str_k == '.') {
				str_k++;
				کلید->range[2*i+1] = str2u(&str_k);
			}
			جب (*str_k) {
				صحیح flag;
				ثابت حرف *idx;

				اگر (*str_k == ',' && !i++) {
					str_k++;
					توڑو;
				} /* no else needed: fall through to syntax error
					because comma isn't in OPT_STR */
				idx = strchr(OPT_STR, *str_k);
				اگر (!idx)
					bb_simple_error_پیغام_and_die("unknown key option");
				flag = 1 << (idx - OPT_STR);
				اگر (flag & ~FLAG_allowed_for_k)
					bb_simple_error_پیغام_and_die("unknown sort type");
				/* b after ',' means strip _trailing_ space */
				اگر (i && flag == FLAG_b)
					flag = FLAG_bb;
				کلید->flags |= flag;
				str_k++;
			}
		}
	}
#اگر_ختم

	/* Open input files and read data */
	argv += optind;
	اگر (!*argv)
		*--argv = (حرف*)"-";
	linecount = 0;
	سطریں = تہی;
	کرو {
		/* coreutils 6.9 compat: abort on first open error,
		 * do not continue to next file: */
		FILE *fp = xfopen_stdin(*argv);
		برائے (;;) {
			حرف *سطر = GET_سطر(fp);
			اگر (!سطر)
				توڑو;

#اگر ENABLE_FEATURE_ترتیب_دو_OPTIMIZE_حافظہ
			اگر (گنتی_مقصد_optimize_dups != 0)
				گنتی_مقصد_optimize_dups--;
			اگر (گنتی_مقصد_optimize_dups == 0) {
				سائز_t len;
				حرف *new_سطر;

				/* On kernel/linux/arch/ *.[ch] files,
				 * this reduces memory usage by 6%.
				 *  yes | head -99999999 | sort
				 * goes down from 1900Mb to 380 Mb.
				 */
				len = لڑی_لمبائی(سطر);
				اگر (len <= پچھلا_len) {
					new_سطر = پچھلا_سطر + (پچھلا_len - len);
					اگر (strcmp(سطر, new_سطر) == 0) {
						/* it's a tail of the prev line */
						اگر (can_drop_dups && پچھلا_len == len) {
							/* it's identical to prev line */
							آزاد(سطر);
							جاری;
						}
						آزاد(سطر);
						سطر = new_سطر;
						/* continue using longer prev_line
						 * for future tail tests.
						 */
						جاؤ skip;
					}
				}
				پچھلا_len = len;
				پچھلا_سطر = سطر;
 skip: ;
			}
#ورنہ
//TODO: lighter version which only drops total dups if can_drop_dups == true
#اگر_ختم
			سطریں = xrealloc_سمتیہ(سطریں, 6, linecount);
			سطریں[linecount++] = سطر;
		}
		fclose_if_not_stdin(fp);
	} جب (*++argv);

#اگر ENABLE_FEATURE_ترتیب_دو_BIG
	/* If no key, perform alphabetic sort */
	اگر (!کلید_list)
		جوڑو_کلید()->range[0] = 1;
	/* Handle -c */
	اگر (option_mask32 & FLAG_c) {
		صحیح j = (option_mask32 & FLAG_u) ? -1 : 0;
		برائے (i = 1; i < linecount; i++) {
			اگر (compare_کلیدیں(&سطریں[i-1], &سطریں[i]) > j) {
				fprintf(stderr, "Check line %u\n", i);
				واپس EXIT_FAILURE;
			}
		}
		واپس EXIT_SUCCESS;
	}
#اگر_ختم

	/* For stable sort, store original line position beyond terminating NUL */
	اگر (option_mask32 & FLAG_s) {
		برائے (i = 0; i < linecount; i++) {
			uint32_t *p32;
			حرف *سطر;
			غیرعلامتی len;

			سطر = سطریں[i];
			len = (لڑی_لمبائی(سطر) + 4) & (~3u);
			سطریں[i] = سطر = xrealloc(سطر, len + 4);
			p32 = (خالی*)(سطر + len);
			*p32 = i;
		}
		/*option_mask32 |= FLAG_no_tie_break;*/
		/* ^^^redundant: if FLAG_s, compare_keys() does no tie break */
	}

	/* Perform the actual sort */
	qsort(سطریں, linecount, حجم(سطریں[0]), compare_کلیدیں);

	/* Handle -u */
	اگر (option_mask32 & FLAG_u) {
		صحیح j = 0;
		/* coreutils 6.3 drop lines for which only key is the same:
		 * - disabling last-resort compare, or else compare_keys()
		 * will be the same only for completely identical lines
		 * - disabling -s (same reasons)
		 */
		option_mask32 = (option_mask32 | FLAG_no_tie_break) & (~FLAG_s);
		برائے (i = 1; i < linecount; i++) {
			اگر (compare_کلیدیں(&سطریں[j], &سطریں[i]) == 0)
				آزاد(سطریں[i]);
			ورنہ
				سطریں[++j] = سطریں[i];
		}
		اگر (linecount)
			linecount = j+1;
	}

	/* Print it */
#اگر ENABLE_FEATURE_ترتیب_دو_BIG
	/* Open output file _after_ we read all input ones */
	اگر (option_mask32 & FLAG_o)
		xmove_fd(xopen(str_o, O_WRONLY|O_CREAT|O_TRUNC), STDOUT_FILENO);
#اگر_ختم
	{
		صحیح ch = (option_mask32 & FLAG_z) ? '\0' : '\n';
		برائے (i = 0; i < linecount; i++)
			لکھو("%s%c", سطریں[i], ch);
	}

	fflush_stdout_and_exit_SUCCESS();
}
