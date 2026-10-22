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

#ಅನ್ತರ್ಭಾವ "libbb.h"

/* This is a NOFORK applet. Be very careful! */

ಪೂರ್ಣಾಂಕ link_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ link_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	getopt32(argv, "^" "" "\0" "=2");
	argv += optind;
	ಆದರೆ (link(argv[0], argv[1]) != 0) {
		/* shared message */
		bb_perror_ಸಂದೇಶ_and_die("can't create %slink '%s' to '%s'",
			"hard",	argv[1], argv[0]
		);
	}
	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}
