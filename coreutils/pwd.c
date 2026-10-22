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

static පූර්ණ_සංඛ්‍යා logical_getcwd(void)
{
	sa_nracanaa stat st1;
	sa_nracanaa stat st2;
	var_na *wd;
	var_na *p;

	wd = getenv("PWD");
	නම් (!wd || wd[0] != '/')
		ආපසු 0;

	p = wd;
	අතරතුර (*p) {
		/* doing strstr(p, "/.") by hand is smaller and faster... */
		නම් (*p++ != '/')
			continue;
		නම් (*p != '.')
			continue;
		/* we found "/.", skip to next char */
		p++;
		නම් (*p == '.')
			p++; /* we found "/.." */
		නම් (*p == '\0' || *p == '/')
			ආපසු 0; /* "/./" or "/../" component: bad */
	}

	නම් (stat(wd, &st1) != 0)
		ආපසු 0;
	නම් (stat(".", &st2) != 0)
		ආපසු 0;
	නම් (st1.st_ino != st2.st_ino)
		ආපසු 0;
	නම් (st1.st_dev != st2.st_dev)
		ආපසු 0;

	puts(wd);
	ආපසු 1;
}

පූර්ණ_සංඛ්‍යා pwd_main(පූර්ණ_සංඛ්‍යා argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
පූර්ණ_සංඛ්‍යා pwd_main(පූර්ණ_සංඛ්‍යා argc UNUSED_PARAM, var_na **argv UNUSED_PARAM)
{
	var_na *buf;

	නම් (ENABLE_DESKTOP) {
		/* TODO: assume -L if $POSIXLY_CORRECT? (coreutils does that)
		 * Rationale:
		 * POSIX requires a default of -L, but most scripts expect -P
		 */
		unsigned opt = getopt32(argv, "LP");
		නම් ((opt & 1) && logical_getcwd())
			ආපසු fflush_all();
	}

	buf = xrealloc_getcwd_or_warn(NULL);

	නම් (buf) {
		puts(buf);
		free(buf);
		ආපසු fflush_all();
	}

	ආපසු EXIT_FAILURE;
}
