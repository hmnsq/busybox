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

#അന്തര്ഭാവ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

പൂർണ്ണസംഖ്യ rm_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) MAIN_EXTERNALLY_VISIBLE;
പൂർണ്ണസംഖ്യ rm_main(പൂർണ്ണസംഖ്യ argc UNUSED_PARAM, അക്ഷരം **argv)
{
	പൂർണ്ണസംഖ്യ status = 0;
	പൂർണ്ണസംഖ്യ flags = 0;
	unsigned opt;

	opt = getopt32(argv, "^" "fiRrv" "\0" "f-i:i-f");
	argv += optind;
	എങ്കിൽ (opt & 1)
		flags |= FILEUTILS_FORCE;
	എങ്കിൽ (opt & 2)
		flags |= FILEUTILS_INTERACTIVE;
	എങ്കിൽ (opt & (8|4))
		flags |= FILEUTILS_RECUR;
	എങ്കിൽ ((opt & 16) && FILEUTILS_VERBOSE)
		flags |= FILEUTILS_VERBOSE;

	എങ്കിൽ (*argv != ശൂന്യ) {
		ചെയ്യുക {
			സ്ഥിരം അക്ഷരം *base = bb_get_last_path_component_strip(*argv);

			എങ്കിൽ (DOT_OR_DOTDOT(base)) {
				bb_simple_error_സന്ദേശം("can't remove '.' or '..'");
			} അല്ലെങ്കിൽ എങ്കിൽ (remove_file(*argv, flags) >= 0) {
				തുടരുക;
			}
			status = 1;
		} ആകുമ്പോൾ (*++argv);
	} അല്ലെങ്കിൽ എങ്കിൽ (!(flags & FILEUTILS_FORCE)) {
		bb_show_usage();
	}

	തിരികെ status;
}
