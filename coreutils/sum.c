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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "common_bufsiz.h"

ਗਿਣਤੀ { ਜੋੜ_BSD, PRINT_NAME, ਜੋੜ_SYSV };

/* BSD: calculate and print the rotated checksum and the size in 1K blocks
   The checksum varies depending on sizeof (int). */
/* SYSV: calculate and print the checksum and the size in 512-byte blocks */
/* Return 1 if successful.  */
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਜੋੜ_file(ਅਚਲ ਅੱਖਰ *file, ਬਿਨਾਨਿਸ਼ਾਨੀ type)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ ਕੁੱਲ_ਬਾਈਟਾਂ = 0;
	ਪੂਰਨ fd, r;
	/* The sum of all the input bytes, modulo (UINT_MAX + 1).  */
	ਬਿਨਾਨਿਸ਼ਾਨੀ s = 0;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ buf bb_common_bufsiz1
	ਤਿਆਰੀ_common_bufsiz();

	fd = open_or_warn_stdin(file);
	ਜੇ (fd == -1)
		ਵਾਪਸ 0;

	ਜਦੋਂ (1) {
		ਮਾਪ_t ਬਾਈਟਾਂ_read = ਸੁਰੱਖਿਅਤ_read(fd, buf, COMMON_BUFSIZE);

		ਜੇ ((ssize_t)ਬਾਈਟਾਂ_read <= 0) {
			r = (fd && close(fd) != 0);
			ਜੇ (!ਬਾਈਟਾਂ_read && !r)
				/* no error */
				ਰੋਕੋ;
			bb_simple_perror_ਸੁਨੇਹਾ(file);
			ਵਾਪਸ 0;
		}

		ਕੁੱਲ_ਬਾਈਟਾਂ += ਬਾਈਟਾਂ_read;
		ਜੇ (type >= ਜੋੜ_SYSV) {
			ਕਰੋ s += buf[--ਬਾਈਟਾਂ_read]; ਜਦੋਂ (ਬਾਈਟਾਂ_read);
		} ਨਹੀਂਤਾਂ {
			r = 0;
			ਕਰੋ {
				s = (s >> 1) + ((s & 1) << 15);
				s += buf[r++];
				s &= 0xffff; /* Keep it within bounds. */
			} ਜਦੋਂ (--ਬਾਈਟਾਂ_read);
		}
	}

	ਜੇ (type < PRINT_NAME)
		file = "";
	ਜੇ (type >= ਜੋੜ_SYSV) {
		r = (s & 0xffff) + ((s & 0xffffffff) >> 16);
		s = (r & 0xffff) + (r >> 16);
		ਲਿਖੋ("%u %llu %s\n", s, (ਕੁੱਲ_ਬਾਈਟਾਂ + 511) / 512, file);
	} ਨਹੀਂਤਾਂ
		ਲਿਖੋ("%05u %5llu %s\n", s, (ਕੁੱਲ_ਬਾਈਟਾਂ + 1023) / 1024, file);
	ਵਾਪਸ 1;
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ buf
}

ਪੂਰਨ ਜੋੜ_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ ਜੋੜ_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ n;
	ਬਿਨਾਨਿਸ਼ਾਨੀ type = ਜੋੜ_BSD;

	n = getopt32(argv, "sr");
	argv += optind;
	ਜੇ (n & 1) type = ਜੋੜ_SYSV;
	/* give the bsd priority over sysv func */
	ਜੇ (n & 2) type = ਜੋੜ_BSD;

	ਜੇ (!argv[0]) {
		/* Do not print the name */
		n = ਜੋੜ_file("-", type);
	} ਨਹੀਂਤਾਂ {
		/* Need to print the name if either
		 * - more than one file given
		 * - doing sysv */
		type += (argv[1] || type == ਜੋੜ_SYSV);
		n = 1;
		ਕਰੋ {
			n &= ਜੋੜ_file(*argv, type);
		} ਜਦੋਂ (*++argv);
	}
	ਵਾਪਸ !n;
}
