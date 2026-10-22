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

#അന്തര്ഭാവ "libbb.h"

പൂർണ്ണസംഖ്യ nice_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) MAIN_EXTERNALLY_VISIBLE;
പൂർണ്ണസംഖ്യ nice_main(പൂർണ്ണസംഖ്യ argc UNUSED_PARAM, അക്ഷരം **argv)
{
	പൂർണ്ണസംഖ്യ old_priority, adjustment;

	old_priority = getpriority(PRIO_PROCESS, 0);

	എങ്കിൽ (!*++argv) { /* No args, so (GNU) output current nice value. */
		printf("%d\n", old_priority);
		fflush_stdout_and_exit_SUCCESS();
	}

	adjustment = 10;  /* Set default adjustment. */

	എങ്കിൽ (argv[0][0] == '-') {
		അക്ഷരം *nnn = argv[0] + 1;
		എങ്കിൽ (nnn[0] == 'n') { /* -n */
			nnn += 1;
			എങ്കിൽ (!nnn[0]) { /* "-n NNN" */
				nnn = *++argv;
			}
			/* else: "-nNNN" (w/o space) */
		}
		/* else: "-NNN" (NNN may be negative) - same as "-n NNN" */

		എങ്കിൽ (!nnn || !argv[1]) {  /* Missing priority or PROG! */
			bb_show_usage();
		}
		adjustment = xatoi_range(nnn, INT_MIN/2, INT_MAX/2);
		argv++;
	}

	{  /* Set our priority. */
		പൂർണ്ണസംഖ്യ prio = old_priority + adjustment;

		എങ്കിൽ (setpriority(PRIO_PROCESS, 0, prio) < 0) {
			bb_perror_സന്ദേശം_and_die("setpriority(%d)", prio);
		}
	}

	BB_EXECVP_or_die(argv);
}
