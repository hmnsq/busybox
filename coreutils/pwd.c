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

#అన్తర్భావ "libbb.h"

స్థిర పూర్ణాంకం logical_getcwd(శూన్య)
{
	నిర్మాణం stat st1;
	నిర్మాణం stat st2;
	అక్షరం *wd;
	అక్షరం *p;

	wd = getenv("PWD");
	ఒకవేళ (!wd || wd[0] != '/')
		తిరిగి 0;

	p = wd;
	అయితే (*p) {
		/* doing strstr(p, "/.") by hand is smaller and faster... */
		ఒకవేళ (*p++ != '/')
			కొనసాగు;
		ఒకవేళ (*p != '.')
			కొనసాగు;
		/* we found "/.", skip to next char */
		p++;
		ఒకవేళ (*p == '.')
			p++; /* we found "/.." */
		ఒకవేళ (*p == '\0' || *p == '/')
			తిరిగి 0; /* "/./" or "/../" component: bad */
	}

	ఒకవేళ (stat(wd, &st1) != 0)
		తిరిగి 0;
	ఒకవేళ (stat(".", &st2) != 0)
		తిరిగి 0;
	ఒకవేళ (st1.st_ino != st2.st_ino)
		తిరిగి 0;
	ఒకవేళ (st1.st_dev != st2.st_dev)
		తిరిగి 0;

	puts(wd);
	తిరిగి 1;
}

పూర్ణాంకం pwd_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం pwd_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv వాడని_PARAM)
{
	అక్షరం *buf;

	ఒకవేళ (ENABLE_DESKTOP) {
		/* TODO: assume -L if $POSIXLY_CORRECT? (coreutils does that)
		 * Rationale:
		 * POSIX requires a default of -L, but most scripts expect -P
		 */
		గుర్తులేని opt = getopt32(argv, "LP");
		ఒకవేళ ((opt & 1) && logical_getcwd())
			తిరిగి fflush_all();
	}

	buf = xrealloc_getcwd_or_warn(ఖాళీ);

	ఒకవేళ (buf) {
		puts(buf);
		విడుదల(buf);
		తిరిగి fflush_all();
	}

	తిరిగి EXIT_FAILURE;
}
