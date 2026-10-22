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

#ଅନ୍ତର୍ଭାଵ "libbb.h"

#ଯଦି ENABLE_LFS
# define XATOU_SFX xatoull_sfx
#ଅନ୍ୟଥା
# define XATOU_SFX xatoul_sfx
#endif

/* This is a NOFORK applet. Be very careful! */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା truncate_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv) MAIN_EXTERNALLY_VISIBLE;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା truncate_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc UNUSED_PARAM, ଅକ୍ଷର **argv)
{
	unsigned opts;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = O_WRONLY | O_NONBLOCK;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = EXIT_SUCCESS;
	ଅକ୍ଷର *size_str;
	off_t size;

	ଗଣନା {
		OPT_NOCREATE  = (1 << 0),
		OPT_SIZE = (1 << 1),
	};

	opts = getopt32(argv, "^" "cs:" "\0" "s:-1", &size_str);

	ଯଦି (!(opts & OPT_NOCREATE))
		flags |= O_CREAT;

	// TODO: coreutils 8.17 also support "m" (lowercase) suffix
	// with truncate, but not with dd!
	// We share kMG_suffixes[], so we can't make both tools
	// compatible at once...
	size = XATOU_SFX(size_str, kMG_suffixes);

	argv += optind;
	ଯେତେବେଳେ (*argv) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd = open(*argv, flags, 0666);
		ଯଦି (fd < 0) {
			ଯଦି (errno != ENOENT || !(opts & OPT_NOCREATE)) {
				bb_perror_ସନ୍ଦେଶ("%s: open", *argv);
				ret = EXIT_FAILURE;
			}
			/* else: ENOENT && OPT_NOCREATE:
			 * do not report error, exitcode is also 0.
			 */
		} ଅନ୍ୟଥା {
			ଯଦି (ftruncate(fd, size) == -1) {
				bb_perror_ସନ୍ଦେଶ("%s: truncate", *argv);
				ret = EXIT_FAILURE;
			}
			xclose(fd);
		}
		++argv;
	}

	ଫେରନ୍ତୁ ret;
}
