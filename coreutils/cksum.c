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

#ضمن "libbb.h"
#ضمن "common_bufsiz.h"

/* This is a NOEXEC applet. Be very careful! */

#وصف_ڪريو IS_CKSUM (ENABLE_CKSUM && (!ENABLE_چڪري_جاچ_32 || applet_name[1] == 'k'))
#وصف_ڪريو IS_چڪري_جاچ_32 (ENABLE_چڪري_جاچ_32 && (!ENABLE_CKSUM || applet_name[1] == 'r'))

صحيح cksum_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح cksum_main(صحيح argc اڻ_استعمال_ٿيل_PARAM, حرف **argv)
{
	uint32_t *چڪري_جاچ_32_جدول = چڪري_جاچ_32_filltable(عدم, IS_CKSUM);
	exitcode_t exit_ڪوڊ = EXIT_SUCCESS;

#إذا ENABLE_DESKTOP
	getopt32(argv, ""); /* cksum coreutils 6.9 compat */
	argv += optind;
#وإلا
	argv++;
#جيڪڏهن_پورو

	تياري_common_bufsiz();
	افعل {
		uint32_t چڪري_جاچ;
		IF_CKSUM(off_t filesize;)
		ثابت حرف *fname = *argv ? *argv : bb_نياپو_standard_داخلا;
		صحيح fd = open_or_warn_stdin(fname);

		إذا (fd < 0) {
			exit_ڪوڊ = EXIT_FAILURE;
			استمر;
		}

		چڪري_جاچ = IS_CKSUM ? 0 : 0xffffffff;
		IF_CKSUM(filesize = 0;)
#وصف_ڪريو read_buf bb_common_bufsiz1
		لكل (;;) {
			صحيح بائيٽون_read = محفوظ_read(fd, read_buf, COMMON_BUFSIZE);
			إذا (بائيٽون_read < 0)
				bb_simple_perror_نياپو_and_die(fname);
			إذا (بائيٽون_read > 0) {
				IF_CKSUM(filesize += بائيٽون_read;)
			} وإلا {
				IF_CKSUM(uoff_t t;)

				close(fd);
				إذا (IS_چڪري_جاچ_32)
					اكسر;
#إذا ENABLE_CKSUM
				fd = -1; /* break flag */
				/* Checksum filesize bytes, LSB first */
				t = filesize;
				/*bytes_read = 0; - already is */
				طالما (t != 0) {
					read_buf[بائيٽون_read++] = (uint8_t)t;
					t >>= 8;
				}
#جيڪڏهن_پورو
			}
			چڪري_جاچ = (IS_CKSUM ? چڪري_جاچ_32_بلاڪ_endian1 : چڪري_جاچ_32_بلاڪ_endian0)(چڪري_جاچ, read_buf, بائيٽون_read, چڪري_جاچ_32_جدول);
			إذا (ENABLE_CKSUM && fd < 0)
				اكسر;
		}

		چڪري_جاچ = ~چڪري_جاچ;
#إذا ENABLE_CKSUM
		إذا (IS_CKSUM)
			لکو((*argv ? "%u %"OFF_FMT"u %s\n" : "%u %"OFF_FMT"u\n"),
				(بنانشان)چڪري_جاچ, filesize, *argv);
		وإلا
#جيڪڏهن_پورو
			لکو((*argv ? "%08x %s\n" : "%08x\n"),
				(بنانشان)چڪري_جاچ, *argv);
	} طالما (*argv && *++argv);

	fflush_stdout_and_exit(exit_ڪوڊ);
}
