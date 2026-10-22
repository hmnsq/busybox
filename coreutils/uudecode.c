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

#అన్తర్భావ "libbb.h"

#ఒకవేళ ENABLE_UUDECODE
స్థిర శూన్య FAST_FUNC read_stduu(FILE *src_ప్రవాహం, FILE *dst_ప్రవాహం, పూర్ణాంకం flags వాడని_PARAM)
{
	అక్షరం *పంక్తి;

	కొరకు (;;) {
		పూర్ణాంకం encoded_len, str_len;
		అక్షరం *పంక్తి_ptr, *dst;
		కొలత_t పంక్తి_len;

		పంక్తి_len = 64 * 1024;
		పంక్తి = xmalloc_fgets_str_len(src_ప్రవాహం, "\n", &పంక్తి_len);
		ఒకవేళ (!పంక్తి)
			ఆపు;
		/* Handle both Unix and MSDOS text.
		 * Note: space should not be trimmed, some encoders use it instead of "`"
		 * for padding of last incomplete 4-char block.
		 */
		str_len = పంక్తి_len;
		అయితే (--str_len >= 0
		 && (పంక్తి[str_len] == '\n' || పంక్తి[str_len] == '\r')
		) {
			పంక్తి[str_len] = '\0';
		}

		ఒకవేళ (strcmp(పంక్తి, "end") == 0) {
			తిరిగి; /* the only non-error exit */
		}

		పంక్తి_ptr = పంక్తి;
		అయితే (*పంక్తి_ptr) {
			*పంక్తి_ptr = (*పంక్తి_ptr - 0x20) & 0x3f;
			పంక్తి_ptr++;
		}
		str_len = పంక్తి_ptr - పంక్తి;

		encoded_len = పంక్తి[0] * 4 / 3;
		/* Check that line is not too short. (we tolerate
		 * overly _long_ line to accommodate possible extra "`").
		 * Empty line case is also caught here. */
		ఒకవేళ (str_len <= encoded_len) {
			ఆపు; /* go to bb_error_msg_and_die("short file"); */
		}
		ఒకవేళ (encoded_len <= 0) {
			/* Ignore the "`\n" line, why is it even in the encode file ? */
			విడుదల(పంక్తి);
			కొనసాగు;
		}
		ఒకవేళ (encoded_len > 60) {
			bb_simple_error_సందేశం_and_die("line too long");
		}

		dst = పంక్తి;
		పంక్తి_ptr = పంక్తి + 1;
		చేయి {
			/* Merge four 6 bit chars to three 8 bit chars */
			*dst++ = పంక్తి_ptr[0] << 2 | పంక్తి_ptr[1] >> 4;
			encoded_len--;
			ఒకవేళ (encoded_len == 0) {
				ఆపు;
			}

			*dst++ = పంక్తి_ptr[1] << 4 | పంక్తి_ptr[2] >> 2;
			encoded_len--;
			ఒకవేళ (encoded_len == 0) {
				ఆపు;
			}

			*dst++ = పంక్తి_ptr[2] << 6 | పంక్తి_ptr[3];
			పంక్తి_ptr += 4;
			encoded_len -= 2;
		} అయితే (encoded_len > 0);
		fwrite(పంక్తి, 1, dst - పంక్తి, dst_ప్రవాహం);
		విడుదల(పంక్తి);
	}
	bb_simple_error_సందేశం_and_die("short file");
}

