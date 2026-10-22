/* vi: set sw=4 ts=4: */
/*
 * Mar 16, 2003      Manuel Novoa III   (mjn3@codepoet.org)
 *
 * Now does proper error checking on output and returns a failure exit code
 * if one or more paths cannot be resolved.
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config REALPATH
//config:	bool "realpath (2.5 kb)"
//config:	default y
//config:	help
//config:	Return the canonicalized absolute pathname.
//config:	This isn't provided by GNU shellutils, but where else does it belong.

//applet:IF_REALPATH(APPLET_NOFORK(realpath, realpath, BB_DIR_USR_BIN, BB_SUID_DROP, realpath))

//kbuild:lib-$(CONFIG_REALPATH) += realpath.o

/* BB_AUDIT SUSv3 N/A -- Apparently a busybox extension. */

//usage:#define realpath_trivial_usage
//usage:       "FILE..."
//usage:#define realpath_full_usage "\n\n"
//usage:       "Print absolute pathnames of FILEs"

#ضمن "libbb.h"

صحيح realpath_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح realpath_main(صحيح argc UNUSED_PARAM, حرف **argv)
{
	صحيح retval = EXIT_SUCCESS;

	إذا (!*++argv) {
		bb_show_usage();
	}

	افعل {
		/* NOFORK: only one alloc is allowed; must free */
		حرف *resolved_path = xmalloc_realpath_coreutils(*argv);
		إذا (resolved_path != عدم) {
			puts(resolved_path);
			حرر(resolved_path);
		} وإلا {
			retval = EXIT_FAILURE;
			bb_simple_perror_پیغام(*argv);
		}
	} طالما (*++argv);

	fflush_stdout_and_exit(retval);
}
