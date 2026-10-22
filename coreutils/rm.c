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

#అన్తర్భావ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

పూర్ణాంకం rm_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం rm_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	పూర్ణాంకం స్థితి_2 = 0;
	పూర్ణాంకం flags = 0;
	గుర్తులేని opt;

	opt = getopt32(argv, "^" "fiRrv" "\0" "f-i:i-f");
	argv += optind;
	ఒకవేళ (opt & 1)
		flags |= FILEUTILS_FORCE;
	ఒకవేళ (opt & 2)
		flags |= FILEUTILS_INTERACTIVE;
	ఒకవేళ (opt & (8|4))
		flags |= FILEUTILS_RECUR;
	ఒకవేళ ((opt & 16) && FILEUTILS_VERBOSE)
		flags |= FILEUTILS_VERBOSE;

	ఒకవేళ (*argv != ఖాళీ) {
		చేయి {
			స్థిరరాశి అక్షరం *పునాది = bb_get_చివరి_మార్గం_component_strip(*argv);

			ఒకవేళ (DOT_OR_DOTDOT(పునాది)) {
				bb_simple_error_సందేశం("can't remove '.' or '..'");
			} లేకపోతే ఒకవేళ (remove_file(*argv, flags) >= 0) {
				కొనసాగు;
			}
			స్థితి_2 = 1;
		} అయితే (*++argv);
	} లేకపోతే ఒకవేళ (!(flags & FILEUTILS_FORCE)) {
		bb_show_usage();
	}

	తిరిగి స్థితి_2;
}
