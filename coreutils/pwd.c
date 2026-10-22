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

static ціле logical_getcwd(void)
{
	struct stat st1;
	struct stat st2;
	char *wd;
	char *p;

	wd = getenv("PWD");
	якщо (!wd || wd[0] != '/')
		повернути 0;

	p = wd;
	поки (*p) {
		/* doing strstr(p, "/.") by hand is smaller and faster... */
		якщо (*p++ != '/')
			continue;
		якщо (*p != '.')
			continue;
		/* we found "/.", skip to next char */
		p++;
		якщо (*p == '.')
			p++; /* we found "/.." */
		якщо (*p == '\0' || *p == '/')
			повернути 0; /* "/./" or "/../" component: bad */
	}

	якщо (stat(wd, &st1) != 0)
		повернути 0;
	якщо (stat(".", &st2) != 0)
		повернути 0;
	якщо (st1.st_ino != st2.st_ino)
		повернути 0;
	якщо (st1.st_dev != st2.st_dev)
		повернути 0;

	puts(wd);
	повернути 1;
}

ціле pwd_main(ціле argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
ціле pwd_main(ціле argc UNUSED_PARAM, char **argv UNUSED_PARAM)
{
	char *buf;

	якщо (ENABLE_DESKTOP) {
		/* TODO: assume -L if $POSIXLY_CORRECT? (coreutils does that)
		 * Rationale:
		 * POSIX requires a default of -L, but most scripts expect -P
		 */
		unsigned opt = getopt32(argv, "LP");
		якщо ((opt & 1) && logical_getcwd())
			повернути fflush_all();
	}

	buf = xrealloc_getcwd_or_warn(NULL);

	якщо (buf) {
		puts(buf);
		free(buf);
		повернути fflush_all();
	}

	повернути EXIT_FAILURE;
}
