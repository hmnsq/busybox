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

static 정수 logical_getcwd(void)
{
	struct stat st1;
	struct stat st2;
	char *wd;
	char *p;

	wd = getenv("PWD");
	만약 (!wd || wd[0] != '/')
		반환 0;

	p = wd;
	동안 (*p) {
		/* doing strstr(p, "/.") by hand is smaller and faster... */
		만약 (*p++ != '/')
			continue;
		만약 (*p != '.')
			continue;
		/* we found "/.", skip to next char */
		p++;
		만약 (*p == '.')
			p++; /* we found "/.." */
		만약 (*p == '\0' || *p == '/')
			반환 0; /* "/./" or "/../" component: bad */
	}

	만약 (stat(wd, &st1) != 0)
		반환 0;
	만약 (stat(".", &st2) != 0)
		반환 0;
	만약 (st1.st_ino != st2.st_ino)
		반환 0;
	만약 (st1.st_dev != st2.st_dev)
		반환 0;

	puts(wd);
	반환 1;
}

정수 pwd_main(정수 argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
정수 pwd_main(정수 argc UNUSED_PARAM, char **argv UNUSED_PARAM)
{
	char *buf;

	만약 (ENABLE_DESKTOP) {
		/* TODO: assume -L if $POSIXLY_CORRECT? (coreutils does that)
		 * Rationale:
		 * POSIX requires a default of -L, but most scripts expect -P
		 */
		unsigned opt = getopt32(argv, "LP");
		만약 ((opt & 1) && logical_getcwd())
			반환 fflush_all();
	}

	buf = xrealloc_getcwd_or_warn(NULL);

	만약 (buf) {
		puts(buf);
		free(buf);
		반환 fflush_all();
	}

	반환 EXIT_FAILURE;
}
