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

#अन्तर्भाव "libbb.h"

पूर्णाङ्क unlink_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क unlink_main(पूर्णाङ्क argc UNUSED_PARAM, वर्ण **argv)
{
	getopt32(argv, "^" "" "\0" "=1");
	argv += optind;
	xunlink(argv[0]);
	फर्काऊ 0;
}
