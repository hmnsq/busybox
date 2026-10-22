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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"

/* This is a NOFORK applet. Be very careful! */


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PARENTS          (1 << 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VERBOSE          ((1 << 1) * ENABLE_FEATURE_VERBOSE)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IGNORE_NON_ਖਾਲੀ_ਥਾਵਾਂ ((1 << 2) * ENABLE_LONG_OPTS)

ਪੂਰਨ rmdir_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ rmdir_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
	ਪੂਰਨ ਸਥਿਤੀ = EXIT_SUCCESS;
	ਪੂਰਨ flags;
	ਅੱਖਰ *ਰਸਤਾ;

	flags = getopt32long(argv, "pv",
		"parents\0"                  No_ਦਲੀਲ "p"
		/* Debian etch: many packages fail to be purged or installed
		 * because they desperately want this option: */
		"ignore-fail-on-non-empty\0" No_ਦਲੀਲ "\xff"
		IF_FEATURE_VERBOSE(
		"verbose\0"                  No_ਦਲੀਲ "v"
		)
	);
	argv += optind;

	ਜੇ (!*argv) {
		bb_show_usage();
	}

	ਕਰੋ {
		ਰਸਤਾ = *argv;

		ਜਦੋਂ (1) {
			ਜੇ (flags & VERBOSE) {
				ਲਿਖੋ("rmdir: removing directory, '%s'\n", ਰਸਤਾ);
			}

			ਜੇ (rmdir(ਰਸਤਾ) < 0) {
#ਜੇ ENABLE_LONG_OPTS
				ਜੇ ((flags & IGNORE_NON_ਖਾਲੀ_ਥਾਵਾਂ) && errno == ENOTEMPTY)
					ਰੋਕੋ;
#ਜੇ_ਅੰਤ
				bb_perror_ਸੁਨੇਹਾ("'%s'", ਰਸਤਾ);  /* Match gnu rmdir msg. */
				ਸਥਿਤੀ = EXIT_FAILURE;
			} ਨਹੀਂਤਾਂ ਜੇ (flags & PARENTS) {
				/* Note: path was not "" since rmdir succeeded. */
				ਰਸਤਾ = dirname(ਰਸਤਾ);
				/* Path is now just the parent component.  Dirname
				 * returns "." if there are no parents.
				 */
				ਜੇ (NOT_LONE_CHAR(ਰਸਤਾ, '.')) {
					ਜਾਰੀ;
				}
			}
			ਰੋਕੋ;
		}
	} ਜਦੋਂ (*++argv);

	ਵਾਪਸ ਸਥਿਤੀ;
}
