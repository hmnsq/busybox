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
	целое size;
	char buf[1];
};

целое tac_main(целое argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
целое tac_main(целое argc UNUSED_PARAM, char **argv)
{
	char **name;
	FILE *f;
	struct lstring *line = NULL;
	llist_t *list = NULL;
	целое retval = EXIT_SUCCESS;

#если ENABLE_DESKTOP
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
#иначе
	argv++;
#endif
	если (!*argv)
		*--argv = (char *)"-";
	/* We will read from last file to first */
	name = argv;
	пока (*name)
		name++;

	do {
		целое ch, i;

		name--;
		f = fopen_or_warn_stdin(*name);
		если (f == NULL) {
			/* error message is printed by fopen_or_warn_stdin */
			retval = EXIT_FAILURE;
			continue;
		}

		errno = i = 0;
		do {
			ch = fgetc(f);
			если (ch != EOF) {
				если (!(i & 0x7f))
					/* Grow on every 128th char */
					line = xrealloc(line, i + 0x7f + sizeof(целое) + 1);
				line->buf[i++] = ch;
			}
			если (ch == '\n' || (ch == EOF && i != 0)) {
				line = xrealloc(line, i + sizeof(целое));
				line->size = i;
				llist_add_to(&list, line);
				line = NULL;
				i = 0;
			}
		} пока (ch != EOF);
		/* fgetc sets errno to ENOENT on EOF, we don't want
		 * to warn on this non-error! */
		если (errno && errno != ENOENT) {
			bb_simple_perror_msg(*name);
			retval = EXIT_FAILURE;
		}
	} пока (name != argv);

	пока (list) {
		line = (struct lstring *)list->data;
		xwrite(STDOUT_FILENO, line->buf, line->size);
		если (ENABLE_FEATURE_CLEAN_UP) {
			free(llist_pop(&list));
		} иначе {
			list = list->link;
		}
	}

	вернуть retval;
}
