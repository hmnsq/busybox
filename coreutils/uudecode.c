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

#ضمن "libbb.h"

#إذا ENABLE_UUDECODE
ساكن فراغ FAST_FUNC read_stduu(FILE *src_دھارا, FILE *dst_دھارا, صحيح flags بن_ورتیا_PARAM)
{
	حرف *سطر;

	لكل (;;) {
		صحيح encoded_len, str_len;
		حرف *سطر_ptr, *dst;
		ناپ_t سطر_len;

		سطر_len = 64 * 1024;
		سطر = xmalloc_fgets_str_len(src_دھارا, "\n", &سطر_len);
		إذا (!سطر)
			اكسر;
		/* Handle both Unix and MSDOS text.
		 * Note: space should not be trimmed, some encoders use it instead of "`"
		 * for padding of last incomplete 4-char block.
		 */
		str_len = سطر_len;
		طالما (--str_len >= 0
		 && (سطر[str_len] == '\n' || سطر[str_len] == '\r')
		) {
			سطر[str_len] = '\0';
		}

		إذا (strcmp(سطر, "end") == 0) {
			أرجع; /* the only non-error exit */
		}

		سطر_ptr = سطر;
		طالما (*سطر_ptr) {
			*سطر_ptr = (*سطر_ptr - 0x20) & 0x3f;
			سطر_ptr++;
		}
		str_len = سطر_ptr - سطر;

		encoded_len = سطر[0] * 4 / 3;
		/* Check that line is not too short. (we tolerate
		 * overly _long_ line to accommodate possible extra "`").
		 * Empty line case is also caught here. */
		إذا (str_len <= encoded_len) {
			اكسر; /* go to bb_error_msg_and_die("short file"); */
		}
		إذا (encoded_len <= 0) {
			/* Ignore the "`\n" line, why is it even in the encode file ? */
			حرر(سطر);
			استمر;
		}
		إذا (encoded_len > 60) {
			bb_simple_error_پیغام_and_die("line too long");
		}

		dst = سطر;
		سطر_ptr = سطر + 1;
		افعل {
			/* Merge four 6 bit chars to three 8 bit chars */
			*dst++ = سطر_ptr[0] << 2 | سطر_ptr[1] >> 4;
			encoded_len--;
			إذا (encoded_len == 0) {
				اكسر;
			}

			*dst++ = سطر_ptr[1] << 4 | سطر_ptr[2] >> 2;
			encoded_len--;
			إذا (encoded_len == 0) {
				اكسر;
			}

			*dst++ = سطر_ptr[2] << 6 | سطر_ptr[3];
			سطر_ptr += 4;
			encoded_len -= 2;
		} طالما (encoded_len > 0);
		fwrite(سطر, 1, dst - سطر, dst_دھارا);
		حرر(سطر);
	}
	bb_simple_error_پیغام_and_die("short file");
}

