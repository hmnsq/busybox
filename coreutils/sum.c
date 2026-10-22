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

#अन्तर्भाव "libbb.h"
#अन्तर्भाव "common_bufsiz.h"

गणना { SUM_BSD, PRINT_NAME, SUM_SYSV };

/* BSD: calculate and print the rotated checksum and the size in 1K blocks
   The checksum varies depending on sizeof (int). */
/* SYSV: calculate and print the checksum and the size in 512-byte blocks */
/* Return 1 if successful.  */
स्थिर अचिन्हित sum_file(अचर वर्ण *file, अचिन्हित type)
{
	अचिन्हित दीर्घ दीर्घ total_bytes = 0;
	पूर्णाङ्क fd, r;
	/* The sum of all the input bytes, modulo (UINT_MAX + 1).  */
	अचिन्हित s = 0;

#define buf bb_common_bufsiz1
	setup_common_bufsiz();

	fd = open_or_warn_stdin(file);
	यदि (fd == -1)
		फर्काऊ 0;

	जबसम्म (1) {
		माप_प्रकार bytes_read = safe_read(fd, buf, COMMON_BUFSIZE);

		यदि ((ssize_t)bytes_read <= 0) {
			r = (fd && बंद(fd) != 0);
			यदि (!bytes_read && !r)
				/* no error */
				रोक;
			bb_simple_perror_सन्देश(file);
			फर्काऊ 0;
		}

		total_bytes += bytes_read;
		यदि (type >= SUM_SYSV) {
			गर s += buf[--bytes_read]; जबसम्म (bytes_read);
		} अन्यथा {
			r = 0;
			गर {
				s = (s >> 1) + ((s & 1) << 15);
				s += buf[r++];
				s &= 0xffff; /* Keep it within bounds. */
			} जबसम्म (--bytes_read);
		}
	}

	यदि (type < PRINT_NAME)
		file = "";
	यदि (type >= SUM_SYSV) {
		r = (s & 0xffff) + ((s & 0xffffffff) >> 16);
		s = (r & 0xffff) + (r >> 16);
		म_लिखो("%u %llu %s\n", s, (total_bytes + 511) / 512, file);
	} अन्यथा
		म_लिखो("%05u %5llu %s\n", s, (total_bytes + 1023) / 1024, file);
	फर्काऊ 1;
#undef buf
}

पूर्णाङ्क sum_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क sum_main(पूर्णाङ्क argc UNUSED_PARAM, वर्ण **argv)
{
	अचिन्हित n;
	अचिन्हित type = SUM_BSD;

	n = getopt32(argv, "sr");
	argv += optind;
	यदि (n & 1) type = SUM_SYSV;
	/* give the bsd priority over sysv func */
	यदि (n & 2) type = SUM_BSD;

	यदि (!argv[0]) {
		/* Do not print the name */
		n = sum_file("-", type);
	} अन्यथा {
		/* Need to print the name if either
		 * - more than one file given
		 * - doing sysv */
		type += (argv[1] || type == SUM_SYSV);
		n = 1;
		गर {
			n &= sum_file(*argv, type);
		} जबसम्म (*++argv);
	}
	फर्काऊ !n;
}
