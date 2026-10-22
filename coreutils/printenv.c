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

#அந்தர்பாவ "libbb.h"

/* This is a NOFORK applet. Be very careful! */

முழு printenv_main(முழு argc, எழுத்து **argv) MAIN_EXTERNALLY_VISIBLE;
முழு printenv_main(முழு argc UNUSED_PARAM, எழுத்து **argv)
{
	முழு exit_code = EXIT_SUCCESS;

	/* no variables specified, show whole env */
	எனில் (!argv[1]) {
		எழுத்து **e = environ;

		/* environ can be NULL! (for example, after clearenv())
		 * Check for that:
		 */
		எனில் (e)
			போது (*e)
				puts(*e++);
	} இல்லையெனில் {
		/* search for specified variables and print them out if found */
		எழுத்து *arg, *env;

		போது ((arg = *++argv) != வெற்று) {
			env = getenv(arg);
			எனில் (env)
				puts(env);
			இல்லையெனில்
				exit_code = EXIT_FAILURE;
		}
	}

	fflush_stdout_and_exit(exit_code);
}
