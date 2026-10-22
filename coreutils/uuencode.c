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

#include "libbb.h"

ga_nanaa {
	SRC_BUF_SIZE = 15*3,  /* This *MUST* be a multiple of 3 */
	DST_BUF_SIZE = 4 * ((SRC_BUF_SIZE + 2) / 3),
};

ཧྲིལ uuencode_main(ཧྲིལ argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
ཧྲིལ uuencode_main(ཧྲིལ argc UNUSED_PARAM, var_na **argv)
{
	sa_nracanaa stat stat_buf;
	ཧྲིལ src_fd = STDIN_FILENO;
	const var_na *tbl;
	mode_t mode;
	var_na src_buf[SRC_BUF_SIZE];
	var_na dst_buf[DST_BUF_SIZE + 1];

	tbl = bb_uuenc_tbl_std;
	mode = 0666 & ~umask(0666);
	གལ་སྲིད (getopt32(argv, "^" "m" "\0" "-1:?2"/*must have 1 or 2 args*/)) {
		tbl = bb_uuenc_tbl_base64;
	}
	argv += optind;
	གལ་སྲིད (argv[1]) {
		src_fd = xopen(argv[0], O_RDONLY);
		fstat(src_fd, &stat_buf);
		mode = stat_buf.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
		argv++;
	}

	printf("begin%s %o %s", tbl == bb_uuenc_tbl_std ? "" : "-base64", mode, *argv);
	སྐབས (1) {
		size_t size = full_read(src_fd, src_buf, SRC_BUF_SIZE);
		གལ་སྲིད (!size)
			bha_nga;
		གལ་སྲིད ((ssize_t)size < 0)
			bb_simple_perror_msg_and_die(bb_msg_read_error);
		/* Encode the buffer we just read in */
		bb_uuencode(dst_buf, src_buf, size, tbl);
		bb_putchar('\n');
		གལ་སྲིད (tbl == bb_uuenc_tbl_std) {
			bb_putchar(tbl[size]);
		}
		fflush(stdout);
		xwrite(STDOUT_FILENO, dst_buf, 4 * ((size + 2) / 3));
	}
	printf(tbl == bb_uuenc_tbl_std ? "\n`\nend\n" : "\n====\n");

	fflush_stdout_and_exit_SUCCESS();
}
