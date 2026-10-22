/* vi: set sw=4 ts=4: */
/*
 * Copyright (C) 2003 Glenn L. McGrath
 * Copyright (C) 2003-2004 Erik Andersen
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config MD5SUM
//config:	bool "md5sum (6.7 kb)"
//config:	default y
//config:	help
//config:	Compute and check MD5 message digest
//config:
//config:config SHA1SUM
//config:	bool "sha1sum (6.7 kb)"
//config:	default y
//config:	help
//config:	Compute and check SHA1 message digest
//config:
//config:config SHA256SUM
//config:	bool "sha256sum (8.2 kb)"
//config:	default y
//config:	help
//config:	Compute and check SHA256 message digest
//config:
//config:config SHA512SUM
//config:	bool "sha512sum (7.3 kb)"
//config:	default y
//config:	help
//config:	Compute and check SHA512 message digest
//config:
//config:config SHA3SUM
//config:	bool "sha3sum (6.3 kb)"
//config:	default y
//config:	help
//config:	Compute and check SHA3 message digest
//config:
//config:comment "Common options for md5sum, sha1sum, sha256sum, sha512sum, sha3sum"
//config:	depends on MD5SUM || SHA1SUM || SHA256SUM || SHA512SUM || SHA3SUM
//config:
//config:config FEATURE_MD5_SHA1_SUM_CHECK
//config:	bool "Enable -c, -s and -w options"
//config:	default y
//config:	depends on MD5SUM || SHA1SUM || SHA256SUM || SHA512SUM || SHA3SUM
//config:	help
//config:	Enabling the -c options allows files to be checked
//config:	against pre-calculated hash values.
//config:	-s and -w are useful options when verifying checksums.

//applet:IF_MD5SUM(APPLET_NOEXEC(md5sum, md5_sha1_sum, BB_DIR_USR_BIN, BB_SUID_DROP, md5sum))
//applet:IF_SHA1SUM(APPLET_NOEXEC(sha1sum, md5_sha1_sum, BB_DIR_USR_BIN, BB_SUID_DROP, sha1sum))
//applet:IF_SHA3SUM(APPLET_NOEXEC(sha3sum, md5_sha1_sum, BB_DIR_USR_BIN, BB_SUID_DROP, sha3sum))
//applet:IF_SHA256SUM(APPLET_NOEXEC(sha256sum, md5_sha1_sum, BB_DIR_USR_BIN, BB_SUID_DROP, sha256sum))
//applet:IF_SHA512SUM(APPLET_NOEXEC(sha512sum, md5_sha1_sum, BB_DIR_USR_BIN, BB_SUID_DROP, sha512sum))

//kbuild:lib-$(CONFIG_MD5SUM)    += md5_sha1_sum.o
//kbuild:lib-$(CONFIG_SHA1SUM)   += md5_sha1_sum.o
//kbuild:lib-$(CONFIG_SHA256SUM) += md5_sha1_sum.o
//kbuild:lib-$(CONFIG_SHA512SUM) += md5_sha1_sum.o
//kbuild:lib-$(CONFIG_SHA3SUM)   += md5_sha1_sum.o

//usage:#define md5sum_trivial_usage
//usage:	IF_FEATURE_MD5_SHA1_SUM_CHECK("[-c[sw]] ")"[FILE]..."
//usage:#define md5sum_full_usage "\n\n"
//usage:       "Print" IF_FEATURE_MD5_SHA1_SUM_CHECK(" or check") " MD5 checksums"
//usage:	IF_FEATURE_MD5_SHA1_SUM_CHECK( "\n"
//usage:     "\n	-c	Check sums against list in FILEs"
//usage:     "\n	-s	Don't output anything, status code shows success"
//usage:     "\n	-w	Warn about improperly formatted checksum lines"
//usage:	)
//usage:
//usage:#define md5sum_example_usage
//usage:       "$ md5sum < busybox\n"
//usage:       "6fd11e98b98a58f64ff3398d7b324003\n"
//usage:       "$ md5sum busybox\n"
//usage:       "6fd11e98b98a58f64ff3398d7b324003  busybox\n"
//usage:       "$ md5sum -c -\n"
//usage:       "6fd11e98b98a58f64ff3398d7b324003  busybox\n"
//usage:       "busybox: OK\n"
//usage:       "^D\n"
//usage:
//usage:#define sha1sum_trivial_usage
//usage:	IF_FEATURE_MD5_SHA1_SUM_CHECK("[-c[sw]] ")"[FILE]..."
//usage:#define sha1sum_full_usage "\n\n"
//usage:       "Print" IF_FEATURE_MD5_SHA1_SUM_CHECK(" or check") " SHA1 checksums"
//usage:	IF_FEATURE_MD5_SHA1_SUM_CHECK( "\n"
//usage:     "\n	-c	Check sums against list in FILEs"
//usage:     "\n	-s	Don't output anything, status code shows success"
//usage:     "\n	-w	Warn about improperly formatted checksum lines"
//usage:	)
//usage:
//usage:#define sha256sum_trivial_usage
//usage:	IF_FEATURE_MD5_SHA1_SUM_CHECK("[-c[sw]] ")"[FILE]..."
//usage:#define sha256sum_full_usage "\n\n"
//usage:       "Print" IF_FEATURE_MD5_SHA1_SUM_CHECK(" or check") " SHA256 checksums"
//usage:	IF_FEATURE_MD5_SHA1_SUM_CHECK( "\n"
//usage:     "\n	-c	Check sums against list in FILEs"
//usage:     "\n	-s	Don't output anything, status code shows success"
//usage:     "\n	-w	Warn about improperly formatted checksum lines"
//usage:	)
//usage:
//usage:#define sha512sum_trivial_usage
//usage:	IF_FEATURE_MD5_SHA1_SUM_CHECK("[-c[sw]] ")"[FILE]..."
//usage:#define sha512sum_full_usage "\n\n"
//usage:       "Print" IF_FEATURE_MD5_SHA1_SUM_CHECK(" or check") " SHA512 checksums"
//usage:	IF_FEATURE_MD5_SHA1_SUM_CHECK( "\n"
//usage:     "\n	-c	Check sums against list in FILEs"
//usage:     "\n	-s	Don't output anything, status code shows success"
//usage:     "\n	-w	Warn about improperly formatted checksum lines"
//usage:	)
//usage:
//usage:#define sha3sum_trivial_usage
//usage:	IF_FEATURE_MD5_SHA1_SUM_CHECK("[-c[sw]] ")"[-a BITS] [FILE]..."
//usage:#define sha3sum_full_usage "\n\n"
//usage:       "Print" IF_FEATURE_MD5_SHA1_SUM_CHECK(" or check") " SHA3 checksums"
//usage:	IF_FEATURE_MD5_SHA1_SUM_CHECK( "\n"
//usage:     "\n	-c	Check sums against list in FILEs"
//usage:     "\n	-s	Don't output anything, status code shows success"
//usage:     "\n	-w	Warn about improperly formatted checksum lines"
//usage:	)
//usage:     "\n	-a BITS	224 (default), 256, 384, 512"

//FIXME: GNU coreutils 8.25 has no -s option, it has only these two long opts:
// --quiet   don't print OK for each successfully verified file
// --status  don't output anything, status code shows success

#अन्तर्भाव "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

गणना {
	/* 4th letter of applet_name is... */
	HASH_MD5 = 's', /* "md5>s<um" */
	HASH_SHA1 = '1',
	HASH_SHA256 = '2',
	HASH_SHA3 = '3',
	HASH_SHA512 = '5',
};

