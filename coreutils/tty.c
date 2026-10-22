/* vi: set sw=4 ts=4: */
/*
 * tty implementation for busybox
 *
 * Copyright (C) 2003  Manuel Novoa III  <mjn3@codepoet.org>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config TTY
//config:	bool "tty (3.9 kb)"
//config:	default y
//config:	help
//config:	tty is used to print the name of the current terminal to
//config:	standard output.

//applet:IF_TTY(APPLET_NOFORK(tty, tty, BB_DIR_USR_BIN, BB_SUID_DROP, tty))

//kbuild:lib-$(CONFIG_TTY) += tty.o

/* BB_AUDIT SUSv4 compliant */
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/tty.html */

//usage:#define tty_trivial_usage
//usage:       "" IF_INCLUDE_SUSv2("[-s]")
//usage:#define tty_full_usage "\n\n"
//usage:       "Print file name of stdin's terminal"
//usage:	IF_INCLUDE_SUSv2( "\n"
//usage:     "\n	-s	Print nothing, only return exit status"
//usage:	)
//usage:
//usage:#define tty_example_usage
//usage:       "$ tty\n"
//usage:       "/dev/tty2\n"

#ಅನ್ತರ್ಭಾವ "libbb.h"

ಪೂರ್ಣಾಙ್ಕ tty_main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಙ್ಕ tty_main(ಪೂರ್ಣಾಙ್ಕ argc UNUSED_PARAM, ವರ್ಣ **argv)
{
	ಧ್ರುವ ವರ್ಣ *s;
	IF_INCLUDE_SUSv2(ಪೂರ್ಣಾಙ್ಕ silent;)  /* Note: No longer relevant in SUSv3. */
	ಪೂರ್ಣಾಙ್ಕ retval;

	xfunc_error_retval = 2;  /* SUSv3 requires > 1 for error. */

	IF_INCLUDE_SUSv2(silent = getopt32(argv, "s");)
	IF_INCLUDE_SUSv2(argv += optind;)
	IF_NOT_INCLUDE_SUSv2(argv += 1;)

	/* gnu tty outputs a warning that it is ignoring all args. */
	bb_warn_ignoring_args(argv[0]);

	retval = EXIT_SUCCESS;

	s = xmalloc_ttyname(STDIN_FILENO);
	ಯದಿ (s == ರಿಕ್ತ) {
	/* According to SUSv3, ttyname can fail with EBADF or ENOTTY.
	 * We know the file descriptor is good, so failure means not a tty. */
		s = "not a tty";
		retval = EXIT_FAILURE;
	}
	IF_INCLUDE_SUSv2(ಯದಿ (!silent) puts(s);)
	IF_NOT_INCLUDE_SUSv2(puts(s);)

	fflush_stdout_and_exit(retval);
}
