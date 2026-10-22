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

गणना { योग_BSD, PRINT_NAME, योग_SYSV };

/* BSD: calculate and print the rotated checksum and the size in 1K blocks
   The checksum varies depending on sizeof (int). */
/* SYSV: calculate and print the checksum and the size in 512-byte blocks */
/* Return 1 if successful.  */
स्थिर अचिह्नित योग_file(ध्रुव वर्ण *file, अचिह्नित type)
{
	अचिह्नित दीर्घ दीर्घ योग_अष्टकानि = 0;
	पूर्णाङ्क fd, r;
	/* The sum of all the input bytes, modulo (UINT_MAX + 1).  */
	अचिह्नित s = 0;

#परिभाषय buf bb_common_bufsiz1
	सज्जता_common_bufsiz();

	fd = open_or_warn_stdin(file);
	यदि (fd == -1)
		प्रत्यागम 0;

	यावत् (1) {
		आकार_t अष्टकानि_read = safe_read(fd, buf, COMMON_BUFSIZE);

		यदि ((ssize_t)अष्टकानि_read <= 0) {
			r = (fd && close(fd) != 0);
			यदि (!अष्टकानि_read && !r)
				/* no error */
				भङ्ग;
			bb_simple_perror_सन्देशः(file);
			प्रत्यागम 0;
		}

		योग_अष्टकानि += अष्टकानि_read;
		यदि (type >= योग_SYSV) {
			कुरु s += buf[--अष्टकानि_read]; यावत् (अष्टकानि_read);
		} अन्यथा {
			r = 0;
			कुरु {
				s = (s >> 1) + ((s & 1) << 15);
				s += buf[r++];
				s &= 0xffff; /* Keep it within bounds. */
			} यावत् (--अष्टकानि_read);
		}
	}

	यदि (type < PRINT_NAME)
		file = "";
	यदि (type >= योग_SYSV) {
		r = (s & 0xffff) + ((s & 0xffffffff) >> 16);
		s = (r & 0xffff) + (r >> 16);
		प्रारूप_लिख("%u %llu %s\n", s, (योग_अष्टकानि + 511) / 512, file);
	} अन्यथा
		प्रारूप_लिख("%05u %5llu %s\n", s, (योग_अष्टकानि + 1023) / 1024, file);
	प्रत्यागम 1;
#अपरिभाषय buf
}

पूर्णाङ्क योग_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क योग_main(पूर्णाङ्क argc अप्रयुक्तम्_PARAM, वर्ण **argv)
{
	अचिह्नित n;
	अचिह्नित type = योग_BSD;

	n = getopt32(argv, "sr");
	argv += optind;
	यदि (n & 1) type = योग_SYSV;
	/* give the bsd priority over sysv func */
	यदि (n & 2) type = योग_BSD;

	यदि (!argv[0]) {
		/* Do not print the name */
		n = योग_file("-", type);
	} अन्यथा {
		/* Need to print the name if either
		 * - more than one file given
		 * - doing sysv */
		type += (argv[1] || type == योग_SYSV);
		n = 1;
		कुरु {
			n &= योग_file(*argv, type);
		} यावत् (*++argv);
	}
	प्रत्यागम !n;
}
