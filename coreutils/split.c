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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "common_bufsiz.h"

#ਜੇ ENABLE_FEATURE_ਵੰਡੋ_FANCY
ਸਥਿਰ ਅਚਲ ਬਣਤਰ suffix_mult ਵੰਡੋ_suffixes[] ALIGN_SUFFIX = {
	{ "b", 512 },
	{ "k", 1024 },
	{ "m", 1024*1024 },
	{ "g", 1024*1024*1024 },
	{ "", 0 }
};
#ਜੇ_ਅੰਤ

/* Increment the suffix part of the filename.
 * Returns NULL if we are out of filenames.
 */
ਸਥਿਰ ਅੱਖਰ *ਅਗਲਾ_file(ਅੱਖਰ *old, ਬਿਨਾਨਿਸ਼ਾਨੀ suffix_len)
{
	ਮਾਪ_t ਸਿਰਾ = ਲੜੀ_ਲੰਬਾਈ(old);
	ਬਿਨਾਨਿਸ਼ਾਨੀ i = 1;
	ਅੱਖਰ *curr;

	ਜਦੋਂ (1) {
		curr = old + ਸਿਰਾ - i;
		ਜੇ (*curr < 'z') {
			*curr += 1;
			ਰੋਕੋ;
		}
		i++;
		ਜੇ (i > suffix_len) {
			ਵਾਪਸ ਨਿਰਾ;
		}
		*curr = 'a';
	}

	ਵਾਪਸ old;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ read_ਬਫਰ bb_common_bufsiz1
ਗਿਣਤੀ { READ_ਬਫਰ_ਮਾਪ = COMMON_BUFSIZE - 1 };

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵੰਡੋ_OPT_l (1<<0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵੰਡੋ_OPT_b (1<<1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵੰਡੋ_OPT_a (1<<2)

ਪੂਰਨ ਵੰਡੋ_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ ਵੰਡੋ_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ suffix_len = 2;
	ਅੱਖਰ *pfx;
	ਅੱਖਰ *ਗਿਣਤੀ_p;
	ਅਚਲ ਅੱਖਰ *sfx;
	off_t cnt = 1000;
	off_t ਬਚਿਆ = 0;
	ਬਿਨਾਨਿਸ਼ਾਨੀ opt;
	ssize_t ਬਾਈਟਾਂ_read, ਮੰਜ਼ਿਲ_write;
	ਅੱਖਰ *src;

	ਤਿਆਰੀ_common_bufsiz();

	opt = getopt32(argv, "^"
			"l:b:a:+" /* -a N */
			"\0" "?2"/*max 2 args*/,
			&ਗਿਣਤੀ_p, &ਗਿਣਤੀ_p, &suffix_len
	);

	ਜੇ (opt & ਵੰਡੋ_OPT_l)
		cnt = XATOOFF(ਗਿਣਤੀ_p);
	ਜੇ (opt & ਵੰਡੋ_OPT_b) // FIXME: also needs XATOOFF
		cnt = xatoull_sfx(ਗਿਣਤੀ_p,
				IF_FEATURE_ਵੰਡੋ_FANCY(ਵੰਡੋ_suffixes)
				IF_NOT_FEATURE_ਵੰਡੋ_FANCY(km_suffixes)
		);
	sfx = "x";

	argv += optind;
	ਜੇ (argv[0]) {
		ਪੂਰਨ fd;
		ਜੇ (argv[1])
			sfx = argv[1];
		fd = xopen_stdin(argv[0]);
		xmove_fd(fd, STDIN_FILENO);
	} ਨਹੀਂਤਾਂ {
		argv[0] = (ਅੱਖਰ *) bb_ਸੁਨੇਹਾ_standard_ਨਿਵੇਸ਼;
	}

	ਜੇ (NAME_MAX < ਲੜੀ_ਲੰਬਾਈ(sfx) + suffix_len)
		bb_simple_error_ਸੁਨੇਹਾ_and_die("suffix too long");

	{
		ਅੱਖਰ *char_p = xzalloc(suffix_len + 1);
		memset(char_p, 'a', suffix_len);
		pfx = xasprintf("%s%s", sfx, char_p);
		ਜੇ (ENABLE_FEATURE_CLEAN_ਉੱਪਰ)
			ਮੁਕਤ(char_p);
	}

	ਜਦੋਂ (1) {
		ਬਾਈਟਾਂ_read = ਸੁਰੱਖਿਅਤ_read(STDIN_FILENO, read_ਬਫਰ, READ_ਬਫਰ_ਮਾਪ);
		ਜੇ (!ਬਾਈਟਾਂ_read)
			ਰੋਕੋ;
		ਜੇ (ਬਾਈਟਾਂ_read < 0)
			bb_simple_perror_ਸੁਨੇਹਾ_and_die(argv[0]);
		src = read_ਬਫਰ;
		ਕਰੋ {
			ਜੇ (!ਬਚਿਆ) {
				ਜੇ (!pfx)
					bb_simple_error_ਸੁਨੇਹਾ_and_die("suffixes exhausted");
				xmove_fd(xopen(pfx, O_WRONLY | O_CREAT | O_TRUNC), 1);
				pfx = ਅਗਲਾ_file(pfx, suffix_len);
				ਬਚਿਆ = cnt;
			}

			ਜੇ (opt & ਵੰਡੋ_OPT_b) {
				/* split by bytes */
				ਮੰਜ਼ਿਲ_write = (ਬਾਈਟਾਂ_read < ਬਚਿਆ) ? ਬਾਈਟਾਂ_read : ਬਚਿਆ;
				ਬਚਿਆ -= ਮੰਜ਼ਿਲ_write;
			} ਨਹੀਂਤਾਂ {
				/* split by lines */
				/* can be sped up by using _memrchr_
				 * and writing many lines at once... */
				ਅੱਖਰ *ਸਿਰਾ = memchr(src, '\n', ਬਾਈਟਾਂ_read);
				ਜੇ (ਸਿਰਾ) {
					--ਬਚਿਆ;
					ਮੰਜ਼ਿਲ_write = ਸਿਰਾ - src + 1;
				} ਨਹੀਂਤਾਂ {
					ਮੰਜ਼ਿਲ_write = ਬਾਈਟਾਂ_read;
				}
			}

			xwrite(STDOUT_FILENO, src, ਮੰਜ਼ਿਲ_write);
			ਬਾਈਟਾਂ_read -= ਮੰਜ਼ਿਲ_write;
			src += ਮੰਜ਼ਿਲ_write;
		} ਜਦੋਂ (ਬਾਈਟਾਂ_read);
	}
	ਵਾਪਸ EXIT_SUCCESS;
}
