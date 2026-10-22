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

#समावेश "libbb.h"
#समावेश "common_bufsiz.h"

#अगर ENABLE_FEATURE_SPLIT_FANCY
अटल स्थिर काष्ठा suffix_mult split_suffixes[] ALIGN_SUFFIX = {
	{ "b", 512 },
	{ "k", 1024 },
	{ "m", 1024*1024 },
	{ "g", 1024*1024*1024 },
	{ "", 0 }
};
#पूर्ण_अगर

/* Increment the suffix part of the filename.
 * Returns NULL if we are out of filenames.
 */
अटल अक्षर *अग्र_file(अक्षर *old, अचिन्हित suffix_len)
{
	माप_प्रकार end = म_माप(old);
	अचिन्हित i = 1;
	अक्षर *curr;

	जबतक (1) {
		curr = old + end - i;
		अगर (*curr < 'z') {
			*curr += 1;
			अवरोध;
		}
		i++;
		अगर (i > suffix_len) {
			वापस शून्य;
		}
		*curr = 'a';
	}

	वापस old;
}

#घोषणा read_बफर bb_common_bufsiz1
क्रमागत { READ_बफर_आकार = COMMON_BUFSIZE - 1 };

#घोषणा SPLIT_OPT_l (1<<0)
#घोषणा SPLIT_OPT_b (1<<1)
#घोषणा SPLIT_OPT_a (1<<2)

पूर्णांक split_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक split_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	अचिन्हित suffix_len = 2;
	अक्षर *pfx;
	अक्षर *गिनती_p;
	स्थिर अक्षर *sfx;
	off_t cnt = 1000;
	off_t remaining = 0;
	अचिन्हित opt;
	ssize_t bytes_read, to_write;
	अक्षर *src;

	setup_common_bufsiz();

	opt = getopt32(argv, "^"
			"l:b:a:+" /* -a N */
			"\0" "?2"/*max 2 args*/,
			&गिनती_p, &गिनती_p, &suffix_len
	);

	अगर (opt & SPLIT_OPT_l)
		cnt = XATOOFF(गिनती_p);
	अगर (opt & SPLIT_OPT_b) // FIXME: also needs XATOOFF
		cnt = xatoull_sfx(गिनती_p,
				IF_FEATURE_SPLIT_FANCY(split_suffixes)
				IF_NOT_FEATURE_SPLIT_FANCY(km_suffixes)
		);
	sfx = "x";

	argv += optind;
	अगर (argv[0]) {
		पूर्णांक fd;
		अगर (argv[1])
			sfx = argv[1];
		fd = xopen_stdin(argv[0]);
		xmove_fd(fd, STDIN_FILENO);
	} अन्यथा {
		argv[0] = (अक्षर *) bb_संदेश_standard_निवेश;
	}

	अगर (NAME_MAX < म_माप(sfx) + suffix_len)
		bb_simple_error_संदेश_and_die("suffix too long");

	{
		अक्षर *char_p = xzalloc(suffix_len + 1);
		स_रखो(char_p, 'a', suffix_len);
		pfx = xasprintf("%s%s", sfx, char_p);
		अगर (ENABLE_FEATURE_CLEAN_UP)
			मुक्त(char_p);
	}

	जबतक (1) {
		bytes_read = safe_read(STDIN_FILENO, read_बफर, READ_बफर_आकार);
		अगर (!bytes_read)
			अवरोध;
		अगर (bytes_read < 0)
			bb_simple_perror_संदेश_and_die(argv[0]);
		src = read_बफर;
		करो {
			अगर (!remaining) {
				अगर (!pfx)
					bb_simple_error_संदेश_and_die("suffixes exhausted");
				xmove_fd(xopen(pfx, O_WRONLY | O_CREAT | O_TRUNC), 1);
				pfx = अग्र_file(pfx, suffix_len);
				remaining = cnt;
			}

			अगर (opt & SPLIT_OPT_b) {
				/* split by bytes */
				to_write = (bytes_read < remaining) ? bytes_read : remaining;
				remaining -= to_write;
			} अन्यथा {
				/* split by lines */
				/* can be sped up by using _memrchr_
				 * and writing many lines at once... */
				अक्षर *end = स_प्रथम(src, '\n', bytes_read);
				अगर (end) {
					--remaining;
					to_write = end - src + 1;
				} अन्यथा {
					to_write = bytes_read;
				}
			}

			xwrite(STDOUT_FILENO, src, to_write);
			bytes_read -= to_write;
			src += to_write;
		} जबतक (bytes_read);
	}
	वापस निकास_सफल;
}
