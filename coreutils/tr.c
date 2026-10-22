/* vi: set sw=4 ts=4: */
/*
 * Mini tr implementation for busybox
 *
 * Copyright (c) 1987,1997, Prentice Hall   All rights reserved.
 *
 * The name of Prentice Hall may not be used to endorse or promote
 * products derived from this software without specific prior
 * written permission.
 *
 * Copyright (c) Michiel Huisjes
 *
 * This version of tr is adapted from Minix tr and was modified
 * by Erik Andersen <andersen@codepoet.org> to be used in busybox.
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
/* http://www.opengroup.org/onlinepubs/009695399/utilities/tr.html
 * TODO: graph, print
 */
//config:config TR
//config:	bool "tr (5.3 kb)"
//config:	default y
//config:	help
//config:	tr is used to squeeze, and/or delete characters from standard
//config:	input, writing to standard output.
//config:
//config:config FEATURE_TR_CLASSES
//config:	bool "Enable character classes (such as [:upper:])"
//config:	default y
//config:	depends on TR
//config:	help
//config:	Enable character classes, enabling commands such as:
//config:	tr [:upper:] [:lower:] to convert input into lowercase.
//config:
//config:config FEATURE_TR_EQUIV
//config:	bool "Enable equivalence classes"
//config:	default y
//config:	depends on TR
//config:	help
//config:	Enable equivalence classes, which essentially add the enclosed
//config:	character to the current set. For instance, tr [=a=] xyz would
//config:	replace all instances of 'a' with 'xyz'. This option is mainly
//config:	useful for cases when no other way of expressing a character
//config:	is possible.

//applet:IF_TR(APPLET(tr, BB_DIR_USR_BIN, BB_SUID_DROP))

//kbuild:lib-$(CONFIG_TR) += tr.o

//usage:#define tr_trivial_usage
//usage:       "[-cds] STRING1 [STRING2]"
//usage:#define tr_full_usage "\n\n"
//usage:       "Translate, squeeze, or delete characters from stdin, writing to stdout\n"
//usage:     "\n	-c	Take complement of STRING1"
//usage:     "\n	-d	Delete input characters coded STRING1"
//usage:     "\n	-s	Squeeze multiple output characters of STRING2 into one character"
//usage:
//usage:#define tr_example_usage
//usage:       "$ echo \"gdkkn vnqkc\" | tr [a-y] [b-z]\n"
//usage:       "hello world\n"

#অন্তর্ভাব "libbb.h"

গণনা {
	ASCII = 256,
	/* string buffer needs to be at least as big as the whole "alphabet".
	 * BUFSIZ == ASCII is ok, but we will realloc in expand
	 * even for smallest patterns, let's avoid that by using *2:
	 */
	TR_BUFSIZ = (BUFSIZ > ASCII*2) ? BUFSIZ : ASCII*2,
};

স্থির শূন্য map(অক্ষর *pvector,
		অক্ষর *string1, unsigned string1_len,
		অক্ষর *string2, unsigned string2_len)
{
	অক্ষর last = '0';
	unsigned i, j;

	জন্য (j = 0, i = 0; i < string1_len; i++) {
		যদি (string2_len <= j)
			pvector[(unsigned অক্ষর)(string1[i])] = last;
		অন্যথা
			pvector[(unsigned অক্ষর)(string1[i])] = last = string2[j++];
	}
}

/* supported constructs:
 *   Ranges,  e.g.,  0-9   ==>  0123456789
 *   Escapes, e.g.,  \a    ==>  Control-G
 *   Character classes, e.g. [:upper:] ==> A...Z
 *   Equiv classess, e.g. [=A=] ==> A   (hmmmmmmm?)
 * not supported:
 *   [x*N] - repeat char x N times
 *   [x*] - repeat char x until it fills STRING2:
 * # echo qwe123 | /usr/bin/tr 123456789 '[d]'
 * qwe[d]
 * # echo qwe123 | /usr/bin/tr 123456789 '[d*]'
 * qweddd
 */
