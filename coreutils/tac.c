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

#include "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

struct lstring {
	số_nguyên size;
	char buf[1];
};

số_nguyên tac_main(số_nguyên argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
số_nguyên tac_main(số_nguyên argc UNUSED_PARAM, char **argv)
{
	char **name;
	FILE *f;
	struct lstring *line = NULL;
	llist_t *list = NULL;
	số_nguyên retval = EXIT_SUCCESS;

#nếu ENABLE_DESKTOP
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
#không_thì
	argv++;
#endif
	nếu (!*argv)
		*--argv = (char *)"-";
	/* We will read from last file to first */
	name = argv;
	trong_khi (*name)
		name++;

	do {
		số_nguyên ch, i;

		name--;
		f = fopen_or_warn_stdin(*name);
		nếu (f == NULL) {
			/* error message is printed by fopen_or_warn_stdin */
			retval = EXIT_FAILURE;
			continue;
		}

		errno = i = 0;
		do {
			ch = fgetc(f);
			nếu (ch != EOF) {
				nếu (!(i & 0x7f))
					/* Grow on every 128th char */
					line = xrealloc(line, i + 0x7f + sizeof(số_nguyên) + 1);
				line->buf[i++] = ch;
			}
			nếu (ch == '\n' || (ch == EOF && i != 0)) {
				line = xrealloc(line, i + sizeof(số_nguyên));
				line->size = i;
				llist_add_to(&list, line);
				line = NULL;
				i = 0;
			}
		} trong_khi (ch != EOF);
		/* fgetc sets errno to ENOENT on EOF, we don't want
		 * to warn on this non-error! */
		nếu (errno && errno != ENOENT) {
			bb_simple_perror_msg(*name);
			retval = EXIT_FAILURE;
		}
	} trong_khi (name != argv);

	trong_khi (list) {
		line = (struct lstring *)list->data;
		xwrite(STDOUT_FILENO, line->buf, line->size);
		nếu (ENABLE_FEATURE_CLEAN_UP) {
			free(llist_pop(&list));
		} không_thì {
			list = list->link;
		}
	}

	trả_về retval;
}
