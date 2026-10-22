/* vi: set sw=4 ts=4: */
/*
 * Mini pwd implementation for busybox
 *
 * Copyright (C) 1995, 1996 by Bruce Perens <bruce@pixar.com>.
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config PWD
//config:	bool "pwd (4 kb)"
//config:	default y
//config:	help
//config:	pwd is used to print the current directory.

//applet:IF_PWD(APPLET_NOFORK(pwd, pwd, BB_DIR_BIN, BB_SUID_DROP, pwd))

//kbuild:lib-$(CONFIG_PWD) += pwd.o

//usage:#define pwd_trivial_usage
//usage:       ""
//usage:#define pwd_full_usage "\n\n"
//usage:       "Print the full filename of the current working directory"
//usage:
//usage:#define pwd_example_usage
//usage:       "$ pwd\n"
//usage:       "/root\n"

#অন্তর্ভাব "libbb.h"

স্থির পূর্ণসংখ্যা logical_getcwd(শূন্য)
{
	কাঠামো stat st1;
	কাঠামো stat st2;
	অক্ষর *wd;
	অক্ষর *p;

	wd = getenv("PWD");
	যদি (!wd || wd[0] != '/')
		ফেরত 0;

	p = wd;
	যখন (*p) {
		/* doing strstr(p, "/.") by hand is smaller and faster... */
		যদি (*p++ != '/')
			চালু;
		যদি (*p != '.')
			চালু;
		/* we found "/.", skip to next char */
		p++;
		যদি (*p == '.')
			p++; /* we found "/.." */
		যদি (*p == '\0' || *p == '/')
			ফেরত 0; /* "/./" or "/../" component: bad */
	}

	যদি (stat(wd, &st1) != 0)
		ফেরত 0;
	যদি (stat(".", &st2) != 0)
		ফেরত 0;
	যদি (st1.st_ino != st2.st_ino)
		ফেরত 0;
	যদি (st1.st_dev != st2.st_dev)
		ফেরত 0;

	puts(wd);
	ফেরত 1;
}

পূর্ণসংখ্যা pwd_main(পূর্ণসংখ্যা argc, অক্ষর **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণসংখ্যা pwd_main(পূর্ণসংখ্যা argc UNUSED_PARAM, অক্ষর **argv UNUSED_PARAM)
{
	অক্ষর *buf;

	যদি (ENABLE_DESKTOP) {
		/* TODO: assume -L if $POSIXLY_CORRECT? (coreutils does that)
		 * Rationale:
		 * POSIX requires a default of -L, but most scripts expect -P
		 */
		unsigned opt = getopt32(argv, "LP");
		যদি ((opt & 1) && logical_getcwd())
			ফেরত fflush_all();
	}

	buf = xrealloc_getcwd_or_warn(ফাঁকা);

	যদি (buf) {
		puts(buf);
		মুক্ত(buf);
		ফেরত fflush_all();
	}

	ফেরত EXIT_FAILURE;
}
