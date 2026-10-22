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

#అన్తర్భావ "libbb.h"

గణన {
	ASCII = 256,
	/* string buffer needs to be at least as big as the whole "alphabet".
	 * BUFSIZ == ASCII is ok, but we will realloc in expand
	 * even for smallest patterns, let's avoid that by using *2:
	 */
	TR_BUFSIZ = (BUFSIZ > ASCII*2) ? BUFSIZ : ASCII*2,
};

స్థిర శూన్య map(అక్షరం *pvector,
		అక్షరం *string1, గుర్తులేని string1_len,
		అక్షరం *string2, గుర్తులేని string2_len)
{
	అక్షరం చివరి = '0';
	గుర్తులేని i, j;

	కొరకు (j = 0, i = 0; i < string1_len; i++) {
		ఒకవేళ (string2_len <= j)
			pvector[(గుర్తులేని అక్షరం)(string1[i])] = చివరి;
		లేకపోతే
			pvector[(గుర్తులేని అక్షరం)(string1[i])] = చివరి = string2[j++];
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
స్థిర గుర్తులేని expand(అక్షరం *arg, అక్షరం **బఫరు_p)
{
	అక్షరం *బఫరు = *బఫరు_p;
	గుర్తులేని pos = 0;
	గుర్తులేని కొలత = TR_BUFSIZ;
	గుర్తులేని i; /* can't be unsigned char: must be able to hold 256 */
	గుర్తులేని అక్షరం ac;

	అయితే (*arg) {
		ఒకవేళ (pos + ASCII > కొలత) {
			కొలత += ASCII;
			*బఫరు_p = బఫరు = xrealloc(బఫరు, కొలత);
		}
		ఒకవేళ (*arg == '\\') {
			స్థిరరాశి అక్షరం *z;
			arg++;
			z = arg;
			ac = bb_process_escape_sequence(&z);
			arg = (అక్షరం *)z;
			arg--;
			*arg = ac;
			/*
			 * fall through, there may be a range.
			 * If not, current char will be treated anyway.
			 */
		}
		ఒకవేళ (arg[1] == '-') { /* "0-9..." */
			ac = arg[2];
			ఒకవేళ (ac == '\0') { /* "0-": copy verbatim */
				బఫరు[pos++] = *arg++; /* copy '0' */
				కొనసాగు; /* next iter will copy '-' and stop */
			}
			i = (గుర్తులేని అక్షరం) *arg;
			arg += 3; /* skip 0-9 or 0-\ */
			ఒకవేళ (ac == '\\') {
				స్థిరరాశి అక్షరం *z;
				z = arg;
				ac = bb_process_escape_sequence(&z);
				arg = (అక్షరం *)z;
			}
			అయితే (i <= ac) /* ok: i is unsigned _int_ */
				బఫరు[pos++] = i++;
			కొనసాగు;
		}
		ఒకవేళ ((ENABLE_FEATURE_TR_CLASSES || ENABLE_FEATURE_TR_EQUIV)
		 && *arg == '['
		) {
			arg++;
			i = (గుర్తులేని అక్షరం) *arg++;
			/* "[xyz...". i=x, arg points to y */
			ఒకవేళ (ENABLE_FEATURE_TR_CLASSES && i == ':') { /* [:class:] */
#నిర్వచించు CLO ":]\0"
				స్థిర స్థిరరాశి అక్షరం classes[] ALIGN1 =
					"alpha"CLO "alnum"CLO "digit"CLO
					"lower"CLO "upper"CLO "space"CLO
					"blank"CLO "punct"CLO "cntrl"CLO
					"xdigit"CLO;
				గణన {
					CLASS_invalid = 0, /* we increment the retval */
					CLASS_alpha = 1,
					CLASS_alnum = 2,
					CLASS_digit = 3,
					CLASS_క్రింది = 4,
					CLASS_పై = 5,
					CLASS_space = 6,
					CLASS_blank = 7,
					CLASS_punct = 8,
					CLASS_cntrl = 9,
					CLASS_xdigit = 10,
					//CLASS_graph = 11,
					//CLASS_print = 12,
				};
				smalluint j;
				అక్షరం *tmp;

				/* xdigit needs 8, not 7 */
				i = 7 + (arg[0] == 'x');
				tmp = xstrndup(arg, i);
				j = సూచీ_లోపల_అక్షరశ్రేణులు(classes, tmp) + 1;
				విడుదల(tmp);

				ఒకవేళ (j == CLASS_invalid)
					వెళ్ళు skip_bracket;

				arg += i;
				ఒకవేళ (j == CLASS_alnum || j == CLASS_digit || j == CLASS_xdigit) {
					కొరకు (i = '0'; i <= '9'; i++)
						బఫరు[pos++] = i;
				}
				ఒకవేళ (j == CLASS_alpha || j == CLASS_alnum || j == CLASS_పై) {
					కొరకు (i = 'A'; i <= 'Z'; i++)
						బఫరు[pos++] = i;
				}
				ఒకవేళ (j == CLASS_alpha || j == CLASS_alnum || j == CLASS_క్రింది) {
					కొరకు (i = 'a'; i <= 'z'; i++)
						బఫరు[pos++] = i;
				}
				ఒకవేళ (j == CLASS_space || j == CLASS_blank) {
					బఫరు[pos++] = '\t';
					ఒకవేళ (j == CLASS_space) {
						బఫరు[pos++] = '\n';
						బఫరు[pos++] = '\v';
						బఫరు[pos++] = '\f';
						బఫరు[pos++] = '\r';
					}
					బఫరు[pos++] = ' ';
				}
				ఒకవేళ (j == CLASS_punct || j == CLASS_cntrl) {
					కొరకు (i = '\0'; i < ASCII; i++) {
						ఒకవేళ ((j == CLASS_punct && isprint_asciionly(i) && !అక్షరమా_అంకెనా(i) && !ఖాళీనా(i))
						 || (j == CLASS_cntrl && iscntrl(i))
						) {
							బఫరు[pos++] = i;
						}
					}
				}
				ఒకవేళ (j == CLASS_xdigit) {
					కొరకు (i = 'A'; i <= 'F'; i++) {
						బఫరు[pos + 6] = i | 0x20;
						బఫరు[pos++] = i;
					}
					pos += 6;
				}
				కొనసాగు;
			}
			/* "[xyz...", i=x, arg points to y */
			ఒకవేళ (ENABLE_FEATURE_TR_EQUIV && i == '=') { /* [=CHAR=] */
				బఫరు[pos++] = *arg; /* copy CHAR */
				ఒకవేళ (!arg[0] || arg[1] != '=' || arg[2] != ']')
					bb_show_usage();
				arg += 3;  /* skip CHAR=] */
				కొనసాగు;
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
		బఫరు[pos++] = *arg++;
	}
	తిరిగి pos;
}

/* NB: buffer is guaranteed to be at least TR_BUFSIZE
 * (which is >= ASCII) big.
 */
స్థిర పూర్ణాంకం complement(అక్షరం *బఫరు, పూర్ణాంకం బఫరు_len)
{
	పూర్ణాంకం len;
	అక్షరం conv[ASCII];
	గుర్తులేని అక్షరం ch;

	len = 0;
	ch = '\0';
	అయితే (1) {
		ఒకవేళ (memchr(బఫరు, ch, బఫరు_len) == ఖాళీ)
			conv[len++] = ch;
		ఒకవేళ (++ch == '\0')
			ఆపు;
	}
	జ్ఞాపకం_నకలు(బఫరు, conv, len);
	తిరిగి len;
}

పూర్ణాంకం tr_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం tr_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	పూర్ణాంకం i;
	smalluint opts;
	ssize_t read_chars;
	కొలత_t లోపల_సూచీ, బహిర్గతం_సూచీ;
	గుర్తులేని చివరి = UCHAR_MAX + 1; /* not equal to any char */
	గుర్తులేని అక్షరం coded, c;
	అక్షరం *str1 = xmalloc(TR_BUFSIZ);
	అక్షరం *str2 = xmalloc(TR_BUFSIZ);
	పూర్ణాంకం str2_నిడివి;
	పూర్ణాంకం str1_నిడివి;
	అక్షరం *సదిశ = xzalloc(ASCII * 3);
	అక్షరం *invec  = సదిశ + ASCII;
	అక్షరం *outvec = సదిశ + ASCII * 2;

#నిర్వచించు TR_OPT_complement   (3 << 0)
#నిర్వచించు TR_OPT_delete       (1 << 2)
#నిర్వచించు TR_OPT_squeeze_reps (1 << 3)

	కొరకు (i = 0; i < ASCII; i++) {
		సదిశ[i] = i;
		/*invec[i] = outvec[i] = FALSE; - done by xzalloc */
	}

	/* -C/-c difference is that -C complements "characters",
	 * and -c complements "values" (binary bytes I guess).
	 * In POSIX locale, these are the same.
	 */

	/* '+': stop at first non-option */
	opts = getopt32(argv, "^+" "Ccds" "\0" "-1:?2");
	argv += optind;

	str1_నిడివి = expand(*argv++, &str1);
	str2_నిడివి = 0;
	ఒకవేళ (opts & TR_OPT_complement)
		str1_నిడివి = complement(str1, str1_నిడివి);
	ఒకవేళ (*argv) {
		ఒకవేళ (argv[0][0] == '\0')
			bb_simple_error_సందేశం_and_die("STRING2 cannot be empty");
		str2_నిడివి = expand(*argv, &str2);
		map(సదిశ, str1, str1_నిడివి,
				str2, str2_నిడివి);
	}
	కొరకు (i = 0; i < str1_నిడివి; i++)
		invec[(గుర్తులేని అక్షరం)(str1[i])] = TRUE;
	కొరకు (i = 0; i < str2_నిడివి; i++)
		outvec[(గుర్తులేని అక్షరం)(str2[i])] = TRUE;

	వెళ్ళు ప్రారంభం_from;

	/* In this loop, str1 space is reused as input buffer,
	 * str2 - as output one. */
	కొరకు (;;) {
		/* If we're out of input, flush output and read more input. */
		ఒకవేళ ((ssize_t)లోపల_సూచీ == read_chars) {
			ఒకవేళ (బహిర్గతం_సూచీ) {
				xwrite(STDOUT_FILENO, str2, బహిర్గతం_సూచీ);
 ప్రారంభం_from:
				బహిర్గతం_సూచీ = 0;
			}
			read_chars = సురక్షిత_read(STDIN_FILENO, str1, TR_BUFSIZ);
			ఒకవేళ (read_chars <= 0) {
				ఒకవేళ (read_chars < 0)
					bb_simple_perror_సందేశం_and_die(bb_సందేశం_read_error);
				ఆపు;
			}
			లోపల_సూచీ = 0;
		}
		c = str1[లోపల_సూచీ++];
		ఒకవేళ ((opts & TR_OPT_delete) && invec[c])
			కొనసాగు;
		coded = సదిశ[c];
		ఒకవేళ ((opts & TR_OPT_squeeze_reps) && చివరి == coded
		 && (invec[c] || outvec[coded])
		) {
			కొనసాగు;
		}
		str2[బహిర్గతం_సూచీ++] = చివరి = coded;
	}

	ఒకవేళ (ENABLE_FEATURE_CLEAN_పైకి) {
		విడుదల(సదిశ);
		విడుదల(str2);
		విడుదల(str1);
	}

	తిరిగి EXIT_SUCCESS;
}
