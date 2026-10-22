/* vi: set sw=4 ts=4: */
/*
 * printenv implementation for busybox
 *
 * Copyright (C) 2005 by Erik Andersen <andersen@codepoet.org>
 * Copyright (C) 2005 by Mike Frysinger <vapier@gentoo.org>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config PRINTENV
//config:	bool "printenv (1.6 kb)"
//config:	default y
//config:	help
//config:	printenv is used to print all or part of environment.

//applet:IF_PRINTENV(APPLET_NOFORK(printenv, printenv, BB_DIR_BIN, BB_SUID_DROP, printenv))

//kbuild:lib-$(CONFIG_PRINTENV) += printenv.o

//usage:#define printenv_trivial_usage
//usage:       "[VARIABLE]..."
//usage:#define printenv_full_usage "\n\n"
//usage:       "Print environment VARIABLEs.\n"
//usage:       "If no VARIABLE specified, print all."

#अन्तर्भाव "libbb.h"

/* This is a NOFORK applet. Be very careful! */

पूर्णाङ्क printenv_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क printenv_main(पूर्णाङ्क argc UNUSED_PARAM, वर्ण **argv)
{
	पूर्णाङ्क exit_code = निकास_सफल;

	/* no variables specified, show whole env */
	यदि (!argv[1]) {
		वर्ण **e = environ;

		/* environ can be NULL! (for example, after clearenv())
		 * Check for that:
		 */
		यदि (e)
			जबसम्म (*e)
				माला_दो(*e++);
	} अन्यथा {
		/* search for specified variables and print them out if found */
		वर्ण *arg, *env;

		जबसम्म ((arg = *++argv) != रिक्त) {
			env = दो_पर्या(arg);
			यदि (env)
				माला_दो(env);
			अन्यथा
				exit_code = निकास_त्रुटि;
		}
	}

	fflush_stdout_and_exit(exit_code);
}
