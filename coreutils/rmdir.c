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

#అన్తర్భావ "libbb.h"

/* This is a NOFORK applet. Be very careful! */


#నిర్వచించు PARENTS          (1 << 0)
#నిర్వచించు VERBOSE          ((1 << 1) * ENABLE_FEATURE_VERBOSE)
#నిర్వచించు IGNORE_NON_ఖాళీలు ((1 << 2) * ENABLE_LONG_OPTS)

పూర్ణాంకం rmdir_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం rmdir_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	పూర్ణాంకం స్థితి_2 = EXIT_SUCCESS;
	పూర్ణాంకం flags;
	అక్షరం *మార్గం;

	flags = getopt32long(argv, "pv",
		"parents\0"                  No_వాదం "p"
		/* Debian etch: many packages fail to be purged or installed
		 * because they desperately want this option: */
		"ignore-fail-on-non-empty\0" No_వాదం "\xff"
		IF_FEATURE_VERBOSE(
		"verbose\0"                  No_వాదం "v"
		)
	);
	argv += optind;

	ఒకవేళ (!*argv) {
		bb_show_usage();
	}

	చేయి {
		మార్గం = *argv;

		అయితే (1) {
			ఒకవేళ (flags & VERBOSE) {
				రాయి("rmdir: removing directory, '%s'\n", మార్గం);
			}

			ఒకవేళ (rmdir(మార్గం) < 0) {
#ఒకవేళ ENABLE_LONG_OPTS
				ఒకవేళ ((flags & IGNORE_NON_ఖాళీలు) && errno == ENOTEMPTY)
					ఆపు;
#అయితే_ముగింపు
				bb_perror_సందేశం("'%s'", మార్గం);  /* Match gnu rmdir msg. */
				స్థితి_2 = EXIT_FAILURE;
			} లేకపోతే ఒకవేళ (flags & PARENTS) {
				/* Note: path was not "" since rmdir succeeded. */
				మార్గం = dirname(మార్గం);
				/* Path is now just the parent component.  Dirname
				 * returns "." if there are no parents.
				 */
				ఒకవేళ (NOT_LONE_CHAR(మార్గం, '.')) {
					కొనసాగు;
				}
			}
			ఆపు;
		}
	} అయితే (*++argv);

	తిరిగి స్థితి_2;
}
