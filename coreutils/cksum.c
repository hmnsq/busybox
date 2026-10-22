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

#شامل "libbb.h"
#شامل "common_bufsiz.h"

/* This is a NOEXEC applet. Be very careful! */

#تعریف_کرو IS_CKSUM (ENABLE_CKSUM && (!ENABLE_چکری_جانچ_32 || applet_name[1] == 'k'))
#تعریف_کرو IS_چکری_جانچ_32 (ENABLE_چکری_جانچ_32 && (!ENABLE_CKSUM || applet_name[1] == 'r'))

صحیح cksum_main(صحیح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح cksum_main(صحیح argc غیر_مستعمل_PARAM, حرف **argv)
{
	uint32_t *چکری_جانچ_32_جدول = چکری_جانچ_32_filltable(تہی, IS_CKSUM);
	exitcode_t exit_رمز = EXIT_SUCCESS;

#اگر ENABLE_DESKTOP
	getopt32(argv, ""); /* cksum coreutils 6.9 compat */
	argv += optind;
#ورنہ
	argv++;
#اگر_ختم

	تیاری_common_bufsiz();
	کرو {
		uint32_t چکری_جانچ;
		IF_CKSUM(off_t filesize;)
		ثابت حرف *fname = *argv ? *argv : bb_پیغام_standard_داخلہ;
		صحیح fd = open_or_warn_stdin(fname);

		اگر (fd < 0) {
			exit_رمز = EXIT_FAILURE;
			جاری;
		}

		چکری_جانچ = IS_CKSUM ? 0 : 0xffffffff;
		IF_CKSUM(filesize = 0;)
#تعریف_کرو read_buf bb_common_bufsiz1
		برائے (;;) {
			صحیح بائٹیں_read = محفوظ_read(fd, read_buf, COMMON_BUFSIZE);
			اگر (بائٹیں_read < 0)
				bb_simple_perror_پیغام_and_die(fname);
			اگر (بائٹیں_read > 0) {
				IF_CKSUM(filesize += بائٹیں_read;)
			} ورنہ {
				IF_CKSUM(uoff_t t;)

				close(fd);
				اگر (IS_چکری_جانچ_32)
					توڑو;
#اگر ENABLE_CKSUM
				fd = -1; /* break flag */
				/* Checksum filesize bytes, LSB first */
				t = filesize;
				/*bytes_read = 0; - already is */
				جب (t != 0) {
					read_buf[بائٹیں_read++] = (uint8_t)t;
					t >>= 8;
				}
#اگر_ختم
			}
			چکری_جانچ = (IS_CKSUM ? چکری_جانچ_32_بلاک_endian1 : چکری_جانچ_32_بلاک_endian0)(چکری_جانچ, read_buf, بائٹیں_read, چکری_جانچ_32_جدول);
			اگر (ENABLE_CKSUM && fd < 0)
				توڑو;
		}

		چکری_جانچ = ~چکری_جانچ;
#اگر ENABLE_CKSUM
		اگر (IS_CKSUM)
			لکھو((*argv ? "%u %"OFF_FMT"u %s\n" : "%u %"OFF_FMT"u\n"),
				(غیرعلامتی)چکری_جانچ, filesize, *argv);
		ورنہ
#اگر_ختم
			لکھو((*argv ? "%08x %s\n" : "%08x\n"),
				(غیرعلامتی)چکری_جانچ, *argv);
	} جب (*argv && *++argv);

	fflush_stdout_and_exit(exit_رمز);
}
