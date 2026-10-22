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

#অন্তর্ভাৱ "libbb.h"

পূর্ণাঙ্ক realpath_main(পূর্ণাঙ্ক argc, ৱর্ণ **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণাঙ্ক realpath_main(পূর্ণাঙ্ক argc UNUSED_PARAM, ৱর্ণ **argv)
{
	পূর্ণাঙ্ক retval = EXIT_SUCCESS;

	যদি (!*++argv) {
		bb_show_usage();
	}

	কৰা {
		/* NOFORK: only one alloc is allowed; must free */
		ৱর্ণ *resolved_path = xmalloc_realpath_coreutils(*argv);
		যদি (resolved_path != রিক্ত) {
			puts(resolved_path);
			মুক্ত(resolved_path);
		} অন্যথা {
			retval = EXIT_FAILURE;
			bb_simple_perror_বাৰ্তা(*argv);
		}
	} যেতিয়ালৈ (*++argv);

	fflush_stdout_and_exit(retval);
}
