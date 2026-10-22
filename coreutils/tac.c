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

#அந்தர்பாவ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

கட்டமைப்பு lstring {
	முழு size;
	எழுத்து buf[1];
};

முழு tac_main(முழு argc, எழுத்து **argv) MAIN_EXTERNALLY_VISIBLE;
முழு tac_main(முழு argc UNUSED_PARAM, எழுத்து **argv)
{
	எழுத்து **name;
	FILE *f;
	கட்டமைப்பு lstring *வரி = வெற்று;
	llist_t *list = வெற்று;
	முழு retval = EXIT_SUCCESS;

#எனில் ENABLE_DESKTOP
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
#இல்லையெனில்
	argv++;
#endif
	எனில் (!*argv)
		*--argv = (எழுத்து *)"-";
	/* We will read from last file to first */
	name = argv;
	போது (*name)
		name++;

	செய் {
		முழு ch, i;

		name--;
		f = fopen_or_warn_stdin(*name);
		எனில் (f == வெற்று) {
			/* error message is printed by fopen_or_warn_stdin */
			retval = EXIT_FAILURE;
			தொடர்;
		}

		errno = i = 0;
		செய் {
			ch = fgetc(f);
			எனில் (ch != EOF) {
				எனில் (!(i & 0x7f))
					/* Grow on every 128th char */
					வரி = xrealloc(வரி, i + 0x7f + அளவு(முழு) + 1);
				வரி->buf[i++] = ch;
			}
			எனில் (ch == '\n' || (ch == EOF && i != 0)) {
				வரி = xrealloc(வரி, i + அளவு(முழு));
				வரி->size = i;
				llist_add_to(&list, வரி);
				வரி = வெற்று;
				i = 0;
			}
		} போது (ch != EOF);
		/* fgetc sets errno to ENOENT on EOF, we don't want
		 * to warn on this non-error! */
		எனில் (errno && errno != ENOENT) {
			bb_simple_perror_செய்தி(*name);
			retval = EXIT_FAILURE;
		}
	} போது (name != argv);

	போது (list) {
		வரி = (கட்டமைப்பு lstring *)list->data;
		xwrite(STDOUT_FILENO, வரி->buf, வரி->size);
		எனில் (ENABLE_FEATURE_CLEAN_UP) {
			விடுவி(llist_எடு(&list));
		} இல்லையெனில் {
			list = list->link;
		}
	}

	திரும்பு retval;
}
