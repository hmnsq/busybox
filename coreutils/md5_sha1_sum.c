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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

ਗਿਣਤੀ {
	/* 4th letter of applet_name is... */
	HASH_MD5 = 's', /* "md5>s<um" */
	HASH_SHA1 = '1',
	HASH_SHA256 = '2',
	HASH_SHA3 = '3',
	HASH_SHA512 = '5',
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLAG_SILENT  1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLAG_CHECK   2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLAG_WARN    4

/* This might be useful elsewhere */
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *hash_bin_ਮੰਜ਼ਿਲ_hex(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *hash_ਮੁੱਲ,
				ਬਿਨਾਨਿਸ਼ਾਨੀ hash_ਲੰਮਾਈ)
{
	/* xzalloc zero-terminates */
	ਅੱਖਰ *hex_ਮੁੱਲ = xzalloc((hash_ਲੰਮਾਈ * 2) + 1);
	bin2hex(hex_ਮੁੱਲ, (ਅੱਖਰ*)hash_ਮੁੱਲ, hash_ਲੰਮਾਈ);
	ਵਾਪਸ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)hex_ਮੁੱਲ;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUFSZ (CONFIG_FEATURE_COPYBUF_KB < 4 ? 4096 : CONFIG_FEATURE_COPYBUF_KB * 1024)

#ਜੇ !ENABLE_SHA3_ਜੋੜ
# define hash_file(b,f,w) hash_file(b,f)
#ਜੇ_ਅੰਤ
ਸਥਿਰ uint8_t *hash_file(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਵਿੱਚ_buf, ਅਚਲ ਅੱਖਰ *filename, ਬਿਨਾਨਿਸ਼ਾਨੀ sha3_ਚੌੜਾਈ)
{
	ਪੂਰਨ src_fd, hash_len, ਗਿਣਤੀ_2;
	ਸੰਘ _ctx_ {
		sha3_ctx_t sha3;
		sha512_ctx_t sha512;
		sha256_ctx_t sha256;
		sha1_ctx_t sha1;
		md5_ctx_t md5;
	} context;
	uint8_t *hash_ਮੁੱਲ;
	ਖਾਲੀ FAST_FUNC (*update)(ਖਾਲੀ*, ਅਚਲ ਖਾਲੀ*, ਮਾਪ_t);
	ਬਿਨਾਨਿਸ਼ਾਨੀ FAST_FUNC (*final)(ਖਾਲੀ*, ਖਾਲੀ*);
	ਅੱਖਰ hash_algo;

	src_fd = open_or_warn_stdin(filename);
	ਜੇ (src_fd < 0) {
		ਵਾਪਸ ਨਿਰਾ;
	}

	hash_algo = applet_name[3];

	/* figure specific hash algorithms */
	ਜੇ (ENABLE_MD5_ਜੋੜ && hash_algo == HASH_MD5) {
		md5_begin(&context.md5);
		update = (ਖਾਲੀ*)md5_hash;
		final = (ਖਾਲੀ*)md5_ਸਿਰਾ;
		hash_len = 16;
	}
	ਨਹੀਂਤਾਂ ਜੇ (ENABLE_SHA1_ਜੋੜ && hash_algo == HASH_SHA1) {
		sha1_begin(&context.sha1);
		update = (ਖਾਲੀ*)sha1_hash;
		final = (ਖਾਲੀ*)sha1_ਸਿਰਾ;
		hash_len = 20;
	}
	ਨਹੀਂਤਾਂ ਜੇ (ENABLE_SHA256_ਜੋੜ && hash_algo == HASH_SHA256) {
		sha256_begin(&context.sha256);
		update = (ਖਾਲੀ*)sha256_hash;
		final = (ਖਾਲੀ*)sha256_ਸਿਰਾ;
		hash_len = 32;
	}
	ਨਹੀਂਤਾਂ ਜੇ (ENABLE_SHA512_ਜੋੜ && hash_algo == HASH_SHA512) {
		sha512_begin(&context.sha512);
		update = (ਖਾਲੀ*)sha512_hash;
		final = (ਖਾਲੀ*)sha512_ਸਿਰਾ;
		hash_len = 64;
	}
#ਜੇ ENABLE_SHA3_ਜੋੜ
	ਨਹੀਂਤਾਂ ਜੇ (ENABLE_SHA3_ਜੋੜ && hash_algo == HASH_SHA3) {
		sha3_begin(&context.sha3);
		update = (ਖਾਲੀ*)sha3_hash;
		final = (ਖਾਲੀ*)sha3_ਸਿਰਾ;
		/*
		 * Should support 224, 256, 384, 512.
		 * We allow any value which does not blow the algorithm up.
		 */
		ਜੇ (sha3_ਚੌੜਾਈ >= 1600/2 /* input block can't be <= 0 */
		 || sha3_ਚੌੜਾਈ == 0      /* hash len can't be 0 */
		 || (sha3_ਚੌੜਾਈ & 0x1f)  /* should be multiple of 32 */
		/* (because input uses up to 8 byte wide word XORs. 32/4=8) */
		) {
			bb_error_ਸੁਨੇਹਾ_and_die("bad -a%u", sha3_ਚੌੜਾਈ);
		}
		sha3_ਚੌੜਾਈ /= 4;
		context.sha3.ਨਿਵੇਸ਼_ਖੰਡ_ਬਾਈਟਾਂ = 1600/8 - sha3_ਚੌੜਾਈ;
		hash_len = sha3_ਚੌੜਾਈ/2;
	}
#ਜੇ_ਅੰਤ
	ਨਹੀਂਤਾਂ {
		xfunc_die(); /* can't reach this */
	}

	{
		ਜਦੋਂ ((ਗਿਣਤੀ_2 = ਸੁਰੱਖਿਅਤ_read(src_fd, ਵਿੱਚ_buf, BUFSZ)) > 0) {
			update(&context, ਵਿੱਚ_buf, ਗਿਣਤੀ_2);
		}
		hash_ਮੁੱਲ = ਨਿਰਾ;
		ਜੇ (ਗਿਣਤੀ_2 < 0)
			bb_perror_ਸੁਨੇਹਾ("can't read '%s'", filename);
		ਨਹੀਂਤਾਂ /* count == 0 */ {
			final(&context, ਵਿੱਚ_buf);
			hash_ਮੁੱਲ = hash_bin_ਮੰਜ਼ਿਲ_hex(ਵਿੱਚ_buf, hash_len);
		}
	}

	ਜੇ (src_fd != STDIN_FILENO) {
		close(src_fd);
	}

	ਵਾਪਸ hash_ਮੁੱਲ;
}

ਪੂਰਨ md5_sha1_ਜੋੜ_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ md5_sha1_ਜੋੜ_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਵਿੱਚ_buf;
	ਪੂਰਨ return_ਮੁੱਲ = EXIT_SUCCESS;
	ਬਿਨਾਨਿਸ਼ਾਨੀ flags;
#ਜੇ ENABLE_SHA3_ਜੋੜ
	ਬਿਨਾਨਿਸ਼ਾਨੀ sha3_ਚੌੜਾਈ = 224;
#ਜੇ_ਅੰਤ

	ਜੇ (ENABLE_FEATURE_MD5_SHA1_ਜੋੜ_CHECK) {
		/* -b "binary", -t "text" are ignored (shaNNNsum compat) */
		/* -s and -w require -c */
#ਜੇ ENABLE_SHA3_ਜੋੜ
		ਜੇ (applet_name[3] == HASH_SHA3)
			flags = getopt32(argv, "^" "scwbta:+" "\0" "s?c:w?c", &sha3_ਚੌੜਾਈ);
		ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
			flags = getopt32(argv, "^" "scwbt" "\0" "s?c:w?c");
	} ਨਹੀਂਤਾਂ {
#ਜੇ ENABLE_SHA3_ਜੋੜ
		ਜੇ (applet_name[3] == HASH_SHA3)
			getopt32(argv, "a:+", &sha3_ਚੌੜਾਈ);
		ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
			getopt32(argv, "");
	}
	argv += optind;
	//argc -= optind;
	ਜੇ (!*argv)
		*--argv = (ਅੱਖਰ*)"-";

	/* The buffer is not alloc/freed for each input file:
	 * for big values of COPYBUF_KB, this helps to keep its pages
	 * pre-faulted and possibly even fully cached on local CPU.
	 */
	ਵਿੱਚ_buf = xmalloc(BUFSZ);

	ਕਰੋ {
		ਜੇ (ENABLE_FEATURE_MD5_SHA1_ਜੋੜ_CHECK && (flags & FLAG_CHECK)) {
			FILE *pre_computed_ਧਾਰਾ;
			ਅੱਖਰ *ਸਤਰ;
			ਪੂਰਨ ਗਿਣਤੀ_ਕੁੱਲ = 0;
			ਪੂਰਨ ਗਿਣਤੀ_failed = 0;

			pre_computed_ਧਾਰਾ = xfopen_stdin(*argv);

			ਜਦੋਂ ((ਸਤਰ = xmalloc_fgetline(pre_computed_ਧਾਰਾ)) != ਨਿਰਾ) {
				uint8_t *hash_ਮੁੱਲ;
				ਅੱਖਰ *filename_ptr;

				ਗਿਣਤੀ_ਕੁੱਲ++;
				filename_ptr = strchr(ਸਤਰ, ' ');
				ਜੇ (!filename_ptr) {
					ਜੇ (flags & FLAG_WARN) {
						bb_simple_error_ਸੁਨੇਹਾ("invalid format");
					}
					ਗਿਣਤੀ_failed++;
					return_ਮੁੱਲ = EXIT_FAILURE;
					ਮੁਕਤ(ਸਤਰ);
					ਜਾਰੀ;
				}
				*filename_ptr++ = '\0';
				/* coreutils 9.1 allows "HASH FILENAME" format,
				 * with only one space. Skip the 'correct'
				 * "  " or " *" delimiter if it is there:
				 */
				ਜੇ (*filename_ptr == ' ' || *filename_ptr == '*')
					filename_ptr++;

				hash_ਮੁੱਲ = hash_file(ਵਿੱਚ_buf, filename_ptr, sha3_ਚੌੜਾਈ);

				ਜੇ (hash_ਮੁੱਲ && (strcasecmp((ਅੱਖਰ*)hash_ਮੁੱਲ, ਸਤਰ) == 0)) {
					ਜੇ (!(flags & FLAG_SILENT))
						ਲਿਖੋ("%s: OK\n", filename_ptr);
				} ਨਹੀਂਤਾਂ {
					ਜੇ (!(flags & FLAG_SILENT))
						ਲਿਖੋ("%s: FAILED\n", filename_ptr);
					ਗਿਣਤੀ_failed++;
					return_ਮੁੱਲ = EXIT_FAILURE;
				}
				/* possible free(NULL) */
				ਮੁਕਤ(hash_ਮੁੱਲ);
				ਮੁਕਤ(ਸਤਰ);
			}
			ਜੇ (ਗਿਣਤੀ_failed && !(flags & FLAG_SILENT)) {
				bb_error_ਸੁਨੇਹਾ("WARNING: %d of %d computed checksums did NOT match",
						ਗਿਣਤੀ_failed, ਗਿਣਤੀ_ਕੁੱਲ);
			}
			ਜੇ (ਗਿਣਤੀ_ਕੁੱਲ == 0) {
				return_ਮੁੱਲ = EXIT_FAILURE;
				/*
				 * md5sum from GNU coreutils 8.25 says:
				 * md5sum: <FILE>: no properly formatted MD5 checksum lines found
				 */
				bb_error_ਸੁਨੇਹਾ("%s: no checksum lines found", *argv);
			}
			fclose_if_not_stdin(pre_computed_ਧਾਰਾ);
		} ਨਹੀਂਤਾਂ {
			uint8_t *hash_ਮੁੱਲ = hash_file(ਵਿੱਚ_buf, *argv, sha3_ਚੌੜਾਈ);
			ਜੇ (hash_ਮੁੱਲ == ਨਿਰਾ) {
				return_ਮੁੱਲ = EXIT_FAILURE;
			} ਨਹੀਂਤਾਂ {
				ਲਿਖੋ("%s  %s\n", hash_ਮੁੱਲ, *argv);
				ਮੁਕਤ(hash_ਮੁੱਲ);
			}
		}
	} ਜਦੋਂ (*++argv);

	ਵਾਪਸ return_ਮੁੱਲ;
}
