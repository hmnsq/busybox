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

#إذا ENABLE_FEATURE_ونڈو_FANCY
ساكن ثابت بنية suffix_mult ونڈو_suffixes[] ALIGN_SUFFIX = {
	{ "b", 512 },
	{ "k", 1024 },
	{ "m", 1024*1024 },
	{ "g", 1024*1024*1024 },
	{ "", 0 }
};
#جے_انت

/* Increment the suffix part of the filename.
 * Returns NULL if we are out of filenames.
 */
ساكن حرف *اگلا_file(حرف *old, بنانشانی suffix_len)
{
	ناپ_t سرا = لڑی_لمبائی(old);
	بنانشانی i = 1;
	حرف *curr;

	طالما (1) {
		curr = old + سرا - i;
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

#تعریف_کرو read_بفر bb_common_bufsiz1
تعداد { READ_بفر_ناپ = COMMON_BUFSIZE - 1 };

#تعریف_کرو ونڈو_OPT_l (1<<0)
#تعریف_کرو ونڈو_OPT_b (1<<1)
#تعریف_کرو ونڈو_OPT_a (1<<2)

صحيح ونڈو_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح ونڈو_main(صحيح argc بن_ورتیا_PARAM, حرف **argv)
{
	بنانشانی suffix_len = 2;
	حرف *pfx;
	حرف *گنتی_p;
	ثابت حرف *sfx;
	off_t cnt = 1000;
	off_t بچیا = 0;
	بنانشانی opt;
	ssize_t بائٹاں_read, منزل_write;
	حرف *src;

	تیاری_common_bufsiz();

	opt = getopt32(argv, "^"
			"l:b:a:+" /* -a N */
			"\0" "?2"/*max 2 args*/,
			&گنتی_p, &گنتی_p, &suffix_len
	);

	إذا (opt & ونڈو_OPT_l)
		cnt = XATOOFF(گنتی_p);
	إذا (opt & ونڈو_OPT_b) // FIXME: also needs XATOOFF
		cnt = xatoull_sfx(گنتی_p,
				IF_FEATURE_ونڈو_FANCY(ونڈو_suffixes)
				IF_NOT_FEATURE_ونڈو_FANCY(km_suffixes)
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
		argv[0] = (حرف *) bb_پیغام_standard_داخلہ;
	}

	إذا (NAME_MAX < لڑی_لمبائی(sfx) + suffix_len)
		bb_simple_error_پیغام_and_die("suffix too long");

	{
		حرف *char_p = xzalloc(suffix_len + 1);
		memset(char_p, 'a', suffix_len);
		pfx = xasprintf("%s%s", sfx, char_p);
		إذا (ENABLE_FEATURE_CLEAN_اتے)
			حرر(char_p);
	}

	طالما (1) {
		بائٹاں_read = محفوظ_read(STDIN_FILENO, read_بفر, READ_بفر_ناپ);
		إذا (!بائٹاں_read)
			اكسر;
		إذا (بائٹاں_read < 0)
			bb_simple_perror_پیغام_and_die(argv[0]);
		src = read_بفر;
		افعل {
			إذا (!بچیا) {
				إذا (!pfx)
					bb_simple_error_پیغام_and_die("suffixes exhausted");
				xmove_fd(xopen(pfx, O_WRONLY | O_CREAT | O_TRUNC), 1);
				pfx = اگلا_file(pfx, suffix_len);
				بچیا = cnt;
			}

			إذا (opt & ونڈو_OPT_b) {
				/* split by bytes */
				منزل_write = (بائٹاں_read < بچیا) ? بائٹاں_read : بچیا;
				بچیا -= منزل_write;
			} وإلا {
				/* split by lines */
				/* can be sped up by using _memrchr_
				 * and writing many lines at once... */
				حرف *سرا = memchr(src, '\n', بائٹاں_read);
				إذا (سرا) {
					--بچیا;
					منزل_write = سرا - src + 1;
				} وإلا {
					منزل_write = بائٹاں_read;
				}
			}

			xwrite(STDOUT_FILENO, src, منزل_write);
			بائٹاں_read -= منزل_write;
			src += منزل_write;
		} طالما (بائٹاں_read);
	}
	أرجع EXIT_SUCCESS;
}
