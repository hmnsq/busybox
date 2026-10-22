/* vi: set sw=4 ts=4: */
/*
 * Mini hostid implementation for busybox
 *
 * Copyright (C) 2000  Edward Betts <edward@debian.org>.
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config HOSTID
//config:	bool "hostid (566 bytes)"
//config:	default y
//config:	help
//config:	hostid prints the numeric identifier (in hexadecimal) for
//config:	the current host.

//applet:IF_HOSTID(APPLET_NOFORK(hostid, hostid, BB_DIR_USR_BIN, BB_SUID_DROP, hostid))

//kbuild:lib-$(CONFIG_HOSTID) += hostid.o

/* BB_AUDIT SUSv3 N/A -- Matches GNU behavior. */

//usage:#define hostid_trivial_usage
//usage:       ""
//usage:#define hostid_full_usage "\n\n"
//usage:       "Print out a unique 32-bit identifier for the machine"

#شامل "libbb.h"

/* This is a NOFORK applet. Be very careful! */

صحیح hostid_main(صحیح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح hostid_main(صحیح argc غیر_مستعمل_PARAM, حرف **argv غیر_مستعمل_PARAM)
{
	اگر (argv[1]) {
		bb_show_usage();
	}

	/* POSIX says gethostid returns a "32-bit identifier" */
	لکھو("%08x\n", (غیرعلامتی)(uint32_t)gethostid());

	واپس fflush_all();
}
