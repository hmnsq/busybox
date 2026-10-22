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

स्थिर पूर्णांक logical_getcwd(शून्य)
{
	संरचना stat st1;
	संरचना stat st2;
	अक्षर *wd;
	अक्षर *p;

	wd = दो_पर्या("PWD");
	जर (!wd || wd[0] != '/')
		परत 0;

	p = wd;
	पर्यंत (*p) {
		/* doing strstr(p, "/.") by hand is smaller and faster... */
		जर (*p++ != '/')
			सुरूठेवा;
		जर (*p != '.')
			सुरूठेवा;
		/* we found "/.", skip to next char */
		p++;
		जर (*p == '.')
			p++; /* we found "/.." */
		जर (*p == '\0' || *p == '/')
			परत 0; /* "/./" or "/../" component: bad */
	}

	जर (stat(wd, &st1) != 0)
		परत 0;
	जर (stat(".", &st2) != 0)
		परत 0;
	जर (st1.st_ino != st2.st_ino)
		परत 0;
	जर (st1.st_dev != st2.st_dev)
		परत 0;

	माला_दो(wd);
	परत 1;
}

पूर्णांक pwd_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक pwd_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv UNUSED_PARAM)
{
	अक्षर *buf;

	जर (ENABLE_DESKTOP) {
		/* TODO: assume -L if $POSIXLY_CORRECT? (coreutils does that)
		 * Rationale:
		 * POSIX requires a default of -L, but most scripts expect -P
		 */
		अचिन्हित opt = getopt32(argv, "LP");
		जर ((opt & 1) && logical_getcwd())
			परत fflush_all();
	}

	buf = xrealloc_getcwd_or_warn(रिक्त);

	जर (buf) {
		माला_दो(buf);
		मुक्त(buf);
		परत fflush_all();
	}

	परत निकास_त्रुटि;
}
