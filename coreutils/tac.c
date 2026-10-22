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

#شامل "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

ساختار lstring {
	صحیح size;
	نویسه buf[1];
};

صحیح tac_main(صحیح argc, نویسه **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح tac_main(صحیح argc UNUSED_PARAM, نویسه **argv)
{
	نویسه **name;
	FILE *f;
	ساختار lstring *سطر = پوچ;
	llist_t *list = پوچ;
	صحیح retval = EXIT_SUCCESS;

#اگر ENABLE_DESKTOP
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
#وگرنه
	argv++;
#endif
	اگر (!*argv)
		*--argv = (نویسه *)"-";
	/* We will read from last file to first */
	name = argv;
	وقتی (*name)
		name++;

	انجامبده {
		صحیح ch, i;

		name--;
		f = fopen_or_warn_stdin(*name);
		اگر (f == پوچ) {
			/* error message is printed by fopen_or_warn_stdin */
			retval = EXIT_FAILURE;
			ادامه;
		}

		errno = i = 0;
		انجامبده {
			ch = fgetc(f);
			اگر (ch != EOF) {
				اگر (!(i & 0x7f))
					/* Grow on every 128th char */
					سطر = xrealloc(سطر, i + 0x7f + اندازه(صحیح) + 1);
				سطر->buf[i++] = ch;
			}
			اگر (ch == '\n' || (ch == EOF && i != 0)) {
				سطر = xrealloc(سطر, i + اندازه(صحیح));
				سطر->size = i;
				llist_add_to(&list, سطر);
				سطر = پوچ;
				i = 0;
			}
		} وقتی (ch != EOF);
		/* fgetc sets errno to ENOENT on EOF, we don't want
		 * to warn on this non-error! */
		اگر (errno && errno != ENOENT) {
			bb_simple_perror_پیام(*name);
			retval = EXIT_FAILURE;
		}
	} وقتی (name != argv);

	وقتی (list) {
		سطر = (ساختار lstring *)list->data;
		xwrite(STDOUT_FILENO, سطر->buf, سطر->size);
		اگر (ENABLE_FEATURE_CLEAN_UP) {
			آزاد(llist_برداشت(&list));
		} وگرنه {
			list = list->link;
		}
	}

	بازگشت retval;
}
