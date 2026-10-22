/* vi: set sw=4 ts=4: */
/*
 * mknod implementation for busybox
 *
 * Copyright (C) 2003  Manuel Novoa III  <mjn3@codepoet.org>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config MKNOD
//config:	bool "mknod (4.6 kb)"
//config:	default y
//config:	help
//config:	mknod is used to create FIFOs or block/character special
//config:	files with the specified names.

//applet:IF_MKNOD(APPLET_NOEXEC(mknod, mknod, BB_DIR_BIN, BB_SUID_DROP, mknod))

//kbuild:lib-$(CONFIG_MKNOD) += mknod.o

/* BB_AUDIT SUSv3 N/A -- Matches GNU behavior. */

//usage:#define mknod_trivial_usage
//usage:       "[-m MODE] " IF_SELINUX("[-Z] ") "NAME TYPE [MAJOR MINOR]"
//usage:#define mknod_full_usage "\n\n"
//usage:       "Create a special file (block, character, or pipe)\n"
//usage:     "\n	-m MODE	Creation mode (default a=rw)"
//usage:	IF_SELINUX(
//usage:     "\n	-Z	Set security context"
//usage:	)
//usage:     "\nTYPE:"
//usage:     "\n	b	Block device"
//usage:     "\n	c or u	Character device"
//usage:     "\n	p	Named pipe (MAJOR MINOR must be omitted)"
//usage:
//usage:#define mknod_example_usage
//usage:       "$ mknod /dev/fd0 b 2 0\n"
//usage:       "$ mknod -m 644 /tmp/pipe p\n"

#ifdef __linux__
# include <sys/sysmacros.h>  // For makedev
#endif

#अन्तर्भाव "libbb.h"
#अन्तर्भाव "libcoreutils/coreutils.h"

/* This is a NOEXEC applet. Be very careful! */

स्थिर स्थिरांक अक्षर modes_chars[] ALIGN1 = { 'p', 'c', 'u', 'b', 0, 1, 1, 2 };
स्थिर स्थिरांक mode_t modes_cubp[] = { S_IFIFO, S_IFCHR, S_IFBLK };

पूर्णांक mknod_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक mknod_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	mode_t mode;
	dev_t dev;
	स्थिरांक अक्षर *type, *arg;

	mode = getopt_mk_fifo_nod(argv);
	argv += optind;
	//argc -= optind;

	जर (!argv[0] || !argv[1])
		bb_show_usage();
	type = म_अक्षर(modes_chars, argv[1][0]);
	जर (!type)
		bb_show_usage();

	mode |= modes_cubp[(पूर्णांक)(type[4])];

	dev = 0;
	arg = argv[2];
	जर (*type != 'p') {
		जर (!argv[2] || !argv[3])
			bb_show_usage();
		/* Autodetect what the system supports; these macros should
		 * optimize out to two constants. */
		dev = makedev(xatoul_range(argv[2], 0, major(अच_पूर्णांक_उच्च)),
				xatoul_range(argv[3], 0, minor(अच_पूर्णांक_उच्च)));
		arg = argv[4];
	}
	जर (arg)
		bb_show_usage();

	जर (mknod(argv[0], mode, dev) != 0) {
		bb_simple_perror_संदेश_and_die(argv[0]);
	}
	परत निकास_सफल;
}
