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

#అన్తర్భావ "libbb.h"

/* This is a NOFORK applet. Be very careful! */

పూర్ణాంకం mkdir_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం mkdir_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	దీర్ఘ బహుళకం = -1;
	పూర్ణాంకం స్థితి_2 = EXIT_SUCCESS;
	పూర్ణాంకం flags = 0;
	గుర్తులేని opt;
	అక్షరం *smode;
#ఒకవేళ ENABLE_SELINUX
	security_context_t scontext;
#అయితే_ముగింపు

	opt = getopt32long(argv, "m:pv" IF_SELINUX("Z:"),
			"mode\0"    Required_వాదం "m"
			"parents\0" No_వాదం       "p"
# ఒకవేళ ENABLE_SELINUX
			"context\0" Required_వాదం "Z"
# endif
# ఒకవేళ ENABLE_FEATURE_VERBOSE
			"verbose\0" No_వాదం       "v"
# endif
			, &smode IF_SELINUX(,&scontext)
	);
	ఒకవేళ (opt & 1) {
		బహుళకం_t mmode = bb_parse_బహుళకం(smode, 0777);
		ఒకవేళ (mmode == (బహుళకం_t)-1) {
			bb_error_సందేశం_and_die("invalid mode '%s'", smode);
		}
		బహుళకం = mmode;
	}
	ఒకవేళ (opt & 2)
		flags |= FILEUTILS_RECUR;
	ఒకవేళ ((opt & 4) && FILEUTILS_VERBOSE)
		flags |= FILEUTILS_VERBOSE;
#ఒకవేళ ENABLE_SELINUX
	ఒకవేళ (opt & 8) {
		selinux_or_die();
		setfscreatecon_or_die(scontext);
	}
#అయితే_ముగింపు

	argv += optind;
	ఒకవేళ (!argv[0])
		bb_show_usage();

	చేయి {
		ఒకవేళ (bb_make_directory(*argv, బహుళకం, flags)) {
			స్థితి_2 = EXIT_FAILURE;
		}
	} అయితే (*++argv);

	తిరిగి స్థితి_2;
}