صحيح uudecode_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح uudecode_main(صحيح argc بن_ورتیا_PARAM, حرف **argv)
{
	FILE *src_دھارا;
	حرف *outname = عدم;
	حرف *سطر;

	getopt32(argv, "^" "o:" "\0" "?1"/* 1 arg max*/, &outname);
	argv += optind;

	إذا (!argv[0])
		*--argv = (حرف*)"-";
	src_دھارا = xfopen_stdin(argv[0]);

	/* Search for the start of the encoding */
	طالما ((سطر = xmalloc_fgetline(src_دھارا)) != عدم) {
		فراغ FAST_FUNC (*decode_fn_ptr)(FILE *src, FILE *dst, صحيح flags);
		حرف *سطر_ptr;
		FILE *dst_دھارا;
		صحيح منوال;

		إذا (is_prefixed_with(سطر, "begin-base64 ")) {
			سطر_ptr = سطر + 13;
			decode_fn_ptr = read_base64;
		} وإلا إذا (is_prefixed_with(سطر, "begin ")) {
			سطر_ptr = سطر + 6;
			decode_fn_ptr = read_stduu;
		} وإلا {
			حرر(سطر);
			استمر;
		}

		/* begin line found. decode and exit */
		منوال = bb_strtou(سطر_ptr, عدم, 8);
		إذا (outname == عدم) {
			outname = strchr(سطر_ptr, ' ');
			إذا (!outname)
				اكسر;
			outname++;
			trim(outname); /* remove trailing space (and '\r' for DOS text) */
			إذا (!outname[0])
				اكسر;
		}
		dst_دھارا = stdout;
		إذا (NOT_LONE_DASH(outname)
/* https://pubs.opengroup.org/onlinepubs/9699919799/utilities/uudecode.html
 * https://pubs.opengroup.org/onlinepubs/9699919799/utilities/uuencode.html
 * The above says that output file name specified in input file
 * or overridden by -o OUTFILE can be special "/dev/stdout" string.
 * This usually works "implicitly": many systems have /dev/stdout.
 * If ENABLE_DESKTOP, support that explicitly:
 */
		 && (!ENABLE_DESKTOP || strcmp(outname, "/dev/stdout") != 0)
		) {
			dst_دھارا = xfopen_for_write(outname);
			fchmod(fileno(dst_دھارا), منوال & (S_IRWXU | S_IRWXG | S_IRWXO));
		}
		حرر(سطر);
		decode_fn_ptr(src_دھارا, dst_دھارا, /*flags:*/ BASE64_FLAG_UU_STOP + BASE64_FLAG_NO_STOP_CHAR);
		/* fclose_if_not_stdin(src_stream); - redundant */
		أرجع EXIT_SUCCESS;
	}
	bb_simple_error_پیغام_and_die("no 'begin' line");
}
#جے_انت

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

#إذا ENABLE_BASE32 || ENABLE_BASE64

# إذا ENABLE_BASE32
ساكن فراغ bb_b32encode(حرف *p, ثابت فراغ *src, صحيح لمائی)
{
#تعریف_کرو tbl bb_uuenc_tbl_base32
	ثابت بنانشانی حرف *s = src;

	/* Transform 5x8 bits to 8x5 bits */
	طالما (لمائی > 0) {
		بنانشانی cur, اگلا;

		لمائی--;
		cur = *s++;
		*p++ = tbl[cur >> 3];			// xxxxx--- -------- -------- -------- --------
		cur &= 7;

		اگلا = 0;
		إذا (--لمائی >= 0)
			اگلا = *s++;
		*p++ = tbl[(cur << 2) + (اگلا >> 6)];	// -----xxx xx------ -------- -------- --------
		cur = اگلا & 0x3f;

		*p++ = tbl[cur >> 1];			// -------- --xxxxx- -------- -------- --------
		cur &= 1;

		اگلا = 0;
		إذا (--لمائی >= 0)
			اگلا = *s++;
		*p++ = tbl[(cur << 4) + (اگلا >> 4)];	// -------- -------x xxxx---- -------- --------
		cur = اگلا & 0xf;

		اگلا = 0;
		إذا (--لمائی >= 0)
			اگلا = *s++;
		*p++ = tbl[(cur << 1) + (اگلا >> 7)];	// -------- -------- ----xxxx x------- --------
		cur = اگلا & 0x7f;

		*p++ = tbl[cur >> 2];			// -------- -------- -------- -xxxxx-- --------
		cur &= 3;

		اگلا = 0;
		إذا (--لمائی >= 0)
			اگلا = *s++;
		*p++ = tbl[(cur << 3) + (اگلا >> 5)];	// -------- -------- -------- ------xx xxx-----
		cur = اگلا & 0x1f;

		*p++ = tbl[cur];			// -------- -------- -------- -------- ---xxxxx
	}
#تعریف_رد tbl
	/* Zero-terminate */
	*p = '\0';
	/* Pad as necessary */
	لمائی = ((-لمائی) * 3) >> 1; /* -4 => 6 pad chars, -3 => 4, -2 => 3, -1 => 1 */
	طالما (لمائی--) {
		*--p = '=';
	}
}
# وإلا
فراغ bb_b32encode(حرف *p, ثابت فراغ *src, صحيح لمائی); /* undefined */
# endif

