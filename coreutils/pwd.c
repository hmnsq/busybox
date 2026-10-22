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

static ចំនួនគត់ logical_getcwd(void)
{
	sa_nracanaa stat st1;
	sa_nracanaa stat st2;
	var_na *wd;
	var_na *p;

	wd = getenv("PWD");
	ប្រសិនបើ (!wd || wd[0] != '/')
		ត្រឡប់ 0;

	p = wd;
	ខណៈពេល (*p) {
		/* doing strstr(p, "/.") by hand is smaller and faster... */
		ប្រសិនបើ (*p++ != '/')
			continue;
		ប្រសិនបើ (*p != '.')
			continue;
		/* we found "/.", skip to next char */
		p++;
		ប្រសិនបើ (*p == '.')
			p++; /* we found "/.." */
		ប្រសិនបើ (*p == '\0' || *p == '/')
			ត្រឡប់ 0; /* "/./" or "/../" component: bad */
	}

	ប្រសិនបើ (stat(wd, &st1) != 0)
		ត្រឡប់ 0;
	ប្រសិនបើ (stat(".", &st2) != 0)
		ត្រឡប់ 0;
	ប្រសិនបើ (st1.st_ino != st2.st_ino)
		ត្រឡប់ 0;
	ប្រសិនបើ (st1.st_dev != st2.st_dev)
		ត្រឡប់ 0;

	puts(wd);
	ត្រឡប់ 1;
}

ចំនួនគត់ pwd_main(ចំនួនគត់ argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
ចំនួនគត់ pwd_main(ចំនួនគត់ argc UNUSED_PARAM, var_na **argv UNUSED_PARAM)
{
	var_na *buf;

	ប្រសិនបើ (ENABLE_DESKTOP) {
		/* TODO: assume -L if $POSIXLY_CORRECT? (coreutils does that)
		 * Rationale:
		 * POSIX requires a default of -L, but most scripts expect -P
		 */
		unsigned opt = getopt32(argv, "LP");
		ប្រសិនបើ ((opt & 1) && logical_getcwd())
			ត្រឡប់ fflush_all();
	}

	buf = xrealloc_getcwd_or_warn(NULL);

	ប្រសិនបើ (buf) {
		puts(buf);
		free(buf);
		ត្រឡប់ fflush_all();
	}

	ត្រឡប់ EXIT_FAILURE;
}
