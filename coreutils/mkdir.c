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

#अन्तर्भाव "libbb.h"

/* This is a NOFORK applet. Be very careful! */

पूर्णाङ्क mkdir_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क mkdir_main(पूर्णाङ्क argc अप्रयुक्तम्_PARAM, वर्ण **argv)
{
	दीर्घ बहुलकम् = -1;
	पूर्णाङ्क स्थितिः = EXIT_SUCCESS;
	पूर्णाङ्क flags = 0;
	अचिह्नित opt;
	वर्ण *smode;
#यदि ENABLE_SELINUX
	security_context_t scontext;
#यदि_अन्तः

	opt = getopt32long(argv, "m:pv" IF_SELINUX("Z:"),
			"mode\0"    Required_तर्कः "m"
			"parents\0" No_तर्कः       "p"
# यदि ENABLE_SELINUX
			"context\0" Required_तर्कः "Z"
# endif
# यदि ENABLE_FEATURE_VERBOSE
			"verbose\0" No_तर्कः       "v"
# endif
			, &smode IF_SELINUX(,&scontext)
	);
	यदि (opt & 1) {
		बहुलकम्_t mmode = bb_parse_बहुलकम्(smode, 0777);
		यदि (mmode == (बहुलकम्_t)-1) {
			bb_error_सन्देशः_and_die("invalid mode '%s'", smode);
		}
		बहुलकम् = mmode;
	}
	यदि (opt & 2)
		flags |= FILEUTILS_RECUR;
	यदि ((opt & 4) && FILEUTILS_VERBOSE)
		flags |= FILEUTILS_VERBOSE;
#यदि ENABLE_SELINUX
	यदि (opt & 8) {
		selinux_or_die();
		setfscreatecon_or_die(scontext);
	}
#यदि_अन्तः

	argv += optind;
	यदि (!argv[0])
		bb_show_usage();

	कुरु {
		यदि (bb_make_directory(*argv, बहुलकम्, flags)) {
			स्थितिः = EXIT_FAILURE;
		}
	} यावत् (*++argv);

	प्रत्यागम स्थितिः;
}
