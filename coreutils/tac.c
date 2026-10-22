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
	पूर्णांक size;
	अक्षर buf[1];
};

पूर्णांक tac_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक tac_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	अक्षर **name;
	खाता *f;
	संरचना lstring *ओळ = रिक्त;
	llist_t *list = रिक्त;
	पूर्णांक retval = निकास_सफल;

#जर ENABLE_DESKTOP
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
	जर (!*argv)
		*--argv = (अक्षर *)"-";
	/* We will read from last file to first */
	name = argv;
	पर्यंत (*name)
		name++;

	करा {
		पूर्णांक ch, i;

		name--;
		f = fopen_or_warn_stdin(*name);
		जर (f == रिक्त) {
			/* error message is printed by fopen_or_warn_stdin */
			retval = निकास_त्रुटि;
			सुरूठेवा;
		}

		त्रुटि_सं = i = 0;
		करा {
			ch = ख_अक्षर_लो(f);
			जर (ch != खातापूर्ण) {
				जर (!(i & 0x7f))
					/* Grow on every 128th char */
					ओळ = xrealloc(ओळ, i + 0x7f + आकार(पूर्णांक) + 1);
				ओळ->buf[i++] = ch;
			}
			जर (ch == '\n' || (ch == खातापूर्ण && i != 0)) {
				ओळ = xrealloc(ओळ, i + आकार(पूर्णांक));
				ओळ->size = i;
				llist_add_to(&list, ओळ);
				ओळ = रिक्त;
				i = 0;
			}
		} पर्यंत (ch != खातापूर्ण);
		/* fgetc sets errno to ENOENT on EOF, we don't want
		 * to warn on this non-error! */
		जर (त्रुटि_सं && त्रुटि_सं != ENOENT) {
			bb_simple_perror_संदेश(*name);
			retval = निकास_त्रुटि;
		}
	} पर्यंत (name != argv);

	पर्यंत (list) {
		ओळ = (संरचना lstring *)list->data;
		xwrite(STDOUT_FILENO, ओळ->buf, ओळ->size);
		जर (ENABLE_FEATURE_CLEAN_UP) {
			मुक्त(llist_काढा(&list));
		} अन्यथा {
			list = list->link;
		}
	}

	परत retval;
}