#परिभाषय FLAG_SILENT  1
#परिभाषय FLAG_CHECK   2
#परिभाषय FLAG_WARN    4

/* This might be useful elsewhere */
स्थिर अचिह्नित वर्ण *hash_bin_गन्तव्यम्_hex(अचिह्नित वर्ण *hash_मानम्,
				अचिह्नित hash_दीर्घता)
{
	/* xzalloc zero-terminates */
	वर्ण *hex_मानम् = xzalloc((hash_दीर्घता * 2) + 1);
	bin2hex(hex_मानम्, (वर्ण*)hash_मानम्, hash_दीर्घता);
	प्रत्यागम (अचिह्नित वर्ण *)hex_मानम्;
}

#परिभाषय BUFSZ (CONFIG_FEATURE_COPYBUF_KB < 4 ? 4096 : CONFIG_FEATURE_COPYBUF_KB * 1024)

#यदि !ENABLE_SHA3_योग
# define hash_file(b,f,w) hash_file(b,f)
#यदि_अन्तः
स्थिर uint8_t *hash_file(अचिह्नित वर्ण *अन्तर्_buf, ध्रुव वर्ण *filename, अचिह्नित sha3_विस्तारः)
{
	पूर्णाङ्क src_fd, hash_len, गणनम्;
	सङ्घ _ctx_ {
		sha3_ctx_t sha3;
		sha512_ctx_t sha512;
		sha256_ctx_t sha256;
		sha1_ctx_t sha1;
		md5_ctx_t md5;
	} context;
	uint8_t *hash_मानम्;
	शून्य FAST_FUNC (*update)(शून्य*, ध्रुव शून्य*, आकार_t);
	अचिह्नित FAST_FUNC (*final)(शून्य*, शून्य*);
	वर्ण hash_algo;

	src_fd = open_or_warn_stdin(filename);
	यदि (src_fd < 0) {
		प्रत्यागम रिक्त;
	}

	hash_algo = applet_name[3];

	/* figure specific hash algorithms */
	यदि (ENABLE_MD5_योग && hash_algo == HASH_MD5) {
		md5_begin(&context.md5);
		update = (शून्य*)md5_hash;
		final = (शून्य*)md5_अन्तः;
		hash_len = 16;
	}
	अन्यथा यदि (ENABLE_SHA1_योग && hash_algo == HASH_SHA1) {
		sha1_begin(&context.sha1);
		update = (शून्य*)sha1_hash;
		final = (शून्य*)sha1_अन्तः;
		hash_len = 20;
	}
	अन्यथा यदि (ENABLE_SHA256_योग && hash_algo == HASH_SHA256) {
		sha256_begin(&context.sha256);
		update = (शून्य*)sha256_hash;
		final = (शून्य*)sha256_अन्तः;
		hash_len = 32;
	}
	अन्यथा यदि (ENABLE_SHA512_योग && hash_algo == HASH_SHA512) {
		sha512_begin(&context.sha512);
		update = (शून्य*)sha512_hash;
		final = (शून्य*)sha512_अन्तः;
		hash_len = 64;
	}
#यदि ENABLE_SHA3_योग
	अन्यथा यदि (ENABLE_SHA3_योग && hash_algo == HASH_SHA3) {
		sha3_begin(&context.sha3);
		update = (शून्य*)sha3_hash;
		final = (शून्य*)sha3_अन्तः;
		/*
		 * Should support 224, 256, 384, 512.
		 * We allow any value which does not blow the algorithm up.
		 */
		यदि (sha3_विस्तारः >= 1600/2 /* input block can't be <= 0 */
		 || sha3_विस्तारः == 0      /* hash len can't be 0 */
		 || (sha3_विस्तारः & 0x1f)  /* should be multiple of 32 */
		/* (because input uses up to 8 byte wide word XORs. 32/4=8) */
		) {
			bb_error_सन्देशः_and_die("bad -a%u", sha3_विस्तारः);
		}
		sha3_विस्तारः /= 4;
		context.sha3.निवेशः_खण्डम्_अष्टकानि = 1600/8 - sha3_विस्तारः;
		hash_len = sha3_विस्तारः/2;
	}
#यदि_अन्तः
	अन्यथा {
		xfunc_die(); /* can't reach this */
	}

	{
		यावत् ((गणनम् = safe_read(src_fd, अन्तर्_buf, BUFSZ)) > 0) {
			update(&context, अन्तर्_buf, गणनम्);
		}
		hash_मानम् = रिक्त;
		यदि (गणनम् < 0)
			bb_perror_सन्देशः("can't read '%s'", filename);
		अन्यथा /* count == 0 */ {
			final(&context, अन्तर्_buf);
			hash_मानम् = hash_bin_गन्तव्यम्_hex(अन्तर्_buf, hash_len);
		}
	}

	यदि (src_fd != STDIN_FILENO) {
		close(src_fd);
	}

	प्रत्यागम hash_मानम्;
}

