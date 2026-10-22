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
	TR_BUFSIZ = (बफ_मान > ASCII*2) ? बफ_मान : ASCII*2,
};

स्थिर शून्य map(वर्ण *pvector,
		वर्ण *string1, अचिन्हित string1_len,
		वर्ण *string2, अचिन्हित string2_len)
{
	वर्ण last = '0';
	अचिन्हित i, j;

	प्रति (j = 0, i = 0; i < string1_len; i++) {
		यदि (string2_len <= j)
			pvector[(अचिन्हित वर्ण)(string1[i])] = last;
		अन्यथा
			pvector[(अचिन्हित वर्ण)(string1[i])] = last = string2[j++];
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
स्थिर अचिन्हित expand(वर्ण *arg, वर्ण **buffer_p)
{
	वर्ण *buffer = *buffer_p;
	अचिन्हित pos = 0;
	अचिन्हित size = TR_BUFSIZ;
	अचिन्हित i; /* can't be unsigned char: must be able to hold 256 */
	अचिन्हित वर्ण ac;

	यावत् (*arg) {
		यदि (pos + ASCII > size) {
			size += ASCII;
			*buffer_p = buffer = xrealloc(buffer, size);
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
				buffer[pos++] = *arg++; /* copy '0' */
				अनुवर्त; /* next iter will copy '-' and stop */
			}
			i = (अचिन्हित वर्ण) *arg;
			arg += 3; /* skip 0-9 or 0-\ */
			यदि (ac == '\\') {
				ध्रुव वर्ण *z;
				z = arg;
				ac = bb_process_escape_sequence(&z);
				arg = (वर्ण *)z;
			}
			यावत् (i <= ac) /* ok: i is unsigned _int_ */
				buffer[pos++] = i++;
			अनुवर्त;
		}
		यदि ((ENABLE_FEATURE_TR_CLASSES || ENABLE_FEATURE_TR_EQUIV)
		 && *arg == '['
		) {
			arg++;
			i = (अचिन्हित वर्ण) *arg++;
			/* "[xyz...". i=x, arg points to y */
			यदि (ENABLE_FEATURE_TR_CLASSES && i == ':') { /* [:class:] */
#define CLO ":]\0"
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
				वर्ण *tmp;

				/* xdigit needs 8, not 7 */
				i = 7 + (arg[0] == 'x');
				tmp = xstrndup(arg, i);
				j = index_in_strings(classes, tmp) + 1;
				मुक्त(tmp);

				यदि (j == CLASS_invalid)
					गच्छ skip_bracket;

				arg += i;
				यदि (j == CLASS_alnum || j == CLASS_digit || j == CLASS_xdigit) {
					प्रति (i = '0'; i <= '9'; i++)
						buffer[pos++] = i;
				}
				यदि (j == CLASS_alpha || j == CLASS_alnum || j == CLASS_upper) {
					प्रति (i = 'A'; i <= 'Z'; i++)
						buffer[pos++] = i;
				}
				यदि (j == CLASS_alpha || j == CLASS_alnum || j == CLASS_lower) {
					प्रति (i = 'a'; i <= 'z'; i++)
						buffer[pos++] = i;
				}
				यदि (j == CLASS_space || j == CLASS_blank) {
					buffer[pos++] = '\t';
					यदि (j == CLASS_space) {
						buffer[pos++] = '\n';
						buffer[pos++] = '\v';
						buffer[pos++] = '\f';
						buffer[pos++] = '\r';
					}
					buffer[pos++] = ' ';
				}
				यदि (j == CLASS_punct || j == CLASS_cntrl) {
					प्रति (i = '\0'; i < ASCII; i++) {
						यदि ((j == CLASS_punct && isprint_asciionly(i) && !है_अक्षर_अंक(i) && !है_खाली(i))
						 || (j == CLASS_cntrl && है_नियंत्रण(i))
						) {
							buffer[pos++] = i;
						}
					}
				}
				यदि (j == CLASS_xdigit) {
					प्रति (i = 'A'; i <= 'F'; i++) {
						buffer[pos + 6] = i | 0x20;
						buffer[pos++] = i;
					}
					pos += 6;
				}
				अनुवर्त;
			}
			/* "[xyz...", i=x, arg points to y */
			यदि (ENABLE_FEATURE_TR_EQUIV && i == '=') { /* [=CHAR=] */
				buffer[pos++] = *arg; /* copy CHAR */
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
		buffer[pos++] = *arg++;
	}
	प्रत्यागम pos;
}

/* NB: buffer is guaranteed to be at least TR_BUFSIZE
 * (which is >= ASCII) big.
 */
स्थिर पूर्णाङ्क complement(वर्ण *buffer, पूर्णाङ्क buffer_len)
{
	पूर्णाङ्क len;
	वर्ण conv[ASCII];
	अचिन्हित वर्ण ch;

	len = 0;
	ch = '\0';
	यावत् (1) {
		यदि (स_प्रथम(buffer, ch, buffer_len) == रिक्त)
			conv[len++] = ch;
		यदि (++ch == '\0')
			भङ्ग;
	}
	स_नकल(buffer, conv, len);
	प्रत्यागम len;
}

पूर्णाङ्क tr_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क tr_main(पूर्णाङ्क argc UNUSED_PARAM, वर्ण **argv)
{
	पूर्णाङ्क i;
	smalluint opts;
	ssize_t read_chars;
	माप_प्रकार in_index, out_index;
	अचिन्हित last = अच_अक्षर_उच्च + 1; /* not equal to any char */
	अचिन्हित वर्ण coded, c;
	वर्ण *str1 = xmalloc(TR_BUFSIZ);
	वर्ण *str2 = xmalloc(TR_BUFSIZ);
	पूर्णाङ्क str2_length;
	पूर्णाङ्क str1_length;
	वर्ण *vector = xzalloc(ASCII * 3);
	वर्ण *invec  = vector + ASCII;
	वर्ण *outvec = vector + ASCII * 2;

#define TR_OPT_complement   (3 << 0)
#define TR_OPT_delete       (1 << 2)
#define TR_OPT_squeeze_reps (1 << 3)

	प्रति (i = 0; i < ASCII; i++) {
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

	str1_length = expand(*argv++, &str1);
	str2_length = 0;
	यदि (opts & TR_OPT_complement)
		str1_length = complement(str1, str1_length);
	यदि (*argv) {
		यदि (argv[0][0] == '\0')
			bb_simple_error_संदेश_and_die("STRING2 cannot be empty");
		str2_length = expand(*argv, &str2);
		map(vector, str1, str1_length,
				str2, str2_length);
	}
	प्रति (i = 0; i < str1_length; i++)
		invec[(अचिन्हित वर्ण)(str1[i])] = TRUE;
	प्रति (i = 0; i < str2_length; i++)
		outvec[(अचिन्हित वर्ण)(str2[i])] = TRUE;

	गच्छ start_from;

	/* In this loop, str1 space is reused as input buffer,
	 * str2 - as output one. */
	प्रति (;;) {
		/* If we're out of input, flush output and read more input. */
		यदि ((ssize_t)in_index == read_chars) {
			यदि (out_index) {
				xwrite(STDOUT_FILENO, str2, out_index);
 start_from:
				out_index = 0;
			}
			read_chars = safe_read(STDIN_FILENO, str1, TR_BUFSIZ);
			यदि (read_chars <= 0) {
				यदि (read_chars < 0)
					bb_simple_perror_संदेश_and_die(bb_संदेश_read_error);
				भङ्ग;
			}
			in_index = 0;
		}
		c = str1[in_index++];
		यदि ((opts & TR_OPT_delete) && invec[c])
			अनुवर्त;
		coded = vector[c];
		यदि ((opts & TR_OPT_squeeze_reps) && last == coded
		 && (invec[c] || outvec[coded])
		) {
			अनुवर्त;
		}
		str2[out_index++] = last = coded;
	}

	यदि (ENABLE_FEATURE_CLEAN_UP) {
		मुक्त(vector);
		मुक्त(str2);
		मुक्त(str1);
	}

	प्रत्यागम निकास_सफल;
}
