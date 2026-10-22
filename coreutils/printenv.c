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

ಪೂರ್ಣಾಂಕ printenv_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ printenv_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	ಪೂರ್ಣಾಂಕ exit_ಸಂಕೇತ = EXIT_SUCCESS;

	/* no variables specified, show whole env */
	ಆದರೆ (!argv[1]) {
		ಅಕ್ಷರ **e = environ;

		/* environ can be NULL! (for example, after clearenv())
		 * Check for that:
		 */
		ಆದರೆ (e)
			ಆಗ (*e)
				puts(*e++);
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* search for specified variables and print them out if found */
		ಅಕ್ಷರ *arg, *env;

		ಆಗ ((arg = *++argv) != ಖಾಲಿ) {
			env = getenv(arg);
			ಆದರೆ (env)
				puts(env);
			ಇಲ್ಲದಿದ್ದರೆ
				exit_ಸಂಕೇತ = EXIT_FAILURE;
		}
	}

	fflush_stdout_and_exit(exit_ಸಂಕೇತ);
}
