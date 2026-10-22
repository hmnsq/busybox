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

#شامل "libbb.h"

#اگر ENABLE_UUDECODE
ایستا تهی FAST_FUNC read_stduu(FILE *src_stream, FILE *dst_stream, صحیح flags UNUSED_PARAM)
{
	نویسه *سطر;

	برای (;;) {
		صحیح encoded_len, str_len;
		نویسه *سطر_ptr, *dst;
		size_t سطر_len;

		سطر_len = 64 * 1024;
		سطر = xmalloc_fgets_str_len(src_stream, "\n", &سطر_len);
		اگر (!سطر)
			بشکن;
		/* Handle both Unix and MSDOS text.
		 * Note: space should not be trimmed, some encoders use it instead of "`"
		 * for padding of last incomplete 4-char block.
		 */
		str_len = سطر_len;
		وقتی (--str_len >= 0
		 && (سطر[str_len] == '\n' || سطر[str_len] == '\r')
		) {
			سطر[str_len] = '\0';
		}

		اگر (strcmp(سطر, "end") == 0) {
			بازگشت; /* the only non-error exit */
		}

		سطر_ptr = سطر;
		وقتی (*سطر_ptr) {
			*سطر_ptr = (*سطر_ptr - 0x20) & 0x3f;
			سطر_ptr++;
		}
		str_len = سطر_ptr - سطر;

		encoded_len = سطر[0] * 4 / 3;
		/* Check that line is not too short. (we tolerate
		 * overly _long_ line to accommodate possible extra "`").
		 * Empty line case is also caught here. */
		اگر (str_len <= encoded_len) {
			بشکن; /* go to bb_error_msg_and_die("short file"); */
		}
		اگر (encoded_len <= 0) {
			/* Ignore the "`\n" line, why is it even in the encode file ? */
			آزاد(سطر);
			ادامه;
		}
		اگر (encoded_len > 60) {
			bb_simple_error_پیام_and_die("line too long");
		}

		dst = سطر;
		سطر_ptr = سطر + 1;
		انجامبده {
			/* Merge four 6 bit chars to three 8 bit chars */
			*dst++ = سطر_ptr[0] << 2 | سطر_ptr[1] >> 4;
			encoded_len--;
			اگر (encoded_len == 0) {
				بشکن;
			}

			*dst++ = سطر_ptr[1] << 4 | سطر_ptr[2] >> 2;
			encoded_len--;
			اگر (encoded_len == 0) {
				بشکن;
			}

			*dst++ = سطر_ptr[2] << 6 | سطر_ptr[3];
			سطر_ptr += 4;
			encoded_len -= 2;
		} وقتی (encoded_len > 0);
		fwrite(سطر, 1, dst - سطر, dst_stream);
		آزاد(سطر);
	}
	bb_simple_error_پیام_and_die("short file");
}

