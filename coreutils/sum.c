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

#অন্তর্ভাৱ "libbb.h"
#অন্তর্ভাৱ "common_bufsiz.h"

গণনা { SUM_BSD, PRINT_NAME, SUM_SYSV };

/* BSD: calculate and print the rotated checksum and the size in 1K blocks
   The checksum varies depending on sizeof (int). */
/* SYSV: calculate and print the checksum and the size in 512-byte blocks */
/* Return 1 if successful.  */
স্থির unsigned sum_file(ধ্রুৱ ৱর্ণ *file, unsigned type)
{
	unsigned long long total_bytes = 0;
	পূর্ণাঙ্ক fd, r;
	/* The sum of all the input bytes, modulo (UINT_MAX + 1).  */
	unsigned s = 0;

#define buf bb_common_bufsiz1
	setup_common_bufsiz();

	fd = open_or_warn_stdin(file);
	যদি (fd == -1)
		ঘূৰাই 0;

	যেতিয়ালৈ (1) {
		size_t bytes_read = safe_read(fd, buf, COMMON_BUFSIZE);

		যদি ((ssize_t)bytes_read <= 0) {
			r = (fd && close(fd) != 0);
			যদি (!bytes_read && !r)
				/* no error */
				ভঙ্গ;
			bb_simple_perror_বাৰ্তা(file);
			ঘূৰাই 0;
		}

		total_bytes += bytes_read;
		যদি (type >= SUM_SYSV) {
			কৰা s += buf[--bytes_read]; যেতিয়ালৈ (bytes_read);
		} অন্যথা {
			r = 0;
			কৰা {
				s = (s >> 1) + ((s & 1) << 15);
				s += buf[r++];
				s &= 0xffff; /* Keep it within bounds. */
			} যেতিয়ালৈ (--bytes_read);
		}
	}

	যদি (type < PRINT_NAME)
		file = "";
	যদি (type >= SUM_SYSV) {
		r = (s & 0xffff) + ((s & 0xffffffff) >> 16);
		s = (r & 0xffff) + (r >> 16);
		printf("%u %llu %s\n", s, (total_bytes + 511) / 512, file);
	} অন্যথা
		printf("%05u %5llu %s\n", s, (total_bytes + 1023) / 1024, file);
	ঘূৰাই 1;
#undef buf
}

পূর্ণাঙ্ক sum_main(পূর্ণাঙ্ক argc, ৱর্ণ **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণাঙ্ক sum_main(পূর্ণাঙ্ক argc UNUSED_PARAM, ৱর্ণ **argv)
{
	unsigned n;
	unsigned type = SUM_BSD;

	n = getopt32(argv, "sr");
	argv += optind;
	যদি (n & 1) type = SUM_SYSV;
	/* give the bsd priority over sysv func */
	যদি (n & 2) type = SUM_BSD;

	যদি (!argv[0]) {
		/* Do not print the name */
		n = sum_file("-", type);
	} অন্যথা {
		/* Need to print the name if either
		 * - more than one file given
		 * - doing sysv */
		type += (argv[1] || type == SUM_SYSV);
		n = 1;
		কৰা {
			n &= sum_file(*argv, type);
		} যেতিয়ালৈ (*++argv);
	}
	ঘূৰাই !n;
}
