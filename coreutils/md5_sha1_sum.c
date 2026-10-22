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

#ಅನ್ತರ್ಭಾವ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

ಎಣಿಕೆ {
	/* 4th letter of applet_name is... */
	HASH_MD5 = 's', /* "md5>s<um" */
	HASH_SHA1 = '1',
	HASH_SHA256 = '2',
	HASH_SHA3 = '3',
	HASH_SHA512 = '5',
};

#ವ್ಯಾಖ್ಯಾನಿಸು FLAG_SILENT  1
#ವ್ಯಾಖ್ಯಾನಿಸು FLAG_CHECK   2
#ವ್ಯಾಖ್ಯಾನಿಸು FLAG_WARN    4

/* This might be useful elsewhere */
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *hash_bin_ಗಮ್ಯ_hex(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *hash_ಮೌಲ್ಯ,
				ಚಿಹ್ನೆರಹಿತ hash_ಉದ್ದಳತೆ)
{
	/* xzalloc zero-terminates */
	ಅಕ್ಷರ *hex_ಮೌಲ್ಯ = xzalloc((hash_ಉದ್ದಳತೆ * 2) + 1);
	bin2hex(hex_ಮೌಲ್ಯ, (ಅಕ್ಷರ*)hash_ಮೌಲ್ಯ, hash_ಉದ್ದಳತೆ);
	ಹಿಂತಿರುಗಿ (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)hex_ಮೌಲ್ಯ;
}

#ವ್ಯಾಖ್ಯಾನಿಸು BUFSZ (CONFIG_FEATURE_COPYBUF_KB < 4 ? 4096 : CONFIG_FEATURE_COPYBUF_KB * 1024)

#ಆದರೆ !ENABLE_SHA3_ಮೊತ್ತ
# define hash_file(b,f,w) hash_file(b,f)
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ uint8_t *hash_file(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಒಳ_buf, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename, ಚಿಹ್ನೆರಹಿತ sha3_ಅಗಲ)
{
	ಪೂರ್ಣಾಂಕ src_fd, hash_len, ಎಣಿಕೆ_2;
	ಒಕ್ಕೂಟ _ctx_ {
		sha3_ctx_t sha3;
		sha512_ctx_t sha512;
		sha256_ctx_t sha256;
		sha1_ctx_t sha1;
		md5_ctx_t md5;
	} context;
	uint8_t *hash_ಮೌಲ್ಯ;
	ಶೂನ್ಯ FAST_FUNC (*update)(ಶೂನ್ಯ*, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*, ಗಾತ್ರ_t);
	ಚಿಹ್ನೆರಹಿತ FAST_FUNC (*final)(ಶೂನ್ಯ*, ಶೂನ್ಯ*);
	ಅಕ್ಷರ hash_algo;

	src_fd = open_or_warn_stdin(filename);
	ಆದರೆ (src_fd < 0) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	hash_algo = applet_name[3];

	/* figure specific hash algorithms */
	ಆದರೆ (ENABLE_MD5_ಮೊತ್ತ && hash_algo == HASH_MD5) {
		md5_begin(&context.md5);
		update = (ಶೂನ್ಯ*)md5_hash;
		final = (ಶೂನ್ಯ*)md5_ಕೊನೆ;
		hash_len = 16;
	}
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ENABLE_SHA1_ಮೊತ್ತ && hash_algo == HASH_SHA1) {
		sha1_begin(&context.sha1);
		update = (ಶೂನ್ಯ*)sha1_hash;
		final = (ಶೂನ್ಯ*)sha1_ಕೊನೆ;
		hash_len = 20;
	}
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ENABLE_SHA256_ಮೊತ್ತ && hash_algo == HASH_SHA256) {
		sha256_begin(&context.sha256);
		update = (ಶೂನ್ಯ*)sha256_hash;
		final = (ಶೂನ್ಯ*)sha256_ಕೊನೆ;
		hash_len = 32;
	}
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ENABLE_SHA512_ಮೊತ್ತ && hash_algo == HASH_SHA512) {
		sha512_begin(&context.sha512);
		update = (ಶೂನ್ಯ*)sha512_hash;
		final = (ಶೂನ್ಯ*)sha512_ಕೊನೆ;
		hash_len = 64;
	}
