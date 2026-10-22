/* vi: set sw=4 ts=4: */
/*
 * nice implementation for busybox
 *
 * Copyright (C) 2005  Manuel Novoa III  <mjn3@codepoet.org>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config NICE
//config:	bool "nice (2.3 kb)"
//config:	default y
//config:	help
//config:	nice runs a program with modified scheduling priority.

//applet:IF_NICE(APPLET_NOEXEC(nice, nice, BB_DIR_BIN, BB_SUID_DROP, nice))

//kbuild:lib-$(CONFIG_NICE) += nice.o

//usage:#define nice_trivial_usage
//usage:       "[-n ADJUST] [PROG ARGS]"
//usage:#define nice_full_usage "\n\n"
//usage:       "Change scheduling priority, run PROG\n"
//usage:     "\n	-n ADJUST	Adjust priority by ADJUST"

#समावेश "libbb.h"

पूर्णांक nice_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक nice_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	पूर्णांक old_priority, adjustment;

	old_priority = getpriority(PRIO_PROCESS, 0);

	अगर (!*++argv) { /* No args, so (GNU) output current nice value. */
		म_लिखो("%d\n", old_priority);
		fflush_stdout_and_exit_SUCCESS();
	}

	adjustment = 10;  /* Set default adjustment. */

	अगर (argv[0][0] == '-') {
		अक्षर *nnn = argv[0] + 1;
		अगर (nnn[0] == 'n') { /* -n */
			nnn += 1;
			अगर (!nnn[0]) { /* "-n NNN" */
				nnn = *++argv;
			}
			/* else: "-nNNN" (w/o space) */
		}
		/* else: "-NNN" (NNN may be negative) - same as "-n NNN" */

		अगर (!nnn || !argv[1]) {  /* Missing priority or PROG! */
			bb_show_usage();
		}
		adjustment = xatoi_range(nnn, पूर्णांक_न्यून/2, पूर्णांक_उच्च/2);
		argv++;
	}

	{  /* Set our priority. */
		पूर्णांक prio = old_priority + adjustment;

		अगर (setpriority(PRIO_PROCESS, 0, prio) < 0) {
			bb_perror_संदेश_and_die("setpriority(%d)", prio);
		}
	}

	BB_EXECVP_or_die(argv);
}