పూర్ణాంకం uudecode_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం uudecode_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	FILE *src_ప్రవాహం;
	అక్షరం *outname = ఖాళీ;
	అక్షరం *పంక్తి;

	getopt32(argv, "^" "o:" "\0" "?1"/* 1 arg max*/, &outname);
	argv += optind;

	ఒకవేళ (!argv[0])
		*--argv = (అక్షరం*)"-";
	src_ప్రవాహం = xfopen_stdin(argv[0]);

	/* Search for the start of the encoding */
	అయితే ((పంక్తి = xmalloc_fgetline(src_ప్రవాహం)) != ఖాళీ) {
		శూన్య FAST_FUNC (*decode_fn_ptr)(FILE *src, FILE *dst, పూర్ణాంకం flags);
		అక్షరం *పంక్తి_ptr;
		FILE *dst_ప్రవాహం;
		పూర్ణాంకం బహుళకం;

		ఒకవేళ (is_prefixed_with(పంక్తి, "begin-base64 ")) {
			పంక్తి_ptr = పంక్తి + 13;
			decode_fn_ptr = read_base64;
		} లేకపోతే ఒకవేళ (is_prefixed_with(పంక్తి, "begin ")) {
			పంక్తి_ptr = పంక్తి + 6;
			decode_fn_ptr = read_stduu;
		} లేకపోతే {
			విడుదల(పంక్తి);
			కొనసాగు;
		}

		/* begin line found. decode and exit */
		బహుళకం = bb_strtou(పంక్తి_ptr, ఖాళీ, 8);
		ఒకవేళ (outname == ఖాళీ) {
			outname = strchr(పంక్తి_ptr, ' ');
			ఒకవేళ (!outname)
				ఆపు;
			outname++;
			trim(outname); /* remove trailing space (and '\r' for DOS text) */
			ఒకవేళ (!outname[0])
				ఆపు;
		}
		dst_ప్రవాహం = stdout;
		ఒకవేళ (NOT_LONE_DASH(outname)
/* https://pubs.opengroup.org/onlinepubs/9699919799/utilities/uudecode.html
 * https://pubs.opengroup.org/onlinepubs/9699919799/utilities/uuencode.html
 * The above says that output file name specified in input file
 * or overridden by -o OUTFILE can be special "/dev/stdout" string.
 * This usually works "implicitly": many systems have /dev/stdout.
 * If ENABLE_DESKTOP, support that explicitly:
 */
		 && (!ENABLE_DESKTOP || strcmp(outname, "/dev/stdout") != 0)
		) {
			dst_ప్రవాహం = xfopen_for_write(outname);
			fchmod(fileno(dst_ప్రవాహం), బహుళకం & (S_IRWXU | S_IRWXG | S_IRWXO));
		}
		విడుదల(పంక్తి);
		decode_fn_ptr(src_ప్రవాహం, dst_ప్రవాహం, /*flags:*/ BASE64_FLAG_UU_STOP + BASE64_FLAG_NO_STOP_CHAR);
		/* fclose_if_not_stdin(src_stream); - redundant */
		తిరిగి EXIT_SUCCESS;
	}
	bb_simple_error_సందేశం_and_die("no 'begin' line");
}
#అయితే_ముగింపు

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

#ఒకవేళ ENABLE_BASE32 || ENABLE_BASE64

# ఒకవేళ ENABLE_BASE32
స్థిర శూన్య bb_b32encode(అక్షరం *p, స్థిరరాశి శూన్య *src, పూర్ణాంకం నిడివి)
{
#నిర్వచించు tbl bb_uuenc_tbl_base32
	స్థిరరాశి గుర్తులేని అక్షరం *s = src;

	/* Transform 5x8 bits to 8x5 bits */
	అయితే (నిడివి > 0) {
		గుర్తులేని cur, తదుపరి;

		నిడివి--;
		cur = *s++;
		*p++ = tbl[cur >> 3];			// xxxxx--- -------- -------- -------- --------
		cur &= 7;

		తదుపరి = 0;
		ఒకవేళ (--నిడివి >= 0)
			తదుపరి = *s++;
		*p++ = tbl[(cur << 2) + (తదుపరి >> 6)];	// -----xxx xx------ -------- -------- --------
		cur = తదుపరి & 0x3f;

		*p++ = tbl[cur >> 1];			// -------- --xxxxx- -------- -------- --------
		cur &= 1;

		తదుపరి = 0;
		ఒకవేళ (--నిడివి >= 0)
			తదుపరి = *s++;
		*p++ = tbl[(cur << 4) + (తదుపరి >> 4)];	// -------- -------x xxxx---- -------- --------
		cur = తదుపరి & 0xf;

		తదుపరి = 0;
		ఒకవేళ (--నిడివి >= 0)
			తదుపరి = *s++;
		*p++ = tbl[(cur << 1) + (తదుపరి >> 7)];	// -------- -------- ----xxxx x------- --------
		cur = తదుపరి & 0x7f;

		*p++ = tbl[cur >> 2];			// -------- -------- -------- -xxxxx-- --------
		cur &= 3;

		తదుపరి = 0;
		ఒకవేళ (--నిడివి >= 0)
			తదుపరి = *s++;
		*p++ = tbl[(cur << 3) + (తదుపరి >> 5)];	// -------- -------- -------- ------xx xxx-----
		cur = తదుపరి & 0x1f;

		*p++ = tbl[cur];			// -------- -------- -------- -------- ---xxxxx
	}
#నిర్వచనం_రద్దు tbl
	/* Zero-terminate */
	*p = '\0';
	/* Pad as necessary */
	నిడివి = ((-నిడివి) * 3) >> 1; /* -4 => 6 pad chars, -3 => 4, -2 => 3, -1 => 1 */
	అయితే (నిడివి--) {
		*--p = '=';
	}
}
# లేకపోతే
శూన్య bb_b32encode(అక్షరం *p, స్థిరరాశి శూన్య *src, పూర్ణాంకం నిడివి); /* undefined */
# endif

