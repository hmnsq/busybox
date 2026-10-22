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

স্থির পূর্ণাঙ্ক logical_getcwd(শূন্য)
{
	সংরচনা stat st1;
	সংরচনা stat st2;
	বর্ণ *wd;
	বর্ণ *p;

	wd = getenv("PWD");
	যদি (!wd || wd[0] != '/')
		প্রত্যাগম 0;

	p = wd;
	যাবত্ (*p) {
		/* doing strstr(p, "/.") by hand is smaller and faster... */
		যদি (*p++ != '/')
			অনুবর্ত;
		যদি (*p != '.')
			অনুবর্ত;
		/* we found "/.", skip to next char */
		p++;
		যদি (*p == '.')
			p++; /* we found "/.." */
		যদি (*p == '\0' || *p == '/')
			প্রত্যাগম 0; /* "/./" or "/../" component: bad */
	}

	যদি (stat(wd, &st1) != 0)
		প্রত্যাগম 0;
	যদি (stat(".", &st2) != 0)
		প্রত্যাগম 0;
	যদি (st1.st_ino != st2.st_ino)
		প্রত্যাগম 0;
	যদি (st1.st_dev != st2.st_dev)
		প্রত্যাগম 0;

	puts(wd);
	প্রত্যাগম 1;
}

পূর্ণাঙ্ক pwd_main(পূর্ণাঙ্ক argc, বর্ণ **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণাঙ্ক pwd_main(পূর্ণাঙ্ক argc UNUSED_PARAM, বর্ণ **argv UNUSED_PARAM)
{
	বর্ণ *buf;

	যদি (ENABLE_DESKTOP) {
		/* TODO: assume -L if $POSIXLY_CORRECT? (coreutils does that)
		 * Rationale:
		 * POSIX requires a default of -L, but most scripts expect -P
		 */
		unsigned opt = getopt32(argv, "LP");
		যদি ((opt & 1) && logical_getcwd())
			প্রত্যাগম fflush_all();
	}

	buf = xrealloc_getcwd_or_warn(রিক্ত);

	যদি (buf) {
		puts(buf);
		মুক্ত(buf);
		প্রত্যাগম fflush_all();
	}

	প্রত্যাগম EXIT_FAILURE;
}
