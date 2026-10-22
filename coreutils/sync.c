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

#ಅನ್ತರ್ಭಾವ "libbb.h"

/* This is a NOFORK applet. Be very careful! */

#ಆದರೆ ENABLE_FEATURE_SYNC_FANCY || ENABLE_FSYNC
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sync_common(ಪೂರ್ಣಾಂಕ opts, ಅಕ್ಷರ **argv)
{
	ಪೂರ್ಣಾಂಕ ret;
	ಎಣಿಕೆ {
		OPT_DATASYNC = (1 << 0),
		OPT_SYNCFS   = (1 << 1),
	};

	ret = EXIT_SUCCESS;
	ಮಾಡು {
		/* GNU "sync FILE" uses O_NONBLOCK open */
		ಪೂರ್ಣಾಂಕ fd = open_or_warn(*argv, /*O_NOATIME |*/ O_NOCTTY | O_RDONLY | O_NONBLOCK);
		/* open(NOATIME) can only be used by owner or root, don't use NOATIME here */

		ಆದರೆ (fd < 0) {
			ret = EXIT_FAILURE;
			ಹೋಗು ಮುಂದಿನ;
		}
# ಆದರೆ ENABLE_FEATURE_SYNC_FANCY
		ಆದರೆ (opts & OPT_SYNCFS) {
			/*
			 * syncfs is documented to only fail with EBADF,
			 * which can't happen here. So, no error checks.
			 */
			syncfs(fd);
		} ಇಲ್ಲದಿದ್ದರೆ
# endif
		ಆದರೆ (((opts & OPT_DATASYNC) ? fdatasync(fd) : fsync(fd)) != 0) {
			bb_simple_perror_ಸಂದೇಶ(*argv);
			ret = EXIT_FAILURE;
		}
		close(fd);
 ಮುಂದಿನ:
		argv++;
	} ಆಗ (*argv);

	ಹಿಂತಿರುಗಿ ret;
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_SYNC
ಪೂರ್ಣಾಂಕ sync_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ sync_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv IF_NOT_DESKTOP(ಬಳಸದ_PARAM))
{
# ಆದರೆ !ENABLE_FEATURE_SYNC_FANCY
	/* coreutils-6.9 compat */
	bb_warn_ignoring_ವಾದಗಳು(argv[1]);
	sync();
	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
# ಇಲ್ಲದಿದ್ದರೆ
	ಚಿಹ್ನೆರಹಿತ opts = getopt32(argv, "^" "df" "\0" "d--f:f--d");
	argv += optind;
	ಆದರೆ (!argv[0]) {
		sync();
		ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
	}
	ಹಿಂತಿರುಗಿ sync_common(opts, argv);
# endif
}
#ಆದರೆ_ಅಂತ್ಯ

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

#ಆದರೆ ENABLE_FSYNC
ಪೂರ್ಣಾಂಕ fsync_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ fsync_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	ಪೂರ್ಣಾಂಕ opts = getopt32(argv, "^" "d" "\0" "-1"/*min 1 arg*/);
	argv += optind;
	ಹಿಂತಿರುಗಿ sync_common(opts, argv);
}
#ಆದರೆ_ಅಂತ್ಯ
