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

#समावेश "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

पूर्णांक rm_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक rm_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	पूर्णांक status = 0;
	पूर्णांक flags = 0;
	अचिन्हित opt;

	opt = getopt32(argv, "^" "fiRrv" "\0" "f-i:i-f");
	argv += optind;
	अगर (opt & 1)
		flags |= FILEUTILS_FORCE;
	अगर (opt & 2)
		flags |= FILEUTILS_INTERACTIVE;
	अगर (opt & (8|4))
		flags |= FILEUTILS_RECUR;
	अगर ((opt & 16) && FILEUTILS_VERBOSE)
		flags |= FILEUTILS_VERBOSE;

	अगर (*argv != शून्य) {
		करो {
			स्थिर अक्षर *base = bb_get_last_path_component_strip(*argv);

			अगर (DOT_OR_DOTDOT(base)) {
				bb_simple_error_संदेश("can't remove '.' or '..'");
			} अन्यथा अगर (remove_file(*argv, flags) >= 0) {
				जारी;
			}
			status = 1;
		} जबतक (*++argv);
	} अन्यथा अगर (!(flags & FILEUTILS_FORCE)) {
		bb_show_usage();
	}

	वापस status;
}
