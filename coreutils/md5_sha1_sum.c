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

#समावेश "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

क्रमागत {
	/* 4th letter of applet_name is... */
	HASH_MD5 = 's', /* "md5>s<um" */
	HASH_SHA1 = '1',
	HASH_SHA256 = '2',
	HASH_SHA3 = '3',
	HASH_SHA512 = '5',
};

#घोषणा FLAG_SILENT  1
#घोषणा FLAG_CHECK   2
#घोषणा FLAG_WARN    4

/* This might be useful elsewhere */
अटल अचिन्हित अक्षर *hash_bin_to_hex(अचिन्हित अक्षर *hash_मूल्य,
				अचिन्हित hash_लंबाई)
{
	/* xzalloc zero-terminates */
	अक्षर *hex_मूल्य = xzalloc((hash_लंबाई * 2) + 1);
	bin2hex(hex_मूल्य, (अक्षर*)hash_मूल्य, hash_लंबाई);
	वापस (अचिन्हित अक्षर *)hex_मूल्य;
}

#घोषणा BUFSZ (CONFIG_FEATURE_COPYBUF_KB < 4 ? 4096 : CONFIG_FEATURE_COPYBUF_KB * 1024)

#अगर !ENABLE_SHA3_योग
# define hash_file(b,f,w) hash_file(b,f)
#पूर्ण_अगर
अटल uint8_t *hash_file(अचिन्हित अक्षर *in_buf, स्थिर अक्षर *filename, अचिन्हित sha3_width)
{
	पूर्णांक src_fd, hash_len, गिनती;
	जोड़ _ctx_ {
		sha3_ctx_t sha3;
		sha512_ctx_t sha512;
		sha256_ctx_t sha256;
		sha1_ctx_t sha1;
		md5_ctx_t md5;
	} context;
	uint8_t *hash_मूल्य;
	व्योम FAST_FUNC (*update)(व्योम*, स्थिर व्योम*, माप_प्रकार);
	अचिन्हित FAST_FUNC (*final)(व्योम*, व्योम*);
	अक्षर hash_algo;

	src_fd = open_or_warn_stdin(filename);
	अगर (src_fd < 0) {
		वापस शून्य;
	}

	hash_algo = applet_name[3];

	/* figure specific hash algorithms */
	अगर (ENABLE_MD5_योग && hash_algo == HASH_MD5) {
		md5_begin(&context.md5);
		update = (व्योम*)md5_hash;
		final = (व्योम*)md5_end;
		hash_len = 16;
	}
	अन्यथा अगर (ENABLE_SHA1_योग && hash_algo == HASH_SHA1) {
		sha1_begin(&context.sha1);
		update = (व्योम*)sha1_hash;
		final = (व्योम*)sha1_end;
		hash_len = 20;
	}
	अन्यथा अगर (ENABLE_SHA256_योग && hash_algo == HASH_SHA256) {
		sha256_begin(&context.sha256);
		update = (व्योम*)sha256_hash;
		final = (व्योम*)sha256_end;
		hash_len = 32;
	}
	अन्यथा अगर (ENABLE_SHA512_योग && hash_algo == HASH_SHA512) {
		sha512_begin(&context.sha512);
		update = (व्योम*)sha512_hash;
		final = (व्योम*)sha512_end;
		hash_len = 64;
	}
#अगर ENABLE_SHA3_योग
	अन्यथा अगर (ENABLE_SHA3_योग && hash_algo == HASH_SHA3) {
		sha3_begin(&context.sha3);
		update = (व्योम*)sha3_hash;
		final = (व्योम*)sha3_end;
		/*
		 * Should support 224, 256, 384, 512.
		 * We allow any value which does not blow the algorithm up.
		 */
		अगर (sha3_width >= 1600/2 /* input block can't be <= 0 */
		 || sha3_width == 0      /* hash len can't be 0 */
		 || (sha3_width & 0x1f)  /* should be multiple of 32 */
		/* (because input uses up to 8 byte wide word XORs. 32/4=8) */
		) {
			bb_error_संदेश_and_die("bad -a%u", sha3_width);
		}
		sha3_width /= 4;
		context.sha3.निवेश_block_bytes = 1600/8 - sha3_width;
		hash_len = sha3_width/2;
	}
#पूर्ण_अगर
	अन्यथा {
		xfunc_die(); /* can't reach this */
	}

	{
		जबतक ((गिनती = safe_read(src_fd, in_buf, BUFSZ)) > 0) {
			update(&context, in_buf, गिनती);
		}
		hash_मूल्य = शून्य;
		अगर (गिनती < 0)
			bb_perror_संदेश("can't read '%s'", filename);
		अन्यथा /* count == 0 */ {
			final(&context, in_buf);
			hash_मूल्य = hash_bin_to_hex(in_buf, hash_len);
		}
	}

	अगर (src_fd != STDIN_FILENO) {
		बंद(src_fd);
	}

	वापस hash_मूल्य;
}

