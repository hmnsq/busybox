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

#ಅನ್ತರ್ಭಾವ "libbb.h"

ಪೂರ್ಣಾಂಕ realpath_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ realpath_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	ಪೂರ್ಣಾಂಕ retval = EXIT_SUCCESS;

	ಆದರೆ (!*++argv) {
		bb_show_usage();
	}

	ಮಾಡು {
		/* NOFORK: only one alloc is allowed; must free */
		ಅಕ್ಷರ *resolved_ಮಾರ್ಗ = xmalloc_realpath_coreutils(*argv);
		ಆದರೆ (resolved_ಮಾರ್ಗ != ಖಾಲಿ) {
			puts(resolved_ಮಾರ್ಗ);
			ಬಿಡುಗಡೆ(resolved_ಮಾರ್ಗ);
		} ಇಲ್ಲದಿದ್ದರೆ {
			retval = EXIT_FAILURE;
			bb_simple_perror_ಸಂದೇಶ(*argv);
		}
	} ಆಗ (*++argv);

	fflush_stdout_and_exit(retval);
}