স্থির unsigned expand(অক্ষর *arg, অক্ষর **বাফার_p)
{
	অক্ষর *বাফার = *বাফার_p;
	unsigned pos = 0;
	unsigned আকার_2 = TR_BUFSIZ;
	unsigned i; /* can't be unsigned char: must be able to hold 256 */
	unsigned অক্ষর ac;

	যখন (*arg) {
		যদি (pos + ASCII > আকার_2) {
			আকার_2 += ASCII;
			*বাফার_p = বাফার = xrealloc(বাফার, আকার_2);
		}
		যদি (*arg == '\\') {
			ধ্রুবক অক্ষর *z;
			arg++;
			z = arg;
			ac = bb_process_escape_sequence(&z);
			arg = (অক্ষর *)z;
			arg--;
			*arg = ac;
			/*
			 * fall through, there may be a range.
			 * If not, current char will be treated anyway.
			 */
		}
		যদি (arg[1] == '-') { /* "0-9..." */
			ac = arg[2];
			যদি (ac == '\0') { /* "0-": copy verbatim */
				বাফার[pos++] = *arg++; /* copy '0' */
				চালু; /* next iter will copy '-' and stop */
			}
			i = (unsigned অক্ষর) *arg;
			arg += 3; /* skip 0-9 or 0-\ */
			যদি (ac == '\\') {
				ধ্রুবক অক্ষর *z;
				z = arg;
				ac = bb_process_escape_sequence(&z);
				arg = (অক্ষর *)z;
			}
			যখন (i <= ac) /* ok: i is unsigned _int_ */
				বাফার[pos++] = i++;
			চালু;
		}
		যদি ((ENABLE_FEATURE_TR_CLASSES || ENABLE_FEATURE_TR_EQUIV)
		 && *arg == '['
		) {
			arg++;
			i = (unsigned অক্ষর) *arg++;
			/* "[xyz...". i=x, arg points to y */
			যদি (ENABLE_FEATURE_TR_CLASSES && i == ':') { /* [:class:] */
#define CLO ":]\0"
				স্থির ধ্রুবক অক্ষর classes[] ALIGN1 =
					"alpha"CLO "alnum"CLO "digit"CLO
					"lower"CLO "upper"CLO "space"CLO
					"blank"CLO "punct"CLO "cntrl"CLO
					"xdigit"CLO;
				গণনা {
					CLASS_invalid = 0, /* we increment the retval */
					CLASS_alpha = 1,
					CLASS_alnum = 2,
					CLASS_digit = 3,
					CLASS_lower = 4,
					CLASS_upper = 5,
					CLASS_space = 6,
					CLASS_blank = 7,
					CLASS_punct = 8,
					CLASS_cntrl = 9,
					CLASS_xdigit = 10,
					//CLASS_graph = 11,
					//CLASS_print = 12,
				};
				smalluint j;
				অক্ষর *tmp;

				/* xdigit needs 8, not 7 */
				i = 7 + (arg[0] == 'x');
				tmp = xstrndup(arg, i);
				j = সূচক_in_strings(classes, tmp) + 1;
				মুক্ত(tmp);

				যদি (j == CLASS_invalid)
					যাও skip_bracket;

				arg += i;
				যদি (j == CLASS_alnum || j == CLASS_digit || j == CLASS_xdigit) {
					জন্য (i = '0'; i <= '9'; i++)
						বাফার[pos++] = i;
				}
				যদি (j == CLASS_alpha || j == CLASS_alnum || j == CLASS_upper) {
					জন্য (i = 'A'; i <= 'Z'; i++)
						বাফার[pos++] = i;
				}
				যদি (j == CLASS_alpha || j == CLASS_alnum || j == CLASS_lower) {
					জন্য (i = 'a'; i <= 'z'; i++)
						বাফার[pos++] = i;
				}
				যদি (j == CLASS_space || j == CLASS_blank) {
					বাফার[pos++] = '\t';
					যদি (j == CLASS_space) {
						বাফার[pos++] = '\n';
						বাফার[pos++] = '\v';
						বাফার[pos++] = '\f';
						বাফার[pos++] = '\r';
					}
					বাফার[pos++] = ' ';
				}
				যদি (j == CLASS_punct || j == CLASS_cntrl) {
					জন্য (i = '\0'; i < ASCII; i++) {
						যদি ((j == CLASS_punct && isprint_asciionly(i) && !isalnum(i) && !isspace(i))
						 || (j == CLASS_cntrl && iscntrl(i))
						) {
							বাফার[pos++] = i;
						}
					}
				}
				যদি (j == CLASS_xdigit) {
					জন্য (i = 'A'; i <= 'F'; i++) {
						বাফার[pos + 6] = i | 0x20;
						বাফার[pos++] = i;
					}
					pos += 6;
				}
				চালু;
			}
			/* "[xyz...", i=x, arg points to y */
			যদি (ENABLE_FEATURE_TR_EQUIV && i == '=') { /* [=CHAR=] */
				বাফার[pos++] = *arg; /* copy CHAR */
				যদি (!arg[0] || arg[1] != '=' || arg[2] != ']')
					bb_show_usage();
				arg += 3;  /* skip CHAR=] */
				চালু;
			}
			/* The rest of "[xyz..." cases is treated as normal
			 * string, "[" has no special meaning here:
			 * tr "[a-z]" "[A-Z]" can be written as tr "a-z" "A-Z",
			 * also try tr "[a-z]" "_A-Z+" and you'll see that
			 * [] is not special here.
			 */
 skip_bracket:
			arg -= 2; /* points to "[" in "[xyz..." */
		}
		বাফার[pos++] = *arg++;
	}
	ফেরত pos;
}

