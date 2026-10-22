/* vi: set sw=4 ts=4: */
/*
 * Mini mkdir implementation for busybox
 *
 * Copyright (C) 2001 Matt Kraai <kraai@alumni.carnegiemellon.edu>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
/* Mar 16, 2003      Manuel Novoa III   (mjn3@codepoet.org)
 *
 * Fixed broken permission setting when -p was used; especially in
 * conjunction with -m.
 */
/* Nov 28, 2006      Yoshinori Sato <ysato@users.sourceforge.jp>: Add SELinux Support.
 */
//config:config MKDIR
//config:	bool "mkdir (4.7 kb)"
//config:	default y
//config:	help
//config:	mkdir is used to create directories with the specified names.

//applet:IF_MKDIR(APPLET_NOFORK(mkdir, mkdir, BB_DIR_BIN, BB_SUID_DROP, mkdir))

//kbuild:lib-$(CONFIG_MKDIR) += mkdir.o

/* BB_AUDIT SUSv3 compliant */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/mkdir.html */

//usage:#define mkdir_trivial_usage
//usage:       "[-m MODE] [-p] DIRECTORY..."
//usage:#define mkdir_full_usage "\n\n"
//usage:       "Create DIRECTORY\n"
//usage:     "\n	-m MODE	Mode"
//usage:     "\n	-p	No error if exists; make parent directories as needed"
//usage:	IF_SELINUX(
//usage:     "\n	-Z	Set security context"
//usage:	)
//usage:
//usage:#define mkdir_example_usage
//usage:       "$ mkdir /tmp/foo\n"
//usage:       "$ mkdir /tmp/foo\n"
//usage:       "/tmp/foo: File exists\n"
//usage:       "$ mkdir /tmp/foo/bar/baz\n"
//usage:       "/tmp/foo/bar/baz: No such file or directory\n"
//usage:       "$ mkdir -p /tmp/foo/bar/baz\n"

#include "libbb.h"

/* This is a NOFORK applet. Be very careful! */

จำนวนเต็ม mkdir_main(จำนวนเต็ม argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
จำนวนเต็ม mkdir_main(จำนวนเต็ม argc UNUSED_PARAM, var_na **argv)
{
	long mode = -1;
	จำนวนเต็ม status = EXIT_SUCCESS;
	จำนวนเต็ม flags = 0;
	unsigned opt;
	var_na *smode;
#ถ้า ENABLE_SELINUX
	security_context_t scontext;
#endif

	opt = getopt32long(argv, "m:pv" IF_SELINUX("Z:"),
			"mode\0"    Required_argument "m"
			"parents\0" No_argument       "p"
# ถ้า ENABLE_SELINUX
			"context\0" Required_argument "Z"
# endif
# ถ้า ENABLE_FEATURE_VERBOSE
			"verbose\0" No_argument       "v"
# endif
			, &smode IF_SELINUX(,&scontext)
	);
	ถ้า (opt & 1) {
		mode_t mmode = bb_parse_mode(smode, 0777);
		ถ้า (mmode == (mode_t)-1) {
			bb_error_msg_and_die("invalid mode '%s'", smode);
		}
		mode = mmode;
	}
	ถ้า (opt & 2)
		flags |= FILEUTILS_RECUR;
	ถ้า ((opt & 4) && FILEUTILS_VERBOSE)
		flags |= FILEUTILS_VERBOSE;
#ถ้า ENABLE_SELINUX
	ถ้า (opt & 8) {
		selinux_or_die();
		setfscreatecon_or_die(scontext);
	}
#endif

	argv += optind;
	ถ้า (!argv[0])
		bb_show_usage();

	do {
		ถ้า (bb_make_directory(*argv, mode, flags)) {
			status = EXIT_FAILURE;
		}
	} ขณะ (*++argv);

	คืน status;
}
