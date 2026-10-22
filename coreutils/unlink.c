/* vi: set sw=4 ts=4: */
/*
 * unlink for busybox
 *
 * Copyright (C) 2014 Isaac Dunham <ibid.ag@gmail.com>
 *
 * Licensed under GPLv2, see LICENSE in this source tree
 */
//config:config UNLINK
//config:	bool "unlink (3.5 kb)"
//config:	default y
//config:	help
//config:	unlink deletes a file by calling unlink()

//applet:IF_UNLINK(APPLET_NOFORK(unlink, unlink, BB_DIR_USR_BIN, BB_SUID_DROP, unlink))

//kbuild:lib-$(CONFIG_UNLINK) += unlink.o

//usage:#define unlink_trivial_usage
//usage:	"FILE"
//usage:#define unlink_full_usage "\n\n"
//usage:	"Delete FILE by calling unlink()"

#അന്തര്ഭാവ "libbb.h"

പൂർണ്ണസംഖ്യ unlink_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) MAIN_EXTERNALLY_VISIBLE;
പൂർണ്ണസംഖ്യ unlink_main(പൂർണ്ണസംഖ്യ argc UNUSED_PARAM, അക്ഷരം **argv)
{
	getopt32(argv, "^" "" "\0" "=1");
	argv += optind;
	xunlink(argv[0]);
	തിരികെ 0;
}
