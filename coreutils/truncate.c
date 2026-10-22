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

#अन्तर्भाव "libbb.h"

#जर ENABLE_LFS
# define XATOU_SFX xatoull_sfx
#अन्यथा
# define XATOU_SFX xatoul_sfx
#endif

/* This is a NOFORK applet. Be very careful! */

पूर्णांक truncate_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक truncate_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	अचिन्हित opts;
	पूर्णांक flags = O_WRONLY | O_NONBLOCK;
	पूर्णांक ret = निकास_सफल;
	अक्षर *size_str;
	off_t size;

	गणना {
		OPT_NOCREATE  = (1 << 0),
		OPT_SIZE = (1 << 1),
	};

	opts = getopt32(argv, "^" "cs:" "\0" "s:-1", &size_str);

	जर (!(opts & OPT_NOCREATE))
		flags |= O_CREAT;

	// TODO: coreutils 8.17 also support "m" (lowercase) suffix
	// with truncate, but not with dd!
	// We share kMG_suffixes[], so we can't make both tools
	// compatible at once...
	size = XATOU_SFX(size_str, kMG_suffixes);

	argv += optind;
	पर्यंत (*argv) {
		पूर्णांक fd = खोलो(*argv, flags, 0666);
		जर (fd < 0) {
			जर (त्रुटि_सं != ENOENT || !(opts & OPT_NOCREATE)) {
				bb_perror_संदेश("%s: open", *argv);
				ret = निकास_त्रुटि;
			}
			/* else: ENOENT && OPT_NOCREATE:
			 * do not report error, exitcode is also 0.
			 */
		} अन्यथा {
			जर (ftruncate(fd, size) == -1) {
				bb_perror_संदेश("%s: truncate", *argv);
				ret = निकास_त्रुटि;
			}
			xclose(fd);
		}
		++argv;
	}

	परत ret;
}
