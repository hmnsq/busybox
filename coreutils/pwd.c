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

static số_nguyên logical_getcwd(void)
{
	struct stat st1;
	struct stat st2;
	char *wd;
	char *p;

	wd = getenv("PWD");
	nếu (!wd || wd[0] != '/')
		trả_về 0;

	p = wd;
	trong_khi (*p) {
		/* doing strstr(p, "/.") by hand is smaller and faster... */
		nếu (*p++ != '/')
			continue;
		nếu (*p != '.')
			continue;
		/* we found "/.", skip to next char */
		p++;
		nếu (*p == '.')
			p++; /* we found "/.." */
		nếu (*p == '\0' || *p == '/')
			trả_về 0; /* "/./" or "/../" component: bad */
	}

	nếu (stat(wd, &st1) != 0)
		trả_về 0;
	nếu (stat(".", &st2) != 0)
		trả_về 0;
	nếu (st1.st_ino != st2.st_ino)
		trả_về 0;
	nếu (st1.st_dev != st2.st_dev)
		trả_về 0;

	puts(wd);
	trả_về 1;
}

số_nguyên pwd_main(số_nguyên argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
số_nguyên pwd_main(số_nguyên argc UNUSED_PARAM, char **argv UNUSED_PARAM)
{
	char *buf;

	nếu (ENABLE_DESKTOP) {
		/* TODO: assume -L if $POSIXLY_CORRECT? (coreutils does that)
		 * Rationale:
		 * POSIX requires a default of -L, but most scripts expect -P
		 */
		unsigned opt = getopt32(argv, "LP");
		nếu ((opt & 1) && logical_getcwd())
			trả_về fflush_all();
	}

	buf = xrealloc_getcwd_or_warn(NULL);

	nếu (buf) {
		puts(buf);
		free(buf);
		trả_về fflush_all();
	}

	trả_về EXIT_FAILURE;
}
