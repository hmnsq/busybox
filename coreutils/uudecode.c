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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"

#ਜੇ ENABLE_UUDECODE
ਸਥਿਰ ਖਾਲੀ FAST_FUNC read_stduu(FILE *src_ਧਾਰਾ, FILE *dst_ਧਾਰਾ, ਪੂਰਨ flags ਅਣਵਰਤਿਆ_PARAM)
{
	ਅੱਖਰ *ਸਤਰ;

	ਲਈ (;;) {
		ਪੂਰਨ encoded_len, str_len;
		ਅੱਖਰ *ਸਤਰ_ptr, *dst;
		ਮਾਪ_t ਸਤਰ_len;

		ਸਤਰ_len = 64 * 1024;
		ਸਤਰ = xmalloc_fgets_str_len(src_ਧਾਰਾ, "\n", &ਸਤਰ_len);
		ਜੇ (!ਸਤਰ)
			ਰੋਕੋ;
		/* Handle both Unix and MSDOS text.
		 * Note: space should not be trimmed, some encoders use it instead of "`"
		 * for padding of last incomplete 4-char block.
		 */
		str_len = ਸਤਰ_len;
		ਜਦੋਂ (--str_len >= 0
		 && (ਸਤਰ[str_len] == '\n' || ਸਤਰ[str_len] == '\r')
		) {
			ਸਤਰ[str_len] = '\0';
		}

		ਜੇ (strcmp(ਸਤਰ, "end") == 0) {
			ਵਾਪਸ; /* the only non-error exit */
		}

		ਸਤਰ_ptr = ਸਤਰ;
		ਜਦੋਂ (*ਸਤਰ_ptr) {
			*ਸਤਰ_ptr = (*ਸਤਰ_ptr - 0x20) & 0x3f;
			ਸਤਰ_ptr++;
		}
		str_len = ਸਤਰ_ptr - ਸਤਰ;

		encoded_len = ਸਤਰ[0] * 4 / 3;
		/* Check that line is not too short. (we tolerate
		 * overly _long_ line to accommodate possible extra "`").
		 * Empty line case is also caught here. */
		ਜੇ (str_len <= encoded_len) {
			ਰੋਕੋ; /* go to bb_error_msg_and_die("short file"); */
		}
		ਜੇ (encoded_len <= 0) {
			/* Ignore the "`\n" line, why is it even in the encode file ? */
			ਮੁਕਤ(ਸਤਰ);
			ਜਾਰੀ;
		}
		ਜੇ (encoded_len > 60) {
			bb_simple_error_ਸੁਨੇਹਾ_and_die("line too long");
		}

		dst = ਸਤਰ;
		ਸਤਰ_ptr = ਸਤਰ + 1;
		ਕਰੋ {
			/* Merge four 6 bit chars to three 8 bit chars */
			*dst++ = ਸਤਰ_ptr[0] << 2 | ਸਤਰ_ptr[1] >> 4;
			encoded_len--;
			ਜੇ (encoded_len == 0) {
				ਰੋਕੋ;
			}

			*dst++ = ਸਤਰ_ptr[1] << 4 | ਸਤਰ_ptr[2] >> 2;
			encoded_len--;
			ਜੇ (encoded_len == 0) {
				ਰੋਕੋ;
			}

			*dst++ = ਸਤਰ_ptr[2] << 6 | ਸਤਰ_ptr[3];
			ਸਤਰ_ptr += 4;
			encoded_len -= 2;
		} ਜਦੋਂ (encoded_len > 0);
		fwrite(ਸਤਰ, 1, dst - ਸਤਰ, dst_ਧਾਰਾ);
		ਮੁਕਤ(ਸਤਰ);
	}
	bb_simple_error_ਸੁਨੇਹਾ_and_die("short file");
}

