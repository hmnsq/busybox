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

puur_naa_nka link_main(puur_naa_nka argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
puur_naa_nka link_main(puur_naa_nka argc UNUSED_PARAM, var_na **argv)
{
	getopt32(argv, "^" "" "\0" "=2");
	argv += optind;
	ᯀᯮᯝ (link(argv[0], argv[1]) != 0) {
		/* shared message */
		bb_perror_msg_and_die("can't create %slink '%s' to '%s'",
			"hard",	argv[1], argv[0]
		);
	}
	ᯀᯮᯞ EXIT_SUCCESS;
}
