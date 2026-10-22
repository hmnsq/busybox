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

#अन्तर्भाव "libbb.h"

पूर्णाङ्क nice_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क nice_main(पूर्णाङ्क argc अप्रयुक्तम्_PARAM, वर्ण **argv)
{
	पूर्णाङ्क old_प्राथमिकता, adjustment;

	old_प्राथमिकता = getpriority(PRIO_PROCESS, 0);

	यदि (!*++argv) { /* No args, so (GNU) output current nice value. */
		प्रारूप_लिख("%d\n", old_प्राथमिकता);
		fflush_stdout_and_exit_SUCCESS();
	}

	adjustment = 10;  /* Set default adjustment. */

	यदि (argv[0][0] == '-') {
		वर्ण *nnn = argv[0] + 1;
		यदि (nnn[0] == 'n') { /* -n */
			nnn += 1;
			यदि (!nnn[0]) { /* "-n NNN" */
				nnn = *++argv;
			}
			/* else: "-nNNN" (w/o space) */
		}
		/* else: "-NNN" (NNN may be negative) - same as "-n NNN" */

		यदि (!nnn || !argv[1]) {  /* Missing priority or PROG! */
			bb_show_usage();
		}
		adjustment = xatoi_range(nnn, INT_MIN/2, अधिकतम_पूर्णाङ्कः/2);
		argv++;
	}

	{  /* Set our priority. */
		पूर्णाङ्क prio = old_प्राथमिकता + adjustment;

		यदि (setpriority(PRIO_PROCESS, 0, prio) < 0) {
			bb_perror_सन्देशः_and_die("setpriority(%d)", prio);
		}
	}

	BB_EXECVP_or_die(argv);
}
