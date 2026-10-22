/* vi: set sw=4 ts=4: */
/*
 * Mini logname implementation for busybox
 *
 * Copyright (C) 2000  Edward Betts <edward@debian.org>.
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
/* Mar 16, 2003      Manuel Novoa III   (mjn3@codepoet.org)
 *
 * SUSv3 specifies the string used is that returned from getlogin().
 * The previous implementation used getpwuid() for geteuid(), which
 * is _not_ the same.  Erik apparently made this change almost 3 years
 * ago to avoid failing when no utmp was available.  However, the
 * correct course of action wrt SUSv3 for a failing getlogin() is
 * a diagnostic message and an error return.
 */
//config:config LOGNAME
//config:	bool "logname (1.4 kb)"
//config:	default y
//config:	help
//config:	logname is used to print the current user's login name.

//applet:IF_LOGNAME(APPLET_NOFORK(logname, logname, BB_DIR_USR_BIN, BB_SUID_DROP, logname))

//kbuild:lib-$(CONFIG_LOGNAME) += logname.o

/* BB_AUDIT SUSv3 compliant */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/logname.html */

//usage:#define logname_trivial_usage
//usage:       ""
//usage:#define logname_full_usage "\n\n"
//usage:       "Print the name of the current user"
//usage:
//usage:#define logname_example_usage
//usage:       "$ logname\n"
//usage:       "root\n"

#अन्तर्भाव "libbb.h"

/* This is a NOFORK applet. Be very careful! */

पूर्णाङ्क logname_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क logname_main(पूर्णाङ्क argc UNUSED_PARAM, वर्ण **argv UNUSED_PARAM)
{
	वर्ण buf[64];

	यदि (argv[1]) {
		bb_show_usage();
	}

	/* Using _r function - avoid pulling in static buffer from libc */
	यदि (getlogin_r(buf, परिमाण(buf)) == 0) {
		माला_दो(buf);
		प्रत्यागम fflush_all();
	}

	bb_simple_perror_संदेश_and_die("getlogin");
}
