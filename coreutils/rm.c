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

#ಅನ್ತರ್ಭಾವ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

ಪೂರ್ಣಾಂಕ rm_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ rm_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2 = 0;
	ಪೂರ್ಣಾಂಕ flags = 0;
	ಚಿಹ್ನೆರಹಿತ opt;

	opt = getopt32(argv, "^" "fiRrv" "\0" "f-i:i-f");
	argv += optind;
	ಆದರೆ (opt & 1)
		flags |= FILEUTILS_FORCE;
	ಆದರೆ (opt & 2)
		flags |= FILEUTILS_INTERACTIVE;
	ಆದರೆ (opt & (8|4))
		flags |= FILEUTILS_RECUR;
	ಆದರೆ ((opt & 16) && FILEUTILS_VERBOSE)
		flags |= FILEUTILS_VERBOSE;

	ಆದರೆ (*argv != ಖಾಲಿ) {
		ಮಾಡು {
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ತಳ = bb_get_ಕೊನೆಯ_ಮಾರ್ಗ_component_strip(*argv);

			ಆದರೆ (DOT_OR_DOTDOT(ತಳ)) {
				bb_simple_error_ಸಂದೇಶ("can't remove '.' or '..'");
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (remove_file(*argv, flags) >= 0) {
				ಮುಂದುವರಿ;
			}
			ಸ್ಥಿತಿ_2 = 1;
		} ಆಗ (*++argv);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!(flags & FILEUTILS_FORCE)) {
		bb_show_usage();
	}

	ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
}
