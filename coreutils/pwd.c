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

#அந்தர்பாவ "libbb.h"

நிலையான முழு logical_getcwd(வெறுமை)
{
	கட்டமைப்பு stat st1;
	கட்டமைப்பு stat st2;
	எழுத்து *wd;
	எழுத்து *p;

	wd = getenv("PWD");
	எனில் (!wd || wd[0] != '/')
		திரும்பு 0;

	p = wd;
	போது (*p) {
		/* doing strstr(p, "/.") by hand is smaller and faster... */
		எனில் (*p++ != '/')
			தொடர்;
		எனில் (*p != '.')
			தொடர்;
		/* we found "/.", skip to next char */
		p++;
		எனில் (*p == '.')
			p++; /* we found "/.." */
		எனில் (*p == '\0' || *p == '/')
			திரும்பு 0; /* "/./" or "/../" component: bad */
	}

	எனில் (stat(wd, &st1) != 0)
		திரும்பு 0;
	எனில் (stat(".", &st2) != 0)
		திரும்பு 0;
	எனில் (st1.st_ino != st2.st_ino)
		திரும்பு 0;
	எனில் (st1.st_dev != st2.st_dev)
		திரும்பு 0;

	puts(wd);
	திரும்பு 1;
}

முழு pwd_main(முழு argc, எழுத்து **argv) MAIN_EXTERNALLY_VISIBLE;
முழு pwd_main(முழு argc UNUSED_PARAM, எழுத்து **argv UNUSED_PARAM)
{
	எழுத்து *buf;

	எனில் (ENABLE_DESKTOP) {
		/* TODO: assume -L if $POSIXLY_CORRECT? (coreutils does that)
		 * Rationale:
		 * POSIX requires a default of -L, but most scripts expect -P
		 */
		unsigned opt = getopt32(argv, "LP");
		எனில் ((opt & 1) && logical_getcwd())
			திரும்பு fflush_all();
	}

	buf = xrealloc_getcwd_or_warn(வெற்று);

	எனில் (buf) {
		puts(buf);
		விடுவி(buf);
		திரும்பு fflush_all();
	}

	திரும்பு EXIT_FAILURE;
}
