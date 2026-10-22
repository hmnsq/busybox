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

#அந்தர்பாவ "libbb.h"
#அந்தர்பாவ "common_bufsiz.h"

/* This is a NOEXEC applet. Be very careful! */

#define IS_CKSUM (ENABLE_CKSUM && (!ENABLE_CRC32 || applet_name[1] == 'k'))
#define IS_CRC32 (ENABLE_CRC32 && (!ENABLE_CKSUM || applet_name[1] == 'r'))

முழு cksum_main(முழு argc, எழுத்து **argv) MAIN_EXTERNALLY_VISIBLE;
முழு cksum_main(முழு argc UNUSED_PARAM, எழுத்து **argv)
{
	uint32_t *crc32_table = crc32_filltable(வெற்று, IS_CKSUM);
	exitcode_t exit_code = EXIT_SUCCESS;

#எனில் ENABLE_DESKTOP
	getopt32(argv, ""); /* cksum coreutils 6.9 compat */
	argv += optind;
#இல்லையெனில்
	argv++;
#endif

	setup_common_bufsiz();
	செய் {
		uint32_t crc;
		IF_CKSUM(off_t filesize;)
		மாறிலி எழுத்து *fname = *argv ? *argv : bb_செய்தி_standard_உள்ளீடு;
		முழு fd = open_or_warn_stdin(fname);

		எனில் (fd < 0) {
			exit_code = EXIT_FAILURE;
			தொடர்;
		}

		crc = IS_CKSUM ? 0 : 0xffffffff;
		IF_CKSUM(filesize = 0;)
#define read_buf bb_common_bufsiz1
		க்கு (;;) {
			முழு bytes_read = safe_read(fd, read_buf, COMMON_BUFSIZE);
			எனில் (bytes_read < 0)
				bb_simple_perror_செய்தி_and_die(fname);
			எனில் (bytes_read > 0) {
				IF_CKSUM(filesize += bytes_read;)
			} இல்லையெனில் {
				IF_CKSUM(uoff_t t;)

				close(fd);
				எனில் (IS_CRC32)
					நிறுத்து;
#எனில் ENABLE_CKSUM
				fd = -1; /* break flag */
				/* Checksum filesize bytes, LSB first */
				t = filesize;
				/*bytes_read = 0; - already is */
				போது (t != 0) {
					read_buf[bytes_read++] = (uint8_t)t;
					t >>= 8;
				}
#endif
			}
			crc = (IS_CKSUM ? crc32_block_endian1 : crc32_block_endian0)(crc, read_buf, bytes_read, crc32_table);
			எனில் (ENABLE_CKSUM && fd < 0)
				நிறுத்து;
		}

		crc = ~crc;
#எனில் ENABLE_CKSUM
		எனில் (IS_CKSUM)
			printf((*argv ? "%u %"OFF_FMT"u %s\n" : "%u %"OFF_FMT"u\n"),
				(unsigned)crc, filesize, *argv);
		இல்லையெனில்
#endif
			printf((*argv ? "%08x %s\n" : "%08x\n"),
				(unsigned)crc, *argv);
	} போது (*argv && *++argv);

	fflush_stdout_and_exit(exit_code);
}