పూర్ణాంకం పునాది_అంకె_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం పునాది_అంకె_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	FILE *src_ప్రవాహం;
	గుర్తులేని opts;
	గుర్తులేని నిలువు = 76;

	opts = getopt32(argv, "^" "diw:+" "\0" "?1"/* 1 arg max*/, &నిలువు);
	argv += optind;

	ఒకవేళ (!argv[0])
		*--argv = (అక్షరం*)"-";
	src_ప్రవాహం = xfopen_stdin(argv[0]);
	ఒకవేళ (opts & 1) {
		/* -d: decode */
		పూర్ణాంకం flags = (గుర్తులేని అక్షరం)EOF;
		ఒకవేళ (ENABLE_BASE32 && (!ENABLE_BASE64 || applet_name[4] == '3'))
			flags = ((గుర్తులేని అక్షరం)EOF) | BASE64_32;
		read_base64(src_ప్రవాహం, stdout, flags);
	} లేకపోతే {
		గణన {
			SRC_BUF_కొలత = 3 * 5 * 32, /* this *MUST* be a multiple of 3 and 5 */
			DST_BUF_కొలత = 8 * ((SRC_BUF_కొలత + 4) / 5), /* max growth on encode (base32 case) */
		};
		/* Use one buffer for both input and output:
		 * encoding reads input "left-to-right",
		 * it's safe to place source at the end of the buffer and
		 * overwrite it while encoding, just be careful to have a gap.
		 */
		అక్షరం dst_buf[((DST_BUF_కొలత + /*gap:*/ 16) /*round up to 16:*/ | 0xf) + 1];
#నిర్వచించు src_buf (dst_buf + పరిమాణం(dst_buf) - SRC_BUF_కొలత)
		పూర్ణాంకం src_fd, rem;

		src_fd = fileno(src_ప్రవాహం);
		rem = 0;
		అయితే (1) {
			కొలత_t కొలత = నిండిన_read(src_fd, src_buf, SRC_BUF_కొలత);
			ఒకవేళ ((ssize_t)కొలత < 0)
				bb_simple_perror_సందేశం_and_die(bb_సందేశం_read_error);
			ఒకవేళ (కొలత == 0) {
				ఒకవేళ (rem != 0) bb_putchar('\n');
				ఆపు;
			}

			/* Encode the buffer we just read in */
			ఒకవేళ (ENABLE_BASE32 && (!ENABLE_BASE64 || applet_name[4] == '3')) {
				bb_b32encode(dst_buf, src_buf, కొలత);
				కొలత = 8 * ((కొలత + 4) / 5);
			} లేకపోతే {
				bb_uuencode(dst_buf, src_buf, కొలత, bb_uuenc_tbl_base64);
				కొలత = 4 * ((కొలత + 2) / 3);
			}

			ఒకవేళ (నిలువు == 0) {
				fputs_stdout(dst_buf);
			} లేకపోతే {
				అక్షరం *ఫలితం = dst_buf;
				ఒకవేళ (rem == 0)
					rem = నిలువు;
				అయితే (1) {
					పూర్ణాంకం బహిర్గతం = కొలత < rem ? కొలత : rem;
					rem -= బహిర్గతం;
					రాయి(rem != 0 ? "%.*s" : "%.*s\n", బహిర్గతం, ఫలితం);
					ఒకవేళ (rem != 0)
						ఆపు;
					కొలత -= బహిర్గతం;
					ఒకవేళ (కొలత == 0)
						ఆపు;
					ఫలితం += బహిర్గతం;
					rem = నిలువు;
				}
			}
		}
#నిర్వచనం_రద్దు src_buf
	}

	fflush_stdout_and_exit_SUCCESS();
}
#అయితే_ముగింపు

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
పూర్తి
mkdir unpk_u unpk_m 2>/dev/null
for f in u_*; do
    ./uudecode <$f -o unpk_u/${f:2}
    diff -a ${f:2} unpk_u/${f:2} >/dev/null 2>&1
    echo uudecode $f: $?
పూర్తి
for f in m_*; do
    ./uudecode <$f -o unpk_m/${f:2}
    diff -a ${f:2} unpk_m/${f:2} >/dev/null 2>&1
    echo uudecode $f: $?
పూర్తి
*/
