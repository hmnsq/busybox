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

#شامل "libbb.h"

#اگر ENABLE_LFS
# define XATOU_SFX xatoull_sfx
#ورنہ
# define XATOU_SFX xatoul_sfx
#اگر_ختم

/* This is a NOFORK applet. Be very careful! */

صحیح truncate_main(صحیح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح truncate_main(صحیح argc غیر_مستعمل_PARAM, حرف **argv)
{
	غیرعلامتی opts;
	صحیح flags = O_WRONLY | O_NONBLOCK;
	صحیح ret = EXIT_SUCCESS;
	حرف *سائز_str;
	off_t سائز;

	شمار {
		OPT_NOCREATE  = (1 << 0),
		OPT_سائز = (1 << 1),
	};

	opts = getopt32(argv, "^" "cs:" "\0" "s:-1", &سائز_str);

	اگر (!(opts & OPT_NOCREATE))
		flags |= O_CREAT;

	// TODO: coreutils 8.17 also support "m" (lowercase) suffix
	// with truncate, but not with dd!
	// We share kMG_suffixes[], so we can't make both tools
	// compatible at once...
	سائز = XATOU_SFX(سائز_str, kMG_suffixes);

	argv += optind;
	جب (*argv) {
		صحیح fd = open(*argv, flags, 0666);
		اگر (fd < 0) {
			اگر (errno != ENOENT || !(opts & OPT_NOCREATE)) {
				bb_perror_پیغام("%s: open", *argv);
				ret = EXIT_FAILURE;
			}
			/* else: ENOENT && OPT_NOCREATE:
			 * do not report error, exitcode is also 0.
			 */
		} ورنہ {
			اگر (ftruncate(fd, سائز) == -1) {
				bb_perror_پیغام("%s: truncate", *argv);
				ret = EXIT_FAILURE;
			}
			xclose(fd);
		}
		++argv;
	}

	واپس ret;
}
