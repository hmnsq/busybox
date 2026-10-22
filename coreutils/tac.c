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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

ਬਣਤਰ lstring {
	ਪੂਰਨ ਮਾਪ;
	ਅੱਖਰ buf[1];
};

ਪੂਰਨ tac_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ tac_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
	ਅੱਖਰ **name;
	FILE *f;
	ਬਣਤਰ lstring *ਸਤਰ = ਨਿਰਾ;
	llist_t *list = ਨਿਰਾ;
	ਪੂਰਨ retval = EXIT_SUCCESS;

#ਜੇ ENABLE_DESKTOP
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
#ਨਹੀਂਤਾਂ
	argv++;
#ਜੇ_ਅੰਤ
	ਜੇ (!*argv)
		*--argv = (ਅੱਖਰ *)"-";
	/* We will read from last file to first */
	name = argv;
	ਜਦੋਂ (*name)
		name++;

	ਕਰੋ {
		ਪੂਰਨ ch, i;

		name--;
		f = fopen_or_warn_stdin(*name);
		ਜੇ (f == ਨਿਰਾ) {
			/* error message is printed by fopen_or_warn_stdin */
			retval = EXIT_FAILURE;
			ਜਾਰੀ;
		}

		errno = i = 0;
		ਕਰੋ {
			ch = fgetc(f);
			ਜੇ (ch != EOF) {
				ਜੇ (!(i & 0x7f))
					/* Grow on every 128th char */
					ਸਤਰ = xrealloc(ਸਤਰ, i + 0x7f + ਆਕਾਰ(ਪੂਰਨ) + 1);
				ਸਤਰ->buf[i++] = ch;
			}
			ਜੇ (ch == '\n' || (ch == EOF && i != 0)) {
				ਸਤਰ = xrealloc(ਸਤਰ, i + ਆਕਾਰ(ਪੂਰਨ));
				ਸਤਰ->ਮਾਪ = i;
				llist_ਜੋੜੋ_ਮੰਜ਼ਿਲ(&list, ਸਤਰ);
				ਸਤਰ = ਨਿਰਾ;
				i = 0;
			}
		} ਜਦੋਂ (ch != EOF);
		/* fgetc sets errno to ENOENT on EOF, we don't want
		 * to warn on this non-error! */
		ਜੇ (errno && errno != ENOENT) {
			bb_simple_perror_ਸੁਨੇਹਾ(*name);
			retval = EXIT_FAILURE;
		}
	} ਜਦੋਂ (name != argv);

	ਜਦੋਂ (list) {
		ਸਤਰ = (ਬਣਤਰ lstring *)list->ਅੰਕੜੇ;
		xwrite(STDOUT_FILENO, ਸਤਰ->buf, ਸਤਰ->ਮਾਪ);
		ਜੇ (ENABLE_FEATURE_CLEAN_ਉੱਪਰ) {
			ਮੁਕਤ(llist_ਕੱਢੋ(&list));
		} ਨਹੀਂਤਾਂ {
			list = list->link;
		}
	}

	ਵਾਪਸ retval;
}