#ಆದರೆ ENABLE_SHA3_ಮೊತ್ತ
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ENABLE_SHA3_ಮೊತ್ತ && hash_algo == HASH_SHA3) {
		sha3_begin(&context.sha3);
		update = (ಶೂನ್ಯ*)sha3_hash;
		final = (ಶೂನ್ಯ*)sha3_ಕೊನೆ;
		/*
		 * Should support 224, 256, 384, 512.
		 * We allow any value which does not blow the algorithm up.
		 */
		ಆದರೆ (sha3_ಅಗಲ >= 1600/2 /* input block can't be <= 0 */
		 || sha3_ಅಗಲ == 0      /* hash len can't be 0 */
		 || (sha3_ಅಗಲ & 0x1f)  /* should be multiple of 32 */
		/* (because input uses up to 8 byte wide word XORs. 32/4=8) */
		) {
			bb_error_ಸಂದೇಶ_and_die("bad -a%u", sha3_ಅಗಲ);
		}
		sha3_ಅಗಲ /= 4;
		context.sha3.ಒಳಹರಿವು_ಖಂಡ_ಬೈಟುಗಳು = 1600/8 - sha3_ಅಗಲ;
		hash_len = sha3_ಅಗಲ/2;
	}
#ಆದರೆ_ಅಂತ್ಯ
	ಇಲ್ಲದಿದ್ದರೆ {
		xfunc_die(); /* can't reach this */
	}

	{
		ಆಗ ((ಎಣಿಕೆ_2 = ಸುರಕ್ಷಿತ_read(src_fd, ಒಳ_buf, BUFSZ)) > 0) {
			update(&context, ಒಳ_buf, ಎಣಿಕೆ_2);
		}
		hash_ಮೌಲ್ಯ = ಖಾಲಿ;
		ಆದರೆ (ಎಣಿಕೆ_2 < 0)
			bb_perror_ಸಂದೇಶ("can't read '%s'", filename);
		ಇಲ್ಲದಿದ್ದರೆ /* count == 0 */ {
			final(&context, ಒಳ_buf);
			hash_ಮೌಲ್ಯ = hash_bin_ಗಮ್ಯ_hex(ಒಳ_buf, hash_len);
		}
	}

	ಆದರೆ (src_fd != STDIN_FILENO) {
		close(src_fd);
	}

	ಹಿಂತಿರುಗಿ hash_ಮೌಲ್ಯ;
}

ಪೂರ್ಣಾಂಕ md5_sha1_ಮೊತ್ತ_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ md5_sha1_ಮೊತ್ತ_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಒಳ_buf;
	ಪೂರ್ಣಾಂಕ return_ಮೌಲ್ಯ = EXIT_SUCCESS;
	ಚಿಹ್ನೆರಹಿತ flags;
#ಆದರೆ ENABLE_SHA3_ಮೊತ್ತ
	ಚಿಹ್ನೆರಹಿತ sha3_ಅಗಲ = 224;
