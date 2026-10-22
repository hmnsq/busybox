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

#अन्तर्भाव "libbb.h"

पूर्णाङ्क realpath_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क realpath_main(पूर्णाङ्क argc अप्रयुक्तम्_PARAM, वर्ण **argv)
{
	पूर्णाङ्क retval = EXIT_SUCCESS;

	यदि (!*++argv) {
		bb_show_usage();
	}

	कुरु {
		/* NOFORK: only one alloc is allowed; must free */
		वर्ण *resolved_मार्गः = xmalloc_realpath_coreutils(*argv);
		यदि (resolved_मार्गः != रिक्त) {
			पङ्क्तिं_लिख(resolved_मार्गः);
			मुक्त(resolved_मार्गः);
		} अन्यथा {
			retval = EXIT_FAILURE;
			bb_simple_perror_सन्देशः(*argv);
		}
	} यावत् (*++argv);

	fflush_stdout_and_exit(retval);
}
