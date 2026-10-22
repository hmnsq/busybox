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

#അന്തര്ഭാവ "libbb.h"

#എങ്കിൽ ENABLE_LFS
# define XATOU_SFX xatoull_sfx
#അല്ലെങ്കിൽ
# define XATOU_SFX xatoul_sfx
#endif

/* This is a NOFORK applet. Be very careful! */

പൂർണ്ണസംഖ്യ truncate_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) MAIN_EXTERNALLY_VISIBLE;
പൂർണ്ണസംഖ്യ truncate_main(പൂർണ്ണസംഖ്യ argc UNUSED_PARAM, അക്ഷരം **argv)
{
	unsigned opts;
	പൂർണ്ണസംഖ്യ flags = O_WRONLY | O_NONBLOCK;
	പൂർണ്ണസംഖ്യ ret = EXIT_SUCCESS;
	അക്ഷരം *size_str;
	off_t size;

	എണ്ണൽ {
		OPT_NOCREATE  = (1 << 0),
		OPT_SIZE = (1 << 1),
	};

	opts = getopt32(argv, "^" "cs:" "\0" "s:-1", &size_str);

	എങ്കിൽ (!(opts & OPT_NOCREATE))
		flags |= O_CREAT;

	// TODO: coreutils 8.17 also support "m" (lowercase) suffix
	// with truncate, but not with dd!
	// We share kMG_suffixes[], so we can't make both tools
	// compatible at once...
	size = XATOU_SFX(size_str, kMG_suffixes);

	argv += optind;
	ആകുമ്പോൾ (*argv) {
		പൂർണ്ണസംഖ്യ fd = open(*argv, flags, 0666);
		എങ്കിൽ (fd < 0) {
			എങ്കിൽ (errno != ENOENT || !(opts & OPT_NOCREATE)) {
				bb_perror_സന്ദേശം("%s: open", *argv);
				ret = EXIT_FAILURE;
			}
			/* else: ENOENT && OPT_NOCREATE:
			 * do not report error, exitcode is also 0.
			 */
		} അല്ലെങ്കിൽ {
			എങ്കിൽ (ftruncate(fd, size) == -1) {
				bb_perror_സന്ദേശം("%s: truncate", *argv);
				ret = EXIT_FAILURE;
			}
			xclose(fd);
		}
		++argv;
	}

	തിരികെ ret;
}
