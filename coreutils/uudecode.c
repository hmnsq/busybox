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

#ಯದಿ ENABLE_UUDECODE
ಸ್ಥಿರ ಶೂನ್ಯ FAST_FUNC read_stduu(FILE *src_stream, FILE *dst_stream, ಪೂರ್ಣಾಙ್ಕ flags UNUSED_PARAM)
{
	ವರ್ಣ *ಸಾಲು;

	ಪ್ರತಿ (;;) {
		ಪೂರ್ಣಾಙ್ಕ encoded_len, str_len;
		ವರ್ಣ *ಸಾಲು_ptr, *dst;
		size_t ಸಾಲು_len;

		ಸಾಲು_len = 64 * 1024;
		ಸಾಲು = xmalloc_fgets_str_len(src_stream, "\n", &ಸಾಲು_len);
		ಯದಿ (!ಸಾಲು)
			ಭಙ್ಗ;
		/* Handle both Unix and MSDOS text.
		 * Note: space should not be trimmed, some encoders use it instead of "`"
		 * for padding of last incomplete 4-char block.
		 */
		str_len = ಸಾಲು_len;
		ಯಾವತ್ (--str_len >= 0
		 && (ಸಾಲು[str_len] == '\n' || ಸಾಲು[str_len] == '\r')
		) {
			ಸಾಲು[str_len] = '\0';
		}

		ಯದಿ (strcmp(ಸಾಲು, "end") == 0) {
			ಪ್ರತ್ಯಾಗಮ; /* the only non-error exit */
		}

		ಸಾಲು_ptr = ಸಾಲು;
		ಯಾವತ್ (*ಸಾಲು_ptr) {
			*ಸಾಲು_ptr = (*ಸಾಲು_ptr - 0x20) & 0x3f;
			ಸಾಲು_ptr++;
		}
		str_len = ಸಾಲು_ptr - ಸಾಲು;

		encoded_len = ಸಾಲು[0] * 4 / 3;
		/* Check that line is not too short. (we tolerate
		 * overly _long_ line to accommodate possible extra "`").
		 * Empty line case is also caught here. */
		ಯದಿ (str_len <= encoded_len) {
			ಭಙ್ಗ; /* go to bb_error_msg_and_die("short file"); */
		}
		ಯದಿ (encoded_len <= 0) {
			/* Ignore the "`\n" line, why is it even in the encode file ? */
			ಮುಕ್ತ(ಸಾಲು);
			ಅನುವರ್ತ;
		}
		ಯದಿ (encoded_len > 60) {
			bb_simple_error_ಸಂದೇಶ_and_die("line too long");
		}

		dst = ಸಾಲು;
		ಸಾಲು_ptr = ಸಾಲು + 1;
		ಕುರು {
			/* Merge four 6 bit chars to three 8 bit chars */
			*dst++ = ಸಾಲು_ptr[0] << 2 | ಸಾಲು_ptr[1] >> 4;
			encoded_len--;
			ಯದಿ (encoded_len == 0) {
				ಭಙ್ಗ;
			}

			*dst++ = ಸಾಲು_ptr[1] << 4 | ಸಾಲು_ptr[2] >> 2;
			encoded_len--;
			ಯದಿ (encoded_len == 0) {
				ಭಙ್ಗ;
			}

			*dst++ = ಸಾಲು_ptr[2] << 6 | ಸಾಲು_ptr[3];
			ಸಾಲು_ptr += 4;
			encoded_len -= 2;
		} ಯಾವತ್ (encoded_len > 0);
		fwrite(ಸಾಲು, 1, dst - ಸಾಲು, dst_stream);
		ಮುಕ್ತ(ಸಾಲು);
	}
	bb_simple_error_ಸಂದೇಶ_and_die("short file");
}

