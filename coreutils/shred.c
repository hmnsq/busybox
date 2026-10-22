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

#अन्तर्भाव "libbb.h"

पूर्णाङ्क shred_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क shred_main(पूर्णाङ्क argc अप्रयुक्तम्_PARAM, वर्ण **argv)
{
	वर्ण *opt_s;
	पूर्णाङ्क rand_fd = rand_fd; /* for compiler */
	पूर्णाङ्क zero_fd;
	अचिह्नित अङ्कः_iter = 3;
	अचिह्नित opt;
	गणना {
		OPT_f = (1 << 0),
		OPT_u = (1 << 1),
		OPT_z = (1 << 2),
		OPT_n = (1 << 3),
		OPT_v = (1 << 4),
		OPT_x = (1 << 5),
		OPT_s = (1 << 6),
	};

	opt = getopt32(argv, "^" "fuzn:+vxs:" "\0" "-1"/*min 1 arg*/, &अङ्कः_iter, &opt_s);
	argv += optind;

	zero_fd = xopen("/dev/zero", O_RDONLY);
	यदि (अङ्कः_iter != 0)
		rand_fd = xopen("/dev/urandom", O_RDONLY);

	प्रति (;;) {
		संरचना stat sb;
		ध्रुव वर्ण *fname;
		अचिह्नित i;
		पूर्णाङ्क fd;

		fname = *argv++;
		यदि (!fname)
			भङ्ग;
		fd = -1;
		यदि (opt & OPT_f) {
			fd = उद्घाटय(fname, O_WRONLY);
			यदि (fd < 0)
				chmod(fname, 0666);
		}
		यदि (fd < 0)
			fd = xopen(fname, O_WRONLY);

		यदि (fstat(fd, &sb) == 0 && sb.st_आकार > 0) {
			off_t आकार = sb.st_आकार;

			यदि (opt & OPT_s) {
				आकार = BB_STRTOOFF(opt_s, रिक्त, 0); /* accepts oct/hex */
				यदि (errno || आकार < 0) bb_show_usage();
			}

			प्रति (i = 0; i < अङ्कः_iter; i++) {
				bb_copyfd_आकार(rand_fd, fd, आकार);
				fdatasync(fd);
				xlseek(fd, 0, SEEK_समुच्चयः);
			}
			यदि (opt & OPT_z) {
				bb_copyfd_आकार(zero_fd, fd, आकार);
				fdatasync(fd);
			}
		}
		यदि (opt & OPT_u) {
			ftruncate(fd, 0);
			xunlink(fname);
		}
		xclose(fd);
	}

	प्रत्यागम EXIT_SUCCESS;
}
