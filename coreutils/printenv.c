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

#ضمن "libbb.h"

/* This is a NOFORK applet. Be very careful! */

صحيح printenv_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح printenv_main(صحيح argc UNUSED_PARAM, حرف **argv)
{
	صحيح exit_code = EXIT_SUCCESS;

	/* no variables specified, show whole env */
	𐡄𐡍 (!argv[1]) {
		حرف **e = environ;

		/* environ can be NULL! (for example, after clearenv())
		 * Check for that:
		 */
		𐡄𐡍 (e)
			طالما (*e)
				puts(*e++);
	} وإلا {
		/* search for specified variables and print them out if found */
		حرف *arg, *env;

		طالما ((arg = *++argv) != عدم) {
			env = getenv(arg);
			𐡄𐡍 (env)
				puts(env);
			وإلا
				exit_code = EXIT_FAILURE;
		}
	}

	fflush_stdout_and_exit(exit_code);
}
