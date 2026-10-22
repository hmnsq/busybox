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

#شامل "libbb.h"

/* This is a NOFORK applet. Be very careful! */

صحیح printenv_main(صحیح argc, نویسه **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح printenv_main(صحیح argc UNUSED_PARAM, نویسه **argv)
{
	صحیح exit_code = EXIT_SUCCESS;

	/* no variables specified, show whole env */
	اگر (!argv[1]) {
		نویسه **e = environ;

		/* environ can be NULL! (for example, after clearenv())
		 * Check for that:
		 */
		اگر (e)
			وقتی (*e)
				puts(*e++);
	} وگرنه {
		/* search for specified variables and print them out if found */
		نویسه *arg, *env;

		وقتی ((arg = *++argv) != پوچ) {
			env = getenv(arg);
			اگر (env)
				puts(env);
			وگرنه
				exit_code = EXIT_FAILURE;
		}
	}

	fflush_stdout_and_exit(exit_code);
}
