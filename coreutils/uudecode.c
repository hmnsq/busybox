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
ساكن فراغ FAST_FUNC read_stduu(FILE *src_وهڪرو, FILE *dst_وهڪرو, صحيح flags اڻ_استعمال_ٿيل_PARAM)
{
	حرف *سٽ;

	لكل (;;) {
		صحيح encoded_len, str_len;
		حرف *سٽ_ptr, *dst;
		سائيز_t سٽ_len;

		سٽ_len = 64 * 1024;
		سٽ = xmalloc_fgets_str_len(src_وهڪرو, "\n", &سٽ_len);
		إذا (!سٽ)
			اكسر;
		/* Handle both Unix and MSDOS text.
		 * Note: space should not be trimmed, some encoders use it instead of "`"
		 * for padding of last incomplete 4-char block.
		 */
		str_len = سٽ_len;
		طالما (--str_len >= 0
		 && (سٽ[str_len] == '\n' || سٽ[str_len] == '\r')
		) {
			سٽ[str_len] = '\0';
		}

		إذا (strcmp(سٽ, "end") == 0) {
			أرجع; /* the only non-error exit */
		}

		سٽ_ptr = سٽ;
		طالما (*سٽ_ptr) {
			*سٽ_ptr = (*سٽ_ptr - 0x20) & 0x3f;
			سٽ_ptr++;
		}
		str_len = سٽ_ptr - سٽ;

		encoded_len = سٽ[0] * 4 / 3;
		/* Check that line is not too short. (we tolerate
		 * overly _long_ line to accommodate possible extra "`").
		 * Empty line case is also caught here. */
		إذا (str_len <= encoded_len) {
			اكسر; /* go to bb_error_msg_and_die("short file"); */
		}
		إذا (encoded_len <= 0) {
			/* Ignore the "`\n" line, why is it even in the encode file ? */
			حرر(سٽ);
			استمر;
		}
		إذا (encoded_len > 60) {
			bb_simple_error_نياپو_and_die("line too long");
		}

		dst = سٽ;
		سٽ_ptr = سٽ + 1;
		افعل {
			/* Merge four 6 bit chars to three 8 bit chars */
			*dst++ = سٽ_ptr[0] << 2 | سٽ_ptr[1] >> 4;
			encoded_len--;
			إذا (encoded_len == 0) {
				اكسر;
			}

			*dst++ = سٽ_ptr[1] << 4 | سٽ_ptr[2] >> 2;
			encoded_len--;
			إذا (encoded_len == 0) {
				اكسر;
			}

			*dst++ = سٽ_ptr[2] << 6 | سٽ_ptr[3];
			سٽ_ptr += 4;
			encoded_len -= 2;
		} طالما (encoded_len > 0);
		fwrite(سٽ, 1, dst - سٽ, dst_وهڪرو);
		حرر(سٽ);
	}
	bb_simple_error_نياپو_and_die("short file");
}

