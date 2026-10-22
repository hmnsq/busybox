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

#ଅନ୍ତର୍ଭାଵ "libbb.h"
#ଅନ୍ତର୍ଭାଵ "common_bufsiz.h"

#ଯଦି ENABLE_FEATURE_SPLIT_FANCY
ସ୍ଥିର ଅଚଳ ସଂରଚନା suffix_mult split_suffixes[] ALIGN_SUFFIX = {
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
ସ୍ଥିର ଅକ୍ଷର *next_file(ଅକ୍ଷର *old, unsigned suffix_len)
{
	size_t end = strlen(old);
	unsigned i = 1;
	ଅକ୍ଷର *curr;

	ଯେତେବେଳେ (1) {
		curr = old + end - i;
		ଯଦି (*curr < 'z') {
			*curr += 1;
			ଭାଙ୍ଗ;
		}
		i++;
		ଯଦି (i > suffix_len) {
			ଫେରନ୍ତୁ ଖାଲି;
		}
		*curr = 'a';
	}

	ଫେରନ୍ତୁ old;
}

#define read_buffer bb_common_bufsiz1
ଗଣନା { READ_BUFFER_SIZE = COMMON_BUFSIZE - 1 };

#define SPLIT_OPT_l (1<<0)
#define SPLIT_OPT_b (1<<1)
#define SPLIT_OPT_a (1<<2)

ପୂର୍ଣ୍ଣସଂଖ୍ୟା split_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv) MAIN_EXTERNALLY_VISIBLE;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା split_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc UNUSED_PARAM, ଅକ୍ଷର **argv)
{
	unsigned suffix_len = 2;
	ଅକ୍ଷର *pfx;
	ଅକ୍ଷର *ଗଣନା_p;
	ଅଚଳ ଅକ୍ଷର *sfx;
	off_t cnt = 1000;
	off_t remaining = 0;
	unsigned opt;
	ssize_t bytes_read, to_write;
	ଅକ୍ଷର *src;

	setup_common_bufsiz();

	opt = getopt32(argv, "^"
			"l:b:a:+" /* -a N */
			"\0" "?2"/*max 2 args*/,
			&ଗଣନା_p, &ଗଣନା_p, &suffix_len
	);

	ଯଦି (opt & SPLIT_OPT_l)
		cnt = XATOOFF(ଗଣନା_p);
	ଯଦି (opt & SPLIT_OPT_b) // FIXME: also needs XATOOFF
		cnt = xatoull_sfx(ଗଣନା_p,
				IF_FEATURE_SPLIT_FANCY(split_suffixes)
				IF_NOT_FEATURE_SPLIT_FANCY(km_suffixes)
		);
	sfx = "x";

	argv += optind;
	ଯଦି (argv[0]) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;
		ଯଦି (argv[1])
			sfx = argv[1];
		fd = xopen_stdin(argv[0]);
		xmove_fd(fd, STDIN_FILENO);
	} ଅନ୍ୟଥା {
		argv[0] = (ଅକ୍ଷର *) bb_ସନ୍ଦେଶ_standard_ନିବେଶ;
	}

	ଯଦି (NAME_MAX < strlen(sfx) + suffix_len)
		bb_simple_error_ସନ୍ଦେଶ_and_die("suffix too long");

	{
		ଅକ୍ଷର *char_p = xzalloc(suffix_len + 1);
		memset(char_p, 'a', suffix_len);
		pfx = xasprintf("%s%s", sfx, char_p);
		ଯଦି (ENABLE_FEATURE_CLEAN_UP)
			ମୁକ୍ତ(char_p);
	}

	ଯେତେବେଳେ (1) {
		bytes_read = safe_read(STDIN_FILENO, read_buffer, READ_BUFFER_SIZE);
		ଯଦି (!bytes_read)
			ଭାଙ୍ଗ;
		ଯଦି (bytes_read < 0)
			bb_simple_perror_ସନ୍ଦେଶ_and_die(argv[0]);
		src = read_buffer;
		କର {
			ଯଦି (!remaining) {
				ଯଦି (!pfx)
					bb_simple_error_ସନ୍ଦେଶ_and_die("suffixes exhausted");
				xmove_fd(xopen(pfx, O_WRONLY | O_CREAT | O_TRUNC), 1);
				pfx = next_file(pfx, suffix_len);
				remaining = cnt;
			}

			ଯଦି (opt & SPLIT_OPT_b) {
				/* split by bytes */
				to_write = (bytes_read < remaining) ? bytes_read : remaining;
				remaining -= to_write;
			} ଅନ୍ୟଥା {
				/* split by lines */
				/* can be sped up by using _memrchr_
				 * and writing many lines at once... */
				ଅକ୍ଷର *end = memchr(src, '\n', bytes_read);
				ଯଦି (end) {
					--remaining;
					to_write = end - src + 1;
				} ଅନ୍ୟଥା {
					to_write = bytes_read;
				}
			}

			xwrite(STDOUT_FILENO, src, to_write);
			bytes_read -= to_write;
			src += to_write;
		} ଯେତେବେଳେ (bytes_read);
	}
	ଫେରନ୍ତୁ EXIT_SUCCESS;
}
