/* vi: set sw=4 ts=4: */
/*
 * Mini mktemp implementation for busybox
 *
 * Copyright (C) 2000 by Daniel Jacobowitz
 * Written by Daniel Jacobowitz <dan@debian.org>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
/* Coreutils 6.12 man page says:
 *        mktemp [OPTION]... [TEMPLATE]
 * Create a temporary file or directory, safely, and print its name. If
 * TEMPLATE is not specified, use tmp.XXXXXXXXXX.
 * -d, --directory
 *        create a directory, not a file
 * -q, --quiet
 *        suppress diagnostics about file/dir-creation failure
 * -u, --dry-run
 *        do not create anything; merely print a name (unsafe)
 * --tmpdir[=DIR]
 *        interpret TEMPLATE relative to DIR. If DIR is not specified,
 *        use  $TMPDIR if set, else /tmp.  With this option, TEMPLATE must
 *        not be an absolute name. Unlike with -t, TEMPLATE may contain
 *        slashes, but even here, mktemp still creates only the final com-
 *        ponent.
 * -p DIR use DIR as a prefix; implies -t [deprecated]
 * -t     interpret TEMPLATE as a single file name component, relative  to
 *        a  directory:  $TMPDIR, if set; else the directory specified via
 *        -p; else /tmp [deprecated]
 */
//config:config MKTEMP
//config:	bool "mktemp (4.5 kb)"
//config:	default y
//config:	help
//config:	mktemp is used to create unique temporary files

//applet:IF_MKTEMP(APPLET_NOEXEC(mktemp, mktemp, BB_DIR_BIN, BB_SUID_DROP, mktemp))

//kbuild:lib-$(CONFIG_MKTEMP) += mktemp.o

//usage:#define mktemp_trivial_usage
//usage:       "[-dt] [-p DIR] [TEMPLATE]"
//usage:#define mktemp_full_usage "\n\n"
//usage:       "Create a temporary file with name based on TEMPLATE and print its name.\n"
//usage:       "TEMPLATE must end with XXXXXX (e.g. [/dir/]nameXXXXXX).\n"
//usage:       "Without TEMPLATE, -t tmp.XXXXXX is assumed.\n"
//usage:     "\n	-d	Make directory, not file"
//usage:     "\n	-q	Fail silently on errors"
//usage:     "\n	-t	Prepend base directory name to TEMPLATE"
//usage:     "\n	-p DIR	Use DIR as a base directory (implies -t)"
//usage:     "\n	-u	Do not create anything; print a name"
//usage:     "\n"
//usage:     "\nBase directory is: -p DIR, else $TMPDIR, else /tmp"
//usage:
//usage:#define mktemp_example_usage
//usage:       "$ mktemp /tmp/temp.XXXXXX\n"
//usage:       "/tmp/temp.mWiLjM\n"
//usage:       "$ ls -la /tmp/temp.mWiLjM\n"
//usage:       "-rw-------    1 andersen andersen        0 Apr 25 17:10 /tmp/temp.mWiLjM\n"

#அந்தர்பாவ "libbb.h"

முழு mktemp_main(முழு argc, எழுத்து **argv) MAIN_EXTERNALLY_VISIBLE;
முழு mktemp_main(முழு argc UNUSED_PARAM, எழுத்து **argv)
{
	மாறிலி எழுத்து *path;
	எழுத்து *chp;
	unsigned opts;
	எண்ணிக்கை {
		OPT_d = 1 << 0,
		OPT_q = 1 << 1,
		OPT_t = 1 << 2,
		OPT_p = 1 << 3,
		OPT_u = 1 << 4,
		OPT_tmpdir = (1 << 5) * ENABLE_LONG_OPTS,
	};

	path = getenv("TMPDIR");
	எனில் (!path || path[0] == '\0')
		path = "/tmp";

#எனில் ENABLE_LONG_OPTS
	opts = getopt32long(argv, "^"
		"dqtp:u"
		"\0"
		"?1" /* 1 arg max */,
		"directory\0" No_argument       "d"
		"quiet\0"     No_argument       "q"
		"dry-run\0"   No_argument       "u"
		"tmpdir\0"    Optional_argument "\xff"
		, &path, &path
	);
#இல்லையெனில்
	opts = getopt32(argv, "^" "dqtp:u" "\0" "?1"/*1 arg max*/, &path);
#endif

	chp = argv[optind];
	எனில் (!chp) {
		/* GNU coreutils 8.4:
		 * bare "mktemp" -> "mktemp -t tmp.XXXXXX"
		 */
		chp = xstrdup("tmp.XXXXXX");
		opts |= OPT_t;
	}
#எனில் 0
	/* Don't allow directory separator in template */
	எனில் ((opts & OPT_t) && bb_basename(chp) != chp) {
		errno = EINVAL;
		செல் error;
	}
#endif
	எனில் (opts & (OPT_t|OPT_p|OPT_tmpdir))
		chp = concat_path_file(path, chp);

	எனில் (opts & OPT_u) {
		chp = mktemp(chp);
		எனில் (chp[0] == '\0')
			செல் error;
	} இல்லையெனில் எனில் (opts & OPT_d) {
		எனில் (mkdtemp(chp) == வெற்று)
			செல் error;
	} இல்லையெனில் {
		எனில் (mkstemp(chp) < 0)
			செல் error;
	}
	puts(chp);
	திரும்பு EXIT_SUCCESS;
 error:
	எனில் (opts & OPT_q)
		திரும்பு EXIT_FAILURE;
	/* don't use chp as it gets mangled in case of error */
	bb_perror_nomsg_and_die();
}
