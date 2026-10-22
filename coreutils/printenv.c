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

#অন্তর্ভাব "libbb.h"

/* This is a NOFORK applet. Be very careful! */

পূর্ণসংখ্যা printenv_main(পূর্ণসংখ্যা argc, অক্ষর **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণসংখ্যা printenv_main(পূর্ণসংখ্যা argc UNUSED_PARAM, অক্ষর **argv)
{
	পূর্ণসংখ্যা exit_code = EXIT_SUCCESS;

	/* no variables specified, show whole env */
	যদি (!argv[1]) {
		অক্ষর **e = environ;

		/* environ can be NULL! (for example, after clearenv())
		 * Check for that:
		 */
		যদি (e)
			যখন (*e)
				puts(*e++);
	} অন্যথা {
		/* search for specified variables and print them out if found */
		অক্ষর *arg, *env;

		যখন ((arg = *++argv) != ফাঁকা) {
			env = getenv(arg);
			যদি (env)
				puts(env);
			অন্যথা
				exit_code = EXIT_FAILURE;
		}
	}

	fflush_stdout_and_exit(exit_code);
}
