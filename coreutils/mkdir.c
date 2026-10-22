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

#ಅನ್ತರ್ಭಾವ "libbb.h"

/* This is a NOFORK applet. Be very careful! */

ಪೂರ್ಣಾಂಕ mkdir_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ mkdir_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	ದೀರ್ಘ ಬಹುಲಕ = -1;
	ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2 = EXIT_SUCCESS;
	ಪೂರ್ಣಾಂಕ flags = 0;
	ಚಿಹ್ನೆರಹಿತ opt;
	ಅಕ್ಷರ *smode;
#ಆದರೆ ENABLE_SELINUX
	security_context_t scontext;
#ಆದರೆ_ಅಂತ್ಯ

	opt = getopt32long(argv, "m:pv" IF_SELINUX("Z:"),
			"mode\0"    Required_ವಾದ "m"
			"parents\0" No_ವಾದ       "p"
# ಆದರೆ ENABLE_SELINUX
			"context\0" Required_ವಾದ "Z"
# endif
# ಆದರೆ ENABLE_FEATURE_VERBOSE
			"verbose\0" No_ವಾದ       "v"
# endif
			, &smode IF_SELINUX(,&scontext)
	);
	ಆದರೆ (opt & 1) {
		ಬಹುಲಕ_t mmode = bb_parse_ಬಹುಲಕ(smode, 0777);
		ಆದರೆ (mmode == (ಬಹುಲಕ_t)-1) {
			bb_error_ಸಂದೇಶ_and_die("invalid mode '%s'", smode);
		}
		ಬಹುಲಕ = mmode;
	}
	ಆದರೆ (opt & 2)
		flags |= FILEUTILS_RECUR;
	ಆದರೆ ((opt & 4) && FILEUTILS_VERBOSE)
		flags |= FILEUTILS_VERBOSE;
#ಆದರೆ ENABLE_SELINUX
	ಆದರೆ (opt & 8) {
		selinux_or_die();
		setfscreatecon_or_die(scontext);
	}
#ಆದರೆ_ಅಂತ್ಯ

	argv += optind;
	ಆದರೆ (!argv[0])
		bb_show_usage();

	ಮಾಡು {
		ಆದರೆ (bb_make_directory(*argv, ಬಹುಲಕ, flags)) {
			ಸ್ಥಿತಿ_2 = EXIT_FAILURE;
		}
	} ಆಗ (*++argv);

	ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
}
