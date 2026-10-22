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

#include "libbb.h"

/* This is a NOFORK applet. Be very careful! */

ཧྲིལ printenv_main(ཧྲིལ argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
ཧྲིལ printenv_main(ཧྲིལ argc UNUSED_PARAM, var_na **argv)
{
	ཧྲིལ exit_code = EXIT_SUCCESS;

	/* no variables specified, show whole env */
	གལ་ཏེ (!argv[1]) {
		var_na **e = environ;

		/* environ can be NULL! (for example, after clearenv())
		 * Check for that:
		 */
		གལ་ཏེ (e)
			སྐབས (*e)
				puts(*e++);
	} དེ་མིན {
		/* search for specified variables and print them out if found */
		var_na *arg, *env;

		སྐབས ((arg = *++argv) != NULL) {
			env = getenv(arg);
			གལ་ཏེ (env)
				puts(env);
			དེ་མིན
				exit_code = EXIT_FAILURE;
		}
	}

	fflush_stdout_and_exit(exit_code);
}
