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

ساکن صحیح logical_getcwd(خالی)
{
	ساخت stat st1;
	ساخت stat st2;
	حرف *wd;
	حرف *p;

	wd = getenv("PWD");
	اگر (!wd || wd[0] != '/')
		واپس 0;

	p = wd;
	جب (*p) {
		/* doing strstr(p, "/.") by hand is smaller and faster... */
		اگر (*p++ != '/')
			جاری;
		اگر (*p != '.')
			جاری;
		/* we found "/.", skip to next char */
		p++;
		اگر (*p == '.')
			p++; /* we found "/.." */
		اگر (*p == '\0' || *p == '/')
			واپس 0; /* "/./" or "/../" component: bad */
	}

	اگر (stat(wd, &st1) != 0)
		واپس 0;
	اگر (stat(".", &st2) != 0)
		واپس 0;
	اگر (st1.st_ino != st2.st_ino)
		واپس 0;
	اگر (st1.st_dev != st2.st_dev)
		واپس 0;

	puts(wd);
	واپس 1;
}

صحیح pwd_main(صحیح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح pwd_main(صحیح argc غیر_مستعمل_PARAM, حرف **argv غیر_مستعمل_PARAM)
{
	حرف *buf;

	اگر (ENABLE_DESKTOP) {
		/* TODO: assume -L if $POSIXLY_CORRECT? (coreutils does that)
		 * Rationale:
		 * POSIX requires a default of -L, but most scripts expect -P
		 */
		غیرعلامتی opt = getopt32(argv, "LP");
		اگر ((opt & 1) && logical_getcwd())
			واپس fflush_all();
	}

	buf = xrealloc_getcwd_or_warn(تہی);

	اگر (buf) {
		puts(buf);
		آزاد(buf);
		واپس fflush_all();
	}

	واپس EXIT_FAILURE;
}
