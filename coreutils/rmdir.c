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

#समावेश "libbb.h"

/* This is a NOFORK applet. Be very careful! */


#घोषणा PARENTS          (1 << 0)
#घोषणा VERBOSE          ((1 << 1) * ENABLE_FEATURE_VERBOSE)
#घोषणा IGNORE_NON_EMPTY ((1 << 2) * ENABLE_LONG_OPTS)

पूर्णांक rmdir_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक rmdir_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	पूर्णांक status = निकास_सफल;
	पूर्णांक flags;
	अक्षर *path;

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

	अगर (!*argv) {
		bb_show_usage();
	}

	करो {
		path = *argv;

		जबतक (1) {
			अगर (flags & VERBOSE) {
				म_लिखो("rmdir: removing directory, '%s'\n", path);
			}

			अगर (सूची_हटाओ(path) < 0) {
#अगर ENABLE_LONG_OPTS
				अगर ((flags & IGNORE_NON_EMPTY) && त्रुटि_सं == ENOTEMPTY)
					अवरोध;
#पूर्ण_अगर
				bb_perror_संदेश("'%s'", path);  /* Match gnu rmdir msg. */
				status = निकास_त्रुटि;
			} अन्यथा अगर (flags & PARENTS) {
				/* Note: path was not "" since rmdir succeeded. */
				path = स_नाम(path);
				/* Path is now just the parent component.  Dirname
				 * returns "." if there are no parents.
				 */
				अगर (NOT_LONE_CHAR(path, '.')) {
					जारी;
				}
			}
			अवरोध;
		}
	} जबतक (*++argv);

	वापस status;
}
