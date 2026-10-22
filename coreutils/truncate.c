/*
 * Mini truncate implementation for busybox
 *
 * Copyright (C) 2015 by Ari Sundholm <ari@tuxera.com>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config TRUNCATE
//config:	bool "truncate (4.4 kb)"
//config:	default y
//config:	help
//config:	truncate truncates files to a given size. If a file does
//config:	not exist, it is created unless told otherwise.

//applet:IF_TRUNCATE(APPLET_NOFORK(truncate, truncate, BB_DIR_USR_BIN, BB_SUID_DROP, truncate))

//kbuild:lib-$(CONFIG_TRUNCATE) += truncate.o

//usage:#define truncate_trivial_usage
//usage:       "[-c] -s SIZE FILE..."
//usage:#define truncate_full_usage "\n\n"
//usage:	"Truncate FILEs to SIZE\n"
//usage:	"\n	-c	Do not create files"
//usage:	"\n	-s SIZE"
//usage:
//usage:#define truncate_example_usage
//usage:	"$ truncate -s 1G foo"

#ಅನ್ತರ್ಭಾವ "libbb.h"

#ಯದಿ ENABLE_LFS
# define XATOU_SFX xatoull_sfx
#ಅನ್ಯಥಾ
# define XATOU_SFX xatoul_sfx
#endif

/* This is a NOFORK applet. Be very careful! */

ಪೂರ್ಣಾಙ್ಕ truncate_main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಙ್ಕ truncate_main(ಪೂರ್ಣಾಙ್ಕ argc UNUSED_PARAM, ವರ್ಣ **argv)
{
	unsigned opts;
	ಪೂರ್ಣಾಙ್ಕ flags = O_WRONLY | O_NONBLOCK;
	ಪೂರ್ಣಾಙ್ಕ ret = EXIT_SUCCESS;
	ವರ್ಣ *size_str;
	off_t size;

	ಗಣನಾ {
		OPT_NOCREATE  = (1 << 0),
		OPT_SIZE = (1 << 1),
	};

	opts = getopt32(argv, "^" "cs:" "\0" "s:-1", &size_str);

	ಯದಿ (!(opts & OPT_NOCREATE))
		flags |= O_CREAT;

	// TODO: coreutils 8.17 also support "m" (lowercase) suffix
	// with truncate, but not with dd!
	// We share kMG_suffixes[], so we can't make both tools
	// compatible at once...
	size = XATOU_SFX(size_str, kMG_suffixes);

	argv += optind;
	ಯಾವತ್ (*argv) {
		ಪೂರ್ಣಾಙ್ಕ fd = open(*argv, flags, 0666);
		ಯದಿ (fd < 0) {
			ಯದಿ (errno != ENOENT || !(opts & OPT_NOCREATE)) {
				bb_perror_ಸಂದೇಶ("%s: open", *argv);
				ret = EXIT_FAILURE;
			}
			/* else: ENOENT && OPT_NOCREATE:
			 * do not report error, exitcode is also 0.
			 */
		} ಅನ್ಯಥಾ {
			ಯದಿ (ftruncate(fd, size) == -1) {
				bb_perror_ಸಂದೇಶ("%s: truncate", *argv);
				ret = EXIT_FAILURE;
			}
			xclose(fd);
		}
		++argv;
	}

	ಪ್ರತ್ಯಾಗಮ ret;
}
