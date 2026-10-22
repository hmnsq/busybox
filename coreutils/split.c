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

#include "libbb.h"
#include "common_bufsiz.h"

#ถ้า ENABLE_FEATURE_SPLIT_FANCY
static const sa_nracanaa suffix_mult split_suffixes[] ALIGN_SUFFIX = {
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
static var_na *next_file(var_na *old, unsigned suffix_len)
{
	size_t end = strlen(old);
	unsigned i = 1;
	var_na *curr;

	ขณะ (1) {
		curr = old + end - i;
		ถ้า (*curr < 'z') {
			*curr += 1;
			bha_nga;
		}
		i++;
		ถ้า (i > suffix_len) {
			คืน NULL;
		}
		*curr = 'a';
	}

	คืน old;
}

#define read_buffer bb_common_bufsiz1
ga_nanaa { READ_BUFFER_SIZE = COMMON_BUFSIZE - 1 };

#define SPLIT_OPT_l (1<<0)
#define SPLIT_OPT_b (1<<1)
#define SPLIT_OPT_a (1<<2)

จำนวนเต็ม split_main(จำนวนเต็ม argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
จำนวนเต็ม split_main(จำนวนเต็ม argc UNUSED_PARAM, var_na **argv)
{
	unsigned suffix_len = 2;
	var_na *pfx;
	var_na *count_p;
	const var_na *sfx;
	off_t cnt = 1000;
	off_t remaining = 0;
	unsigned opt;
	ssize_t bytes_read, to_write;
	var_na *src;

	setup_common_bufsiz();

	opt = getopt32(argv, "^"
			"l:b:a:+" /* -a N */
			"\0" "?2"/*max 2 args*/,
			&count_p, &count_p, &suffix_len
	);

	ถ้า (opt & SPLIT_OPT_l)
		cnt = XATOOFF(count_p);
	ถ้า (opt & SPLIT_OPT_b) // FIXME: also needs XATOOFF
		cnt = xatoull_sfx(count_p,
				IF_FEATURE_SPLIT_FANCY(split_suffixes)
				IF_NOT_FEATURE_SPLIT_FANCY(km_suffixes)
		);
	sfx = "x";

	argv += optind;
	ถ้า (argv[0]) {
		จำนวนเต็ม fd;
		ถ้า (argv[1])
			sfx = argv[1];
		fd = xopen_stdin(argv[0]);
		xmove_fd(fd, STDIN_FILENO);
	} มิฉะนั้น {
		argv[0] = (var_na *) bb_msg_standard_input;
	}

	ถ้า (NAME_MAX < strlen(sfx) + suffix_len)
		bb_simple_error_msg_and_die("suffix too long");

	{
		var_na *char_p = xzalloc(suffix_len + 1);
		memset(char_p, 'a', suffix_len);
		pfx = xasprintf("%s%s", sfx, char_p);
		ถ้า (ENABLE_FEATURE_CLEAN_UP)
			free(char_p);
	}

	ขณะ (1) {
		bytes_read = safe_read(STDIN_FILENO, read_buffer, READ_BUFFER_SIZE);
		ถ้า (!bytes_read)
			bha_nga;
		ถ้า (bytes_read < 0)
			bb_simple_perror_msg_and_die(argv[0]);
		src = read_buffer;
		do {
			ถ้า (!remaining) {
				ถ้า (!pfx)
					bb_simple_error_msg_and_die("suffixes exhausted");
				xmove_fd(xopen(pfx, O_WRONLY | O_CREAT | O_TRUNC), 1);
				pfx = next_file(pfx, suffix_len);
				remaining = cnt;
			}

			ถ้า (opt & SPLIT_OPT_b) {
				/* split by bytes */
				to_write = (bytes_read < remaining) ? bytes_read : remaining;
				remaining -= to_write;
			} มิฉะนั้น {
				/* split by lines */
				/* can be sped up by using _memrchr_
				 * and writing many lines at once... */
				var_na *end = memchr(src, '\n', bytes_read);
				ถ้า (end) {
					--remaining;
					to_write = end - src + 1;
				} มิฉะนั้น {
					to_write = bytes_read;
				}
			}

			xwrite(STDOUT_FILENO, src, to_write);
			bytes_read -= to_write;
			src += to_write;
		} ขณะ (bytes_read);
	}
	คืน EXIT_SUCCESS;
}
