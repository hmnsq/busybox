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

#અન્તર્ભાવ "libbb.h"

પૂર્ણાંક nice_main(પૂર્ણાંક argc, અક્ષર **argv) MAIN_EXTERNALLY_VISIBLE;
પૂર્ણાંક nice_main(પૂર્ણાંક argc UNUSED_PARAM, અક્ષર **argv)
{
	પૂર્ણાંક old_priority, adjustment;

	old_priority = getpriority(PRIO_PROCESS, 0);

	જો (!*++argv) { /* No args, so (GNU) output current nice value. */
		printf("%d\n", old_priority);
		fflush_stdout_and_exit_SUCCESS();
	}

	adjustment = 10;  /* Set default adjustment. */

	જો (argv[0][0] == '-') {
		અક્ષર *nnn = argv[0] + 1;
		જો (nnn[0] == 'n') { /* -n */
			nnn += 1;
			જો (!nnn[0]) { /* "-n NNN" */
				nnn = *++argv;
			}
			/* else: "-nNNN" (w/o space) */
		}
		/* else: "-NNN" (NNN may be negative) - same as "-n NNN" */

		જો (!nnn || !argv[1]) {  /* Missing priority or PROG! */
			bb_show_usage();
		}
		adjustment = xatoi_range(nnn, INT_MIN/2, INT_MAX/2);
		argv++;
	}

	{  /* Set our priority. */
		પૂર્ણાંક prio = old_priority + adjustment;

		જો (setpriority(PRIO_PROCESS, 0, prio) < 0) {
			bb_perror_સંદેશ_and_die("setpriority(%d)", prio);
		}
	}

	BB_EXECVP_or_die(argv);
}
