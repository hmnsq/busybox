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

#अन्तर्भाव "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

पूर्णाङ्क rm_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क rm_main(पूर्णाङ्क argc UNUSED_PARAM, वर्ण **argv)
{
	पूर्णाङ्क status = 0;
	पूर्णाङ्क flags = 0;
	unsigned opt;

	opt = getopt32(argv, "^" "fiRrv" "\0" "f-i:i-f");
	argv += optind;
	यदि (opt & 1)
		flags |= FILEUTILS_FORCE;
	यदि (opt & 2)
		flags |= FILEUTILS_INTERACTIVE;
	यदि (opt & (8|4))
		flags |= FILEUTILS_RECUR;
	यदि ((opt & 16) && FILEUTILS_VERBOSE)
		flags |= FILEUTILS_VERBOSE;

	यदि (*argv != रिक्त) {
		कुरु {
			ध्रुव वर्ण *base = bb_get_last_path_component_strip(*argv);

			यदि (DOT_OR_DOTDOT(base)) {
				bb_simple_error_संदेश("can't remove '.' or '..'");
			} अन्यथा यदि (remove_file(*argv, flags) >= 0) {
				अनुवर्त;
			}
			status = 1;
		} यावत् (*++argv);
	} अन्यथा यदि (!(flags & FILEUTILS_FORCE)) {
		bb_show_usage();
	}

	प्रत्यागम status;
}
