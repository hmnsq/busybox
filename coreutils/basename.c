/* vi: set sw=4 ts=4: */
/*
 * Mini basename implementation for busybox
 *
 * Copyright (C) 1999-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
/* Mar 16, 2003      Manuel Novoa III   (mjn3@codepoet.org)
 *
 * Changes:
 * 1) Now checks for too many args.  Need at least one and at most two.
 * 2) Don't check for options, as per SUSv3.
 * 3) Save some space by using strcmp().  Calling strncmp() here was silly.
 */
//config:config BASENAME
//config:	bool "basename (3.7 kb)"
//config:	default y
//config:	help
//config:	basename is used to strip the directory and suffix from filenames,
//config:	leaving just the filename itself. Enable this option if you wish
//config:	to enable the 'basename' utility.

//applet:IF_BASENAME(APPLET_NOFORK(basename, basename, BB_DIR_USR_BIN, BB_SUID_DROP, basename))

//kbuild:lib-$(CONFIG_BASENAME) += basename.o

/* BB_AUDIT SUSv3 compliant */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/basename.html */

//usage:#define basename_trivial_usage
//usage:       "FILE [SUFFIX] | -a FILE... | -s SUFFIX FILE..."
//usage:#define basename_full_usage "\n\n"
//usage:       "Strip directory path and SUFFIX from FILE\n"
//usage:     "\n	-a		All arguments are FILEs"
//usage:     "\n	-s SUFFIX	Remove SUFFIX (implies -a)"
//usage:
//usage:#define basename_example_usage
//usage:       "$ basename /usr/local/bin/foo\n"
//usage:       "foo\n"
//usage:       "$ basename /usr/local/bin/\n"
//usage:       "bin\n"
//usage:       "$ basename /foo/bar.txt .txt\n"
//usage:       "bar"

#अन्तर्भाव "libbb.h"

/* This is a NOFORK applet. Be very careful! */

पूर्णांक basename_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक basename_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	अचिन्हित opts;
	स्थिरांक अक्षर *suffix = रिक्त;

	/* '+': stop at first non-option */
	opts = getopt32(argv, "^+" "as:"
		"\0" "-1" /* At least one argument */
		, &suffix
	);
	argv += optind;

	करा {
		अक्षर *s;
		माप_प्रकार m;

		/* It should strip slash: /abc/def/ -> def */
		s = bb_get_last_path_component_strip(*argv++);
		m = म_माप(s);
		जर (!opts) {
			जर (*argv) {
				suffix = *argv;
				जर (argv[1])
					bb_show_usage();
			}
		}
		जर (suffix) {
			माप_प्रकार n = म_माप(suffix);
			जर ((m > n) && (म_भेद(s + m - n, suffix) == 0)) {
				m -= n;
				/*s[m] = '\0'; - redundant */
			}
		}
		/* puts(s) will do, but we can do without stdio this way: */
		s[m++] = '\n';
		/* NB: != is correct here: */
		जर (full_write(STDOUT_FILENO, s, m) != (ssize_t)m)
			परत निकास_त्रुटि;
	} पर्यंत (opts && *argv);

	परत निकास_सफल;
}
