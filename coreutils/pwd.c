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

#ಅನ್ತರ್ಭಾವ "libbb.h"

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ logical_getcwd(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ stat st1;
	ಸಂರಚನಾ stat st2;
	ವರ್ಣ *wd;
	ವರ್ಣ *p;

	wd = getenv("PWD");
	ಯದಿ (!wd || wd[0] != '/')
		ಪ್ರತ್ಯಾಗಮ 0;

	p = wd;
	ಯಾವತ್ (*p) {
		/* doing strstr(p, "/.") by hand is smaller and faster... */
		ಯದಿ (*p++ != '/')
			ಅನುವರ್ತ;
		ಯದಿ (*p != '.')
			ಅನುವರ್ತ;
		/* we found "/.", skip to next char */
		p++;
		ಯದಿ (*p == '.')
			p++; /* we found "/.." */
		ಯದಿ (*p == '\0' || *p == '/')
			ಪ್ರತ್ಯಾಗಮ 0; /* "/./" or "/../" component: bad */
	}

	ಯದಿ (stat(wd, &st1) != 0)
		ಪ್ರತ್ಯಾಗಮ 0;
	ಯದಿ (stat(".", &st2) != 0)
		ಪ್ರತ್ಯಾಗಮ 0;
	ಯದಿ (st1.st_ino != st2.st_ino)
		ಪ್ರತ್ಯಾಗಮ 0;
	ಯದಿ (st1.st_dev != st2.st_dev)
		ಪ್ರತ್ಯಾಗಮ 0;

	puts(wd);
	ಪ್ರತ್ಯಾಗಮ 1;
}

ಪೂರ್ಣಾಙ್ಕ pwd_main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಙ್ಕ pwd_main(ಪೂರ್ಣಾಙ್ಕ argc UNUSED_PARAM, ವರ್ಣ **argv UNUSED_PARAM)
{
	ವರ್ಣ *buf;

	ಯದಿ (ENABLE_DESKTOP) {
		/* TODO: assume -L if $POSIXLY_CORRECT? (coreutils does that)
		 * Rationale:
		 * POSIX requires a default of -L, but most scripts expect -P
		 */
		unsigned opt = getopt32(argv, "LP");
		ಯದಿ ((opt & 1) && logical_getcwd())
			ಪ್ರತ್ಯಾಗಮ fflush_all();
	}

	buf = xrealloc_getcwd_or_warn(ರಿಕ್ತ);

	ಯದಿ (buf) {
		puts(buf);
		ಮುಕ್ತ(buf);
		ಪ್ರತ್ಯಾಗಮ fflush_all();
	}

	ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;
}
