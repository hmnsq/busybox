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

#समावेश "libbb.h"
#समावेश "common_bufsiz.h"

क्रमागत { योग_BSD, PRINT_NAME, योग_SYSV };

/* BSD: calculate and print the rotated checksum and the size in 1K blocks
   The checksum varies depending on sizeof (int). */
/* SYSV: calculate and print the checksum and the size in 512-byte blocks */
/* Return 1 if successful.  */
अटल अचिन्हित योग_file(स्थिर अक्षर *file, अचिन्हित type)
{
	अचिन्हित दीर्घ दीर्घ योग_bytes = 0;
	पूर्णांक fd, r;
	/* The sum of all the input bytes, modulo (UINT_MAX + 1).  */
	अचिन्हित s = 0;

#घोषणा buf bb_common_bufsiz1
	setup_common_bufsiz();

	fd = open_or_warn_stdin(file);
	अगर (fd == -1)
		वापस 0;

	जबतक (1) {
		माप_प्रकार bytes_read = safe_read(fd, buf, COMMON_BUFSIZE);

		अगर ((ssize_t)bytes_read <= 0) {
			r = (fd && बंद(fd) != 0);
			अगर (!bytes_read && !r)
				/* no error */
				अवरोध;
			bb_simple_perror_संदेश(file);
			वापस 0;
		}

		योग_bytes += bytes_read;
		अगर (type >= योग_SYSV) {
			करो s += buf[--bytes_read]; जबतक (bytes_read);
		} अन्यथा {
			r = 0;
			करो {
				s = (s >> 1) + ((s & 1) << 15);
				s += buf[r++];
				s &= 0xffff; /* Keep it within bounds. */
			} जबतक (--bytes_read);
		}
	}

	अगर (type < PRINT_NAME)
		file = "";
	अगर (type >= योग_SYSV) {
		r = (s & 0xffff) + ((s & 0xffffffff) >> 16);
		s = (r & 0xffff) + (r >> 16);
		म_लिखो("%u %llu %s\n", s, (योग_bytes + 511) / 512, file);
	} अन्यथा
		म_लिखो("%05u %5llu %s\n", s, (योग_bytes + 1023) / 1024, file);
	वापस 1;
#अघोषित buf
}

पूर्णांक योग_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक योग_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	अचिन्हित n;
	अचिन्हित type = योग_BSD;

	n = getopt32(argv, "sr");
	argv += optind;
	अगर (n & 1) type = योग_SYSV;
	/* give the bsd priority over sysv func */
	अगर (n & 2) type = योग_BSD;

	अगर (!argv[0]) {
		/* Do not print the name */
		n = योग_file("-", type);
	} अन्यथा {
		/* Need to print the name if either
		 * - more than one file given
		 * - doing sysv */
		type += (argv[1] || type == योग_SYSV);
		n = 1;
		करो {
			n &= योग_file(*argv, type);
		} जबतक (*++argv);
	}
	वापस !n;
}
