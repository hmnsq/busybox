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

#अन्तर्भाव "libbb.h"

गणना {
	ASCII = 256,
	/* string buffer needs to be at least as big as the whole "alphabet".
	 * BUFSIZ == ASCII is ok, but we will realloc in expand
	 * even for smallest patterns, let's avoid that by using *2:
	 */
	TR_BUFSIZ = (BUFSIZ > ASCII*2) ? BUFSIZ : ASCII*2,
};

स्थिर शून्य map(वर्ण *pvector,
		वर्ण *string1, अचिह्नित string1_len,
		वर्ण *string2, अचिह्नित string2_len)
{
	वर्ण चरमः = '0';
	अचिह्नित i, j;

	प्रति (j = 0, i = 0; i < string1_len; i++) {
		यदि (string2_len <= j)
			pvector[(अचिह्नित वर्ण)(string1[i])] = चरमः;
		अन्यथा
			pvector[(अचिह्नित वर्ण)(string1[i])] = चरमः = string2[j++];
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
स्थिर अचिह्नित expand(वर्ण *arg, वर्ण **बफर_p)
{
	वर्ण *बफर = *बफर_p;
	अचिह्नित pos = 0;
	अचिह्नित आकार = TR_BUFSIZ;
	अचिह्नित i; /* can't be unsigned char: must be able to hold 256 */
	अचिह्नित वर्ण ac;

	यावत् (*arg) {
		यदि (pos + ASCII > आकार) {
			आकार += ASCII;
			*बफर_p = बफर = xrealloc(बफर, आकार);
		}
		यदि (*arg == '\\') {
			ध्रुव वर्ण *z;
			arg++;
			z = arg;
			ac = bb_process_escape_sequence(&z);
			arg = (वर्ण *)z;
			arg--;
			*arg = ac;
			/*
			 * fall through, there may be a range.
			 * If not, current char will be treated anyway.
			 */
		}
		यदि (arg[1] == '-') { /* "0-9..." */
			ac = arg[2];
			यदि (ac == '\0') { /* "0-": copy verbatim */
				बफर[pos++] = *arg++; /* copy '0' */
				अनुवर्त; /* next iter will copy '-' and stop */
			}
			i = (अचिह्नित वर्ण) *arg;
			arg += 3; /* skip 0-9 or 0-\ */
			यदि (ac == '\\') {
				ध्रुव वर्ण *z;
				z = arg;
				ac = bb_process_escape_sequence(&z);
				arg = (वर्ण *)z;
			}
			यावत् (i <= ac) /* ok: i is unsigned _int_ */
				बफर[pos++] = i++;
			अनुवर्त;
		}
		यदि ((ENABLE_FEATURE_TR_CLASSES || ENABLE_FEATURE_TR_EQUIV)
		 && *arg == '['
		) {
			arg++;
			i = (अचिह्नित वर्ण) *arg++;
			/* "[xyz...". i=x, arg points to y */
			यदि (ENABLE_FEATURE_TR_CLASSES && i == ':') { /* [:class:] */
#परिभाषय CLO ":]\0"
				स्थिर ध्रुव वर्ण classes[] ALIGN1 =
					"alpha"CLO "alnum"CLO "digit"CLO
					"lower"CLO "upper"CLO "space"CLO
					"blank"CLO "punct"CLO "cntrl"CLO
					"xdigit"CLO;
				गणना {
					CLASS_invalid = 0, /* we increment the retval */
					CLASS_alpha = 1,
					CLASS_alnum = 2,
					CLASS_digit = 3,
					CLASS_अधस्तनम् = 4,
					CLASS_ऊर्ध्वस्थम् = 5,
					CLASS_space = 6,
					CLASS_blank = 7,
					CLASS_punct = 8,
					CLASS_cntrl = 9,
					CLASS_xdigit = 10,
					//CLASS_graph = 11,
					//CLASS_print = 12,
				};
				smalluint j;
				वर्ण *tmp;

				/* xdigit needs 8, not 7 */
				i = 7 + (arg[0] == 'x');
				tmp = xstrndup(arg, i);
				j = सूचकांक_अन्तर्_वाक्यानि(classes, tmp) + 1;
				मुक्त(tmp);

				यदि (j == CLASS_invalid)
					गच्छ skip_bracket;

				arg += i;
				यदि (j == CLASS_alnum || j == CLASS_digit || j == CLASS_xdigit) {
					प्रति (i = '0'; i <= '9'; i++)
						बफर[pos++] = i;
				}
				यदि (j == CLASS_alpha || j == CLASS_alnum || j == CLASS_ऊर्ध्वस्थम्) {
					प्रति (i = 'A'; i <= 'Z'; i++)
						बफर[pos++] = i;
				}
				यदि (j == CLASS_alpha || j == CLASS_alnum || j == CLASS_अधस्तनम्) {
					प्रति (i = 'a'; i <= 'z'; i++)
						बफर[pos++] = i;
				}
				यदि (j == CLASS_space || j == CLASS_blank) {
					बफर[pos++] = '\t';
					यदि (j == CLASS_space) {
						बफर[pos++] = '\n';
						बफर[pos++] = '\v';
						बफर[pos++] = '\f';
						बफर[pos++] = '\r';
					}
					बफर[pos++] = ' ';
				}
				यदि (j == CLASS_punct || j == CLASS_cntrl) {
					प्रति (i = '\0'; i < ASCII; i++) {
						यदि ((j == CLASS_punct && isprint_asciionly(i) && !अक्षरम्_अङ्कः_वा(i) && !रिक्तम्_वा(i))
						 || (j == CLASS_cntrl && iscntrl(i))
						) {
							बफर[pos++] = i;
						}
					}
				}
				यदि (j == CLASS_xdigit) {
					प्रति (i = 'A'; i <= 'F'; i++) {
						बफर[pos + 6] = i | 0x20;
						बफर[pos++] = i;
					}
					pos += 6;
				}
				अनुवर्त;
			}
			/* "[xyz...", i=x, arg points to y */
			यदि (ENABLE_FEATURE_TR_EQUIV && i == '=') { /* [=CHAR=] */
				बफर[pos++] = *arg; /* copy CHAR */
				यदि (!arg[0] || arg[1] != '=' || arg[2] != ']')
					bb_show_usage();
				arg += 3;  /* skip CHAR=] */
				अनुवर्त;
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
		बफर[pos++] = *arg++;
	}
	प्रत्यागम pos;
}

/* NB: buffer is guaranteed to be at least TR_BUFSIZE
 * (which is >= ASCII) big.
 */
स्थिर पूर्णाङ्क complement(वर्ण *बफर, पूर्णाङ्क बफर_len)
{
	पूर्णाङ्क len;
	वर्ण conv[ASCII];
	अचिह्नित वर्ण ch;

	len = 0;
	ch = '\0';
	यावत् (1) {
		यदि (memchr(बफर, ch, बफर_len) == रिक्त)
			conv[len++] = ch;
		यदि (++ch == '\0')
			भङ्ग;
	}
	स्मृतिं_प्रतिलिख(बफर, conv, len);
	प्रत्यागम len;
}

पूर्णाङ्क tr_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क tr_main(पूर्णाङ्क argc अप्रयुक्तम्_PARAM, वर्ण **argv)
{
	पूर्णाङ्क i;
	smalluint opts;
	ssize_t read_chars;
	आकार_t अन्तर्_सूचकांक, बहिर्गमनम्_सूचकांक;
	अचिह्नित चरमः = UCHAR_MAX + 1; /* not equal to any char */
	अचिह्नित वर्ण coded, c;
	वर्ण *str1 = xmalloc(TR_BUFSIZ);
	वर्ण *str2 = xmalloc(TR_BUFSIZ);
	पूर्णाङ्क str2_दीर्घता;
	पूर्णाङ्क str1_दीर्घता;
	वर्ण *सदिशम् = xzalloc(ASCII * 3);
	वर्ण *invec  = सदिशम् + ASCII;
	वर्ण *outvec = सदिशम् + ASCII * 2;

#परिभाषय TR_OPT_complement   (3 << 0)
#परिभाषय TR_OPT_delete       (1 << 2)
#परिभाषय TR_OPT_squeeze_reps (1 << 3)

	प्रति (i = 0; i < ASCII; i++) {
		सदिशम्[i] = i;
		/*invec[i] = outvec[i] = FALSE; - done by xzalloc */
	}

	/* -C/-c difference is that -C complements "characters",
	 * and -c complements "values" (binary bytes I guess).
	 * In POSIX locale, these are the same.
	 */

	/* '+': stop at first non-option */
	opts = getopt32(argv, "^+" "Ccds" "\0" "-1:?2");
	argv += optind;

	str1_दीर्घता = expand(*argv++, &str1);
	str2_दीर्घता = 0;
	यदि (opts & TR_OPT_complement)
		str1_दीर्घता = complement(str1, str1_दीर्घता);
	यदि (*argv) {
		यदि (argv[0][0] == '\0')
			bb_simple_error_सन्देशः_and_die("STRING2 cannot be empty");
		str2_दीर्घता = expand(*argv, &str2);
		map(सदिशम्, str1, str1_दीर्घता,
				str2, str2_दीर्घता);
	}
	प्रति (i = 0; i < str1_दीर्घता; i++)
		invec[(अचिह्नित वर्ण)(str1[i])] = TRUE;
	प्रति (i = 0; i < str2_दीर्घता; i++)
		outvec[(अचिह्नित वर्ण)(str2[i])] = TRUE;

	गच्छ start_from;

	/* In this loop, str1 space is reused as input buffer,
	 * str2 - as output one. */
	प्रति (;;) {
		/* If we're out of input, flush output and read more input. */
		यदि ((ssize_t)अन्तर्_सूचकांक == read_chars) {
			यदि (बहिर्गमनम्_सूचकांक) {
				xwrite(STDOUT_FILENO, str2, बहिर्गमनम्_सूचकांक);
 start_from:
				बहिर्गमनम्_सूचकांक = 0;
			}
			read_chars = safe_read(STDIN_FILENO, str1, TR_BUFSIZ);
			यदि (read_chars <= 0) {
				यदि (read_chars < 0)
					bb_simple_perror_सन्देशः_and_die(bb_सन्देशः_read_error);
				भङ्ग;
			}
			अन्तर्_सूचकांक = 0;
		}
		c = str1[अन्तर्_सूचकांक++];
		यदि ((opts & TR_OPT_delete) && invec[c])
			अनुवर्त;
		coded = सदिशम्[c];
		यदि ((opts & TR_OPT_squeeze_reps) && चरमः == coded
		 && (invec[c] || outvec[coded])
		) {
			अनुवर्त;
		}
		str2[बहिर्गमनम्_सूचकांक++] = चरमः = coded;
	}

	यदि (ENABLE_FEATURE_CLEAN_ऊर्ध्वम्) {
		मुक्त(सदिशम्);
		मुक्त(str2);
		मुक्त(str1);
	}

	प्रत्यागम EXIT_SUCCESS;
}
