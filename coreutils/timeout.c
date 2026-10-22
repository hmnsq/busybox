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

#شامل "libbb.h"

ساکن NOINLINE صحیح timeout_wait(duration_t timeout, pid_t pid)
{
	/* Just sleep(HUGE_NUM); kill(parent) may kill wrong process! */
	جب (1) {
#اگر ENABLE_FLOAT_DURATION
		اگر (timeout < 1)
			sleep_for_duration(timeout);
		ورنہ
#اگر_ختم
			sleep1();
		اگر (--timeout <= 0)
			توڑو;
		اگر (kill(pid, 0)) {
			/* process is gone */
			واپس EXIT_SUCCESS;
		}
	}
	واپس EXIT_FAILURE;
}

صحیح timeout_main(صحیح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح timeout_main(صحیح argc غیر_مستعمل_PARAM, حرف **argv)
{
	صحیح signo;
	صحیح حالت_2;
	صحیح والد = 0;
	duration_t timeout;
	duration_t kill_timeout;
	pid_t pid;
#اگر !BB_MMU
	حرف *sv1, *sv2;
#اگر_ختم
	ثابت حرف *opt_s = "TERM";
	حرف *opt_k = تہی;

	/* -p option is not documented, it is needed to support NOMMU. */

	/* -t SECONDS; -p PARENT_PID */
	/* '+': stop at first non-option */
	getopt32(argv, "+s:k:" USE_FOR_NOMMU("p:+"), &opt_s, &opt_k, &والد);
	/*argv += optind; - no, wait for bb_daemonize_or_rexec! */

	signo = get_signum(opt_s);
	اگر (signo < 0)
		bb_error_پیغام_and_die("unknown signal '%s'", opt_s);

	kill_timeout = 0;
	اگر (opt_k)
		kill_timeout = parse_duration_str(opt_k);

	اگر (!argv[optind])
		bb_show_usage();
	timeout = parse_duration_str(argv[optind++]);
	اگر (!argv[optind]) /* no PROG? */
		bb_show_usage();

	/* We want to create a grandchild which will watch
	 * and kill the grandparent. Other methods:
	 * making parent watch child disrupts parent<->child link
	 * (example: "tcpsvd 0.0.0.0 1234 timeout service_prog" -
	 * it's better if service_prog is a child of tcpsvd!),
	 * making child watch parent results in programs having
	 * unexpected children.	*/

	اگر (والد) /* we were re-execed, already grandchild */
		جاؤ grandchild;

#اگر !BB_MMU
	sv1 = argv[optind];
	sv2 = argv[optind + 1];
#اگر_ختم
	pid = xvfork();
	اگر (pid == 0) {
		/* Child: spawn grandchild and exit */
		والد = getppid();
#اگر !BB_MMU
		argv[optind] = xasprintf("-p%u", والد);
		argv[optind + 1] = تہی;
#اگر_ختم
		/* NB: exits with nonzero on error: */
		bb_daemonize_or_rexec(0, argv);
		/* Here we are grandchild. Sleep, then kill grandparent */
 grandchild:
		اگر (timeout_wait(timeout, والد) == EXIT_SUCCESS)
			واپس EXIT_SUCCESS;
		kill(والد, signo);

		اگر (kill_timeout > 0) {
			اگر (timeout_wait(kill_timeout, والد) == EXIT_SUCCESS)
				واپس EXIT_SUCCESS;
			kill(والد, SIGKILL);
		}

		واپس EXIT_SUCCESS;
	}

	/*والد*/
	wait(&حالت_2); /* wait for child to die */
	/* Did intermediate [v]fork or exec fail? */
	اگر (!WIFEXITED(حالت_2) || WEXITSTATUS(حالت_2) != 0)
		واپس EXIT_FAILURE;
	/* Ok, exec a program as requested */
	argv += optind;
#اگر !BB_MMU
	argv[0] = sv1;
	argv[1] = sv2;
#اگر_ختم
	BB_EXECVP_or_die(argv);
}
