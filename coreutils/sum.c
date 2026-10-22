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

#అన్తర్భావ "libbb.h"
#అన్తర్భావ "common_bufsiz.h"

గణన { కూడిక_BSD, PRINT_NAME, కూడిక_SYSV };

/* BSD: calculate and print the rotated checksum and the size in 1K blocks
   The checksum varies depending on sizeof (int). */
/* SYSV: calculate and print the checksum and the size in 512-byte blocks */
/* Return 1 if successful.  */
స్థిర గుర్తులేని కూడిక_file(స్థిరరాశి అక్షరం *file, గుర్తులేని type)
{
	గుర్తులేని దీర్ఘ దీర్ఘ మొత్తం_బైట్లు = 0;
	పూర్ణాంకం fd, r;
	/* The sum of all the input bytes, modulo (UINT_MAX + 1).  */
	గుర్తులేని s = 0;

#నిర్వచించు buf bb_common_bufsiz1
	అమరిక_common_bufsiz();

	fd = open_or_warn_stdin(file);
	ఒకవేళ (fd == -1)
		తిరిగి 0;

	అయితే (1) {
		కొలత_t బైట్లు_read = సురక్షిత_read(fd, buf, COMMON_BUFSIZE);

		ఒకవేళ ((ssize_t)బైట్లు_read <= 0) {
			r = (fd && close(fd) != 0);
			ఒకవేళ (!బైట్లు_read && !r)
				/* no error */
				ఆపు;
			bb_simple_perror_సందేశం(file);
			తిరిగి 0;
		}

		మొత్తం_బైట్లు += బైట్లు_read;
		ఒకవేళ (type >= కూడిక_SYSV) {
			చేయి s += buf[--బైట్లు_read]; అయితే (బైట్లు_read);
		} లేకపోతే {
			r = 0;
			చేయి {
				s = (s >> 1) + ((s & 1) << 15);
				s += buf[r++];
				s &= 0xffff; /* Keep it within bounds. */
			} అయితే (--బైట్లు_read);
		}
	}

	ఒకవేళ (type < PRINT_NAME)
		file = "";
	ఒకవేళ (type >= కూడిక_SYSV) {
		r = (s & 0xffff) + ((s & 0xffffffff) >> 16);
		s = (r & 0xffff) + (r >> 16);
		రాయి("%u %llu %s\n", s, (మొత్తం_బైట్లు + 511) / 512, file);
	} లేకపోతే
		రాయి("%05u %5llu %s\n", s, (మొత్తం_బైట్లు + 1023) / 1024, file);
	తిరిగి 1;
#నిర్వచనం_రద్దు buf
}

పూర్ణాంకం కూడిక_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం కూడిక_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	గుర్తులేని n;
	గుర్తులేని type = కూడిక_BSD;

	n = getopt32(argv, "sr");
	argv += optind;
	ఒకవేళ (n & 1) type = కూడిక_SYSV;
	/* give the bsd priority over sysv func */
	ఒకవేళ (n & 2) type = కూడిక_BSD;

	ఒకవేళ (!argv[0]) {
		/* Do not print the name */
		n = కూడిక_file("-", type);
	} లేకపోతే {
		/* Need to print the name if either
		 * - more than one file given
		 * - doing sysv */
		type += (argv[1] || type == కూడిక_SYSV);
		n = 1;
		చేయి {
			n &= కూడిక_file(*argv, type);
		} అయితే (*++argv);
	}
	తిరిగి !n;
}
