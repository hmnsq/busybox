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

#અન્તર્ભાવ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

સંરચના lstring {
	પૂર્ણાંક size;
	અક્ષર buf[1];
};

પૂર્ણાંક tac_main(પૂર્ણાંક argc, અક્ષર **argv) MAIN_EXTERNALLY_VISIBLE;
પૂર્ણાંક tac_main(પૂર્ણાંક argc UNUSED_PARAM, અક્ષર **argv)
{
	અક્ષર **name;
	FILE *f;
	સંરચના lstring *લીટી = ખાલી;
	llist_t *list = ખાલી;
	પૂર્ણાંક retval = EXIT_SUCCESS;

#જો ENABLE_DESKTOP
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
#નહીંતર
	argv++;
#endif
	જો (!*argv)
		*--argv = (અક્ષર *)"-";
	/* We will read from last file to first */
	name = argv;
	જ્યારે (*name)
		name++;

	કરો {
		પૂર્ણાંક ch, i;

		name--;
		f = fopen_or_warn_stdin(*name);
		જો (f == ખાલી) {
			/* error message is printed by fopen_or_warn_stdin */
			retval = EXIT_FAILURE;
			ચાલુ;
		}

		errno = i = 0;
		કરો {
			ch = fgetc(f);
			જો (ch != EOF) {
				જો (!(i & 0x7f))
					/* Grow on every 128th char */
					લીટી = xrealloc(લીટી, i + 0x7f + કદ(પૂર્ણાંક) + 1);
				લીટી->buf[i++] = ch;
			}
			જો (ch == '\n' || (ch == EOF && i != 0)) {
				લીટી = xrealloc(લીટી, i + કદ(પૂર્ણાંક));
				લીટી->size = i;
				llist_add_to(&list, લીટી);
				લીટી = ખાલી;
				i = 0;
			}
		} જ્યારે (ch != EOF);
		/* fgetc sets errno to ENOENT on EOF, we don't want
		 * to warn on this non-error! */
		જો (errno && errno != ENOENT) {
			bb_simple_perror_સંદેશ(*name);
			retval = EXIT_FAILURE;
		}
	} જ્યારે (name != argv);

	જ્યારે (list) {
		લીટી = (સંરચના lstring *)list->data;
		xwrite(STDOUT_FILENO, લીટી->buf, લીટી->size);
		જો (ENABLE_FEATURE_CLEAN_UP) {
			મુક્ત(llist_કાઢો(&list));
		} નહીંતર {
			list = list->link;
		}
	}

	પાછા retval;
}
