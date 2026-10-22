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

#অন্তর্ভাব "libbb.h"

#যদি ENABLE_UUDECODE
স্থির শূন্য FAST_FUNC read_stduu(FILE *src_stream, FILE *dst_stream, পূর্ণাঙ্ক flags UNUSED_PARAM)
{
	বর্ণ *সারি;

	প্রতি (;;) {
		পূর্ণাঙ্ক encoded_len, str_len;
		বর্ণ *সারি_ptr, *dst;
		size_t সারি_len;

		সারি_len = 64 * 1024;
		সারি = xmalloc_fgets_str_len(src_stream, "\n", &সারি_len);
		যদি (!সারি)
			ভঙ্গ;
		/* Handle both Unix and MSDOS text.
		 * Note: space should not be trimmed, some encoders use it instead of "`"
		 * for padding of last incomplete 4-char block.
		 */
		str_len = সারি_len;
		যাবত্ (--str_len >= 0
		 && (সারি[str_len] == '\n' || সারি[str_len] == '\r')
		) {
			সারি[str_len] = '\0';
		}

		যদি (strcmp(সারি, "end") == 0) {
			প্রত্যাগম; /* the only non-error exit */
		}

		সারি_ptr = সারি;
		যাবত্ (*সারি_ptr) {
			*সারি_ptr = (*সারি_ptr - 0x20) & 0x3f;
			সারি_ptr++;
		}
		str_len = সারি_ptr - সারি;

		encoded_len = সারি[0] * 4 / 3;
		/* Check that line is not too short. (we tolerate
		 * overly _long_ line to accommodate possible extra "`").
		 * Empty line case is also caught here. */
		যদি (str_len <= encoded_len) {
			ভঙ্গ; /* go to bb_error_msg_and_die("short file"); */
		}
		যদি (encoded_len <= 0) {
			/* Ignore the "`\n" line, why is it even in the encode file ? */
			মুক্ত(সারি);
			অনুবর্ত;
		}
		যদি (encoded_len > 60) {
			bb_simple_error_বার্তা_and_die("line too long");
		}

		dst = সারি;
		সারি_ptr = সারি + 1;
		কুরু {
			/* Merge four 6 bit chars to three 8 bit chars */
			*dst++ = সারি_ptr[0] << 2 | সারি_ptr[1] >> 4;
			encoded_len--;
			যদি (encoded_len == 0) {
				ভঙ্গ;
			}

			*dst++ = সারি_ptr[1] << 4 | সারি_ptr[2] >> 2;
			encoded_len--;
			যদি (encoded_len == 0) {
				ভঙ্গ;
			}

			*dst++ = সারি_ptr[2] << 6 | সারি_ptr[3];
			সারি_ptr += 4;
			encoded_len -= 2;
		} যাবত্ (encoded_len > 0);
		fwrite(সারি, 1, dst - সারি, dst_stream);
		মুক্ত(সারি);
	}
	bb_simple_error_বার্তা_and_die("short file");
}

