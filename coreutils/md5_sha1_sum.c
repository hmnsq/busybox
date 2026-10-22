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

#شامل "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

شمار {
	/* 4th letter of applet_name is... */
	HASH_MD5 = 's', /* "md5>s<um" */
	HASH_SHA1 = '1',
	HASH_SHA256 = '2',
	HASH_SHA3 = '3',
	HASH_SHA512 = '5',
};

#تعریف_کرو FLAG_SILENT  1
#تعریف_کرو FLAG_CHECK   2
#تعریف_کرو FLAG_WARN    4

/* This might be useful elsewhere */
ساکن غیرعلامتی حرف *hash_bin_مقصد_hex(غیرعلامتی حرف *hash_قدر,
				غیرعلامتی hash_لمبائی)
{
	/* xzalloc zero-terminates */
	حرف *hex_قدر = xzalloc((hash_لمبائی * 2) + 1);
	bin2hex(hex_قدر, (حرف*)hash_قدر, hash_لمبائی);
	واپس (غیرعلامتی حرف *)hex_قدر;
}

#تعریف_کرو BUFSZ (CONFIG_FEATURE_COPYBUF_KB < 4 ? 4096 : CONFIG_FEATURE_COPYBUF_KB * 1024)

#اگر !ENABLE_SHA3_مجموع
# define hash_file(b,f,w) hash_file(b,f)
#اگر_ختم
ساکن uint8_t *hash_file(غیرعلامتی حرف *اندر_buf, ثابت حرف *filename, غیرعلامتی sha3_چوڑائی)
{
	صحیح src_fd, hash_len, گنتی;
	اتحاد _ctx_ {
		sha3_ctx_t sha3;
		sha512_ctx_t sha512;
		sha256_ctx_t sha256;
		sha1_ctx_t sha1;
		md5_ctx_t md5;
	} context;
	uint8_t *hash_قدر;
	خالی FAST_FUNC (*update)(خالی*, ثابت خالی*, سائز_t);
	غیرعلامتی FAST_FUNC (*final)(خالی*, خالی*);
	حرف hash_algo;

	src_fd = open_or_warn_stdin(filename);
	اگر (src_fd < 0) {
		واپس تہی;
	}

	hash_algo = applet_name[3];

	/* figure specific hash algorithms */
	اگر (ENABLE_MD5_مجموع && hash_algo == HASH_MD5) {
		md5_begin(&context.md5);
		update = (خالی*)md5_hash;
		final = (خالی*)md5_انتہا;
		hash_len = 16;
	}
	ورنہ اگر (ENABLE_SHA1_مجموع && hash_algo == HASH_SHA1) {
		sha1_begin(&context.sha1);
		update = (خالی*)sha1_hash;
		final = (خالی*)sha1_انتہا;
		hash_len = 20;
	}
	ورنہ اگر (ENABLE_SHA256_مجموع && hash_algo == HASH_SHA256) {
		sha256_begin(&context.sha256);
		update = (خالی*)sha256_hash;
		final = (خالی*)sha256_انتہا;
		hash_len = 32;
	}
	ورنہ اگر (ENABLE_SHA512_مجموع && hash_algo == HASH_SHA512) {
		sha512_begin(&context.sha512);
		update = (خالی*)sha512_hash;
		final = (خالی*)sha512_انتہا;
		hash_len = 64;
	}
#اگر ENABLE_SHA3_مجموع
	ورنہ اگر (ENABLE_SHA3_مجموع && hash_algo == HASH_SHA3) {
		sha3_begin(&context.sha3);
		update = (خالی*)sha3_hash;
		final = (خالی*)sha3_انتہا;
		/*
		 * Should support 224, 256, 384, 512.
		 * We allow any value which does not blow the algorithm up.
		 */
		اگر (sha3_چوڑائی >= 1600/2 /* input block can't be <= 0 */
		 || sha3_چوڑائی == 0      /* hash len can't be 0 */
		 || (sha3_چوڑائی & 0x1f)  /* should be multiple of 32 */
		/* (because input uses up to 8 byte wide word XORs. 32/4=8) */
		) {
			bb_error_پیغام_and_die("bad -a%u", sha3_چوڑائی);
		}
		sha3_چوڑائی /= 4;
		context.sha3.داخلہ_بلاک_بائٹیں = 1600/8 - sha3_چوڑائی;
		hash_len = sha3_چوڑائی/2;
	}
#اگر_ختم
	ورنہ {
		xfunc_die(); /* can't reach this */
	}

	{
		جب ((گنتی = محفوظ_read(src_fd, اندر_buf, BUFSZ)) > 0) {
			update(&context, اندر_buf, گنتی);
		}
		hash_قدر = تہی;
		اگر (گنتی < 0)
			bb_perror_پیغام("can't read '%s'", filename);
		ورنہ /* count == 0 */ {
			final(&context, اندر_buf);
			hash_قدر = hash_bin_مقصد_hex(اندر_buf, hash_len);
		}
	}

	اگر (src_fd != STDIN_FILENO) {
		close(src_fd);
	}

	واپس hash_قدر;
}

