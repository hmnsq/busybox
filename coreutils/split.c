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

#شامل "libbb.h"
#شامل "common_bufsiz.h"

#اگر ENABLE_FEATURE_بانٹو_FANCY
ساکن ثابت ساخت suffix_mult بانٹو_suffixes[] ALIGN_SUFFIX = {
	{ "b", 512 },
	{ "k", 1024 },
	{ "m", 1024*1024 },
	{ "g", 1024*1024*1024 },
	{ "", 0 }
};
#اگر_ختم

/* Increment the suffix part of the filename.
 * Returns NULL if we are out of filenames.
 */
ساکن حرف *اگلا_file(حرف *old, غیرعلامتی suffix_len)
{
	سائز_t انتہا = لڑی_لمبائی(old);
	غیرعلامتی i = 1;
	حرف *curr;

	جب (1) {
		curr = old + انتہا - i;
		اگر (*curr < 'z') {
			*curr += 1;
			توڑو;
		}
		i++;
		اگر (i > suffix_len) {
			واپس تہی;
		}
		*curr = 'a';
	}

	واپس old;
}

#تعریف_کرو read_بفر bb_common_bufsiz1
شمار { READ_بفر_سائز = COMMON_BUFSIZE - 1 };

#تعریف_کرو بانٹو_OPT_l (1<<0)
#تعریف_کرو بانٹو_OPT_b (1<<1)
#تعریف_کرو بانٹو_OPT_a (1<<2)

صحیح بانٹو_main(صحیح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح بانٹو_main(صحیح argc غیر_مستعمل_PARAM, حرف **argv)
{
	غیرعلامتی suffix_len = 2;
	حرف *pfx;
	حرف *گنتی_p;
	ثابت حرف *sfx;
	off_t cnt = 1000;
	off_t بقایا = 0;
	غیرعلامتی opt;
	ssize_t بائٹیں_read, مقصد_write;
	حرف *src;

	تیاری_common_bufsiz();

	opt = getopt32(argv, "^"
			"l:b:a:+" /* -a N */
			"\0" "?2"/*max 2 args*/,
			&گنتی_p, &گنتی_p, &suffix_len
	);

	اگر (opt & بانٹو_OPT_l)
		cnt = XATOOFF(گنتی_p);
	اگر (opt & بانٹو_OPT_b) // FIXME: also needs XATOOFF
		cnt = xatoull_sfx(گنتی_p,
				IF_FEATURE_بانٹو_FANCY(بانٹو_suffixes)
				IF_NOT_FEATURE_بانٹو_FANCY(km_suffixes)
		);
	sfx = "x";

	argv += optind;
	اگر (argv[0]) {
		صحیح fd;
		اگر (argv[1])
			sfx = argv[1];
		fd = xopen_stdin(argv[0]);
		xmove_fd(fd, STDIN_FILENO);
	} ورنہ {
		argv[0] = (حرف *) bb_پیغام_standard_داخلہ;
	}

	اگر (NAME_MAX < لڑی_لمبائی(sfx) + suffix_len)
		bb_simple_error_پیغام_and_die("suffix too long");

	{
		حرف *char_p = xzalloc(suffix_len + 1);
		memset(char_p, 'a', suffix_len);
		pfx = xasprintf("%s%s", sfx, char_p);
		اگر (ENABLE_FEATURE_CLEAN_اوپر)
			آزاد(char_p);
	}

	جب (1) {
		بائٹیں_read = محفوظ_read(STDIN_FILENO, read_بفر, READ_بفر_سائز);
		اگر (!بائٹیں_read)
			توڑو;
		اگر (بائٹیں_read < 0)
			bb_simple_perror_پیغام_and_die(argv[0]);
		src = read_بفر;
		کرو {
			اگر (!بقایا) {
				اگر (!pfx)
					bb_simple_error_پیغام_and_die("suffixes exhausted");
				xmove_fd(xopen(pfx, O_WRONLY | O_CREAT | O_TRUNC), 1);
				pfx = اگلا_file(pfx, suffix_len);
				بقایا = cnt;
			}

			اگر (opt & بانٹو_OPT_b) {
				/* split by bytes */
				مقصد_write = (بائٹیں_read < بقایا) ? بائٹیں_read : بقایا;
				بقایا -= مقصد_write;
			} ورنہ {
				/* split by lines */
				/* can be sped up by using _memrchr_
				 * and writing many lines at once... */
				حرف *انتہا = memchr(src, '\n', بائٹیں_read);
				اگر (انتہا) {
					--بقایا;
					مقصد_write = انتہا - src + 1;
				} ورنہ {
					مقصد_write = بائٹیں_read;
				}
			}

			xwrite(STDOUT_FILENO, src, مقصد_write);
			بائٹیں_read -= مقصد_write;
			src += مقصد_write;
		} جب (بائٹیں_read);
	}
	واپس EXIT_SUCCESS;
}
