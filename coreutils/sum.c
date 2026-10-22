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

#ضمن "libbb.h"
#ضمن "common_bufsiz.h"

تعداد { مجموعه_BSD, PRINT_NAME, مجموعه_SYSV };

/* BSD: calculate and print the rotated checksum and the size in 1K blocks
   The checksum varies depending on sizeof (int). */
/* SYSV: calculate and print the checksum and the size in 512-byte blocks */
/* Return 1 if successful.  */
ساكن بینښه مجموعه_file(ثابت حرف *file, بینښه type)
{
	بینښه اوږد اوږد ټول_بایټونه = 0;
	صحيح fd, r;
	/* The sum of all the input bytes, modulo (UINT_MAX + 1).  */
	بینښه s = 0;

#تعریف_کړه buf bb_common_bufsiz1
	تیاری_common_bufsiz();

	fd = open_or_warn_stdin(file);
	إذا (fd == -1)
		أرجع 0;

	طالما (1) {
		اندازه_t بایټونه_read = خوندي_read(fd, buf, COMMON_BUFSIZE);

		إذا ((ssize_t)بایټونه_read <= 0) {
			r = (fd && close(fd) != 0);
			إذا (!بایټونه_read && !r)
				/* no error */
				اكسر;
			bb_simple_perror_پیغام(file);
			أرجع 0;
		}

		ټول_بایټونه += بایټونه_read;
		إذا (type >= مجموعه_SYSV) {
			افعل s += buf[--بایټونه_read]; طالما (بایټونه_read);
		} وإلا {
			r = 0;
			افعل {
				s = (s >> 1) + ((s & 1) << 15);
				s += buf[r++];
				s &= 0xffff; /* Keep it within bounds. */
			} طالما (--بایټونه_read);
		}
	}

	إذا (type < PRINT_NAME)
		file = "";
	إذا (type >= مجموعه_SYSV) {
		r = (s & 0xffff) + ((s & 0xffffffff) >> 16);
		s = (r & 0xffff) + (r >> 16);
		چاپ_کړه("%u %llu %s\n", s, (ټول_بایټونه + 511) / 512, file);
	} وإلا
		چاپ_کړه("%05u %5llu %s\n", s, (ټول_بایټونه + 1023) / 1024, file);
	أرجع 1;
#تعریف_لغوه buf
}

صحيح مجموعه_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح مجموعه_main(صحيح argc ناکارول_شوی_PARAM, حرف **argv)
{
	بینښه n;
	بینښه type = مجموعه_BSD;

	n = getopt32(argv, "sr");
	argv += optind;
	إذا (n & 1) type = مجموعه_SYSV;
	/* give the bsd priority over sysv func */
	إذا (n & 2) type = مجموعه_BSD;

	إذا (!argv[0]) {
		/* Do not print the name */
		n = مجموعه_file("-", type);
	} وإلا {
		/* Need to print the name if either
		 * - more than one file given
		 * - doing sysv */
		type += (argv[1] || type == مجموعه_SYSV);
		n = 1;
		افعل {
			n &= مجموعه_file(*argv, type);
		} طالما (*++argv);
	}
	أرجع !n;
}
