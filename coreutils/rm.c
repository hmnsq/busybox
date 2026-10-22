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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

ਪੂਰਨ rm_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ rm_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
	ਪੂਰਨ ਸਥਿਤੀ = 0;
	ਪੂਰਨ flags = 0;
	ਬਿਨਾਨਿਸ਼ਾਨੀ opt;

	opt = getopt32(argv, "^" "fiRrv" "\0" "f-i:i-f");
	argv += optind;
	ਜੇ (opt & 1)
		flags |= FILEUTILS_FORCE;
	ਜੇ (opt & 2)
		flags |= FILEUTILS_INTERACTIVE;
	ਜੇ (opt & (8|4))
		flags |= FILEUTILS_RECUR;
	ਜੇ ((opt & 16) && FILEUTILS_VERBOSE)
		flags |= FILEUTILS_VERBOSE;

	ਜੇ (*argv != ਨਿਰਾ) {
		ਕਰੋ {
			ਅਚਲ ਅੱਖਰ *ਅਧਾਰ = bb_get_ਆਖਰੀ_ਰਸਤਾ_component_strip(*argv);

			ਜੇ (DOT_OR_DOTDOT(ਅਧਾਰ)) {
				bb_simple_error_ਸੁਨੇਹਾ("can't remove '.' or '..'");
			} ਨਹੀਂਤਾਂ ਜੇ (remove_file(*argv, flags) >= 0) {
				ਜਾਰੀ;
			}
			ਸਥਿਤੀ = 1;
		} ਜਦੋਂ (*++argv);
	} ਨਹੀਂਤਾਂ ਜੇ (!(flags & FILEUTILS_FORCE)) {
		bb_show_usage();
	}

	ਵਾਪਸ ਸਥਿਤੀ;
}
