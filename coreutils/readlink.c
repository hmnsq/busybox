/* vi: set sw=4 ts=4: */
/*
 * Mini readlink implementation for busybox
 *
 * Copyright (C) 2000,2001 Matt Kraai <kraai@alumni.carnegiemellon.edu>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config READLINK
//config:	bool "readlink (4.8 kb)"
//config:	default y
//config:	help
//config:	This program reads a symbolic link and returns the name
//config:	of the file it points to
//config:
//config:config FEATURE_READLINK_FOLLOW
//config:	bool "Enable canonicalization by following all symlinks (-f)"
//config:	default y
//config:	depends on READLINK
//config:	help
//config:	Enable the readlink option (-f).

//applet:IF_READLINK(APPLET_NOFORK(readlink, readlink, BB_DIR_USR_BIN, BB_SUID_DROP, readlink))

//kbuild:lib-$(CONFIG_READLINK) += readlink.o

//usage:#define readlink_trivial_usage
//usage:	IF_FEATURE_READLINK_FOLLOW("[-fnv] ")
//usage:	IF_NOT_FEATURE_READLINK_FOLLOW("[-n] ")
//usage:	"FILE"
//usage:#define readlink_full_usage "\n\n"
//usage:       "Display the value of a symlink" "\n"
//usage:     "\n	-n	Don't add newline"
//usage:	IF_FEATURE_READLINK_FOLLOW(
//usage:     "\n	-f	Canonicalize by following all symlinks"
//usage:     "\n	-v	Verbose"
//usage:	)

#అన్తర్భావ "libbb.h"

/*
 * # readlink --version
 * readlink (GNU coreutils) 6.10
 * # readlink --help
 *   -f, --canonicalize
 *      canonicalize by following every symlink in
 *      every component of the given name recursively;
 *      all but the last component must exist
 *   -e, --canonicalize-existing
 *      canonicalize by following every symlink in
 *      every component of the given name recursively,
 *      all components must exist
 *   -m, --canonicalize-missing
 *      canonicalize by following every symlink in
 *      every component of the given name recursively,
 *      without requirements on components existence
 *   -n, --no-newline              do not output the trailing newline
 *   -q, --quiet, -s, --silent     suppress most error messages
 *   -v, --verbose                 report error messages
 *
 * bbox supports: -f (partially) -n -v (fully), -q -s (accepts but ignores)
 * Note: we export the -f flag, but our -f behaves like coreutils' -e.
 * Unfortunately, there isn't a C lib function we can leverage to get this
 * behavior which means we'd have to implement the full stack ourselves :(.
 */

పూర్ణాంకం readlink_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం readlink_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	అక్షరం *buf;
	గుర్తులేని opt;

	/* -n must use bit 0 (see printf below) */
	opt = getopt32(argv, "^" "n" IF_FEATURE_READLINK_FOLLOW("fvsq")
                       "\0" "=1");

	/* compat: coreutils readlink reports errors silently via exit code */
	ఒకవేళ (!(opt & 4)) /* not -v */
		logmode = LOGMODE_NONE;

	/* NOFORK: only one alloc is allowed; must free */
	ఒకవేళ (opt & 2) { /* -f */
		buf = xmalloc_realpath_coreutils(argv[optind]);
	} లేకపోతే {
		buf = xmalloc_readlink_or_warn(argv[optind]);
	}

	ఒకవేళ (!buf)
		తిరిగి EXIT_FAILURE;
	రాయి("%s%s", buf, &"\n"[opt & 1]);
	విడుదల(buf);

	fflush_stdout_and_exit_SUCCESS();
}
