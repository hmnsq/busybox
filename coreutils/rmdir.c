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

#अन्तर्भाव "libbb.h"

/* This is a NOFORK applet. Be very careful! */


#परिभाषय PARENTS          (1 << 0)
#परिभाषय VERBOSE          ((1 << 1) * ENABLE_FEATURE_VERBOSE)
#परिभाषय IGNORE_NON_रिक्तानि ((1 << 2) * ENABLE_LONG_OPTS)

पूर्णाङ्क rmdir_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क rmdir_main(पूर्णाङ्क argc अप्रयुक्तम्_PARAM, वर्ण **argv)
{
	पूर्णाङ्क स्थितिः = EXIT_SUCCESS;
	पूर्णाङ्क flags;
	वर्ण *मार्गः;

	flags = getopt32long(argv, "pv",
		"parents\0"                  No_तर्कः "p"
		/* Debian etch: many packages fail to be purged or installed
		 * because they desperately want this option: */
		"ignore-fail-on-non-empty\0" No_तर्कः "\xff"
		IF_FEATURE_VERBOSE(
		"verbose\0"                  No_तर्कः "v"
		)
	);
	argv += optind;

	यदि (!*argv) {
		bb_show_usage();
	}

	कुरु {
		मार्गः = *argv;

		यावत् (1) {
			यदि (flags & VERBOSE) {
				प्रारूप_लिख("rmdir: removing directory, '%s'\n", मार्गः);
			}

			यदि (rmdir(मार्गः) < 0) {
#यदि ENABLE_LONG_OPTS
				यदि ((flags & IGNORE_NON_रिक्तानि) && errno == ENOTEMPTY)
					भङ्ग;
#यदि_अन्तः
				bb_perror_सन्देशः("'%s'", मार्गः);  /* Match gnu rmdir msg. */
				स्थितिः = EXIT_FAILURE;
			} अन्यथा यदि (flags & PARENTS) {
				/* Note: path was not "" since rmdir succeeded. */
				मार्गः = dirname(मार्गः);
				/* Path is now just the parent component.  Dirname
				 * returns "." if there are no parents.
				 */
				यदि (NOT_LONE_CHAR(मार्गः, '.')) {
					अनुवर्त;
				}
			}
			भङ्ग;
		}
	} यावत् (*++argv);

	प्रत्यागम स्थितिः;
}
