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

#అన్తర్భావ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

గణన {
	/* 4th letter of applet_name is... */
	HASH_MD5 = 's', /* "md5>s<um" */
	HASH_SHA1 = '1',
	HASH_SHA256 = '2',
	HASH_SHA3 = '3',
	HASH_SHA512 = '5',
};

#నిర్వచించు FLAG_SILENT  1
#నిర్వచించు FLAG_CHECK   2
#నిర్వచించు FLAG_WARN    4

/* This might be useful elsewhere */
స్థిర గుర్తులేని అక్షరం *hash_bin_గమ్యం_hex(గుర్తులేని అక్షరం *hash_విలువ,
				గుర్తులేని hash_నిడివి)
{
	/* xzalloc zero-terminates */
	అక్షరం *hex_విలువ = xzalloc((hash_నిడివి * 2) + 1);
	bin2hex(hex_విలువ, (అక్షరం*)hash_విలువ, hash_నిడివి);
	తిరిగి (గుర్తులేని అక్షరం *)hex_విలువ;
}

#నిర్వచించు BUFSZ (CONFIG_FEATURE_COPYBUF_KB < 4 ? 4096 : CONFIG_FEATURE_COPYBUF_KB * 1024)

#ఒకవేళ !ENABLE_SHA3_కూడిక
# define hash_file(b,f,w) hash_file(b,f)
#అయితే_ముగింపు
స్థిర uint8_t *hash_file(గుర్తులేని అక్షరం *లోపల_buf, స్థిరరాశి అక్షరం *filename, గుర్తులేని sha3_వెడల్పు)
{
	పూర్ణాంకం src_fd, hash_len, లెక్క;
	సంఘం _ctx_ {
		sha3_ctx_t sha3;
		sha512_ctx_t sha512;
		sha256_ctx_t sha256;
		sha1_ctx_t sha1;
		md5_ctx_t md5;
	} context;
	uint8_t *hash_విలువ;
	శూన్య FAST_FUNC (*update)(శూన్య*, స్థిరరాశి శూన్య*, కొలత_t);
	గుర్తులేని FAST_FUNC (*final)(శూన్య*, శూన్య*);
	అక్షరం hash_algo;

	src_fd = open_or_warn_stdin(filename);
	ఒకవేళ (src_fd < 0) {
		తిరిగి ఖాళీ;
	}

	hash_algo = applet_name[3];

	/* figure specific hash algorithms */
	ఒకవేళ (ENABLE_MD5_కూడిక && hash_algo == HASH_MD5) {
		md5_begin(&context.md5);
		update = (శూన్య*)md5_hash;
		final = (శూన్య*)md5_చివర;
		hash_len = 16;
	}
	లేకపోతే ఒకవేళ (ENABLE_SHA1_కూడిక && hash_algo == HASH_SHA1) {
		sha1_begin(&context.sha1);
		update = (శూన్య*)sha1_hash;
		final = (శూన్య*)sha1_చివర;
		hash_len = 20;
	}
	లేకపోతే ఒకవేళ (ENABLE_SHA256_కూడిక && hash_algo == HASH_SHA256) {
		sha256_begin(&context.sha256);
		update = (శూన్య*)sha256_hash;
		final = (శూన్య*)sha256_చివర;
		hash_len = 32;
	}
	లేకపోతే ఒకవేళ (ENABLE_SHA512_కూడిక && hash_algo == HASH_SHA512) {
		sha512_begin(&context.sha512);
		update = (శూన్య*)sha512_hash;
		final = (శూన్య*)sha512_చివర;
		hash_len = 64;
	}
#ఒకవేళ ENABLE_SHA3_కూడిక
	లేకపోతే ఒకవేళ (ENABLE_SHA3_కూడిక && hash_algo == HASH_SHA3) {
		sha3_begin(&context.sha3);
		update = (శూన్య*)sha3_hash;
		final = (శూన్య*)sha3_చివర;
		/*
		 * Should support 224, 256, 384, 512.
		 * We allow any value which does not blow the algorithm up.
		 */
		ఒకవేళ (sha3_వెడల్పు >= 1600/2 /* input block can't be <= 0 */
		 || sha3_వెడల్పు == 0      /* hash len can't be 0 */
		 || (sha3_వెడల్పు & 0x1f)  /* should be multiple of 32 */
		/* (because input uses up to 8 byte wide word XORs. 32/4=8) */
		) {
			bb_error_సందేశం_and_die("bad -a%u", sha3_వెడల్పు);
		}
		sha3_వెడల్పు /= 4;
		context.sha3.ఆదానం_ఖండం_బైట్లు = 1600/8 - sha3_వెడల్పు;
		hash_len = sha3_వెడల్పు/2;
	}
#అయితే_ముగింపు
	లేకపోతే {
		xfunc_die(); /* can't reach this */
	}

	{
		అయితే ((లెక్క = సురక్షిత_read(src_fd, లోపల_buf, BUFSZ)) > 0) {
			update(&context, లోపల_buf, లెక్క);
		}
		hash_విలువ = ఖాళీ;
		ఒకవేళ (లెక్క < 0)
			bb_perror_సందేశం("can't read '%s'", filename);
		లేకపోతే /* count == 0 */ {
			final(&context, లోపల_buf);
			hash_విలువ = hash_bin_గమ్యం_hex(లోపల_buf, hash_len);
		}
	}

	ఒకవేళ (src_fd != STDIN_FILENO) {
		close(src_fd);
	}

	తిరిగి hash_విలువ;
}

