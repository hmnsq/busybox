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

#अन्तर्भाव "libbb.h"
#अन्तर्भाव "common_bufsiz.h"

/* This is a NOEXEC applet. Be very careful! */

#परिभाषय IS_CKSUM (ENABLE_CKSUM && (!ENABLE_चक्रीयपरीक्षा_32 || applet_name[1] == 'k'))
#परिभाषय IS_चक्रीयपरीक्षा_32 (ENABLE_चक्रीयपरीक्षा_32 && (!ENABLE_CKSUM || applet_name[1] == 'r'))

पूर्णाङ्क cksum_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क cksum_main(पूर्णाङ्क argc अप्रयुक्तम्_PARAM, वर्ण **argv)
{
	uint32_t *चक्रीयपरीक्षा_32_सारणी = चक्रीयपरीक्षा_32_filltable(रिक्त, IS_CKSUM);
	exitcode_t exit_संहिता = EXIT_SUCCESS;

#यदि ENABLE_DESKTOP
	getopt32(argv, ""); /* cksum coreutils 6.9 compat */
	argv += optind;
#अन्यथा
	argv++;
#यदि_अन्तः

	सज्जता_common_bufsiz();
	कुरु {
		uint32_t crc;
		IF_CKSUM(off_t filesize;)
		ध्रुव वर्ण *fname = *argv ? *argv : bb_सन्देशः_standard_निवेशः;
		पूर्णाङ्क fd = open_or_warn_stdin(fname);

		यदि (fd < 0) {
			exit_संहिता = EXIT_FAILURE;
			अनुवर्त;
		}

		crc = IS_CKSUM ? 0 : 0xffffffff;
		IF_CKSUM(filesize = 0;)
#परिभाषय read_buf bb_common_bufsiz1
		प्रति (;;) {
			पूर्णाङ्क अष्टकानि_read = safe_read(fd, read_buf, COMMON_BUFSIZE);
			यदि (अष्टकानि_read < 0)
				bb_simple_perror_सन्देशः_and_die(fname);
			यदि (अष्टकानि_read > 0) {
				IF_CKSUM(filesize += अष्टकानि_read;)
			} अन्यथा {
				IF_CKSUM(uoff_t t;)

				close(fd);
				यदि (IS_चक्रीयपरीक्षा_32)
					भङ्ग;
#यदि ENABLE_CKSUM
				fd = -1; /* break flag */
				/* Checksum filesize bytes, LSB first */
				t = filesize;
				/*bytes_read = 0; - already is */
				यावत् (t != 0) {
					read_buf[अष्टकानि_read++] = (uint8_t)t;
					t >>= 8;
				}
#यदि_अन्तः
			}
			crc = (IS_CKSUM ? चक्रीयपरीक्षा_32_खण्डम्_endian1 : चक्रीयपरीक्षा_32_खण्डम्_endian0)(crc, read_buf, अष्टकानि_read, चक्रीयपरीक्षा_32_सारणी);
			यदि (ENABLE_CKSUM && fd < 0)
				भङ्ग;
		}

		crc = ~crc;
#यदि ENABLE_CKSUM
		यदि (IS_CKSUM)
			प्रारूप_लिख((*argv ? "%u %"OFF_FMT"u %s\n" : "%u %"OFF_FMT"u\n"),
				(अचिह्नित)crc, filesize, *argv);
		अन्यथा
#यदि_अन्तः
			प्रारूप_लिख((*argv ? "%08x %s\n" : "%08x\n"),
				(अचिह्नित)crc, *argv);
	} यावत् (*argv && *++argv);

	fflush_stdout_and_exit(exit_संहिता);
}
