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

#அந்தர்பாவ "libbb.h"

முழு yes_main(முழு argc, எழுத்து **argv) MAIN_EXTERNALLY_VISIBLE;
முழு yes_main(முழு argc UNUSED_PARAM, எழுத்து **argv)
{
	எழுத்து **pp;

	argv[0] = (எழுத்து*)"y";
	எனில் (argv[1])
		++argv;

	செய் {
		pp = argv;
		போது (1) {
			fputs_stdout(*pp);
			எனில் (!*++pp)
				நிறுத்து;
			putchar(' ');
		}
	} போது (putchar('\n') != EOF);

	bb_perror_nomsg_and_die();
}