पूर्णांक md5_sha1_योग_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक md5_sha1_योग_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	अचिन्हित अक्षर *in_buf;
	पूर्णांक return_मूल्य = निकास_सफल;
	अचिन्हित flags;
#अगर ENABLE_SHA3_योग
	अचिन्हित sha3_width = 224;
#पूर्ण_अगर

	अगर (ENABLE_FEATURE_MD5_SHA1_योग_CHECK) {
		/* -b "binary", -t "text" are ignored (shaNNNsum compat) */
		/* -s and -w require -c */
#अगर ENABLE_SHA3_योग
		अगर (applet_name[3] == HASH_SHA3)
			flags = getopt32(argv, "^" "scwbta:+" "\0" "s?c:w?c", &sha3_width);
		अन्यथा
#पूर्ण_अगर
			flags = getopt32(argv, "^" "scwbt" "\0" "s?c:w?c");
	} अन्यथा {
#अगर ENABLE_SHA3_योग
		अगर (applet_name[3] == HASH_SHA3)
			getopt32(argv, "a:+", &sha3_width);
		अन्यथा
#पूर्ण_अगर
			getopt32(argv, "");
	}
	argv += optind;
	//argc -= optind;
	अगर (!*argv)
		*--argv = (अक्षर*)"-";

	/* The buffer is not alloc/freed for each input file:
	 * for big values of COPYBUF_KB, this helps to keep its pages
	 * pre-faulted and possibly even fully cached on local CPU.
	 */
	in_buf = xmalloc(BUFSZ);

	करो {
		अगर (ENABLE_FEATURE_MD5_SHA1_योग_CHECK && (flags & FLAG_CHECK)) {
			खाता *pre_computed_stream;
			अक्षर *पंक्ति;
			पूर्णांक गिनती_योग = 0;
			पूर्णांक गिनती_failed = 0;

			pre_computed_stream = xfopen_stdin(*argv);

			जबतक ((पंक्ति = xmalloc_fgetline(pre_computed_stream)) != शून्य) {
				uint8_t *hash_मूल्य;
				अक्षर *filename_ptr;

				गिनती_योग++;
				filename_ptr = म_अक्षर(पंक्ति, ' ');
				अगर (!filename_ptr) {
					अगर (flags & FLAG_WARN) {
						bb_simple_error_संदेश("invalid format");
					}
					गिनती_failed++;
					return_मूल्य = निकास_त्रुटि;
					मुक्त(पंक्ति);
					जारी;
				}
				*filename_ptr++ = '\0';
				/* coreutils 9.1 allows "HASH FILENAME" format,
				 * with only one space. Skip the 'correct'
				 * "  " or " *" delimiter if it is there:
				 */
				अगर (*filename_ptr == ' ' || *filename_ptr == '*')
					filename_ptr++;

				hash_मूल्य = hash_file(in_buf, filename_ptr, sha3_width);

				अगर (hash_मूल्य && (strcasecmp((अक्षर*)hash_मूल्य, पंक्ति) == 0)) {
					अगर (!(flags & FLAG_SILENT))
						म_लिखो("%s: OK\n", filename_ptr);
				} अन्यथा {
					अगर (!(flags & FLAG_SILENT))
						म_लिखो("%s: FAILED\n", filename_ptr);
					गिनती_failed++;
					return_मूल्य = निकास_त्रुटि;
				}
				/* possible free(NULL) */
				मुक्त(hash_मूल्य);
				मुक्त(पंक्ति);
			}
			अगर (गिनती_failed && !(flags & FLAG_SILENT)) {
				bb_error_संदेश("WARNING: %d of %d computed checksums did NOT match",
						गिनती_failed, गिनती_योग);
			}
			अगर (गिनती_योग == 0) {
				return_मूल्य = निकास_त्रुटि;
				/*
				 * md5sum from GNU coreutils 8.25 says:
				 * md5sum: <FILE>: no properly formatted MD5 checksum lines found
				 */
				bb_error_संदेश("%s: no checksum lines found", *argv);
			}
			fclose_if_not_stdin(pre_computed_stream);
		} अन्यथा {
			uint8_t *hash_मूल्य = hash_file(in_buf, *argv, sha3_width);
			अगर (hash_मूल्य == शून्य) {
				return_मूल्य = निकास_त्रुटि;
			} अन्यथा {
				म_लिखो("%s  %s\n", hash_मूल्य, *argv);
				मुक्त(hash_मूल्य);
			}
		}
	} जबतक (*++argv);

	वापस return_मूल्य;
}