/* NB: buffer is guaranteed to be at least TR_BUFSIZE
 * (which is >= ASCII) big.
 */
স্থির পূর্ণসংখ্যা complement(অক্ষর *বাফার, পূর্ণসংখ্যা বাফার_len)
{
	পূর্ণসংখ্যা len;
	অক্ষর conv[ASCII];
	unsigned অক্ষর ch;

	len = 0;
	ch = '\0';
	যখন (1) {
		যদি (memchr(বাফার, ch, বাফার_len) == ফাঁকা)
			conv[len++] = ch;
		যদি (++ch == '\0')
			বিরতি;
	}
	memcpy(বাফার, conv, len);
	ফেরত len;
}

পূর্ণসংখ্যা tr_main(পূর্ণসংখ্যা argc, অক্ষর **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণসংখ্যা tr_main(পূর্ণসংখ্যা argc UNUSED_PARAM, অক্ষর **argv)
{
	পূর্ণসংখ্যা i;
	smalluint opts;
	ssize_t read_chars;
	আকার_t in_সূচক, out_সূচক;
	unsigned last = UCHAR_MAX + 1; /* not equal to any char */
	unsigned অক্ষর coded, c;
	অক্ষর *str1 = xmalloc(TR_BUFSIZ);
	অক্ষর *str2 = xmalloc(TR_BUFSIZ);
	পূর্ণসংখ্যা str2_দৈর্ঘ্য;
	পূর্ণসংখ্যা str1_দৈর্ঘ্য;
	অক্ষর *vector = xzalloc(ASCII * 3);
	অক্ষর *invec  = vector + ASCII;
	অক্ষর *outvec = vector + ASCII * 2;

#define TR_OPT_complement   (3 << 0)
#define TR_OPT_delete       (1 << 2)
#define TR_OPT_squeeze_reps (1 << 3)

	জন্য (i = 0; i < ASCII; i++) {
		vector[i] = i;
		/*invec[i] = outvec[i] = FALSE; - done by xzalloc */
	}

	/* -C/-c difference is that -C complements "characters",
	 * and -c complements "values" (binary bytes I guess).
	 * In POSIX locale, these are the same.
	 */

	/* '+': stop at first non-option */
	opts = getopt32(argv, "^+" "Ccds" "\0" "-1:?2");
	argv += optind;

	str1_দৈর্ঘ্য = expand(*argv++, &str1);
	str2_দৈর্ঘ্য = 0;
	যদি (opts & TR_OPT_complement)
		str1_দৈর্ঘ্য = complement(str1, str1_দৈর্ঘ্য);
	যদি (*argv) {
		যদি (argv[0][0] == '\0')
			bb_simple_error_বার্তা_and_die("STRING2 cannot be empty");
		str2_দৈর্ঘ্য = expand(*argv, &str2);
		map(vector, str1, str1_দৈর্ঘ্য,
				str2, str2_দৈর্ঘ্য);
	}
	জন্য (i = 0; i < str1_দৈর্ঘ্য; i++)
		invec[(unsigned অক্ষর)(str1[i])] = TRUE;
	জন্য (i = 0; i < str2_দৈর্ঘ্য; i++)
		outvec[(unsigned অক্ষর)(str2[i])] = TRUE;

	যাও start_from;

	/* In this loop, str1 space is reused as input buffer,
	 * str2 - as output one. */
	জন্য (;;) {
		/* If we're out of input, flush output and read more input. */
		যদি ((ssize_t)in_সূচক == read_chars) {
			যদি (out_সূচক) {
				xwrite(STDOUT_FILENO, str2, out_সূচক);
 start_from:
				out_সূচক = 0;
			}
			read_chars = safe_read(STDIN_FILENO, str1, TR_BUFSIZ);
			যদি (read_chars <= 0) {
				যদি (read_chars < 0)
					bb_simple_perror_বার্তা_and_die(bb_বার্তা_read_error);
				বিরতি;
			}
			in_সূচক = 0;
		}
		c = str1[in_সূচক++];
		যদি ((opts & TR_OPT_delete) && invec[c])
			চালু;
		coded = vector[c];
		যদি ((opts & TR_OPT_squeeze_reps) && last == coded
		 && (invec[c] || outvec[coded])
		) {
			চালু;
		}
		str2[out_সূচক++] = last = coded;
	}

	যদি (ENABLE_FEATURE_CLEAN_UP) {
		মুক্ত(vector);
		মুক্ত(str2);
		মুক্ত(str1);
	}

	ফেরত EXIT_SUCCESS;
}
