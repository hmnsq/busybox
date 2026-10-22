/* vi: set sw=4 ts=4: */
/*
 * Mini mkdir implementation for busybox
 *
 * Copyright (C) 2001 Matt Kraai <kraai@alumni.carnegiemellon.edu>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
/* Mar 16, 2003      Manuel Novoa III   (mjn3@codepoet.org)
 *
 * Fixed broken permission setting when -p was used; especially in
 * conjunction with -m.
 */
/* Nov 28, 2006      Yoshinori Sato <ysato@users.sourceforge.jp>: Add SELinux Support.
 */
//config:config MKDIR
//config:	bool "mkdir (4.7 kb)"
//config:	default y
//config:	help
//config:	mkdir is used to create directories with the specified names.

//applet:IF_MKDIR(APPLET_NOFORK(mkdir, mkdir, BB_DIR_BIN, BB_SUID_DROP, mkdir))

//kbuild:lib-$(CONFIG_MKDIR) += mkdir.o

/* BB_AUDIT SUSv3 compliant */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/mkdir.html */

//usage:#define mkdir_trivial_usage
//usage:       "[-m MODE] [-p] DIRECTORY..."
//usage:#define mkdir_full_usage "\n\n"
//usage:       "Create DIRECTORY\n"
//usage:     "\n	-m MODE	Mode"
//usage:     "\n	-p	No error if exists; make parent directories as needed"
//usage:	IF_SELINUX(
//usage:     "\n	-Z	Set security context"
//usage:	)
//usage:
//usage:#define mkdir_example_usage
//usage:       "$ mkdir /tmp/foo\n"
//usage:       "$ mkdir /tmp/foo\n"
//usage:       "/tmp/foo: File exists\n"
//usage:       "$ mkdir /tmp/foo/bar/baz\n"
//usage:       "/tmp/foo/bar/baz: No such file or directory\n"
//usage:       "$ mkdir -p /tmp/foo/bar/baz\n"

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"

/* This is a NOFORK applet. Be very careful! */

ਪੂਰਨ mkdir_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ mkdir_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
	ਲੰਮਾ ਬਹੁਲਕ = -1;
	ਪੂਰਨ ਸਥਿਤੀ = EXIT_SUCCESS;
	ਪੂਰਨ flags = 0;
	ਬਿਨਾਨਿਸ਼ਾਨੀ opt;
	ਅੱਖਰ *smode;
#ਜੇ ENABLE_SELINUX
	security_context_t scontext;
#ਜੇ_ਅੰਤ

	opt = getopt32long(argv, "m:pv" IF_SELINUX("Z:"),
			"mode\0"    Required_ਦਲੀਲ "m"
			"parents\0" No_ਦਲੀਲ       "p"
# ਜੇ ENABLE_SELINUX
			"context\0" Required_ਦਲੀਲ "Z"
# endif
# ਜੇ ENABLE_FEATURE_VERBOSE
			"verbose\0" No_ਦਲੀਲ       "v"
# endif
			, &smode IF_SELINUX(,&scontext)
	);
	ਜੇ (opt & 1) {
		ਬਹੁਲਕ_t mmode = bb_parse_ਬਹੁਲਕ(smode, 0777);
		ਜੇ (mmode == (ਬਹੁਲਕ_t)-1) {
			bb_error_ਸੁਨੇਹਾ_and_die("invalid mode '%s'", smode);
		}
		ਬਹੁਲਕ = mmode;
	}
	ਜੇ (opt & 2)
		flags |= FILEUTILS_RECUR;
	ਜੇ ((opt & 4) && FILEUTILS_VERBOSE)
		flags |= FILEUTILS_VERBOSE;
#ਜੇ ENABLE_SELINUX
	ਜੇ (opt & 8) {
		selinux_or_die();
		setfscreatecon_or_die(scontext);
	}
#ਜੇ_ਅੰਤ

	argv += optind;
	ਜੇ (!argv[0])
		bb_show_usage();

	ਕਰੋ {
		ਜੇ (bb_make_directory(*argv, ਬਹੁਲਕ, flags)) {
			ਸਥਿਤੀ = EXIT_FAILURE;
		}
	} ਜਦੋਂ (*++argv);

	ਵਾਪਸ ਸਥਿਤੀ;
}
