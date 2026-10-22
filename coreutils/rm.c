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

#ضمن "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

صحيح rm_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح rm_main(صحيح argc اڻ_استعمال_ٿيل_PARAM, حرف **argv)
{
	صحيح حالت = 0;
	صحيح flags = 0;
	بنانشان opt;

	opt = getopt32(argv, "^" "fiRrv" "\0" "f-i:i-f");
	argv += optind;
	إذا (opt & 1)
		flags |= FILEUTILS_FORCE;
	إذا (opt & 2)
		flags |= FILEUTILS_INTERACTIVE;
	إذا (opt & (8|4))
		flags |= FILEUTILS_RECUR;
	إذا ((opt & 16) && FILEUTILS_VERBOSE)
		flags |= FILEUTILS_VERBOSE;

	إذا (*argv != عدم) {
		افعل {
			ثابت حرف *بنياد = bb_get_آخري_رستو_component_strip(*argv);

			إذا (DOT_OR_DOTDOT(بنياد)) {
				bb_simple_error_نياپو("can't remove '.' or '..'");
			} وإلا إذا (remove_file(*argv, flags) >= 0) {
				استمر;
			}
			حالت = 1;
		} طالما (*++argv);
	} وإلا إذا (!(flags & FILEUTILS_FORCE)) {
		bb_show_usage();
	}

	أرجع حالت;
}
