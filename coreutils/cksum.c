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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "common_bufsiz.h"

/* This is a NOEXEC applet. Be very careful! */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IS_CKSUM (ENABLE_CKSUM && (!ENABLE_ਚੱਕਰੀ_ਜਾਂਚ_32 || applet_name[1] == 'k'))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IS_ਚੱਕਰੀ_ਜਾਂਚ_32 (ENABLE_ਚੱਕਰੀ_ਜਾਂਚ_32 && (!ENABLE_CKSUM || applet_name[1] == 'r'))

ਪੂਰਨ cksum_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ cksum_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
	uint32_t *ਚੱਕਰੀ_ਜਾਂਚ_32_ਸਾਰਣੀ = ਚੱਕਰੀ_ਜਾਂਚ_32_filltable(ਨਿਰਾ, IS_CKSUM);
	exitcode_t exit_ਕੋਡ = EXIT_SUCCESS;

#ਜੇ ENABLE_DESKTOP
	getopt32(argv, ""); /* cksum coreutils 6.9 compat */
	argv += optind;
#ਨਹੀਂਤਾਂ
	argv++;
#ਜੇ_ਅੰਤ

	ਤਿਆਰੀ_common_bufsiz();
	ਕਰੋ {
		uint32_t ਚੱਕਰੀ_ਜਾਂਚ;
		IF_CKSUM(off_t filesize;)
		ਅਚਲ ਅੱਖਰ *fname = *argv ? *argv : bb_ਸੁਨੇਹਾ_standard_ਨਿਵੇਸ਼;
		ਪੂਰਨ fd = open_or_warn_stdin(fname);

		ਜੇ (fd < 0) {
			exit_ਕੋਡ = EXIT_FAILURE;
			ਜਾਰੀ;
		}

		ਚੱਕਰੀ_ਜਾਂਚ = IS_CKSUM ? 0 : 0xffffffff;
		IF_CKSUM(filesize = 0;)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ read_buf bb_common_bufsiz1
		ਲਈ (;;) {
			ਪੂਰਨ ਬਾਈਟਾਂ_read = ਸੁਰੱਖਿਅਤ_read(fd, read_buf, COMMON_BUFSIZE);
			ਜੇ (ਬਾਈਟਾਂ_read < 0)
				bb_simple_perror_ਸੁਨੇਹਾ_and_die(fname);
			ਜੇ (ਬਾਈਟਾਂ_read > 0) {
				IF_CKSUM(filesize += ਬਾਈਟਾਂ_read;)
			} ਨਹੀਂਤਾਂ {
				IF_CKSUM(uoff_t t;)

				close(fd);
				ਜੇ (IS_ਚੱਕਰੀ_ਜਾਂਚ_32)
					ਰੋਕੋ;
#ਜੇ ENABLE_CKSUM
				fd = -1; /* break flag */
				/* Checksum filesize bytes, LSB first */
				t = filesize;
				/*bytes_read = 0; - already is */
				ਜਦੋਂ (t != 0) {
					read_buf[ਬਾਈਟਾਂ_read++] = (uint8_t)t;
					t >>= 8;
				}
#ਜੇ_ਅੰਤ
			}
			ਚੱਕਰੀ_ਜਾਂਚ = (IS_CKSUM ? ਚੱਕਰੀ_ਜਾਂਚ_32_ਖੰਡ_endian1 : ਚੱਕਰੀ_ਜਾਂਚ_32_ਖੰਡ_endian0)(ਚੱਕਰੀ_ਜਾਂਚ, read_buf, ਬਾਈਟਾਂ_read, ਚੱਕਰੀ_ਜਾਂਚ_32_ਸਾਰਣੀ);
			ਜੇ (ENABLE_CKSUM && fd < 0)
				ਰੋਕੋ;
		}

		ਚੱਕਰੀ_ਜਾਂਚ = ~ਚੱਕਰੀ_ਜਾਂਚ;
#ਜੇ ENABLE_CKSUM
		ਜੇ (IS_CKSUM)
			ਲਿਖੋ((*argv ? "%u %"OFF_FMT"u %s\n" : "%u %"OFF_FMT"u\n"),
				(ਬਿਨਾਨਿਸ਼ਾਨੀ)ਚੱਕਰੀ_ਜਾਂਚ, filesize, *argv);
		ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
			ਲਿਖੋ((*argv ? "%08x %s\n" : "%08x\n"),
				(ਬਿਨਾਨਿਸ਼ਾਨੀ)ਚੱਕਰੀ_ਜਾਂਚ, *argv);
	} ਜਦੋਂ (*argv && *++argv);

	fflush_stdout_and_exit(exit_ਕੋਡ);
}
