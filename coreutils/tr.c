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

#شامل "libbb.h"

شمار {
	ASCII = 256,
	/* string buffer needs to be at least as big as the whole "alphabet".
	 * BUFSIZ == ASCII is ok, but we will realloc in expand
	 * even for smallest patterns, let's avoid that by using *2:
	 */
	TR_BUFSIZ = (BUFSIZ > ASCII*2) ? BUFSIZ : ASCII*2,
};

ساکن خالی map(حرف *pvector,
		حرف *string1, غیرعلامتی string1_len,
		حرف *string2, غیرعلامتی string2_len)
{
	حرف آخری = '0';
	غیرعلامتی i, j;

	برائے (j = 0, i = 0; i < string1_len; i++) {
		اگر (string2_len <= j)
			pvector[(غیرعلامتی حرف)(string1[i])] = آخری;
		ورنہ
			pvector[(غیرعلامتی حرف)(string1[i])] = آخری = string2[j++];
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
ساکن غیرعلامتی expand(حرف *arg, حرف **بفر_p)
{
	حرف *بفر = *بفر_p;
	غیرعلامتی pos = 0;
	غیرعلامتی سائز = TR_BUFSIZ;
	غیرعلامتی i; /* can't be unsigned char: must be able to hold 256 */
	غیرعلامتی حرف ac;

	جب (*arg) {
		اگر (pos + ASCII > سائز) {
			سائز += ASCII;
			*بفر_p = بفر = xrealloc(بفر, سائز);
		}
		اگر (*arg == '\\') {
			ثابت حرف *z;
			arg++;
			z = arg;
			ac = bb_process_escape_sequence(&z);
			arg = (حرف *)z;
			arg--;
			*arg = ac;
			/*
			 * fall through, there may be a range.
			 * If not, current char will be treated anyway.
			 */
		}
		اگر (arg[1] == '-') { /* "0-9..." */
			ac = arg[2];
			اگر (ac == '\0') { /* "0-": copy verbatim */
				بفر[pos++] = *arg++; /* copy '0' */
				جاری; /* next iter will copy '-' and stop */
			}
			i = (غیرعلامتی حرف) *arg;
			arg += 3; /* skip 0-9 or 0-\ */
			اگر (ac == '\\') {
				ثابت حرف *z;
				z = arg;
				ac = bb_process_escape_sequence(&z);
				arg = (حرف *)z;
			}
			جب (i <= ac) /* ok: i is unsigned _int_ */
				بفر[pos++] = i++;
			جاری;
		}
		اگر ((ENABLE_FEATURE_TR_CLASSES || ENABLE_FEATURE_TR_EQUIV)
		 && *arg == '['
		) {
			arg++;
			i = (غیرعلامتی حرف) *arg++;
			/* "[xyz...". i=x, arg points to y */
			اگر (ENABLE_FEATURE_TR_CLASSES && i == ':') { /* [:class:] */
#تعریف_کرو CLO ":]\0"
				ساکن ثابت حرف classes[] ALIGN1 =
					"alpha"CLO "alnum"CLO "digit"CLO
					"lower"CLO "upper"CLO "space"CLO
					"blank"CLO "punct"CLO "cntrl"CLO
					"xdigit"CLO;
				شمار {
					CLASS_invalid = 0, /* we increment the retval */
					CLASS_alpha = 1,
					CLASS_alnum = 2,
					CLASS_digit = 3,
					CLASS_زیریں = 4,
					CLASS_بالائی = 5,
					CLASS_space = 6,
					CLASS_blank = 7,
					CLASS_punct = 8,
					CLASS_cntrl = 9,
					CLASS_xdigit = 10,
					//CLASS_graph = 11,
					//CLASS_print = 12,
				};
				smalluint j;
				حرف *tmp;

				/* xdigit needs 8, not 7 */
				i = 7 + (arg[0] == 'x');
				tmp = xstrndup(arg, i);
				j = اشاریہ_اندر_لڑیاں(classes, tmp) + 1;
				آزاد(tmp);

				اگر (j == CLASS_invalid)
					جاؤ skip_bracket;

				arg += i;
				اگر (j == CLASS_alnum || j == CLASS_digit || j == CLASS_xdigit) {
					برائے (i = '0'; i <= '9'; i++)
						بفر[pos++] = i;
				}
				اگر (j == CLASS_alpha || j == CLASS_alnum || j == CLASS_بالائی) {
					برائے (i = 'A'; i <= 'Z'; i++)
						بفر[pos++] = i;
				}
				اگر (j == CLASS_alpha || j == CLASS_alnum || j == CLASS_زیریں) {
					برائے (i = 'a'; i <= 'z'; i++)
						بفر[pos++] = i;
				}
				اگر (j == CLASS_space || j == CLASS_blank) {
					بفر[pos++] = '\t';
					اگر (j == CLASS_space) {
						بفر[pos++] = '\n';
						بفر[pos++] = '\v';
						بفر[pos++] = '\f';
						بفر[pos++] = '\r';
					}
					بفر[pos++] = ' ';
				}
				اگر (j == CLASS_punct || j == CLASS_cntrl) {
					برائے (i = '\0'; i < ASCII; i++) {
						اگر ((j == CLASS_punct && isprint_asciionly(i) && !حرف_یا_ہندسہ(i) && !خلا_ہے(i))
						 || (j == CLASS_cntrl && iscntrl(i))
						) {
							بفر[pos++] = i;
						}
					}
				}
				اگر (j == CLASS_xdigit) {
					برائے (i = 'A'; i <= 'F'; i++) {
						بفر[pos + 6] = i | 0x20;
						بفر[pos++] = i;
					}
					pos += 6;
				}
				جاری;
			}
			/* "[xyz...", i=x, arg points to y */
			اگر (ENABLE_FEATURE_TR_EQUIV && i == '=') { /* [=CHAR=] */
				بفر[pos++] = *arg; /* copy CHAR */
				اگر (!arg[0] || arg[1] != '=' || arg[2] != ']')
					bb_show_usage();
				arg += 3;  /* skip CHAR=] */
				جاری;
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
		بفر[pos++] = *arg++;
	}
	واپس pos;
}

/* NB: buffer is guaranteed to be at least TR_BUFSIZE
 * (which is >= ASCII) big.
 */
ساکن صحیح complement(حرف *بفر, صحیح بفر_len)
{
	صحیح len;
	حرف conv[ASCII];
	غیرعلامتی حرف ch;

	len = 0;
	ch = '\0';
	جب (1) {
		اگر (memchr(بفر, ch, بفر_len) == تہی)
			conv[len++] = ch;
		اگر (++ch == '\0')
			توڑو;
	}
	حافظہ_نقل(بفر, conv, len);
	واپس len;
}

صحیح tr_main(صحیح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح tr_main(صحیح argc غیر_مستعمل_PARAM, حرف **argv)
{
	صحیح i;
	smalluint opts;
	ssize_t read_chars;
	سائز_t اندر_اشاریہ, برآمد_اشاریہ;
	غیرعلامتی آخری = UCHAR_MAX + 1; /* not equal to any char */
	غیرعلامتی حرف coded, c;
	حرف *str1 = xmalloc(TR_BUFSIZ);
	حرف *str2 = xmalloc(TR_BUFSIZ);
	صحیح str2_لمبائی;
	صحیح str1_لمبائی;
	حرف *سمتیہ = xzalloc(ASCII * 3);
	حرف *invec  = سمتیہ + ASCII;
	حرف *outvec = سمتیہ + ASCII * 2;

#تعریف_کرو TR_OPT_complement   (3 << 0)
#تعریف_کرو TR_OPT_delete       (1 << 2)
#تعریف_کرو TR_OPT_squeeze_reps (1 << 3)

	برائے (i = 0; i < ASCII; i++) {
		سمتیہ[i] = i;
		/*invec[i] = outvec[i] = FALSE; - done by xzalloc */
	}

	/* -C/-c difference is that -C complements "characters",
	 * and -c complements "values" (binary bytes I guess).
	 * In POSIX locale, these are the same.
	 */

	/* '+': stop at first non-option */
	opts = getopt32(argv, "^+" "Ccds" "\0" "-1:?2");
	argv += optind;

	str1_لمبائی = expand(*argv++, &str1);
	str2_لمبائی = 0;
	اگر (opts & TR_OPT_complement)
		str1_لمبائی = complement(str1, str1_لمبائی);
	اگر (*argv) {
		اگر (argv[0][0] == '\0')
			bb_simple_error_پیغام_and_die("STRING2 cannot be empty");
		str2_لمبائی = expand(*argv, &str2);
		map(سمتیہ, str1, str1_لمبائی,
				str2, str2_لمبائی);
	}
	برائے (i = 0; i < str1_لمبائی; i++)
		invec[(غیرعلامتی حرف)(str1[i])] = TRUE;
	برائے (i = 0; i < str2_لمبائی; i++)
		outvec[(غیرعلامتی حرف)(str2[i])] = TRUE;

	جاؤ آغاز_from;

	/* In this loop, str1 space is reused as input buffer,
	 * str2 - as output one. */
	برائے (;;) {
		/* If we're out of input, flush output and read more input. */
		اگر ((ssize_t)اندر_اشاریہ == read_chars) {
			اگر (برآمد_اشاریہ) {
				xwrite(STDOUT_FILENO, str2, برآمد_اشاریہ);
 آغاز_from:
				برآمد_اشاریہ = 0;
			}
			read_chars = محفوظ_read(STDIN_FILENO, str1, TR_BUFSIZ);
			اگر (read_chars <= 0) {
				اگر (read_chars < 0)
					bb_simple_perror_پیغام_and_die(bb_پیغام_read_error);
				توڑو;
			}
			اندر_اشاریہ = 0;
		}
		c = str1[اندر_اشاریہ++];
		اگر ((opts & TR_OPT_delete) && invec[c])
			جاری;
		coded = سمتیہ[c];
		اگر ((opts & TR_OPT_squeeze_reps) && آخری == coded
		 && (invec[c] || outvec[coded])
		) {
			جاری;
		}
		str2[برآمد_اشاریہ++] = آخری = coded;
	}

	اگر (ENABLE_FEATURE_CLEAN_اوپر) {
		آزاد(سمتیہ);
		آزاد(str2);
		آزاد(str1);
	}

	واپس EXIT_SUCCESS;
}
