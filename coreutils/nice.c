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

#అన్తర్భావ "libbb.h"

పూర్ణాంకం nice_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం nice_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	పూర్ణాంకం old_ప్రాధాన్యత, adjustment;

	old_ప్రాధాన్యత = getpriority(PRIO_PROCESS, 0);

	ఒకవేళ (!*++argv) { /* No args, so (GNU) output current nice value. */
		రాయి("%d\n", old_ప్రాధాన్యత);
		fflush_stdout_and_exit_SUCCESS();
	}

	adjustment = 10;  /* Set default adjustment. */

	ఒకవేళ (argv[0][0] == '-') {
		అక్షరం *nnn = argv[0] + 1;
		ఒకవేళ (nnn[0] == 'n') { /* -n */
			nnn += 1;
			ఒకవేళ (!nnn[0]) { /* "-n NNN" */
				nnn = *++argv;
			}
			/* else: "-nNNN" (w/o space) */
		}
		/* else: "-NNN" (NNN may be negative) - same as "-n NNN" */

		ఒకవేళ (!nnn || !argv[1]) {  /* Missing priority or PROG! */
			bb_show_usage();
		}
		adjustment = xatoi_range(nnn, INT_MIN/2, గరిష్ఠ_పూర్ణాంకం/2);
		argv++;
	}

	{  /* Set our priority. */
		పూర్ణాంకం prio = old_ప్రాధాన్యత + adjustment;

		ఒకవేళ (setpriority(PRIO_PROCESS, 0, prio) < 0) {
			bb_perror_సందేశం_and_die("setpriority(%d)", prio);
		}
	}

	BB_EXECVP_or_die(argv);
}
