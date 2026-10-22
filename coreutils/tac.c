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

#अन्तर्भाव "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

संरचना lstring {
	पूर्णाङ्क size;
	वर्ण buf[1];
};

पूर्णाङ्क tac_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क tac_main(पूर्णाङ्क argc UNUSED_PARAM, वर्ण **argv)
{
	वर्ण **name;
	FILE *f;
	संरचना lstring *ओळ = रिक्त;
	llist_t *list = रिक्त;
	पूर्णाङ्क retval = EXIT_SUCCESS;

#यदि ENABLE_DESKTOP
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
#अन्यथा
	argv++;
#endif
	यदि (!*argv)
		*--argv = (वर्ण *)"-";
	/* We will read from last file to first */
	name = argv;
	यावत् (*name)
		name++;

	कुरु {
		पूर्णाङ्क ch, i;

		name--;
		f = fopen_or_warn_stdin(*name);
		यदि (f == रिक्त) {
			/* error message is printed by fopen_or_warn_stdin */
			retval = EXIT_FAILURE;
			अनुवर्त;
		}

		errno = i = 0;
		कुरु {
			ch = fgetc(f);
			यदि (ch != EOF) {
				यदि (!(i & 0x7f))
					/* Grow on every 128th char */
					ओळ = xrealloc(ओळ, i + 0x7f + परिमाण(पूर्णाङ्क) + 1);
				ओळ->buf[i++] = ch;
			}
			यदि (ch == '\n' || (ch == EOF && i != 0)) {
				ओळ = xrealloc(ओळ, i + परिमाण(पूर्णाङ्क));
				ओळ->size = i;
				llist_add_to(&list, ओळ);
				ओळ = रिक्त;
				i = 0;
			}
		} यावत् (ch != EOF);
		/* fgetc sets errno to ENOENT on EOF, we don't want
		 * to warn on this non-error! */
		यदि (errno && errno != ENOENT) {
			bb_simple_perror_संदेश(*name);
			retval = EXIT_FAILURE;
		}
	} यावत् (name != argv);

	यावत् (list) {
		ओळ = (संरचना lstring *)list->data;
		xwrite(STDOUT_FILENO, ओळ->buf, ओळ->size);
		यदि (ENABLE_FEATURE_CLEAN_UP) {
			मुक्त(llist_काढा(&list));
		} अन्यथा {
			list = list->link;
		}
	}

	प्रत्यागम retval;
}
