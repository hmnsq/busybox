/* vi: set sw=4 ts=4: */
/*
 * Mini sync implementation for busybox
 *
 * Copyright (C) 1995, 1996 by Bruce Perens <bruce@pixar.com>.
 * Copyright (C) 2015 by Ari Sundholm <ari@tuxera.com>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config SYNC
//config:	bool "sync (4 kb)"
//config:	default y
//config:	help
//config:	sync is used to flush filesystem buffers.
//config:config FEATURE_SYNC_FANCY
//config:	bool "Enable -d and -f flags (requires syncfs(2) in libc)"
//config:	default y
//config:	depends on SYNC
//config:	help
//config:	sync -d FILE... executes fdatasync() on each FILE.
//config:	sync -f FILE... executes syncfs() on each FILE.

//               APPLET_NOFORK:name  main  location    suid_type     help
//applet:IF_SYNC(APPLET_NOFORK(sync, sync, BB_DIR_BIN, BB_SUID_DROP, sync))

//kbuild:lib-$(CONFIG_SYNC) += sync.o

/* BB_AUDIT SUSv3 N/A -- Matches GNU behavior. */

//usage:#define sync_trivial_usage
//usage:       ""IF_FEATURE_SYNC_FANCY("[-df] [FILE]...")
//usage:#define sync_full_usage "\n\n"
//usage:    IF_NOT_FEATURE_SYNC_FANCY(
//usage:       "Write all buffered blocks to disk"
//usage:    )
//usage:    IF_FEATURE_SYNC_FANCY(
//usage:       "Write all buffered blocks (in FILEs) to disk"
//usage:     "\n	-d	Avoid syncing metadata"
//usage:     "\n	-f	Sync filesystems underlying FILEs"
//usage:    )

#অন্তর্ভাব "libbb.h"

/* This is a NOFORK applet. Be very careful! */

#যদি ENABLE_FEATURE_SYNC_FANCY || ENABLE_FSYNC
স্থির পূর্ণাঙ্ক sync_common(পূর্ণাঙ্ক opts, বর্ণ **argv)
{
	পূর্ণাঙ্ক ret;
	গণনা {
		OPT_DATASYNC = (1 << 0),
		OPT_SYNCFS   = (1 << 1),
	};

	ret = EXIT_SUCCESS;
	কুরু {
		/* GNU "sync FILE" uses O_NONBLOCK open */
		পূর্ণাঙ্ক fd = open_or_warn(*argv, /*O_NOATIME |*/ O_NOCTTY | O_RDONLY | O_NONBLOCK);
		/* open(NOATIME) can only be used by owner or root, don't use NOATIME here */

		যদি (fd < 0) {
			ret = EXIT_FAILURE;
			গচ্ছ next;
		}
# যদি ENABLE_FEATURE_SYNC_FANCY
		যদি (opts & OPT_SYNCFS) {
			/*
			 * syncfs is documented to only fail with EBADF,
			 * which can't happen here. So, no error checks.
			 */
			syncfs(fd);
		} অন্যথা
# endif
		যদি (((opts & OPT_DATASYNC) ? fdatasync(fd) : fsync(fd)) != 0) {
			bb_simple_perror_বার্তা(*argv);
			ret = EXIT_FAILURE;
		}
		close(fd);
 next:
		argv++;
	} যাবত্ (*argv);

	প্রত্যাগম ret;
}
#endif

#যদি ENABLE_SYNC
পূর্ণাঙ্ক sync_main(পূর্ণাঙ্ক argc, বর্ণ **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণাঙ্ক sync_main(পূর্ণাঙ্ক argc UNUSED_PARAM, বর্ণ **argv IF_NOT_DESKTOP(UNUSED_PARAM))
{
# যদি !ENABLE_FEATURE_SYNC_FANCY
	/* coreutils-6.9 compat */
	bb_warn_ignoring_args(argv[1]);
	sync();
	প্রত্যাগম EXIT_SUCCESS;
# অন্যথা
	unsigned opts = getopt32(argv, "^" "df" "\0" "d--f:f--d");
	argv += optind;
	যদি (!argv[0]) {
		sync();
		প্রত্যাগম EXIT_SUCCESS;
	}
	প্রত্যাগম sync_common(opts, argv);
# endif
}
#endif

/*
 * Mini fsync implementation for busybox
 *
 * Copyright (C) 2008 Nokia Corporation. All rights reserved.
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config FSYNC
//config:	bool "fsync (3.8 kb)"
//config:	default y
//config:	help
//config:	fsync is used to flush file-related cached blocks to disk.

//                APPLET_NOFORK:name   main   location    suid_type     help
//applet:IF_FSYNC(APPLET_NOFORK(fsync, fsync, BB_DIR_BIN, BB_SUID_DROP, fsync))

//kbuild:lib-$(CONFIG_FSYNC) += sync.o

//usage:#define fsync_trivial_usage
//usage:       "[-d] FILE..."
//usage:#define fsync_full_usage "\n\n"
//usage:       "Write all buffered blocks in FILEs to disk\n"
//usage:     "\n	-d	Avoid syncing metadata"

#যদি ENABLE_FSYNC
পূর্ণাঙ্ক fsync_main(পূর্ণাঙ্ক argc, বর্ণ **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণাঙ্ক fsync_main(পূর্ণাঙ্ক argc UNUSED_PARAM, বর্ণ **argv)
{
	পূর্ণাঙ্ক opts = getopt32(argv, "^" "d" "\0" "-1"/*min 1 arg*/);
	argv += optind;
	প্রত্যাগম sync_common(opts, argv);
}
#endif
