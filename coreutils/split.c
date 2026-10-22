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

#அந்தர்பாவ "libbb.h"
#அந்தர்பாவ "common_bufsiz.h"

#எனில் ENABLE_FEATURE_SPLIT_FANCY
நிலையான மாறிலி கட்டமைப்பு suffix_mult split_suffixes[] ALIGN_SUFFIX = {
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
நிலையான எழுத்து *next_file(எழுத்து *old, unsigned suffix_len)
{
	size_t end = strlen(old);
	unsigned i = 1;
	எழுத்து *curr;

	போது (1) {
		curr = old + end - i;
		எனில் (*curr < 'z') {
			*curr += 1;
			நிறுத்து;
		}
		i++;
		எனில் (i > suffix_len) {
			திரும்பு வெற்று;
		}
		*curr = 'a';
	}

	திரும்பு old;
}

#define read_buffer bb_common_bufsiz1
எண்ணிக்கை { READ_BUFFER_SIZE = COMMON_BUFSIZE - 1 };

#define SPLIT_OPT_l (1<<0)
#define SPLIT_OPT_b (1<<1)
#define SPLIT_OPT_a (1<<2)

முழு split_main(முழு argc, எழுத்து **argv) MAIN_EXTERNALLY_VISIBLE;
முழு split_main(முழு argc UNUSED_PARAM, எழுத்து **argv)
{
	unsigned suffix_len = 2;
	எழுத்து *pfx;
	எழுத்து *மொத்தம்_p;
	மாறிலி எழுத்து *sfx;
	off_t cnt = 1000;
	off_t remaining = 0;
	unsigned opt;
	ssize_t bytes_read, to_write;
	எழுத்து *src;

	setup_common_bufsiz();

	opt = getopt32(argv, "^"
			"l:b:a:+" /* -a N */
			"\0" "?2"/*max 2 args*/,
			&மொத்தம்_p, &மொத்தம்_p, &suffix_len
	);

	எனில் (opt & SPLIT_OPT_l)
		cnt = XATOOFF(மொத்தம்_p);
	எனில் (opt & SPLIT_OPT_b) // FIXME: also needs XATOOFF
		cnt = xatoull_sfx(மொத்தம்_p,
				IF_FEATURE_SPLIT_FANCY(split_suffixes)
				IF_NOT_FEATURE_SPLIT_FANCY(km_suffixes)
		);
	sfx = "x";

	argv += optind;
	எனில் (argv[0]) {
		முழு fd;
		எனில் (argv[1])
			sfx = argv[1];
		fd = xopen_stdin(argv[0]);
		xmove_fd(fd, STDIN_FILENO);
	} இல்லையெனில் {
		argv[0] = (எழுத்து *) bb_செய்தி_standard_உள்ளீடு;
	}

	எனில் (NAME_MAX < strlen(sfx) + suffix_len)
		bb_simple_error_செய்தி_and_die("suffix too long");

	{
		எழுத்து *char_p = xzalloc(suffix_len + 1);
		memset(char_p, 'a', suffix_len);
		pfx = xasprintf("%s%s", sfx, char_p);
		எனில் (ENABLE_FEATURE_CLEAN_UP)
			விடுவி(char_p);
	}

	போது (1) {
		bytes_read = safe_read(STDIN_FILENO, read_buffer, READ_BUFFER_SIZE);
		எனில் (!bytes_read)
			நிறுத்து;
		எனில் (bytes_read < 0)
			bb_simple_perror_செய்தி_and_die(argv[0]);
		src = read_buffer;
		செய் {
			எனில் (!remaining) {
				எனில் (!pfx)
					bb_simple_error_செய்தி_and_die("suffixes exhausted");
				xmove_fd(xopen(pfx, O_WRONLY | O_CREAT | O_TRUNC), 1);
				pfx = next_file(pfx, suffix_len);
				remaining = cnt;
			}

			எனில் (opt & SPLIT_OPT_b) {
				/* split by bytes */
				to_write = (bytes_read < remaining) ? bytes_read : remaining;
				remaining -= to_write;
			} இல்லையெனில் {
				/* split by lines */
				/* can be sped up by using _memrchr_
				 * and writing many lines at once... */
				எழுத்து *end = memchr(src, '\n', bytes_read);
				எனில் (end) {
					--remaining;
					to_write = end - src + 1;
				} இல்லையெனில் {
					to_write = bytes_read;
				}
			}

			xwrite(STDOUT_FILENO, src, to_write);
			bytes_read -= to_write;
			src += to_write;
		} போது (bytes_read);
	}
	திரும்பு EXIT_SUCCESS;
}