पूर्णाङ्क md5_sha1_योग_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क md5_sha1_योग_main(पूर्णाङ्क argc अप्रयुक्तम्_PARAM, वर्ण **argv)
{
	अचिह्नित वर्ण *अन्तर्_buf;
	पूर्णाङ्क return_मानम् = EXIT_SUCCESS;
	अचिह्नित flags;
#यदि ENABLE_SHA3_योग
	अचिह्नित sha3_विस्तारः = 224;
#यदि_अन्तः

	यदि (ENABLE_FEATURE_MD5_SHA1_योग_CHECK) {
		/* -b "binary", -t "text" are ignored (shaNNNsum compat) */
		/* -s and -w require -c */
#यदि ENABLE_SHA3_योग
		यदि (applet_name[3] == HASH_SHA3)
			flags = getopt32(argv, "^" "scwbta:+" "\0" "s?c:w?c", &sha3_विस्तारः);
		अन्यथा
#यदि_अन्तः
			flags = getopt32(argv, "^" "scwbt" "\0" "s?c:w?c");
	} अन्यथा {
#यदि ENABLE_SHA3_योग
		यदि (applet_name[3] == HASH_SHA3)
			getopt32(argv, "a:+", &sha3_विस्तारः);
		अन्यथा
#यदि_अन्तः
			getopt32(argv, "");
	}
	argv += optind;
	//argc -= optind;
	यदि (!*argv)
		*--argv = (वर्ण*)"-";

	/* The buffer is not alloc/freed for each input file:
	 * for big values of COPYBUF_KB, this helps to keep its pages
	 * pre-faulted and possibly even fully cached on local CPU.
	 */
	अन्तर्_buf = xmalloc(BUFSZ);

	कुरु {
		यदि (ENABLE_FEATURE_MD5_SHA1_योग_CHECK && (flags & FLAG_CHECK)) {
			FILE *pre_computed_प्रवाहः;
			वर्ण *पङ्क्तिः;
			पूर्णाङ्क गणनम्_योग = 0;
			पूर्णाङ्क गणनम्_failed = 0;

			pre_computed_प्रवाहः = xfopen_stdin(*argv);

			यावत् ((पङ्क्तिः = xmalloc_fgetline(pre_computed_प्रवाहः)) != रिक्त) {
				uint8_t *hash_मानम्;
				वर्ण *filename_ptr;

				गणनम्_योग++;
				filename_ptr = strchr(पङ्क्तिः, ' ');
				यदि (!filename_ptr) {
					यदि (flags & FLAG_WARN) {
						bb_simple_error_सन्देशः("invalid format");
					}
					गणनम्_failed++;
					return_मानम् = EXIT_FAILURE;
					मुक्त(पङ्क्तिः);
					अनुवर्त;
				}
				*filename_ptr++ = '\0';
				/* coreutils 9.1 allows "HASH FILENAME" format,
				 * with only one space. Skip the 'correct'
				 * "  " or " *" delimiter if it is there:
				 */
				यदि (*filename_ptr == ' ' || *filename_ptr == '*')
					filename_ptr++;

				hash_मानम् = hash_file(अन्तर्_buf, filename_ptr, sha3_विस्तारः);

				यदि (hash_मानम् && (strcasecmp((वर्ण*)hash_मानम्, पङ्क्तिः) == 0)) {
					यदि (!(flags & FLAG_SILENT))
						प्रारूप_लिख("%s: OK\n", filename_ptr);
				} अन्यथा {
					यदि (!(flags & FLAG_SILENT))
						प्रारूप_लिख("%s: FAILED\n", filename_ptr);
					गणनम्_failed++;
					return_मानम् = EXIT_FAILURE;
				}
				/* possible free(NULL) */
				मुक्त(hash_मानम्);
				मुक्त(पङ्क्तिः);
			}
			यदि (गणनम्_failed && !(flags & FLAG_SILENT)) {
				bb_error_सन्देशः("WARNING: %d of %d computed checksums did NOT match",
						गणनम्_failed, गणनम्_योग);
			}
			यदि (गणनम्_योग == 0) {
				return_मानम् = EXIT_FAILURE;
				/*
				 * md5sum from GNU coreutils 8.25 says:
				 * md5sum: <FILE>: no properly formatted MD5 checksum lines found
				 */
				bb_error_सन्देशः("%s: no checksum lines found", *argv);
			}
			fclose_if_not_stdin(pre_computed_प्रवाहः);
		} अन्यथा {
			uint8_t *hash_मानम् = hash_file(अन्तर्_buf, *argv, sha3_विस्तारः);
			यदि (hash_मानम् == रिक्त) {
				return_मानम् = EXIT_FAILURE;
			} अन्यथा {
				प्रारूप_लिख("%s  %s\n", hash_मानम्, *argv);
				मुक्त(hash_मानम्);
			}
		}
	} यावत् (*++argv);

	प्रत्यागम return_मानम्;
}
