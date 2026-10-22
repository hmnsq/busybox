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

#समावेश "libbb.h"

पूर्णांक realpath_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक realpath_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	पूर्णांक retval = निकास_सफल;

	अगर (!*++argv) {
		bb_show_usage();
	}

	करो {
		/* NOFORK: only one alloc is allowed; must free */
		अक्षर *resolved_path = xmalloc_realpath_coreutils(*argv);
		अगर (resolved_path != शून्य) {
			माला_दो(resolved_path);
			मुक्त(resolved_path);
		} अन्यथा {
			retval = निकास_त्रुटि;
			bb_simple_perror_संदेश(*argv);
		}
	} जबतक (*++argv);

	fflush_stdout_and_exit(retval);
}
