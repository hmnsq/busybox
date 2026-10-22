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

#അന്തര്ഭാവ "libbb.h"
#അന്തര്ഭാവ "common_bufsiz.h"

/* This is a NOEXEC applet. Be very careful! */

#define IS_CKSUM (ENABLE_CKSUM && (!ENABLE_CRC32 || applet_name[1] == 'k'))
#define IS_CRC32 (ENABLE_CRC32 && (!ENABLE_CKSUM || applet_name[1] == 'r'))

പൂർണ്ണസംഖ്യ cksum_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) MAIN_EXTERNALLY_VISIBLE;
പൂർണ്ണസംഖ്യ cksum_main(പൂർണ്ണസംഖ്യ argc UNUSED_PARAM, അക്ഷരം **argv)
{
	uint32_t *crc32_table = crc32_filltable(ശൂന്യ, IS_CKSUM);
	exitcode_t exit_code = EXIT_SUCCESS;

#എങ്കിൽ ENABLE_DESKTOP
	getopt32(argv, ""); /* cksum coreutils 6.9 compat */
	argv += optind;
#അല്ലെങ്കിൽ
	argv++;
#endif

	setup_common_bufsiz();
	ചെയ്യുക {
		uint32_t crc;
		IF_CKSUM(off_t filesize;)
		സ്ഥിരം അക്ഷരം *fname = *argv ? *argv : bb_സന്ദേശം_standard_നിവേശം;
		പൂർണ്ണസംഖ്യ fd = open_or_warn_stdin(fname);

		എങ്കിൽ (fd < 0) {
			exit_code = EXIT_FAILURE;
			തുടരുക;
		}

		crc = IS_CKSUM ? 0 : 0xffffffff;
		IF_CKSUM(filesize = 0;)
#define read_buf bb_common_bufsiz1
		വേണ്ടി (;;) {
			പൂർണ്ണസംഖ്യ bytes_read = safe_read(fd, read_buf, COMMON_BUFSIZE);
			എങ്കിൽ (bytes_read < 0)
				bb_simple_perror_സന്ദേശം_and_die(fname);
			എങ്കിൽ (bytes_read > 0) {
				IF_CKSUM(filesize += bytes_read;)
			} അല്ലെങ്കിൽ {
				IF_CKSUM(uoff_t t;)

				close(fd);
				എങ്കിൽ (IS_CRC32)
					നിർത്തുക;
#എങ്കിൽ ENABLE_CKSUM
				fd = -1; /* break flag */
				/* Checksum filesize bytes, LSB first */
				t = filesize;
				/*bytes_read = 0; - already is */
				ആകുമ്പോൾ (t != 0) {
					read_buf[bytes_read++] = (uint8_t)t;
					t >>= 8;
				}
#endif
			}
			crc = (IS_CKSUM ? crc32_block_endian1 : crc32_block_endian0)(crc, read_buf, bytes_read, crc32_table);
			എങ്കിൽ (ENABLE_CKSUM && fd < 0)
				നിർത്തുക;
		}

		crc = ~crc;
#എങ്കിൽ ENABLE_CKSUM
		എങ്കിൽ (IS_CKSUM)
			printf((*argv ? "%u %"OFF_FMT"u %s\n" : "%u %"OFF_FMT"u\n"),
				(unsigned)crc, filesize, *argv);
		അല്ലെങ്കിൽ
#endif
			printf((*argv ? "%08x %s\n" : "%08x\n"),
				(unsigned)crc, *argv);
	} ആകുമ്പോൾ (*argv && *++argv);

	fflush_stdout_and_exit(exit_code);
}
