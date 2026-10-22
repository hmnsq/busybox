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

#അന്തര്ഭാവ "libbb.h"

പൂർണ്ണസംഖ്യ realpath_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) MAIN_EXTERNALLY_VISIBLE;
പൂർണ്ണസംഖ്യ realpath_main(പൂർണ്ണസംഖ്യ argc UNUSED_PARAM, അക്ഷരം **argv)
{
	പൂർണ്ണസംഖ്യ retval = EXIT_SUCCESS;

	എങ്കിൽ (!*++argv) {
		bb_show_usage();
	}

	ചെയ്യുക {
		/* NOFORK: only one alloc is allowed; must free */
		അക്ഷരം *resolved_path = xmalloc_realpath_coreutils(*argv);
		എങ്കിൽ (resolved_path != ശൂന്യ) {
			puts(resolved_path);
			സ്വതന്ത്രം(resolved_path);
		} അല്ലെങ്കിൽ {
			retval = EXIT_FAILURE;
			bb_simple_perror_സന്ദേശം(*argv);
		}
	} ആകുമ്പോൾ (*++argv);

	fflush_stdout_and_exit(retval);
}
