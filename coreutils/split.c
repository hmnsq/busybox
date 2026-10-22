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

#অন্তর্ভাব "libbb.h"
#অন্তর্ভাব "common_bufsiz.h"

#যদি ENABLE_FEATURE_SPLIT_FANCY
স্থির ধ্রুব সংরচনা suffix_mult split_suffixes[] ALIGN_SUFFIX = {
	{ "b", 512 },
	{ "k", 1024 },
	{ "m", 1024*1024 },
	{ "g", 1024*1024*1024 },
	{ "", 0 }
};
#endif

/* Increment the suffix part of the filename.
 * Returns NULL if we are out of filenames.
 */
স্থির বর্ণ *next_file(বর্ণ *old, unsigned suffix_len)
{
	size_t end = strlen(old);
	unsigned i = 1;
	বর্ণ *curr;

	যাবত্ (1) {
		curr = old + end - i;
		যদি (*curr < 'z') {
			*curr += 1;
			ভঙ্গ;
		}
		i++;
		যদি (i > suffix_len) {
			প্রত্যাগম রিক্ত;
		}
		*curr = 'a';
	}

	প্রত্যাগম old;
}

#define read_buffer bb_common_bufsiz1
গণনা { READ_BUFFER_SIZE = COMMON_BUFSIZE - 1 };

#define SPLIT_OPT_l (1<<0)
#define SPLIT_OPT_b (1<<1)
#define SPLIT_OPT_a (1<<2)

পূর্ণাঙ্ক split_main(পূর্ণাঙ্ক argc, বর্ণ **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণাঙ্ক split_main(পূর্ণাঙ্ক argc UNUSED_PARAM, বর্ণ **argv)
{
	unsigned suffix_len = 2;
	বর্ণ *pfx;
	বর্ণ *গুনতি_p;
	ধ্রুব বর্ণ *sfx;
	off_t cnt = 1000;
	off_t remaining = 0;
	unsigned opt;
	ssize_t bytes_read, to_write;
	বর্ণ *src;

	setup_common_bufsiz();

	opt = getopt32(argv, "^"
			"l:b:a:+" /* -a N */
			"\0" "?2"/*max 2 args*/,
			&গুনতি_p, &গুনতি_p, &suffix_len
	);

	যদি (opt & SPLIT_OPT_l)
		cnt = XATOOFF(গুনতি_p);
	যদি (opt & SPLIT_OPT_b) // FIXME: also needs XATOOFF
		cnt = xatoull_sfx(গুনতি_p,
				IF_FEATURE_SPLIT_FANCY(split_suffixes)
				IF_NOT_FEATURE_SPLIT_FANCY(km_suffixes)
		);
	sfx = "x";

	argv += optind;
	যদি (argv[0]) {
		পূর্ণাঙ্ক fd;
		যদি (argv[1])
			sfx = argv[1];
		fd = xopen_stdin(argv[0]);
		xmove_fd(fd, STDIN_FILENO);
	} অন্যথা {
		argv[0] = (বর্ণ *) bb_বার্তা_standard_প্রবেশ;
	}

	যদি (NAME_MAX < strlen(sfx) + suffix_len)
		bb_simple_error_বার্তা_and_die("suffix too long");

	{
		বর্ণ *char_p = xzalloc(suffix_len + 1);
		memset(char_p, 'a', suffix_len);
		pfx = xasprintf("%s%s", sfx, char_p);
		যদি (ENABLE_FEATURE_CLEAN_UP)
			মুক্ত(char_p);
	}

	যাবত্ (1) {
		bytes_read = safe_read(STDIN_FILENO, read_buffer, READ_BUFFER_SIZE);
		যদি (!bytes_read)
			ভঙ্গ;
		যদি (bytes_read < 0)
			bb_simple_perror_বার্তা_and_die(argv[0]);
		src = read_buffer;
		কুরু {
			যদি (!remaining) {
				যদি (!pfx)
					bb_simple_error_বার্তা_and_die("suffixes exhausted");
				xmove_fd(xopen(pfx, O_WRONLY | O_CREAT | O_TRUNC), 1);
				pfx = next_file(pfx, suffix_len);
				remaining = cnt;
			}

			যদি (opt & SPLIT_OPT_b) {
				/* split by bytes */
				to_write = (bytes_read < remaining) ? bytes_read : remaining;
				remaining -= to_write;
			} অন্যথা {
				/* split by lines */
				/* can be sped up by using _memrchr_
				 * and writing many lines at once... */
				বর্ণ *end = memchr(src, '\n', bytes_read);
				যদি (end) {
					--remaining;
					to_write = end - src + 1;
				} অন্যথা {
					to_write = bytes_read;
				}
			}

			xwrite(STDOUT_FILENO, src, to_write);
			bytes_read -= to_write;
			src += to_write;
		} যাবত্ (bytes_read);
	}
	প্রত্যাগম EXIT_SUCCESS;
}