ਪੂਰਨ uudecode_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ uudecode_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
	FILE *src_ਧਾਰਾ;
	ਅੱਖਰ *outname = ਨਿਰਾ;
	ਅੱਖਰ *ਸਤਰ;

	getopt32(argv, "^" "o:" "\0" "?1"/* 1 arg max*/, &outname);
	argv += optind;

	ਜੇ (!argv[0])
		*--argv = (ਅੱਖਰ*)"-";
	src_ਧਾਰਾ = xfopen_stdin(argv[0]);

	/* Search for the start of the encoding */
	ਜਦੋਂ ((ਸਤਰ = xmalloc_fgetline(src_ਧਾਰਾ)) != ਨਿਰਾ) {
		ਖਾਲੀ FAST_FUNC (*decode_fn_ptr)(FILE *src, FILE *dst, ਪੂਰਨ flags);
		ਅੱਖਰ *ਸਤਰ_ptr;
		FILE *dst_ਧਾਰਾ;
		ਪੂਰਨ ਬਹੁਲਕ;

		ਜੇ (is_prefixed_with(ਸਤਰ, "begin-base64 ")) {
			ਸਤਰ_ptr = ਸਤਰ + 13;
			decode_fn_ptr = read_base64;
		} ਨਹੀਂਤਾਂ ਜੇ (is_prefixed_with(ਸਤਰ, "begin ")) {
			ਸਤਰ_ptr = ਸਤਰ + 6;
			decode_fn_ptr = read_stduu;
		} ਨਹੀਂਤਾਂ {
			ਮੁਕਤ(ਸਤਰ);
			ਜਾਰੀ;
		}

		/* begin line found. decode and exit */
		ਬਹੁਲਕ = bb_strtou(ਸਤਰ_ptr, ਨਿਰਾ, 8);
		ਜੇ (outname == ਨਿਰਾ) {
			outname = strchr(ਸਤਰ_ptr, ' ');
			ਜੇ (!outname)
				ਰੋਕੋ;
			outname++;
			trim(outname); /* remove trailing space (and '\r' for DOS text) */
			ਜੇ (!outname[0])
				ਰੋਕੋ;
		}
		dst_ਧਾਰਾ = stdout;
		ਜੇ (NOT_LONE_DASH(outname)
/* https://pubs.opengroup.org/onlinepubs/9699919799/utilities/uudecode.html
 * https://pubs.opengroup.org/onlinepubs/9699919799/utilities/uuencode.html
 * The above says that output file name specified in input file
 * or overridden by -o OUTFILE can be special "/dev/stdout" string.
 * This usually works "implicitly": many systems have /dev/stdout.
 * If ENABLE_DESKTOP, support that explicitly:
 */
		 && (!ENABLE_DESKTOP || strcmp(outname, "/dev/stdout") != 0)
		) {
			dst_ਧਾਰਾ = xfopen_for_write(outname);
			fchmod(fileno(dst_ਧਾਰਾ), ਬਹੁਲਕ & (S_IRWXU | S_IRWXG | S_IRWXO));
		}
		ਮੁਕਤ(ਸਤਰ);
		decode_fn_ptr(src_ਧਾਰਾ, dst_ਧਾਰਾ, /*flags:*/ BASE64_FLAG_UU_STOP + BASE64_FLAG_NO_STOP_CHAR);
		/* fclose_if_not_stdin(src_stream); - redundant */
		ਵਾਪਸ EXIT_SUCCESS;
	}
	bb_simple_error_ਸੁਨੇਹਾ_and_die("no 'begin' line");
}
#ਜੇ_ਅੰਤ

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

#ਜੇ ENABLE_BASE32 || ENABLE_BASE64

# ਜੇ ENABLE_BASE32
ਸਥਿਰ ਖਾਲੀ bb_b32encode(ਅੱਖਰ *p, ਅਚਲ ਖਾਲੀ *src, ਪੂਰਨ ਲੰਮਾਈ)
{
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ tbl bb_uuenc_tbl_base32
	ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *s = src;

	/* Transform 5x8 bits to 8x5 bits */
	ਜਦੋਂ (ਲੰਮਾਈ > 0) {
		ਬਿਨਾਨਿਸ਼ਾਨੀ cur, ਅਗਲਾ;

		ਲੰਮਾਈ--;
		cur = *s++;
		*p++ = tbl[cur >> 3];			// xxxxx--- -------- -------- -------- --------
		cur &= 7;

		ਅਗਲਾ = 0;
		ਜੇ (--ਲੰਮਾਈ >= 0)
			ਅਗਲਾ = *s++;
		*p++ = tbl[(cur << 2) + (ਅਗਲਾ >> 6)];	// -----xxx xx------ -------- -------- --------
		cur = ਅਗਲਾ & 0x3f;

		*p++ = tbl[cur >> 1];			// -------- --xxxxx- -------- -------- --------
		cur &= 1;

		ਅਗਲਾ = 0;
		ਜੇ (--ਲੰਮਾਈ >= 0)
			ਅਗਲਾ = *s++;
		*p++ = tbl[(cur << 4) + (ਅਗਲਾ >> 4)];	// -------- -------x xxxx---- -------- --------
		cur = ਅਗਲਾ & 0xf;

		ਅਗਲਾ = 0;
		ਜੇ (--ਲੰਮਾਈ >= 0)
			ਅਗਲਾ = *s++;
		*p++ = tbl[(cur << 1) + (ਅਗਲਾ >> 7)];	// -------- -------- ----xxxx x------- --------
		cur = ਅਗਲਾ & 0x7f;

		*p++ = tbl[cur >> 2];			// -------- -------- -------- -xxxxx-- --------
		cur &= 3;

		ਅਗਲਾ = 0;
		ਜੇ (--ਲੰਮਾਈ >= 0)
			ਅਗਲਾ = *s++;
		*p++ = tbl[(cur << 3) + (ਅਗਲਾ >> 5)];	// -------- -------- -------- ------xx xxx-----
		cur = ਅਗਲਾ & 0x1f;

		*p++ = tbl[cur];			// -------- -------- -------- -------- ---xxxxx
	}
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ tbl
	/* Zero-terminate */
	*p = '\0';
	/* Pad as necessary */
	ਲੰਮਾਈ = ((-ਲੰਮਾਈ) * 3) >> 1; /* -4 => 6 pad chars, -3 => 4, -2 => 3, -1 => 1 */
	ਜਦੋਂ (ਲੰਮਾਈ--) {
		*--p = '=';
	}
}
# ਨਹੀਂਤਾਂ
ਖਾਲੀ bb_b32encode(ਅੱਖਰ *p, ਅਚਲ ਖਾਲੀ *src, ਪੂਰਨ ਲੰਮਾਈ); /* undefined */
# endif

