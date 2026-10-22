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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"

ਗਿਣਤੀ {
	SRC_BUF_ਮਾਪ = 15*3,  /* This *MUST* be a multiple of 3 */
	DST_BUF_ਮਾਪ = 4 * ((SRC_BUF_ਮਾਪ + 2) / 3),
};

ਪੂਰਨ uuencode_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ uuencode_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
	ਬਣਤਰ stat stat_buf;
	ਪੂਰਨ src_fd = STDIN_FILENO;
	ਅਚਲ ਅੱਖਰ *tbl;
	ਬਹੁਲਕ_t ਬਹੁਲਕ;
	ਅੱਖਰ src_buf[SRC_BUF_ਮਾਪ];
	ਅੱਖਰ dst_buf[DST_BUF_ਮਾਪ + 1];

	tbl = bb_uuenc_tbl_std;
	ਬਹੁਲਕ = 0666 & ~umask(0666);
	ਜੇ (getopt32(argv, "^" "m" "\0" "-1:?2"/*must have 1 or 2 args*/)) {
		tbl = bb_uuenc_tbl_base64;
	}
	argv += optind;
	ਜੇ (argv[1]) {
		src_fd = xopen(argv[0], O_RDONLY);
		fstat(src_fd, &stat_buf);
		ਬਹੁਲਕ = stat_buf.st_ਬਹੁਲਕ & (S_IRWXU | S_IRWXG | S_IRWXO);
		argv++;
	}

	ਲਿਖੋ("begin%s %o %s", tbl == bb_uuenc_tbl_std ? "" : "-base64", ਬਹੁਲਕ, *argv);
	ਜਦੋਂ (1) {
		ਮਾਪ_t ਮਾਪ = ਭਰਿਆ_read(src_fd, src_buf, SRC_BUF_ਮਾਪ);
		ਜੇ (!ਮਾਪ)
			ਰੋਕੋ;
		ਜੇ ((ssize_t)ਮਾਪ < 0)
			bb_simple_perror_ਸੁਨੇਹਾ_and_die(bb_ਸੁਨੇਹਾ_read_error);
		/* Encode the buffer we just read in */
		bb_uuencode(dst_buf, src_buf, ਮਾਪ, tbl);
		bb_putchar('\n');
		ਜੇ (tbl == bb_uuenc_tbl_std) {
			bb_putchar(tbl[ਮਾਪ]);
		}
		fflush(stdout);
		xwrite(STDOUT_FILENO, dst_buf, 4 * ((ਮਾਪ + 2) / 3));
	}
	ਲਿਖੋ(tbl == bb_uuenc_tbl_std ? "\n`\nend\n" : "\n====\n");

	fflush_stdout_and_exit_SUCCESS();
}
