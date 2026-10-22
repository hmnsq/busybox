/* vi: set sw=4 ts=4: */
/*
 * Copyright 2003, Glenn McGrath
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 *
 * Based on specification from
 * http://www.opengroup.org/onlinepubs/007904975/utilities/uuencode.html
 *
 * Bugs: the spec doesn't mention anything about "`\n`\n" prior to the
 * "end" line
 */
//config:config UUDECODE
//config:	bool "uudecode (5.9 kb)"
//config:	default y
//config:	help
//config:	uudecode is used to decode a uuencoded file.

//applet:IF_UUDECODE(APPLET(uudecode, BB_DIR_USR_BIN, BB_SUID_DROP))

//kbuild:lib-$(CONFIG_UUDECODE) += uudecode.o

//usage:#define uudecode_trivial_usage
//usage:       "[-o OUTFILE] [INFILE]"
//usage:#define uudecode_full_usage "\n\n"
//usage:       "Uudecode a file\n"
//usage:       "Finds OUTFILE in uuencoded source unless -o is given"
//usage:
//usage:#define uudecode_example_usage
//usage:       "$ uudecode -o busybox busybox.uu\n"
//usage:       "$ ls -l busybox\n"
//usage:       "-rwxr-xr-x   1 ams      ams        245264 Jun  7 21:35 busybox\n"

#ಅನ್ತರ್ಭಾವ "libbb.h"

#ಆದರೆ ENABLE_UUDECODE
ಸ್ಥಿರ ಶೂನ್ಯ FAST_FUNC read_stduu(FILE *src_ಹರಿವು, FILE *dst_ಹರಿವು, ಪೂರ್ಣಾಂಕ flags ಬಳಸದ_PARAM)
{
	ಅಕ್ಷರ *ಸಾಲು;

	ಗಾಗಿ (;;) {
		ಪೂರ್ಣಾಂಕ encoded_len, str_len;
		ಅಕ್ಷರ *ಸಾಲು_ptr, *dst;
		ಗಾತ್ರ_t ಸಾಲು_len;

		ಸಾಲು_len = 64 * 1024;
		ಸಾಲು = xmalloc_fgets_str_len(src_ಹರಿವು, "\n", &ಸಾಲು_len);
		ಆದರೆ (!ಸಾಲು)
			ನಿಲ್ಲಿಸು;
		/* Handle both Unix and MSDOS text.
		 * Note: space should not be trimmed, some encoders use it instead of "`"
		 * for padding of last incomplete 4-char block.
		 */
		str_len = ಸಾಲು_len;
		ಆಗ (--str_len >= 0
		 && (ಸಾಲು[str_len] == '\n' || ಸಾಲು[str_len] == '\r')
		) {
			ಸಾಲು[str_len] = '\0';
		}

		ಆದರೆ (strcmp(ಸಾಲು, "end") == 0) {
			ಹಿಂತಿರುಗಿ; /* the only non-error exit */
		}

		ಸಾಲು_ptr = ಸಾಲು;
		ಆಗ (*ಸಾಲು_ptr) {
			*ಸಾಲು_ptr = (*ಸಾಲು_ptr - 0x20) & 0x3f;
			ಸಾಲು_ptr++;
		}
		str_len = ಸಾಲು_ptr - ಸಾಲು;

		encoded_len = ಸಾಲು[0] * 4 / 3;
		/* Check that line is not too short. (we tolerate
		 * overly _long_ line to accommodate possible extra "`").
		 * Empty line case is also caught here. */
		ಆದರೆ (str_len <= encoded_len) {
			ನಿಲ್ಲಿಸು; /* go to bb_error_msg_and_die("short file"); */
		}
		ಆದರೆ (encoded_len <= 0) {
			/* Ignore the "`\n" line, why is it even in the encode file ? */
			ಬಿಡುಗಡೆ(ಸಾಲು);
			ಮುಂದುವರಿ;
		}
		ಆದರೆ (encoded_len > 60) {
			bb_simple_error_ಸಂದೇಶ_and_die("line too long");
		}

		dst = ಸಾಲು;
		ಸಾಲು_ptr = ಸಾಲು + 1;
		ಮಾಡು {
			/* Merge four 6 bit chars to three 8 bit chars */
			*dst++ = ಸಾಲು_ptr[0] << 2 | ಸಾಲು_ptr[1] >> 4;
			encoded_len--;
			ಆದರೆ (encoded_len == 0) {
				ನಿಲ್ಲಿಸು;
			}

			*dst++ = ಸಾಲು_ptr[1] << 4 | ಸಾಲು_ptr[2] >> 2;
			encoded_len--;
			ಆದರೆ (encoded_len == 0) {
				ನಿಲ್ಲಿಸು;
			}

			*dst++ = ಸಾಲು_ptr[2] << 6 | ಸಾಲು_ptr[3];
			ಸಾಲು_ptr += 4;
			encoded_len -= 2;
		} ಆಗ (encoded_len > 0);
		fwrite(ಸಾಲು, 1, dst - ಸಾಲು, dst_ಹರಿವು);
		ಬಿಡುಗಡೆ(ಸಾಲು);
	}
	bb_simple_error_ಸಂದೇಶ_and_die("short file");
}

