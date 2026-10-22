/* vi: set sw=4 ts=4: */
/*
 * split - split a file into pieces
 * Copyright (c) 2007 Bernhard Reutner-Fischer
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config SPLIT
//config:	bool "split (5.2 kb)"
//config:	default y
//config:	help
//config:	Split a file into pieces.
//config:
//config:config FEATURE_SPLIT_FANCY
//config:	bool "Fancy extensions"
//config:	default y
//config:	depends on SPLIT
//config:	help
//config:	Add support for features not required by SUSv3.
//config:	Supports additional suffixes 'b' for 512 bytes,
//config:	'g' for 1GiB for the -b option.

//applet:IF_SPLIT(APPLET(split, BB_DIR_USR_BIN, BB_SUID_DROP))

//kbuild:lib-$(CONFIG_SPLIT) += split.o

/* BB_AUDIT: SUSv3 compliant
 * SUSv3 requirements:
 * http://www.opengroup.org/onlinepubs/009695399/utilities/split.html
 */

//usage:#define split_trivial_usage
//usage:       "[OPTIONS] [INPUT [PREFIX]]"
//usage:#define split_full_usage "\n\n"
//usage:       "	-b N[k|m]	Split by N (kilo|mega)bytes"
//usage:     "\n	-l N		Split by N lines"
//usage:     "\n	-a N		Use N letters as suffix"
//usage:
//usage:#define split_example_usage
//usage:       "$ split TODO foo\n"
//usage:       "$ cat TODO | split -a 2 -l 2 TODO_\n"

#अन्तर्भाव "libbb.h"
#अन्तर्भाव "common_bufsiz.h"

#यदि ENABLE_FEATURE_विभज_FANCY
स्थिर ध्रुव संरचना suffix_mult विभज_suffixes[] ALIGN_SUFFIX = {
	{ "b", 512 },
	{ "k", 1024 },
	{ "m", 1024*1024 },
	{ "g", 1024*1024*1024 },
	{ "", 0 }
};
#यदि_अन्तः

/* Increment the suffix part of the filename.
 * Returns NULL if we are out of filenames.
 */
स्थिर वर्ण *अग्र_file(वर्ण *old, अचिह्नित suffix_len)
{
	आकार_t अन्तः = वाक्यदैर्घ्यम्(old);
	अचिह्नित i = 1;
	वर्ण *curr;

	यावत् (1) {
		curr = old + अन्तः - i;
		यदि (*curr < 'z') {
			*curr += 1;
			भङ्ग;
		}
		i++;
		यदि (i > suffix_len) {
			प्रत्यागम रिक्त;
		}
		*curr = 'a';
	}

	प्रत्यागम old;
}

#परिभाषय read_बफर bb_common_bufsiz1
गणना { READ_बफर_आकार = COMMON_BUFSIZE - 1 };

#परिभाषय विभज_OPT_l (1<<0)
#परिभाषय विभज_OPT_b (1<<1)
#परिभाषय विभज_OPT_a (1<<2)

पूर्णाङ्क विभज_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क विभज_main(पूर्णाङ्क argc अप्रयुक्तम्_PARAM, वर्ण **argv)
{
	अचिह्नित suffix_len = 2;
	वर्ण *pfx;
	वर्ण *गणनम्_p;
	ध्रुव वर्ण *sfx;
	off_t cnt = 1000;
	off_t remaining = 0;
	अचिह्नित opt;
	ssize_t अष्टकानि_read, गन्तव्यम्_write;
	वर्ण *src;

	सज्जता_common_bufsiz();

	opt = getopt32(argv, "^"
			"l:b:a:+" /* -a N */
			"\0" "?2"/*max 2 args*/,
			&गणनम्_p, &गणनम्_p, &suffix_len
	);

	यदि (opt & विभज_OPT_l)
		cnt = XATOOFF(गणनम्_p);
	यदि (opt & विभज_OPT_b) // FIXME: also needs XATOOFF
		cnt = xatoull_sfx(गणनम्_p,
				IF_FEATURE_विभज_FANCY(विभज_suffixes)
				IF_NOT_FEATURE_विभज_FANCY(km_suffixes)
		);
	sfx = "x";

	argv += optind;
	यदि (argv[0]) {
		पूर्णाङ्क fd;
		यदि (argv[1])
			sfx = argv[1];
		fd = xopen_stdin(argv[0]);
		xmove_fd(fd, STDIN_FILENO);
	} अन्यथा {
		argv[0] = (वर्ण *) bb_सन्देशः_standard_निवेशः;
	}

	यदि (NAME_MAX < वाक्यदैर्घ्यम्(sfx) + suffix_len)
		bb_simple_error_सन्देशः_and_die("suffix too long");

	{
		वर्ण *char_p = xzalloc(suffix_len + 1);
		स्मृतिं_पूरय(char_p, 'a', suffix_len);
		pfx = xasprintf("%s%s", sfx, char_p);
		यदि (ENABLE_FEATURE_CLEAN_ऊर्ध्वम्)
			मुक्त(char_p);
	}

	यावत् (1) {
		अष्टकानि_read = safe_read(STDIN_FILENO, read_बफर, READ_बफर_आकार);
		यदि (!अष्टकानि_read)
			भङ्ग;
		यदि (अष्टकानि_read < 0)
			bb_simple_perror_सन्देशः_and_die(argv[0]);
		src = read_बफर;
		कुरु {
			यदि (!remaining) {
				यदि (!pfx)
					bb_simple_error_सन्देशः_and_die("suffixes exhausted");
				xmove_fd(xopen(pfx, O_WRONLY | O_CREAT | O_TRUNC), 1);
				pfx = अग्र_file(pfx, suffix_len);
				remaining = cnt;
			}

			यदि (opt & विभज_OPT_b) {
				/* split by bytes */
				गन्तव्यम्_write = (अष्टकानि_read < remaining) ? अष्टकानि_read : remaining;
				remaining -= गन्तव्यम्_write;
			} अन्यथा {
				/* split by lines */
				/* can be sped up by using _memrchr_
				 * and writing many lines at once... */
				वर्ण *अन्तः = memchr(src, '\n', अष्टकानि_read);
				यदि (अन्तः) {
					--remaining;
					गन्तव्यम्_write = अन्तः - src + 1;
				} अन्यथा {
					गन्तव्यम्_write = अष्टकानि_read;
				}
			}

			xwrite(STDOUT_FILENO, src, गन्तव्यम्_write);
			अष्टकानि_read -= गन्तव्यम्_write;
			src += गन्तव्यम्_write;
		} यावत् (अष्टकानि_read);
	}
	प्रत्यागम EXIT_SUCCESS;
}
