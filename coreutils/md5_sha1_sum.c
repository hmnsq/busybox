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

#define FLAG_SILENT  1
#define FLAG_CHECK   2
#define FLAG_WARN    4

/* This might be useful elsewhere */
स्थिर अचिन्हित वर्ण *hash_bin_to_hex(अचिन्हित वर्ण *hash_मान,
				अचिन्हित hash_length)
{
	/* xzalloc zero-terminates */
	वर्ण *hex_मान = xzalloc((hash_length * 2) + 1);
	bin2hex(hex_मान, (वर्ण*)hash_मान, hash_length);
	फर्काऊ (अचिन्हित वर्ण *)hex_मान;
}

#define BUFSZ (CONFIG_FEATURE_COPYBUF_KB < 4 ? 4096 : CONFIG_FEATURE_COPYBUF_KB * 1024)

#यदि !ENABLE_SHA3SUM
# define hash_file(b,f,w) hash_file(b,f)
#endif
स्थिर uint8_t *hash_file(अचिन्हित वर्ण *in_buf, अचर वर्ण *filename, अचिन्हित sha3_width)
{
	पूर्णाङ्क src_fd, hash_len, गन्ती;
	सङ्घ _ctx_ {
		sha3_ctx_t sha3;
		sha512_ctx_t sha512;
		sha256_ctx_t sha256;
		sha1_ctx_t sha1;
		md5_ctx_t md5;
	} context;
	uint8_t *hash_मान;
	शून्य FAST_FUNC (*update)(शून्य*, अचर शून्य*, माप_प्रकार);
	अचिन्हित FAST_FUNC (*final)(शून्य*, शून्य*);
	वर्ण hash_algo;

	src_fd = open_or_warn_stdin(filename);
	यदि (src_fd < 0) {
		फर्काऊ रिक्त;
	}

	hash_algo = applet_name[3];

	/* figure specific hash algorithms */
	यदि (ENABLE_MD5SUM && hash_algo == HASH_MD5) {
		md5_begin(&context.md5);
		update = (शून्य*)md5_hash;
		final = (शून्य*)md5_end;
		hash_len = 16;
	}
	अन्यथा यदि (ENABLE_SHA1SUM && hash_algo == HASH_SHA1) {
		sha1_begin(&context.sha1);
		update = (शून्य*)sha1_hash;
		final = (शून्य*)sha1_end;
		hash_len = 20;
	}
	अन्यथा यदि (ENABLE_SHA256SUM && hash_algo == HASH_SHA256) {
		sha256_begin(&context.sha256);
		update = (शून्य*)sha256_hash;
		final = (शून्य*)sha256_end;
		hash_len = 32;
	}
	अन्यथा यदि (ENABLE_SHA512SUM && hash_algo == HASH_SHA512) {
		sha512_begin(&context.sha512);
		update = (शून्य*)sha512_hash;
		final = (शून्य*)sha512_end;
		hash_len = 64;
	}
#यदि ENABLE_SHA3SUM
	अन्यथा यदि (ENABLE_SHA3SUM && hash_algo == HASH_SHA3) {
		sha3_begin(&context.sha3);
		update = (शून्य*)sha3_hash;
		final = (शून्य*)sha3_end;
		/*
		 * Should support 224, 256, 384, 512.
		 * We allow any value which does not blow the algorithm up.
		 */
		यदि (sha3_width >= 1600/2 /* input block can't be <= 0 */
		 || sha3_width == 0      /* hash len can't be 0 */
		 || (sha3_width & 0x1f)  /* should be multiple of 32 */
		/* (because input uses up to 8 byte wide word XORs. 32/4=8) */
		) {
			bb_error_सन्देश_and_die("bad -a%u", sha3_width);
		}
		sha3_width /= 4;
		context.sha3.आगत_block_bytes = 1600/8 - sha3_width;
		hash_len = sha3_width/2;
	}
#endif
	अन्यथा {
		xfunc_die(); /* can't reach this */
	}

	{
		जबसम्म ((गन्ती = safe_read(src_fd, in_buf, BUFSZ)) > 0) {
			update(&context, in_buf, गन्ती);
		}
		hash_मान = रिक्त;
		यदि (गन्ती < 0)
			bb_perror_सन्देश("can't read '%s'", filename);
		अन्यथा /* count == 0 */ {
			final(&context, in_buf);
			hash_मान = hash_bin_to_hex(in_buf, hash_len);
		}
	}

	यदि (src_fd != STDIN_FILENO) {
		बंद(src_fd);
	}

	फर्काऊ hash_मान;
}

