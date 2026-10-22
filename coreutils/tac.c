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

#ضمن "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

بنية lstring {
	صحيح size;
	حرف buf[1];
};

صحيح tac_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح tac_main(صحيح argc UNUSED_PARAM, حرف **argv)
{
	حرف **name;
	FILE *f;
	بنية lstring *line = عدم;
	llist_t *list = عدم;
	صحيح retval = EXIT_SUCCESS;

#𐤀𐤌 ENABLE_DESKTOP
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
#وإلا
	argv++;
#endif
	𐤀𐤌 (!*argv)
		*--argv = (حرف *)"-";
	/* We will read from last file to first */
	name = argv;
	طالما (*name)
		name++;

	𐤐𐤏𐤋 {
		صحيح ch, i;

		name--;
		f = fopen_or_warn_stdin(*name);
		𐤀𐤌 (f == عدم) {
			/* error message is printed by fopen_or_warn_stdin */
			retval = EXIT_FAILURE;
			استمر;
		}

		errno = i = 0;
		𐤐𐤏𐤋 {
			ch = fgetc(f);
			𐤀𐤌 (ch != EOF) {
				𐤀𐤌 (!(i & 0x7f))
					/* Grow on every 128th char */
					line = xrealloc(line, i + 0x7f + حجم(صحيح) + 1);
				line->buf[i++] = ch;
			}
			𐤀𐤌 (ch == '\n' || (ch == EOF && i != 0)) {
				line = xrealloc(line, i + حجم(صحيح));
				line->size = i;
				llist_add_to(&list, line);
				line = عدم;
				i = 0;
			}
		} طالما (ch != EOF);
		/* fgetc sets errno to ENOENT on EOF, we don't want
		 * to warn on this non-error! */
		𐤀𐤌 (errno && errno != ENOENT) {
			bb_simple_perror_msg(*name);
			retval = EXIT_FAILURE;
		}
	} طالما (name != argv);

	طالما (list) {
		line = (بنية lstring *)list->data;
		xwrite(STDOUT_FILENO, line->buf, line->size);
		𐤀𐤌 (ENABLE_FEATURE_CLEAN_UP) {
			حرر(llist_pop(&list));
		} وإلا {
			list = list->link;
		}
	}

	أرجع retval;
}
