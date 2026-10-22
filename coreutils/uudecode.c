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

#അന്തര്ഭാവ "libbb.h"

#എങ്കിൽ ENABLE_UUDECODE
സ്ഥിതം ശൂന്യം FAST_FUNC read_stduu(FILE *src_stream, FILE *dst_stream, പൂർണ്ണസംഖ്യ flags UNUSED_PARAM)
{
	അക്ഷരം *വരി;

	വേണ്ടി (;;) {
		പൂർണ്ണസംഖ്യ encoded_len, str_len;
		അക്ഷരം *വരി_ptr, *dst;
		size_t വരി_len;

		വരി_len = 64 * 1024;
		വരി = xmalloc_fgets_str_len(src_stream, "\n", &വരി_len);
		എങ്കിൽ (!വരി)
			നിർത്തുക;
		/* Handle both Unix and MSDOS text.
		 * Note: space should not be trimmed, some encoders use it instead of "`"
		 * for padding of last incomplete 4-char block.
		 */
		str_len = വരി_len;
		ആകുമ്പോൾ (--str_len >= 0
		 && (വരി[str_len] == '\n' || വരി[str_len] == '\r')
		) {
			വരി[str_len] = '\0';
		}

		എങ്കിൽ (strcmp(വരി, "end") == 0) {
			തിരികെ; /* the only non-error exit */
		}

		വരി_ptr = വരി;
		ആകുമ്പോൾ (*വരി_ptr) {
			*വരി_ptr = (*വരി_ptr - 0x20) & 0x3f;
			വരി_ptr++;
		}
		str_len = വരി_ptr - വരി;

		encoded_len = വരി[0] * 4 / 3;
		/* Check that line is not too short. (we tolerate
		 * overly _long_ line to accommodate possible extra "`").
		 * Empty line case is also caught here. */
		എങ്കിൽ (str_len <= encoded_len) {
			നിർത്തുക; /* go to bb_error_msg_and_die("short file"); */
		}
		എങ്കിൽ (encoded_len <= 0) {
			/* Ignore the "`\n" line, why is it even in the encode file ? */
			സ്വതന്ത്രം(വരി);
			തുടരുക;
		}
		എങ്കിൽ (encoded_len > 60) {
			bb_simple_error_സന്ദേശം_and_die("line too long");
		}

		dst = വരി;
		വരി_ptr = വരി + 1;
		ചെയ്യുക {
			/* Merge four 6 bit chars to three 8 bit chars */
			*dst++ = വരി_ptr[0] << 2 | വരി_ptr[1] >> 4;
			encoded_len--;
			എങ്കിൽ (encoded_len == 0) {
				നിർത്തുക;
			}

			*dst++ = വരി_ptr[1] << 4 | വരി_ptr[2] >> 2;
			encoded_len--;
			എങ്കിൽ (encoded_len == 0) {
				നിർത്തുക;
			}

			*dst++ = വരി_ptr[2] << 6 | വരി_ptr[3];
			വരി_ptr += 4;
			encoded_len -= 2;
		} ആകുമ്പോൾ (encoded_len > 0);
		fwrite(വരി, 1, dst - വരി, dst_stream);
		സ്വതന്ത്രം(വരി);
	}
	bb_simple_error_സന്ദേശം_and_die("short file");
}

