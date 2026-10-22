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

#ಅನ್ತರ್ಭಾವ "libbb.h"
#ಅನ್ತರ್ಭಾವ "common_bufsiz.h"

/* This is a NOEXEC applet. Be very careful! */

#define IS_CKSUM (ENABLE_CKSUM && (!ENABLE_CRC32 || applet_name[1] == 'k'))
#define IS_CRC32 (ENABLE_CRC32 && (!ENABLE_CKSUM || applet_name[1] == 'r'))

ಪೂರ್ಣಾಙ್ಕ cksum_main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಙ್ಕ cksum_main(ಪೂರ್ಣಾಙ್ಕ argc UNUSED_PARAM, ವರ್ಣ **argv)
{
	uint32_t *crc32_table = crc32_filltable(ರಿಕ್ತ, IS_CKSUM);
	exitcode_t exit_code = EXIT_SUCCESS;

#ಯದಿ ENABLE_DESKTOP
	getopt32(argv, ""); /* cksum coreutils 6.9 compat */
	argv += optind;
#ಅನ್ಯಥಾ
	argv++;
#endif

	setup_common_bufsiz();
	ಕುರು {
		uint32_t crc;
		IF_CKSUM(off_t filesize;)
		ಧ್ರುವ ವರ್ಣ *fname = *argv ? *argv : bb_ಸಂದೇಶ_standard_ಒಳಹರಿವು;
		ಪೂರ್ಣಾಙ್ಕ fd = open_or_warn_stdin(fname);

		ಯದಿ (fd < 0) {
			exit_code = EXIT_FAILURE;
			ಅನುವರ್ತ;
		}

		crc = IS_CKSUM ? 0 : 0xffffffff;
		IF_CKSUM(filesize = 0;)
#define read_buf bb_common_bufsiz1
		ಪ್ರತಿ (;;) {
			ಪೂರ್ಣಾಙ್ಕ bytes_read = safe_read(fd, read_buf, COMMON_BUFSIZE);
			ಯದಿ (bytes_read < 0)
				bb_simple_perror_ಸಂದೇಶ_and_die(fname);
			ಯದಿ (bytes_read > 0) {
				IF_CKSUM(filesize += bytes_read;)
			} ಅನ್ಯಥಾ {
				IF_CKSUM(uoff_t t;)

				close(fd);
				ಯದಿ (IS_CRC32)
					ಭಙ್ಗ;
#ಯದಿ ENABLE_CKSUM
				fd = -1; /* break flag */
				/* Checksum filesize bytes, LSB first */
				t = filesize;
				/*bytes_read = 0; - already is */
				ಯಾವತ್ (t != 0) {
					read_buf[bytes_read++] = (uint8_t)t;
					t >>= 8;
				}
#endif
			}
			crc = (IS_CKSUM ? crc32_block_endian1 : crc32_block_endian0)(crc, read_buf, bytes_read, crc32_table);
			ಯದಿ (ENABLE_CKSUM && fd < 0)
				ಭಙ್ಗ;
		}

		crc = ~crc;
#ಯದಿ ENABLE_CKSUM
		ಯದಿ (IS_CKSUM)
			printf((*argv ? "%u %"OFF_FMT"u %s\n" : "%u %"OFF_FMT"u\n"),
				(unsigned)crc, filesize, *argv);
		ಅನ್ಯಥಾ
#endif
			printf((*argv ? "%08x %s\n" : "%08x\n"),
				(unsigned)crc, *argv);
	} ಯಾವತ್ (*argv && *++argv);

	fflush_stdout_and_exit(exit_code);
}
