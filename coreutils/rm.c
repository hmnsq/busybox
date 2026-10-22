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

#অন্তর্ভাব "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

পূর্ণসংখ্যা rm_main(পূর্ণসংখ্যা argc, অক্ষর **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণসংখ্যা rm_main(পূর্ণসংখ্যা argc UNUSED_PARAM, অক্ষর **argv)
{
	পূর্ণসংখ্যা status = 0;
	পূর্ণসংখ্যা flags = 0;
	unsigned opt;

	opt = getopt32(argv, "^" "fiRrv" "\0" "f-i:i-f");
	argv += optind;
	যদি (opt & 1)
		flags |= FILEUTILS_FORCE;
	যদি (opt & 2)
		flags |= FILEUTILS_INTERACTIVE;
	যদি (opt & (8|4))
		flags |= FILEUTILS_RECUR;
	যদি ((opt & 16) && FILEUTILS_VERBOSE)
		flags |= FILEUTILS_VERBOSE;

	যদি (*argv != ফাঁকা) {
		করো {
			ধ্রুবক অক্ষর *base = bb_get_last_path_component_strip(*argv);

			যদি (DOT_OR_DOTDOT(base)) {
				bb_simple_error_বার্তা("can't remove '.' or '..'");
			} অন্যথা যদি (remove_file(*argv, flags) >= 0) {
				চালু;
			}
			status = 1;
		} যখন (*++argv);
	} অন্যথা যদি (!(flags & FILEUTILS_FORCE)) {
		bb_show_usage();
	}

	ফেরত status;
}
