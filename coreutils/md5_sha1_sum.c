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

#ضمن "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

تعداد {
	/* 4th letter of applet_name is... */
	HASH_MD5 = 's', /* "md5>s<um" */
	HASH_SHA1 = '1',
	HASH_SHA256 = '2',
	HASH_SHA3 = '3',
	HASH_SHA512 = '5',
};

#تعریف_کن FLAG_SILENT  1
#تعریف_کن FLAG_CHECK   2
#تعریف_کن FLAG_WARN    4

/* This might be useful elsewhere */
ساكن بیعلامت حرف *hash_bin_مقصد_hex(بیعلامت حرف *hash_مقدار,
				بیعلامت hash_درازی)
{
	/* xzalloc zero-terminates */
	حرف *hex_مقدار = xzalloc((hash_درازی * 2) + 1);
	bin2hex(hex_مقدار, (حرف*)hash_مقدار, hash_درازی);
	أرجع (بیعلامت حرف *)hex_مقدار;
}

#تعریف_کن BUFSZ (CONFIG_FEATURE_COPYBUF_KB < 4 ? 4096 : CONFIG_FEATURE_COPYBUF_KB * 1024)

#إذا !ENABLE_SHA3_جمع
# define hash_file(b,f,w) hash_file(b,f)
#پایان_اگر
ساكن uint8_t *hash_file(بیعلامت حرف *درون_buf, ثابت حرف *filename, بیعلامت sha3_عرض)
{
	صحيح src_fd, hash_len, شمار;
	اتحاد _ctx_ {
		sha3_ctx_t sha3;
		sha512_ctx_t sha512;
		sha256_ctx_t sha256;
		sha1_ctx_t sha1;
		md5_ctx_t md5;
	} context;
	uint8_t *hash_مقدار;
	فراغ FAST_FUNC (*update)(فراغ*, ثابت فراغ*, اندازه_t);
	بیعلامت FAST_FUNC (*final)(فراغ*, فراغ*);
	حرف hash_algo;

	src_fd = open_or_warn_stdin(filename);
	إذا (src_fd < 0) {
		أرجع عدم;
	}

	hash_algo = applet_name[3];

	/* figure specific hash algorithms */
	إذا (ENABLE_MD5_جمع && hash_algo == HASH_MD5) {
		md5_begin(&context.md5);
		update = (فراغ*)md5_hash;
		final = (فراغ*)md5_انتها;
		hash_len = 16;
	}
	وإلا إذا (ENABLE_SHA1_جمع && hash_algo == HASH_SHA1) {
		sha1_begin(&context.sha1);
		update = (فراغ*)sha1_hash;
		final = (فراغ*)sha1_انتها;
		hash_len = 20;
	}
	وإلا إذا (ENABLE_SHA256_جمع && hash_algo == HASH_SHA256) {
		sha256_begin(&context.sha256);
		update = (فراغ*)sha256_hash;
		final = (فراغ*)sha256_انتها;
		hash_len = 32;
	}
	وإلا إذا (ENABLE_SHA512_جمع && hash_algo == HASH_SHA512) {
		sha512_begin(&context.sha512);
		update = (فراغ*)sha512_hash;
		final = (فراغ*)sha512_انتها;
		hash_len = 64;
	}
#إذا ENABLE_SHA3_جمع
	وإلا إذا (ENABLE_SHA3_جمع && hash_algo == HASH_SHA3) {
		sha3_begin(&context.sha3);
		update = (فراغ*)sha3_hash;
		final = (فراغ*)sha3_انتها;
		/*
		 * Should support 224, 256, 384, 512.
		 * We allow any value which does not blow the algorithm up.
		 */
		إذا (sha3_عرض >= 1600/2 /* input block can't be <= 0 */
		 || sha3_عرض == 0      /* hash len can't be 0 */
		 || (sha3_عرض & 0x1f)  /* should be multiple of 32 */
		/* (because input uses up to 8 byte wide word XORs. 32/4=8) */
		) {
			bb_error_پیام_and_die("bad -a%u", sha3_عرض);
		}
		sha3_عرض /= 4;
		context.sha3.درون_داد_بلوک_بایتها = 1600/8 - sha3_عرض;
		hash_len = sha3_عرض/2;
	}
#پایان_اگر
	وإلا {
		xfunc_die(); /* can't reach this */
	}

	{
		طالما ((شمار = امن_read(src_fd, درون_buf, BUFSZ)) > 0) {
			update(&context, درون_buf, شمار);
		}
		hash_مقدار = عدم;
		إذا (شمار < 0)
			bb_perror_پیام("can't read '%s'", filename);
		وإلا /* count == 0 */ {
			final(&context, درون_buf);
			hash_مقدار = hash_bin_مقصد_hex(درون_buf, hash_len);
		}
	}

	إذا (src_fd != STDIN_FILENO) {
		close(src_fd);
	}

	أرجع hash_مقدار;
}

