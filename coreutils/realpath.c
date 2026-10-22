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

#ଅନ୍ତର୍ଭାଵ "libbb.h"

ପୂର୍ଣ୍ଣସଂଖ୍ୟା realpath_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv) MAIN_EXTERNALLY_VISIBLE;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା realpath_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc UNUSED_PARAM, ଅକ୍ଷର **argv)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval = EXIT_SUCCESS;

	ଯଦି (!*++argv) {
		bb_show_usage();
	}

	କର {
		/* NOFORK: only one alloc is allowed; must free */
		ଅକ୍ଷର *resolved_path = xmalloc_realpath_coreutils(*argv);
		ଯଦି (resolved_path != ଖାଲି) {
			puts(resolved_path);
			ମୁକ୍ତ(resolved_path);
		} ଅନ୍ୟଥା {
			retval = EXIT_FAILURE;
			bb_simple_perror_ସନ୍ଦେଶ(*argv);
		}
	} ଯେତେବେଳେ (*++argv);

	fflush_stdout_and_exit(retval);
}
