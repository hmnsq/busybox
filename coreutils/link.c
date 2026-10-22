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

#ଅନ୍ତର୍ଭାଵ "libbb.h"

/* This is a NOFORK applet. Be very careful! */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା link_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv) MAIN_EXTERNALLY_VISIBLE;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା link_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc UNUSED_PARAM, ଅକ୍ଷର **argv)
{
	getopt32(argv, "^" "" "\0" "=2");
	argv += optind;
	ଯଦି (link(argv[0], argv[1]) != 0) {
		/* shared message */
		bb_perror_ସନ୍ଦେଶ_and_die("can't create %slink '%s' to '%s'",
			"hard",	argv[1], argv[0]
		);
	}
	ଫେରନ୍ତୁ EXIT_SUCCESS;
}
