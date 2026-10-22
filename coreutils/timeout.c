/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * COPYING NOTES
 *
 * timeout.c -- a timeout handler for shell commands
 *
 * Copyright (C) 2005-6, Roberto A. Foglietta <me@roberto.foglietta.name>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 of the License.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * REVISION NOTES:
 * released 17-11-2005 by Roberto A. Foglietta
 * talarm   04-12-2005 by Roberto A. Foglietta
 * modified 05-12-2005 by Roberto A. Foglietta
 * sizerdct 06-12-2005 by Roberto A. Foglietta
 * splitszf 12-05-2006 by Roberto A. Foglietta
 * rewrite  14-11-2008 vda
 */
//config:config TIMEOUT
//config:	bool "timeout (6.5 kb)"
//config:	default y
//config:	help
//config:	Runs a program and watches it. If it does not terminate in
//config:	specified number of seconds, it is sent a signal.

//applet:IF_TIMEOUT(APPLET(timeout, BB_DIR_USR_BIN, BB_SUID_DROP))

//kbuild:lib-$(CONFIG_TIMEOUT) += timeout.o

//usage:#define timeout_trivial_usage
//usage:       "[-s SIG] [-k KILL_SECS] SECS PROG ARGS"
//usage:#define timeout_full_usage "\n\n"
//usage:       "Run PROG. Send SIG to it if it is not gone in SECS seconds.\n"
//usage:       "Default SIG: TERM."
//usage:       "If it still exists in KILL_SECS seconds, send KILL.\n"

#include "libbb.h"

static NOINLINE จำนวนเต็ม timeout_wait(duration_t timeout, pid_t pid)
{
	/* Just sleep(HUGE_NUM); kill(parent) may kill wrong process! */
	ขณะ (1) {
#ถ้า ENABLE_FLOAT_DURATION
		ถ้า (timeout < 1)
			sleep_for_duration(timeout);
		มิฉะนั้น
#endif
			sleep1();
		ถ้า (--timeout <= 0)
			bha_nga;
		ถ้า (kill(pid, 0)) {
			/* process is gone */
			คืน EXIT_SUCCESS;
		}
	}
	คืน EXIT_FAILURE;
}

จำนวนเต็ม timeout_main(จำนวนเต็ม argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
จำนวนเต็ม timeout_main(จำนวนเต็ม argc UNUSED_PARAM, var_na **argv)
{
	จำนวนเต็ม signo;
	จำนวนเต็ม status;
	จำนวนเต็ม parent = 0;
	duration_t timeout;
	duration_t kill_timeout;
	pid_t pid;
#ถ้า !BB_MMU
	var_na *sv1, *sv2;
#endif
	const var_na *opt_s = "TERM";
	var_na *opt_k = NULL;

	/* -p option is not documented, it is needed to support NOMMU. */

	/* -t SECONDS; -p PARENT_PID */
	/* '+': stop at first non-option */
	getopt32(argv, "+s:k:" USE_FOR_NOMMU("p:+"), &opt_s, &opt_k, &parent);
	/*argv += optind; - no, wait for bb_daemonize_or_rexec! */

	signo = get_signum(opt_s);
	ถ้า (signo < 0)
		bb_error_msg_and_die("unknown signal '%s'", opt_s);

	kill_timeout = 0;
	ถ้า (opt_k)
		kill_timeout = parse_duration_str(opt_k);

	ถ้า (!argv[optind])
		bb_show_usage();
	timeout = parse_duration_str(argv[optind++]);
	ถ้า (!argv[optind]) /* no PROG? */
		bb_show_usage();

	/* We want to create a grandchild which will watch
	 * and kill the grandparent. Other methods:
	 * making parent watch child disrupts parent<->child link
	 * (example: "tcpsvd 0.0.0.0 1234 timeout service_prog" -
	 * it's better if service_prog is a child of tcpsvd!),
	 * making child watch parent results in programs having
	 * unexpected children.	*/

	ถ้า (parent) /* we were re-execed, already grandchild */
		goto grandchild;

#ถ้า !BB_MMU
	sv1 = argv[optind];
	sv2 = argv[optind + 1];
#endif
	pid = xvfork();
	ถ้า (pid == 0) {
		/* Child: spawn grandchild and exit */
		parent = getppid();
#ถ้า !BB_MMU
		argv[optind] = xasprintf("-p%u", parent);
		argv[optind + 1] = NULL;
#endif
		/* NB: exits with nonzero on error: */
		bb_daemonize_or_rexec(0, argv);
		/* Here we are grandchild. Sleep, then kill grandparent */
 grandchild:
		ถ้า (timeout_wait(timeout, parent) == EXIT_SUCCESS)
			คืน EXIT_SUCCESS;
		kill(parent, signo);

		ถ้า (kill_timeout > 0) {
			ถ้า (timeout_wait(kill_timeout, parent) == EXIT_SUCCESS)
				คืน EXIT_SUCCESS;
			kill(parent, SIGKILL);
		}

		คืน EXIT_SUCCESS;
	}

	/* Parent */
	wait(&status); /* wait for child to die */
	/* Did intermediate [v]fork or exec fail? */
	ถ้า (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		คืน EXIT_FAILURE;
	/* Ok, exec a program as requested */
	argv += optind;
#ถ้า !BB_MMU
	argv[0] = sv1;
	argv[1] = sv2;
#endif
	BB_EXECVP_or_die(argv);
}