ಪೂರ್ಣಾಂಕ uudecode_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ uudecode_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	FILE *src_ಹರಿವು;
	ಅಕ್ಷರ *outname = ಖಾಲಿ;
	ಅಕ್ಷರ *ಸಾಲು;

	getopt32(argv, "^" "o:" "\0" "?1"/* 1 arg max*/, &outname);
	argv += optind;

	ಆದರೆ (!argv[0])
		*--argv = (ಅಕ್ಷರ*)"-";
	src_ಹರಿವು = xfopen_stdin(argv[0]);

	/* Search for the start of the encoding */
	ಆಗ ((ಸಾಲು = xmalloc_fgetline(src_ಹರಿವು)) != ಖಾಲಿ) {
		ಶೂನ್ಯ FAST_FUNC (*decode_fn_ptr)(FILE *src, FILE *dst, ಪೂರ್ಣಾಂಕ flags);
		ಅಕ್ಷರ *ಸಾಲು_ptr;
		FILE *dst_ಹರಿವು;
		ಪೂರ್ಣಾಂಕ ಬಹುಲಕ;

		ಆದರೆ (is_prefixed_with(ಸಾಲು, "begin-base64 ")) {
			ಸಾಲು_ptr = ಸಾಲು + 13;
			decode_fn_ptr = read_base64;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (is_prefixed_with(ಸಾಲು, "begin ")) {
			ಸಾಲು_ptr = ಸಾಲು + 6;
			decode_fn_ptr = read_stduu;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಬಿಡುಗಡೆ(ಸಾಲು);
			ಮುಂದುವರಿ;
		}

		/* begin line found. decode and exit */
		ಬಹುಲಕ = bb_strtou(ಸಾಲು_ptr, ಖಾಲಿ, 8);
		ಆದರೆ (outname == ಖಾಲಿ) {
			outname = strchr(ಸಾಲು_ptr, ' ');
			ಆದರೆ (!outname)
				ನಿಲ್ಲಿಸು;
			outname++;
			trim(outname); /* remove trailing space (and '\r' for DOS text) */
			ಆದರೆ (!outname[0])
				ನಿಲ್ಲಿಸು;
		}
		dst_ಹರಿವು = stdout;
		ಆದರೆ (NOT_LONE_DASH(outname)
/* https://pubs.opengroup.org/onlinepubs/9699919799/utilities/uudecode.html
 * https://pubs.opengroup.org/onlinepubs/9699919799/utilities/uuencode.html
 * The above says that output file name specified in input file
 * or overridden by -o OUTFILE can be special "/dev/stdout" string.
 * This usually works "implicitly": many systems have /dev/stdout.
 * If ENABLE_DESKTOP, support that explicitly:
 */
		 && (!ENABLE_DESKTOP || strcmp(outname, "/dev/stdout") != 0)
		) {
			dst_ಹರಿವು = xfopen_for_write(outname);
			fchmod(fileno(dst_ಹರಿವು), ಬಹುಲಕ & (S_IRWXU | S_IRWXG | S_IRWXO));
		}
		ಬಿಡುಗಡೆ(ಸಾಲು);
		decode_fn_ptr(src_ಹರಿವು, dst_ಹರಿವು, /*flags:*/ BASE64_FLAG_UU_STOP + BASE64_FLAG_NO_STOP_CHAR);
		/* fclose_if_not_stdin(src_stream); - redundant */
		ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
	}
	bb_simple_error_ಸಂದೇಶ_and_die("no 'begin' line");
}
#ಆದರೆ_ಅಂತ್ಯ

