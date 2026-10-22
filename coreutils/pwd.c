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

#અન્તર્ભાવ "libbb.h"

સ્થિર પૂર્ણાંક logical_getcwd(શૂન્ય)
{
	સંરચના stat st1;
	સંરચના stat st2;
	અક્ષર *wd;
	અક્ષર *p;

	wd = getenv("PWD");
	જો (!wd || wd[0] != '/')
		પાછા 0;

	p = wd;
	જ્યારે (*p) {
		/* doing strstr(p, "/.") by hand is smaller and faster... */
		જો (*p++ != '/')
			ચાલુ;
		જો (*p != '.')
			ચાલુ;
		/* we found "/.", skip to next char */
		p++;
		જો (*p == '.')
			p++; /* we found "/.." */
		જો (*p == '\0' || *p == '/')
			પાછા 0; /* "/./" or "/../" component: bad */
	}

	જો (stat(wd, &st1) != 0)
		પાછા 0;
	જો (stat(".", &st2) != 0)
		પાછા 0;
	જો (st1.st_ino != st2.st_ino)
		પાછા 0;
	જો (st1.st_dev != st2.st_dev)
		પાછા 0;

	puts(wd);
	પાછા 1;
}

પૂર્ણાંક pwd_main(પૂર્ણાંક argc, અક્ષર **argv) MAIN_EXTERNALLY_VISIBLE;
પૂર્ણાંક pwd_main(પૂર્ણાંક argc UNUSED_PARAM, અક્ષર **argv UNUSED_PARAM)
{
	અક્ષર *buf;

	જો (ENABLE_DESKTOP) {
		/* TODO: assume -L if $POSIXLY_CORRECT? (coreutils does that)
		 * Rationale:
		 * POSIX requires a default of -L, but most scripts expect -P
		 */
		unsigned opt = getopt32(argv, "LP");
		જો ((opt & 1) && logical_getcwd())
			પાછા fflush_all();
	}

	buf = xrealloc_getcwd_or_warn(ખાલી);

	જો (buf) {
		puts(buf);
		મુક્ત(buf);
		પાછા fflush_all();
	}

	પાછા EXIT_FAILURE;
}
