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

static ကိန်းပြည့် logical_getcwd(void)
{
	sa_nracanaa stat st1;
	sa_nracanaa stat st2;
	var_na *wd;
	var_na *p;

	wd = getenv("PWD");
	အကယ်၍ (!wd || wd[0] != '/')
		ပြန်ပေး 0;

	p = wd;
	နေစဉ် (*p) {
		/* doing strstr(p, "/.") by hand is smaller and faster... */
		အကယ်၍ (*p++ != '/')
			continue;
		အကယ်၍ (*p != '.')
			continue;
		/* we found "/.", skip to next char */
		p++;
		အကယ်၍ (*p == '.')
			p++; /* we found "/.." */
		အကယ်၍ (*p == '\0' || *p == '/')
			ပြန်ပေး 0; /* "/./" or "/../" component: bad */
	}

	အကယ်၍ (stat(wd, &st1) != 0)
		ပြန်ပေး 0;
	အကယ်၍ (stat(".", &st2) != 0)
		ပြန်ပေး 0;
	အကယ်၍ (st1.st_ino != st2.st_ino)
		ပြန်ပေး 0;
	အကယ်၍ (st1.st_dev != st2.st_dev)
		ပြန်ပေး 0;

	puts(wd);
	ပြန်ပေး 1;
}

ကိန်းပြည့် pwd_main(ကိန်းပြည့် argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
ကိန်းပြည့် pwd_main(ကိန်းပြည့် argc UNUSED_PARAM, var_na **argv UNUSED_PARAM)
{
	var_na *buf;

	အကယ်၍ (ENABLE_DESKTOP) {
		/* TODO: assume -L if $POSIXLY_CORRECT? (coreutils does that)
		 * Rationale:
		 * POSIX requires a default of -L, but most scripts expect -P
		 */
		unsigned opt = getopt32(argv, "LP");
		အကယ်၍ ((opt & 1) && logical_getcwd())
			ပြန်ပေး fflush_all();
	}

	buf = xrealloc_getcwd_or_warn(NULL);

	အကယ်၍ (buf) {
		puts(buf);
		free(buf);
		ပြန်ပေး fflush_all();
	}

	ပြန်ပေး EXIT_FAILURE;
}
