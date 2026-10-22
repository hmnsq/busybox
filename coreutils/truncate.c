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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"

#ਜੇ ENABLE_LFS
# define XATOU_SFX xatoull_sfx
#ਨਹੀਂਤਾਂ
# define XATOU_SFX xatoul_sfx
#ਜੇ_ਅੰਤ

/* This is a NOFORK applet. Be very careful! */

ਪੂਰਨ truncate_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ truncate_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ opts;
	ਪੂਰਨ flags = O_WRONLY | O_NONBLOCK;
	ਪੂਰਨ ret = EXIT_SUCCESS;
	ਅੱਖਰ *ਮਾਪ_str;
	off_t ਮਾਪ;

	ਗਿਣਤੀ {
		OPT_NOCREATE  = (1 << 0),
		OPT_ਮਾਪ = (1 << 1),
	};

	opts = getopt32(argv, "^" "cs:" "\0" "s:-1", &ਮਾਪ_str);

	ਜੇ (!(opts & OPT_NOCREATE))
		flags |= O_CREAT;

	// TODO: coreutils 8.17 also support "m" (lowercase) suffix
	// with truncate, but not with dd!
	// We share kMG_suffixes[], so we can't make both tools
	// compatible at once...
	ਮਾਪ = XATOU_SFX(ਮਾਪ_str, kMG_suffixes);

	argv += optind;
	ਜਦੋਂ (*argv) {
		ਪੂਰਨ fd = open(*argv, flags, 0666);
		ਜੇ (fd < 0) {
			ਜੇ (errno != ENOENT || !(opts & OPT_NOCREATE)) {
				bb_perror_ਸੁਨੇਹਾ("%s: open", *argv);
				ret = EXIT_FAILURE;
			}
			/* else: ENOENT && OPT_NOCREATE:
			 * do not report error, exitcode is also 0.
			 */
		} ਨਹੀਂਤਾਂ {
			ਜੇ (ftruncate(fd, ਮਾਪ) == -1) {
				bb_perror_ਸੁਨੇਹਾ("%s: truncate", *argv);
				ret = EXIT_FAILURE;
			}
			xclose(fd);
		}
		++argv;
	}

	ਵਾਪਸ ret;
}
