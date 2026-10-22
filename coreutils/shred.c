/* vi: set sw=4 ts=4: */
/*
 * Copyright (C) 2017 Denys Vlasenko <vda.linux@googlemail.com>
 *
 * Licensed under GPLv2, see file LICENSE in this source tree.
 */
//config:config SHRED
//config:	bool "shred (5.5 kb)"
//config:	default y
//config:	help
//config:	Overwrite a file to hide its contents, and optionally delete it

//applet:IF_SHRED(APPLET(shred, BB_DIR_USR_BIN, BB_SUID_DROP))

//kbuild:lib-$(CONFIG_SHRED) += shred.o

//usage:#define shred_trivial_usage
//usage:       "[-fuz] [-n N] [-s SIZE] FILE..."
//usage:#define shred_full_usage "\n\n"
//usage:       "Overwrite/delete FILEs\n"
//usage:     "\n	-f	Chmod to ensure writability"
//usage:     "\n	-s SIZE	Size to write"
//usage:     "\n	-n N	Overwrite N times (default 3)"
//usage:     "\n	-z	Final overwrite with zeros"
//usage:     "\n	-u	Remove file"
//-x (exact: don't round up to 4k) and -v (verbose) are accepted but have no effect

/* shred (GNU coreutils) 8.25:
-f, --force		change permissions to allow writing if necessary
-u			truncate and remove file after overwriting
-z, --zero		add a final overwrite with zeros to hide shredding
-n, --iterations=N	overwrite N times instead of the default (3)
-v, --verbose		show progress
-x, --exact		do not round file sizes up to the next full block; this is the default for non-regular files
--random-source=FILE	get random bytes from FILE
-s, --size=N		shred this many bytes (suffixes like K, M, G accepted)
--remove[=HOW]		like -u but give control on HOW to delete;  See below
*/

#ಅನ್ತರ್ಭಾವ "libbb.h"

ಪೂರ್ಣಾಂಕ shred_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ shred_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	ಅಕ್ಷರ *opt_s;
	ಪೂರ್ಣಾಂಕ rand_fd = rand_fd; /* for compiler */
	ಪೂರ್ಣಾಂಕ zero_fd;
	ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_iter = 3;
	ಚಿಹ್ನೆರಹಿತ opt;
	ಎಣಿಕೆ {
		OPT_f = (1 << 0),
		OPT_u = (1 << 1),
		OPT_z = (1 << 2),
		OPT_n = (1 << 3),
		OPT_v = (1 << 4),
		OPT_x = (1 << 5),
		OPT_s = (1 << 6),
	};

	opt = getopt32(argv, "^" "fuzn:+vxs:" "\0" "-1"/*min 1 arg*/, &ಅಂಕೆ_iter, &opt_s);
	argv += optind;

	zero_fd = xopen("/dev/zero", O_RDONLY);
	ಆದರೆ (ಅಂಕೆ_iter != 0)
		rand_fd = xopen("/dev/urandom", O_RDONLY);

	ಗಾಗಿ (;;) {
		ರಚನೆ stat sb;
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname;
		ಚಿಹ್ನೆರಹಿತ i;
		ಪೂರ್ಣಾಂಕ fd;

		fname = *argv++;
		ಆದರೆ (!fname)
			ನಿಲ್ಲಿಸು;
		fd = -1;
		ಆದರೆ (opt & OPT_f) {
			fd = open(fname, O_WRONLY);
			ಆದರೆ (fd < 0)
				chmod(fname, 0666);
		}
		ಆದರೆ (fd < 0)
			fd = xopen(fname, O_WRONLY);

		ಆದರೆ (fstat(fd, &sb) == 0 && sb.st_ಗಾತ್ರ > 0) {
			off_t ಗಾತ್ರ_2 = sb.st_ಗಾತ್ರ;

			ಆದರೆ (opt & OPT_s) {
				ಗಾತ್ರ_2 = BB_STRTOOFF(opt_s, ಖಾಲಿ, 0); /* accepts oct/hex */
				ಆದರೆ (errno || ಗಾತ್ರ_2 < 0) bb_show_usage();
			}

			ಗಾಗಿ (i = 0; i < ಅಂಕೆ_iter; i++) {
				bb_copyfd_ಗಾತ್ರ(rand_fd, fd, ಗಾತ್ರ_2);
				fdatasync(fd);
				xlseek(fd, 0, SEEK_ಗಣ);
			}
			ಆದರೆ (opt & OPT_z) {
				bb_copyfd_ಗಾತ್ರ(zero_fd, fd, ಗಾತ್ರ_2);
				fdatasync(fd);
			}
		}
		ಆದರೆ (opt & OPT_u) {
			ftruncate(fd, 0);
			xunlink(fname);
		}
		xclose(fd);
	}

	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}
