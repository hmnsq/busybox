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

#అన్తర్భావ "libbb.h"
#అన్తర్భావ "common_bufsiz.h"

/* This is a NOEXEC applet. Be very careful! */

#నిర్వచించు IS_CKSUM (ENABLE_CKSUM && (!ENABLE_చక్రీయ_తనిఖీ_32 || applet_name[1] == 'k'))
#నిర్వచించు IS_చక్రీయ_తనిఖీ_32 (ENABLE_చక్రీయ_తనిఖీ_32 && (!ENABLE_CKSUM || applet_name[1] == 'r'))

పూర్ణాంకం cksum_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం cksum_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	uint32_t *చక్రీయ_తనిఖీ_32_పట్టిక = చక్రీయ_తనిఖీ_32_filltable(ఖాళీ, IS_CKSUM);
	exitcode_t exit_సంకేతం = EXIT_SUCCESS;

#ఒకవేళ ENABLE_DESKTOP
	getopt32(argv, ""); /* cksum coreutils 6.9 compat */
	argv += optind;
#లేకపోతే
	argv++;
#అయితే_ముగింపు

	అమరిక_common_bufsiz();
	చేయి {
		uint32_t చక్రీయ_తనిఖీ;
		IF_CKSUM(off_t filesize;)
		స్థిరరాశి అక్షరం *fname = *argv ? *argv : bb_సందేశం_standard_ఆదానం;
		పూర్ణాంకం fd = open_or_warn_stdin(fname);

		ఒకవేళ (fd < 0) {
			exit_సంకేతం = EXIT_FAILURE;
			కొనసాగు;
		}

		చక్రీయ_తనిఖీ = IS_CKSUM ? 0 : 0xffffffff;
		IF_CKSUM(filesize = 0;)
#నిర్వచించు read_buf bb_common_bufsiz1
		కొరకు (;;) {
			పూర్ణాంకం బైట్లు_read = సురక్షిత_read(fd, read_buf, COMMON_BUFSIZE);
			ఒకవేళ (బైట్లు_read < 0)
				bb_simple_perror_సందేశం_and_die(fname);
			ఒకవేళ (బైట్లు_read > 0) {
				IF_CKSUM(filesize += బైట్లు_read;)
			} లేకపోతే {
				IF_CKSUM(uoff_t t;)

				close(fd);
				ఒకవేళ (IS_చక్రీయ_తనిఖీ_32)
					ఆపు;
#ఒకవేళ ENABLE_CKSUM
				fd = -1; /* break flag */
				/* Checksum filesize bytes, LSB first */
				t = filesize;
				/*bytes_read = 0; - already is */
				అయితే (t != 0) {
					read_buf[బైట్లు_read++] = (uint8_t)t;
					t >>= 8;
				}
#అయితే_ముగింపు
			}
			చక్రీయ_తనిఖీ = (IS_CKSUM ? చక్రీయ_తనిఖీ_32_ఖండం_endian1 : చక్రీయ_తనిఖీ_32_ఖండం_endian0)(చక్రీయ_తనిఖీ, read_buf, బైట్లు_read, చక్రీయ_తనిఖీ_32_పట్టిక);
			ఒకవేళ (ENABLE_CKSUM && fd < 0)
				ఆపు;
		}

		చక్రీయ_తనిఖీ = ~చక్రీయ_తనిఖీ;
#ఒకవేళ ENABLE_CKSUM
		ఒకవేళ (IS_CKSUM)
			రాయి((*argv ? "%u %"OFF_FMT"u %s\n" : "%u %"OFF_FMT"u\n"),
				(గుర్తులేని)చక్రీయ_తనిఖీ, filesize, *argv);
		లేకపోతే
#అయితే_ముగింపు
			రాయి((*argv ? "%08x %s\n" : "%08x\n"),
				(గుర్తులేని)చక్రీయ_తనిఖీ, *argv);
	} అయితే (*argv && *++argv);

	fflush_stdout_and_exit(exit_సంకేతం);
}
