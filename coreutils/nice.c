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

#include "libbb.h"

puur_naa_nka nice_main(puur_naa_nka argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
puur_naa_nka nice_main(puur_naa_nka argc UNUSED_PARAM, var_na **argv)
{
	puur_naa_nka old_priority, adjustment;

	old_priority = getpriority(PRIO_PROCESS, 0);

	ပေႃး (!*++argv) { /* No args, so (GNU) output current nice value. */
		printf("%d\n", old_priority);
		fflush_stdout_and_exit_SUCCESS();
	}

	adjustment = 10;  /* Set default adjustment. */

	ပေႃး (argv[0][0] == '-') {
		var_na *nnn = argv[0] + 1;
		ပေႃး (nnn[0] == 'n') { /* -n */
			nnn += 1;
			ပေႃး (!nnn[0]) { /* "-n NNN" */
				nnn = *++argv;
			}
			/* else: "-nNNN" (w/o space) */
		}
		/* else: "-NNN" (NNN may be negative) - same as "-n NNN" */

		ပေႃး (!nnn || !argv[1]) {  /* Missing priority or PROG! */
			bb_show_usage();
		}
		adjustment = xatoi_range(nnn, INT_MIN/2, INT_MAX/2);
		argv++;
	}

	{  /* Set our priority. */
		puur_naa_nka prio = old_priority + adjustment;

		ပေႃး (setpriority(PRIO_PROCESS, 0, prio) < 0) {
			bb_perror_msg_and_die("setpriority(%d)", prio);
		}
	}

	BB_EXECVP_or_die(argv);
}
