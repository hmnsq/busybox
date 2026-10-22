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

#شامل "libbb.h"

صحیح realpath_main(صحیح argc, نویسه **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح realpath_main(صحیح argc UNUSED_PARAM, نویسه **argv)
{
	صحیح retval = EXIT_SUCCESS;

	اگر (!*++argv) {
		bb_show_usage();
	}

	انجامبده {
		/* NOFORK: only one alloc is allowed; must free */
		نویسه *resolved_path = xmalloc_realpath_coreutils(*argv);
		اگر (resolved_path != پوچ) {
			puts(resolved_path);
			آزاد(resolved_path);
		} وگرنه {
			retval = EXIT_FAILURE;
			bb_simple_perror_پیام(*argv);
		}
	} وقتی (*++argv);

	fflush_stdout_and_exit(retval);
}
