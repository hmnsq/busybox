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

#ವ್ಯಾಖ್ಯಾನಿಸು IS_CKSUM (ENABLE_CKSUM && (!ENABLE_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32 || applet_name[1] == 'k'))
#ವ್ಯಾಖ್ಯಾನಿಸು IS_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32 (ENABLE_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32 && (!ENABLE_CKSUM || applet_name[1] == 'r'))

ಪೂರ್ಣಾಂಕ cksum_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ cksum_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	uint32_t *ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_ಕೋಷ್ಟಕ = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_filltable(ಖಾಲಿ, IS_CKSUM);
	exitcode_t exit_ಸಂಕೇತ = EXIT_SUCCESS;

#ಆದರೆ ENABLE_DESKTOP
	getopt32(argv, ""); /* cksum coreutils 6.9 compat */
	argv += optind;
#ಇಲ್ಲದಿದ್ದರೆ
	argv++;
#ಆದರೆ_ಅಂತ್ಯ

	ಸಿದ್ಧತೆ_common_bufsiz();
	ಮಾಡು {
		uint32_t ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ;
		IF_CKSUM(off_t filesize;)
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname = *argv ? *argv : bb_ಸಂದೇಶ_standard_ಒಳಹರಿವು;
		ಪೂರ್ಣಾಂಕ fd = open_or_warn_stdin(fname);

		ಆದರೆ (fd < 0) {
			exit_ಸಂಕೇತ = EXIT_FAILURE;
			ಮುಂದುವರಿ;
		}

		ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = IS_CKSUM ? 0 : 0xffffffff;
		IF_CKSUM(filesize = 0;)
#ವ್ಯಾಖ್ಯಾನಿಸು read_buf bb_common_bufsiz1
		ಗಾಗಿ (;;) {
			ಪೂರ್ಣಾಂಕ ಬೈಟುಗಳು_read = ಸುರಕ್ಷಿತ_read(fd, read_buf, COMMON_BUFSIZE);
			ಆದರೆ (ಬೈಟುಗಳು_read < 0)
				bb_simple_perror_ಸಂದೇಶ_and_die(fname);
			ಆದರೆ (ಬೈಟುಗಳು_read > 0) {
				IF_CKSUM(filesize += ಬೈಟುಗಳು_read;)
			} ಇಲ್ಲದಿದ್ದರೆ {
				IF_CKSUM(uoff_t t;)

				close(fd);
				ಆದರೆ (IS_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32)
					ನಿಲ್ಲಿಸು;
#ಆದರೆ ENABLE_CKSUM
				fd = -1; /* break flag */
				/* Checksum filesize bytes, LSB first */
				t = filesize;
				/*bytes_read = 0; - already is */
				ಆಗ (t != 0) {
					read_buf[ಬೈಟುಗಳು_read++] = (uint8_t)t;
					t >>= 8;
				}
#ಆದರೆ_ಅಂತ್ಯ
			}
			ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = (IS_CKSUM ? ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_ಖಂಡ_endian1 : ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_ಖಂಡ_endian0)(ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ, read_buf, ಬೈಟುಗಳು_read, ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_ಕೋಷ್ಟಕ);
			ಆದರೆ (ENABLE_CKSUM && fd < 0)
				ನಿಲ್ಲಿಸು;
		}

		ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = ~ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ;
#ಆದರೆ ENABLE_CKSUM
		ಆದರೆ (IS_CKSUM)
			ಬರೆ((*argv ? "%u %"OFF_FMT"u %s\n" : "%u %"OFF_FMT"u\n"),
				(ಚಿಹ್ನೆರಹಿತ)ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ, filesize, *argv);
		ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
			ಬರೆ((*argv ? "%08x %s\n" : "%08x\n"),
				(ಚಿಹ್ನೆರಹಿತ)ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ, *argv);
	} ಆಗ (*argv && *++argv);

	fflush_stdout_and_exit(exit_ಸಂಕೇತ);
}
