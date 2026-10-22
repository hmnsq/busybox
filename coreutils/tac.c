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

#അന്തര്ഭാവ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

ഘടന lstring {
	പൂർണ്ണസംഖ്യ size;
	അക്ഷരം buf[1];
};

പൂർണ്ണസംഖ്യ tac_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) MAIN_EXTERNALLY_VISIBLE;
പൂർണ്ണസംഖ്യ tac_main(പൂർണ്ണസംഖ്യ argc UNUSED_PARAM, അക്ഷരം **argv)
{
	അക്ഷരം **name;
	FILE *f;
	ഘടന lstring *വരി = ശൂന്യ;
	llist_t *list = ശൂന്യ;
	പൂർണ്ണസംഖ്യ retval = EXIT_SUCCESS;

#എങ്കിൽ ENABLE_DESKTOP
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
#അല്ലെങ്കിൽ
	argv++;
#endif
	എങ്കിൽ (!*argv)
		*--argv = (അക്ഷരം *)"-";
	/* We will read from last file to first */
	name = argv;
	ആകുമ്പോൾ (*name)
		name++;

	ചെയ്യുക {
		പൂർണ്ണസംഖ്യ ch, i;

		name--;
		f = fopen_or_warn_stdin(*name);
		എങ്കിൽ (f == ശൂന്യ) {
			/* error message is printed by fopen_or_warn_stdin */
			retval = EXIT_FAILURE;
			തുടരുക;
		}

		errno = i = 0;
		ചെയ്യുക {
			ch = fgetc(f);
			എങ്കിൽ (ch != EOF) {
				എങ്കിൽ (!(i & 0x7f))
					/* Grow on every 128th char */
					വരി = xrealloc(വരി, i + 0x7f + വലുപ്പം(പൂർണ്ണസംഖ്യ) + 1);
				വരി->buf[i++] = ch;
			}
			എങ്കിൽ (ch == '\n' || (ch == EOF && i != 0)) {
				വരി = xrealloc(വരി, i + വലുപ്പം(പൂർണ്ണസംഖ്യ));
				വരി->size = i;
				llist_add_to(&list, വരി);
				വരി = ശൂന്യ;
				i = 0;
			}
		} ആകുമ്പോൾ (ch != EOF);
		/* fgetc sets errno to ENOENT on EOF, we don't want
		 * to warn on this non-error! */
		എങ്കിൽ (errno && errno != ENOENT) {
			bb_simple_perror_സന്ദേശം(*name);
			retval = EXIT_FAILURE;
		}
	} ആകുമ്പോൾ (name != argv);

	ആകുമ്പോൾ (list) {
		വരി = (ഘടന lstring *)list->data;
		xwrite(STDOUT_FILENO, വരി->buf, വരി->size);
		എങ്കിൽ (ENABLE_FEATURE_CLEAN_UP) {
			സ്വതന്ത്രം(llist_എടുക്കുക(&list));
		} അല്ലെങ്കിൽ {
			list = list->link;
		}
	}

	തിരികെ retval;
}
