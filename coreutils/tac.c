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

#ಅನ್ತರ್ಭಾವ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

ಸಂರಚನಾ lstring {
	ಪೂರ್ಣಾಙ್ಕ size;
	ವರ್ಣ buf[1];
};

ಪೂರ್ಣಾಙ್ಕ tac_main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಙ್ಕ tac_main(ಪೂರ್ಣಾಙ್ಕ argc UNUSED_PARAM, ವರ್ಣ **argv)
{
	ವರ್ಣ **name;
	FILE *f;
	ಸಂರಚನಾ lstring *ಸಾಲು = ರಿಕ್ತ;
	llist_t *list = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ retval = EXIT_SUCCESS;

#ಯದಿ ENABLE_DESKTOP
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
#ಅನ್ಯಥಾ
	argv++;
#endif
	ಯದಿ (!*argv)
		*--argv = (ವರ್ಣ *)"-";
	/* We will read from last file to first */
	name = argv;
	ಯಾವತ್ (*name)
		name++;

	ಕುರು {
		ಪೂರ್ಣಾಙ್ಕ ch, i;

		name--;
		f = fopen_or_warn_stdin(*name);
		ಯದಿ (f == ರಿಕ್ತ) {
			/* error message is printed by fopen_or_warn_stdin */
			retval = EXIT_FAILURE;
			ಅನುವರ್ತ;
		}

		errno = i = 0;
		ಕುರು {
			ch = fgetc(f);
			ಯದಿ (ch != EOF) {
				ಯದಿ (!(i & 0x7f))
					/* Grow on every 128th char */
					ಸಾಲು = xrealloc(ಸಾಲು, i + 0x7f + ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ) + 1);
				ಸಾಲು->buf[i++] = ch;
			}
			ಯದಿ (ch == '\n' || (ch == EOF && i != 0)) {
				ಸಾಲು = xrealloc(ಸಾಲು, i + ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ));
				ಸಾಲು->size = i;
				llist_add_to(&list, ಸಾಲು);
				ಸಾಲು = ರಿಕ್ತ;
				i = 0;
			}
		} ಯಾವತ್ (ch != EOF);
		/* fgetc sets errno to ENOENT on EOF, we don't want
		 * to warn on this non-error! */
		ಯದಿ (errno && errno != ENOENT) {
			bb_simple_perror_ಸಂದೇಶ(*name);
			retval = EXIT_FAILURE;
		}
	} ಯಾವತ್ (name != argv);

	ಯಾವತ್ (list) {
		ಸಾಲು = (ಸಂರಚನಾ lstring *)list->data;
		xwrite(STDOUT_FILENO, ಸಾಲು->buf, ಸಾಲು->size);
		ಯದಿ (ENABLE_FEATURE_CLEAN_UP) {
			ಮುಕ್ತ(llist_ತೆಗೆ(&list));
		} ಅನ್ಯಥಾ {
			list = list->link;
		}
	}

	ಪ್ರತ್ಯಾಗಮ retval;
}
