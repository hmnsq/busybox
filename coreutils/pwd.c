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

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ logical_getcwd(ಶೂನ್ಯ)
{
	ರಚನೆ stat st1;
	ರಚನೆ stat st2;
	ಅಕ್ಷರ *wd;
	ಅಕ್ಷರ *p;

	wd = getenv("PWD");
	ಆದರೆ (!wd || wd[0] != '/')
		ಹಿಂತಿರುಗಿ 0;

	p = wd;
	ಆಗ (*p) {
		/* doing strstr(p, "/.") by hand is smaller and faster... */
		ಆದರೆ (*p++ != '/')
			ಮುಂದುವರಿ;
		ಆದರೆ (*p != '.')
			ಮುಂದುವರಿ;
		/* we found "/.", skip to next char */
		p++;
		ಆದರೆ (*p == '.')
			p++; /* we found "/.." */
		ಆದರೆ (*p == '\0' || *p == '/')
			ಹಿಂತಿರುಗಿ 0; /* "/./" or "/../" component: bad */
	}

	ಆದರೆ (stat(wd, &st1) != 0)
		ಹಿಂತಿರುಗಿ 0;
	ಆದರೆ (stat(".", &st2) != 0)
		ಹಿಂತಿರುಗಿ 0;
	ಆದರೆ (st1.st_ino != st2.st_ino)
		ಹಿಂತಿರುಗಿ 0;
	ಆದರೆ (st1.st_dev != st2.st_dev)
		ಹಿಂತಿರುಗಿ 0;

	puts(wd);
	ಹಿಂತಿರುಗಿ 1;
}

ಪೂರ್ಣಾಂಕ pwd_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ pwd_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv ಬಳಸದ_PARAM)
{
	ಅಕ್ಷರ *buf;

	ಆದರೆ (ENABLE_DESKTOP) {
		/* TODO: assume -L if $POSIXLY_CORRECT? (coreutils does that)
		 * Rationale:
		 * POSIX requires a default of -L, but most scripts expect -P
		 */
		ಚಿಹ್ನೆರಹಿತ opt = getopt32(argv, "LP");
		ಆದರೆ ((opt & 1) && logical_getcwd())
			ಹಿಂತಿರುಗಿ fflush_all();
	}

	buf = xrealloc_getcwd_or_warn(ಖಾಲಿ);

	ಆದರೆ (buf) {
		puts(buf);
		ಬಿಡುಗಡೆ(buf);
		ಹಿಂತಿರುಗಿ fflush_all();
	}

	ಹಿಂತಿರುಗಿ EXIT_FAILURE;
}
