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

#अन्तर्भाव "libbb.h"

#जइ ENABLE_UUDECODE
स्थिर शून्य FAST_FUNC read_stduu(FILE *src_stream, FILE *dst_stream, पूर्णाङ्क flags UNUSED_PARAM)
{
	वर्ण *पङ्क्तिः;

	प्रति (;;) {
		पूर्णाङ्क encoded_len, str_len;
		वर्ण *पङ्क्तिः_ptr, *dst;
		size_t पङ्क्तिः_len;

		पङ्क्तिः_len = 64 * 1024;
		पङ्क्तिः = xmalloc_fgets_str_len(src_stream, "\n", &पङ्क्तिः_len);
		जइ (!पङ्क्तिः)
			भङ्ग;
		/* Handle both Unix and MSDOS text.
		 * Note: space should not be trimmed, some encoders use it instead of "`"
		 * for padding of last incomplete 4-char block.
		 */
		str_len = पङ्क्तिः_len;
		जाव (--str_len >= 0
		 && (पङ्क्तिः[str_len] == '\n' || पङ्क्तिः[str_len] == '\r')
		) {
			पङ्क्तिः[str_len] = '\0';
		}

		जइ (strcmp(पङ्क्तिः, "end") == 0) {
			प्रत्यागम; /* the only non-error exit */
		}

		पङ्क्तिः_ptr = पङ्क्तिः;
		जाव (*पङ्क्तिः_ptr) {
			*पङ्क्तिः_ptr = (*पङ्क्तिः_ptr - 0x20) & 0x3f;
			पङ्क्तिः_ptr++;
		}
		str_len = पङ्क्तिः_ptr - पङ्क्तिः;

		encoded_len = पङ्क्तिः[0] * 4 / 3;
		/* Check that line is not too short. (we tolerate
		 * overly _long_ line to accommodate possible extra "`").
		 * Empty line case is also caught here. */
		जइ (str_len <= encoded_len) {
			भङ्ग; /* go to bb_error_msg_and_die("short file"); */
		}
		जइ (encoded_len <= 0) {
			/* Ignore the "`\n" line, why is it even in the encode file ? */
			मुक्त(पङ्क्तिः);
			अनुवर्त;
		}
		जइ (encoded_len > 60) {
			bb_simple_error_सन्देशः_and_die("line too long");
		}

		dst = पङ्क्तिः;
		पङ्क्तिः_ptr = पङ्क्तिः + 1;
		करेहि {
			/* Merge four 6 bit chars to three 8 bit chars */
			*dst++ = पङ्क्तिः_ptr[0] << 2 | पङ्क्तिः_ptr[1] >> 4;
			encoded_len--;
			जइ (encoded_len == 0) {
				भङ्ग;
			}

			*dst++ = पङ्क्तिः_ptr[1] << 4 | पङ्क्तिः_ptr[2] >> 2;
			encoded_len--;
			जइ (encoded_len == 0) {
				भङ्ग;
			}

			*dst++ = पङ्क्तिः_ptr[2] << 6 | पङ्क्तिः_ptr[3];
			पङ्क्तिः_ptr += 4;
			encoded_len -= 2;
		} जाव (encoded_len > 0);
		fwrite(पङ्क्तिः, 1, dst - पङ्क्तिः, dst_stream);
		मुक्त(पङ्क्तिः);
	}
	bb_simple_error_सन्देशः_and_die("short file");
}

