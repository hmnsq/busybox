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

#ଅନ୍ତର୍ଭାଵ "libbb.h"

/* This is a NOFORK applet. Be very careful! */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା mkdir_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv) MAIN_EXTERNALLY_VISIBLE;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା mkdir_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc UNUSED_PARAM, ଅକ୍ଷର **argv)
{
	long mode = -1;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା status = EXIT_SUCCESS;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = 0;
	unsigned opt;
	ଅକ୍ଷର *smode;
#ଯଦି ENABLE_SELINUX
	security_context_t scontext;
#endif

	opt = getopt32long(argv, "m:pv" IF_SELINUX("Z:"),
			"mode\0"    Required_argument "m"
			"parents\0" No_argument       "p"
# ଯଦି ENABLE_SELINUX
			"context\0" Required_argument "Z"
# endif
# ଯଦି ENABLE_FEATURE_VERBOSE
			"verbose\0" No_argument       "v"
# endif
			, &smode IF_SELINUX(,&scontext)
	);
	ଯଦି (opt & 1) {
		mode_t mmode = bb_parse_mode(smode, 0777);
		ଯଦି (mmode == (mode_t)-1) {
			bb_error_ସନ୍ଦେଶ_and_die("invalid mode '%s'", smode);
		}
		mode = mmode;
	}
	ଯଦି (opt & 2)
		flags |= FILEUTILS_RECUR;
	ଯଦି ((opt & 4) && FILEUTILS_VERBOSE)
		flags |= FILEUTILS_VERBOSE;
#ଯଦି ENABLE_SELINUX
	ଯଦି (opt & 8) {
		selinux_or_die();
		setfscreatecon_or_die(scontext);
	}
#endif

	argv += optind;
	ଯଦି (!argv[0])
		bb_show_usage();

	କର {
		ଯଦି (bb_make_directory(*argv, mode, flags)) {
			status = EXIT_FAILURE;
		}
	} ଯେତେବେଳେ (*++argv);

	ଫେରନ୍ତୁ status;
}
