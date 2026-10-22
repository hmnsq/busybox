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

#অন্তর্ভাব "libbb.h"

#যদি ENABLE_LFS
# define XATOU_SFX xatoull_sfx
#অন্যথা
# define XATOU_SFX xatoul_sfx
#endif

/* This is a NOFORK applet. Be very careful! */

পূর্ণসংখ্যা truncate_main(পূর্ণসংখ্যা argc, অক্ষর **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণসংখ্যা truncate_main(পূর্ণসংখ্যা argc UNUSED_PARAM, অক্ষর **argv)
{
	unsigned opts;
	পূর্ণসংখ্যা flags = O_WRONLY | O_NONBLOCK;
	পূর্ণসংখ্যা ret = EXIT_SUCCESS;
	অক্ষর *আকার_str;
	off_t আকার_2;

	গণনা {
		OPT_NOCREATE  = (1 << 0),
		OPT_আকার = (1 << 1),
	};

	opts = getopt32(argv, "^" "cs:" "\0" "s:-1", &আকার_str);

	যদি (!(opts & OPT_NOCREATE))
		flags |= O_CREAT;

	// TODO: coreutils 8.17 also support "m" (lowercase) suffix
	// with truncate, but not with dd!
	// We share kMG_suffixes[], so we can't make both tools
	// compatible at once...
	আকার_2 = XATOU_SFX(আকার_str, kMG_suffixes);

	argv += optind;
	যখন (*argv) {
		পূর্ণসংখ্যা fd = open(*argv, flags, 0666);
		যদি (fd < 0) {
			যদি (errno != ENOENT || !(opts & OPT_NOCREATE)) {
				bb_perror_বার্তা("%s: open", *argv);
				ret = EXIT_FAILURE;
			}
			/* else: ENOENT && OPT_NOCREATE:
			 * do not report error, exitcode is also 0.
			 */
		} অন্যথা {
			যদি (ftruncate(fd, আকার_2) == -1) {
				bb_perror_বার্তা("%s: truncate", *argv);
				ret = EXIT_FAILURE;
			}
			xclose(fd);
		}
		++argv;
	}

	ফেরত ret;
}
