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

#شامل "libbb.h"
#شامل "common_bufsiz.h"

شمار { مجموع_BSD, PRINT_NAME, مجموع_SYSV };

/* BSD: calculate and print the rotated checksum and the size in 1K blocks
   The checksum varies depending on sizeof (int). */
/* SYSV: calculate and print the checksum and the size in 512-byte blocks */
/* Return 1 if successful.  */
ساکن غیرعلامتی مجموع_file(ثابت حرف *file, غیرعلامتی type)
{
	غیرعلامتی طویل طویل کل_بائٹیں = 0;
	صحیح fd, r;
	/* The sum of all the input bytes, modulo (UINT_MAX + 1).  */
	غیرعلامتی s = 0;

#تعریف_کرو buf bb_common_bufsiz1
	تیاری_common_bufsiz();

	fd = open_or_warn_stdin(file);
	اگر (fd == -1)
		واپس 0;

	جب (1) {
		سائز_t بائٹیں_read = محفوظ_read(fd, buf, COMMON_BUFSIZE);

		اگر ((ssize_t)بائٹیں_read <= 0) {
			r = (fd && close(fd) != 0);
			اگر (!بائٹیں_read && !r)
				/* no error */
				توڑو;
			bb_simple_perror_پیغام(file);
			واپس 0;
		}

		کل_بائٹیں += بائٹیں_read;
		اگر (type >= مجموع_SYSV) {
			کرو s += buf[--بائٹیں_read]; جب (بائٹیں_read);
		} ورنہ {
			r = 0;
			کرو {
				s = (s >> 1) + ((s & 1) << 15);
				s += buf[r++];
				s &= 0xffff; /* Keep it within bounds. */
			} جب (--بائٹیں_read);
		}
	}

	اگر (type < PRINT_NAME)
		file = "";
	اگر (type >= مجموع_SYSV) {
		r = (s & 0xffff) + ((s & 0xffffffff) >> 16);
		s = (r & 0xffff) + (r >> 16);
		لکھو("%u %llu %s\n", s, (کل_بائٹیں + 511) / 512, file);
	} ورنہ
		لکھو("%05u %5llu %s\n", s, (کل_بائٹیں + 1023) / 1024, file);
	واپس 1;
#تعریف_منسوخ buf
}

صحیح مجموع_main(صحیح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح مجموع_main(صحیح argc غیر_مستعمل_PARAM, حرف **argv)
{
	غیرعلامتی n;
	غیرعلامتی type = مجموع_BSD;

	n = getopt32(argv, "sr");
	argv += optind;
	اگر (n & 1) type = مجموع_SYSV;
	/* give the bsd priority over sysv func */
	اگر (n & 2) type = مجموع_BSD;

	اگر (!argv[0]) {
		/* Do not print the name */
		n = مجموع_file("-", type);
	} ورنہ {
		/* Need to print the name if either
		 * - more than one file given
		 * - doing sysv */
		type += (argv[1] || type == مجموع_SYSV);
		n = 1;
		کرو {
			n &= مجموع_file(*argv, type);
		} جب (*++argv);
	}
	واپس !n;
}