ਪੂਰਨ ਅਧਾਰ_ਅੰਕ_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ ਅਧਾਰ_ਅੰਕ_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
	FILE *src_ਧਾਰਾ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ opts;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਖਾਨਾ = 76;

	opts = getopt32(argv, "^" "diw:+" "\0" "?1"/* 1 arg max*/, &ਖਾਨਾ);
	argv += optind;

	ਜੇ (!argv[0])
		*--argv = (ਅੱਖਰ*)"-";
	src_ਧਾਰਾ = xfopen_stdin(argv[0]);
	ਜੇ (opts & 1) {
		/* -d: decode */
		ਪੂਰਨ flags = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)EOF;
		ਜੇ (ENABLE_BASE32 && (!ENABLE_BASE64 || applet_name[4] == '3'))
			flags = ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)EOF) | BASE64_32;
		read_base64(src_ਧਾਰਾ, stdout, flags);
	} ਨਹੀਂਤਾਂ {
		ਗਿਣਤੀ {
			SRC_BUF_ਮਾਪ = 3 * 5 * 32, /* this *MUST* be a multiple of 3 and 5 */
			DST_BUF_ਮਾਪ = 8 * ((SRC_BUF_ਮਾਪ + 4) / 5), /* max growth on encode (base32 case) */
		};
		/* Use one buffer for both input and output:
		 * encoding reads input "left-to-right",
		 * it's safe to place source at the end of the buffer and
		 * overwrite it while encoding, just be careful to have a gap.
		 */
		ਅੱਖਰ dst_buf[((DST_BUF_ਮਾਪ + /*gap:*/ 16) /*round up to 16:*/ | 0xf) + 1];
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ src_buf (dst_buf + ਆਕਾਰ(dst_buf) - SRC_BUF_ਮਾਪ)
		ਪੂਰਨ src_fd, rem;

		src_fd = fileno(src_ਧਾਰਾ);
		rem = 0;
		ਜਦੋਂ (1) {
			ਮਾਪ_t ਮਾਪ = ਭਰਿਆ_read(src_fd, src_buf, SRC_BUF_ਮਾਪ);
			ਜੇ ((ssize_t)ਮਾਪ < 0)
				bb_simple_perror_ਸੁਨੇਹਾ_and_die(bb_ਸੁਨੇਹਾ_read_error);
			ਜੇ (ਮਾਪ == 0) {
				ਜੇ (rem != 0) bb_putchar('\n');
				ਰੋਕੋ;
			}

			/* Encode the buffer we just read in */
			ਜੇ (ENABLE_BASE32 && (!ENABLE_BASE64 || applet_name[4] == '3')) {
				bb_b32encode(dst_buf, src_buf, ਮਾਪ);
				ਮਾਪ = 8 * ((ਮਾਪ + 4) / 5);
			} ਨਹੀਂਤਾਂ {
				bb_uuencode(dst_buf, src_buf, ਮਾਪ, bb_uuenc_tbl_base64);
				ਮਾਪ = 4 * ((ਮਾਪ + 2) / 3);
			}

			ਜੇ (ਖਾਨਾ == 0) {
				fputs_stdout(dst_buf);
			} ਨਹੀਂਤਾਂ {
				ਅੱਖਰ *ਨਤੀਜਾ = dst_buf;
				ਜੇ (rem == 0)
					rem = ਖਾਨਾ;
				ਜਦੋਂ (1) {
					ਪੂਰਨ ਬਾਹਰਲਾ = ਮਾਪ < rem ? ਮਾਪ : rem;
					rem -= ਬਾਹਰਲਾ;
					ਲਿਖੋ(rem != 0 ? "%.*s" : "%.*s\n", ਬਾਹਰਲਾ, ਨਤੀਜਾ);
					ਜੇ (rem != 0)
						ਰੋਕੋ;
					ਮਾਪ -= ਬਾਹਰਲਾ;
					ਜੇ (ਮਾਪ == 0)
						ਰੋਕੋ;
					ਨਤੀਜਾ += ਬਾਹਰਲਾ;
					rem = ਖਾਨਾ;
				}
			}
		}
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ src_buf
	}

	fflush_stdout_and_exit_SUCCESS();
}
#ਜੇ_ਅੰਤ

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
ਪੂਰਾ
mkdir unpk_u unpk_m 2>/dev/null
for f in u_*; do
    ./uudecode <$f -o unpk_u/${f:2}
    diff -a ${f:2} unpk_u/${f:2} >/dev/null 2>&1
    echo uudecode $f: $?
ਪੂਰਾ
for f in m_*; do
    ./uudecode <$f -o unpk_m/${f:2}
    diff -a ${f:2} unpk_m/${f:2} >/dev/null 2>&1
    echo uudecode $f: $?
ਪੂਰਾ
*/