#ಆದರೆ_ಅಂತ್ಯ

	ಆದರೆ (ENABLE_FEATURE_MD5_SHA1_ಮೊತ್ತ_CHECK) {
		/* -b "binary", -t "text" are ignored (shaNNNsum compat) */
		/* -s and -w require -c */
#ಆದರೆ ENABLE_SHA3_ಮೊತ್ತ
		ಆದರೆ (applet_name[3] == HASH_SHA3)
			flags = getopt32(argv, "^" "scwbta:+" "\0" "s?c:w?c", &sha3_ಅಗಲ);
		ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
			flags = getopt32(argv, "^" "scwbt" "\0" "s?c:w?c");
	} ಇಲ್ಲದಿದ್ದರೆ {
#ಆದರೆ ENABLE_SHA3_ಮೊತ್ತ
		ಆದರೆ (applet_name[3] == HASH_SHA3)
			getopt32(argv, "a:+", &sha3_ಅಗಲ);
		ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
			getopt32(argv, "");
	}
	argv += optind;
	//argc -= optind;
	ಆದರೆ (!*argv)
		*--argv = (ಅಕ್ಷರ*)"-";

	/* The buffer is not alloc/freed for each input file:
	 * for big values of COPYBUF_KB, this helps to keep its pages
	 * pre-faulted and possibly even fully cached on local CPU.
	 */
	ಒಳ_buf = xmalloc(BUFSZ);

	ಮಾಡು {
		ಆದರೆ (ENABLE_FEATURE_MD5_SHA1_ಮೊತ್ತ_CHECK && (flags & FLAG_CHECK)) {
			FILE *pre_computed_ಹರಿವು;
			ಅಕ್ಷರ *ಸಾಲು;
			ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_ಒಟ್ಟು = 0;
			ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_failed = 0;

			pre_computed_ಹರಿವು = xfopen_stdin(*argv);

			ಆಗ ((ಸಾಲು = xmalloc_fgetline(pre_computed_ಹರಿವು)) != ಖಾಲಿ) {
				uint8_t *hash_ಮೌಲ್ಯ;
				ಅಕ್ಷರ *filename_ptr;

				ಎಣಿಕೆ_ಒಟ್ಟು++;
				filename_ptr = strchr(ಸಾಲು, ' ');
				ಆದರೆ (!filename_ptr) {
					ಆದರೆ (flags & FLAG_WARN) {
						bb_simple_error_ಸಂದೇಶ("invalid format");
					}
					ಎಣಿಕೆ_failed++;
					return_ಮೌಲ್ಯ = EXIT_FAILURE;
					ಬಿಡುಗಡೆ(ಸಾಲು);
					ಮುಂದುವರಿ;
				}
				*filename_ptr++ = '\0';
				/* coreutils 9.1 allows "HASH FILENAME" format,
				 * with only one space. Skip the 'correct'
				 * "  " or " *" delimiter if it is there:
				 */
				ಆದರೆ (*filename_ptr == ' ' || *filename_ptr == '*')
					filename_ptr++;

				hash_ಮೌಲ್ಯ = hash_file(ಒಳ_buf, filename_ptr, sha3_ಅಗಲ);

				ಆದರೆ (hash_ಮೌಲ್ಯ && (strcasecmp((ಅಕ್ಷರ*)hash_ಮೌಲ್ಯ, ಸಾಲು) == 0)) {
					ಆದರೆ (!(flags & FLAG_SILENT))
						ಬರೆ("%s: OK\n", filename_ptr);
				} ಇಲ್ಲದಿದ್ದರೆ {
					ಆದರೆ (!(flags & FLAG_SILENT))
						ಬರೆ("%s: FAILED\n", filename_ptr);
					ಎಣಿಕೆ_failed++;
					return_ಮೌಲ್ಯ = EXIT_FAILURE;
				}
				/* possible free(NULL) */
				ಬಿಡುಗಡೆ(hash_ಮೌಲ್ಯ);
				ಬಿಡುಗಡೆ(ಸಾಲು);
			}
			ಆದರೆ (ಎಣಿಕೆ_failed && !(flags & FLAG_SILENT)) {
				bb_error_ಸಂದೇಶ("WARNING: %d of %d computed checksums did NOT match",
						ಎಣಿಕೆ_failed, ಎಣಿಕೆ_ಒಟ್ಟು);
			}
			ಆದರೆ (ಎಣಿಕೆ_ಒಟ್ಟು == 0) {
				return_ಮೌಲ್ಯ = EXIT_FAILURE;
				/*
				 * md5sum from GNU coreutils 8.25 says:
				 * md5sum: <FILE>: no properly formatted MD5 checksum lines found
				 */
				bb_error_ಸಂದೇಶ("%s: no checksum lines found", *argv);
			}
			fclose_if_not_stdin(pre_computed_ಹರಿವು);
		} ಇಲ್ಲದಿದ್ದರೆ {
			uint8_t *hash_ಮೌಲ್ಯ = hash_file(ಒಳ_buf, *argv, sha3_ಅಗಲ);
			ಆದರೆ (hash_ಮೌಲ್ಯ == ಖಾಲಿ) {
				return_ಮೌಲ್ಯ = EXIT_FAILURE;
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಬರೆ("%s  %s\n", hash_ಮೌಲ್ಯ, *argv);
				ಬಿಡುಗಡೆ(hash_ಮೌಲ್ಯ);
			}
		}
	} ಆಗ (*++argv);

	ಹಿಂತಿರುಗಿ return_ಮೌಲ್ಯ;
}