صحیح uudecode_main(صحیح argc, نویسه **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح uudecode_main(صحیح argc UNUSED_PARAM, نویسه **argv)
{
	FILE *src_stream;
	نویسه *outname = پوچ;
	نویسه *سطر;

	getopt32(argv, "^" "o:" "\0" "?1"/* 1 arg max*/, &outname);
	argv += optind;

	اگر (!argv[0])
		*--argv = (نویسه*)"-";
	src_stream = xfopen_stdin(argv[0]);

	/* Search for the start of the encoding */
	وقتی ((سطر = xmalloc_fgetline(src_stream)) != پوچ) {
		تهی FAST_FUNC (*decode_fn_ptr)(FILE *src, FILE *dst, صحیح flags);
		نویسه *سطر_ptr;
		FILE *dst_stream;
		صحیح mode;

		اگر (is_prefixed_with(سطر, "begin-base64 ")) {
			سطر_ptr = سطر + 13;
			decode_fn_ptr = read_base64;
		} وگرنه اگر (is_prefixed_with(سطر, "begin ")) {
			سطر_ptr = سطر + 6;
			decode_fn_ptr = read_stduu;
		} وگرنه {
			آزاد(سطر);
			ادامه;
		}

		/* begin line found. decode and exit */
		mode = bb_strtou(سطر_ptr, پوچ, 8);
		اگر (outname == پوچ) {
			outname = strchr(سطر_ptr, ' ');
			اگر (!outname)
				بشکن;
			outname++;
			trim(outname); /* remove trailing space (and '\r' for DOS text) */
			اگر (!outname[0])
				بشکن;
		}
		dst_stream = stdout;
		اگر (NOT_LONE_DASH(outname)
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
		آزاد(سطر);
		decode_fn_ptr(src_stream, dst_stream, /*flags:*/ BASE64_FLAG_UU_STOP + BASE64_FLAG_NO_STOP_CHAR);
		/* fclose_if_not_stdin(src_stream); - redundant */
		بازگشت EXIT_SUCCESS;
	}
	bb_simple_error_پیام_and_die("no 'begin' line");
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

#اگر ENABLE_BASE32 || ENABLE_BASE64

# اگر ENABLE_BASE32
ایستا تهی bb_b32encode(نویسه *p, ثابت تهی *src, صحیح length)
{
#define tbl bb_uuenc_tbl_base32
	ثابت unsigned نویسه *s = src;

	/* Transform 5x8 bits to 8x5 bits */
	وقتی (length > 0) {
		unsigned cur, next;

		length--;
		cur = *s++;
		*p++ = tbl[cur >> 3];			// xxxxx--- -------- -------- -------- --------
		cur &= 7;

		next = 0;
		اگر (--length >= 0)
			next = *s++;
		*p++ = tbl[(cur << 2) + (next >> 6)];	// -----xxx xx------ -------- -------- --------
		cur = next & 0x3f;

		*p++ = tbl[cur >> 1];			// -------- --xxxxx- -------- -------- --------
		cur &= 1;

		next = 0;
		اگر (--length >= 0)
			next = *s++;
		*p++ = tbl[(cur << 4) + (next >> 4)];	// -------- -------x xxxx---- -------- --------
		cur = next & 0xf;

		next = 0;
		اگر (--length >= 0)
			next = *s++;
		*p++ = tbl[(cur << 1) + (next >> 7)];	// -------- -------- ----xxxx x------- --------
		cur = next & 0x7f;

		*p++ = tbl[cur >> 2];			// -------- -------- -------- -xxxxx-- --------
		cur &= 3;

		next = 0;
		اگر (--length >= 0)
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
	وقتی (length--) {
		*--p = '=';
	}
}
# وگرنه
تهی bb_b32encode(نویسه *p, ثابت تهی *src, صحیح length); /* undefined */
# endif

صحیح base_رقم_main(صحیح argc, نویسه **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح base_رقم_main(صحیح argc UNUSED_PARAM, نویسه **argv)
{
	FILE *src_stream;
	unsigned opts;
	unsigned col = 76;

	opts = getopt32(argv, "^" "diw:+" "\0" "?1"/* 1 arg max*/, &col);
	argv += optind;

	اگر (!argv[0])
		*--argv = (نویسه*)"-";
	src_stream = xfopen_stdin(argv[0]);
	اگر (opts & 1) {
		/* -d: decode */
		صحیح flags = (unsigned نویسه)EOF;
		اگر (ENABLE_BASE32 && (!ENABLE_BASE64 || applet_name[4] == '3'))
			flags = ((unsigned نویسه)EOF) | BASE64_32;
		read_base64(src_stream, stdout, flags);
	} وگرنه {
		شمارش {
			SRC_BUF_SIZE = 3 * 5 * 32, /* this *MUST* be a multiple of 3 and 5 */
			DST_BUF_SIZE = 8 * ((SRC_BUF_SIZE + 4) / 5), /* max growth on encode (base32 case) */
		};
		/* Use one buffer for both input and output:
		 * encoding reads input "left-to-right",
		 * it's safe to place source at the end of the buffer and
		 * overwrite it while encoding, just be careful to have a gap.
		 */
		نویسه dst_buf[((DST_BUF_SIZE + /*gap:*/ 16) /*round up to 16:*/ | 0xf) + 1];
#define src_buf (dst_buf + اندازه(dst_buf) - SRC_BUF_SIZE)
		صحیح src_fd, rem;

		src_fd = fileno(src_stream);
		rem = 0;
		وقتی (1) {
			size_t size = full_read(src_fd, src_buf, SRC_BUF_SIZE);
			اگر ((ssize_t)size < 0)
				bb_simple_perror_پیام_and_die(bb_پیام_read_error);
			اگر (size == 0) {
				اگر (rem != 0) bb_putchar('\n');
				بشکن;
			}

			/* Encode the buffer we just read in */
			اگر (ENABLE_BASE32 && (!ENABLE_BASE64 || applet_name[4] == '3')) {
				bb_b32encode(dst_buf, src_buf, size);
				size = 8 * ((size + 4) / 5);
			} وگرنه {
				bb_uuencode(dst_buf, src_buf, size, bb_uuenc_tbl_base64);
				size = 4 * ((size + 2) / 3);
			}

			اگر (col == 0) {
				fputs_stdout(dst_buf);
			} وگرنه {
				نویسه *result = dst_buf;
				اگر (rem == 0)
					rem = col;
				وقتی (1) {
					صحیح out = size < rem ? size : rem;
					rem -= out;
					printf(rem != 0 ? "%.*s" : "%.*s\n", out, result);
					اگر (rem != 0)
						بشکن;
					size -= out;
					اگر (size == 0)
						بشکن;
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
