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

static 整数 logical_getcwd(void)
{
	struct stat st1;
	struct stat st2;
	char *wd;
	char *p;

	wd = getenv("PWD");
	もし (!wd || wd[0] != '/')
		戻る 0;

	p = wd;
	ながら (*p) {
		/* doing strstr(p, "/.") by hand is smaller and faster... */
		もし (*p++ != '/')
			continue;
		もし (*p != '.')
			continue;
		/* we found "/.", skip to next char */
		p++;
		もし (*p == '.')
			p++; /* we found "/.." */
		もし (*p == '\0' || *p == '/')
			戻る 0; /* "/./" or "/../" component: bad */
	}

	もし (stat(wd, &st1) != 0)
		戻る 0;
	もし (stat(".", &st2) != 0)
		戻る 0;
	もし (st1.st_ino != st2.st_ino)
		戻る 0;
	もし (st1.st_dev != st2.st_dev)
		戻る 0;

	puts(wd);
	戻る 1;
}

整数 pwd_main(整数 argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
整数 pwd_main(整数 argc UNUSED_PARAM, char **argv UNUSED_PARAM)
{
	char *buf;

	もし (ENABLE_DESKTOP) {
		/* TODO: assume -L if $POSIXLY_CORRECT? (coreutils does that)
		 * Rationale:
		 * POSIX requires a default of -L, but most scripts expect -P
		 */
		unsigned opt = getopt32(argv, "LP");
		もし ((opt & 1) && logical_getcwd())
			戻る fflush_all();
	}

	buf = xrealloc_getcwd_or_warn(NULL);

	もし (buf) {
		puts(buf);
		free(buf);
		戻る fflush_all();
	}

	戻る EXIT_FAILURE;
}
