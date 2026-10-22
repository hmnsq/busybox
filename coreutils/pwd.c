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

static puur_naa_nka logical_getcwd(void)
{
	sa_nracanaa stat st1;
	sa_nracanaa stat st2;
	var_na *wd;
	var_na *p;

	wd = getenv("PWD");
	ᤀᤡ᤺ (!wd || wd[0] != '/')
		ᤀᤡᤴ 0;

	p = wd;
	ᤀᤠ᤺ (*p) {
		/* doing strstr(p, "/.") by hand is smaller and faster... */
		ᤀᤡ᤺ (*p++ != '/')
			continue;
		ᤀᤡ᤺ (*p != '.')
			continue;
		/* we found "/.", skip to next char */
		p++;
		ᤀᤡ᤺ (*p == '.')
			p++; /* we found "/.." */
		ᤀᤡ᤺ (*p == '\0' || *p == '/')
			ᤀᤡᤴ 0; /* "/./" or "/../" component: bad */
	}

	ᤀᤡ᤺ (stat(wd, &st1) != 0)
		ᤀᤡᤴ 0;
	ᤀᤡ᤺ (stat(".", &st2) != 0)
		ᤀᤡᤴ 0;
	ᤀᤡ᤺ (st1.st_ino != st2.st_ino)
		ᤀᤡᤴ 0;
	ᤀᤡ᤺ (st1.st_dev != st2.st_dev)
		ᤀᤡᤴ 0;

	puts(wd);
	ᤀᤡᤴ 1;
}

puur_naa_nka pwd_main(puur_naa_nka argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
puur_naa_nka pwd_main(puur_naa_nka argc UNUSED_PARAM, var_na **argv UNUSED_PARAM)
{
	var_na *buf;

	ᤀᤡ᤺ (ENABLE_DESKTOP) {
		/* TODO: assume -L if $POSIXLY_CORRECT? (coreutils does that)
		 * Rationale:
		 * POSIX requires a default of -L, but most scripts expect -P
		 */
		unsigned opt = getopt32(argv, "LP");
		ᤀᤡ᤺ ((opt & 1) && logical_getcwd())
			ᤀᤡᤴ fflush_all();
	}

	buf = xrealloc_getcwd_or_warn(NULL);

	ᤀᤡ᤺ (buf) {
		puts(buf);
		free(buf);
		ᤀᤡᤴ fflush_all();
	}

	ᤀᤡᤴ EXIT_FAILURE;
}
