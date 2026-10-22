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

#अन्तर्भाव "libbb.h"

/* This is a NOFORK applet. Be very careful! */

#यदि ENABLE_FEATURE_SYNC_FANCY || ENABLE_FSYNC
स्थिर पूर्णाङ्क sync_common(पूर्णाङ्क opts, वर्ण **argv)
{
	पूर्णाङ्क ret;
	गणना {
		OPT_DATASYNC = (1 << 0),
		OPT_SYNCFS   = (1 << 1),
	};

	ret = EXIT_SUCCESS;
	कुरु {
		/* GNU "sync FILE" uses O_NONBLOCK open */
		पूर्णाङ्क fd = open_or_warn(*argv, /*O_NOATIME |*/ O_NOCTTY | O_RDONLY | O_NONBLOCK);
		/* open(NOATIME) can only be used by owner or root, don't use NOATIME here */

		यदि (fd < 0) {
			ret = EXIT_FAILURE;
			गच्छ अग्र;
		}
# यदि ENABLE_FEATURE_SYNC_FANCY
		यदि (opts & OPT_SYNCFS) {
			/*
			 * syncfs is documented to only fail with EBADF,
			 * which can't happen here. So, no error checks.
			 */
			syncfs(fd);
		} अन्यथा
# endif
		यदि (((opts & OPT_DATASYNC) ? fdatasync(fd) : fsync(fd)) != 0) {
			bb_simple_perror_सन्देशः(*argv);
			ret = EXIT_FAILURE;
		}
		close(fd);
 अग्र:
		argv++;
	} यावत् (*argv);

	प्रत्यागम ret;
}
#यदि_अन्तः

#यदि ENABLE_SYNC
पूर्णाङ्क sync_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क sync_main(पूर्णाङ्क argc अप्रयुक्तम्_PARAM, वर्ण **argv IF_NOT_DESKTOP(अप्रयुक्तम्_PARAM))
{
# यदि !ENABLE_FEATURE_SYNC_FANCY
	/* coreutils-6.9 compat */
	bb_warn_ignoring_तर्काः(argv[1]);
	sync();
	प्रत्यागम EXIT_SUCCESS;
# अन्यथा
	अचिह्नित opts = getopt32(argv, "^" "df" "\0" "d--f:f--d");
	argv += optind;
	यदि (!argv[0]) {
		sync();
		प्रत्यागम EXIT_SUCCESS;
	}
	प्रत्यागम sync_common(opts, argv);
# endif
}
#यदि_अन्तः

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

#यदि ENABLE_FSYNC
पूर्णाङ्क fsync_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क fsync_main(पूर्णाङ्क argc अप्रयुक्तम्_PARAM, वर्ण **argv)
{
	पूर्णाङ्क opts = getopt32(argv, "^" "d" "\0" "-1"/*min 1 arg*/);
	argv += optind;
	प्रत्यागम sync_common(opts, argv);
}
#यदि_अन्तः
