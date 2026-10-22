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

#അന്തര്ഭാവ "libbb.h"
#അന്തര്ഭാവ "common_bufsiz.h"

എണ്ണൽ { SUM_BSD, PRINT_NAME, SUM_SYSV };

/* BSD: calculate and print the rotated checksum and the size in 1K blocks
   The checksum varies depending on sizeof (int). */
/* SYSV: calculate and print the checksum and the size in 512-byte blocks */
/* Return 1 if successful.  */
സ്ഥിതം unsigned sum_file(സ്ഥിരം അക്ഷരം *file, unsigned type)
{
	unsigned long long total_bytes = 0;
	പൂർണ്ണസംഖ്യ fd, r;
	/* The sum of all the input bytes, modulo (UINT_MAX + 1).  */
	unsigned s = 0;

#define buf bb_common_bufsiz1
	setup_common_bufsiz();

	fd = open_or_warn_stdin(file);
	എങ്കിൽ (fd == -1)
		തിരികെ 0;

	ആകുമ്പോൾ (1) {
		size_t bytes_read = safe_read(fd, buf, COMMON_BUFSIZE);

		എങ്കിൽ ((ssize_t)bytes_read <= 0) {
			r = (fd && close(fd) != 0);
			എങ്കിൽ (!bytes_read && !r)
				/* no error */
				നിർത്തുക;
			bb_simple_perror_സന്ദേശം(file);
			തിരികെ 0;
		}

		total_bytes += bytes_read;
		എങ്കിൽ (type >= SUM_SYSV) {
			ചെയ്യുക s += buf[--bytes_read]; ആകുമ്പോൾ (bytes_read);
		} അല്ലെങ്കിൽ {
			r = 0;
			ചെയ്യുക {
				s = (s >> 1) + ((s & 1) << 15);
				s += buf[r++];
				s &= 0xffff; /* Keep it within bounds. */
			} ആകുമ്പോൾ (--bytes_read);
		}
	}

	എങ്കിൽ (type < PRINT_NAME)
		file = "";
	എങ്കിൽ (type >= SUM_SYSV) {
		r = (s & 0xffff) + ((s & 0xffffffff) >> 16);
		s = (r & 0xffff) + (r >> 16);
		printf("%u %llu %s\n", s, (total_bytes + 511) / 512, file);
	} അല്ലെങ്കിൽ
		printf("%05u %5llu %s\n", s, (total_bytes + 1023) / 1024, file);
	തിരികെ 1;
#undef buf
}

പൂർണ്ണസംഖ്യ sum_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) MAIN_EXTERNALLY_VISIBLE;
പൂർണ്ണസംഖ്യ sum_main(പൂർണ്ണസംഖ്യ argc UNUSED_PARAM, അക്ഷരം **argv)
{
	unsigned n;
	unsigned type = SUM_BSD;

	n = getopt32(argv, "sr");
	argv += optind;
	എങ്കിൽ (n & 1) type = SUM_SYSV;
	/* give the bsd priority over sysv func */
	എങ്കിൽ (n & 2) type = SUM_BSD;

	എങ്കിൽ (!argv[0]) {
		/* Do not print the name */
		n = sum_file("-", type);
	} അല്ലെങ്കിൽ {
		/* Need to print the name if either
		 * - more than one file given
		 * - doing sysv */
		type += (argv[1] || type == SUM_SYSV);
		n = 1;
		ചെയ്യുക {
			n &= sum_file(*argv, type);
		} ആകുമ്പോൾ (*++argv);
	}
	തിരികെ !n;
}
