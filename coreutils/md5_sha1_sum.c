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

#تعریف_کړه FLAG_SILENT  1
#تعریف_کړه FLAG_CHECK   2
#تعریف_کړه FLAG_WARN    4

/* This might be useful elsewhere */
ساكن بینښه حرف *hash_bin_منزل_hex(بینښه حرف *hash_ارزښت,
				بینښه hash_اوږدتیا)
{
	/* xzalloc zero-terminates */
	حرف *hex_ارزښت = xzalloc((hash_اوږدتیا * 2) + 1);
	bin2hex(hex_ارزښت, (حرف*)hash_ارزښت, hash_اوږدتیا);
	أرجع (بینښه حرف *)hex_ارزښت;
}

#تعریف_کړه BUFSZ (CONFIG_FEATURE_COPYBUF_KB < 4 ? 4096 : CONFIG_FEATURE_COPYBUF_KB * 1024)

#إذا !ENABLE_SHA3_مجموعه
# define hash_file(b,f,w) hash_file(b,f)
#د_که_پای
ساكن uint8_t *hash_file(بینښه حرف *دننه_buf, ثابت حرف *filename, بینښه sha3_پلنوالی)
{
	صحيح src_fd, hash_len, شمېر;
	اتحاد _ctx_ {
		sha3_ctx_t sha3;
		sha512_ctx_t sha512;
		sha256_ctx_t sha256;
		sha1_ctx_t sha1;
		md5_ctx_t md5;
	} context;
	uint8_t *hash_ارزښت;
	فراغ FAST_FUNC (*update)(فراغ*, ثابت فراغ*, اندازه_t);
	بینښه FAST_FUNC (*final)(فراغ*, فراغ*);
	حرف hash_algo;

	src_fd = open_or_warn_stdin(filename);
	إذا (src_fd < 0) {
		أرجع عدم;
	}

	hash_algo = applet_name[3];

	/* figure specific hash algorithms */
	إذا (ENABLE_MD5_مجموعه && hash_algo == HASH_MD5) {
		md5_begin(&context.md5);
		update = (فراغ*)md5_hash;
		final = (فراغ*)md5_پای_ټکی;
		hash_len = 16;
	}
	وإلا إذا (ENABLE_SHA1_مجموعه && hash_algo == HASH_SHA1) {
		sha1_begin(&context.sha1);
		update = (فراغ*)sha1_hash;
		final = (فراغ*)sha1_پای_ټکی;
		hash_len = 20;
	}
	وإلا إذا (ENABLE_SHA256_مجموعه && hash_algo == HASH_SHA256) {
		sha256_begin(&context.sha256);
		update = (فراغ*)sha256_hash;
		final = (فراغ*)sha256_پای_ټکی;
		hash_len = 32;
	}
	وإلا إذا (ENABLE_SHA512_مجموعه && hash_algo == HASH_SHA512) {
		sha512_begin(&context.sha512);
		update = (فراغ*)sha512_hash;
		final = (فراغ*)sha512_پای_ټکی;
		hash_len = 64;
	}
#إذا ENABLE_SHA3_مجموعه
	وإلا إذا (ENABLE_SHA3_مجموعه && hash_algo == HASH_SHA3) {
		sha3_begin(&context.sha3);
		update = (فراغ*)sha3_hash;
		final = (فراغ*)sha3_پای_ټکی;
		/*
		 * Should support 224, 256, 384, 512.
		 * We allow any value which does not blow the algorithm up.
		 */
		إذا (sha3_پلنوالی >= 1600/2 /* input block can't be <= 0 */
		 || sha3_پلنوالی == 0      /* hash len can't be 0 */
		 || (sha3_پلنوالی & 0x1f)  /* should be multiple of 32 */
		/* (because input uses up to 8 byte wide word XORs. 32/4=8) */
		) {
			bb_error_پیغام_and_die("bad -a%u", sha3_پلنوالی);
		}
		sha3_پلنوالی /= 4;
		context.sha3.ننوت_بلاک_بایټونه = 1600/8 - sha3_پلنوالی;
		hash_len = sha3_پلنوالی/2;
	}
#د_که_پای
	وإلا {
		xfunc_die(); /* can't reach this */
	}

	{
		طالما ((شمېر = خوندي_read(src_fd, دننه_buf, BUFSZ)) > 0) {
			update(&context, دننه_buf, شمېر);
		}
		hash_ارزښت = عدم;
		إذا (شمېر < 0)
			bb_perror_پیغام("can't read '%s'", filename);
		وإلا /* count == 0 */ {
			final(&context, دننه_buf);
			hash_ارزښت = hash_bin_منزل_hex(دننه_buf, hash_len);
		}
	}

	إذا (src_fd != STDIN_FILENO) {
		close(src_fd);
	}

	أرجع hash_ارزښت;
}

