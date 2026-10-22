/* vi: set sw=4 ts=4: */
/*
 * Copyright (C) 2000 by Glenn McGrath
 *
 * based on the function base64_encode from http.c in wget v1.6
 * Copyright (C) 1995, 1996, 1997, 1998, 2000 Free Software Foundation, Inc.
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config UUENCODE
//config:	bool "uuencode (4.7 kb)"
//config:	default y
//config:	help
//config:	uuencode is used to uuencode a file.

//applet:IF_UUENCODE(APPLET(uuencode, BB_DIR_USR_BIN, BB_SUID_DROP))

//kbuild:lib-$(CONFIG_UUENCODE) += uuencode.o

//usage:#define uuencode_trivial_usage
//usage:       "[-m] [FILE] STORED_FILENAME"
//usage:#define uuencode_full_usage "\n\n"
//usage:       "Uuencode FILE (or stdin) to stdout\n"
//usage:     "\n	-m	Use base64 encoding per RFC1521"
//usage:
//usage:#define uuencode_example_usage
//usage:       "$ uuencode busybox busybox\n"
//usage:       "begin 755 busybox\n"
//usage:       "<encoded file snipped>\n"
//usage:       "$ uudecode busybox busybox > busybox.uu\n"
//usage:       "$\n"

#అన్తర్భావ "libbb.h"

గణన {
	SRC_BUF_కొలత = 15*3,  /* This *MUST* be a multiple of 3 */
	DST_BUF_కొలత = 4 * ((SRC_BUF_కొలత + 2) / 3),
};

పూర్ణాంకం uuencode_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం uuencode_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	నిర్మాణం stat stat_buf;
	పూర్ణాంకం src_fd = STDIN_FILENO;
	స్థిరరాశి అక్షరం *tbl;
	బహుళకం_t బహుళకం;
	అక్షరం src_buf[SRC_BUF_కొలత];
	అక్షరం dst_buf[DST_BUF_కొలత + 1];

	tbl = bb_uuenc_tbl_std;
	బహుళకం = 0666 & ~umask(0666);
	ఒకవేళ (getopt32(argv, "^" "m" "\0" "-1:?2"/*must have 1 or 2 args*/)) {
		tbl = bb_uuenc_tbl_base64;
	}
	argv += optind;
	ఒకవేళ (argv[1]) {
		src_fd = xopen(argv[0], O_RDONLY);
		fstat(src_fd, &stat_buf);
		బహుళకం = stat_buf.st_బహుళకం & (S_IRWXU | S_IRWXG | S_IRWXO);
		argv++;
	}

	రాయి("begin%s %o %s", tbl == bb_uuenc_tbl_std ? "" : "-base64", బహుళకం, *argv);
	అయితే (1) {
		కొలత_t కొలత = నిండిన_read(src_fd, src_buf, SRC_BUF_కొలత);
		ఒకవేళ (!కొలత)
			ఆపు;
		ఒకవేళ ((ssize_t)కొలత < 0)
			bb_simple_perror_సందేశం_and_die(bb_సందేశం_read_error);
		/* Encode the buffer we just read in */
		bb_uuencode(dst_buf, src_buf, కొలత, tbl);
		bb_putchar('\n');
		ఒకవేళ (tbl == bb_uuenc_tbl_std) {
			bb_putchar(tbl[కొలత]);
		}
		fflush(stdout);
		xwrite(STDOUT_FILENO, dst_buf, 4 * ((కొలత + 2) / 3));
	}
	రాయి(tbl == bb_uuenc_tbl_std ? "\n`\nend\n" : "\n====\n");

	fflush_stdout_and_exit_SUCCESS();
}
