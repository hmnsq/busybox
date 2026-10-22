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

#অন্তর্ভাব "libbb.h"

গণনা {
	SRC_BUF_আকার = 15*3,  /* This *MUST* be a multiple of 3 */
	DST_BUF_আকার = 4 * ((SRC_BUF_আকার + 2) / 3),
};

পূর্ণসংখ্যা uuencode_main(পূর্ণসংখ্যা argc, অক্ষর **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণসংখ্যা uuencode_main(পূর্ণসংখ্যা argc UNUSED_PARAM, অক্ষর **argv)
{
	কাঠামো stat stat_buf;
	পূর্ণসংখ্যা src_fd = STDIN_FILENO;
	ধ্রুবক অক্ষর *tbl;
	mode_t mode;
	অক্ষর src_buf[SRC_BUF_আকার];
	অক্ষর dst_buf[DST_BUF_আকার + 1];

	tbl = bb_uuenc_tbl_std;
	mode = 0666 & ~umask(0666);
	যদি (getopt32(argv, "^" "m" "\0" "-1:?2"/*must have 1 or 2 args*/)) {
		tbl = bb_uuenc_tbl_base64;
	}
	argv += optind;
	যদি (argv[1]) {
		src_fd = xopen(argv[0], O_RDONLY);
		fstat(src_fd, &stat_buf);
		mode = stat_buf.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
		argv++;
	}

	printf("begin%s %o %s", tbl == bb_uuenc_tbl_std ? "" : "-base64", mode, *argv);
	যখন (1) {
		আকার_t আকার_2 = full_read(src_fd, src_buf, SRC_BUF_আকার);
		যদি (!আকার_2)
			বিরতি;
		যদি ((ssize_t)আকার_2 < 0)
			bb_simple_perror_বার্তা_and_die(bb_বার্তা_read_error);
		/* Encode the buffer we just read in */
		bb_uuencode(dst_buf, src_buf, আকার_2, tbl);
		bb_putchar('\n');
		যদি (tbl == bb_uuenc_tbl_std) {
			bb_putchar(tbl[আকার_2]);
		}
		fflush(stdout);
		xwrite(STDOUT_FILENO, dst_buf, 4 * ((আকার_2 + 2) / 3));
	}
	printf(tbl == bb_uuenc_tbl_std ? "\n`\nend\n" : "\n====\n");

	fflush_stdout_and_exit_SUCCESS();
}