صحيح uudecode_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح uudecode_main(صحيح argc اڻ_استعمال_ٿيل_PARAM, حرف **argv)
{
	FILE *src_وهڪرو;
	حرف *outname = عدم;
	حرف *سٽ;

	getopt32(argv, "^" "o:" "\0" "?1"/* 1 arg max*/, &outname);
	argv += optind;

	إذا (!argv[0])
		*--argv = (حرف*)"-";
	src_وهڪرو = xfopen_stdin(argv[0]);

	/* Search for the start of the encoding */
	طالما ((سٽ = xmalloc_fgetline(src_وهڪرو)) != عدم) {
		فراغ FAST_FUNC (*decode_fn_ptr)(FILE *src, FILE *dst, صحيح flags);
		حرف *سٽ_ptr;
		FILE *dst_وهڪرو;
		صحيح گهڻائي;

		إذا (is_prefixed_with(سٽ, "begin-base64 ")) {
			سٽ_ptr = سٽ + 13;
			decode_fn_ptr = read_base64;
		} وإلا إذا (is_prefixed_with(سٽ, "begin ")) {
			سٽ_ptr = سٽ + 6;
			decode_fn_ptr = read_stduu;
		} وإلا {
			حرر(سٽ);
			استمر;
		}

		/* begin line found. decode and exit */
		گهڻائي = bb_strtou(سٽ_ptr, عدم, 8);
		إذا (outname == عدم) {
			outname = strchr(سٽ_ptr, ' ');
			إذا (!outname)
				اكسر;
			outname++;
			trim(outname); /* remove trailing space (and '\r' for DOS text) */
			إذا (!outname[0])
				اكسر;
		}
		dst_وهڪرو = stdout;
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
			dst_وهڪرو = xfopen_for_write(outname);
			fchmod(fileno(dst_وهڪرو), گهڻائي & (S_IRWXU | S_IRWXG | S_IRWXO));
		}
		حرر(سٽ);
		decode_fn_ptr(src_وهڪرو, dst_وهڪرو, /*flags:*/ BASE64_FLAG_UU_STOP + BASE64_FLAG_NO_STOP_CHAR);
		/* fclose_if_not_stdin(src_stream); - redundant */
		أرجع EXIT_SUCCESS;
	}
	bb_simple_error_نياپو_and_die("no 'begin' line");
}
#جيڪڏهن_پورو

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
ساكن فراغ bb_b32encode(حرف *p, ثابت فراغ *src, صحيح ڊيگهائي)
{
#وصف_ڪريو tbl bb_uuenc_tbl_base32
	ثابت بنانشان حرف *s = src;

	/* Transform 5x8 bits to 8x5 bits */
	طالما (ڊيگهائي > 0) {
		بنانشان cur, ايندڙ;

		ڊيگهائي--;
		cur = *s++;
		*p++ = tbl[cur >> 3];			// xxxxx--- -------- -------- -------- --------
		cur &= 7;

		ايندڙ = 0;
		إذا (--ڊيگهائي >= 0)
			ايندڙ = *s++;
		*p++ = tbl[(cur << 2) + (ايندڙ >> 6)];	// -----xxx xx------ -------- -------- --------
		cur = ايندڙ & 0x3f;

		*p++ = tbl[cur >> 1];			// -------- --xxxxx- -------- -------- --------
		cur &= 1;

		ايندڙ = 0;
		إذا (--ڊيگهائي >= 0)
			ايندڙ = *s++;
		*p++ = tbl[(cur << 4) + (ايندڙ >> 4)];	// -------- -------x xxxx---- -------- --------
		cur = ايندڙ & 0xf;

		ايندڙ = 0;
		إذا (--ڊيگهائي >= 0)
			ايندڙ = *s++;
		*p++ = tbl[(cur << 1) + (ايندڙ >> 7)];	// -------- -------- ----xxxx x------- --------
		cur = ايندڙ & 0x7f;

		*p++ = tbl[cur >> 2];			// -------- -------- -------- -xxxxx-- --------
		cur &= 3;

		ايندڙ = 0;
		إذا (--ڊيگهائي >= 0)
			ايندڙ = *s++;
		*p++ = tbl[(cur << 3) + (ايندڙ >> 5)];	// -------- -------- -------- ------xx xxx-----
		cur = ايندڙ & 0x1f;

		*p++ = tbl[cur];			// -------- -------- -------- -------- ---xxxxx
	}
#وصف_رد tbl
	/* Zero-terminate */
	*p = '\0';
	/* Pad as necessary */
	ڊيگهائي = ((-ڊيگهائي) * 3) >> 1; /* -4 => 6 pad chars, -3 => 4, -2 => 3, -1 => 1 */
	طالما (ڊيگهائي--) {
		*--p = '=';
	}
}
# وإلا
فراغ bb_b32encode(حرف *p, ثابت فراغ *src, صحيح ڊيگهائي); /* undefined */
# endif

