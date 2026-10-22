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

#ଅନ୍ତର୍ଭାଵ "libbb.h"

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା logical_getcwd(ଶୂନ୍ୟ)
{
	ସଂରଚନା stat st1;
	ସଂରଚନା stat st2;
	ଅକ୍ଷର *wd;
	ଅକ୍ଷର *p;

	wd = getenv("PWD");
	ଯଦି (!wd || wd[0] != '/')
		ଫେରନ୍ତୁ 0;

	p = wd;
	ଯେତେବେଳେ (*p) {
		/* doing strstr(p, "/.") by hand is smaller and faster... */
		ଯଦି (*p++ != '/')
			ଜାରି;
		ଯଦି (*p != '.')
			ଜାରି;
		/* we found "/.", skip to next char */
		p++;
		ଯଦି (*p == '.')
			p++; /* we found "/.." */
		ଯଦି (*p == '\0' || *p == '/')
			ଫେରନ୍ତୁ 0; /* "/./" or "/../" component: bad */
	}

	ଯଦି (stat(wd, &st1) != 0)
		ଫେରନ୍ତୁ 0;
	ଯଦି (stat(".", &st2) != 0)
		ଫେରନ୍ତୁ 0;
	ଯଦି (st1.st_ino != st2.st_ino)
		ଫେରନ୍ତୁ 0;
	ଯଦି (st1.st_dev != st2.st_dev)
		ଫେରନ୍ତୁ 0;

	puts(wd);
	ଫେରନ୍ତୁ 1;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା pwd_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv) MAIN_EXTERNALLY_VISIBLE;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା pwd_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc UNUSED_PARAM, ଅକ୍ଷର **argv UNUSED_PARAM)
{
	ଅକ୍ଷର *buf;

	ଯଦି (ENABLE_DESKTOP) {
		/* TODO: assume -L if $POSIXLY_CORRECT? (coreutils does that)
		 * Rationale:
		 * POSIX requires a default of -L, but most scripts expect -P
		 */
		unsigned opt = getopt32(argv, "LP");
		ଯଦି ((opt & 1) && logical_getcwd())
			ଫେରନ୍ତୁ fflush_all();
	}

	buf = xrealloc_getcwd_or_warn(ଖାଲି);

	ଯଦି (buf) {
		puts(buf);
		ମୁକ୍ତ(buf);
		ଫେରନ୍ତୁ fflush_all();
	}

	ଫେରନ୍ତୁ EXIT_FAILURE;
}
