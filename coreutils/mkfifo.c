/* vi: set sw=4 ts=4: */
/*
 * mkfifo implementation for busybox
 *
 * Copyright (C) 2003  Manuel Novoa III  <mjn3@codepoet.org>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config MKFIFO
//config:	bool "mkfifo (4 kb)"
//config:	default y
//config:	help
//config:	mkfifo is used to create FIFOs (named pipes).
//config:	The 'mknod' program can also create FIFOs.

//applet:IF_MKFIFO(APPLET_NOEXEC(mkfifo, mkfifo, BB_DIR_USR_BIN, BB_SUID_DROP, mkfifo))

//kbuild:lib-$(CONFIG_MKFIFO) += mkfifo.o

/* BB_AUDIT SUSv3 compliant */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/mkfifo.html */

//usage:#define mkfifo_trivial_usage
//usage:       "[-m MODE] " IF_SELINUX("[-Z] ") "NAME"
//usage:#define mkfifo_full_usage "\n\n"
//usage:       "Create named pipe\n"
//usage:     "\n	-m MODE	Mode (default a=rw)"
//usage:	IF_SELINUX(
//usage:     "\n	-Z	Set security context"
//usage:	)

#അന്തര്ഭാവ "libbb.h"
#അന്തര്ഭാവ "libcoreutils/coreutils.h"

/* This is a NOEXEC applet. Be very careful! */

പൂർണ്ണസംഖ്യ mkfifo_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) MAIN_EXTERNALLY_VISIBLE;
പൂർണ്ണസംഖ്യ mkfifo_main(പൂർണ്ണസംഖ്യ argc UNUSED_PARAM, അക്ഷരം **argv)
{
	mode_t mode;
	പൂർണ്ണസംഖ്യ retval = EXIT_SUCCESS;

	mode = getopt_mk_fifo_nod(argv);

	argv += optind;
	എങ്കിൽ (!*argv) {
		bb_show_usage();
	}

	ചെയ്യുക {
		എങ്കിൽ (mkfifo(*argv, mode) < 0) {
			bb_simple_perror_സന്ദേശം(*argv);  /* Avoid multibyte problems. */
			retval = EXIT_FAILURE;
		}
	} ആകുമ്പോൾ (*++argv);

	തിരികെ retval;
}
