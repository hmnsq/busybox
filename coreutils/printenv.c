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

#અન્તર્ભાવ "libbb.h"

/* This is a NOFORK applet. Be very careful! */

પૂર્ણાંક printenv_main(પૂર્ણાંક argc, અક્ષર **argv) MAIN_EXTERNALLY_VISIBLE;
પૂર્ણાંક printenv_main(પૂર્ણાંક argc UNUSED_PARAM, અક્ષર **argv)
{
	પૂર્ણાંક exit_code = EXIT_SUCCESS;

	/* no variables specified, show whole env */
	જો (!argv[1]) {
		અક્ષર **e = environ;

		/* environ can be NULL! (for example, after clearenv())
		 * Check for that:
		 */
		જો (e)
			જ્યારે (*e)
				puts(*e++);
	} નહીંતર {
		/* search for specified variables and print them out if found */
		અક્ષર *arg, *env;

		જ્યારે ((arg = *++argv) != ખાલી) {
			env = getenv(arg);
			જો (env)
				puts(env);
			નહીંતર
				exit_code = EXIT_FAILURE;
		}
	}

	fflush_stdout_and_exit(exit_code);
}
