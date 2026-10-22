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

#ଅନ୍ତର୍ଭାଵ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

ସଂରଚନା lstring {
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା size;
	ଅକ୍ଷର buf[1];
};

ପୂର୍ଣ୍ଣସଂଖ୍ୟା tac_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv) MAIN_EXTERNALLY_VISIBLE;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା tac_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc UNUSED_PARAM, ଅକ୍ଷର **argv)
{
	ଅକ୍ଷର **name;
	FILE *f;
	ସଂରଚନା lstring *ଧାଡ଼ି = ଖାଲି;
	llist_t *list = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval = EXIT_SUCCESS;

#ଯଦି ENABLE_DESKTOP
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
#ଅନ୍ୟଥା
	argv++;
#endif
	ଯଦି (!*argv)
		*--argv = (ଅକ୍ଷର *)"-";
	/* We will read from last file to first */
	name = argv;
	ଯେତେବେଳେ (*name)
		name++;

	କର {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch, i;

		name--;
		f = fopen_or_warn_stdin(*name);
		ଯଦି (f == ଖାଲି) {
			/* error message is printed by fopen_or_warn_stdin */
			retval = EXIT_FAILURE;
			ଜାରି;
		}

		errno = i = 0;
		କର {
			ch = fgetc(f);
			ଯଦି (ch != EOF) {
				ଯଦି (!(i & 0x7f))
					/* Grow on every 128th char */
					ଧାଡ଼ି = xrealloc(ଧାଡ଼ି, i + 0x7f + ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା) + 1);
				ଧାଡ଼ି->buf[i++] = ch;
			}
			ଯଦି (ch == '\n' || (ch == EOF && i != 0)) {
				ଧାଡ଼ି = xrealloc(ଧାଡ଼ି, i + ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା));
				ଧାଡ଼ି->size = i;
				llist_add_to(&list, ଧାଡ଼ି);
				ଧାଡ଼ି = ଖାଲି;
				i = 0;
			}
		} ଯେତେବେଳେ (ch != EOF);
		/* fgetc sets errno to ENOENT on EOF, we don't want
		 * to warn on this non-error! */
		ଯଦି (errno && errno != ENOENT) {
			bb_simple_perror_ସନ୍ଦେଶ(*name);
			retval = EXIT_FAILURE;
		}
	} ଯେତେବେଳେ (name != argv);

	ଯେତେବେଳେ (list) {
		ଧାଡ଼ି = (ସଂରଚନା lstring *)list->data;
		xwrite(STDOUT_FILENO, ଧାଡ଼ି->buf, ଧାଡ଼ି->size);
		ଯଦି (ENABLE_FEATURE_CLEAN_UP) {
			ମୁକ୍ତ(llist_କାଢ଼(&list));
		} ଅନ୍ୟଥା {
			list = list->link;
		}
	}

	ଫେରନ୍ତୁ retval;
}
