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

#అన్తర్భావ "libbb.h"
#అన్తర్భావ "common_bufsiz.h"

#ఒకవేళ ENABLE_FEATURE_విభజించు_FANCY
స్థిర స్థిరరాశి నిర్మాణం suffix_mult విభజించు_suffixes[] ALIGN_SUFFIX = {
	{ "b", 512 },
	{ "k", 1024 },
	{ "m", 1024*1024 },
	{ "g", 1024*1024*1024 },
	{ "", 0 }
};
#అయితే_ముగింపు

/* Increment the suffix part of the filename.
 * Returns NULL if we are out of filenames.
 */
స్థిర అక్షరం *తదుపరి_file(అక్షరం *old, గుర్తులేని suffix_len)
{
	కొలత_t చివర = అక్షరశ్రేణి_పొడవు(old);
	గుర్తులేని i = 1;
	అక్షరం *curr;

	అయితే (1) {
		curr = old + చివర - i;
		ఒకవేళ (*curr < 'z') {
			*curr += 1;
			ఆపు;
		}
		i++;
		ఒకవేళ (i > suffix_len) {
			తిరిగి ఖాళీ;
		}
		*curr = 'a';
	}

	తిరిగి old;
}

#నిర్వచించు read_బఫరు bb_common_bufsiz1
గణన { READ_బఫరు_కొలత = COMMON_BUFSIZE - 1 };

#నిర్వచించు విభజించు_OPT_l (1<<0)
#నిర్వచించు విభజించు_OPT_b (1<<1)
#నిర్వచించు విభజించు_OPT_a (1<<2)

పూర్ణాంకం విభజించు_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం విభజించు_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	గుర్తులేని suffix_len = 2;
	అక్షరం *pfx;
	అక్షరం *లెక్క_p;
	స్థిరరాశి అక్షరం *sfx;
	off_t cnt = 1000;
	off_t మిగిలిన = 0;
	గుర్తులేని opt;
	ssize_t బైట్లు_read, గమ్యం_write;
	అక్షరం *src;

	అమరిక_common_bufsiz();

	opt = getopt32(argv, "^"
			"l:b:a:+" /* -a N */
			"\0" "?2"/*max 2 args*/,
			&లెక్క_p, &లెక్క_p, &suffix_len
	);

	ఒకవేళ (opt & విభజించు_OPT_l)
		cnt = XATOOFF(లెక్క_p);
	ఒకవేళ (opt & విభజించు_OPT_b) // FIXME: also needs XATOOFF
		cnt = xatoull_sfx(లెక్క_p,
				IF_FEATURE_విభజించు_FANCY(విభజించు_suffixes)
				IF_NOT_FEATURE_విభజించు_FANCY(km_suffixes)
		);
	sfx = "x";

	argv += optind;
	ఒకవేళ (argv[0]) {
		పూర్ణాంకం fd;
		ఒకవేళ (argv[1])
			sfx = argv[1];
		fd = xopen_stdin(argv[0]);
		xmove_fd(fd, STDIN_FILENO);
	} లేకపోతే {
		argv[0] = (అక్షరం *) bb_సందేశం_standard_ఆదానం;
	}

	ఒకవేళ (NAME_MAX < అక్షరశ్రేణి_పొడవు(sfx) + suffix_len)
		bb_simple_error_సందేశం_and_die("suffix too long");

	{
		అక్షరం *char_p = xzalloc(suffix_len + 1);
		memset(char_p, 'a', suffix_len);
		pfx = xasprintf("%s%s", sfx, char_p);
		ఒకవేళ (ENABLE_FEATURE_CLEAN_పైకి)
			విడుదల(char_p);
	}

	అయితే (1) {
		బైట్లు_read = సురక్షిత_read(STDIN_FILENO, read_బఫరు, READ_బఫరు_కొలత);
		ఒకవేళ (!బైట్లు_read)
			ఆపు;
		ఒకవేళ (బైట్లు_read < 0)
			bb_simple_perror_సందేశం_and_die(argv[0]);
		src = read_బఫరు;
		చేయి {
			ఒకవేళ (!మిగిలిన) {
				ఒకవేళ (!pfx)
					bb_simple_error_సందేశం_and_die("suffixes exhausted");
				xmove_fd(xopen(pfx, O_WRONLY | O_CREAT | O_TRUNC), 1);
				pfx = తదుపరి_file(pfx, suffix_len);
				మిగిలిన = cnt;
			}

			ఒకవేళ (opt & విభజించు_OPT_b) {
				/* split by bytes */
				గమ్యం_write = (బైట్లు_read < మిగిలిన) ? బైట్లు_read : మిగిలిన;
				మిగిలిన -= గమ్యం_write;
			} లేకపోతే {
				/* split by lines */
				/* can be sped up by using _memrchr_
				 * and writing many lines at once... */
				అక్షరం *చివర = memchr(src, '\n', బైట్లు_read);
				ఒకవేళ (చివర) {
					--మిగిలిన;
					గమ్యం_write = చివర - src + 1;
				} లేకపోతే {
					గమ్యం_write = బైట్లు_read;
				}
			}

			xwrite(STDOUT_FILENO, src, గమ్యం_write);
			బైట్లు_read -= గమ్యం_write;
			src += గమ్యం_write;
		} అయితే (బైట్లు_read);
	}
	తిరిగి EXIT_SUCCESS;
}
