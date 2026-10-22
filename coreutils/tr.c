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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"

ਗਿਣਤੀ {
	ASCII = 256,
	/* string buffer needs to be at least as big as the whole "alphabet".
	 * BUFSIZ == ASCII is ok, but we will realloc in expand
	 * even for smallest patterns, let's avoid that by using *2:
	 */
	TR_BUFSIZ = (BUFSIZ > ASCII*2) ? BUFSIZ : ASCII*2,
};

ਸਥਿਰ ਖਾਲੀ map(ਅੱਖਰ *pvector,
		ਅੱਖਰ *string1, ਬਿਨਾਨਿਸ਼ਾਨੀ string1_len,
		ਅੱਖਰ *string2, ਬਿਨਾਨਿਸ਼ਾਨੀ string2_len)
{
	ਅੱਖਰ ਆਖਰੀ = '0';
	ਬਿਨਾਨਿਸ਼ਾਨੀ i, j;

	ਲਈ (j = 0, i = 0; i < string1_len; i++) {
		ਜੇ (string2_len <= j)
			pvector[(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(string1[i])] = ਆਖਰੀ;
		ਨਹੀਂਤਾਂ
			pvector[(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(string1[i])] = ਆਖਰੀ = string2[j++];
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
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ expand(ਅੱਖਰ *arg, ਅੱਖਰ **ਬਫਰ_p)
{
	ਅੱਖਰ *ਬਫਰ = *ਬਫਰ_p;
	ਬਿਨਾਨਿਸ਼ਾਨੀ pos = 0;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ = TR_BUFSIZ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ i; /* can't be unsigned char: must be able to hold 256 */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ac;

	ਜਦੋਂ (*arg) {
		ਜੇ (pos + ASCII > ਮਾਪ) {
			ਮਾਪ += ASCII;
			*ਬਫਰ_p = ਬਫਰ = xrealloc(ਬਫਰ, ਮਾਪ);
		}
		ਜੇ (*arg == '\\') {
			ਅਚਲ ਅੱਖਰ *z;
			arg++;
			z = arg;
			ac = bb_process_escape_sequence(&z);
			arg = (ਅੱਖਰ *)z;
			arg--;
			*arg = ac;
			/*
			 * fall through, there may be a range.
			 * If not, current char will be treated anyway.
			 */
		}
		ਜੇ (arg[1] == '-') { /* "0-9..." */
			ac = arg[2];
			ਜੇ (ac == '\0') { /* "0-": copy verbatim */
				ਬਫਰ[pos++] = *arg++; /* copy '0' */
				ਜਾਰੀ; /* next iter will copy '-' and stop */
			}
			i = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) *arg;
			arg += 3; /* skip 0-9 or 0-\ */
			ਜੇ (ac == '\\') {
				ਅਚਲ ਅੱਖਰ *z;
				z = arg;
				ac = bb_process_escape_sequence(&z);
				arg = (ਅੱਖਰ *)z;
			}
			ਜਦੋਂ (i <= ac) /* ok: i is unsigned _int_ */
				ਬਫਰ[pos++] = i++;
			ਜਾਰੀ;
		}
		ਜੇ ((ENABLE_FEATURE_TR_CLASSES || ENABLE_FEATURE_TR_EQUIV)
		 && *arg == '['
		) {
			arg++;
			i = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) *arg++;
			/* "[xyz...". i=x, arg points to y */
			ਜੇ (ENABLE_FEATURE_TR_CLASSES && i == ':') { /* [:class:] */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CLO ":]\0"
				ਸਥਿਰ ਅਚਲ ਅੱਖਰ classes[] ALIGN1 =
					"alpha"CLO "alnum"CLO "digit"CLO
					"lower"CLO "upper"CLO "space"CLO
					"blank"CLO "punct"CLO "cntrl"CLO
					"xdigit"CLO;
				ਗਿਣਤੀ {
					CLASS_invalid = 0, /* we increment the retval */
					CLASS_alpha = 1,
					CLASS_alnum = 2,
					CLASS_digit = 3,
					CLASS_ਹੇਠਲਾ = 4,
					CLASS_ਉੱਤਲਾ = 5,
					CLASS_space = 6,
					CLASS_blank = 7,
					CLASS_punct = 8,
					CLASS_cntrl = 9,
					CLASS_xdigit = 10,
					//CLASS_graph = 11,
					//CLASS_print = 12,
				};
				smalluint j;
				ਅੱਖਰ *tmp;

				/* xdigit needs 8, not 7 */
				i = 7 + (arg[0] == 'x');
				tmp = xstrndup(arg, i);
				j = ਸੂਚਕਾਂਕ_ਵਿੱਚ_ਲੜੀਆਂ(classes, tmp) + 1;
				ਮੁਕਤ(tmp);

				ਜੇ (j == CLASS_invalid)
					ਜਾਓ skip_bracket;

				arg += i;
				ਜੇ (j == CLASS_alnum || j == CLASS_digit || j == CLASS_xdigit) {
					ਲਈ (i = '0'; i <= '9'; i++)
						ਬਫਰ[pos++] = i;
				}
				ਜੇ (j == CLASS_alpha || j == CLASS_alnum || j == CLASS_ਉੱਤਲਾ) {
					ਲਈ (i = 'A'; i <= 'Z'; i++)
						ਬਫਰ[pos++] = i;
				}
				ਜੇ (j == CLASS_alpha || j == CLASS_alnum || j == CLASS_ਹੇਠਲਾ) {
					ਲਈ (i = 'a'; i <= 'z'; i++)
						ਬਫਰ[pos++] = i;
				}
				ਜੇ (j == CLASS_space || j == CLASS_blank) {
					ਬਫਰ[pos++] = '\t';
					ਜੇ (j == CLASS_space) {
						ਬਫਰ[pos++] = '\n';
						ਬਫਰ[pos++] = '\v';
						ਬਫਰ[pos++] = '\f';
						ਬਫਰ[pos++] = '\r';
					}
					ਬਫਰ[pos++] = ' ';
				}
				ਜੇ (j == CLASS_punct || j == CLASS_cntrl) {
					ਲਈ (i = '\0'; i < ASCII; i++) {
						ਜੇ ((j == CLASS_punct && isprint_asciionly(i) && !ਅੱਖਰ_ਜਾਂ_ਅੰਕ(i) && !ਖਾਲੀ_ਥਾਂ_ਹੈ(i))
						 || (j == CLASS_cntrl && iscntrl(i))
						) {
							ਬਫਰ[pos++] = i;
						}
					}
				}
				ਜੇ (j == CLASS_xdigit) {
					ਲਈ (i = 'A'; i <= 'F'; i++) {
						ਬਫਰ[pos + 6] = i | 0x20;
						ਬਫਰ[pos++] = i;
					}
					pos += 6;
				}
				ਜਾਰੀ;
			}
			/* "[xyz...", i=x, arg points to y */
			ਜੇ (ENABLE_FEATURE_TR_EQUIV && i == '=') { /* [=CHAR=] */
				ਬਫਰ[pos++] = *arg; /* copy CHAR */
				ਜੇ (!arg[0] || arg[1] != '=' || arg[2] != ']')
					bb_show_usage();
				arg += 3;  /* skip CHAR=] */
				ਜਾਰੀ;
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
		ਬਫਰ[pos++] = *arg++;
	}
	ਵਾਪਸ pos;
}

/* NB: buffer is guaranteed to be at least TR_BUFSIZE
 * (which is >= ASCII) big.
 */
ਸਥਿਰ ਪੂਰਨ complement(ਅੱਖਰ *ਬਫਰ, ਪੂਰਨ ਬਫਰ_len)
{
	ਪੂਰਨ len;
	ਅੱਖਰ conv[ASCII];
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ch;

	len = 0;
	ch = '\0';
	ਜਦੋਂ (1) {
		ਜੇ (memchr(ਬਫਰ, ch, ਬਫਰ_len) == ਨਿਰਾ)
			conv[len++] = ch;
		ਜੇ (++ch == '\0')
			ਰੋਕੋ;
	}
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਬਫਰ, conv, len);
	ਵਾਪਸ len;
}

ਪੂਰਨ tr_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ tr_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
	ਪੂਰਨ i;
	smalluint opts;
	ssize_t read_chars;
	ਮਾਪ_t ਵਿੱਚ_ਸੂਚਕਾਂਕ, ਬਾਹਰਲਾ_ਸੂਚਕਾਂਕ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਆਖਰੀ = UCHAR_MAX + 1; /* not equal to any char */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ coded, c;
	ਅੱਖਰ *str1 = xmalloc(TR_BUFSIZ);
	ਅੱਖਰ *str2 = xmalloc(TR_BUFSIZ);
	ਪੂਰਨ str2_ਲੰਮਾਈ;
	ਪੂਰਨ str1_ਲੰਮਾਈ;
	ਅੱਖਰ *ਸਦਿਸ਼ = xzalloc(ASCII * 3);
	ਅੱਖਰ *invec  = ਸਦਿਸ਼ + ASCII;
	ਅੱਖਰ *outvec = ਸਦਿਸ਼ + ASCII * 2;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TR_OPT_complement   (3 << 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TR_OPT_delete       (1 << 2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TR_OPT_squeeze_reps (1 << 3)

	ਲਈ (i = 0; i < ASCII; i++) {
		ਸਦਿਸ਼[i] = i;
		/*invec[i] = outvec[i] = FALSE; - done by xzalloc */
	}

	/* -C/-c difference is that -C complements "characters",
	 * and -c complements "values" (binary bytes I guess).
	 * In POSIX locale, these are the same.
	 */

	/* '+': stop at first non-option */
	opts = getopt32(argv, "^+" "Ccds" "\0" "-1:?2");
	argv += optind;

	str1_ਲੰਮਾਈ = expand(*argv++, &str1);
	str2_ਲੰਮਾਈ = 0;
	ਜੇ (opts & TR_OPT_complement)
		str1_ਲੰਮਾਈ = complement(str1, str1_ਲੰਮਾਈ);
	ਜੇ (*argv) {
		ਜੇ (argv[0][0] == '\0')
			bb_simple_error_ਸੁਨੇਹਾ_and_die("STRING2 cannot be empty");
		str2_ਲੰਮਾਈ = expand(*argv, &str2);
		map(ਸਦਿਸ਼, str1, str1_ਲੰਮਾਈ,
				str2, str2_ਲੰਮਾਈ);
	}
	ਲਈ (i = 0; i < str1_ਲੰਮਾਈ; i++)
		invec[(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(str1[i])] = TRUE;
	ਲਈ (i = 0; i < str2_ਲੰਮਾਈ; i++)
		outvec[(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(str2[i])] = TRUE;

	ਜਾਓ ਸ਼ੁਰੂ_from;

	/* In this loop, str1 space is reused as input buffer,
	 * str2 - as output one. */
	ਲਈ (;;) {
		/* If we're out of input, flush output and read more input. */
		ਜੇ ((ssize_t)ਵਿੱਚ_ਸੂਚਕਾਂਕ == read_chars) {
			ਜੇ (ਬਾਹਰਲਾ_ਸੂਚਕਾਂਕ) {
				xwrite(STDOUT_FILENO, str2, ਬਾਹਰਲਾ_ਸੂਚਕਾਂਕ);
 ਸ਼ੁਰੂ_from:
				ਬਾਹਰਲਾ_ਸੂਚਕਾਂਕ = 0;
			}
			read_chars = ਸੁਰੱਖਿਅਤ_read(STDIN_FILENO, str1, TR_BUFSIZ);
			ਜੇ (read_chars <= 0) {
				ਜੇ (read_chars < 0)
					bb_simple_perror_ਸੁਨੇਹਾ_and_die(bb_ਸੁਨੇਹਾ_read_error);
				ਰੋਕੋ;
			}
			ਵਿੱਚ_ਸੂਚਕਾਂਕ = 0;
		}
		c = str1[ਵਿੱਚ_ਸੂਚਕਾਂਕ++];
		ਜੇ ((opts & TR_OPT_delete) && invec[c])
			ਜਾਰੀ;
		coded = ਸਦਿਸ਼[c];
		ਜੇ ((opts & TR_OPT_squeeze_reps) && ਆਖਰੀ == coded
		 && (invec[c] || outvec[coded])
		) {
			ਜਾਰੀ;
		}
		str2[ਬਾਹਰਲਾ_ਸੂਚਕਾਂਕ++] = ਆਖਰੀ = coded;
	}

	ਜੇ (ENABLE_FEATURE_CLEAN_ਉੱਪਰ) {
		ਮੁਕਤ(ਸਦਿਸ਼);
		ਮੁਕਤ(str2);
		ਮੁਕਤ(str1);
	}

	ਵਾਪਸ EXIT_SUCCESS;
}