पूर्णाङ्क uudecode_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क uudecode_main(पूर्णाङ्क argc UNUSED_PARAM, वर्ण **argv)
{
	FILE *src_stream;
	वर्ण *outname = रिक्त;
	वर्ण *पङ्क्तिः;

	getopt32(argv, "^" "o:" "\0" "?1"/* 1 arg max*/, &outname);
	argv += optind;

	जइ (!argv[0])
		*--argv = (वर्ण*)"-";
	src_stream = xfopen_stdin(argv[0]);

	/* Search for the start of the encoding */
	जाव ((पङ्क्तिः = xmalloc_fgetline(src_stream)) != रिक्त) {
		शून्य FAST_FUNC (*decode_fn_ptr)(FILE *src, FILE *dst, पूर्णाङ्क flags);
		वर्ण *पङ्क्तिः_ptr;
		FILE *dst_stream;
		पूर्णाङ्क mode;

		जइ (is_prefixed_with(पङ्क्तिः, "begin-base64 ")) {
			पङ्क्तिः_ptr = पङ्क्तिः + 13;
			decode_fn_ptr = read_base64;
		} अण्णहा जइ (is_prefixed_with(पङ्क्तिः, "begin ")) {
			पङ्क्तिः_ptr = पङ्क्तिः + 6;
			decode_fn_ptr = read_stduu;
		} अण्णहा {
			मुक्त(पङ्क्तिः);
			अनुवर्त;
		}

		/* begin line found. decode and exit */
		mode = bb_strtou(पङ्क्तिः_ptr, रिक्त, 8);
		जइ (outname == रिक्त) {
			outname = strchr(पङ्क्तिः_ptr, ' ');
			जइ (!outname)
				भङ्ग;
			outname++;
			trim(outname); /* remove trailing space (and '\r' for DOS text) */
			जइ (!outname[0])
				भङ्ग;
		}
		dst_stream = stdout;
		जइ (NOT_LONE_DASH(outname)
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
		मुक्त(पङ्क्तिः);
		decode_fn_ptr(src_stream, dst_stream, /*flags:*/ BASE64_FLAG_UU_STOP + BASE64_FLAG_NO_STOP_CHAR);
		/* fclose_if_not_stdin(src_stream); - redundant */
		प्रत्यागम EXIT_SUCCESS;
	}
	bb_simple_error_सन्देशः_and_die("no 'begin' line");
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

#जइ ENABLE_BASE32 || ENABLE_BASE64

# जइ ENABLE_BASE32
स्थिर शून्य bb_b32encode(वर्ण *p, ध्रुव शून्य *src, पूर्णाङ्क length)
{
#define tbl bb_uuenc_tbl_base32
	ध्रुव unsigned वर्ण *s = src;

	/* Transform 5x8 bits to 8x5 bits */
	जाव (length > 0) {
		unsigned cur, next;

		length--;
		cur = *s++;
		*p++ = tbl[cur >> 3];			// xxxxx--- -------- -------- -------- --------
		cur &= 7;

		next = 0;
		जइ (--length >= 0)
			next = *s++;
		*p++ = tbl[(cur << 2) + (next >> 6)];	// -----xxx xx------ -------- -------- --------
		cur = next & 0x3f;

		*p++ = tbl[cur >> 1];			// -------- --xxxxx- -------- -------- --------
		cur &= 1;

		next = 0;
		जइ (--length >= 0)
			next = *s++;
		*p++ = tbl[(cur << 4) + (next >> 4)];	// -------- -------x xxxx---- -------- --------
		cur = next & 0xf;

		next = 0;
		जइ (--length >= 0)
			next = *s++;
		*p++ = tbl[(cur << 1) + (next >> 7)];	// -------- -------- ----xxxx x------- --------
		cur = next & 0x7f;

		*p++ = tbl[cur >> 2];			// -------- -------- -------- -xxxxx-- --------
		cur &= 3;

		next = 0;
		जइ (--length >= 0)
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
	जाव (length--) {
		*--p = '=';
	}
}
# अण्णहा
शून्य bb_b32encode(वर्ण *p, ध्रुव शून्य *src, पूर्णाङ्क length); /* undefined */
# endif

पूर्णाङ्क base_अङ्कः_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क base_अङ्कः_main(पूर्णाङ्क argc UNUSED_PARAM, वर्ण **argv)
{
	FILE *src_stream;
	unsigned opts;
	unsigned col = 76;

	opts = getopt32(argv, "^" "diw:+" "\0" "?1"/* 1 arg max*/, &col);
	argv += optind;

	जइ (!argv[0])
		*--argv = (वर्ण*)"-";
	src_stream = xfopen_stdin(argv[0]);
	जइ (opts & 1) {
		/* -d: decode */
		पूर्णाङ्क flags = (unsigned वर्ण)EOF;
		जइ (ENABLE_BASE32 && (!ENABLE_BASE64 || applet_name[4] == '3'))
			flags = ((unsigned वर्ण)EOF) | BASE64_32;
		read_base64(src_stream, stdout, flags);
	} अण्णहा {
		गणना {
			SRC_BUF_SIZE = 3 * 5 * 32, /* this *MUST* be a multiple of 3 and 5 */
			DST_BUF_SIZE = 8 * ((SRC_BUF_SIZE + 4) / 5), /* max growth on encode (base32 case) */
		};
		/* Use one buffer for both input and output:
		 * encoding reads input "left-to-right",
		 * it's safe to place source at the end of the buffer and
		 * overwrite it while encoding, just be careful to have a gap.
		 */
		वर्ण dst_buf[((DST_BUF_SIZE + /*gap:*/ 16) /*round up to 16:*/ | 0xf) + 1];
#define src_buf (dst_buf + परिमाण(dst_buf) - SRC_BUF_SIZE)
		पूर्णाङ्क src_fd, rem;

		src_fd = fileno(src_stream);
		rem = 0;
		जाव (1) {
			size_t size = full_read(src_fd, src_buf, SRC_BUF_SIZE);
			जइ ((ssize_t)size < 0)
				bb_simple_perror_सन्देशः_and_die(bb_सन्देशः_read_error);
			जइ (size == 0) {
				जइ (rem != 0) bb_putchar('\n');
				भङ्ग;
			}

			/* Encode the buffer we just read in */
			जइ (ENABLE_BASE32 && (!ENABLE_BASE64 || applet_name[4] == '3')) {
				bb_b32encode(dst_buf, src_buf, size);
				size = 8 * ((size + 4) / 5);
			} अण्णहा {
				bb_uuencode(dst_buf, src_buf, size, bb_uuenc_tbl_base64);
				size = 4 * ((size + 2) / 3);
			}

			जइ (col == 0) {
				fputs_stdout(dst_buf);
			} अण्णहा {
				वर्ण *result = dst_buf;
				जइ (rem == 0)
					rem = col;
				जाव (1) {
					पूर्णाङ्क out = size < rem ? size : rem;
					rem -= out;
					printf(rem != 0 ? "%.*s" : "%.*s\n", out, result);
					जइ (rem != 0)
						भङ्ग;
					size -= out;
					जइ (size == 0)
						भङ्ग;
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
