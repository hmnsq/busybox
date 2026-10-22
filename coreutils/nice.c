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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"

ਪੂਰਨ nice_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ nice_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
	ਪੂਰਨ old_ਤਰਜੀਹ, adjustment;

	old_ਤਰਜੀਹ = getpriority(PRIO_PROCESS, 0);

	ਜੇ (!*++argv) { /* No args, so (GNU) output current nice value. */
		ਲਿਖੋ("%d\n", old_ਤਰਜੀਹ);
		fflush_stdout_and_exit_SUCCESS();
	}

	adjustment = 10;  /* Set default adjustment. */

	ਜੇ (argv[0][0] == '-') {
		ਅੱਖਰ *nnn = argv[0] + 1;
		ਜੇ (nnn[0] == 'n') { /* -n */
			nnn += 1;
			ਜੇ (!nnn[0]) { /* "-n NNN" */
				nnn = *++argv;
			}
			/* else: "-nNNN" (w/o space) */
		}
		/* else: "-NNN" (NNN may be negative) - same as "-n NNN" */

		ਜੇ (!nnn || !argv[1]) {  /* Missing priority or PROG! */
			bb_show_usage();
		}
		adjustment = xatoi_range(nnn, INT_MIN/2, ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ/2);
		argv++;
	}

	{  /* Set our priority. */
		ਪੂਰਨ prio = old_ਤਰਜੀਹ + adjustment;

		ਜੇ (setpriority(PRIO_PROCESS, 0, prio) < 0) {
			bb_perror_ਸੁਨੇਹਾ_and_die("setpriority(%d)", prio);
		}
	}

	BB_EXECVP_or_die(argv);
}
