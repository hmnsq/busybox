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
	ᰀᰧ (!wd || wd[0] != '/')
		ᰀᰧᰌ 0;

	p = wd;
	ᰀᰬ (*p) {
		/* doing strstr(p, "/.") by hand is smaller and faster... */
		ᰀᰧ (*p++ != '/')
			continue;
		ᰀᰧ (*p != '.')
			continue;
		/* we found "/.", skip to next char */
		p++;
		ᰀᰧ (*p == '.')
			p++; /* we found "/.." */
		ᰀᰧ (*p == '\0' || *p == '/')
			ᰀᰧᰌ 0; /* "/./" or "/../" component: bad */
	}

	ᰀᰧ (stat(wd, &st1) != 0)
		ᰀᰧᰌ 0;
	ᰀᰧ (stat(".", &st2) != 0)
		ᰀᰧᰌ 0;
	ᰀᰧ (st1.st_ino != st2.st_ino)
		ᰀᰧᰌ 0;
	ᰀᰧ (st1.st_dev != st2.st_dev)
		ᰀᰧᰌ 0;

	puts(wd);
	ᰀᰧᰌ 1;
}

puur_naa_nka pwd_main(puur_naa_nka argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
puur_naa_nka pwd_main(puur_naa_nka argc UNUSED_PARAM, var_na **argv UNUSED_PARAM)
{
	var_na *buf;

	ᰀᰧ (ENABLE_DESKTOP) {
		/* TODO: assume -L if $POSIXLY_CORRECT? (coreutils does that)
		 * Rationale:
		 * POSIX requires a default of -L, but most scripts expect -P
		 */
		unsigned opt = getopt32(argv, "LP");
		ᰀᰧ ((opt & 1) && logical_getcwd())
			ᰀᰧᰌ fflush_all();
	}

	buf = xrealloc_getcwd_or_warn(NULL);

	ᰀᰧ (buf) {
		puts(buf);
		free(buf);
		ᰀᰧᰌ fflush_all();
	}

	ᰀᰧᰌ EXIT_FAILURE;
}
