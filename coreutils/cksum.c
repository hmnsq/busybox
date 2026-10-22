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

#समावेश "libbb.h"
#समावेश "common_bufsiz.h"

/* This is a NOEXEC applet. Be very careful! */

#घोषणा IS_CKSUM (ENABLE_CKSUM && (!ENABLE_CRC32 || applet_name[1] == 'k'))
#घोषणा IS_CRC32 (ENABLE_CRC32 && (!ENABLE_CKSUM || applet_name[1] == 'r'))

पूर्णांक cksum_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक cksum_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	uint32_t *crc32_table = crc32_filltable(शून्य, IS_CKSUM);
	exitcode_t exit_code = निकास_सफल;

#अगर ENABLE_DESKTOP
	getopt32(argv, ""); /* cksum coreutils 6.9 compat */
	argv += optind;
#अन्यथा
	argv++;
#पूर्ण_अगर

	setup_common_bufsiz();
	करो {
		uint32_t crc;
		IF_CKSUM(off_t filesize;)
		स्थिर अक्षर *fname = *argv ? *argv : bb_संदेश_standard_निवेश;
		पूर्णांक fd = open_or_warn_stdin(fname);

		अगर (fd < 0) {
			exit_code = निकास_त्रुटि;
			जारी;
		}

		crc = IS_CKSUM ? 0 : 0xffffffff;
		IF_CKSUM(filesize = 0;)
#घोषणा read_buf bb_common_bufsiz1
		क्रम (;;) {
			पूर्णांक bytes_read = safe_read(fd, read_buf, COMMON_BUFSIZE);
			अगर (bytes_read < 0)
				bb_simple_perror_संदेश_and_die(fname);
			अगर (bytes_read > 0) {
				IF_CKSUM(filesize += bytes_read;)
			} अन्यथा {
				IF_CKSUM(uoff_t t;)

				बंद(fd);
				अगर (IS_CRC32)
					अवरोध;
#अगर ENABLE_CKSUM
				fd = -1; /* break flag */
				/* Checksum filesize bytes, LSB first */
				t = filesize;
				/*bytes_read = 0; - already is */
				जबतक (t != 0) {
					read_buf[bytes_read++] = (uint8_t)t;
					t >>= 8;
				}
#पूर्ण_अगर
			}
			crc = (IS_CKSUM ? crc32_block_endian1 : crc32_block_endian0)(crc, read_buf, bytes_read, crc32_table);
			अगर (ENABLE_CKSUM && fd < 0)
				अवरोध;
		}

		crc = ~crc;
#अगर ENABLE_CKSUM
		अगर (IS_CKSUM)
			म_लिखो((*argv ? "%u %"OFF_FMT"u %s\n" : "%u %"OFF_FMT"u\n"),
				(अचिन्हित)crc, filesize, *argv);
		अन्यथा
#पूर्ण_अगर
			म_लिखो((*argv ? "%08x %s\n" : "%08x\n"),
				(अचिन्हित)crc, *argv);
	} जबतक (*argv && *++argv);

	fflush_stdout_and_exit(exit_code);
}
