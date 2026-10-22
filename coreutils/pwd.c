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

static ຈຳນວນເຕັມ logical_getcwd(void)
{
	sa_nracanaa stat st1;
	sa_nracanaa stat st2;
	var_na *wd;
	var_na *p;

	wd = getenv("PWD");
	ຖ້າ (!wd || wd[0] != '/')
		ສົ່ງຄືນ 0;

	p = wd;
	ໃນຂະນະທີ່ (*p) {
		/* doing strstr(p, "/.") by hand is smaller and faster... */
		ຖ້າ (*p++ != '/')
			continue;
		ຖ້າ (*p != '.')
			continue;
		/* we found "/.", skip to next char */
		p++;
		ຖ້າ (*p == '.')
			p++; /* we found "/.." */
		ຖ້າ (*p == '\0' || *p == '/')
			ສົ່ງຄືນ 0; /* "/./" or "/../" component: bad */
	}

	ຖ້າ (stat(wd, &st1) != 0)
		ສົ່ງຄືນ 0;
	ຖ້າ (stat(".", &st2) != 0)
		ສົ່ງຄືນ 0;
	ຖ້າ (st1.st_ino != st2.st_ino)
		ສົ່ງຄືນ 0;
	ຖ້າ (st1.st_dev != st2.st_dev)
		ສົ່ງຄືນ 0;

	puts(wd);
	ສົ່ງຄືນ 1;
}

ຈຳນວນເຕັມ pwd_main(ຈຳນວນເຕັມ argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
ຈຳນວນເຕັມ pwd_main(ຈຳນວນເຕັມ argc UNUSED_PARAM, var_na **argv UNUSED_PARAM)
{
	var_na *buf;

	ຖ້າ (ENABLE_DESKTOP) {
		/* TODO: assume -L if $POSIXLY_CORRECT? (coreutils does that)
		 * Rationale:
		 * POSIX requires a default of -L, but most scripts expect -P
		 */
		unsigned opt = getopt32(argv, "LP");
		ຖ້າ ((opt & 1) && logical_getcwd())
			ສົ່ງຄືນ fflush_all();
	}

	buf = xrealloc_getcwd_or_warn(NULL);

	ຖ້າ (buf) {
		puts(buf);
		free(buf);
		ສົ່ງຄືນ fflush_all();
	}

	ສົ່ງຄືນ EXIT_FAILURE;
}
