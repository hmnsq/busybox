/*
 * link implementation for busybox
 *
 * Copyright (C) 2017 Denys Vlasenko <vda.linux@googlemail.com>
 *
 * Licensed under GPLv2, see file LICENSE in this source tree.
 */
//config:config LINK
//config:	bool "link (3.5 kb)"
//config:	default y
//config:	help
//config:	link creates hard links between files.

//applet:IF_LINK(APPLET_NOFORK(link, link, BB_DIR_BIN, BB_SUID_DROP, link))

//kbuild:lib-$(CONFIG_LINK) += link.o

//usage:#define link_trivial_usage
//usage:       "FILE LINK"
//usage:#define link_full_usage "\n\n"
//usage:       "Create hard LINK to FILE"

#include "libbb.h"

/* This is a NOFORK applet. Be very careful! */

정수 link_main(정수 argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
정수 link_main(정수 argc UNUSED_PARAM, char **argv)
{
	getopt32(argv, "^" "" "\0" "=2");
	argv += optind;
	만약 (link(argv[0], argv[1]) != 0) {
		/* shared message */
		bb_perror_msg_and_die("can't create %slink '%s' to '%s'",
			"hard",	argv[1], argv[0]
		);
	}
	반환 EXIT_SUCCESS;
}