पूर्णाङ्क md5_sha1_sum_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क md5_sha1_sum_main(पूर्णाङ्क argc UNUSED_PARAM, वर्ण **argv)
{
	अचिन्हित वर्ण *in_buf;
	पूर्णाङ्क return_मान = निकास_सफल;
	अचिन्हित flags;
#यदि ENABLE_SHA3SUM
	अचिन्हित sha3_width = 224;
#endif

	यदि (ENABLE_FEATURE_MD5_SHA1_SUM_CHECK) {
		/* -b "binary", -t "text" are ignored (shaNNNsum compat) */
		/* -s and -w require -c */
#यदि ENABLE_SHA3SUM
		यदि (applet_name[3] == HASH_SHA3)
			flags = getopt32(argv, "^" "scwbta:+" "\0" "s?c:w?c", &sha3_width);
		अन्यथा
#endif
			flags = getopt32(argv, "^" "scwbt" "\0" "s?c:w?c");
	} अन्यथा {
#यदि ENABLE_SHA3SUM
		यदि (applet_name[3] == HASH_SHA3)
			getopt32(argv, "a:+", &sha3_width);
		अन्यथा
#endif
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
	in_buf = xmalloc(BUFSZ);

	गर {
		यदि (ENABLE_FEATURE_MD5_SHA1_SUM_CHECK && (flags & FLAG_CHECK)) {
			खाता *pre_computed_stream;
			वर्ण *पङ्क्ति;
			पूर्णाङ्क गन्ती_total = 0;
			पूर्णाङ्क गन्ती_failed = 0;

			pre_computed_stream = xfopen_stdin(*argv);

			जबसम्म ((पङ्क्ति = xmalloc_fgetline(pre_computed_stream)) != रिक्त) {
				uint8_t *hash_मान;
				वर्ण *filename_ptr;

				गन्ती_total++;
				filename_ptr = म_अक्षर(पङ्क्ति, ' ');
				यदि (!filename_ptr) {
					यदि (flags & FLAG_WARN) {
						bb_simple_error_सन्देश("invalid format");
					}
					गन्ती_failed++;
					return_मान = निकास_त्रुटि;
					मुक्त(पङ्क्ति);
					जारी;
				}
				*filename_ptr++ = '\0';
				/* coreutils 9.1 allows "HASH FILENAME" format,
				 * with only one space. Skip the 'correct'
				 * "  " or " *" delimiter if it is there:
				 */
				यदि (*filename_ptr == ' ' || *filename_ptr == '*')
					filename_ptr++;

				hash_मान = hash_file(in_buf, filename_ptr, sha3_width);

				यदि (hash_मान && (strcasecmp((वर्ण*)hash_मान, पङ्क्ति) == 0)) {
					यदि (!(flags & FLAG_SILENT))
						म_लिखो("%s: OK\n", filename_ptr);
				} अन्यथा {
					यदि (!(flags & FLAG_SILENT))
						म_लिखो("%s: FAILED\n", filename_ptr);
					गन्ती_failed++;
					return_मान = निकास_त्रुटि;
				}
				/* possible free(NULL) */
				मुक्त(hash_मान);
				मुक्त(पङ्क्ति);
			}
			यदि (गन्ती_failed && !(flags & FLAG_SILENT)) {
				bb_error_सन्देश("WARNING: %d of %d computed checksums did NOT match",
						गन्ती_failed, गन्ती_total);
			}
			यदि (गन्ती_total == 0) {
				return_मान = निकास_त्रुटि;
				/*
				 * md5sum from GNU coreutils 8.25 says:
				 * md5sum: <FILE>: no properly formatted MD5 checksum lines found
				 */
				bb_error_सन्देश("%s: no checksum lines found", *argv);
			}
			fclose_if_not_stdin(pre_computed_stream);
		} अन्यथा {
			uint8_t *hash_मान = hash_file(in_buf, *argv, sha3_width);
			यदि (hash_मान == रिक्त) {
				return_मान = निकास_त्रुटि;
			} अन्यथा {
				म_लिखो("%s  %s\n", hash_मान, *argv);
				मुक्त(hash_मान);
			}
		}
	} जबसम्म (*++argv);

	फर्काऊ return_मान;
}
