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

static цео logical_getcwd(void)
{
	struct stat st1;
	struct stat st2;
	char *wd;
	char *p;

	wd = getenv("PWD");
	ако (!wd || wd[0] != '/')
		врати 0;

	p = wd;
	док (*p) {
		/* doing strstr(p, "/.") by hand is smaller and faster... */
		ако (*p++ != '/')
			continue;
		ако (*p != '.')
			continue;
		/* we found "/.", skip to next char */
		p++;
		ако (*p == '.')
			p++; /* we found "/.." */
		ако (*p == '\0' || *p == '/')
			врати 0; /* "/./" or "/../" component: bad */
	}

	ако (stat(wd, &st1) != 0)
		врати 0;
	ако (stat(".", &st2) != 0)
		врати 0;
	ако (st1.st_ino != st2.st_ino)
		врати 0;
	ако (st1.st_dev != st2.st_dev)
		врати 0;

	puts(wd);
	врати 1;
}

цео pwd_main(цео argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
цео pwd_main(цео argc UNUSED_PARAM, char **argv UNUSED_PARAM)
{
	char *buf;

	ако (ENABLE_DESKTOP) {
		/* TODO: assume -L if $POSIXLY_CORRECT? (coreutils does that)
		 * Rationale:
		 * POSIX requires a default of -L, but most scripts expect -P
		 */
		unsigned opt = getopt32(argv, "LP");
		ако ((opt & 1) && logical_getcwd())
			врати fflush_all();
	}

	buf = xrealloc_getcwd_or_warn(NULL);

	ако (buf) {
		puts(buf);
		free(buf);
		врати fflush_all();
	}

	врати EXIT_FAILURE;
}
