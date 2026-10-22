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

#ಆದರೆ ENABLE_LFS
# define XATOU_SFX xatoull_sfx
#ಇಲ್ಲದಿದ್ದರೆ
# define XATOU_SFX xatoul_sfx
#ಆದರೆ_ಅಂತ್ಯ

/* This is a NOFORK applet. Be very careful! */

ಪೂರ್ಣಾಂಕ truncate_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ truncate_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	ಚಿಹ್ನೆರಹಿತ opts;
	ಪೂರ್ಣಾಂಕ flags = O_WRONLY | O_NONBLOCK;
	ಪೂರ್ಣಾಂಕ ret = EXIT_SUCCESS;
	ಅಕ್ಷರ *ಗಾತ್ರ_str;
	off_t ಗಾತ್ರ_2;

	ಎಣಿಕೆ {
		OPT_NOCREATE  = (1 << 0),
		OPT_ಗಾತ್ರ = (1 << 1),
	};

	opts = getopt32(argv, "^" "cs:" "\0" "s:-1", &ಗಾತ್ರ_str);

	ಆದರೆ (!(opts & OPT_NOCREATE))
		flags |= O_CREAT;

	// TODO: coreutils 8.17 also support "m" (lowercase) suffix
	// with truncate, but not with dd!
	// We share kMG_suffixes[], so we can't make both tools
	// compatible at once...
	ಗಾತ್ರ_2 = XATOU_SFX(ಗಾತ್ರ_str, kMG_suffixes);

	argv += optind;
	ಆಗ (*argv) {
		ಪೂರ್ಣಾಂಕ fd = open(*argv, flags, 0666);
		ಆದರೆ (fd < 0) {
			ಆದರೆ (errno != ENOENT || !(opts & OPT_NOCREATE)) {
				bb_perror_ಸಂದೇಶ("%s: open", *argv);
				ret = EXIT_FAILURE;
			}
			/* else: ENOENT && OPT_NOCREATE:
			 * do not report error, exitcode is also 0.
			 */
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (ftruncate(fd, ಗಾತ್ರ_2) == -1) {
				bb_perror_ಸಂದೇಶ("%s: truncate", *argv);
				ret = EXIT_FAILURE;
			}
			xclose(fd);
		}
		++argv;
	}

	ಹಿಂತಿರುಗಿ ret;
}
