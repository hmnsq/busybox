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

#ಅನ್ತರ್ಭಾವ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

ಪೂರ್ಣಾಙ್ಕ rm_main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಙ್ಕ rm_main(ಪೂರ್ಣಾಙ್ಕ argc UNUSED_PARAM, ವರ್ಣ **argv)
{
	ಪೂರ್ಣಾಙ್ಕ status = 0;
	ಪೂರ್ಣಾಙ್ಕ flags = 0;
	unsigned opt;

	opt = getopt32(argv, "^" "fiRrv" "\0" "f-i:i-f");
	argv += optind;
	ಯದಿ (opt & 1)
		flags |= FILEUTILS_FORCE;
	ಯದಿ (opt & 2)
		flags |= FILEUTILS_INTERACTIVE;
	ಯದಿ (opt & (8|4))
		flags |= FILEUTILS_RECUR;
	ಯದಿ ((opt & 16) && FILEUTILS_VERBOSE)
		flags |= FILEUTILS_VERBOSE;

	ಯದಿ (*argv != ರಿಕ್ತ) {
		ಕುರು {
			ಧ್ರುವ ವರ್ಣ *base = bb_get_last_path_component_strip(*argv);

			ಯದಿ (DOT_OR_DOTDOT(base)) {
				bb_simple_error_ಸಂದೇಶ("can't remove '.' or '..'");
			} ಅನ್ಯಥಾ ಯದಿ (remove_file(*argv, flags) >= 0) {
				ಅನುವರ್ತ;
			}
			status = 1;
		} ಯಾವತ್ (*++argv);
	} ಅನ್ಯಥಾ ಯದಿ (!(flags & FILEUTILS_FORCE)) {
		bb_show_usage();
	}

	ಪ್ರತ್ಯಾಗಮ status;
}
