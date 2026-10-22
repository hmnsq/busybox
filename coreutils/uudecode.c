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

#அந்தர்பாவ "libbb.h"

#எனில் ENABLE_UUDECODE
நிலையான வெறுமை FAST_FUNC read_stduu(FILE *src_stream, FILE *dst_stream, முழு flags UNUSED_PARAM)
{
	எழுத்து *வரி;

	க்கு (;;) {
		முழு encoded_len, str_len;
		எழுத்து *வரி_ptr, *dst;
		size_t வரி_len;

		வரி_len = 64 * 1024;
		வரி = xmalloc_fgets_str_len(src_stream, "\n", &வரி_len);
		எனில் (!வரி)
			நிறுத்து;
		/* Handle both Unix and MSDOS text.
		 * Note: space should not be trimmed, some encoders use it instead of "`"
		 * for padding of last incomplete 4-char block.
		 */
		str_len = வரி_len;
		போது (--str_len >= 0
		 && (வரி[str_len] == '\n' || வரி[str_len] == '\r')
		) {
			வரி[str_len] = '\0';
		}

		எனில் (strcmp(வரி, "end") == 0) {
			திரும்பு; /* the only non-error exit */
		}

		வரி_ptr = வரி;
		போது (*வரி_ptr) {
			*வரி_ptr = (*வரி_ptr - 0x20) & 0x3f;
			வரி_ptr++;
		}
		str_len = வரி_ptr - வரி;

		encoded_len = வரி[0] * 4 / 3;
		/* Check that line is not too short. (we tolerate
		 * overly _long_ line to accommodate possible extra "`").
		 * Empty line case is also caught here. */
		எனில் (str_len <= encoded_len) {
			நிறுத்து; /* go to bb_error_msg_and_die("short file"); */
		}
		எனில் (encoded_len <= 0) {
			/* Ignore the "`\n" line, why is it even in the encode file ? */
			விடுவி(வரி);
			தொடர்;
		}
		எனில் (encoded_len > 60) {
			bb_simple_error_செய்தி_and_die("line too long");
		}

		dst = வரி;
		வரி_ptr = வரி + 1;
		செய் {
			/* Merge four 6 bit chars to three 8 bit chars */
			*dst++ = வரி_ptr[0] << 2 | வரி_ptr[1] >> 4;
			encoded_len--;
			எனில் (encoded_len == 0) {
				நிறுத்து;
			}

			*dst++ = வரி_ptr[1] << 4 | வரி_ptr[2] >> 2;
			encoded_len--;
			எனில் (encoded_len == 0) {
				நிறுத்து;
			}

			*dst++ = வரி_ptr[2] << 6 | வரி_ptr[3];
			வரி_ptr += 4;
			encoded_len -= 2;
		} போது (encoded_len > 0);
		fwrite(வரி, 1, dst - வரி, dst_stream);
		விடுவி(வரி);
	}
	bb_simple_error_செய்தி_and_die("short file");
}

