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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"

/* This is a NOFORK applet. Be very careful! */

ਪੂਰਨ printenv_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ printenv_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
	ਪੂਰਨ exit_ਕੋਡ = EXIT_SUCCESS;

	/* no variables specified, show whole env */
	ਜੇ (!argv[1]) {
		ਅੱਖਰ **e = environ;

		/* environ can be NULL! (for example, after clearenv())
		 * Check for that:
		 */
		ਜੇ (e)
			ਜਦੋਂ (*e)
				puts(*e++);
	} ਨਹੀਂਤਾਂ {
		/* search for specified variables and print them out if found */
		ਅੱਖਰ *arg, *env;

		ਜਦੋਂ ((arg = *++argv) != ਨਿਰਾ) {
			env = getenv(arg);
			ਜੇ (env)
				puts(env);
			ਨਹੀਂਤਾਂ
				exit_ਕੋਡ = EXIT_FAILURE;
		}
	}

	fflush_stdout_and_exit(exit_ਕੋਡ);
}