పూర్ణాంకం md5_sha1_కూడిక_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం md5_sha1_కూడిక_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	గుర్తులేని అక్షరం *లోపల_buf;
	పూర్ణాంకం return_విలువ = EXIT_SUCCESS;
	గుర్తులేని flags;
#ఒకవేళ ENABLE_SHA3_కూడిక
	గుర్తులేని sha3_వెడల్పు = 224;
#అయితే_ముగింపు

	ఒకవేళ (ENABLE_FEATURE_MD5_SHA1_కూడిక_CHECK) {
		/* -b "binary", -t "text" are ignored (shaNNNsum compat) */
		/* -s and -w require -c */
#ఒకవేళ ENABLE_SHA3_కూడిక
		ఒకవేళ (applet_name[3] == HASH_SHA3)
			flags = getopt32(argv, "^" "scwbta:+" "\0" "s?c:w?c", &sha3_వెడల్పు);
		లేకపోతే
#అయితే_ముగింపు
			flags = getopt32(argv, "^" "scwbt" "\0" "s?c:w?c");
	} లేకపోతే {
#ఒకవేళ ENABLE_SHA3_కూడిక
		ఒకవేళ (applet_name[3] == HASH_SHA3)
			getopt32(argv, "a:+", &sha3_వెడల్పు);
		లేకపోతే
#అయితే_ముగింపు
			getopt32(argv, "");
	}
	argv += optind;
	//argc -= optind;
	ఒకవేళ (!*argv)
		*--argv = (అక్షరం*)"-";

	/* The buffer is not alloc/freed for each input file:
	 * for big values of COPYBUF_KB, this helps to keep its pages
	 * pre-faulted and possibly even fully cached on local CPU.
	 */
	లోపల_buf = xmalloc(BUFSZ);

	చేయి {
		ఒకవేళ (ENABLE_FEATURE_MD5_SHA1_కూడిక_CHECK && (flags & FLAG_CHECK)) {
			FILE *pre_computed_ప్రవాహం;
			అక్షరం *పంక్తి;
			పూర్ణాంకం లెక్క_మొత్తం = 0;
			పూర్ణాంకం లెక్క_failed = 0;

			pre_computed_ప్రవాహం = xfopen_stdin(*argv);

			అయితే ((పంక్తి = xmalloc_fgetline(pre_computed_ప్రవాహం)) != ఖాళీ) {
				uint8_t *hash_విలువ;
				అక్షరం *filename_ptr;

				లెక్క_మొత్తం++;
				filename_ptr = strchr(పంక్తి, ' ');
				ఒకవేళ (!filename_ptr) {
					ఒకవేళ (flags & FLAG_WARN) {
						bb_simple_error_సందేశం("invalid format");
					}
					లెక్క_failed++;
					return_విలువ = EXIT_FAILURE;
					విడుదల(పంక్తి);
					కొనసాగు;
				}
				*filename_ptr++ = '\0';
				/* coreutils 9.1 allows "HASH FILENAME" format,
				 * with only one space. Skip the 'correct'
				 * "  " or " *" delimiter if it is there:
				 */
				ఒకవేళ (*filename_ptr == ' ' || *filename_ptr == '*')
					filename_ptr++;

				hash_విలువ = hash_file(లోపల_buf, filename_ptr, sha3_వెడల్పు);

				ఒకవేళ (hash_విలువ && (strcasecmp((అక్షరం*)hash_విలువ, పంక్తి) == 0)) {
					ఒకవేళ (!(flags & FLAG_SILENT))
						రాయి("%s: OK\n", filename_ptr);
				} లేకపోతే {
					ఒకవేళ (!(flags & FLAG_SILENT))
						రాయి("%s: FAILED\n", filename_ptr);
					లెక్క_failed++;
					return_విలువ = EXIT_FAILURE;
				}
				/* possible free(NULL) */
				విడుదల(hash_విలువ);
				విడుదల(పంక్తి);
			}
			ఒకవేళ (లెక్క_failed && !(flags & FLAG_SILENT)) {
				bb_error_సందేశం("WARNING: %d of %d computed checksums did NOT match",
						లెక్క_failed, లెక్క_మొత్తం);
			}
			ఒకవేళ (లెక్క_మొత్తం == 0) {
				return_విలువ = EXIT_FAILURE;
				/*
				 * md5sum from GNU coreutils 8.25 says:
				 * md5sum: <FILE>: no properly formatted MD5 checksum lines found
				 */
				bb_error_సందేశం("%s: no checksum lines found", *argv);
			}
			fclose_if_not_stdin(pre_computed_ప్రవాహం);
		} లేకపోతే {
			uint8_t *hash_విలువ = hash_file(లోపల_buf, *argv, sha3_వెడల్పు);
			ఒకవేళ (hash_విలువ == ఖాళీ) {
				return_విలువ = EXIT_FAILURE;
			} లేకపోతే {
				రాయి("%s  %s\n", hash_విలువ, *argv);
				విడుదల(hash_విలువ);
			}
		}
	} అయితే (*++argv);

	తిరిగి return_విలువ;
}