পূর্ণাঙ্ক uudecode_main(পূর্ণাঙ্ক argc, বর্ণ **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণাঙ্ক uudecode_main(পূর্ণাঙ্ক argc UNUSED_PARAM, বর্ণ **argv)
{
	FILE *src_stream;
	বর্ণ *outname = রিক্ত;
	বর্ণ *সারি;

	getopt32(argv, "^" "o:" "\0" "?1"/* 1 arg max*/, &outname);
	argv += optind;

	যদি (!argv[0])
		*--argv = (বর্ণ*)"-";
	src_stream = xfopen_stdin(argv[0]);

	/* Search for the start of the encoding */
	যাবত্ ((সারি = xmalloc_fgetline(src_stream)) != রিক্ত) {
		শূন্য FAST_FUNC (*decode_fn_ptr)(FILE *src, FILE *dst, পূর্ণাঙ্ক flags);
		বর্ণ *সারি_ptr;
		FILE *dst_stream;
		পূর্ণাঙ্ক mode;

		যদি (is_prefixed_with(সারি, "begin-base64 ")) {
			সারি_ptr = সারি + 13;
			decode_fn_ptr = read_base64;
		} অন্যথা যদি (is_prefixed_with(সারি, "begin ")) {
			সারি_ptr = সারি + 6;
			decode_fn_ptr = read_stduu;
		} অন্যথা {
			মুক্ত(সারি);
			অনুবর্ত;
		}

		/* begin line found. decode and exit */
		mode = bb_strtou(সারি_ptr, রিক্ত, 8);
		যদি (outname == রিক্ত) {
			outname = strchr(সারি_ptr, ' ');
			যদি (!outname)
				ভঙ্গ;
			outname++;
			trim(outname); /* remove trailing space (and '\r' for DOS text) */
			যদি (!outname[0])
				ভঙ্গ;
		}
		dst_stream = stdout;
		যদি (NOT_LONE_DASH(outname)
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
		মুক্ত(সারি);
		decode_fn_ptr(src_stream, dst_stream, /*flags:*/ BASE64_FLAG_UU_STOP + BASE64_FLAG_NO_STOP_CHAR);
		/* fclose_if_not_stdin(src_stream); - redundant */
		প্রত্যাগম EXIT_SUCCESS;
	}
	bb_simple_error_বার্তা_and_die("no 'begin' line");
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

#যদি ENABLE_BASE32 || ENABLE_BASE64

# যদি ENABLE_BASE32
স্থির শূন্য bb_b32encode(বর্ণ *p, ধ্রুব শূন্য *src, পূর্ণাঙ্ক length)
{
#define tbl bb_uuenc_tbl_base32
	ধ্রুব unsigned বর্ণ *s = src;

	/* Transform 5x8 bits to 8x5 bits */
	যাবত্ (length > 0) {
		unsigned cur, next;

		length--;
		cur = *s++;
		*p++ = tbl[cur >> 3];			// xxxxx--- -------- -------- -------- --------
		cur &= 7;

		next = 0;
		যদি (--length >= 0)
			next = *s++;
		*p++ = tbl[(cur << 2) + (next >> 6)];	// -----xxx xx------ -------- -------- --------
		cur = next & 0x3f;

		*p++ = tbl[cur >> 1];			// -------- --xxxxx- -------- -------- --------
		cur &= 1;

		next = 0;
		যদি (--length >= 0)
			next = *s++;
		*p++ = tbl[(cur << 4) + (next >> 4)];	// -------- -------x xxxx---- -------- --------
		cur = next & 0xf;

		next = 0;
		যদি (--length >= 0)
			next = *s++;
		*p++ = tbl[(cur << 1) + (next >> 7)];	// -------- -------- ----xxxx x------- --------
		cur = next & 0x7f;

		*p++ = tbl[cur >> 2];			// -------- -------- -------- -xxxxx-- --------
		cur &= 3;

		next = 0;
		যদি (--length >= 0)
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
	যাবত্ (length--) {
		*--p = '=';
	}
}
# অন্যথা
শূন্য bb_b32encode(বর্ণ *p, ধ্রুব শূন্য *src, পূর্ণাঙ্ক length); /* undefined */
# endif

পূর্ণাঙ্ক base_অঙ্ক_main(পূর্ণাঙ্ক argc, বর্ণ **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণাঙ্ক base_অঙ্ক_main(পূর্ণাঙ্ক argc UNUSED_PARAM, বর্ণ **argv)
{
	FILE *src_stream;
	unsigned opts;
	unsigned col = 76;

	opts = getopt32(argv, "^" "diw:+" "\0" "?1"/* 1 arg max*/, &col);
	argv += optind;

	যদি (!argv[0])
		*--argv = (বর্ণ*)"-";
	src_stream = xfopen_stdin(argv[0]);
	যদি (opts & 1) {
		/* -d: decode */
		পূর্ণাঙ্ক flags = (unsigned বর্ণ)EOF;
		যদি (ENABLE_BASE32 && (!ENABLE_BASE64 || applet_name[4] == '3'))
			flags = ((unsigned বর্ণ)EOF) | BASE64_32;
		read_base64(src_stream, stdout, flags);
	} অন্যথা {
		গণনা {
			SRC_BUF_SIZE = 3 * 5 * 32, /* this *MUST* be a multiple of 3 and 5 */
			DST_BUF_SIZE = 8 * ((SRC_BUF_SIZE + 4) / 5), /* max growth on encode (base32 case) */
		};
		/* Use one buffer for both input and output:
		 * encoding reads input "left-to-right",
		 * it's safe to place source at the end of the buffer and
		 * overwrite it while encoding, just be careful to have a gap.
		 */
		বর্ণ dst_buf[((DST_BUF_SIZE + /*gap:*/ 16) /*round up to 16:*/ | 0xf) + 1];
#define src_buf (dst_buf + পরিমাণ(dst_buf) - SRC_BUF_SIZE)
		পূর্ণাঙ্ক src_fd, rem;

		src_fd = fileno(src_stream);
		rem = 0;
		যাবত্ (1) {
			size_t size = full_read(src_fd, src_buf, SRC_BUF_SIZE);
			যদি ((ssize_t)size < 0)
				bb_simple_perror_বার্তা_and_die(bb_বার্তা_read_error);
			যদি (size == 0) {
				যদি (rem != 0) bb_putchar('\n');
				ভঙ্গ;
			}

			/* Encode the buffer we just read in */
			যদি (ENABLE_BASE32 && (!ENABLE_BASE64 || applet_name[4] == '3')) {
				bb_b32encode(dst_buf, src_buf, size);
				size = 8 * ((size + 4) / 5);
			} অন্যথা {
				bb_uuencode(dst_buf, src_buf, size, bb_uuenc_tbl_base64);
				size = 4 * ((size + 2) / 3);
			}

			যদি (col == 0) {
				fputs_stdout(dst_buf);
			} অন্যথা {
				বর্ণ *result = dst_buf;
				যদি (rem == 0)
					rem = col;
				যাবত্ (1) {
					পূর্ণাঙ্ক out = size < rem ? size : rem;
					rem -= out;
					printf(rem != 0 ? "%.*s" : "%.*s\n", out, result);
					যদি (rem != 0)
						ভঙ্গ;
					size -= out;
					যদি (size == 0)
						ভঙ্গ;
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
