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

ಎಣಿಕೆ { ಮೊತ್ತ_BSD, PRINT_NAME, ಮೊತ್ತ_SYSV };

/* BSD: calculate and print the rotated checksum and the size in 1K blocks
   The checksum varies depending on sizeof (int). */
/* SYSV: calculate and print the checksum and the size in 512-byte blocks */
/* Return 1 if successful.  */
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಮೊತ್ತ_file(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *file, ಚಿಹ್ನೆರಹಿತ type)
{
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ ಒಟ್ಟು_ಬೈಟುಗಳು = 0;
	ಪೂರ್ಣಾಂಕ fd, r;
	/* The sum of all the input bytes, modulo (UINT_MAX + 1).  */
	ಚಿಹ್ನೆರಹಿತ s = 0;

#ವ್ಯಾಖ್ಯಾನಿಸು buf bb_common_bufsiz1
	ಸಿದ್ಧತೆ_common_bufsiz();

	fd = open_or_warn_stdin(file);
	ಆದರೆ (fd == -1)
		ಹಿಂತಿರುಗಿ 0;

	ಆಗ (1) {
		ಗಾತ್ರ_t ಬೈಟುಗಳು_read = ಸುರಕ್ಷಿತ_read(fd, buf, COMMON_BUFSIZE);

		ಆದರೆ ((ssize_t)ಬೈಟುಗಳು_read <= 0) {
			r = (fd && close(fd) != 0);
			ಆದರೆ (!ಬೈಟುಗಳು_read && !r)
				/* no error */
				ನಿಲ್ಲಿಸು;
			bb_simple_perror_ಸಂದೇಶ(file);
			ಹಿಂತಿರುಗಿ 0;
		}

		ಒಟ್ಟು_ಬೈಟುಗಳು += ಬೈಟುಗಳು_read;
		ಆದರೆ (type >= ಮೊತ್ತ_SYSV) {
			ಮಾಡು s += buf[--ಬೈಟುಗಳು_read]; ಆಗ (ಬೈಟುಗಳು_read);
		} ಇಲ್ಲದಿದ್ದರೆ {
			r = 0;
			ಮಾಡು {
				s = (s >> 1) + ((s & 1) << 15);
				s += buf[r++];
				s &= 0xffff; /* Keep it within bounds. */
			} ಆಗ (--ಬೈಟುಗಳು_read);
		}
	}

	ಆದರೆ (type < PRINT_NAME)
		file = "";
	ಆದರೆ (type >= ಮೊತ್ತ_SYSV) {
		r = (s & 0xffff) + ((s & 0xffffffff) >> 16);
		s = (r & 0xffff) + (r >> 16);
		ಬರೆ("%u %llu %s\n", s, (ಒಟ್ಟು_ಬೈಟುಗಳು + 511) / 512, file);
	} ಇಲ್ಲದಿದ್ದರೆ
		ಬರೆ("%05u %5llu %s\n", s, (ಒಟ್ಟು_ಬೈಟುಗಳು + 1023) / 1024, file);
	ಹಿಂತಿರುಗಿ 1;
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು buf
}

ಪೂರ್ಣಾಂಕ ಮೊತ್ತ_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ ಮೊತ್ತ_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	ಚಿಹ್ನೆರಹಿತ n;
	ಚಿಹ್ನೆರಹಿತ type = ಮೊತ್ತ_BSD;

	n = getopt32(argv, "sr");
	argv += optind;
	ಆದರೆ (n & 1) type = ಮೊತ್ತ_SYSV;
	/* give the bsd priority over sysv func */
	ಆದರೆ (n & 2) type = ಮೊತ್ತ_BSD;

	ಆದರೆ (!argv[0]) {
		/* Do not print the name */
		n = ಮೊತ್ತ_file("-", type);
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Need to print the name if either
		 * - more than one file given
		 * - doing sysv */
		type += (argv[1] || type == ಮೊತ್ತ_SYSV);
		n = 1;
		ಮಾಡು {
			n &= ಮೊತ್ತ_file(*argv, type);
		} ಆಗ (*++argv);
	}
	ಹಿಂತಿರುಗಿ !n;
}
