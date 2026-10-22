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

#ଅନ୍ତର୍ଭାଵ "libbb.h"

ଗଣନା {
	ASCII = 256,
	/* string buffer needs to be at least as big as the whole "alphabet".
	 * BUFSIZ == ASCII is ok, but we will realloc in expand
	 * even for smallest patterns, let's avoid that by using *2:
	 */
	TR_BUFSIZ = (BUFSIZ > ASCII*2) ? BUFSIZ : ASCII*2,
};

ସ୍ଥିର ଶୂନ୍ୟ map(ଅକ୍ଷର *pvector,
		ଅକ୍ଷର *string1, unsigned string1_len,
		ଅକ୍ଷର *string2, unsigned string2_len)
{
	ଅକ୍ଷର last = '0';
	unsigned i, j;

	ପାଇଁ (j = 0, i = 0; i < string1_len; i++) {
		ଯଦି (string2_len <= j)
			pvector[(unsigned ଅକ୍ଷର)(string1[i])] = last;
		ଅନ୍ୟଥା
			pvector[(unsigned ଅକ୍ଷର)(string1[i])] = last = string2[j++];
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
ସ୍ଥିର unsigned expand(ଅକ୍ଷର *arg, ଅକ୍ଷର **buffer_p)
{
	ଅକ୍ଷର *buffer = *buffer_p;
	unsigned pos = 0;
	unsigned size = TR_BUFSIZ;
	unsigned i; /* can't be unsigned char: must be able to hold 256 */
	unsigned ଅକ୍ଷର ac;

	ଯେତେବେଳେ (*arg) {
		ଯଦି (pos + ASCII > size) {
			size += ASCII;
			*buffer_p = buffer = xrealloc(buffer, size);
		}
		ଯଦି (*arg == '\\') {
			ଅଚଳ ଅକ୍ଷର *z;
			arg++;
			z = arg;
			ac = bb_process_escape_sequence(&z);
			arg = (ଅକ୍ଷର *)z;
			arg--;
			*arg = ac;
			/*
			 * fall through, there may be a range.
			 * If not, current char will be treated anyway.
			 */
		}
		ଯଦି (arg[1] == '-') { /* "0-9..." */
			ac = arg[2];
			ଯଦି (ac == '\0') { /* "0-": copy verbatim */
				buffer[pos++] = *arg++; /* copy '0' */
				ଜାରି; /* next iter will copy '-' and stop */
			}
			i = (unsigned ଅକ୍ଷର) *arg;
			arg += 3; /* skip 0-9 or 0-\ */
			ଯଦି (ac == '\\') {
				ଅଚଳ ଅକ୍ଷର *z;
				z = arg;
				ac = bb_process_escape_sequence(&z);
				arg = (ଅକ୍ଷର *)z;
			}
			ଯେତେବେଳେ (i <= ac) /* ok: i is unsigned _int_ */
				buffer[pos++] = i++;
			ଜାରି;
		}
		ଯଦି ((ENABLE_FEATURE_TR_CLASSES || ENABLE_FEATURE_TR_EQUIV)
		 && *arg == '['
		) {
			arg++;
			i = (unsigned ଅକ୍ଷର) *arg++;
			/* "[xyz...". i=x, arg points to y */
			ଯଦି (ENABLE_FEATURE_TR_CLASSES && i == ':') { /* [:class:] */
#define CLO ":]\0"
				ସ୍ଥିର ଅଚଳ ଅକ୍ଷର classes[] ALIGN1 =
					"alpha"CLO "alnum"CLO "digit"CLO
					"lower"CLO "upper"CLO "space"CLO
					"blank"CLO "punct"CLO "cntrl"CLO
					"xdigit"CLO;
				ଗଣନା {
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
				ଅକ୍ଷର *tmp;

				/* xdigit needs 8, not 7 */
				i = 7 + (arg[0] == 'x');
				tmp = xstrndup(arg, i);
				j = index_in_strings(classes, tmp) + 1;
				ମୁକ୍ତ(tmp);

				ଯଦି (j == CLASS_invalid)
					ଯାଅ skip_bracket;

				arg += i;
				ଯଦି (j == CLASS_alnum || j == CLASS_digit || j == CLASS_xdigit) {
					ପାଇଁ (i = '0'; i <= '9'; i++)
						buffer[pos++] = i;
				}
				ଯଦି (j == CLASS_alpha || j == CLASS_alnum || j == CLASS_upper) {
					ପାଇଁ (i = 'A'; i <= 'Z'; i++)
						buffer[pos++] = i;
				}
				ଯଦି (j == CLASS_alpha || j == CLASS_alnum || j == CLASS_lower) {
					ପାଇଁ (i = 'a'; i <= 'z'; i++)
						buffer[pos++] = i;
				}
				ଯଦି (j == CLASS_space || j == CLASS_blank) {
					buffer[pos++] = '\t';
					ଯଦି (j == CLASS_space) {
						buffer[pos++] = '\n';
						buffer[pos++] = '\v';
						buffer[pos++] = '\f';
						buffer[pos++] = '\r';
					}
					buffer[pos++] = ' ';
				}
				ଯଦି (j == CLASS_punct || j == CLASS_cntrl) {
					ପାଇଁ (i = '\0'; i < ASCII; i++) {
						ଯଦି ((j == CLASS_punct && isprint_asciionly(i) && !isalnum(i) && !isspace(i))
						 || (j == CLASS_cntrl && iscntrl(i))
						) {
							buffer[pos++] = i;
						}
					}
				}
				ଯଦି (j == CLASS_xdigit) {
					ପାଇଁ (i = 'A'; i <= 'F'; i++) {
						buffer[pos + 6] = i | 0x20;
						buffer[pos++] = i;
					}
					pos += 6;
				}
				ଜାରି;
			}
			/* "[xyz...", i=x, arg points to y */
			ଯଦି (ENABLE_FEATURE_TR_EQUIV && i == '=') { /* [=CHAR=] */
				buffer[pos++] = *arg; /* copy CHAR */
				ଯଦି (!arg[0] || arg[1] != '=' || arg[2] != ']')
					bb_show_usage();
				arg += 3;  /* skip CHAR=] */
				ଜାରି;
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
	ଫେରନ୍ତୁ pos;
}

/* NB: buffer is guaranteed to be at least TR_BUFSIZE
 * (which is >= ASCII) big.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା complement(ଅକ୍ଷର *buffer, ପୂର୍ଣ୍ଣସଂଖ୍ୟା buffer_len)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len;
	ଅକ୍ଷର conv[ASCII];
	unsigned ଅକ୍ଷର ch;

	len = 0;
	ch = '\0';
	ଯେତେବେଳେ (1) {
		ଯଦି (memchr(buffer, ch, buffer_len) == ଖାଲି)
			conv[len++] = ch;
		ଯଦି (++ch == '\0')
			ଭାଙ୍ଗ;
	}
	memcpy(buffer, conv, len);
	ଫେରନ୍ତୁ len;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା tr_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv) MAIN_EXTERNALLY_VISIBLE;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା tr_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc UNUSED_PARAM, ଅକ୍ଷର **argv)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	smalluint opts;
	ssize_t read_chars;
	size_t in_index, out_index;
	unsigned last = UCHAR_MAX + 1; /* not equal to any char */
	unsigned ଅକ୍ଷର coded, c;
	ଅକ୍ଷର *str1 = xmalloc(TR_BUFSIZ);
	ଅକ୍ଷର *str2 = xmalloc(TR_BUFSIZ);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା str2_length;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା str1_length;
	ଅକ୍ଷର *vector = xzalloc(ASCII * 3);
	ଅକ୍ଷର *invec  = vector + ASCII;
	ଅକ୍ଷର *outvec = vector + ASCII * 2;

#define TR_OPT_complement   (3 << 0)
#define TR_OPT_delete       (1 << 2)
#define TR_OPT_squeeze_reps (1 << 3)

	ପାଇଁ (i = 0; i < ASCII; i++) {
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
	ଯଦି (opts & TR_OPT_complement)
		str1_length = complement(str1, str1_length);
	ଯଦି (*argv) {
		ଯଦି (argv[0][0] == '\0')
			bb_simple_error_ସନ୍ଦେଶ_and_die("STRING2 cannot be empty");
		str2_length = expand(*argv, &str2);
		map(vector, str1, str1_length,
				str2, str2_length);
	}
	ପାଇଁ (i = 0; i < str1_length; i++)
		invec[(unsigned ଅକ୍ଷର)(str1[i])] = TRUE;
	ପାଇଁ (i = 0; i < str2_length; i++)
		outvec[(unsigned ଅକ୍ଷର)(str2[i])] = TRUE;

	ଯାଅ start_from;

	/* In this loop, str1 space is reused as input buffer,
	 * str2 - as output one. */
	ପାଇଁ (;;) {
		/* If we're out of input, flush output and read more input. */
		ଯଦି ((ssize_t)in_index == read_chars) {
			ଯଦି (out_index) {
				xwrite(STDOUT_FILENO, str2, out_index);
 start_from:
				out_index = 0;
			}
			read_chars = safe_read(STDIN_FILENO, str1, TR_BUFSIZ);
			ଯଦି (read_chars <= 0) {
				ଯଦି (read_chars < 0)
					bb_simple_perror_ସନ୍ଦେଶ_and_die(bb_ସନ୍ଦେଶ_read_error);
				ଭାଙ୍ଗ;
			}
			in_index = 0;
		}
		c = str1[in_index++];
		ଯଦି ((opts & TR_OPT_delete) && invec[c])
			ଜାରି;
		coded = vector[c];
		ଯଦି ((opts & TR_OPT_squeeze_reps) && last == coded
		 && (invec[c] || outvec[coded])
		) {
			ଜାରି;
		}
		str2[out_index++] = last = coded;
	}

	ଯଦି (ENABLE_FEATURE_CLEAN_UP) {
		ମୁକ୍ତ(vector);
		ମୁକ୍ତ(str2);
		ମୁକ୍ତ(str1);
	}

	ଫେରନ୍ତୁ EXIT_SUCCESS;
}
