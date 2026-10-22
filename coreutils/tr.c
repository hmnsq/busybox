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

ಎಣಿಕೆ {
	ASCII = 256,
	/* string buffer needs to be at least as big as the whole "alphabet".
	 * BUFSIZ == ASCII is ok, but we will realloc in expand
	 * even for smallest patterns, let's avoid that by using *2:
	 */
	TR_BUFSIZ = (BUFSIZ > ASCII*2) ? BUFSIZ : ASCII*2,
};

ಸ್ಥಿರ ಶೂನ್ಯ map(ಅಕ್ಷರ *pvector,
		ಅಕ್ಷರ *string1, ಚಿಹ್ನೆರಹಿತ string1_len,
		ಅಕ್ಷರ *string2, ಚಿಹ್ನೆರಹಿತ string2_len)
{
	ಅಕ್ಷರ ಕೊನೆಯ = '0';
	ಚಿಹ್ನೆರಹಿತ i, j;

	ಗಾಗಿ (j = 0, i = 0; i < string1_len; i++) {
		ಆದರೆ (string2_len <= j)
			pvector[(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(string1[i])] = ಕೊನೆಯ;
		ಇಲ್ಲದಿದ್ದರೆ
			pvector[(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(string1[i])] = ಕೊನೆಯ = string2[j++];
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
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ expand(ಅಕ್ಷರ *arg, ಅಕ್ಷರ **ಬಫರ್_p)
{
	ಅಕ್ಷರ *ಬಫರ್ = *ಬಫರ್_p;
	ಚಿಹ್ನೆರಹಿತ pos = 0;
	ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_2 = TR_BUFSIZ;
	ಚಿಹ್ನೆರಹಿತ i; /* can't be unsigned char: must be able to hold 256 */
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ac;

	ಆಗ (*arg) {
		ಆದರೆ (pos + ASCII > ಗಾತ್ರ_2) {
			ಗಾತ್ರ_2 += ASCII;
			*ಬಫರ್_p = ಬಫರ್ = xrealloc(ಬಫರ್, ಗಾತ್ರ_2);
		}
		ಆದರೆ (*arg == '\\') {
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z;
			arg++;
			z = arg;
			ac = bb_process_escape_sequence(&z);
			arg = (ಅಕ್ಷರ *)z;
			arg--;
			*arg = ac;
			/*
			 * fall through, there may be a range.
			 * If not, current char will be treated anyway.
			 */
		}
		ಆದರೆ (arg[1] == '-') { /* "0-9..." */
			ac = arg[2];
			ಆದರೆ (ac == '\0') { /* "0-": copy verbatim */
				ಬಫರ್[pos++] = *arg++; /* copy '0' */
				ಮುಂದುವರಿ; /* next iter will copy '-' and stop */
			}
			i = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) *arg;
			arg += 3; /* skip 0-9 or 0-\ */
			ಆದರೆ (ac == '\\') {
				ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z;
				z = arg;
				ac = bb_process_escape_sequence(&z);
				arg = (ಅಕ್ಷರ *)z;
			}
			ಆಗ (i <= ac) /* ok: i is unsigned _int_ */
				ಬಫರ್[pos++] = i++;
			ಮುಂದುವರಿ;
		}
		ಆದರೆ ((ENABLE_FEATURE_TR_CLASSES || ENABLE_FEATURE_TR_EQUIV)
		 && *arg == '['
		) {
			arg++;
			i = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) *arg++;
			/* "[xyz...". i=x, arg points to y */
			ಆದರೆ (ENABLE_FEATURE_TR_CLASSES && i == ':') { /* [:class:] */
#ವ್ಯಾಖ್ಯಾನಿಸು CLO ":]\0"
				ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ classes[] ALIGN1 =
					"alpha"CLO "alnum"CLO "digit"CLO
					"lower"CLO "upper"CLO "space"CLO
					"blank"CLO "punct"CLO "cntrl"CLO
					"xdigit"CLO;
				ಎಣಿಕೆ {
					CLASS_invalid = 0, /* we increment the retval */
					CLASS_alpha = 1,
					CLASS_alnum = 2,
					CLASS_digit = 3,
					CLASS_ಅಧೋ = 4,
					CLASS_ಊರ್ಧ್ವ = 5,
					CLASS_space = 6,
					CLASS_blank = 7,
					CLASS_punct = 8,
					CLASS_cntrl = 9,
					CLASS_xdigit = 10,
					//CLASS_graph = 11,
					//CLASS_print = 12,
				};
				smalluint j;
				ಅಕ್ಷರ *tmp;

				/* xdigit needs 8, not 7 */
				i = 7 + (arg[0] == 'x');
				tmp = xstrndup(arg, i);
				j = ಸೂಚ್ಯಂಕ_ಒಳ_ಅಕ್ಷರಸರಣಿಗಳು(classes, tmp) + 1;
				ಬಿಡುಗಡೆ(tmp);

				ಆದರೆ (j == CLASS_invalid)
					ಹೋಗು skip_bracket;

				arg += i;
				ಆದರೆ (j == CLASS_alnum || j == CLASS_digit || j == CLASS_xdigit) {
					ಗಾಗಿ (i = '0'; i <= '9'; i++)
						ಬಫರ್[pos++] = i;
				}
				ಆದರೆ (j == CLASS_alpha || j == CLASS_alnum || j == CLASS_ಊರ್ಧ್ವ) {
					ಗಾಗಿ (i = 'A'; i <= 'Z'; i++)
						ಬಫರ್[pos++] = i;
				}
				ಆದರೆ (j == CLASS_alpha || j == CLASS_alnum || j == CLASS_ಅಧೋ) {
					ಗಾಗಿ (i = 'a'; i <= 'z'; i++)
						ಬಫರ್[pos++] = i;
				}
				ಆದರೆ (j == CLASS_space || j == CLASS_blank) {
					ಬಫರ್[pos++] = '\t';
					ಆದರೆ (j == CLASS_space) {
						ಬಫರ್[pos++] = '\n';
						ಬಫರ್[pos++] = '\v';
						ಬಫರ್[pos++] = '\f';
						ಬಫರ್[pos++] = '\r';
					}
					ಬಫರ್[pos++] = ' ';
				}
				ಆದರೆ (j == CLASS_punct || j == CLASS_cntrl) {
					ಗಾಗಿ (i = '\0'; i < ASCII; i++) {
						ಆದರೆ ((j == CLASS_punct && isprint_asciionly(i) && !ಅಕ್ಷರವೇ_ಅಂಕಿಯೇ(i) && !ಖಾಲಿಯೇ(i))
						 || (j == CLASS_cntrl && iscntrl(i))
						) {
							ಬಫರ್[pos++] = i;
						}
					}
				}
				ಆದರೆ (j == CLASS_xdigit) {
					ಗಾಗಿ (i = 'A'; i <= 'F'; i++) {
						ಬಫರ್[pos + 6] = i | 0x20;
						ಬಫರ್[pos++] = i;
					}
					pos += 6;
				}
				ಮುಂದುವರಿ;
			}
			/* "[xyz...", i=x, arg points to y */
			ಆದರೆ (ENABLE_FEATURE_TR_EQUIV && i == '=') { /* [=CHAR=] */
				ಬಫರ್[pos++] = *arg; /* copy CHAR */
				ಆದರೆ (!arg[0] || arg[1] != '=' || arg[2] != ']')
					bb_show_usage();
				arg += 3;  /* skip CHAR=] */
				ಮುಂದುವರಿ;
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
		ಬಫರ್[pos++] = *arg++;
	}
	ಹಿಂತಿರುಗಿ pos;
}

/* NB: buffer is guaranteed to be at least TR_BUFSIZE
 * (which is >= ASCII) big.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ complement(ಅಕ್ಷರ *ಬಫರ್, ಪೂರ್ಣಾಂಕ ಬಫರ್_len)
{
	ಪೂರ್ಣಾಂಕ len;
	ಅಕ್ಷರ conv[ASCII];
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ch;

	len = 0;
	ch = '\0';
	ಆಗ (1) {
		ಆದರೆ (memchr(ಬಫರ್, ch, ಬಫರ್_len) == ಖಾಲಿ)
			conv[len++] = ch;
		ಆದರೆ (++ch == '\0')
			ನಿಲ್ಲಿಸು;
	}
	ಸ್ಮರಣೆ_ನಕಲು(ಬಫರ್, conv, len);
	ಹಿಂತಿರುಗಿ len;
}

ಪೂರ್ಣಾಂಕ tr_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ tr_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	ಪೂರ್ಣಾಂಕ i;
	smalluint opts;
	ssize_t read_chars;
	ಗಾತ್ರ_t ಒಳ_ಸೂಚ್ಯಂಕ, ಹೊರಹೋಗು_ಸೂಚ್ಯಂಕ;
	ಚಿಹ್ನೆರಹಿತ ಕೊನೆಯ = UCHAR_MAX + 1; /* not equal to any char */
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ coded, c;
	ಅಕ್ಷರ *str1 = xmalloc(TR_BUFSIZ);
	ಅಕ್ಷರ *str2 = xmalloc(TR_BUFSIZ);
	ಪೂರ್ಣಾಂಕ str2_ಉದ್ದಳತೆ;
	ಪೂರ್ಣಾಂಕ str1_ಉದ್ದಳತೆ;
	ಅಕ್ಷರ *ಸದಿಶ = xzalloc(ASCII * 3);
	ಅಕ್ಷರ *invec  = ಸದಿಶ + ASCII;
	ಅಕ್ಷರ *outvec = ಸದಿಶ + ASCII * 2;

#ವ್ಯಾಖ್ಯಾನಿಸು TR_OPT_complement   (3 << 0)
#ವ್ಯಾಖ್ಯಾನಿಸು TR_OPT_delete       (1 << 2)
#ವ್ಯಾಖ್ಯಾನಿಸು TR_OPT_squeeze_reps (1 << 3)

	ಗಾಗಿ (i = 0; i < ASCII; i++) {
		ಸದಿಶ[i] = i;
		/*invec[i] = outvec[i] = FALSE; - done by xzalloc */
	}

	/* -C/-c difference is that -C complements "characters",
	 * and -c complements "values" (binary bytes I guess).
	 * In POSIX locale, these are the same.
	 */

	/* '+': stop at first non-option */
	opts = getopt32(argv, "^+" "Ccds" "\0" "-1:?2");
	argv += optind;

	str1_ಉದ್ದಳತೆ = expand(*argv++, &str1);
	str2_ಉದ್ದಳತೆ = 0;
	ಆದರೆ (opts & TR_OPT_complement)
		str1_ಉದ್ದಳತೆ = complement(str1, str1_ಉದ್ದಳತೆ);
	ಆದರೆ (*argv) {
		ಆದರೆ (argv[0][0] == '\0')
			bb_simple_error_ಸಂದೇಶ_and_die("STRING2 cannot be empty");
		str2_ಉದ್ದಳತೆ = expand(*argv, &str2);
		map(ಸದಿಶ, str1, str1_ಉದ್ದಳತೆ,
				str2, str2_ಉದ್ದಳತೆ);
	}
	ಗಾಗಿ (i = 0; i < str1_ಉದ್ದಳತೆ; i++)
		invec[(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(str1[i])] = TRUE;
	ಗಾಗಿ (i = 0; i < str2_ಉದ್ದಳತೆ; i++)
		outvec[(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(str2[i])] = TRUE;

	ಹೋಗು ಆರಂಭ_from;

	/* In this loop, str1 space is reused as input buffer,
	 * str2 - as output one. */
	ಗಾಗಿ (;;) {
		/* If we're out of input, flush output and read more input. */
		ಆದರೆ ((ssize_t)ಒಳ_ಸೂಚ್ಯಂಕ == read_chars) {
			ಆದರೆ (ಹೊರಹೋಗು_ಸೂಚ್ಯಂಕ) {
				xwrite(STDOUT_FILENO, str2, ಹೊರಹೋಗು_ಸೂಚ್ಯಂಕ);
 ಆರಂಭ_from:
				ಹೊರಹೋಗು_ಸೂಚ್ಯಂಕ = 0;
			}
			read_chars = ಸುರಕ್ಷಿತ_read(STDIN_FILENO, str1, TR_BUFSIZ);
			ಆದರೆ (read_chars <= 0) {
				ಆದರೆ (read_chars < 0)
					bb_simple_perror_ಸಂದೇಶ_and_die(bb_ಸಂದೇಶ_read_error);
				ನಿಲ್ಲಿಸು;
			}
			ಒಳ_ಸೂಚ್ಯಂಕ = 0;
		}
		c = str1[ಒಳ_ಸೂಚ್ಯಂಕ++];
		ಆದರೆ ((opts & TR_OPT_delete) && invec[c])
			ಮುಂದುವರಿ;
		coded = ಸದಿಶ[c];
		ಆದರೆ ((opts & TR_OPT_squeeze_reps) && ಕೊನೆಯ == coded
		 && (invec[c] || outvec[coded])
		) {
			ಮುಂದುವರಿ;
		}
		str2[ಹೊರಹೋಗು_ಸೂಚ್ಯಂಕ++] = ಕೊನೆಯ = coded;
	}

	ಆದರೆ (ENABLE_FEATURE_CLEAN_ಮೇಲೆ) {
		ಬಿಡುಗಡೆ(ಸದಿಶ);
		ಬಿಡುಗಡೆ(str2);
		ಬಿಡುಗಡೆ(str1);
	}

	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}
