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

#ضمن "libbb.h"
#ضمن "common_bufsiz.h"

#إذا ENABLE_FEATURE_ورهايو_FANCY
ساكن ثابت بنية suffix_mult ورهايو_suffixes[] ALIGN_SUFFIX = {
	{ "b", 512 },
	{ "k", 1024 },
	{ "m", 1024*1024 },
	{ "g", 1024*1024*1024 },
	{ "", 0 }
};
#جيڪڏهن_پورو

/* Increment the suffix part of the filename.
 * Returns NULL if we are out of filenames.
 */
ساكن حرف *ايندڙ_file(حرف *old, بنانشان suffix_len)
{
	سائيز_t پڇاڙي = لڙي_ڊيگهه(old);
	بنانشان i = 1;
	حرف *curr;

	طالما (1) {
		curr = old + پڇاڙي - i;
		إذا (*curr < 'z') {
			*curr += 1;
			اكسر;
		}
		i++;
		إذا (i > suffix_len) {
			أرجع عدم;
		}
		*curr = 'a';
	}

	أرجع old;
}

#وصف_ڪريو read_بفر bb_common_bufsiz1
تعداد { READ_بفر_سائيز = COMMON_BUFSIZE - 1 };

#وصف_ڪريو ورهايو_OPT_l (1<<0)
#وصف_ڪريو ورهايو_OPT_b (1<<1)
#وصف_ڪريو ورهايو_OPT_a (1<<2)

صحيح ورهايو_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح ورهايو_main(صحيح argc اڻ_استعمال_ٿيل_PARAM, حرف **argv)
{
	بنانشان suffix_len = 2;
	حرف *pfx;
	حرف *ڳڻپ_p;
	ثابت حرف *sfx;
	off_t cnt = 1000;
	off_t بچيل = 0;
	بنانشان opt;
	ssize_t بائيٽون_read, منزل_write;
	حرف *src;

	تياري_common_bufsiz();

	opt = getopt32(argv, "^"
			"l:b:a:+" /* -a N */
			"\0" "?2"/*max 2 args*/,
			&ڳڻپ_p, &ڳڻپ_p, &suffix_len
	);

	إذا (opt & ورهايو_OPT_l)
		cnt = XATOOFF(ڳڻپ_p);
	إذا (opt & ورهايو_OPT_b) // FIXME: also needs XATOOFF
		cnt = xatoull_sfx(ڳڻپ_p,
				IF_FEATURE_ورهايو_FANCY(ورهايو_suffixes)
				IF_NOT_FEATURE_ورهايو_FANCY(km_suffixes)
		);
	sfx = "x";

	argv += optind;
	إذا (argv[0]) {
		صحيح fd;
		إذا (argv[1])
			sfx = argv[1];
		fd = xopen_stdin(argv[0]);
		xmove_fd(fd, STDIN_FILENO);
	} وإلا {
		argv[0] = (حرف *) bb_نياپو_standard_داخلا;
	}

	إذا (NAME_MAX < لڙي_ڊيگهه(sfx) + suffix_len)
		bb_simple_error_نياپو_and_die("suffix too long");

	{
		حرف *char_p = xzalloc(suffix_len + 1);
		memset(char_p, 'a', suffix_len);
		pfx = xasprintf("%s%s", sfx, char_p);
		إذا (ENABLE_FEATURE_CLEAN_مٿي)
			حرر(char_p);
	}

	طالما (1) {
		بائيٽون_read = محفوظ_read(STDIN_FILENO, read_بفر, READ_بفر_سائيز);
		إذا (!بائيٽون_read)
			اكسر;
		إذا (بائيٽون_read < 0)
			bb_simple_perror_نياپو_and_die(argv[0]);
		src = read_بفر;
		افعل {
			إذا (!بچيل) {
				إذا (!pfx)
					bb_simple_error_نياپو_and_die("suffixes exhausted");
				xmove_fd(xopen(pfx, O_WRONLY | O_CREAT | O_TRUNC), 1);
				pfx = ايندڙ_file(pfx, suffix_len);
				بچيل = cnt;
			}

			إذا (opt & ورهايو_OPT_b) {
				/* split by bytes */
				منزل_write = (بائيٽون_read < بچيل) ? بائيٽون_read : بچيل;
				بچيل -= منزل_write;
			} وإلا {
				/* split by lines */
				/* can be sped up by using _memrchr_
				 * and writing many lines at once... */
				حرف *پڇاڙي = memchr(src, '\n', بائيٽون_read);
				إذا (پڇاڙي) {
					--بچيل;
					منزل_write = پڇاڙي - src + 1;
				} وإلا {
					منزل_write = بائيٽون_read;
				}
			}

			xwrite(STDOUT_FILENO, src, منزل_write);
			بائيٽون_read -= منزل_write;
			src += منزل_write;
		} طالما (بائيٽون_read);
	}
	أرجع EXIT_SUCCESS;
}
