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

#شامل "libbb.h"

ایستا صحیح logical_getcwd(تهی)
{
	ساختار stat st1;
	ساختار stat st2;
	نویسه *wd;
	نویسه *p;

	wd = getenv("PWD");
	اگر (!wd || wd[0] != '/')
		بازگشت 0;

	p = wd;
	وقتی (*p) {
		/* doing strstr(p, "/.") by hand is smaller and faster... */
		اگر (*p++ != '/')
			ادامه;
		اگر (*p != '.')
			ادامه;
		/* we found "/.", skip to next char */
		p++;
		اگر (*p == '.')
			p++; /* we found "/.." */
		اگر (*p == '\0' || *p == '/')
			بازگشت 0; /* "/./" or "/../" component: bad */
	}

	اگر (stat(wd, &st1) != 0)
		بازگشت 0;
	اگر (stat(".", &st2) != 0)
		بازگشت 0;
	اگر (st1.st_ino != st2.st_ino)
		بازگشت 0;
	اگر (st1.st_dev != st2.st_dev)
		بازگشت 0;

	puts(wd);
	بازگشت 1;
}

صحیح pwd_main(صحیح argc, نویسه **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح pwd_main(صحیح argc UNUSED_PARAM, نویسه **argv UNUSED_PARAM)
{
	نویسه *buf;

	اگر (ENABLE_DESKTOP) {
		/* TODO: assume -L if $POSIXLY_CORRECT? (coreutils does that)
		 * Rationale:
		 * POSIX requires a default of -L, but most scripts expect -P
		 */
		unsigned opt = getopt32(argv, "LP");
		اگر ((opt & 1) && logical_getcwd())
			بازگشت fflush_all();
	}

	buf = xrealloc_getcwd_or_warn(پوچ);

	اگر (buf) {
		puts(buf);
		آزاد(buf);
		بازگشت fflush_all();
	}

	بازگشت EXIT_FAILURE;
}
