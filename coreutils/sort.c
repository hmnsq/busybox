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
enum {
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

static const char sort_opt_str[] ALIGN1 = "^"
			"nghMVucszbrdfimS:T:o:k:*t:"
			"\0" "o--o:t--t"/*-t, -o: at most one of each*/;
/*
 * OPT_STR must not be string literal, needs to have stable address:
 * code uses "strchr(OPT_STR,c) - OPT_STR" idiom.
 */
#define OPT_STR (sort_opt_str + 1)

#如果 ENABLE_FEATURE_SORT_BIG
static char key_separator;

static struct sort_key {
	struct sort_key *next_key;  /* linked list */
	unsigned range[4];          /* start word, start char, end word, end char */
	unsigned flags;
} *key_list;


/* This is a NOEXEC applet. Be very careful! */


static char *get_key(char *str, struct sort_key *key, 整数 flags)
{
	整数 start = start; /* for compiler */
	整数 end;
	整数 len, j;
	unsigned i;

	/* Special case whole string, so we don't have to make a copy */
	如果 (key->range[0] == 1 && !key->range[1] && !key->range[2] && !key->range[3]
	 && !(flags & (FLAG_b | FLAG_d | FLAG_f | FLAG_i | FLAG_bb))
	) {
		返回 str;
	}

	/* Find start of key on first pass, end on second pass */
	len = strlen(str);
	为 (j = 0; j < 2; j++) {
		如果 (!key->range[2*j])
			end = len;
		/* Loop through fields */
		否则 {
			unsigned char ch = 0;

			end = 0;
			为 (i = 1; i < key->range[2*j] + j; i++) {
				如果 (key_separator) {
					/* Skip body of key and separator */
					当 ((ch = str[end]) != '\0') {
							end++;
						如果 (ch == key_separator)
							break;
					}
				} 否则 {
					/* Skip leading blanks */
					当 (isspace(str[end]))
						end++;
					/* Skip body of key */
					当 (str[end] != '\0') {
						如果 (isspace(str[end]))
							break;
						end++;
					}
				}
			}
			/* Remove last delim: "abc:def:" => "abc:def" */
			如果 (j && ch) {
				//if (str[end-1] != key_separator)
				//  bb_error_msg(_and_die("BUG! "
				//  "str[start:%d,end:%d]:'%.*s'",
				//  start, end, (int)(end-start), &str[start]);
				end--;
			}
		}
		如果 (!j) start = end;
	}
	/* Strip leading whitespace if necessary */
	如果 (flags & FLAG_b)
		/* not using skip_whitespace() for speed */
		当 (isspace(str[start])) start++;
	/* Strip trailing whitespace if necessary */
	如果 (flags & FLAG_bb)
		当 (end > start && isspace(str[end-1])) end--;
	/* -kSTART,N.ENDCHAR: honor ENDCHAR (1-based) */
	如果 (key->range[3]) {
		end = key->range[3];
		如果 (end > len) end = len;
	}
	/* -kN.STARTCHAR[,...]: honor STARTCHAR (1-based) */
	如果 (key->range[1]) {
		start += key->range[1] - 1;
		如果 (start > len) start = len;
	}
	/* Make the copy */
	如果 (end < start)
		end = start;
	str = xstrndup(str+start, end-start);
	/* Handle -d */
	如果 (flags & FLAG_d) {
		为 (start = end = 0; str[end]; end++)
			如果 (isspace(str[end]) || isalnum(str[end]))
				str[start++] = str[end];
		str[start] = '\0';
	}
	/* Handle -i */
	如果 (flags & FLAG_i) {
		为 (start = end = 0; str[end]; end++)
			如果 (isprint_asciionly(str[end]))
				str[start++] = str[end];
		str[start] = '\0';
	}
	/* Handle -f */
	如果 (flags & FLAG_f)
		为 (i = 0; str[i]; i++)
			str[i] = toupper(str[i]);

	返回 str;
}

static struct sort_key *add_key(void)
{
	struct sort_key **pkey = &key_list;
	当 (*pkey)
		pkey = &((*pkey)->next_key);
	返回 *pkey = xzalloc(sizeof(struct sort_key));
}

#define GET_LINE(fp) \
	((option_mask32 & FLAG_z) \
	? bb_get_chunk_from_file(fp, NULL) \
	: xmalloc_fgetline(fp))
#否则
#define GET_LINE(fp) xmalloc_fgetline(fp)
#endif

#如果 ENABLE_FEATURE_SORT_BIG
static 整数 scale_suffix(const char *tail)
{
	static const char suffix[] ALIGN1 = "kmgtpezy";
	const char *s;
	整数 n;

	如果 (!tail[0])
		返回 -1;
	s = strchr(suffix, tail[0] | 0x20);
	如果 (!s)
		返回 -1;
	n = s - suffix;
	如果 (n != 0 && tail[0] >= 'a')
		返回 -1; /* mg... not accepted, only MG... */
	返回 n;
}
#endif

/* Iterate through keys list and perform comparisons */
static 整数 compare_keys(const void *xarg, const void *yarg)
{
	整数 flags = option_mask32, retval = 0;
	char *x, *y;

#如果 ENABLE_FEATURE_SORT_BIG
	struct sort_key *key;

	为 (key = key_list; !retval && key; key = key->next_key) {
		flags = key->flags ? key->flags : option_mask32;
		/* Chop out and modify key chunks, handling -dfib */
		x = get_key(*(char **)xarg, key, flags);
		y = get_key(*(char **)yarg, key, flags);
#否则
	/* This curly bracket serves no purpose but to match the nesting
	 * level of the for () loop we're not using */
	{
		x = *(char **)xarg;
		y = *(char **)yarg;
#endif
		/* Perform actual comparison */
		switch (flags & (FLAG_n | FLAG_g | FLAG_h | FLAG_M | FLAG_V)) {
		default:
			bb_simple_error_msg_and_die("unknown sort type");
			break;
#如果 defined(HAVE_STRVERSCMP) && HAVE_STRVERSCMP == 1
		case FLAG_V:
			retval = strverscmp(x, y);
			break;
#endif
		/* Ascii sort */
		case 0:
#如果 ENABLE_LOCALE_SUPPORT
			retval = strcoll(x, y);
#否则
			retval = strcmp(x, y);
#endif
			break;
#如果 ENABLE_FEATURE_SORT_BIG
		case FLAG_g:
		case FLAG_h: {
			char *xx, *yy;
//TODO: needs setlocale(LC_NUMERIC, "C")?
			double dx = strtod(x, &xx);
			double dy = strtod(y, &yy);
			/* not numbers < NaN < -infinity < numbers < +infinity) */
			如果 (x == xx)
				retval = (y == yy ? 0 : -1);
			否则 如果 (y == yy)
				retval = 1;
			/* Check for isnan */
			否则 如果 (dx != dx)
				retval = (dy != dy) ? 0 : -1;
			否则 如果 (dy != dy)
				retval = 1;
			否则 {
				如果 (flags & FLAG_h) {
					整数 xs = scale_suffix(xx);
					整数 ys = scale_suffix(yy);
					如果 (xs != ys) {
						retval = xs - ys;
						break;
					}
				}
				/* Check for infinity.  Could underflow, but it avoids libm. */
				如果 (1.0 / dx == 0.0) {
					如果 (dx < 0)
						retval = (1.0 / dy == 0.0 && dy < 0) ? 0 : -1;
					否则
						retval = (1.0 / dy == 0.0 && dy > 0) ? 0 : 1;
				} 否则 如果 (1.0 / dy == 0.0)
					retval = (dy < 0) ? 1 : -1;
				否则
					retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			}
			break;
		}
		case FLAG_M: {
			struct tm thyme;
			整数 dx;
			char *xx, *yy;

			xx = strptime(skip_whitespace(x), "%b", &thyme);
			dx = thyme.tm_mon;
			yy = strptime(skip_whitespace(y), "%b", &thyme);
			如果 (!xx)
				retval = (!yy) ? 0 : -1;
			否则 如果 (!yy)
				retval = 1;
			否则
				retval = dx - thyme.tm_mon;
			break;
		}
		/* Full floating point version of -n */
		case FLAG_n: {
			double dx = atof(x);
			double dy = atof(y);
			retval = (dx > dy) ? 1 : ((dx < dy) ? -1 : 0);
			break;
		}
		} /* switch */
		/* Free key copies. */
		如果 (x != *(char **)xarg) free(x);
		如果 (y != *(char **)yarg) free(y);
		/* if (retval) break; - done by for () anyway */
#否则
		/* Integer version of -n for tiny systems */
		case FLAG_n:
			retval = atoi(x) - atoi(y);
			break;
		} /* switch */
#endif
	} /* for */

	如果 (retval == 0) {
		/* So far lines are "the same" */

		如果 (option_mask32 & FLAG_s) {
			/* "Stable sort": later line is "greater than",
			 * IOW: do not allow qsort() to swap equal lines.
			 */
			uint32_t *p32;
			uint32_t x32, y32;
			char *line;
			unsigned len;

			line = *(char**)xarg;
			len = (strlen(line) + 4) & (~3u);
			p32 = (void*)(line + len);
			x32 = *p32;
			line = *(char**)yarg;
			len = (strlen(line) + 4) & (~3u);
			p32 = (void*)(line + len);
			y32 = *p32;

			/* If x > y, 1, else -1 */
			retval = (x32 > y32) * 2 - 1;
			/* Here, -r has no effect! */
			返回 retval;
		}
		如果 (!(option_mask32 & FLAG_no_tie_break)) {
			/* fallback sort */
			flags = option_mask32;
			retval = strcmp(*(char **)xarg, *(char **)yarg);
		}
	}

	如果 (flags & FLAG_r)
		返回 -retval;

	返回 retval;
}

#如果 ENABLE_FEATURE_SORT_BIG
static unsigned str2u(char **str)
{
	unsigned long lu;
	如果 (!isdigit((*str)[0]))
		bb_simple_error_msg_and_die("bad field specification");
	lu = strtoul(*str, str, 10);
	如果 ((sizeof(long) > sizeof(整数) && lu > INT_MAX) || !lu)
		bb_simple_error_msg_and_die("bad field specification");
	返回 lu;
}
#endif

整数 sort_main(整数 argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
整数 sort_main(整数 argc UNUSED_PARAM, char **argv)
{
	char **lines;
	char *str_ignored, *str_o, *str_t;
	llist_t *lst_k = NULL;
	整数 i;
	整数 linecount;
	unsigned opts;
#如果 ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
	bool can_drop_dups;
	size_t prev_len = 0;
	char *prev_line = (char*) "";
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
#如果 ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
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
	如果 (opts & FLAG_s)
		count_to_optimize_dups = (size_t)-1L;
#endif
	/* global b strips leading and trailing spaces */
	如果 (opts & FLAG_b)
		option_mask32 |= FLAG_bb;
#如果 ENABLE_FEATURE_SORT_BIG
	如果 (opts & FLAG_t) {
		如果 (!str_t[0] || str_t[1])
			bb_simple_error_msg_and_die("bad -t parameter");
		key_separator = str_t[0];
	}
	/* note: below this point we use option_mask32, not opts,
	 * since that reduces register pressure and makes code smaller */

	/* Parse sort key */
	当 (lst_k) {
		enum {
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
		struct sort_key *key = add_key();
		char *str_k = llist_pop(&lst_k);

		i = 0; /* i==0 before comma, 1 after (-k3,6) */
		当 (*str_k) {
			/* Start of range */
			/* Cannot use bb_strtou - suffix can be a letter */
			key->range[2*i] = str2u(&str_k);
			如果 (*str_k == '.') {
				str_k++;
				key->range[2*i+1] = str2u(&str_k);
			}
			当 (*str_k) {
				整数 flag;
				const char *idx;

				如果 (*str_k == ',' && !i++) {
					str_k++;
					break;
				} /* no else needed: fall through to syntax error
					because comma isn't in OPT_STR */
				idx = strchr(OPT_STR, *str_k);
				如果 (!idx)
					bb_simple_error_msg_and_die("unknown key option");
				flag = 1 << (idx - OPT_STR);
				如果 (flag & ~FLAG_allowed_for_k)
					bb_simple_error_msg_and_die("unknown sort type");
				/* b after ',' means strip _trailing_ space */
				如果 (i && flag == FLAG_b)
					flag = FLAG_bb;
				key->flags |= flag;
				str_k++;
			}
		}
	}
#endif

	/* Open input files and read data */
	argv += optind;
	如果 (!*argv)
		*--argv = (char*)"-";
	linecount = 0;
	lines = NULL;
	do {
		/* coreutils 6.9 compat: abort on first open error,
		 * do not continue to next file: */
		FILE *fp = xfopen_stdin(*argv);
		为 (;;) {
			char *line = GET_LINE(fp);
			如果 (!line)
				break;

#如果 ENABLE_FEATURE_SORT_OPTIMIZE_MEMORY
			如果 (count_to_optimize_dups != 0)
				count_to_optimize_dups--;
			如果 (count_to_optimize_dups == 0) {
				size_t len;
				char *new_line;

				/* On kernel/linux/arch/ *.[ch] files,
				 * this reduces memory usage by 6%.
				 *  yes | head -99999999 | sort
				 * goes down from 1900Mb to 380 Mb.
				 */
				len = strlen(line);
				如果 (len <= prev_len) {
					new_line = prev_line + (prev_len - len);
					如果 (strcmp(line, new_line) == 0) {
						/* it's a tail of the prev line */
						如果 (can_drop_dups && prev_len == len) {
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
#否则
//TODO: lighter version which only drops total dups if can_drop_dups == true
#endif
			lines = xrealloc_vector(lines, 6, linecount);
			lines[linecount++] = line;
		}
		fclose_if_not_stdin(fp);
	} 当 (*++argv);

#如果 ENABLE_FEATURE_SORT_BIG
	/* If no key, perform alphabetic sort */
	如果 (!key_list)
		add_key()->range[0] = 1;
	/* Handle -c */
	如果 (option_mask32 & FLAG_c) {
		整数 j = (option_mask32 & FLAG_u) ? -1 : 0;
		为 (i = 1; i < linecount; i++) {
			如果 (compare_keys(&lines[i-1], &lines[i]) > j) {
				fprintf(stderr, "Check line %u\n", i);
				返回 EXIT_FAILURE;
			}
		}
		返回 EXIT_SUCCESS;
	}
#endif

	/* For stable sort, store original line position beyond terminating NUL */
	如果 (option_mask32 & FLAG_s) {
		为 (i = 0; i < linecount; i++) {
			uint32_t *p32;
			char *line;
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
	qsort(lines, linecount, sizeof(lines[0]), compare_keys);

	/* Handle -u */
	如果 (option_mask32 & FLAG_u) {
		整数 j = 0;
		/* coreutils 6.3 drop lines for which only key is the same:
		 * - disabling last-resort compare, or else compare_keys()
		 * will be the same only for completely identical lines
		 * - disabling -s (same reasons)
		 */
		option_mask32 = (option_mask32 | FLAG_no_tie_break) & (~FLAG_s);
		为 (i = 1; i < linecount; i++) {
			如果 (compare_keys(&lines[j], &lines[i]) == 0)
				free(lines[i]);
			否则
				lines[++j] = lines[i];
		}
		如果 (linecount)
			linecount = j+1;
	}

	/* Print it */
#如果 ENABLE_FEATURE_SORT_BIG
	/* Open output file _after_ we read all input ones */
	如果 (option_mask32 & FLAG_o)
		xmove_fd(xopen(str_o, O_WRONLY|O_CREAT|O_TRUNC), STDOUT_FILENO);
#endif
	{
		整数 ch = (option_mask32 & FLAG_z) ? '\0' : '\n';
		为 (i = 0; i < linecount; i++)
			printf("%s%c", lines[i], ch);
	}

	fflush_stdout_and_exit_SUCCESS();
}
