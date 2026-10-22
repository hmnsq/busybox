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

ರಚನೆ lstring {
	ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2;
	ಅಕ್ಷರ buf[1];
};

ಪೂರ್ಣಾಂಕ tac_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ tac_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	ಅಕ್ಷರ **name;
	FILE *f;
	ರಚನೆ lstring *ಸಾಲು = ಖಾಲಿ;
	llist_t *list = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ retval = EXIT_SUCCESS;

#ಆದರೆ ENABLE_DESKTOP
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
#ಇಲ್ಲದಿದ್ದರೆ
	argv++;
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (!*argv)
		*--argv = (ಅಕ್ಷರ *)"-";
	/* We will read from last file to first */
	name = argv;
	ಆಗ (*name)
		name++;

	ಮಾಡು {
		ಪೂರ್ಣಾಂಕ ch, i;

		name--;
		f = fopen_or_warn_stdin(*name);
		ಆದರೆ (f == ಖಾಲಿ) {
			/* error message is printed by fopen_or_warn_stdin */
			retval = EXIT_FAILURE;
			ಮುಂದುವರಿ;
		}

		errno = i = 0;
		ಮಾಡು {
			ch = fgetc(f);
			ಆದರೆ (ch != EOF) {
				ಆದರೆ (!(i & 0x7f))
					/* Grow on every 128th char */
					ಸಾಲು = xrealloc(ಸಾಲು, i + 0x7f + ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ) + 1);
				ಸಾಲು->buf[i++] = ch;
			}
			ಆದರೆ (ch == '\n' || (ch == EOF && i != 0)) {
				ಸಾಲು = xrealloc(ಸಾಲು, i + ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ));
				ಸಾಲು->ಗಾತ್ರ_2 = i;
				llist_ಕೂಡಿಸು_ಗಮ್ಯ(&list, ಸಾಲು);
				ಸಾಲು = ಖಾಲಿ;
				i = 0;
			}
		} ಆಗ (ch != EOF);
		/* fgetc sets errno to ENOENT on EOF, we don't want
		 * to warn on this non-error! */
		ಆದರೆ (errno && errno != ENOENT) {
			bb_simple_perror_ಸಂದೇಶ(*name);
			retval = EXIT_FAILURE;
		}
	} ಆಗ (name != argv);

	ಆಗ (list) {
		ಸಾಲು = (ರಚನೆ lstring *)list->ದತ್ತಾಂಶ;
		xwrite(STDOUT_FILENO, ಸಾಲು->buf, ಸಾಲು->ಗಾತ್ರ_2);
		ಆದರೆ (ENABLE_FEATURE_CLEAN_ಮೇಲೆ) {
			ಬಿಡುಗಡೆ(llist_ತೆಗೆ(&list));
		} ಇಲ್ಲದಿದ್ದರೆ {
			list = list->link;
		}
	}

	ಹಿಂತಿರುಗಿ retval;
}