//config:config BASE32
//config:	bool "base32 (5.5 kb)"
//config:	default y
//config:	help
//config:	Base32 encode and decode

//config:config BASE64
//config:	bool "base64 (5.3 kb)"
//config:	default y
//config:	help
//config:	Base64 encode and decode

//usage:#define base32_trivial_usage
//usage:	"[-d] [-w COL] [FILE]"
//usage:#define base32_full_usage "\n\n"
//usage:       "Base32 encode or decode FILE to standard output\n"
//usage:     "\n	-d	Decode data"
//usage:     "\n	-w COL	Wrap lines at COL (default 76, 0 disables)"
////usage:     "\n	-i	When decoding, ignore non-alphabet characters"

//usage:#define base64_trivial_usage
//usage:	"[-d] [-w COL] [FILE]"
//usage:#define base64_full_usage "\n\n"
//usage:       "Base64 encode or decode FILE to standard output\n"
//usage:     "\n	-d	Decode data"
//usage:     "\n	-w COL	Wrap lines at COL (default 76, 0 disables)"
///////:     "\n	-i	When decoding, ignore non-alphabet characters"
// -i is accepted but has no effect: currently, decode_base32/64() functions
// (called via read_base64()) skip invalid chars unconditionally.

//                 APPLET_ODDNAME:name    main     location    suid_type     help
//applet:IF_BASE32(APPLET_ODDNAME(base32, baseNUM, BB_DIR_BIN, BB_SUID_DROP, base32))
//applet:IF_BASE64(APPLET_ODDNAME(base64, baseNUM, BB_DIR_BIN, BB_SUID_DROP, base64))

//kbuild:lib-$(CONFIG_BASE64) += uudecode.o
//kbuild:lib-$(CONFIG_BASE32) += uudecode.o

#ಆದರೆ ENABLE_BASE32 || ENABLE_BASE64

