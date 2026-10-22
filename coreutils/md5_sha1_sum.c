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

#அந்தர்பாவ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

எண்ணிக்கை {
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
நிலையான unsigned எழுத்து *hash_bin_to_hex(unsigned எழுத்து *hash_மதிப்பு,
				unsigned hash_length)
{
	/* xzalloc zero-terminates */
	எழுத்து *hex_மதிப்பு = xzalloc((hash_length * 2) + 1);
	bin2hex(hex_மதிப்பு, (எழுத்து*)hash_மதிப்பு, hash_length);
	திரும்பு (unsigned எழுத்து *)hex_மதிப்பு;
}

#define BUFSZ (CONFIG_FEATURE_COPYBUF_KB < 4 ? 4096 : CONFIG_FEATURE_COPYBUF_KB * 1024)

#எனில் !ENABLE_SHA3SUM
# define hash_file(b,f,w) hash_file(b,f)
#endif
நிலையான uint8_t *hash_file(unsigned எழுத்து *in_buf, மாறிலி எழுத்து *filename, unsigned sha3_width)
{
	முழு src_fd, hash_len, மொத்தம்;
	ஒன்றிப்பு _ctx_ {
		sha3_ctx_t sha3;
		sha512_ctx_t sha512;
		sha256_ctx_t sha256;
		sha1_ctx_t sha1;
		md5_ctx_t md5;
	} context;
	uint8_t *hash_மதிப்பு;
	வெறுமை FAST_FUNC (*update)(வெறுமை*, மாறிலி வெறுமை*, size_t);
	unsigned FAST_FUNC (*final)(வெறுமை*, வெறுமை*);
	எழுத்து hash_algo;

	src_fd = open_or_warn_stdin(filename);
	எனில் (src_fd < 0) {
		திரும்பு வெற்று;
	}

	hash_algo = applet_name[3];

	/* figure specific hash algorithms */
	எனில் (ENABLE_MD5SUM && hash_algo == HASH_MD5) {
		md5_begin(&context.md5);
		update = (வெறுமை*)md5_hash;
		final = (வெறுமை*)md5_end;
		hash_len = 16;
	}
	இல்லையெனில் எனில் (ENABLE_SHA1SUM && hash_algo == HASH_SHA1) {
		sha1_begin(&context.sha1);
		update = (வெறுமை*)sha1_hash;
		final = (வெறுமை*)sha1_end;
		hash_len = 20;
	}
	இல்லையெனில் எனில் (ENABLE_SHA256SUM && hash_algo == HASH_SHA256) {
		sha256_begin(&context.sha256);
		update = (வெறுமை*)sha256_hash;
		final = (வெறுமை*)sha256_end;
		hash_len = 32;
	}
	இல்லையெனில் எனில் (ENABLE_SHA512SUM && hash_algo == HASH_SHA512) {
		sha512_begin(&context.sha512);
		update = (வெறுமை*)sha512_hash;
		final = (வெறுமை*)sha512_end;
		hash_len = 64;
	}
#எனில் ENABLE_SHA3SUM
	இல்லையெனில் எனில் (ENABLE_SHA3SUM && hash_algo == HASH_SHA3) {
		sha3_begin(&context.sha3);
		update = (வெறுமை*)sha3_hash;
		final = (வெறுமை*)sha3_end;
		/*
		 * Should support 224, 256, 384, 512.
		 * We allow any value which does not blow the algorithm up.
		 */
		எனில் (sha3_width >= 1600/2 /* input block can't be <= 0 */
		 || sha3_width == 0      /* hash len can't be 0 */
		 || (sha3_width & 0x1f)  /* should be multiple of 32 */
		/* (because input uses up to 8 byte wide word XORs. 32/4=8) */
		) {
			bb_error_செய்தி_and_die("bad -a%u", sha3_width);
		}
		sha3_width /= 4;
		context.sha3.உள்ளீடு_block_bytes = 1600/8 - sha3_width;
		hash_len = sha3_width/2;
	}
#endif
	இல்லையெனில் {
		xfunc_die(); /* can't reach this */
	}

	{
		போது ((மொத்தம் = safe_read(src_fd, in_buf, BUFSZ)) > 0) {
			update(&context, in_buf, மொத்தம்);
		}
		hash_மதிப்பு = வெற்று;
		எனில் (மொத்தம் < 0)
			bb_perror_செய்தி("can't read '%s'", filename);
		இல்லையெனில் /* count == 0 */ {
			final(&context, in_buf);
			hash_மதிப்பு = hash_bin_to_hex(in_buf, hash_len);
		}
	}

	எனில் (src_fd != STDIN_FILENO) {
		close(src_fd);
	}

	திரும்பு hash_மதிப்பு;
}

முழு md5_sha1_sum_main(முழு argc, எழுத்து **argv) MAIN_EXTERNALLY_VISIBLE;
முழு md5_sha1_sum_main(முழு argc UNUSED_PARAM, எழுத்து **argv)
{
	unsigned எழுத்து *in_buf;
	முழு return_மதிப்பு = EXIT_SUCCESS;
	unsigned flags;
#எனில் ENABLE_SHA3SUM
	unsigned sha3_width = 224;
#endif

	எனில் (ENABLE_FEATURE_MD5_SHA1_SUM_CHECK) {
		/* -b "binary", -t "text" are ignored (shaNNNsum compat) */
		/* -s and -w require -c */
#எனில் ENABLE_SHA3SUM
		எனில் (applet_name[3] == HASH_SHA3)
			flags = getopt32(argv, "^" "scwbta:+" "\0" "s?c:w?c", &sha3_width);
		இல்லையெனில்
#endif
			flags = getopt32(argv, "^" "scwbt" "\0" "s?c:w?c");
	} இல்லையெனில் {
#எனில் ENABLE_SHA3SUM
		எனில் (applet_name[3] == HASH_SHA3)
			getopt32(argv, "a:+", &sha3_width);
		இல்லையெனில்
#endif
			getopt32(argv, "");
	}
	argv += optind;
	//argc -= optind;
	எனில் (!*argv)
		*--argv = (எழுத்து*)"-";

	/* The buffer is not alloc/freed for each input file:
	 * for big values of COPYBUF_KB, this helps to keep its pages
	 * pre-faulted and possibly even fully cached on local CPU.
	 */
	in_buf = xmalloc(BUFSZ);

	செய் {
		எனில் (ENABLE_FEATURE_MD5_SHA1_SUM_CHECK && (flags & FLAG_CHECK)) {
			FILE *pre_computed_stream;
			எழுத்து *வரி;
			முழு மொத்தம்_total = 0;
			முழு மொத்தம்_failed = 0;

			pre_computed_stream = xfopen_stdin(*argv);

			போது ((வரி = xmalloc_fgetline(pre_computed_stream)) != வெற்று) {
				uint8_t *hash_மதிப்பு;
				எழுத்து *filename_ptr;

				மொத்தம்_total++;
				filename_ptr = strchr(வரி, ' ');
				எனில் (!filename_ptr) {
					எனில் (flags & FLAG_WARN) {
						bb_simple_error_செய்தி("invalid format");
					}
					மொத்தம்_failed++;
					return_மதிப்பு = EXIT_FAILURE;
					விடுவி(வரி);
					தொடர்;
				}
				*filename_ptr++ = '\0';
				/* coreutils 9.1 allows "HASH FILENAME" format,
				 * with only one space. Skip the 'correct'
				 * "  " or " *" delimiter if it is there:
				 */
				எனில் (*filename_ptr == ' ' || *filename_ptr == '*')
					filename_ptr++;

				hash_மதிப்பு = hash_file(in_buf, filename_ptr, sha3_width);

				எனில் (hash_மதிப்பு && (strcasecmp((எழுத்து*)hash_மதிப்பு, வரி) == 0)) {
					எனில் (!(flags & FLAG_SILENT))
						printf("%s: OK\n", filename_ptr);
				} இல்லையெனில் {
					எனில் (!(flags & FLAG_SILENT))
						printf("%s: FAILED\n", filename_ptr);
					மொத்தம்_failed++;
					return_மதிப்பு = EXIT_FAILURE;
				}
				/* possible free(NULL) */
				விடுவி(hash_மதிப்பு);
				விடுவி(வரி);
			}
			எனில் (மொத்தம்_failed && !(flags & FLAG_SILENT)) {
				bb_error_செய்தி("WARNING: %d of %d computed checksums did NOT match",
						மொத்தம்_failed, மொத்தம்_total);
			}
			எனில் (மொத்தம்_total == 0) {
				return_மதிப்பு = EXIT_FAILURE;
				/*
				 * md5sum from GNU coreutils 8.25 says:
				 * md5sum: <FILE>: no properly formatted MD5 checksum lines found
				 */
				bb_error_செய்தி("%s: no checksum lines found", *argv);
			}
			fclose_if_not_stdin(pre_computed_stream);
		} இல்லையெனில் {
			uint8_t *hash_மதிப்பு = hash_file(in_buf, *argv, sha3_width);
			எனில் (hash_மதிப்பு == வெற்று) {
				return_மதிப்பு = EXIT_FAILURE;
			} இல்லையெனில் {
				printf("%s  %s\n", hash_மதிப்பு, *argv);
				விடுவி(hash_மதிப்பு);
			}
		}
	} போது (*++argv);

	திரும்பு return_மதிப்பு;
}