പൂർണ്ണസംഖ്യ uudecode_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) MAIN_EXTERNALLY_VISIBLE;
പൂർണ്ണസംഖ്യ uudecode_main(പൂർണ്ണസംഖ്യ argc UNUSED_PARAM, അക്ഷരം **argv)
{
	FILE *src_stream;
	അക്ഷരം *outname = ശൂന്യ;
	അക്ഷരം *വരി;

	getopt32(argv, "^" "o:" "\0" "?1"/* 1 arg max*/, &outname);
	argv += optind;

	എങ്കിൽ (!argv[0])
		*--argv = (അക്ഷരം*)"-";
	src_stream = xfopen_stdin(argv[0]);

	/* Search for the start of the encoding */
	ആകുമ്പോൾ ((വരി = xmalloc_fgetline(src_stream)) != ശൂന്യ) {
		ശൂന്യം FAST_FUNC (*decode_fn_ptr)(FILE *src, FILE *dst, പൂർണ്ണസംഖ്യ flags);
		അക്ഷരം *വരി_ptr;
		FILE *dst_stream;
		പൂർണ്ണസംഖ്യ mode;

		എങ്കിൽ (is_prefixed_with(വരി, "begin-base64 ")) {
			വരി_ptr = വരി + 13;
			decode_fn_ptr = read_base64;
		} അല്ലെങ്കിൽ എങ്കിൽ (is_prefixed_with(വരി, "begin ")) {
			വരി_ptr = വരി + 6;
			decode_fn_ptr = read_stduu;
		} അല്ലെങ്കിൽ {
			സ്വതന്ത്രം(വരി);
			തുടരുക;
		}

		/* begin line found. decode and exit */
		mode = bb_strtou(വരി_ptr, ശൂന്യ, 8);
		എങ്കിൽ (outname == ശൂന്യ) {
			outname = strchr(വരി_ptr, ' ');
			എങ്കിൽ (!outname)
				നിർത്തുക;
			outname++;
			trim(outname); /* remove trailing space (and '\r' for DOS text) */
			എങ്കിൽ (!outname[0])
				നിർത്തുക;
		}
		dst_stream = stdout;
		എങ്കിൽ (NOT_LONE_DASH(outname)
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
		സ്വതന്ത്രം(വരി);
		decode_fn_ptr(src_stream, dst_stream, /*flags:*/ BASE64_FLAG_UU_STOP + BASE64_FLAG_NO_STOP_CHAR);
		/* fclose_if_not_stdin(src_stream); - redundant */
		തിരികെ EXIT_SUCCESS;
	}
	bb_simple_error_സന്ദേശം_and_die("no 'begin' line");
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

#എങ്കിൽ ENABLE_BASE32 || ENABLE_BASE64

# എങ്കിൽ ENABLE_BASE32
സ്ഥിതം ശൂന്യം bb_b32encode(അക്ഷരം *p, സ്ഥിരം ശൂന്യം *src, പൂർണ്ണസംഖ്യ length)
{
#define tbl bb_uuenc_tbl_base32
	സ്ഥിരം unsigned അക്ഷരം *s = src;

	/* Transform 5x8 bits to 8x5 bits */
	ആകുമ്പോൾ (length > 0) {
		unsigned cur, next;

		length--;
		cur = *s++;
		*p++ = tbl[cur >> 3];			// xxxxx--- -------- -------- -------- --------
		cur &= 7;

		next = 0;
		എങ്കിൽ (--length >= 0)
			next = *s++;
		*p++ = tbl[(cur << 2) + (next >> 6)];	// -----xxx xx------ -------- -------- --------
		cur = next & 0x3f;

		*p++ = tbl[cur >> 1];			// -------- --xxxxx- -------- -------- --------
		cur &= 1;

		next = 0;
		എങ്കിൽ (--length >= 0)
			next = *s++;
		*p++ = tbl[(cur << 4) + (next >> 4)];	// -------- -------x xxxx---- -------- --------
		cur = next & 0xf;

		next = 0;
		എങ്കിൽ (--length >= 0)
			next = *s++;
		*p++ = tbl[(cur << 1) + (next >> 7)];	// -------- -------- ----xxxx x------- --------
		cur = next & 0x7f;

		*p++ = tbl[cur >> 2];			// -------- -------- -------- -xxxxx-- --------
		cur &= 3;

		next = 0;
		എങ്കിൽ (--length >= 0)
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
	ആകുമ്പോൾ (length--) {
		*--p = '=';
	}
}
# അല്ലെങ്കിൽ
ശൂന്യം bb_b32encode(അക്ഷരം *p, സ്ഥിരം ശൂന്യം *src, പൂർണ്ണസംഖ്യ length); /* undefined */
# endif

പൂർണ്ണസംഖ്യ base_അക്കം_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) MAIN_EXTERNALLY_VISIBLE;
പൂർണ്ണസംഖ്യ base_അക്കം_main(പൂർണ്ണസംഖ്യ argc UNUSED_PARAM, അക്ഷരം **argv)
{
	FILE *src_stream;
	unsigned opts;
	unsigned col = 76;

	opts = getopt32(argv, "^" "diw:+" "\0" "?1"/* 1 arg max*/, &col);
	argv += optind;

	എങ്കിൽ (!argv[0])
		*--argv = (അക്ഷരം*)"-";
	src_stream = xfopen_stdin(argv[0]);
	എങ്കിൽ (opts & 1) {
		/* -d: decode */
		പൂർണ്ണസംഖ്യ flags = (unsigned അക്ഷരം)EOF;
		എങ്കിൽ (ENABLE_BASE32 && (!ENABLE_BASE64 || applet_name[4] == '3'))
			flags = ((unsigned അക്ഷരം)EOF) | BASE64_32;
		read_base64(src_stream, stdout, flags);
	} അല്ലെങ്കിൽ {
		എണ്ണൽ {
			SRC_BUF_SIZE = 3 * 5 * 32, /* this *MUST* be a multiple of 3 and 5 */
			DST_BUF_SIZE = 8 * ((SRC_BUF_SIZE + 4) / 5), /* max growth on encode (base32 case) */
		};
		/* Use one buffer for both input and output:
		 * encoding reads input "left-to-right",
		 * it's safe to place source at the end of the buffer and
		 * overwrite it while encoding, just be careful to have a gap.
		 */
		അക്ഷരം dst_buf[((DST_BUF_SIZE + /*gap:*/ 16) /*round up to 16:*/ | 0xf) + 1];
#define src_buf (dst_buf + വലുപ്പം(dst_buf) - SRC_BUF_SIZE)
		പൂർണ്ണസംഖ്യ src_fd, rem;

		src_fd = fileno(src_stream);
		rem = 0;
		ആകുമ്പോൾ (1) {
			size_t size = full_read(src_fd, src_buf, SRC_BUF_SIZE);
			എങ്കിൽ ((ssize_t)size < 0)
				bb_simple_perror_സന്ദേശം_and_die(bb_സന്ദേശം_read_error);
			എങ്കിൽ (size == 0) {
				എങ്കിൽ (rem != 0) bb_putchar('\n');
				നിർത്തുക;
			}

			/* Encode the buffer we just read in */
			എങ്കിൽ (ENABLE_BASE32 && (!ENABLE_BASE64 || applet_name[4] == '3')) {
				bb_b32encode(dst_buf, src_buf, size);
				size = 8 * ((size + 4) / 5);
			} അല്ലെങ്കിൽ {
				bb_uuencode(dst_buf, src_buf, size, bb_uuenc_tbl_base64);
				size = 4 * ((size + 2) / 3);
			}

			എങ്കിൽ (col == 0) {
				fputs_stdout(dst_buf);
			} അല്ലെങ്കിൽ {
				അക്ഷരം *result = dst_buf;
				എങ്കിൽ (rem == 0)
					rem = col;
				ആകുമ്പോൾ (1) {
					പൂർണ്ണസംഖ്യ out = size < rem ? size : rem;
					rem -= out;
					printf(rem != 0 ? "%.*s" : "%.*s\n", out, result);
					എങ്കിൽ (rem != 0)
						നിർത്തുക;
					size -= out;
					എങ്കിൽ (size == 0)
						നിർത്തുക;
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
