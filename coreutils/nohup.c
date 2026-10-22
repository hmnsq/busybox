/* vi: set sw=4 ts=4: */
/*
 * nohup - invoke a utility immune to hangups.
 *
 * Busybox version based on nohup specification at
 * http://www.opengroup.org/onlinepubs/007904975/utilities/nohup.html
 *
 * Copyright 2006 Rob Landley <rob@landley.net>
 * Copyright 2006 Bernhard Reutner-Fischer
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config NOHUP
//config:	bool "nohup (2.2 kb)"
//config:	default y
//config:	help
//config:	run a command immune to hangups, with output to a non-tty.

//applet:IF_NOHUP(APPLET_NOEXEC(nohup, nohup, BB_DIR_USR_BIN, BB_SUID_DROP, nohup))

//kbuild:lib-$(CONFIG_NOHUP) += nohup.o

//usage:#define nohup_trivial_usage
//usage:       "PROG ARGS"
//usage:#define nohup_full_usage "\n\n"
//usage:       "Run PROG immune to hangups, with output to a non-tty"
//usage:
//usage:#define nohup_example_usage
//usage:       "$ nohup make &"

#અન્તર્ભાવ "libbb.h"

/* Compat info: nohup (GNU coreutils 6.8) does this:
# nohup true
nohup: ignoring input and appending output to 'nohup.out'
# nohup true 1>/dev/null
nohup: ignoring input and redirecting stderr to stdout
# nohup true 2>zz
# cat zz
nohup: ignoring input and appending output to 'nohup.out'
# nohup true 2>zz 1>/dev/null
# cat zz
nohup: ignoring input
# nohup true </dev/null 1>/dev/null
nohup: redirecting stderr to stdout
# nohup true </dev/null 2>zz 1>/dev/null
# cat zz
  (nothing)
#
*/

પૂર્ણાંક nohup_main(પૂર્ણાંક argc, અક્ષર **argv) MAIN_EXTERNALLY_VISIBLE;
પૂર્ણાંક nohup_main(પૂર્ણાંક argc UNUSED_PARAM, અક્ષર **argv)
{
	અચલ અક્ષર *nohupout;
	અક્ષર *home;

	xfunc_error_retval = 127;

	જો (!argv[1]) {
		bb_show_usage();
	}

	/* If stdin is a tty, detach from it. */
	જો (isatty(STDIN_FILENO)) {
		/* bb_error_msg("ignoring input"); */
		close(STDIN_FILENO);
		xopen(bb_dev_null, O_RDONLY); /* will be fd 0 (STDIN_FILENO) */
	}

	nohupout = "nohup.out";
	/* Redirect stdout to nohup.out, either in "." or in "$HOME". */
	જો (isatty(STDOUT_FILENO)) {
		close(STDOUT_FILENO);
		જો (open(nohupout, O_CREAT|O_WRONLY|O_APPEND, S_IRUSR|S_IWUSR) < 0) {
			home = getenv("HOME");
			જો (home) {
				nohupout = concat_path_file(home, nohupout);
				xopen3(nohupout, O_CREAT|O_WRONLY|O_APPEND, S_IRUSR|S_IWUSR);
			} નહીંતર {
				xopen(bb_dev_null, O_RDONLY); /* will be fd 1 */
			}
		}
		bb_error_સંદેશ("appending output to %s", nohupout);
	}

	/* If we have a tty on stderr, redirect to stdout. */
	જો (isatty(STDERR_FILENO)) {
		/* if (stdout_wasnt_a_tty)
			bb_error_msg("redirecting stderr to stdout"); */
		dup2(STDOUT_FILENO, STDERR_FILENO);
	}

	સંકેત(SIGHUP, SIG_IGN);

	argv++;
	BB_EXECVP_or_die(argv);
}
