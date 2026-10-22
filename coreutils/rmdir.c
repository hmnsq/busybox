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

#ضمن "libbb.h"

/* This is a NOFORK applet. Be very careful! */


#تعریف_کن PARENTS          (1 << 0)
#تعریف_کن VERBOSE          ((1 << 1) * ENABLE_FEATURE_VERBOSE)
#تعریف_کن IGNORE_NON_خالی ((1 << 2) * ENABLE_LONG_OPTS)

صحيح rmdir_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح rmdir_main(صحيح argc استفادهنشده_PARAM, حرف **argv)
{
	صحيح وضعیت = EXIT_SUCCESS;
	صحيح flags;
	حرف *مسیر;

	flags = getopt32long(argv, "pv",
		"parents\0"                  No_آرگومان "p"
		/* Debian etch: many packages fail to be purged or installed
		 * because they desperately want this option: */
		"ignore-fail-on-non-empty\0" No_آرگومان "\xff"
		IF_FEATURE_VERBOSE(
		"verbose\0"                  No_آرگومان "v"
		)
	);
	argv += optind;

	إذا (!*argv) {
		bb_show_usage();
	}

	افعل {
		مسیر = *argv;

		طالما (1) {
			إذا (flags & VERBOSE) {
				چاپ_کن("rmdir: removing directory, '%s'\n", مسیر);
			}

			إذا (rmdir(مسیر) < 0) {
#إذا ENABLE_LONG_OPTS
				إذا ((flags & IGNORE_NON_خالی) && errno == ENOTEMPTY)
					اكسر;
#پایان_اگر
				bb_perror_پیام("'%s'", مسیر);  /* Match gnu rmdir msg. */
				وضعیت = EXIT_FAILURE;
			} وإلا إذا (flags & PARENTS) {
				/* Note: path was not "" since rmdir succeeded. */
				مسیر = dirname(مسیر);
				/* Path is now just the parent component.  Dirname
				 * returns "." if there are no parents.
				 */
				إذا (NOT_LONE_CHAR(مسیر, '.')) {
					استمر;
				}
			}
			اكسر;
		}
	} طالما (*++argv);

	أرجع وضعیت;
}