صحيح بنياد_انگ_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح بنياد_انگ_main(صحيح argc اڻ_استعمال_ٿيل_PARAM, حرف **argv)
{
	FILE *src_وهڪرو;
	بنانشان opts;
	بنانشان کانو = 76;

	opts = getopt32(argv, "^" "diw:+" "\0" "?1"/* 1 arg max*/, &کانو);
	argv += optind;

	إذا (!argv[0])
		*--argv = (حرف*)"-";
	src_وهڪرو = xfopen_stdin(argv[0]);
	إذا (opts & 1) {
		/* -d: decode */
		صحيح flags = (بنانشان حرف)EOF;
		إذا (ENABLE_BASE32 && (!ENABLE_BASE64 || applet_name[4] == '3'))
			flags = ((بنانشان حرف)EOF) | BASE64_32;
		read_base64(src_وهڪرو, stdout, flags);
	} وإلا {
		تعداد {
			SRC_BUF_سائيز = 3 * 5 * 32, /* this *MUST* be a multiple of 3 and 5 */
			DST_BUF_سائيز = 8 * ((SRC_BUF_سائيز + 4) / 5), /* max growth on encode (base32 case) */
		};
		/* Use one buffer for both input and output:
		 * encoding reads input "left-to-right",
		 * it's safe to place source at the end of the buffer and
		 * overwrite it while encoding, just be careful to have a gap.
		 */
		حرف dst_buf[((DST_BUF_سائيز + /*gap:*/ 16) /*round up to 16:*/ | 0xf) + 1];
#وصف_ڪريو src_buf (dst_buf + حجم(dst_buf) - SRC_BUF_سائيز)
		صحيح src_fd, rem;

		src_fd = fileno(src_وهڪرو);
		rem = 0;
		طالما (1) {
			سائيز_t سائيز = ڀريل_read(src_fd, src_buf, SRC_BUF_سائيز);
			إذا ((ssize_t)سائيز < 0)
				bb_simple_perror_نياپو_and_die(bb_نياپو_read_error);
			إذا (سائيز == 0) {
				إذا (rem != 0) bb_putchar('\n');
				اكسر;
			}

			/* Encode the buffer we just read in */
			إذا (ENABLE_BASE32 && (!ENABLE_BASE64 || applet_name[4] == '3')) {
				bb_b32encode(dst_buf, src_buf, سائيز);
				سائيز = 8 * ((سائيز + 4) / 5);
			} وإلا {
				bb_uuencode(dst_buf, src_buf, سائيز, bb_uuenc_tbl_base64);
				سائيز = 4 * ((سائيز + 2) / 3);
			}

			إذا (کانو == 0) {
				fputs_stdout(dst_buf);
			} وإلا {
				حرف *نتيجو = dst_buf;
				إذا (rem == 0)
					rem = کانو;
				طالما (1) {
					صحيح ٻاهريون = سائيز < rem ? سائيز : rem;
					rem -= ٻاهريون;
					لکو(rem != 0 ? "%.*s" : "%.*s\n", ٻاهريون, نتيجو);
					إذا (rem != 0)
						اكسر;
					سائيز -= ٻاهريون;
					إذا (سائيز == 0)
						اكسر;
					نتيجو += ٻاهريون;
					rem = کانو;
				}
			}
		}
#وصف_رد src_buf
	}

	fflush_stdout_and_exit_SUCCESS();
}
#جيڪڏهن_پورو

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
پورو
mkdir unpk_u unpk_m 2>/dev/null
for f in u_*; do
    ./uudecode <$f -o unpk_u/${f:2}
    diff -a ${f:2} unpk_u/${f:2} >/dev/null 2>&1
    echo uudecode $f: $?
پورو
for f in m_*; do
    ./uudecode <$f -o unpk_m/${f:2}
    diff -a ${f:2} unpk_m/${f:2} >/dev/null 2>&1
    echo uudecode $f: $?
پورو
*/
