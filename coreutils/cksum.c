/* vi: set sw=4 ts=4: */
/*
 * cksum - calculate the CRC32 checksum of a file
 *
 * Copyright (C) 2006 by Rob Sullivan, with ideas from code by Walter Harms
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config CKSUM
//config:	bool "cksum (4.3 kb)"
//config:	default y
//config:
//config:config CRC32
//config:	bool "crc32 (4.2 kb)"
//config:	default y

//                APPLET_NOEXEC:name   main   location        suid_type     help
//applet:IF_CKSUM(APPLET_NOEXEC(cksum, cksum, BB_DIR_USR_BIN, BB_SUID_DROP, cksum))
//applet:IF_CRC32(APPLET_NOEXEC(crc32, cksum, BB_DIR_USR_BIN, BB_SUID_DROP, cksum))
/* bb_common_bufsiz1 usage here is safe wrt NOEXEC: not expecting it to be zeroed. */

//kbuild:lib-$(CONFIG_CKSUM) += cksum.o
//kbuild:lib-$(CONFIG_CRC32) += cksum.o

//usage:#define cksum_trivial_usage
//usage:       "FILE..."
//usage:#define cksum_full_usage "\n\n"
//usage:       "Calculate CRC32 checksum of FILEs"

#অন্তর্ভাব "libbb.h"
#অন্তর্ভাব "common_bufsiz.h"

/* This is a NOEXEC applet. Be very careful! */

#define IS_CKSUM (ENABLE_CKSUM && (!ENABLE_CRC32 || applet_name[1] == 'k'))
#define IS_CRC32 (ENABLE_CRC32 && (!ENABLE_CKSUM || applet_name[1] == 'r'))

পূর্ণাঙ্ক cksum_main(পূর্ণাঙ্ক argc, বর্ণ **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণাঙ্ক cksum_main(পূর্ণাঙ্ক argc UNUSED_PARAM, বর্ণ **argv)
{
	uint32_t *crc32_table = crc32_filltable(রিক্ত, IS_CKSUM);
	exitcode_t exit_code = EXIT_SUCCESS;

#যদি ENABLE_DESKTOP
	getopt32(argv, ""); /* cksum coreutils 6.9 compat */
	argv += optind;
#অন্যথা
	argv++;
#endif

	setup_common_bufsiz();
	কুরু {
		uint32_t crc;
		IF_CKSUM(off_t filesize;)
		ধ্রুব বর্ণ *fname = *argv ? *argv : bb_বার্তা_standard_প্রবেশ;
		পূর্ণাঙ্ক fd = open_or_warn_stdin(fname);

		যদি (fd < 0) {
			exit_code = EXIT_FAILURE;
			অনুবর্ত;
		}

		crc = IS_CKSUM ? 0 : 0xffffffff;
		IF_CKSUM(filesize = 0;)
#define read_buf bb_common_bufsiz1
		প্রতি (;;) {
			পূর্ণাঙ্ক bytes_read = safe_read(fd, read_buf, COMMON_BUFSIZE);
			যদি (bytes_read < 0)
				bb_simple_perror_বার্তা_and_die(fname);
			যদি (bytes_read > 0) {
				IF_CKSUM(filesize += bytes_read;)
			} অন্যথা {
				IF_CKSUM(uoff_t t;)

				close(fd);
				যদি (IS_CRC32)
					ভঙ্গ;
#যদি ENABLE_CKSUM
				fd = -1; /* break flag */
				/* Checksum filesize bytes, LSB first */
				t = filesize;
				/*bytes_read = 0; - already is */
				যাবত্ (t != 0) {
					read_buf[bytes_read++] = (uint8_t)t;
					t >>= 8;
				}
#endif
			}
			crc = (IS_CKSUM ? crc32_block_endian1 : crc32_block_endian0)(crc, read_buf, bytes_read, crc32_table);
			যদি (ENABLE_CKSUM && fd < 0)
				ভঙ্গ;
		}

		crc = ~crc;
#যদি ENABLE_CKSUM
		যদি (IS_CKSUM)
			printf((*argv ? "%u %"OFF_FMT"u %s\n" : "%u %"OFF_FMT"u\n"),
				(unsigned)crc, filesize, *argv);
		অন্যথা
#endif
			printf((*argv ? "%08x %s\n" : "%08x\n"),
				(unsigned)crc, *argv);
	} যাবত্ (*argv && *++argv);

	fflush_stdout_and_exit(exit_code);
}