صحيح ادھار_عدد_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح ادھار_عدد_main(صحيح argc بن_ورتیا_PARAM, حرف **argv)
{
	FILE *src_دھارا;
	بنانشانی opts;
	بنانشانی خانہ = 76;

	opts = getopt32(argv, "^" "diw:+" "\0" "?1"/* 1 arg max*/, &خانہ);
	argv += optind;

	إذا (!argv[0])
		*--argv = (حرف*)"-";
	src_دھارا = xfopen_stdin(argv[0]);
	إذا (opts & 1) {
		/* -d: decode */
		صحيح flags = (بنانشانی حرف)EOF;
		إذا (ENABLE_BASE32 && (!ENABLE_BASE64 || applet_name[4] == '3'))
			flags = ((بنانشانی حرف)EOF) | BASE64_32;
		read_base64(src_دھارا, stdout, flags);
	} وإلا {
		تعداد {
			SRC_BUF_ناپ = 3 * 5 * 32, /* this *MUST* be a multiple of 3 and 5 */
			DST_BUF_ناپ = 8 * ((SRC_BUF_ناپ + 4) / 5), /* max growth on encode (base32 case) */
		};
		/* Use one buffer for both input and output:
		 * encoding reads input "left-to-right",
		 * it's safe to place source at the end of the buffer and
		 * overwrite it while encoding, just be careful to have a gap.
		 */
		حرف dst_buf[((DST_BUF_ناپ + /*gap:*/ 16) /*round up to 16:*/ | 0xf) + 1];
#تعریف_کرو src_buf (dst_buf + حجم(dst_buf) - SRC_BUF_ناپ)
		صحيح src_fd, rem;

		src_fd = fileno(src_دھارا);
		rem = 0;
		طالما (1) {
			ناپ_t ناپ = بھریا_read(src_fd, src_buf, SRC_BUF_ناپ);
			إذا ((ssize_t)ناپ < 0)
				bb_simple_perror_پیغام_and_die(bb_پیغام_read_error);
			إذا (ناپ == 0) {
				إذا (rem != 0) bb_putchar('\n');
				اكسر;
			}

			/* Encode the buffer we just read in */
			إذا (ENABLE_BASE32 && (!ENABLE_BASE64 || applet_name[4] == '3')) {
				bb_b32encode(dst_buf, src_buf, ناپ);
				ناپ = 8 * ((ناپ + 4) / 5);
			} وإلا {
				bb_uuencode(dst_buf, src_buf, ناپ, bb_uuenc_tbl_base64);
				ناپ = 4 * ((ناپ + 2) / 3);
			}

			إذا (خانہ == 0) {
				fputs_stdout(dst_buf);
			} وإلا {
				حرف *نتیجہ = dst_buf;
				إذا (rem == 0)
					rem = خانہ;
				طالما (1) {
					صحيح باہرلا = ناپ < rem ? ناپ : rem;
					rem -= باہرلا;
					لکھو(rem != 0 ? "%.*s" : "%.*s\n", باہرلا, نتیجہ);
					إذا (rem != 0)
						اكسر;
					ناپ -= باہرلا;
					إذا (ناپ == 0)
						اكسر;
					نتیجہ += باہرلا;
					rem = خانہ;
				}
			}
		}
#تعریف_رد src_buf
	}

	fflush_stdout_and_exit_SUCCESS();
}
#جے_انت

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
پورا
mkdir unpk_u unpk_m 2>/dev/null
for f in u_*; do
    ./uudecode <$f -o unpk_u/${f:2}
    diff -a ${f:2} unpk_u/${f:2} >/dev/null 2>&1
    echo uudecode $f: $?
پورا
for f in m_*; do
    ./uudecode <$f -o unpk_m/${f:2}
    diff -a ${f:2} unpk_m/${f:2} >/dev/null 2>&1
    echo uudecode $f: $?
پورا
*/