صحيح md5_sha1_جمع_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح md5_sha1_جمع_main(صحيح argc استفادهنشده_PARAM, حرف **argv)
{
	بیعلامت حرف *درون_buf;
	صحيح return_مقدار = EXIT_SUCCESS;
	بیعلامت flags;
#إذا ENABLE_SHA3_جمع
	بیعلامت sha3_عرض = 224;
#پایان_اگر

	إذا (ENABLE_FEATURE_MD5_SHA1_جمع_CHECK) {
		/* -b "binary", -t "text" are ignored (shaNNNsum compat) */
		/* -s and -w require -c */
#إذا ENABLE_SHA3_جمع
		إذا (applet_name[3] == HASH_SHA3)
			flags = getopt32(argv, "^" "scwbta:+" "\0" "s?c:w?c", &sha3_عرض);
		وإلا
#پایان_اگر
			flags = getopt32(argv, "^" "scwbt" "\0" "s?c:w?c");
	} وإلا {
#إذا ENABLE_SHA3_جمع
		إذا (applet_name[3] == HASH_SHA3)
			getopt32(argv, "a:+", &sha3_عرض);
		وإلا
#پایان_اگر
			getopt32(argv, "");
	}
	argv += optind;
	//argc -= optind;
	إذا (!*argv)
		*--argv = (حرف*)"-";

	/* The buffer is not alloc/freed for each input file:
	 * for big values of COPYBUF_KB, this helps to keep its pages
	 * pre-faulted and possibly even fully cached on local CPU.
	 */
	درون_buf = xmalloc(BUFSZ);

	افعل {
		إذا (ENABLE_FEATURE_MD5_SHA1_جمع_CHECK && (flags & FLAG_CHECK)) {
			FILE *pre_computed_جریان;
			حرف *سطر;
			صحيح شمار_مجموع = 0;
			صحيح شمار_failed = 0;

			pre_computed_جریان = xfopen_stdin(*argv);

			طالما ((سطر = xmalloc_fgetline(pre_computed_جریان)) != عدم) {
				uint8_t *hash_مقدار;
				حرف *filename_ptr;

				شمار_مجموع++;
				filename_ptr = strchr(سطر, ' ');
				إذا (!filename_ptr) {
					إذا (flags & FLAG_WARN) {
						bb_simple_error_پیام("invalid format");
					}
					شمار_failed++;
					return_مقدار = EXIT_FAILURE;
					حرر(سطر);
					استمر;
				}
				*filename_ptr++ = '\0';
				/* coreutils 9.1 allows "HASH FILENAME" format,
				 * with only one space. Skip the 'correct'
				 * "  " or " *" delimiter if it is there:
				 */
				إذا (*filename_ptr == ' ' || *filename_ptr == '*')
					filename_ptr++;

				hash_مقدار = hash_file(درون_buf, filename_ptr, sha3_عرض);

				إذا (hash_مقدار && (strcasecmp((حرف*)hash_مقدار, سطر) == 0)) {
					إذا (!(flags & FLAG_SILENT))
						چاپ_کن("%s: OK\n", filename_ptr);
				} وإلا {
					إذا (!(flags & FLAG_SILENT))
						چاپ_کن("%s: FAILED\n", filename_ptr);
					شمار_failed++;
					return_مقدار = EXIT_FAILURE;
				}
				/* possible free(NULL) */
				حرر(hash_مقدار);
				حرر(سطر);
			}
			إذا (شمار_failed && !(flags & FLAG_SILENT)) {
				bb_error_پیام("WARNING: %d of %d computed checksums did NOT match",
						شمار_failed, شمار_مجموع);
			}
			إذا (شمار_مجموع == 0) {
				return_مقدار = EXIT_FAILURE;
				/*
				 * md5sum from GNU coreutils 8.25 says:
				 * md5sum: <FILE>: no properly formatted MD5 checksum lines found
				 */
				bb_error_پیام("%s: no checksum lines found", *argv);
			}
			fclose_if_not_stdin(pre_computed_جریان);
		} وإلا {
			uint8_t *hash_مقدار = hash_file(درون_buf, *argv, sha3_عرض);
			إذا (hash_مقدار == عدم) {
				return_مقدار = EXIT_FAILURE;
			} وإلا {
				چاپ_کن("%s  %s\n", hash_مقدار, *argv);
				حرر(hash_مقدار);
			}
		}
	} طالما (*++argv);

	أرجع return_مقدار;
}