# ಆದರೆ ENABLE_BASE32
ಸ್ಥಿರ ಶೂನ್ಯ bb_b32encode(ಅಕ್ಷರ *p, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *src, ಪೂರ್ಣಾಂಕ ಉದ್ದಳತೆ)
{
#ವ್ಯಾಖ್ಯಾನಿಸು tbl bb_uuenc_tbl_base32
	ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *s = src;

	/* Transform 5x8 bits to 8x5 bits */
	ಆಗ (ಉದ್ದಳತೆ > 0) {
		ಚಿಹ್ನೆರಹಿತ cur, ಮುಂದಿನ;

		ಉದ್ದಳತೆ--;
		cur = *s++;
		*p++ = tbl[cur >> 3];			// xxxxx--- -------- -------- -------- --------
		cur &= 7;

		ಮುಂದಿನ = 0;
		ಆದರೆ (--ಉದ್ದಳತೆ >= 0)
			ಮುಂದಿನ = *s++;
		*p++ = tbl[(cur << 2) + (ಮುಂದಿನ >> 6)];	// -----xxx xx------ -------- -------- --------
		cur = ಮುಂದಿನ & 0x3f;

		*p++ = tbl[cur >> 1];			// -------- --xxxxx- -------- -------- --------
		cur &= 1;

		ಮುಂದಿನ = 0;
		ಆದರೆ (--ಉದ್ದಳತೆ >= 0)
			ಮುಂದಿನ = *s++;
		*p++ = tbl[(cur << 4) + (ಮುಂದಿನ >> 4)];	// -------- -------x xxxx---- -------- --------
		cur = ಮುಂದಿನ & 0xf;

		ಮುಂದಿನ = 0;
		ಆದರೆ (--ಉದ್ದಳತೆ >= 0)
			ಮುಂದಿನ = *s++;
		*p++ = tbl[(cur << 1) + (ಮುಂದಿನ >> 7)];	// -------- -------- ----xxxx x------- --------
		cur = ಮುಂದಿನ & 0x7f;

		*p++ = tbl[cur >> 2];			// -------- -------- -------- -xxxxx-- --------
		cur &= 3;

		ಮುಂದಿನ = 0;
		ಆದರೆ (--ಉದ್ದಳತೆ >= 0)
			ಮುಂದಿನ = *s++;
		*p++ = tbl[(cur << 3) + (ಮುಂದಿನ >> 5)];	// -------- -------- -------- ------xx xxx-----
		cur = ಮುಂದಿನ & 0x1f;

		*p++ = tbl[cur];			// -------- -------- -------- -------- ---xxxxx
	}
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು tbl
	/* Zero-terminate */
	*p = '\0';
	/* Pad as necessary */
	ಉದ್ದಳತೆ = ((-ಉದ್ದಳತೆ) * 3) >> 1; /* -4 => 6 pad chars, -3 => 4, -2 => 3, -1 => 1 */
	ಆಗ (ಉದ್ದಳತೆ--) {
		*--p = '=';
	}
}
# ಇಲ್ಲದಿದ್ದರೆ
ಶೂನ್ಯ bb_b32encode(ಅಕ್ಷರ *p, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *src, ಪೂರ್ಣಾಂಕ ಉದ್ದಳತೆ); /* undefined */
# endif

ಪೂರ್ಣಾಂಕ ತಳ_ಅಂಕೆ_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ ತಳ_ಅಂಕೆ_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	FILE *src_ಹರಿವು;
	ಚಿಹ್ನೆರಹಿತ opts;
	ಚಿಹ್ನೆರಹಿತ ಲಂಬಸಾಲು = 76;

	opts = getopt32(argv, "^" "diw:+" "\0" "?1"/* 1 arg max*/, &ಲಂಬಸಾಲು);
	argv += optind;

	ಆದರೆ (!argv[0])
		*--argv = (ಅಕ್ಷರ*)"-";
	src_ಹರಿವು = xfopen_stdin(argv[0]);
	ಆದರೆ (opts & 1) {
		/* -d: decode */
		ಪೂರ್ಣಾಂಕ flags = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)EOF;
		ಆದರೆ (ENABLE_BASE32 && (!ENABLE_BASE64 || applet_name[4] == '3'))
			flags = ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)EOF) | BASE64_32;
		read_base64(src_ಹರಿವು, stdout, flags);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಎಣಿಕೆ {
			SRC_BUF_ಗಾತ್ರ = 3 * 5 * 32, /* this *MUST* be a multiple of 3 and 5 */
			DST_BUF_ಗಾತ್ರ = 8 * ((SRC_BUF_ಗಾತ್ರ + 4) / 5), /* max growth on encode (base32 case) */
		};
		/* Use one buffer for both input and output:
		 * encoding reads input "left-to-right",
		 * it's safe to place source at the end of the buffer and
		 * overwrite it while encoding, just be careful to have a gap.
		 */
		ಅಕ್ಷರ dst_buf[((DST_BUF_ಗಾತ್ರ + /*gap:*/ 16) /*round up to 16:*/ | 0xf) + 1];