ಪೂರ್ಣಾಙ್ಕ uudecode_main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಙ್ಕ uudecode_main(ಪೂರ್ಣಾಙ್ಕ argc UNUSED_PARAM, ವರ್ಣ **argv)
{
	FILE *src_stream;
	ವರ್ಣ *outname = ರಿಕ್ತ;
	ವರ್ಣ *ಸಾಲು;

	getopt32(argv, "^" "o:" "\0" "?1"/* 1 arg max*/, &outname);
	argv += optind;

	ಯದಿ (!argv[0])
		*--argv = (ವರ್ಣ*)"-";
	src_stream = xfopen_stdin(argv[0]);

	/* Search for the start of the encoding */
	ಯಾವತ್ ((ಸಾಲು = xmalloc_fgetline(src_stream)) != ರಿಕ್ತ) {
		ಶೂನ್ಯ FAST_FUNC (*decode_fn_ptr)(FILE *src, FILE *dst, ಪೂರ್ಣಾಙ್ಕ flags);
		ವರ್ಣ *ಸಾಲು_ptr;
		FILE *dst_stream;
		ಪೂರ್ಣಾಙ್ಕ mode;

		ಯದಿ (is_prefixed_with(ಸಾಲು, "begin-base64 ")) {
			ಸಾಲು_ptr = ಸಾಲು + 13;
			decode_fn_ptr = read_base64;
		} ಅನ್ಯಥಾ ಯದಿ (is_prefixed_with(ಸಾಲು, "begin ")) {
			ಸಾಲು_ptr = ಸಾಲು + 6;
			decode_fn_ptr = read_stduu;
		} ಅನ್ಯಥಾ {
			ಮುಕ್ತ(ಸಾಲು);
			ಅನುವರ್ತ;
		}

		/* begin line found. decode and exit */
		mode = bb_strtou(ಸಾಲು_ptr, ರಿಕ್ತ, 8);
		ಯದಿ (outname == ರಿಕ್ತ) {
			outname = strchr(ಸಾಲು_ptr, ' ');
			ಯದಿ (!outname)
				ಭಙ್ಗ;
			outname++;
			trim(outname); /* remove trailing space (and '\r' for DOS text) */
			ಯದಿ (!outname[0])
				ಭಙ್ಗ;
		}
		dst_stream = stdout;
		ಯದಿ (NOT_LONE_DASH(outname)
/* https://pubs.opengroup.org/onlinepubs/9699919799/utilities/uudecode.html
 * https://pubs.opengroup.org/onlinepubs/9699919799/utilities/uuencode.html
 * The above says that output file name specified in input file
 * or overridden by -o OUTFILE can be special "/dev/stdout" string.
 * This usually works "implicitly": many systems have /dev/stdout.
 * If ENABLE_DESKTOP, support that explicitly:
 */
		 && (!ENABLE_DESKTOP || strcmp(outname, "/dev/stdout") != 0)
		) {
			dst_stream = xfopen_for_write(outname);
			fchmod(fileno(dst_stream), mode & (S_IRWXU | S_IRWXG | S_IRWXO));
		}
		ಮುಕ್ತ(ಸಾಲು);
		decode_fn_ptr(src_stream, dst_stream, /*flags:*/ BASE64_FLAG_UU_STOP + BASE64_FLAG_NO_STOP_CHAR);
		/* fclose_if_not_stdin(src_stream); - redundant */
		ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
	}
	bb_simple_error_ಸಂದೇಶ_and_die("no 'begin' line");
}
#endif

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

#ಯದಿ ENABLE_BASE32 || ENABLE_BASE64

# ಯದಿ ENABLE_BASE32
ಸ್ಥಿರ ಶೂನ್ಯ bb_b32encode(ವರ್ಣ *p, ಧ್ರುವ ಶೂನ್ಯ *src, ಪೂರ್ಣಾಙ್ಕ length)
{
#define tbl bb_uuenc_tbl_base32
	ಧ್ರುವ unsigned ವರ್ಣ *s = src;

	/* Transform 5x8 bits to 8x5 bits */
	ಯಾವತ್ (length > 0) {
		unsigned cur, next;

		length--;
		cur = *s++;
		*p++ = tbl[cur >> 3];			// xxxxx--- -------- -------- -------- --------
		cur &= 7;

		next = 0;
		ಯದಿ (--length >= 0)
			next = *s++;
		*p++ = tbl[(cur << 2) + (next >> 6)];	// -----xxx xx------ -------- -------- --------
		cur = next & 0x3f;

		*p++ = tbl[cur >> 1];			// -------- --xxxxx- -------- -------- --------
		cur &= 1;

		next = 0;
		ಯದಿ (--length >= 0)
			next = *s++;
		*p++ = tbl[(cur << 4) + (next >> 4)];	// -------- -------x xxxx---- -------- --------
		cur = next & 0xf;

		next = 0;
		ಯದಿ (--length >= 0)
			next = *s++;
		*p++ = tbl[(cur << 1) + (next >> 7)];	// -------- -------- ----xxxx x------- --------
		cur = next & 0x7f;

		*p++ = tbl[cur >> 2];			// -------- -------- -------- -xxxxx-- --------
		cur &= 3;

		next = 0;
		ಯದಿ (--length >= 0)
			next = *s++;
		*p++ = tbl[(cur << 3) + (next >> 5)];	// -------- -------- -------- ------xx xxx-----
		cur = next & 0x1f;

		*p++ = tbl[cur];			// -------- -------- -------- -------- ---xxxxx
	}
#undef tbl
	/* Zero-terminate */
	*p = '\0';
	/* Pad as necessary */
	length = ((-length) * 3) >> 1; /* -4 => 6 pad chars, -3 => 4, -2 => 3, -1 => 1 */
	ಯಾವತ್ (length--) {
		*--p = '=';
	}
}
# ಅನ್ಯಥಾ
ಶೂನ್ಯ bb_b32encode(ವರ್ಣ *p, ಧ್ರುವ ಶೂನ್ಯ *src, ಪೂರ್ಣಾಙ್ಕ length); /* undefined */
# endif

