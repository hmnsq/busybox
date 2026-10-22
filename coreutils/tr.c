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

#ಅನ್ತರ್ಭಾವ "libbb.h"

ಗಣನಾ {
	ASCII = 256,
	/* string buffer needs to be at least as big as the whole "alphabet".
	 * BUFSIZ == ASCII is ok, but we will realloc in expand
	 * even for smallest patterns, let's avoid that by using *2:
	 */
	TR_BUFSIZ = (BUFSIZ > ASCII*2) ? BUFSIZ : ASCII*2,
};

ಸ್ಥಿರ ಶೂನ್ಯ map(ವರ್ಣ *pvector,
		ವರ್ಣ *string1, unsigned string1_len,
		ವರ್ಣ *string2, unsigned string2_len)
{
	ವರ್ಣ last = '0';
	unsigned i, j;

	ಪ್ರತಿ (j = 0, i = 0; i < string1_len; i++) {
		ಯದಿ (string2_len <= j)
			pvector[(unsigned ವರ್ಣ)(string1[i])] = last;
		ಅನ್ಯಥಾ
			pvector[(unsigned ವರ್ಣ)(string1[i])] = last = string2[j++];
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
ಸ್ಥಿರ unsigned expand(ವರ್ಣ *arg, ವರ್ಣ **buffer_p)
{
	ವರ್ಣ *buffer = *buffer_p;
	unsigned pos = 0;
	unsigned size = TR_BUFSIZ;
	unsigned i; /* can't be unsigned char: must be able to hold 256 */
	unsigned ವರ್ಣ ac;

	ಯಾವತ್ (*arg) {
		ಯದಿ (pos + ASCII > size) {
			size += ASCII;
			*buffer_p = buffer = xrealloc(buffer, size);
		}
		ಯದಿ (*arg == '\\') {
			ಧ್ರುವ ವರ್ಣ *z;
			arg++;
			z = arg;
			ac = bb_process_escape_sequence(&z);
			arg = (ವರ್ಣ *)z;
			arg--;
			*arg = ac;
			/*
			 * fall through, there may be a range.
			 * If not, current char will be treated anyway.
			 */
		}
		ಯದಿ (arg[1] == '-') { /* "0-9..." */
			ac = arg[2];
			ಯದಿ (ac == '\0') { /* "0-": copy verbatim */
				buffer[pos++] = *arg++; /* copy '0' */
				ಅನುವರ್ತ; /* next iter will copy '-' and stop */
			}
			i = (unsigned ವರ್ಣ) *arg;
			arg += 3; /* skip 0-9 or 0-\ */
			ಯದಿ (ac == '\\') {
				ಧ್ರುವ ವರ್ಣ *z;
				z = arg;
				ac = bb_process_escape_sequence(&z);
				arg = (ವರ್ಣ *)z;
			}
			ಯಾವತ್ (i <= ac) /* ok: i is unsigned _int_ */
				buffer[pos++] = i++;
			ಅನುವರ್ತ;
		}
		ಯದಿ ((ENABLE_FEATURE_TR_CLASSES || ENABLE_FEATURE_TR_EQUIV)
		 && *arg == '['
		) {
			arg++;
			i = (unsigned ವರ್ಣ) *arg++;
			/* "[xyz...". i=x, arg points to y */
			ಯದಿ (ENABLE_FEATURE_TR_CLASSES && i == ':') { /* [:class:] */
#define CLO ":]\0"
				ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ classes[] ALIGN1 =
					"alpha"CLO "alnum"CLO "digit"CLO
					"lower"CLO "upper"CLO "space"CLO
					"blank"CLO "punct"CLO "cntrl"CLO
					"xdigit"CLO;
				ಗಣನಾ {
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
				ವರ್ಣ *tmp;

				/* xdigit needs 8, not 7 */
				i = 7 + (arg[0] == 'x');
				tmp = xstrndup(arg, i);
				j = index_in_strings(classes, tmp) + 1;
				ಮುಕ್ತ(tmp);

				ಯದಿ (j == CLASS_invalid)
					ಗಚ್ಛ skip_bracket;

				arg += i;
				ಯದಿ (j == CLASS_alnum || j == CLASS_digit || j == CLASS_xdigit) {
					ಪ್ರತಿ (i = '0'; i <= '9'; i++)
						buffer[pos++] = i;
				}
				ಯದಿ (j == CLASS_alpha || j == CLASS_alnum || j == CLASS_upper) {
					ಪ್ರತಿ (i = 'A'; i <= 'Z'; i++)
						buffer[pos++] = i;
				}
				ಯದಿ (j == CLASS_alpha || j == CLASS_alnum || j == CLASS_lower) {
					ಪ್ರತಿ (i = 'a'; i <= 'z'; i++)
						buffer[pos++] = i;
				}
				ಯದಿ (j == CLASS_space || j == CLASS_blank) {
					buffer[pos++] = '\t';
					ಯದಿ (j == CLASS_space) {
						buffer[pos++] = '\n';
						buffer[pos++] = '\v';
						buffer[pos++] = '\f';
						buffer[pos++] = '\r';
					}
					buffer[pos++] = ' ';
				}
				ಯದಿ (j == CLASS_punct || j == CLASS_cntrl) {
					ಪ್ರತಿ (i = '\0'; i < ASCII; i++) {
						ಯದಿ ((j == CLASS_punct && isprint_asciionly(i) && !isalnum(i) && !isspace(i))
						 || (j == CLASS_cntrl && iscntrl(i))
						) {
							buffer[pos++] = i;
						}
					}
				}
				ಯದಿ (j == CLASS_xdigit) {
					ಪ್ರತಿ (i = 'A'; i <= 'F'; i++) {
						buffer[pos + 6] = i | 0x20;
						buffer[pos++] = i;
					}
					pos += 6;
				}
				ಅನುವರ್ತ;
			}
			/* "[xyz...", i=x, arg points to y */
			ಯದಿ (ENABLE_FEATURE_TR_EQUIV && i == '=') { /* [=CHAR=] */
				buffer[pos++] = *arg; /* copy CHAR */
				ಯದಿ (!arg[0] || arg[1] != '=' || arg[2] != ']')
					bb_show_usage();
				arg += 3;  /* skip CHAR=] */
				ಅನುವರ್ತ;
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
	ಪ್ರತ್ಯಾಗಮ pos;
}

/* NB: buffer is guaranteed to be at least TR_BUFSIZE
 * (which is >= ASCII) big.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ complement(ವರ್ಣ *buffer, ಪೂರ್ಣಾಙ್ಕ buffer_len)
{
	ಪೂರ್ಣಾಙ್ಕ len;
	ವರ್ಣ conv[ASCII];
	unsigned ವರ್ಣ ch;

	len = 0;
	ch = '\0';
	ಯಾವತ್ (1) {
		ಯದಿ (memchr(buffer, ch, buffer_len) == ರಿಕ್ತ)
			conv[len++] = ch;
		ಯದಿ (++ch == '\0')
			ಭಙ್ಗ;
	}
	memcpy(buffer, conv, len);
	ಪ್ರತ್ಯಾಗಮ len;
}

ಪೂರ್ಣಾಙ್ಕ tr_main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಙ್ಕ tr_main(ಪೂರ್ಣಾಙ್ಕ argc UNUSED_PARAM, ವರ್ಣ **argv)
{
	ಪೂರ್ಣಾಙ್ಕ i;
	smalluint opts;
	ssize_t read_chars;
	size_t in_index, out_index;
	unsigned last = UCHAR_MAX + 1; /* not equal to any char */
	unsigned ವರ್ಣ coded, c;
	ವರ್ಣ *str1 = xmalloc(TR_BUFSIZ);
	ವರ್ಣ *str2 = xmalloc(TR_BUFSIZ);
	ಪೂರ್ಣಾಙ್ಕ str2_length;
	ಪೂರ್ಣಾಙ್ಕ str1_length;
	ವರ್ಣ *vector = xzalloc(ASCII * 3);
	ವರ್ಣ *invec  = vector + ASCII;
	ವರ್ಣ *outvec = vector + ASCII * 2;

#define TR_OPT_complement   (3 << 0)
#define TR_OPT_delete       (1 << 2)
#define TR_OPT_squeeze_reps (1 << 3)

	ಪ್ರತಿ (i = 0; i < ASCII; i++) {
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
	ಯದಿ (opts & TR_OPT_complement)
		str1_length = complement(str1, str1_length);
	ಯದಿ (*argv) {
		ಯದಿ (argv[0][0] == '\0')
			bb_simple_error_ಸಂದೇಶ_and_die("STRING2 cannot be empty");
		str2_length = expand(*argv, &str2);
		map(vector, str1, str1_length,
				str2, str2_length);
	}
	ಪ್ರತಿ (i = 0; i < str1_length; i++)
		invec[(unsigned ವರ್ಣ)(str1[i])] = TRUE;
	ಪ್ರತಿ (i = 0; i < str2_length; i++)
		outvec[(unsigned ವರ್ಣ)(str2[i])] = TRUE;

	ಗಚ್ಛ start_from;

	/* In this loop, str1 space is reused as input buffer,
	 * str2 - as output one. */
	ಪ್ರತಿ (;;) {
		/* If we're out of input, flush output and read more input. */
		ಯದಿ ((ssize_t)in_index == read_chars) {
			ಯದಿ (out_index) {
				xwrite(STDOUT_FILENO, str2, out_index);
 start_from:
				out_index = 0;
			}
			read_chars = safe_read(STDIN_FILENO, str1, TR_BUFSIZ);
			ಯದಿ (read_chars <= 0) {
				ಯದಿ (read_chars < 0)
					bb_simple_perror_ಸಂದೇಶ_and_die(bb_ಸಂದೇಶ_read_error);
				ಭಙ್ಗ;
			}
			in_index = 0;
		}
		c = str1[in_index++];
		ಯದಿ ((opts & TR_OPT_delete) && invec[c])
			ಅನುವರ್ತ;
		coded = vector[c];
		ಯದಿ ((opts & TR_OPT_squeeze_reps) && last == coded
		 && (invec[c] || outvec[coded])
		) {
			ಅನುವರ್ತ;
		}
		str2[out_index++] = last = coded;
	}

	ಯದಿ (ENABLE_FEATURE_CLEAN_UP) {
		ಮುಕ್ತ(vector);
		ಮುಕ್ತ(str2);
		ಮುಕ್ತ(str1);
	}

	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}
