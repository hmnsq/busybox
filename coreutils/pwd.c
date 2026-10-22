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

#include "libbb.h"

static จำนวนเต็ม logical_getcwd(void)
{
	sa_nracanaa stat st1;
	sa_nracanaa stat st2;
	var_na *wd;
	var_na *p;

	wd = getenv("PWD");
	ถ้า (!wd || wd[0] != '/')
		คืน 0;

	p = wd;
	ขณะ (*p) {
		/* doing strstr(p, "/.") by hand is smaller and faster... */
		ถ้า (*p++ != '/')
			continue;
		ถ้า (*p != '.')
			continue;
		/* we found "/.", skip to next char */
		p++;
		ถ้า (*p == '.')
			p++; /* we found "/.." */
		ถ้า (*p == '\0' || *p == '/')
			คืน 0; /* "/./" or "/../" component: bad */
	}

	ถ้า (stat(wd, &st1) != 0)
		คืน 0;
	ถ้า (stat(".", &st2) != 0)
		คืน 0;
	ถ้า (st1.st_ino != st2.st_ino)
		คืน 0;
	ถ้า (st1.st_dev != st2.st_dev)
		คืน 0;

	puts(wd);
	คืน 1;
}

จำนวนเต็ม pwd_main(จำนวนเต็ม argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
จำนวนเต็ม pwd_main(จำนวนเต็ม argc UNUSED_PARAM, var_na **argv UNUSED_PARAM)
{
	var_na *buf;

	ถ้า (ENABLE_DESKTOP) {
		/* TODO: assume -L if $POSIXLY_CORRECT? (coreutils does that)
		 * Rationale:
		 * POSIX requires a default of -L, but most scripts expect -P
		 */
		unsigned opt = getopt32(argv, "LP");
		ถ้า ((opt & 1) && logical_getcwd())
			คืน fflush_all();
	}

	buf = xrealloc_getcwd_or_warn(NULL);

	ถ้า (buf) {
		puts(buf);
		free(buf);
		คืน fflush_all();
	}

	คืน EXIT_FAILURE;
}