ಪೂರ್ಣಾಙ್ಕ base_ಅಂಕೆ_main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಙ್ಕ base_ಅಂಕೆ_main(ಪೂರ್ಣಾಙ್ಕ argc UNUSED_PARAM, ವರ್ಣ **argv)
{
	FILE *src_stream;
	unsigned opts;
	unsigned col = 76;

	opts = getopt32(argv, "^" "diw:+" "\0" "?1"/* 1 arg max*/, &col);
	argv += optind;

	ಯದಿ (!argv[0])
		*--argv = (ವರ್ಣ*)"-";
	src_stream = xfopen_stdin(argv[0]);
	ಯದಿ (opts & 1) {
		/* -d: decode */
		ಪೂರ್ಣಾಙ್ಕ flags = (unsigned ವರ್ಣ)EOF;
		ಯದಿ (ENABLE_BASE32 && (!ENABLE_BASE64 || applet_name[4] == '3'))
			flags = ((unsigned ವರ್ಣ)EOF) | BASE64_32;
		read_base64(src_stream, stdout, flags);
	} ಅನ್ಯಥಾ {
		ಗಣನಾ {
			SRC_BUF_SIZE = 3 * 5 * 32, /* this *MUST* be a multiple of 3 and 5 */
			DST_BUF_SIZE = 8 * ((SRC_BUF_SIZE + 4) / 5), /* max growth on encode (base32 case) */
		};
		/* Use one buffer for both input and output:
		 * encoding reads input "left-to-right",
		 * it's safe to place source at the end of the buffer and
		 * overwrite it while encoding, just be careful to have a gap.
		 */
		ವರ್ಣ dst_buf[((DST_BUF_SIZE + /*gap:*/ 16) /*round up to 16:*/ | 0xf) + 1];
#define src_buf (dst_buf + ಪರಿಮಾಣ(dst_buf) - SRC_BUF_SIZE)
		ಪೂರ್ಣಾಙ್ಕ src_fd, rem;

		src_fd = fileno(src_stream);
		rem = 0;
		ಯಾವತ್ (1) {
			size_t size = full_read(src_fd, src_buf, SRC_BUF_SIZE);
			ಯದಿ ((ssize_t)size < 0)
				bb_simple_perror_ಸಂದೇಶ_and_die(bb_ಸಂದೇಶ_read_error);
			ಯದಿ (size == 0) {
				ಯದಿ (rem != 0) bb_putchar('\n');
				ಭಙ್ಗ;
			}

			/* Encode the buffer we just read in */
			ಯದಿ (ENABLE_BASE32 && (!ENABLE_BASE64 || applet_name[4] == '3')) {
				bb_b32encode(dst_buf, src_buf, size);
				size = 8 * ((size + 4) / 5);
			} ಅನ್ಯಥಾ {
				bb_uuencode(dst_buf, src_buf, size, bb_uuenc_tbl_base64);
				size = 4 * ((size + 2) / 3);
			}

			ಯದಿ (col == 0) {
				fputs_stdout(dst_buf);
			} ಅನ್ಯಥಾ {
				ವರ್ಣ *result = dst_buf;
				ಯದಿ (rem == 0)
					rem = col;
				ಯಾವತ್ (1) {
					ಪೂರ್ಣಾಙ್ಕ out = size < rem ? size : rem;
					rem -= out;
					printf(rem != 0 ? "%.*s" : "%.*s\n", out, result);
					ಯದಿ (rem != 0)
						ಭಙ್ಗ;
					size -= out;
					ಯದಿ (size == 0)
						ಭಙ್ಗ;
					result += out;
					rem = col;
				}
			}
		}
#undef src_buf
	}

	fflush_stdout_and_exit_SUCCESS();
}
#endif

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
done
mkdir unpk_u unpk_m 2>/dev/null
for f in u_*; do
    ./uudecode <$f -o unpk_u/${f:2}
    diff -a ${f:2} unpk_u/${f:2} >/dev/null 2>&1
    echo uudecode $f: $?
done
for f in m_*; do
    ./uudecode <$f -o unpk_m/${f:2}
    diff -a ${f:2} unpk_m/${f:2} >/dev/null 2>&1
    echo uudecode $f: $?
done
*/
