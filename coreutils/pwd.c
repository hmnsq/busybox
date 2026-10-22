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

#समावेश "libbb.h"

अटल पूर्णांक logical_getcwd(व्योम)
{
	काष्ठा stat st1;
	काष्ठा stat st2;
	अक्षर *wd;
	अक्षर *p;

	wd = दो_पर्या("PWD");
	अगर (!wd || wd[0] != '/')
		वापस 0;

	p = wd;
	जबतक (*p) {
		/* doing strstr(p, "/.") by hand is smaller and faster... */
		अगर (*p++ != '/')
			जारी;
		अगर (*p != '.')
			जारी;
		/* we found "/.", skip to next char */
		p++;
		अगर (*p == '.')
			p++; /* we found "/.." */
		अगर (*p == '\0' || *p == '/')
			वापस 0; /* "/./" or "/../" component: bad */
	}

	अगर (stat(wd, &st1) != 0)
		वापस 0;
	अगर (stat(".", &st2) != 0)
		वापस 0;
	अगर (st1.st_ino != st2.st_ino)
		वापस 0;
	अगर (st1.st_dev != st2.st_dev)
		वापस 0;

	माला_दो(wd);
	वापस 1;
}

पूर्णांक pwd_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक pwd_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv UNUSED_PARAM)
{
	अक्षर *buf;

	अगर (ENABLE_DESKTOP) {
		/* TODO: assume -L if $POSIXLY_CORRECT? (coreutils does that)
		 * Rationale:
		 * POSIX requires a default of -L, but most scripts expect -P
		 */
		अचिन्हित opt = getopt32(argv, "LP");
		अगर ((opt & 1) && logical_getcwd())
			वापस fflush_all();
	}

	buf = xrealloc_getcwd_or_warn(शून्य);

	अगर (buf) {
		माला_दो(buf);
		मुक्त(buf);
		वापस fflush_all();
	}

	वापस निकास_त्रुटि;
}
