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

#ಅನ್ತರ್ಭಾವ "libbb.h"
#ಅನ್ತರ್ಭಾವ "common_bufsiz.h"

#ಆದರೆ ENABLE_FEATURE_ವಿಭಜಿಸು_FANCY
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ suffix_mult ವಿಭಜಿಸು_suffixes[] ALIGN_SUFFIX = {
	{ "b", 512 },
	{ "k", 1024 },
	{ "m", 1024*1024 },
	{ "g", 1024*1024*1024 },
	{ "", 0 }
};
#ಆದರೆ_ಅಂತ್ಯ

/* Increment the suffix part of the filename.
 * Returns NULL if we are out of filenames.
 */
ಸ್ಥಿರ ಅಕ್ಷರ *ಮುಂದಿನ_file(ಅಕ್ಷರ *old, ಚಿಹ್ನೆರಹಿತ suffix_len)
{
	ಗಾತ್ರ_t ಕೊನೆ = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(old);
	ಚಿಹ್ನೆರಹಿತ i = 1;
	ಅಕ್ಷರ *curr;

	ಆಗ (1) {
		curr = old + ಕೊನೆ - i;
		ಆದರೆ (*curr < 'z') {
			*curr += 1;
			ನಿಲ್ಲಿಸು;
		}
		i++;
		ಆದರೆ (i > suffix_len) {
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}
		*curr = 'a';
	}

	ಹಿಂತಿರುಗಿ old;
}

#ವ್ಯಾಖ್ಯಾನಿಸು read_ಬಫರ್ bb_common_bufsiz1
ಎಣಿಕೆ { READ_ಬಫರ್_ಗಾತ್ರ = COMMON_BUFSIZE - 1 };

#ವ್ಯಾಖ್ಯಾನಿಸು ವಿಭಜಿಸು_OPT_l (1<<0)
#ವ್ಯಾಖ್ಯಾನಿಸು ವಿಭಜಿಸು_OPT_b (1<<1)
#ವ್ಯಾಖ್ಯಾನಿಸು ವಿಭಜಿಸು_OPT_a (1<<2)

ಪೂರ್ಣಾಂಕ ವಿಭಜಿಸು_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ ವಿಭಜಿಸು_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	ಚಿಹ್ನೆರಹಿತ suffix_len = 2;
	ಅಕ್ಷರ *pfx;
	ಅಕ್ಷರ *ಎಣಿಕೆ_p;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sfx;
	off_t cnt = 1000;
	off_t ಉಳಿದ = 0;
	ಚಿಹ್ನೆರಹಿತ opt;
	ssize_t ಬೈಟುಗಳು_read, ಗಮ್ಯ_write;
	ಅಕ್ಷರ *src;

	ಸಿದ್ಧತೆ_common_bufsiz();

	opt = getopt32(argv, "^"
			"l:b:a:+" /* -a N */
			"\0" "?2"/*max 2 args*/,
			&ಎಣಿಕೆ_p, &ಎಣಿಕೆ_p, &suffix_len
	);

	ಆದರೆ (opt & ವಿಭಜಿಸು_OPT_l)
		cnt = XATOOFF(ಎಣಿಕೆ_p);
	ಆದರೆ (opt & ವಿಭಜಿಸು_OPT_b) // FIXME: also needs XATOOFF
		cnt = xatoull_sfx(ಎಣಿಕೆ_p,
				IF_FEATURE_ವಿಭಜಿಸು_FANCY(ವಿಭಜಿಸು_suffixes)
				IF_NOT_FEATURE_ವಿಭಜಿಸು_FANCY(km_suffixes)
		);
	sfx = "x";

	argv += optind;
	ಆದರೆ (argv[0]) {
		ಪೂರ್ಣಾಂಕ fd;
		ಆದರೆ (argv[1])
			sfx = argv[1];
		fd = xopen_stdin(argv[0]);
		xmove_fd(fd, STDIN_FILENO);
	} ಇಲ್ಲದಿದ್ದರೆ {
		argv[0] = (ಅಕ್ಷರ *) bb_ಸಂದೇಶ_standard_ಒಳಹರಿವು;
	}

	ಆದರೆ (NAME_MAX < ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(sfx) + suffix_len)
		bb_simple_error_ಸಂದೇಶ_and_die("suffix too long");

	{
		ಅಕ್ಷರ *char_p = xzalloc(suffix_len + 1);
		memset(char_p, 'a', suffix_len);
		pfx = xasprintf("%s%s", sfx, char_p);
		ಆದರೆ (ENABLE_FEATURE_CLEAN_ಮೇಲೆ)
			ಬಿಡುಗಡೆ(char_p);
	}

	ಆಗ (1) {
		ಬೈಟುಗಳು_read = ಸುರಕ್ಷಿತ_read(STDIN_FILENO, read_ಬಫರ್, READ_ಬಫರ್_ಗಾತ್ರ);
		ಆದರೆ (!ಬೈಟುಗಳು_read)
			ನಿಲ್ಲಿಸು;
		ಆದರೆ (ಬೈಟುಗಳು_read < 0)
			bb_simple_perror_ಸಂದೇಶ_and_die(argv[0]);
		src = read_ಬಫರ್;
		ಮಾಡು {
			ಆದರೆ (!ಉಳಿದ) {
				ಆದರೆ (!pfx)
					bb_simple_error_ಸಂದೇಶ_and_die("suffixes exhausted");
				xmove_fd(xopen(pfx, O_WRONLY | O_CREAT | O_TRUNC), 1);
				pfx = ಮುಂದಿನ_file(pfx, suffix_len);
				ಉಳಿದ = cnt;
			}

			ಆದರೆ (opt & ವಿಭಜಿಸು_OPT_b) {
				/* split by bytes */
				ಗಮ್ಯ_write = (ಬೈಟುಗಳು_read < ಉಳಿದ) ? ಬೈಟುಗಳು_read : ಉಳಿದ;
				ಉಳಿದ -= ಗಮ್ಯ_write;
			} ಇಲ್ಲದಿದ್ದರೆ {
				/* split by lines */
				/* can be sped up by using _memrchr_
				 * and writing many lines at once... */
				ಅಕ್ಷರ *ಕೊನೆ = memchr(src, '\n', ಬೈಟುಗಳು_read);
				ಆದರೆ (ಕೊನೆ) {
					--ಉಳಿದ;
					ಗಮ್ಯ_write = ಕೊನೆ - src + 1;
				} ಇಲ್ಲದಿದ್ದರೆ {
					ಗಮ್ಯ_write = ಬೈಟುಗಳು_read;
				}
			}

			xwrite(STDOUT_FILENO, src, ಗಮ್ಯ_write);
			ಬೈಟುಗಳು_read -= ಗಮ್ಯ_write;
			src += ಗಮ್ಯ_write;
		} ಆಗ (ಬೈಟುಗಳು_read);
	}
	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}
