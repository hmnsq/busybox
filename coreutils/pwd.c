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

#अन्तर्भाव "libbb.h"

स्थिर पूर्णाङ्क logical_getcwd(शून्य)
{
	संरचना stat st1;
	संरचना stat st2;
	वर्ण *wd;
	वर्ण *p;

	wd = getenv("PWD");
	जइ (!wd || wd[0] != '/')
		प्रत्यागम 0;

	p = wd;
	जाव (*p) {
		/* doing strstr(p, "/.") by hand is smaller and faster... */
		जइ (*p++ != '/')
			अनुवर्त;
		जइ (*p != '.')
			अनुवर्त;
		/* we found "/.", skip to next char */
		p++;
		जइ (*p == '.')
			p++; /* we found "/.." */
		जइ (*p == '\0' || *p == '/')
			प्रत्यागम 0; /* "/./" or "/../" component: bad */
	}

	जइ (stat(wd, &st1) != 0)
		प्रत्यागम 0;
	जइ (stat(".", &st2) != 0)
		प्रत्यागम 0;
	जइ (st1.st_ino != st2.st_ino)
		प्रत्यागम 0;
	जइ (st1.st_dev != st2.st_dev)
		प्रत्यागम 0;

	puts(wd);
	प्रत्यागम 1;
}

पूर्णाङ्क pwd_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क pwd_main(पूर्णाङ्क argc UNUSED_PARAM, वर्ण **argv UNUSED_PARAM)
{
	वर्ण *buf;

	जइ (ENABLE_DESKTOP) {
		/* TODO: assume -L if $POSIXLY_CORRECT? (coreutils does that)
		 * Rationale:
		 * POSIX requires a default of -L, but most scripts expect -P
		 */
		unsigned opt = getopt32(argv, "LP");
		जइ ((opt & 1) && logical_getcwd())
			प्रत्यागम fflush_all();
	}

	buf = xrealloc_getcwd_or_warn(रिक्त);

	जइ (buf) {
		puts(buf);
		मुक्त(buf);
		प्रत्यागम fflush_all();
	}

	प्रत्यागम EXIT_FAILURE;
}
