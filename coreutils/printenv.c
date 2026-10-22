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

#അന്തര്ഭാവ "libbb.h"

/* This is a NOFORK applet. Be very careful! */

പൂർണ്ണസംഖ്യ printenv_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) MAIN_EXTERNALLY_VISIBLE;
പൂർണ്ണസംഖ്യ printenv_main(പൂർണ്ണസംഖ്യ argc UNUSED_PARAM, അക്ഷരം **argv)
{
	പൂർണ്ണസംഖ്യ exit_code = EXIT_SUCCESS;

	/* no variables specified, show whole env */
	എങ്കിൽ (!argv[1]) {
		അക്ഷരം **e = environ;

		/* environ can be NULL! (for example, after clearenv())
		 * Check for that:
		 */
		എങ്കിൽ (e)
			ആകുമ്പോൾ (*e)
				puts(*e++);
	} അല്ലെങ്കിൽ {
		/* search for specified variables and print them out if found */
		അക്ഷരം *arg, *env;

		ആകുമ്പോൾ ((arg = *++argv) != ശൂന്യ) {
			env = getenv(arg);
			എങ്കിൽ (env)
				puts(env);
			അല്ലെങ്കിൽ
				exit_code = EXIT_FAILURE;
		}
	}

	fflush_stdout_and_exit(exit_code);
}
