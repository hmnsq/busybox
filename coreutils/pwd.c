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

	wd = दो_पर्या("PWD");
	यदि (!wd || wd[0] != '/')
		फर्काऊ 0;

	p = wd;
	जबसम्म (*p) {
		/* doing strstr(p, "/.") by hand is smaller and faster... */
		यदि (*p++ != '/')
			जारी;
		यदि (*p != '.')
			जारी;
		/* we found "/.", skip to next char */
		p++;
		यदि (*p == '.')
			p++; /* we found "/.." */
		यदि (*p == '\0' || *p == '/')
			फर्काऊ 0; /* "/./" or "/../" component: bad */
	}

	यदि (stat(wd, &st1) != 0)
		फर्काऊ 0;
	यदि (stat(".", &st2) != 0)
		फर्काऊ 0;
	यदि (st1.st_ino != st2.st_ino)
		फर्काऊ 0;
	यदि (st1.st_dev != st2.st_dev)
		फर्काऊ 0;

	माला_दो(wd);
	फर्काऊ 1;
}

पूर्णाङ्क pwd_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क pwd_main(पूर्णाङ्क argc UNUSED_PARAM, वर्ण **argv UNUSED_PARAM)
{
	वर्ण *buf;

	यदि (ENABLE_DESKTOP) {
		/* TODO: assume -L if $POSIXLY_CORRECT? (coreutils does that)
		 * Rationale:
		 * POSIX requires a default of -L, but most scripts expect -P
		 */
		अचिन्हित opt = getopt32(argv, "LP");
		यदि ((opt & 1) && logical_getcwd())
			फर्काऊ fflush_all();
	}

	buf = xrealloc_getcwd_or_warn(रिक्त);

	यदि (buf) {
		माला_दो(buf);
		मुक्त(buf);
		फर्काऊ fflush_all();
	}

	फर्काऊ निकास_त्रुटि;
}
