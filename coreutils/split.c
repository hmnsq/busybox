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

#إذا ENABLE_FEATURE_تقسیم_FANCY
ساكن ثابت بنية suffix_mult تقسیم_suffixes[] ALIGN_SUFFIX = {
	{ "b", 512 },
	{ "k", 1024 },
	{ "m", 1024*1024 },
	{ "g", 1024*1024*1024 },
	{ "", 0 }
};
#پایان_اگر

/* Increment the suffix part of the filename.
 * Returns NULL if we are out of filenames.
 */
ساكن حرف *بعدی_file(حرف *old, بیعلامت suffix_len)
{
	اندازه_t انتها = طول_رشته(old);
	بیعلامت i = 1;
	حرف *curr;

	طالما (1) {
		curr = old + انتها - i;
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

#تعریف_کن read_میانگیر bb_common_bufsiz1
تعداد { READ_میانگیر_اندازه = COMMON_BUFSIZE - 1 };

#تعریف_کن تقسیم_OPT_l (1<<0)
#تعریف_کن تقسیم_OPT_b (1<<1)
#تعریف_کن تقسیم_OPT_a (1<<2)

صحيح تقسیم_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح تقسیم_main(صحيح argc استفادهنشده_PARAM, حرف **argv)
{
	بیعلامت suffix_len = 2;
	حرف *pfx;
	حرف *شمار_p;
	ثابت حرف *sfx;
	off_t cnt = 1000;
	off_t مانده = 0;
	بیعلامت opt;
	ssize_t بایتها_read, مقصد_write;
	حرف *src;

	راهاندازی_common_bufsiz();

	opt = getopt32(argv, "^"
			"l:b:a:+" /* -a N */
			"\0" "?2"/*max 2 args*/,
			&شمار_p, &شمار_p, &suffix_len
	);

	إذا (opt & تقسیم_OPT_l)
		cnt = XATOOFF(شمار_p);
	إذا (opt & تقسیم_OPT_b) // FIXME: also needs XATOOFF
		cnt = xatoull_sfx(شمار_p,
				IF_FEATURE_تقسیم_FANCY(تقسیم_suffixes)
				IF_NOT_FEATURE_تقسیم_FANCY(km_suffixes)
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
		argv[0] = (حرف *) bb_پیام_standard_درون_داد;
	}

	إذا (NAME_MAX < طول_رشته(sfx) + suffix_len)
		bb_simple_error_پیام_and_die("suffix too long");

	{
		حرف *char_p = xzalloc(suffix_len + 1);
		memset(char_p, 'a', suffix_len);
		pfx = xasprintf("%s%s", sfx, char_p);
		إذا (ENABLE_FEATURE_CLEAN_به_بالا)
			حرر(char_p);
	}

	طالما (1) {
		بایتها_read = امن_read(STDIN_FILENO, read_میانگیر, READ_میانگیر_اندازه);
		إذا (!بایتها_read)
			اكسر;
		إذا (بایتها_read < 0)
			bb_simple_perror_پیام_and_die(argv[0]);
		src = read_میانگیر;
		افعل {
			إذا (!مانده) {
				إذا (!pfx)
					bb_simple_error_پیام_and_die("suffixes exhausted");
				xmove_fd(xopen(pfx, O_WRONLY | O_CREAT | O_TRUNC), 1);
				pfx = بعدی_file(pfx, suffix_len);
				مانده = cnt;
			}

			إذا (opt & تقسیم_OPT_b) {
				/* split by bytes */
				مقصد_write = (بایتها_read < مانده) ? بایتها_read : مانده;
				مانده -= مقصد_write;
			} وإلا {
				/* split by lines */
				/* can be sped up by using _memrchr_
				 * and writing many lines at once... */
				حرف *انتها = memchr(src, '\n', بایتها_read);
				إذا (انتها) {
					--مانده;
					مقصد_write = انتها - src + 1;
				} وإلا {
					مقصد_write = بایتها_read;
				}
			}

			xwrite(STDOUT_FILENO, src, مقصد_write);
			بایتها_read -= مقصد_write;
			src += مقصد_write;
		} طالما (بایتها_read);
	}
	أرجع EXIT_SUCCESS;
}
