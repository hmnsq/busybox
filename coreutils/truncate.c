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

#అన్తర్భావ "libbb.h"

#ఒకవేళ ENABLE_LFS
# define XATOU_SFX xatoull_sfx
#లేకపోతే
# define XATOU_SFX xatoul_sfx
#అయితే_ముగింపు

/* This is a NOFORK applet. Be very careful! */

పూర్ణాంకం truncate_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం truncate_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	గుర్తులేని opts;
	పూర్ణాంకం flags = O_WRONLY | O_NONBLOCK;
	పూర్ణాంకం ret = EXIT_SUCCESS;
	అక్షరం *కొలత_str;
	off_t కొలత;

	గణన {
		OPT_NOCREATE  = (1 << 0),
		OPT_కొలత = (1 << 1),
	};

	opts = getopt32(argv, "^" "cs:" "\0" "s:-1", &కొలత_str);

	ఒకవేళ (!(opts & OPT_NOCREATE))
		flags |= O_CREAT;

	// TODO: coreutils 8.17 also support "m" (lowercase) suffix
	// with truncate, but not with dd!
	// We share kMG_suffixes[], so we can't make both tools
	// compatible at once...
	కొలత = XATOU_SFX(కొలత_str, kMG_suffixes);

	argv += optind;
	అయితే (*argv) {
		పూర్ణాంకం fd = open(*argv, flags, 0666);
		ఒకవేళ (fd < 0) {
			ఒకవేళ (errno != ENOENT || !(opts & OPT_NOCREATE)) {
				bb_perror_సందేశం("%s: open", *argv);
				ret = EXIT_FAILURE;
			}
			/* else: ENOENT && OPT_NOCREATE:
			 * do not report error, exitcode is also 0.
			 */
		} లేకపోతే {
			ఒకవేళ (ftruncate(fd, కొలత) == -1) {
				bb_perror_సందేశం("%s: truncate", *argv);
				ret = EXIT_FAILURE;
			}
			xclose(fd);
		}
		++argv;
	}

	తిరిగి ret;
}
