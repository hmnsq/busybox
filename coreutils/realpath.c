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

#અન્તર્ભાવ "libbb.h"

પૂર્ણાંક realpath_main(પૂર્ણાંક argc, અક્ષર **argv) MAIN_EXTERNALLY_VISIBLE;
પૂર્ણાંક realpath_main(પૂર્ણાંક argc UNUSED_PARAM, અક્ષર **argv)
{
	પૂર્ણાંક retval = EXIT_SUCCESS;

	જો (!*++argv) {
		bb_show_usage();
	}

	કરો {
		/* NOFORK: only one alloc is allowed; must free */
		અક્ષર *resolved_path = xmalloc_realpath_coreutils(*argv);
		જો (resolved_path != ખાલી) {
			puts(resolved_path);
			મુક્ત(resolved_path);
		} નહીંતર {
			retval = EXIT_FAILURE;
			bb_simple_perror_સંદેશ(*argv);
		}
	} જ્યારે (*++argv);

	fflush_stdout_and_exit(retval);
}