#ವ್ಯಾಖ್ಯಾನಿಸು src_buf (dst_buf + ಗಾತ್ರ(dst_buf) - SRC_BUF_ಗಾತ್ರ)
		ಪೂರ್ಣಾಂಕ src_fd, rem;

		src_fd = fileno(src_ಹರಿವು);
		rem = 0;
		ಆಗ (1) {
			ಗಾತ್ರ_t ಗಾತ್ರ_2 = ತುಂಬಿದ_read(src_fd, src_buf, SRC_BUF_ಗಾತ್ರ);
			ಆದರೆ ((ssize_t)ಗಾತ್ರ_2 < 0)
				bb_simple_perror_ಸಂದೇಶ_and_die(bb_ಸಂದೇಶ_read_error);
			ಆದರೆ (ಗಾತ್ರ_2 == 0) {
				ಆದರೆ (rem != 0) bb_putchar('\n');
				ನಿಲ್ಲಿಸು;
			}

			/* Encode the buffer we just read in */
			ಆದರೆ (ENABLE_BASE32 && (!ENABLE_BASE64 || applet_name[4] == '3')) {
				bb_b32encode(dst_buf, src_buf, ಗಾತ್ರ_2);
				ಗಾತ್ರ_2 = 8 * ((ಗಾತ್ರ_2 + 4) / 5);
			} ಇಲ್ಲದಿದ್ದರೆ {
				bb_uuencode(dst_buf, src_buf, ಗಾತ್ರ_2, bb_uuenc_tbl_base64);
				ಗಾತ್ರ_2 = 4 * ((ಗಾತ್ರ_2 + 2) / 3);
			}

			ಆದರೆ (ಲಂಬಸಾಲು == 0) {
				fputs_stdout(dst_buf);
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಅಕ್ಷರ *ಫಲಿತಾಂಶ = dst_buf;
				ಆದರೆ (rem == 0)
					rem = ಲಂಬಸಾಲು;
				ಆಗ (1) {
					ಪೂರ್ಣಾಂಕ ಹೊರಹೋಗು = ಗಾತ್ರ_2 < rem ? ಗಾತ್ರ_2 : rem;
					rem -= ಹೊರಹೋಗು;
					ಬರೆ(rem != 0 ? "%.*s" : "%.*s\n", ಹೊರಹೋಗು, ಫಲಿತಾಂಶ);
					ಆದರೆ (rem != 0)
						ನಿಲ್ಲಿಸು;
					ಗಾತ್ರ_2 -= ಹೊರಹೋಗು;
					ಆದರೆ (ಗಾತ್ರ_2 == 0)
						ನಿಲ್ಲಿಸು;
					ಫಲಿತಾಂಶ += ಹೊರಹೋಗು;
					rem = ಲಂಬಸಾಲು;
				}
			}
		}
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು src_buf
	}

	fflush_stdout_and_exit_SUCCESS();
}
#ಆದರೆ_ಅಂತ್ಯ

/* Test script.
Put this into an empty dir with busybox binary, an run.

#!/bin/sh
test -x busybox || { echo "No ./busybox?"; exit; }
ln -sf busybox uudecode
ln -sf busybox uuencode
>A_null
echo -n A >A
echo -n AB >AB
echo -n ABC >ABC
echo -n ABCD >ABCD
echo -n ABCDE >ABCDE
echo -n ABCDEF >ABCDEF
cat busybox >A_bbox
for f in A*; do
    echo uuencode $f
    ./uuencode    $f <$f >u_$f
    ./uuencode -m $f <$f >m_$f
ಮುಗಿದ
mkdir unpk_u unpk_m 2>/dev/null
for f in u_*; do
    ./uudecode <$f -o unpk_u/${f:2}
    diff -a ${f:2} unpk_u/${f:2} >/dev/null 2>&1
    echo uudecode $f: $?
ಮುಗಿದ
for f in m_*; do
    ./uudecode <$f -o unpk_m/${f:2}
    diff -a ${f:2} unpk_m/${f:2} >/dev/null 2>&1
    echo uudecode $f: $?
ಮುಗಿದ
*/
