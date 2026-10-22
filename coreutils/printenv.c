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

#ಅನ್ತರ್ಭಾವ "libbb.h"

/* This is a NOFORK applet. Be very careful! */

ಪೂರ್ಣಾಙ್ಕ printenv_main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಙ್ಕ printenv_main(ಪೂರ್ಣಾಙ್ಕ argc UNUSED_PARAM, ವರ್ಣ **argv)
{
	ಪೂರ್ಣಾಙ್ಕ exit_code = EXIT_SUCCESS;

	/* no variables specified, show whole env */
	ಯದಿ (!argv[1]) {
		ವರ್ಣ **e = environ;

		/* environ can be NULL! (for example, after clearenv())
		 * Check for that:
		 */
		ಯದಿ (e)
			ಯಾವತ್ (*e)
				puts(*e++);
	} ಅನ್ಯಥಾ {
		/* search for specified variables and print them out if found */
		ವರ್ಣ *arg, *env;

		ಯಾವತ್ ((arg = *++argv) != ರಿಕ್ತ) {
			env = getenv(arg);
			ಯದಿ (env)
				puts(env);
			ಅನ್ಯಥಾ
				exit_code = EXIT_FAILURE;
		}
	}

	fflush_stdout_and_exit(exit_code);
}
