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

#ضمن "libbb.h"

تعداد {
	SRC_BUF_اندازه = 15*3,  /* This *MUST* be a multiple of 3 */
	DST_BUF_اندازه = 4 * ((SRC_BUF_اندازه + 2) / 3),
};

صحيح uuencode_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح uuencode_main(صحيح argc ناکارول_شوی_PARAM, حرف **argv)
{
	بنية stat stat_buf;
	صحيح src_fd = STDIN_FILENO;
	ثابت حرف *tbl;
	ډېرپېښ_t ډېرپېښ;
	حرف src_buf[SRC_BUF_اندازه];
	حرف dst_buf[DST_BUF_اندازه + 1];

	tbl = bb_uuenc_tbl_std;
	ډېرپېښ = 0666 & ~umask(0666);
	إذا (getopt32(argv, "^" "m" "\0" "-1:?2"/*must have 1 or 2 args*/)) {
		tbl = bb_uuenc_tbl_base64;
	}
	argv += optind;
	إذا (argv[1]) {
		src_fd = xopen(argv[0], O_RDONLY);
		fstat(src_fd, &stat_buf);
		ډېرپېښ = stat_buf.st_ډېرپېښ & (S_IRWXU | S_IRWXG | S_IRWXO);
		argv++;
	}

	چاپ_کړه("begin%s %o %s", tbl == bb_uuenc_tbl_std ? "" : "-base64", ډېرپېښ, *argv);
	طالما (1) {
		اندازه_t اندازه = ډک_read(src_fd, src_buf, SRC_BUF_اندازه);
		إذا (!اندازه)
			اكسر;
		إذا ((ssize_t)اندازه < 0)
			bb_simple_perror_پیغام_and_die(bb_پیغام_read_error);
		/* Encode the buffer we just read in */
		bb_uuencode(dst_buf, src_buf, اندازه, tbl);
		bb_putchar('\n');
		إذا (tbl == bb_uuenc_tbl_std) {
			bb_putchar(tbl[اندازه]);
		}
		fflush(stdout);
		xwrite(STDOUT_FILENO, dst_buf, 4 * ((اندازه + 2) / 3));
	}
	چاپ_کړه(tbl == bb_uuenc_tbl_std ? "\n`\nend\n" : "\n====\n");

	fflush_stdout_and_exit_SUCCESS();
}
