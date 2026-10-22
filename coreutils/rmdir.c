/* vi: set sw=4 ts=4: */
/*
 * rmdir implementation for busybox
 *
 * Copyright (C) 2003  Manuel Novoa III  <mjn3@codepoet.org>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config RMDIR
//config:	bool "rmdir (3.8 kb)"
//config:	default y
//config:	help
//config:	rmdir is used to remove empty directories.

//applet:IF_RMDIR(APPLET_NOFORK(rmdir, rmdir, BB_DIR_BIN, BB_SUID_DROP, rmdir))

//kbuild:lib-$(CONFIG_RMDIR) += rmdir.o

/* BB_AUDIT SUSv3 compliant */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/rmdir.html */

//usage:#define rmdir_trivial_usage
//usage:       "[-p] DIRECTORY..."
//usage:#define rmdir_full_usage "\n\n"
//usage:       "Remove DIRECTORY if it is empty\n"
//usage:     "\n	-p	Include parents"
//usage:	IF_LONG_OPTS(
//usage:     "\n	--ignore-fail-on-non-empty"
//usage:	)
//usage:
//usage:#define rmdir_example_usage
//usage:       "# rmdir /tmp/foo\n"

#ಅನ್ತರ್ಭಾವ "libbb.h"

/* This is a NOFORK applet. Be very careful! */


#ವ್ಯಾಖ್ಯಾನಿಸು PARENTS          (1 << 0)
#ವ್ಯಾಖ್ಯಾನಿಸು VERBOSE          ((1 << 1) * ENABLE_FEATURE_VERBOSE)
#ವ್ಯಾಖ್ಯಾನಿಸು IGNORE_NON_ಖಾಲಿಗಳು ((1 << 2) * ENABLE_LONG_OPTS)

ಪೂರ್ಣಾಂಕ rmdir_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ rmdir_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2 = EXIT_SUCCESS;
	ಪೂರ್ಣಾಂಕ flags;
	ಅಕ್ಷರ *ಮಾರ್ಗ;

	flags = getopt32long(argv, "pv",
		"parents\0"                  No_ವಾದ "p"
		/* Debian etch: many packages fail to be purged or installed
		 * because they desperately want this option: */
		"ignore-fail-on-non-empty\0" No_ವಾದ "\xff"
		IF_FEATURE_VERBOSE(
		"verbose\0"                  No_ವಾದ "v"
		)
	);
	argv += optind;

	ಆದರೆ (!*argv) {
		bb_show_usage();
	}

	ಮಾಡು {
		ಮಾರ್ಗ = *argv;

		ಆಗ (1) {
			ಆದರೆ (flags & VERBOSE) {
				ಬರೆ("rmdir: removing directory, '%s'\n", ಮಾರ್ಗ);
			}

			ಆದರೆ (rmdir(ಮಾರ್ಗ) < 0) {
#ಆದರೆ ENABLE_LONG_OPTS
				ಆದರೆ ((flags & IGNORE_NON_ಖಾಲಿಗಳು) && errno == ENOTEMPTY)
					ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
				bb_perror_ಸಂದೇಶ("'%s'", ಮಾರ್ಗ);  /* Match gnu rmdir msg. */
				ಸ್ಥಿತಿ_2 = EXIT_FAILURE;
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (flags & PARENTS) {
				/* Note: path was not "" since rmdir succeeded. */
				ಮಾರ್ಗ = dirname(ಮಾರ್ಗ);
				/* Path is now just the parent component.  Dirname
				 * returns "." if there are no parents.
				 */
				ಆದರೆ (NOT_LONE_CHAR(ಮಾರ್ಗ, '.')) {
					ಮುಂದುವರಿ;
				}
			}
			ನಿಲ್ಲಿಸು;
		}
	} ಆಗ (*++argv);

	ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
}
