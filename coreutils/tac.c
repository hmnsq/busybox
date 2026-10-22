/* vi: set sw=4 ts=4: */
/*
 * tac implementation for busybox
 * tac - concatenate and print files in reverse
 *
 * Copyright (C) 2003  Yang Xiaopeng  <yxp at hanwang.com.cn>
 * Copyright (C) 2007  Natanael Copa  <natanael.copa@gmail.com>
 * Copyright (C) 2007  Tito Ragusa    <farmatito@tiscali.it>
 *
 * Licensed under GPLv2, see file LICENSE in this source tree.
 */
/* Based on Yang Xiaopeng's (yxp at hanwang.com.cn) patch
 * http://www.uclibc.org/lists/busybox/2003-July/008813.html
 */
//config:config TAC
//config:	bool "tac (4.1 kb)"
//config:	default y
//config:	help
//config:	tac is used to concatenate and print files in reverse.

//applet:IF_TAC(APPLET_NOEXEC(tac, tac, BB_DIR_USR_BIN, BB_SUID_DROP, tac))

//kbuild:lib-$(CONFIG_TAC) += tac.o

//usage:#define tac_trivial_usage
//usage:	"[FILE]..."
//usage:#define tac_full_usage "\n\n"
//usage:	"Concatenate FILEs and print them in reverse"

#অন্তর্ভাৱ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

সংরচনা lstring {
	পূর্ণাঙ্ক size;
	ৱর্ণ buf[1];
};

পূর্ণাঙ্ক tac_main(পূর্ণাঙ্ক argc, ৱর্ণ **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণাঙ্ক tac_main(পূর্ণাঙ্ক argc UNUSED_PARAM, ৱর্ণ **argv)
{
	ৱর্ণ **name;
	FILE *f;
	সংরচনা lstring *সাৰি = রিক্ত;
	llist_t *list = রিক্ত;
	পূর্ণাঙ্ক retval = EXIT_SUCCESS;

#যদি ENABLE_DESKTOP
/* tac from coreutils 6.9 supports:
       -b, --before
              attach the separator before instead of after
       -r, --regex
              interpret the separator as a regular expression
       -s, --separator=STRING
              use STRING as the separator instead of newline
We support none, but at least we will complain or handle "--":
*/
	getopt32(argv, "");
	argv += optind;
#অন্যথা
	argv++;
#endif
	যদি (!*argv)
		*--argv = (ৱর্ণ *)"-";
	/* We will read from last file to first */
	name = argv;
	যেতিয়ালৈ (*name)
		name++;

	কৰা {
		পূর্ণাঙ্ক ch, i;

		name--;
		f = fopen_or_warn_stdin(*name);
		যদি (f == রিক্ত) {
			/* error message is printed by fopen_or_warn_stdin */
			retval = EXIT_FAILURE;
			অনুৱর্ত;
		}

		errno = i = 0;
		কৰা {
			ch = fgetc(f);
			যদি (ch != EOF) {
				যদি (!(i & 0x7f))
					/* Grow on every 128th char */
					সাৰি = xrealloc(সাৰি, i + 0x7f + পরিমাণ(পূর্ণাঙ্ক) + 1);
				সাৰি->buf[i++] = ch;
			}
			যদি (ch == '\n' || (ch == EOF && i != 0)) {
				সাৰি = xrealloc(সাৰি, i + পরিমাণ(পূর্ণাঙ্ক));
				সাৰি->size = i;
				llist_add_to(&list, সাৰি);
				সাৰি = রিক্ত;
				i = 0;
			}
		} যেতিয়ালৈ (ch != EOF);
		/* fgetc sets errno to ENOENT on EOF, we don't want
		 * to warn on this non-error! */
		যদি (errno && errno != ENOENT) {
			bb_simple_perror_বাৰ্তা(*name);
			retval = EXIT_FAILURE;
		}
	} যেতিয়ালৈ (name != argv);

	যেতিয়ালৈ (list) {
		সাৰি = (সংরচনা lstring *)list->data;
		xwrite(STDOUT_FILENO, সাৰি->buf, সাৰি->size);
		যদি (ENABLE_FEATURE_CLEAN_UP) {
			মুক্ত(llist_বেৰ_কৰো(&list));
		} অন্যথা {
			list = list->link;
		}
	}

	ঘূৰাই retval;
}
