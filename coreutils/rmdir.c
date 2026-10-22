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

#ଅନ୍ତର୍ଭାଵ "libbb.h"

/* This is a NOFORK applet. Be very careful! */


#define PARENTS          (1 << 0)
#define VERBOSE          ((1 << 1) * ENABLE_FEATURE_VERBOSE)
#define IGNORE_NON_EMPTY ((1 << 2) * ENABLE_LONG_OPTS)

ପୂର୍ଣ୍ଣସଂଖ୍ୟା rmdir_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv) MAIN_EXTERNALLY_VISIBLE;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା rmdir_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc UNUSED_PARAM, ଅକ୍ଷର **argv)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା status = EXIT_SUCCESS;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags;
	ଅକ୍ଷର *path;

	flags = getopt32long(argv, "pv",
		"parents\0"                  No_argument "p"
		/* Debian etch: many packages fail to be purged or installed
		 * because they desperately want this option: */
		"ignore-fail-on-non-empty\0" No_argument "\xff"
		IF_FEATURE_VERBOSE(
		"verbose\0"                  No_argument "v"
		)
	);
	argv += optind;

	ଯଦି (!*argv) {
		bb_show_usage();
	}

	କର {
		path = *argv;

		ଯେତେବେଳେ (1) {
			ଯଦି (flags & VERBOSE) {
				printf("rmdir: removing directory, '%s'\n", path);
			}

			ଯଦି (rmdir(path) < 0) {
#ଯଦି ENABLE_LONG_OPTS
				ଯଦି ((flags & IGNORE_NON_EMPTY) && errno == ENOTEMPTY)
					ଭାଙ୍ଗ;
#endif
				bb_perror_ସନ୍ଦେଶ("'%s'", path);  /* Match gnu rmdir msg. */
				status = EXIT_FAILURE;
			} ଅନ୍ୟଥା ଯଦି (flags & PARENTS) {
				/* Note: path was not "" since rmdir succeeded. */
				path = dirname(path);
				/* Path is now just the parent component.  Dirname
				 * returns "." if there are no parents.
				 */
				ଯଦି (NOT_LONE_CHAR(path, '.')) {
					ଜାରି;
				}
			}
			ଭାଙ୍ଗ;
		}
	} ଯେତେବେଳେ (*++argv);

	ଫେରନ୍ତୁ status;
}
