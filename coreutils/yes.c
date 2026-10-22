/* vi: set sw=4 ts=4: */
/*
 * yes implementation for busybox
 *
 * Copyright (C) 2003  Manuel Novoa III  <mjn3@codepoet.org>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
/* Mar 16, 2003      Manuel Novoa III   (mjn3@codepoet.org)
 *
 * Size reductions and removed redundant applet name prefix from error messages.
 */
//config:config YES
//config:	bool "yes (1.5 kb)"
//config:	default y
//config:	help
//config:	yes is used to repeatedly output a specific string, or
//config:	the default string 'y'.

//applet:IF_YES(APPLET_NOEXEC(yes, yes, BB_DIR_USR_BIN, BB_SUID_DROP, yes))
/* was NOFORK, but then yes can't be ^C'ed if run by hush */

//kbuild:lib-$(CONFIG_YES) += yes.o

/* BB_AUDIT SUSv3 N/A -- Matches GNU behavior. */

//usage:#define yes_trivial_usage
//usage:       "[STRING]"
//usage:#define yes_full_usage "\n\n"
//usage:       "Repeatedly print a line with STRING, or 'y'"

#अन्तर्भाव "libbb.h"

पूर्णाङ्क yes_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क yes_main(पूर्णाङ्क argc UNUSED_PARAM, वर्ण **argv)
{
	वर्ण **pp;

	argv[0] = (वर्ण*)"y";
	यदि (argv[1])
		++argv;

	कुरु {
		pp = argv;
		यावत् (1) {
			fputs_stdout(*pp);
			यदि (!*++pp)
				भङ्ग;
			putchar(' ');
		}
	} यावत् (putchar('\n') != EOF);

	bb_perror_nomsg_and_die();
}
