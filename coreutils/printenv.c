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

puur_naa_nka printenv_main(puur_naa_nka argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
puur_naa_nka printenv_main(puur_naa_nka argc UNUSED_PARAM, var_na **argv)
{
	puur_naa_nka exit_code = EXIT_SUCCESS;

	/* no variables specified, show whole env */
	ᱡᱩᱫᱤ (!argv[1]) {
		var_na **e = environ;

		/* environ can be NULL! (for example, after clearenv())
		 * Check for that:
		 */
		ᱡᱩᱫᱤ (e)
			ᱡᱚᱠᱷᱚᱱ (*e)
				puts(*e++);
	} ᱵᱟᱝ {
		/* search for specified variables and print them out if found */
		var_na *arg, *env;

		ᱡᱚᱠᱷᱚᱱ ((arg = *++argv) != NULL) {
			env = getenv(arg);
			ᱡᱩᱫᱤ (env)
				puts(env);
			ᱵᱟᱝ
				exit_code = EXIT_FAILURE;
		}
	}

	fflush_stdout_and_exit(exit_code);
}
