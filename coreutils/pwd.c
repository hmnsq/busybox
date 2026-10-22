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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"

ਸਥਿਰ ਪੂਰਨ logical_getcwd(ਖਾਲੀ)
{
	ਬਣਤਰ stat st1;
	ਬਣਤਰ stat st2;
	ਅੱਖਰ *wd;
	ਅੱਖਰ *p;

	wd = getenv("PWD");
	ਜੇ (!wd || wd[0] != '/')
		ਵਾਪਸ 0;

	p = wd;
	ਜਦੋਂ (*p) {
		/* doing strstr(p, "/.") by hand is smaller and faster... */
		ਜੇ (*p++ != '/')
			ਜਾਰੀ;
		ਜੇ (*p != '.')
			ਜਾਰੀ;
		/* we found "/.", skip to next char */
		p++;
		ਜੇ (*p == '.')
			p++; /* we found "/.." */
		ਜੇ (*p == '\0' || *p == '/')
			ਵਾਪਸ 0; /* "/./" or "/../" component: bad */
	}

	ਜੇ (stat(wd, &st1) != 0)
		ਵਾਪਸ 0;
	ਜੇ (stat(".", &st2) != 0)
		ਵਾਪਸ 0;
	ਜੇ (st1.st_ino != st2.st_ino)
		ਵਾਪਸ 0;
	ਜੇ (st1.st_dev != st2.st_dev)
		ਵਾਪਸ 0;

	puts(wd);
	ਵਾਪਸ 1;
}

ਪੂਰਨ pwd_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ pwd_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv ਅਣਵਰਤਿਆ_PARAM)
{
	ਅੱਖਰ *buf;

	ਜੇ (ENABLE_DESKTOP) {
		/* TODO: assume -L if $POSIXLY_CORRECT? (coreutils does that)
		 * Rationale:
		 * POSIX requires a default of -L, but most scripts expect -P
		 */
		ਬਿਨਾਨਿਸ਼ਾਨੀ opt = getopt32(argv, "LP");
		ਜੇ ((opt & 1) && logical_getcwd())
			ਵਾਪਸ fflush_all();
	}

	buf = xrealloc_getcwd_or_warn(ਨਿਰਾ);

	ਜੇ (buf) {
		puts(buf);
		ਮੁਕਤ(buf);
		ਵਾਪਸ fflush_all();
	}

	ਵਾਪਸ EXIT_FAILURE;
}
