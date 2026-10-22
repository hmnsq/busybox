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

#ଅନ୍ତର୍ଭାଵ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା rm_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv) MAIN_EXTERNALLY_VISIBLE;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା rm_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc UNUSED_PARAM, ଅକ୍ଷର **argv)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା status = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = 0;
	unsigned opt;

	opt = getopt32(argv, "^" "fiRrv" "\0" "f-i:i-f");
	argv += optind;
	ଯଦି (opt & 1)
		flags |= FILEUTILS_FORCE;
	ଯଦି (opt & 2)
		flags |= FILEUTILS_INTERACTIVE;
	ଯଦି (opt & (8|4))
		flags |= FILEUTILS_RECUR;
	ଯଦି ((opt & 16) && FILEUTILS_VERBOSE)
		flags |= FILEUTILS_VERBOSE;

	ଯଦି (*argv != ଖାଲି) {
		କର {
			ଅଚଳ ଅକ୍ଷର *base = bb_get_last_path_component_strip(*argv);

			ଯଦି (DOT_OR_DOTDOT(base)) {
				bb_simple_error_ସନ୍ଦେଶ("can't remove '.' or '..'");
			} ଅନ୍ୟଥା ଯଦି (remove_file(*argv, flags) >= 0) {
				ଜାରି;
			}
			status = 1;
		} ଯେତେବେଳେ (*++argv);
	} ଅନ୍ୟଥା ଯଦି (!(flags & FILEUTILS_FORCE)) {
		bb_show_usage();
	}

	ଫେରନ୍ତୁ status;
}
