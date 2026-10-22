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

#അന്തര്ഭാവ "libbb.h"

പൂർണ്ണസംഖ്യ shred_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) MAIN_EXTERNALLY_VISIBLE;
പൂർണ്ണസംഖ്യ shred_main(പൂർണ്ണസംഖ്യ argc UNUSED_PARAM, അക്ഷരം **argv)
{
	അക്ഷരം *opt_s;
	പൂർണ്ണസംഖ്യ rand_fd = rand_fd; /* for compiler */
	പൂർണ്ണസംഖ്യ zero_fd;
	unsigned അക്കം_iter = 3;
	unsigned opt;
	എണ്ണൽ {
		OPT_f = (1 << 0),
		OPT_u = (1 << 1),
		OPT_z = (1 << 2),
		OPT_n = (1 << 3),
		OPT_v = (1 << 4),
		OPT_x = (1 << 5),
		OPT_s = (1 << 6),
	};

	opt = getopt32(argv, "^" "fuzn:+vxs:" "\0" "-1"/*min 1 arg*/, &അക്കം_iter, &opt_s);
	argv += optind;

	zero_fd = xopen("/dev/zero", O_RDONLY);
	എങ്കിൽ (അക്കം_iter != 0)
		rand_fd = xopen("/dev/urandom", O_RDONLY);

	വേണ്ടി (;;) {
		ഘടന stat sb;
		സ്ഥിരം അക്ഷരം *fname;
		unsigned i;
		പൂർണ്ണസംഖ്യ fd;

		fname = *argv++;
		എങ്കിൽ (!fname)
			നിർത്തുക;
		fd = -1;
		എങ്കിൽ (opt & OPT_f) {
			fd = open(fname, O_WRONLY);
			എങ്കിൽ (fd < 0)
				chmod(fname, 0666);
		}
		എങ്കിൽ (fd < 0)
			fd = xopen(fname, O_WRONLY);

		എങ്കിൽ (fstat(fd, &sb) == 0 && sb.st_size > 0) {
			off_t size = sb.st_size;

			എങ്കിൽ (opt & OPT_s) {
				size = BB_STRTOOFF(opt_s, ശൂന്യ, 0); /* accepts oct/hex */
				എങ്കിൽ (errno || size < 0) bb_show_usage();
			}

			വേണ്ടി (i = 0; i < അക്കം_iter; i++) {
				bb_copyfd_size(rand_fd, fd, size);
				fdatasync(fd);
				xlseek(fd, 0, SEEK_SET);
			}
			എങ്കിൽ (opt & OPT_z) {
				bb_copyfd_size(zero_fd, fd, size);
				fdatasync(fd);
			}
		}
		എങ്കിൽ (opt & OPT_u) {
			ftruncate(fd, 0);
			xunlink(fname);
		}
		xclose(fd);
	}

	തിരികെ EXIT_SUCCESS;
}
