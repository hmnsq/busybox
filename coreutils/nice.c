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

#شامل "libbb.h"

صحیح nice_main(صحیح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح nice_main(صحیح argc غیر_مستعمل_PARAM, حرف **argv)
{
	صحیح old_ترجیح, adjustment;

	old_ترجیح = getpriority(PRIO_PROCESS, 0);

	اگر (!*++argv) { /* No args, so (GNU) output current nice value. */
		لکھو("%d\n", old_ترجیح);
		fflush_stdout_and_exit_SUCCESS();
	}

	adjustment = 10;  /* Set default adjustment. */

	اگر (argv[0][0] == '-') {
		حرف *nnn = argv[0] + 1;
		اگر (nnn[0] == 'n') { /* -n */
			nnn += 1;
			اگر (!nnn[0]) { /* "-n NNN" */
				nnn = *++argv;
			}
			/* else: "-nNNN" (w/o space) */
		}
		/* else: "-NNN" (NNN may be negative) - same as "-n NNN" */

		اگر (!nnn || !argv[1]) {  /* Missing priority or PROG! */
			bb_show_usage();
		}
		adjustment = xatoi_range(nnn, INT_MIN/2, زیادہ_سے_زیادہ_عدد/2);
		argv++;
	}

	{  /* Set our priority. */
		صحیح prio = old_ترجیح + adjustment;

		اگر (setpriority(PRIO_PROCESS, 0, prio) < 0) {
			bb_perror_پیغام_and_die("setpriority(%d)", prio);
		}
	}

	BB_EXECVP_or_die(argv);
}
