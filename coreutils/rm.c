/* vi: set sw=4 ts=4: */
/*
 * Mini rm implementation for busybox
 *
 * Copyright (C) 2001 Matt Kraai <kraai@alumni.carnegiemellon.edu>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
/* Mar 16, 2003      Manuel Novoa III   (mjn3@codepoet.org)
 *
 * Size reduction.
 */
//config:config RM
//config:	bool "rm (5.5 kb)"
//config:	default y
//config:	help
//config:	rm is used to remove files or directories.

//applet:IF_RM(APPLET_NOEXEC(rm, rm, BB_DIR_BIN, BB_SUID_DROP, rm))
/* was NOFORK, but then "rm -i FILE" can't be ^C'ed if run by hush */

//kbuild:lib-$(CONFIG_RM) += rm.o

/* BB_AUDIT SUSv3 compliant */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/rm.html */

//usage:#define rm_trivial_usage
//usage:       "[-irf] FILE..."
//usage:#define rm_full_usage "\n\n"
//usage:       "Remove (unlink) FILEs\n"
//usage:     "\n	-i	Always prompt before removing"
//usage:     "\n	-f	Never prompt"
//usage:     "\n	-R,-r	Recurse"
//usage:
//usage:#define rm_example_usage
//usage:       "$ rm -rf /tmp/foo\n"

#include "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

整数 rm_main(整数 argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
整数 rm_main(整数 argc UNUSED_PARAM, char **argv)
{
	整数 status = 0;
	整数 flags = 0;
	unsigned opt;

	opt = getopt32(argv, "^" "fiRrv" "\0" "f-i:i-f");
	argv += optind;
	如果 (opt & 1)
		flags |= FILEUTILS_FORCE;
	如果 (opt & 2)
		flags |= FILEUTILS_INTERACTIVE;
	如果 (opt & (8|4))
		flags |= FILEUTILS_RECUR;
	如果 ((opt & 16) && FILEUTILS_VERBOSE)
		flags |= FILEUTILS_VERBOSE;

	如果 (*argv != NULL) {
		do {
			const char *base = bb_get_last_path_component_strip(*argv);

			如果 (DOT_OR_DOTDOT(base)) {
				bb_simple_error_msg("can't remove '.' or '..'");
			} 否则 如果 (remove_file(*argv, flags) >= 0) {
				continue;
			}
			status = 1;
		} 当 (*++argv);
	} 否则 如果 (!(flags & FILEUTILS_FORCE)) {
		bb_show_usage();
	}

	返回 status;
}
