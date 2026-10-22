/* vi: set sw=4 ts=4: */
/*
 * sum -- checksum and count the blocks in a file
 *     Like BSD sum or SysV sum -r, except like SysV sum if -s option is given.
 *
 * Copyright (C) 86, 89, 91, 1995-2002, 2004 Free Software Foundation, Inc.
 * Copyright (C) 2005 by Erik Andersen <andersen@codepoet.org>
 * Copyright (C) 2005 by Mike Frysinger <vapier@gentoo.org>
 *
 * Written by Kayvan Aghaiepour and David MacKenzie
 * Taken from coreutils and turned into a busybox applet by Mike Frysinger
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config SUM
//config:	bool "sum (4.2 kb)"
//config:	default y
//config:	help
//config:	checksum and count the blocks in a file

//applet:IF_SUM(APPLET(sum, BB_DIR_USR_BIN, BB_SUID_DROP))

//kbuild:lib-$(CONFIG_SUM) += sum.o

//usage:#define sum_trivial_usage
//usage:       "[-rs] [FILE]..."
//usage:#define sum_full_usage "\n\n"
//usage:       "Checksum and count the blocks in a file\n"
//usage:     "\n	-r	Use BSD sum algorithm (1K blocks)"
//usage:     "\n	-s	Use System V sum algorithm (512byte blocks)"

#ಅನ್ತರ್ಭಾವ "libbb.h"
#ಅನ್ತರ್ಭಾವ "common_bufsiz.h"

ಗಣನಾ { SUM_BSD, PRINT_NAME, SUM_SYSV };

/* BSD: calculate and print the rotated checksum and the size in 1K blocks
   The checksum varies depending on sizeof (int). */
/* SYSV: calculate and print the checksum and the size in 512-byte blocks */
/* Return 1 if successful.  */
ಸ್ಥಿರ unsigned sum_file(ಧ್ರುವ ವರ್ಣ *file, unsigned type)
{
	unsigned long long total_bytes = 0;
	ಪೂರ್ಣಾಙ್ಕ fd, r;
	/* The sum of all the input bytes, modulo (UINT_MAX + 1).  */
	unsigned s = 0;

#define buf bb_common_bufsiz1
	setup_common_bufsiz();

	fd = open_or_warn_stdin(file);
	ಯದಿ (fd == -1)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಯಾವತ್ (1) {
		size_t bytes_read = safe_read(fd, buf, COMMON_BUFSIZE);

		ಯದಿ ((ssize_t)bytes_read <= 0) {
			r = (fd && close(fd) != 0);
			ಯದಿ (!bytes_read && !r)
				/* no error */
				ಭಙ್ಗ;
			bb_simple_perror_ಸಂದೇಶ(file);
			ಪ್ರತ್ಯಾಗಮ 0;
		}

		total_bytes += bytes_read;
		ಯದಿ (type >= SUM_SYSV) {
			ಕುರು s += buf[--bytes_read]; ಯಾವತ್ (bytes_read);
		} ಅನ್ಯಥಾ {
			r = 0;
			ಕುರು {
				s = (s >> 1) + ((s & 1) << 15);
				s += buf[r++];
				s &= 0xffff; /* Keep it within bounds. */
			} ಯಾವತ್ (--bytes_read);
		}
	}

	ಯದಿ (type < PRINT_NAME)
		file = "";
	ಯದಿ (type >= SUM_SYSV) {
		r = (s & 0xffff) + ((s & 0xffffffff) >> 16);
		s = (r & 0xffff) + (r >> 16);
		printf("%u %llu %s\n", s, (total_bytes + 511) / 512, file);
	} ಅನ್ಯಥಾ
		printf("%05u %5llu %s\n", s, (total_bytes + 1023) / 1024, file);
	ಪ್ರತ್ಯಾಗಮ 1;
#undef buf
}

ಪೂರ್ಣಾಙ್ಕ sum_main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಙ್ಕ sum_main(ಪೂರ್ಣಾಙ್ಕ argc UNUSED_PARAM, ವರ್ಣ **argv)
{
	unsigned n;
	unsigned type = SUM_BSD;

	n = getopt32(argv, "sr");
	argv += optind;
	ಯದಿ (n & 1) type = SUM_SYSV;
	/* give the bsd priority over sysv func */
	ಯದಿ (n & 2) type = SUM_BSD;

	ಯದಿ (!argv[0]) {
		/* Do not print the name */
		n = sum_file("-", type);
	} ಅನ್ಯಥಾ {
		/* Need to print the name if either
		 * - more than one file given
		 * - doing sysv */
		type += (argv[1] || type == SUM_SYSV);
		n = 1;
		ಕುರು {
			n &= sum_file(*argv, type);
		} ಯಾವತ್ (*++argv);
	}
	ಪ್ರತ್ಯಾಗಮ !n;
}
