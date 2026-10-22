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

#અન્તર્ભાવ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

ગણના {
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
સ્થિર unsigned અક્ષર *hash_bin_to_hex(unsigned અક્ષર *hash_કિંમત,
				unsigned hash_length)
{
	/* xzalloc zero-terminates */
	અક્ષર *hex_કિંમત = xzalloc((hash_length * 2) + 1);
	bin2hex(hex_કિંમત, (અક્ષર*)hash_કિંમત, hash_length);
	પાછા (unsigned અક્ષર *)hex_કિંમત;
}

#define BUFSZ (CONFIG_FEATURE_COPYBUF_KB < 4 ? 4096 : CONFIG_FEATURE_COPYBUF_KB * 1024)

#જો !ENABLE_SHA3SUM
# define hash_file(b,f,w) hash_file(b,f)
#endif
સ્થિર uint8_t *hash_file(unsigned અક્ષર *in_buf, અચલ અક્ષર *filename, unsigned sha3_width)
{
	પૂર્ણાંક src_fd, hash_len, ગણતરી;
	સંઘ _ctx_ {
		sha3_ctx_t sha3;
		sha512_ctx_t sha512;
		sha256_ctx_t sha256;
		sha1_ctx_t sha1;
		md5_ctx_t md5;
	} context;
	uint8_t *hash_કિંમત;
	શૂન્ય FAST_FUNC (*update)(શૂન્ય*, અચલ શૂન્ય*, size_t);
	unsigned FAST_FUNC (*final)(શૂન્ય*, શૂન્ય*);
	અક્ષર hash_algo;

	src_fd = open_or_warn_stdin(filename);
	જો (src_fd < 0) {
		પાછા ખાલી;
	}

	hash_algo = applet_name[3];

	/* figure specific hash algorithms */
	જો (ENABLE_MD5SUM && hash_algo == HASH_MD5) {
		md5_begin(&context.md5);
		update = (શૂન્ય*)md5_hash;
		final = (શૂન્ય*)md5_end;
		hash_len = 16;
	}
	નહીંતર જો (ENABLE_SHA1SUM && hash_algo == HASH_SHA1) {
		sha1_begin(&context.sha1);
		update = (શૂન્ય*)sha1_hash;
		final = (શૂન્ય*)sha1_end;
		hash_len = 20;
	}
	નહીંતર જો (ENABLE_SHA256SUM && hash_algo == HASH_SHA256) {
		sha256_begin(&context.sha256);
		update = (શૂન્ય*)sha256_hash;
		final = (શૂન્ય*)sha256_end;
		hash_len = 32;
	}
	નહીંતર જો (ENABLE_SHA512SUM && hash_algo == HASH_SHA512) {
		sha512_begin(&context.sha512);
		update = (શૂન્ય*)sha512_hash;
		final = (શૂન્ય*)sha512_end;
		hash_len = 64;
	}
#જો ENABLE_SHA3SUM
	નહીંતર જો (ENABLE_SHA3SUM && hash_algo == HASH_SHA3) {
		sha3_begin(&context.sha3);
		update = (શૂન્ય*)sha3_hash;
		final = (શૂન્ય*)sha3_end;
		/*
		 * Should support 224, 256, 384, 512.
		 * We allow any value which does not blow the algorithm up.
		 */
		જો (sha3_width >= 1600/2 /* input block can't be <= 0 */
		 || sha3_width == 0      /* hash len can't be 0 */
		 || (sha3_width & 0x1f)  /* should be multiple of 32 */
		/* (because input uses up to 8 byte wide word XORs. 32/4=8) */
		) {
			bb_error_સંદેશ_and_die("bad -a%u", sha3_width);
		}
		sha3_width /= 4;
		context.sha3.આગત_block_bytes = 1600/8 - sha3_width;
		hash_len = sha3_width/2;
	}
#endif
	નહીંતર {
		xfunc_die(); /* can't reach this */
	}

	{
		જ્યારે ((ગણતરી = safe_read(src_fd, in_buf, BUFSZ)) > 0) {
			update(&context, in_buf, ગણતરી);
		}
		hash_કિંમત = ખાલી;
		જો (ગણતરી < 0)
			bb_perror_સંદેશ("can't read '%s'", filename);
		નહીંતર /* count == 0 */ {
			final(&context, in_buf);
			hash_કિંમત = hash_bin_to_hex(in_buf, hash_len);
		}
	}

	જો (src_fd != STDIN_FILENO) {
		close(src_fd);
	}

	પાછા hash_કિંમત;
}

પૂર્ણાંક md5_sha1_sum_main(પૂર્ણાંક argc, અક્ષર **argv) MAIN_EXTERNALLY_VISIBLE;
પૂર્ણાંક md5_sha1_sum_main(પૂર્ણાંક argc UNUSED_PARAM, અક્ષર **argv)
{
	unsigned અક્ષર *in_buf;
	પૂર્ણાંક return_કિંમત = EXIT_SUCCESS;
	unsigned flags;
#જો ENABLE_SHA3SUM
	unsigned sha3_width = 224;
#endif

	જો (ENABLE_FEATURE_MD5_SHA1_SUM_CHECK) {
		/* -b "binary", -t "text" are ignored (shaNNNsum compat) */
		/* -s and -w require -c */
#જો ENABLE_SHA3SUM
		જો (applet_name[3] == HASH_SHA3)
			flags = getopt32(argv, "^" "scwbta:+" "\0" "s?c:w?c", &sha3_width);
		નહીંતર
#endif
			flags = getopt32(argv, "^" "scwbt" "\0" "s?c:w?c");
	} નહીંતર {
#જો ENABLE_SHA3SUM
		જો (applet_name[3] == HASH_SHA3)
			getopt32(argv, "a:+", &sha3_width);
		નહીંતર
#endif
			getopt32(argv, "");
	}
	argv += optind;
	//argc -= optind;
	જો (!*argv)
		*--argv = (અક્ષર*)"-";

	/* The buffer is not alloc/freed for each input file:
	 * for big values of COPYBUF_KB, this helps to keep its pages
	 * pre-faulted and possibly even fully cached on local CPU.
	 */
	in_buf = xmalloc(BUFSZ);

	કરો {
		જો (ENABLE_FEATURE_MD5_SHA1_SUM_CHECK && (flags & FLAG_CHECK)) {
			FILE *pre_computed_stream;
			અક્ષર *લીટી;
			પૂર્ણાંક ગણતરી_total = 0;
			પૂર્ણાંક ગણતરી_failed = 0;

			pre_computed_stream = xfopen_stdin(*argv);

			જ્યારે ((લીટી = xmalloc_fgetline(pre_computed_stream)) != ખાલી) {
				uint8_t *hash_કિંમત;
				અક્ષર *filename_ptr;

				ગણતરી_total++;
				filename_ptr = strchr(લીટી, ' ');
				જો (!filename_ptr) {
					જો (flags & FLAG_WARN) {
						bb_simple_error_સંદેશ("invalid format");
					}
					ગણતરી_failed++;
					return_કિંમત = EXIT_FAILURE;
					મુક્ત(લીટી);
					ચાલુ;
				}
				*filename_ptr++ = '\0';
				/* coreutils 9.1 allows "HASH FILENAME" format,
				 * with only one space. Skip the 'correct'
				 * "  " or " *" delimiter if it is there:
				 */
				જો (*filename_ptr == ' ' || *filename_ptr == '*')
					filename_ptr++;

				hash_કિંમત = hash_file(in_buf, filename_ptr, sha3_width);

				જો (hash_કિંમત && (strcasecmp((અક્ષર*)hash_કિંમત, લીટી) == 0)) {
					જો (!(flags & FLAG_SILENT))
						printf("%s: OK\n", filename_ptr);
				} નહીંતર {
					જો (!(flags & FLAG_SILENT))
						printf("%s: FAILED\n", filename_ptr);
					ગણતરી_failed++;
					return_કિંમત = EXIT_FAILURE;
				}
				/* possible free(NULL) */
				મુક્ત(hash_કિંમત);
				મુક્ત(લીટી);
			}
			જો (ગણતરી_failed && !(flags & FLAG_SILENT)) {
				bb_error_સંદેશ("WARNING: %d of %d computed checksums did NOT match",
						ગણતરી_failed, ગણતરી_total);
			}
			જો (ગણતરી_total == 0) {
				return_કિંમત = EXIT_FAILURE;
				/*
				 * md5sum from GNU coreutils 8.25 says:
				 * md5sum: <FILE>: no properly formatted MD5 checksum lines found
				 */
				bb_error_સંદેશ("%s: no checksum lines found", *argv);
			}
			fclose_if_not_stdin(pre_computed_stream);
		} નહીંતર {
			uint8_t *hash_કિંમત = hash_file(in_buf, *argv, sha3_width);
			જો (hash_કિંમત == ખાલી) {
				return_કિંમત = EXIT_FAILURE;
			} નહીંતર {
				printf("%s  %s\n", hash_કિંમત, *argv);
				મુક્ત(hash_કિંમત);
			}
		}
	} જ્યારે (*++argv);

	પાછા return_કિંમત;
}
