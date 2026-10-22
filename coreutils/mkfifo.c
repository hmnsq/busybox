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

#అన్తర్భావ "libbb.h"
#అన్తర్భావ "libcoreutils/coreutils.h"

/* This is a NOEXEC applet. Be very careful! */

పూర్ణాంకం mkfifo_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం mkfifo_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	బహుళకం_t బహుళకం;
	పూర్ణాంకం retval = EXIT_SUCCESS;

	బహుళకం = getopt_mk_fifo_nod(argv);

	argv += optind;
	ఒకవేళ (!*argv) {
		bb_show_usage();
	}

	చేయి {
		ఒకవేళ (mkfifo(*argv, బహుళకం) < 0) {
			bb_simple_perror_సందేశం(*argv);  /* Avoid multibyte problems. */
			retval = EXIT_FAILURE;
		}
	} అయితే (*++argv);

	తిరిగి retval;
}
