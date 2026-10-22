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

#include "libbb.h"

enum {
	ASCII = 256,
	/* string buffer needs to be at least as big as the whole "alphabet".
	 * BUFSIZ == ASCII is ok, but we will realloc in expand
	 * even for smallest patterns, let's avoid that by using *2:
	 */
	TR_BUFSIZ = (BUFSIZ > ASCII*2) ? BUFSIZ : ASCII*2,
};

static void map(char *pvector,
		char *string1, unsigned string1_len,
		char *string2, unsigned string2_len)
{
	char last = '0';
	unsigned i, j;

	위해 (j = 0, i = 0; i < string1_len; i++) {
		만약 (string2_len <= j)
			pvector[(unsigned char)(string1[i])] = last;
		아니면
			pvector[(unsigned char)(string1[i])] = last = string2[j++];
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
static unsigned expand(char *arg, char **buffer_p)
{
	char *buffer = *buffer_p;
	unsigned pos = 0;
	unsigned size = TR_BUFSIZ;
	unsigned i; /* can't be unsigned char: must be able to hold 256 */
	unsigned char ac;

	동안 (*arg) {
		만약 (pos + ASCII > size) {
			size += ASCII;
			*buffer_p = buffer = xrealloc(buffer, size);
		}
		만약 (*arg == '\\') {
			const char *z;
			arg++;
			z = arg;
			ac = bb_process_escape_sequence(&z);
			arg = (char *)z;
			arg--;
			*arg = ac;
			/*
			 * fall through, there may be a range.
			 * If not, current char will be treated anyway.
			 */
		}
		만약 (arg[1] == '-') { /* "0-9..." */
			ac = arg[2];
			만약 (ac == '\0') { /* "0-": copy verbatim */
				buffer[pos++] = *arg++; /* copy '0' */
				continue; /* next iter will copy '-' and stop */
			}
			i = (unsigned char) *arg;
			arg += 3; /* skip 0-9 or 0-\ */
			만약 (ac == '\\') {
				const char *z;
				z = arg;
				ac = bb_process_escape_sequence(&z);
				arg = (char *)z;
			}
			동안 (i <= ac) /* ok: i is unsigned _int_ */
				buffer[pos++] = i++;
			continue;
		}
		만약 ((ENABLE_FEATURE_TR_CLASSES || ENABLE_FEATURE_TR_EQUIV)
		 && *arg == '['
		) {
			arg++;
			i = (unsigned char) *arg++;
			/* "[xyz...". i=x, arg points to y */
			만약 (ENABLE_FEATURE_TR_CLASSES && i == ':') { /* [:class:] */
#define CLO ":]\0"
				static const char classes[] ALIGN1 =
					"alpha"CLO "alnum"CLO "digit"CLO
					"lower"CLO "upper"CLO "space"CLO
					"blank"CLO "punct"CLO "cntrl"CLO
					"xdigit"CLO;
				enum {
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
				char *tmp;

				/* xdigit needs 8, not 7 */
				i = 7 + (arg[0] == 'x');
				tmp = xstrndup(arg, i);
				j = index_in_strings(classes, tmp) + 1;
				free(tmp);

				만약 (j == CLASS_invalid)
					goto skip_bracket;

				arg += i;
				만약 (j == CLASS_alnum || j == CLASS_digit || j == CLASS_xdigit) {
					위해 (i = '0'; i <= '9'; i++)
						buffer[pos++] = i;
				}
				만약 (j == CLASS_alpha || j == CLASS_alnum || j == CLASS_upper) {
					위해 (i = 'A'; i <= 'Z'; i++)
						buffer[pos++] = i;
				}
				만약 (j == CLASS_alpha || j == CLASS_alnum || j == CLASS_lower) {
					위해 (i = 'a'; i <= 'z'; i++)
						buffer[pos++] = i;
				}
				만약 (j == CLASS_space || j == CLASS_blank) {
					buffer[pos++] = '\t';
					만약 (j == CLASS_space) {
						buffer[pos++] = '\n';
						buffer[pos++] = '\v';
						buffer[pos++] = '\f';
						buffer[pos++] = '\r';
					}
					buffer[pos++] = ' ';
				}
				만약 (j == CLASS_punct || j == CLASS_cntrl) {
					위해 (i = '\0'; i < ASCII; i++) {
						만약 ((j == CLASS_punct && isprint_asciionly(i) && !isalnum(i) && !isspace(i))
						 || (j == CLASS_cntrl && iscntrl(i))
						) {
							buffer[pos++] = i;
						}
					}
				}
				만약 (j == CLASS_xdigit) {
					위해 (i = 'A'; i <= 'F'; i++) {
						buffer[pos + 6] = i | 0x20;
						buffer[pos++] = i;
					}
					pos += 6;
				}
				continue;
			}
			/* "[xyz...", i=x, arg points to y */
			만약 (ENABLE_FEATURE_TR_EQUIV && i == '=') { /* [=CHAR=] */
				buffer[pos++] = *arg; /* copy CHAR */
				만약 (!arg[0] || arg[1] != '=' || arg[2] != ']')
					bb_show_usage();
				arg += 3;  /* skip CHAR=] */
				continue;
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
	반환 pos;
}

/* NB: buffer is guaranteed to be at least TR_BUFSIZE
 * (which is >= ASCII) big.
 */
static 정수 complement(char *buffer, 정수 buffer_len)
{
	정수 len;
	char conv[ASCII];
	unsigned char ch;

	len = 0;
	ch = '\0';
	동안 (1) {
		만약 (memchr(buffer, ch, buffer_len) == NULL)
			conv[len++] = ch;
		만약 (++ch == '\0')
			break;
	}
	memcpy(buffer, conv, len);
	반환 len;
}

정수 tr_main(정수 argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
정수 tr_main(정수 argc UNUSED_PARAM, char **argv)
{
	정수 i;
	smalluint opts;
	ssize_t read_chars;
	size_t in_index, out_index;
	unsigned last = UCHAR_MAX + 1; /* not equal to any char */
	unsigned char coded, c;
	char *str1 = xmalloc(TR_BUFSIZ);
	char *str2 = xmalloc(TR_BUFSIZ);
	정수 str2_length;
	정수 str1_length;
	char *vector = xzalloc(ASCII * 3);
	char *invec  = vector + ASCII;
	char *outvec = vector + ASCII * 2;

#define TR_OPT_complement   (3 << 0)
#define TR_OPT_delete       (1 << 2)
#define TR_OPT_squeeze_reps (1 << 3)

	위해 (i = 0; i < ASCII; i++) {
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
	만약 (opts & TR_OPT_complement)
		str1_length = complement(str1, str1_length);
	만약 (*argv) {
		만약 (argv[0][0] == '\0')
			bb_simple_error_msg_and_die("STRING2 cannot be empty");
		str2_length = expand(*argv, &str2);
		map(vector, str1, str1_length,
				str2, str2_length);
	}
	위해 (i = 0; i < str1_length; i++)
		invec[(unsigned char)(str1[i])] = TRUE;
	위해 (i = 0; i < str2_length; i++)
		outvec[(unsigned char)(str2[i])] = TRUE;

	goto start_from;

	/* In this loop, str1 space is reused as input buffer,
	 * str2 - as output one. */
	위해 (;;) {
		/* If we're out of input, flush output and read more input. */
		만약 ((ssize_t)in_index == read_chars) {
			만약 (out_index) {
				xwrite(STDOUT_FILENO, str2, out_index);
 start_from:
				out_index = 0;
			}
			read_chars = safe_read(STDIN_FILENO, str1, TR_BUFSIZ);
			만약 (read_chars <= 0) {
				만약 (read_chars < 0)
					bb_simple_perror_msg_and_die(bb_msg_read_error);
				break;
			}
			in_index = 0;
		}
		c = str1[in_index++];
		만약 ((opts & TR_OPT_delete) && invec[c])
			continue;
		coded = vector[c];
		만약 ((opts & TR_OPT_squeeze_reps) && last == coded
		 && (invec[c] || outvec[coded])
		) {
			continue;
		}
		str2[out_index++] = last = coded;
	}

	만약 (ENABLE_FEATURE_CLEAN_UP) {
		free(vector);
		free(str2);
		free(str1);
	}

	반환 EXIT_SUCCESS;
}
