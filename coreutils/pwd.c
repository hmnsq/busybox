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

#ضمن "libbb.h"

ساكن صحيح logical_getcwd(فراغ)
{
	بنية stat st1;
	بنية stat st2;
	حرف *wd;
	حرف *p;

	wd = getenv("PWD");
	إذا (!wd || wd[0] != '/')
		أرجع 0;

	p = wd;
	طالما (*p) {
		/* doing strstr(p, "/.") by hand is smaller and faster... */
		إذا (*p++ != '/')
			استمر;
		إذا (*p != '.')
			استمر;
		/* we found "/.", skip to next char */
		p++;
		إذا (*p == '.')
			p++; /* we found "/.." */
		إذا (*p == '\0' || *p == '/')
			أرجع 0; /* "/./" or "/../" component: bad */
	}

	إذا (stat(wd, &st1) != 0)
		أرجع 0;
	إذا (stat(".", &st2) != 0)
		أرجع 0;
	إذا (st1.st_ino != st2.st_ino)
		أرجع 0;
	إذا (st1.st_dev != st2.st_dev)
		أرجع 0;

	puts(wd);
	أرجع 1;
}

صحيح pwd_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح pwd_main(صحيح argc بن_ورتیا_PARAM, حرف **argv بن_ورتیا_PARAM)
{
	حرف *buf;

	إذا (ENABLE_DESKTOP) {
		/* TODO: assume -L if $POSIXLY_CORRECT? (coreutils does that)
		 * Rationale:
		 * POSIX requires a default of -L, but most scripts expect -P
		 */
		بنانشانی opt = getopt32(argv, "LP");
		إذا ((opt & 1) && logical_getcwd())
			أرجع fflush_all();
	}

	buf = xrealloc_getcwd_or_warn(عدم);

	إذا (buf) {
		puts(buf);
		حرر(buf);
		أرجع fflush_all();
	}

	أرجع EXIT_FAILURE;
}