صحیح md5_sha1_مجموع_main(صحیح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح md5_sha1_مجموع_main(صحیح argc غیر_مستعمل_PARAM, حرف **argv)
{
	غیرعلامتی حرف *اندر_buf;
	صحیح return_قدر = EXIT_SUCCESS;
	غیرعلامتی flags;
#اگر ENABLE_SHA3_مجموع
	غیرعلامتی sha3_چوڑائی = 224;
#اگر_ختم

	اگر (ENABLE_FEATURE_MD5_SHA1_مجموع_CHECK) {
		/* -b "binary", -t "text" are ignored (shaNNNsum compat) */
		/* -s and -w require -c */
#اگر ENABLE_SHA3_مجموع
		اگر (applet_name[3] == HASH_SHA3)
			flags = getopt32(argv, "^" "scwbta:+" "\0" "s?c:w?c", &sha3_چوڑائی);
		ورنہ
#اگر_ختم
			flags = getopt32(argv, "^" "scwbt" "\0" "s?c:w?c");
	} ورنہ {
#اگر ENABLE_SHA3_مجموع
		اگر (applet_name[3] == HASH_SHA3)
			getopt32(argv, "a:+", &sha3_چوڑائی);
		ورنہ
#اگر_ختم
			getopt32(argv, "");
	}
	argv += optind;
	//argc -= optind;
	اگر (!*argv)
		*--argv = (حرف*)"-";

	/* The buffer is not alloc/freed for each input file:
	 * for big values of COPYBUF_KB, this helps to keep its pages
	 * pre-faulted and possibly even fully cached on local CPU.
	 */
	اندر_buf = xmalloc(BUFSZ);

	کرو {
		اگر (ENABLE_FEATURE_MD5_SHA1_مجموع_CHECK && (flags & FLAG_CHECK)) {
			FILE *pre_computed_دھارا;
			حرف *سطر;
			صحیح گنتی_کل = 0;
			صحیح گنتی_failed = 0;

			pre_computed_دھارا = xfopen_stdin(*argv);

			جب ((سطر = xmalloc_fgetline(pre_computed_دھارا)) != تہی) {
				uint8_t *hash_قدر;
				حرف *filename_ptr;

				گنتی_کل++;
				filename_ptr = strchr(سطر, ' ');
				اگر (!filename_ptr) {
					اگر (flags & FLAG_WARN) {
						bb_simple_error_پیغام("invalid format");
					}
					گنتی_failed++;
					return_قدر = EXIT_FAILURE;
					آزاد(سطر);
					جاری;
				}
				*filename_ptr++ = '\0';
				/* coreutils 9.1 allows "HASH FILENAME" format,
				 * with only one space. Skip the 'correct'
				 * "  " or " *" delimiter if it is there:
				 */
				اگر (*filename_ptr == ' ' || *filename_ptr == '*')
					filename_ptr++;

				hash_قدر = hash_file(اندر_buf, filename_ptr, sha3_چوڑائی);

				اگر (hash_قدر && (strcasecmp((حرف*)hash_قدر, سطر) == 0)) {
					اگر (!(flags & FLAG_SILENT))
						لکھو("%s: OK\n", filename_ptr);
				} ورنہ {
					اگر (!(flags & FLAG_SILENT))
						لکھو("%s: FAILED\n", filename_ptr);
					گنتی_failed++;
					return_قدر = EXIT_FAILURE;
				}
				/* possible free(NULL) */
				آزاد(hash_قدر);
				آزاد(سطر);
			}
			اگر (گنتی_failed && !(flags & FLAG_SILENT)) {
				bb_error_پیغام("WARNING: %d of %d computed checksums did NOT match",
						گنتی_failed, گنتی_کل);
			}
			اگر (گنتی_کل == 0) {
				return_قدر = EXIT_FAILURE;
				/*
				 * md5sum from GNU coreutils 8.25 says:
				 * md5sum: <FILE>: no properly formatted MD5 checksum lines found
				 */
				bb_error_پیغام("%s: no checksum lines found", *argv);
			}
			fclose_if_not_stdin(pre_computed_دھارا);
		} ورنہ {
			uint8_t *hash_قدر = hash_file(اندر_buf, *argv, sha3_چوڑائی);
			اگر (hash_قدر == تہی) {
				return_قدر = EXIT_FAILURE;
			} ورنہ {
				لکھو("%s  %s\n", hash_قدر, *argv);
				آزاد(hash_قدر);
			}
		}
	} جب (*++argv);

	واپس return_قدر;
}
