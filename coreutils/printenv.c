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

#అన్తర్భావ "libbb.h"

/* This is a NOFORK applet. Be very careful! */

పూర్ణాంకం printenv_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం printenv_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	పూర్ణాంకం exit_సంకేతం = EXIT_SUCCESS;

	/* no variables specified, show whole env */
	ఒకవేళ (!argv[1]) {
		అక్షరం **e = environ;

		/* environ can be NULL! (for example, after clearenv())
		 * Check for that:
		 */
		ఒకవేళ (e)
			అయితే (*e)
				puts(*e++);
	} లేకపోతే {
		/* search for specified variables and print them out if found */
		అక్షరం *arg, *env;

		అయితే ((arg = *++argv) != ఖాళీ) {
			env = getenv(arg);
			ఒకవేళ (env)
				puts(env);
			లేకపోతే
				exit_సంకేతం = EXIT_FAILURE;
		}
	}

	fflush_stdout_and_exit(exit_సంకేతం);
}