முழு uudecode_main(முழு argc, எழுத்து **argv) MAIN_EXTERNALLY_VISIBLE;
முழு uudecode_main(முழு argc UNUSED_PARAM, எழுத்து **argv)
{
	FILE *src_stream;
	எழுத்து *outname = வெற்று;
	எழுத்து *வரி;

	getopt32(argv, "^" "o:" "\0" "?1"/* 1 arg max*/, &outname);
	argv += optind;

	எனில் (!argv[0])
		*--argv = (எழுத்து*)"-";
	src_stream = xfopen_stdin(argv[0]);

	/* Search for the start of the encoding */
	போது ((வரி = xmalloc_fgetline(src_stream)) != வெற்று) {
		வெறுமை FAST_FUNC (*decode_fn_ptr)(FILE *src, FILE *dst, முழு flags);
		எழுத்து *வரி_ptr;
		FILE *dst_stream;
		முழு mode;

		எனில் (is_prefixed_with(வரி, "begin-base64 ")) {
			வரி_ptr = வரி + 13;
			decode_fn_ptr = read_base64;
		} இல்லையெனில் எனில் (is_prefixed_with(வரி, "begin ")) {
			வரி_ptr = வரி + 6;
			decode_fn_ptr = read_stduu;
		} இல்லையெனில் {
			விடுவி(வரி);
			தொடர்;
		}

		/* begin line found. decode and exit */
		mode = bb_strtou(வரி_ptr, வெற்று, 8);
		எனில் (outname == வெற்று) {
			outname = strchr(வரி_ptr, ' ');
			எனில் (!outname)
				நிறுத்து;
			outname++;
			trim(outname); /* remove trailing space (and '\r' for DOS text) */
			எனில் (!outname[0])
				நிறுத்து;
		}
		dst_stream = stdout;
		எனில் (NOT_LONE_DASH(outname)
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
		விடுவி(வரி);
		decode_fn_ptr(src_stream, dst_stream, /*flags:*/ BASE64_FLAG_UU_STOP + BASE64_FLAG_NO_STOP_CHAR);
		/* fclose_if_not_stdin(src_stream); - redundant */
		திரும்பு EXIT_SUCCESS;
	}
	bb_simple_error_செய்தி_and_die("no 'begin' line");
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

#எனில் ENABLE_BASE32 || ENABLE_BASE64

# எனில் ENABLE_BASE32
நிலையான வெறுமை bb_b32encode(எழுத்து *p, மாறிலி வெறுமை *src, முழு length)
{
#define tbl bb_uuenc_tbl_base32
	மாறிலி unsigned எழுத்து *s = src;

	/* Transform 5x8 bits to 8x5 bits */
	போது (length > 0) {
		unsigned cur, next;

		length--;
		cur = *s++;
		*p++ = tbl[cur >> 3];			// xxxxx--- -------- -------- -------- --------
		cur &= 7;

		next = 0;
		எனில் (--length >= 0)
			next = *s++;
		*p++ = tbl[(cur << 2) + (next >> 6)];	// -----xxx xx------ -------- -------- --------
		cur = next & 0x3f;

		*p++ = tbl[cur >> 1];			// -------- --xxxxx- -------- -------- --------
		cur &= 1;

		next = 0;
		எனில் (--length >= 0)
			next = *s++;
		*p++ = tbl[(cur << 4) + (next >> 4)];	// -------- -------x xxxx---- -------- --------
		cur = next & 0xf;

		next = 0;
		எனில் (--length >= 0)
			next = *s++;
		*p++ = tbl[(cur << 1) + (next >> 7)];	// -------- -------- ----xxxx x------- --------
		cur = next & 0x7f;

		*p++ = tbl[cur >> 2];			// -------- -------- -------- -xxxxx-- --------
		cur &= 3;

		next = 0;
		எனில் (--length >= 0)
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
	போது (length--) {
		*--p = '=';
	}
}
# இல்லையெனில்
வெறுமை bb_b32encode(எழுத்து *p, மாறிலி வெறுமை *src, முழு length); /* undefined */
# endif

முழு base_இலக்கம்_main(முழு argc, எழுத்து **argv) MAIN_EXTERNALLY_VISIBLE;
முழு base_இலக்கம்_main(முழு argc UNUSED_PARAM, எழுத்து **argv)
{
	FILE *src_stream;
	unsigned opts;
	unsigned col = 76;

	opts = getopt32(argv, "^" "diw:+" "\0" "?1"/* 1 arg max*/, &col);
	argv += optind;

	எனில் (!argv[0])
		*--argv = (எழுத்து*)"-";
	src_stream = xfopen_stdin(argv[0]);
	எனில் (opts & 1) {
		/* -d: decode */
		முழு flags = (unsigned எழுத்து)EOF;
		எனில் (ENABLE_BASE32 && (!ENABLE_BASE64 || applet_name[4] == '3'))
			flags = ((unsigned எழுத்து)EOF) | BASE64_32;
		read_base64(src_stream, stdout, flags);
	} இல்லையெனில் {
		எண்ணிக்கை {
			SRC_BUF_SIZE = 3 * 5 * 32, /* this *MUST* be a multiple of 3 and 5 */
			DST_BUF_SIZE = 8 * ((SRC_BUF_SIZE + 4) / 5), /* max growth on encode (base32 case) */
		};
		/* Use one buffer for both input and output:
		 * encoding reads input "left-to-right",
		 * it's safe to place source at the end of the buffer and
		 * overwrite it while encoding, just be careful to have a gap.
		 */
		எழுத்து dst_buf[((DST_BUF_SIZE + /*gap:*/ 16) /*round up to 16:*/ | 0xf) + 1];
#define src_buf (dst_buf + அளவு(dst_buf) - SRC_BUF_SIZE)
		முழு src_fd, rem;

		src_fd = fileno(src_stream);
		rem = 0;
		போது (1) {
			size_t size = full_read(src_fd, src_buf, SRC_BUF_SIZE);
			எனில் ((ssize_t)size < 0)
				bb_simple_perror_செய்தி_and_die(bb_செய்தி_read_error);
			எனில் (size == 0) {
				எனில் (rem != 0) bb_putchar('\n');
				நிறுத்து;
			}

			/* Encode the buffer we just read in */
			எனில் (ENABLE_BASE32 && (!ENABLE_BASE64 || applet_name[4] == '3')) {
				bb_b32encode(dst_buf, src_buf, size);
				size = 8 * ((size + 4) / 5);
			} இல்லையெனில் {
				bb_uuencode(dst_buf, src_buf, size, bb_uuenc_tbl_base64);
				size = 4 * ((size + 2) / 3);
			}

			எனில் (col == 0) {
				fputs_stdout(dst_buf);
			} இல்லையெனில் {
				எழுத்து *result = dst_buf;
				எனில் (rem == 0)
					rem = col;
				போது (1) {
					முழு out = size < rem ? size : rem;
					rem -= out;
					printf(rem != 0 ? "%.*s" : "%.*s\n", out, result);
					எனில் (rem != 0)
						நிறுத்து;
					size -= out;
					எனில் (size == 0)
						நிறுத்து;
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