صحيح md5_sha1_مجموعه_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح md5_sha1_مجموعه_main(صحيح argc ناکارول_شوی_PARAM, حرف **argv)
{
	بینښه حرف *دننه_buf;
	صحيح return_ارزښت = EXIT_SUCCESS;
	بینښه flags;
#إذا ENABLE_SHA3_مجموعه
	بینښه sha3_پلنوالی = 224;
#د_که_پای

	إذا (ENABLE_FEATURE_MD5_SHA1_مجموعه_CHECK) {
		/* -b "binary", -t "text" are ignored (shaNNNsum compat) */
		/* -s and -w require -c */
#إذا ENABLE_SHA3_مجموعه
		إذا (applet_name[3] == HASH_SHA3)
			flags = getopt32(argv, "^" "scwbta:+" "\0" "s?c:w?c", &sha3_پلنوالی);
		وإلا
#د_که_پای
			flags = getopt32(argv, "^" "scwbt" "\0" "s?c:w?c");
	} وإلا {
#إذا ENABLE_SHA3_مجموعه
		إذا (applet_name[3] == HASH_SHA3)
			getopt32(argv, "a:+", &sha3_پلنوالی);
		وإلا
#د_که_پای
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
	دننه_buf = xmalloc(BUFSZ);

	افعل {
		إذا (ENABLE_FEATURE_MD5_SHA1_مجموعه_CHECK && (flags & FLAG_CHECK)) {
			FILE *pre_computed_بهیر;
			حرف *کرښه;
			صحيح شمېر_ټول = 0;
			صحيح شمېر_failed = 0;

			pre_computed_بهیر = xfopen_stdin(*argv);

			طالما ((کرښه = xmalloc_fgetline(pre_computed_بهیر)) != عدم) {
				uint8_t *hash_ارزښت;
				حرف *filename_ptr;

				شمېر_ټول++;
				filename_ptr = strchr(کرښه, ' ');
				إذا (!filename_ptr) {
					إذا (flags & FLAG_WARN) {
						bb_simple_error_پیغام("invalid format");
					}
					شمېر_failed++;
					return_ارزښت = EXIT_FAILURE;
					حرر(کرښه);
					استمر;
				}
				*filename_ptr++ = '\0';
				/* coreutils 9.1 allows "HASH FILENAME" format,
				 * with only one space. Skip the 'correct'
				 * "  " or " *" delimiter if it is there:
				 */
				إذا (*filename_ptr == ' ' || *filename_ptr == '*')
					filename_ptr++;

				hash_ارزښت = hash_file(دننه_buf, filename_ptr, sha3_پلنوالی);

				إذا (hash_ارزښت && (strcasecmp((حرف*)hash_ارزښت, کرښه) == 0)) {
					إذا (!(flags & FLAG_SILENT))
						چاپ_کړه("%s: OK\n", filename_ptr);
				} وإلا {
					إذا (!(flags & FLAG_SILENT))
						چاپ_کړه("%s: FAILED\n", filename_ptr);
					شمېر_failed++;
					return_ارزښت = EXIT_FAILURE;
				}
				/* possible free(NULL) */
				حرر(hash_ارزښت);
				حرر(کرښه);
			}
			إذا (شمېر_failed && !(flags & FLAG_SILENT)) {
				bb_error_پیغام("WARNING: %d of %d computed checksums did NOT match",
						شمېر_failed, شمېر_ټول);
			}
			إذا (شمېر_ټول == 0) {
				return_ارزښت = EXIT_FAILURE;
				/*
				 * md5sum from GNU coreutils 8.25 says:
				 * md5sum: <FILE>: no properly formatted MD5 checksum lines found
				 */
				bb_error_پیغام("%s: no checksum lines found", *argv);
			}
			fclose_if_not_stdin(pre_computed_بهیر);
		} وإلا {
			uint8_t *hash_ارزښت = hash_file(دننه_buf, *argv, sha3_پلنوالی);
			إذا (hash_ارزښت == عدم) {
				return_ارزښت = EXIT_FAILURE;
			} وإلا {
				چاپ_کړه("%s  %s\n", hash_ارزښت, *argv);
				حرر(hash_ارزښت);
			}
		}
	} طالما (*++argv);

	أرجع return_ارزښت;
}
