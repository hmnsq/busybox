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

整数 mkdir_main(整数 argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
整数 mkdir_main(整数 argc UNUSED_PARAM, char **argv)
{
	long mode = -1;
	整数 status = EXIT_SUCCESS;
	整数 flags = 0;
	unsigned opt;
	char *smode;
#如果 ENABLE_SELINUX
	security_context_t scontext;
#endif

	opt = getopt32long(argv, "m:pv" IF_SELINUX("Z:"),
			"mode\0"    Required_argument "m"
			"parents\0" No_argument       "p"
# 如果 ENABLE_SELINUX
			"context\0" Required_argument "Z"
# endif
# 如果 ENABLE_FEATURE_VERBOSE
			"verbose\0" No_argument       "v"
# endif
			, &smode IF_SELINUX(,&scontext)
	);
	如果 (opt & 1) {
		mode_t mmode = bb_parse_mode(smode, 0777);
		如果 (mmode == (mode_t)-1) {
			bb_error_msg_and_die("invalid mode '%s'", smode);
		}
		mode = mmode;
	}
	如果 (opt & 2)
		flags |= FILEUTILS_RECUR;
	如果 ((opt & 4) && FILEUTILS_VERBOSE)
		flags |= FILEUTILS_VERBOSE;
#如果 ENABLE_SELINUX
	如果 (opt & 8) {
		selinux_or_die();
		setfscreatecon_or_die(scontext);
	}
#endif

	argv += optind;
	如果 (!argv[0])
		bb_show_usage();

	do {
		如果 (bb_make_directory(*argv, mode, flags)) {
			status = EXIT_FAILURE;
		}
	} 当 (*++argv);

	返回 status;
}
