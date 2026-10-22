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

static ཧྲིལ logical_getcwd(void)
{
	sa_nracanaa stat st1;
	sa_nracanaa stat st2;
	var_na *wd;
	var_na *p;

	wd = getenv("PWD");
	གལ་ཏེ (!wd || wd[0] != '/')
		ཕྱིར 0;

	p = wd;
	སྐབས (*p) {
		/* doing strstr(p, "/.") by hand is smaller and faster... */
		གལ་ཏེ (*p++ != '/')
			continue;
		གལ་ཏེ (*p != '.')
			continue;
		/* we found "/.", skip to next char */
		p++;
		གལ་ཏེ (*p == '.')
			p++; /* we found "/.." */
		གལ་ཏེ (*p == '\0' || *p == '/')
			ཕྱིར 0; /* "/./" or "/../" component: bad */
	}

	གལ་ཏེ (stat(wd, &st1) != 0)
		ཕྱིར 0;
	གལ་ཏེ (stat(".", &st2) != 0)
		ཕྱིར 0;
	གལ་ཏེ (st1.st_ino != st2.st_ino)
		ཕྱིར 0;
	གལ་ཏེ (st1.st_dev != st2.st_dev)
		ཕྱིར 0;

	puts(wd);
	ཕྱིར 1;
}

ཧྲིལ pwd_main(ཧྲིལ argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
ཧྲིལ pwd_main(ཧྲིལ argc UNUSED_PARAM, var_na **argv UNUSED_PARAM)
{
	var_na *buf;

	གལ་ཏེ (ENABLE_DESKTOP) {
		/* TODO: assume -L if $POSIXLY_CORRECT? (coreutils does that)
		 * Rationale:
		 * POSIX requires a default of -L, but most scripts expect -P
		 */
		unsigned opt = getopt32(argv, "LP");
		གལ་ཏེ ((opt & 1) && logical_getcwd())
			ཕྱིར fflush_all();
	}

	buf = xrealloc_getcwd_or_warn(NULL);

	གལ་ཏེ (buf) {
		puts(buf);
		free(buf);
		ཕྱིར fflush_all();
	}

	ཕྱིར EXIT_FAILURE;
}
