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
	ᮚᮦᮔ᮪ (!wd || wd[0] != '/')
		ᮝᮜᮤ 0;

	p = wd;
	ᮞᮨᮓᮨᮜ (*p) {
		/* doing strstr(p, "/.") by hand is smaller and faster... */
		ᮚᮦᮔ᮪ (*p++ != '/')
			continue;
		ᮚᮦᮔ᮪ (*p != '.')
			continue;
		/* we found "/.", skip to next char */
		p++;
		ᮚᮦᮔ᮪ (*p == '.')
			p++; /* we found "/.." */
		ᮚᮦᮔ᮪ (*p == '\0' || *p == '/')
			ᮝᮜᮤ 0; /* "/./" or "/../" component: bad */
	}

	ᮚᮦᮔ᮪ (stat(wd, &st1) != 0)
		ᮝᮜᮤ 0;
	ᮚᮦᮔ᮪ (stat(".", &st2) != 0)
		ᮝᮜᮤ 0;
	ᮚᮦᮔ᮪ (st1.st_ino != st2.st_ino)
		ᮝᮜᮤ 0;
	ᮚᮦᮔ᮪ (st1.st_dev != st2.st_dev)
		ᮝᮜᮤ 0;

	puts(wd);
	ᮝᮜᮤ 1;
}

puur_naa_nka pwd_main(puur_naa_nka argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
puur_naa_nka pwd_main(puur_naa_nka argc UNUSED_PARAM, var_na **argv UNUSED_PARAM)
{
	var_na *buf;

	ᮚᮦᮔ᮪ (ENABLE_DESKTOP) {
		/* TODO: assume -L if $POSIXLY_CORRECT? (coreutils does that)
		 * Rationale:
		 * POSIX requires a default of -L, but most scripts expect -P
		 */
		unsigned opt = getopt32(argv, "LP");
		ᮚᮦᮔ᮪ ((opt & 1) && logical_getcwd())
			ᮝᮜᮤ fflush_all();
	}

	buf = xrealloc_getcwd_or_warn(NULL);

	ᮚᮦᮔ᮪ (buf) {
		puts(buf);
		free(buf);
		ᮝᮜᮤ fflush_all();
	}

	ᮝᮜᮤ EXIT_FAILURE;
}
