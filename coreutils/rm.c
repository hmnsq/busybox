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

#شامل "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

صحیح rm_main(صحیح argc, نویسه **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح rm_main(صحیح argc UNUSED_PARAM, نویسه **argv)
{
	صحیح status = 0;
	صحیح flags = 0;
	unsigned opt;

	opt = getopt32(argv, "^" "fiRrv" "\0" "f-i:i-f");
	argv += optind;
	اگر (opt & 1)
		flags |= FILEUTILS_FORCE;
	اگر (opt & 2)
		flags |= FILEUTILS_INTERACTIVE;
	اگر (opt & (8|4))
		flags |= FILEUTILS_RECUR;
	اگر ((opt & 16) && FILEUTILS_VERBOSE)
		flags |= FILEUTILS_VERBOSE;

	اگر (*argv != پوچ) {
		انجامبده {
			ثابت نویسه *base = bb_get_last_path_component_strip(*argv);

			اگر (DOT_OR_DOTDOT(base)) {
				bb_simple_error_پیام("can't remove '.' or '..'");
			} وگرنه اگر (remove_file(*argv, flags) >= 0) {
				ادامه;
			}
			status = 1;
		} وقتی (*++argv);
	} وگرنه اگر (!(flags & FILEUTILS_FORCE)) {
		bb_show_usage();
	}

	بازگشت status;
}
