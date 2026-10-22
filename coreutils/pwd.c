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

#അന്തര്ഭാവ "libbb.h"

സ്ഥിതം പൂർണ്ണസംഖ്യ logical_getcwd(ശൂന്യം)
{
	ഘടന stat st1;
	ഘടന stat st2;
	അക്ഷരം *wd;
	അക്ഷരം *p;

	wd = getenv("PWD");
	എങ്കിൽ (!wd || wd[0] != '/')
		തിരികെ 0;

	p = wd;
	ആകുമ്പോൾ (*p) {
		/* doing strstr(p, "/.") by hand is smaller and faster... */
		എങ്കിൽ (*p++ != '/')
			തുടരുക;
		എങ്കിൽ (*p != '.')
			തുടരുക;
		/* we found "/.", skip to next char */
		p++;
		എങ്കിൽ (*p == '.')
			p++; /* we found "/.." */
		എങ്കിൽ (*p == '\0' || *p == '/')
			തിരികെ 0; /* "/./" or "/../" component: bad */
	}

	എങ്കിൽ (stat(wd, &st1) != 0)
		തിരികെ 0;
	എങ്കിൽ (stat(".", &st2) != 0)
		തിരികെ 0;
	എങ്കിൽ (st1.st_ino != st2.st_ino)
		തിരികെ 0;
	എങ്കിൽ (st1.st_dev != st2.st_dev)
		തിരികെ 0;

	puts(wd);
	തിരികെ 1;
}

പൂർണ്ണസംഖ്യ pwd_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) MAIN_EXTERNALLY_VISIBLE;
പൂർണ്ണസംഖ്യ pwd_main(പൂർണ്ണസംഖ്യ argc UNUSED_PARAM, അക്ഷരം **argv UNUSED_PARAM)
{
	അക്ഷരം *buf;

	എങ്കിൽ (ENABLE_DESKTOP) {
		/* TODO: assume -L if $POSIXLY_CORRECT? (coreutils does that)
		 * Rationale:
		 * POSIX requires a default of -L, but most scripts expect -P
		 */
		unsigned opt = getopt32(argv, "LP");
		എങ്കിൽ ((opt & 1) && logical_getcwd())
			തിരികെ fflush_all();
	}

	buf = xrealloc_getcwd_or_warn(ശൂന്യ);

	എങ്കിൽ (buf) {
		puts(buf);
		സ്വതന്ത്രം(buf);
		തിരികെ fflush_all();
	}

	തിരികെ EXIT_FAILURE;
}
