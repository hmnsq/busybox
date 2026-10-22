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

#यदि ENABLE_UUDECODE
स्थिर शून्य FAST_FUNC read_stduu(FILE *src_प्रवाहः, FILE *dst_प्रवाहः, पूर्णाङ्क flags अप्रयुक्तम्_PARAM)
{
	वर्ण *पङ्क्तिः;

	प्रति (;;) {
		पूर्णाङ्क encoded_len, str_len;
		वर्ण *पङ्क्तिः_ptr, *dst;
		आकार_t पङ्क्तिः_len;

		पङ्क्तिः_len = 64 * 1024;
		पङ्क्तिः = xmalloc_fgets_str_len(src_प्रवाहः, "\n", &पङ्क्तिः_len);
		यदि (!पङ्क्तिः)
			भङ्ग;
		/* Handle both Unix and MSDOS text.
		 * Note: space should not be trimmed, some encoders use it instead of "`"
		 * for padding of last incomplete 4-char block.
		 */
		str_len = पङ्क्तिः_len;
		यावत् (--str_len >= 0
		 && (पङ्क्तिः[str_len] == '\n' || पङ्क्तिः[str_len] == '\r')
		) {
			पङ्क्तिः[str_len] = '\0';
		}

		यदि (वाक्यं_तुलय(पङ्क्तिः, "end") == 0) {
			प्रत्यागम; /* the only non-error exit */
		}

		पङ्क्तिः_ptr = पङ्क्तिः;
		यावत् (*पङ्क्तिः_ptr) {
			*पङ्क्तिः_ptr = (*पङ्क्तिः_ptr - 0x20) & 0x3f;
			पङ्क्तिः_ptr++;
		}
		str_len = पङ्क्तिः_ptr - पङ्क्तिः;

		encoded_len = पङ्क्तिः[0] * 4 / 3;
		/* Check that line is not too short. (we tolerate
		 * overly _long_ line to accommodate possible extra "`").
		 * Empty line case is also caught here. */
		यदि (str_len <= encoded_len) {
			भङ्ग; /* go to bb_error_msg_and_die("short file"); */
		}
		यदि (encoded_len <= 0) {
			/* Ignore the "`\n" line, why is it even in the encode file ? */
			मुक्त(पङ्क्तिः);
			अनुवर्त;
		}
		यदि (encoded_len > 60) {
			bb_simple_error_सन्देशः_and_die("line too long");
		}

		dst = पङ्क्तिः;
		पङ्क्तिः_ptr = पङ्क्तिः + 1;
		कुरु {
			/* Merge four 6 bit chars to three 8 bit chars */
			*dst++ = पङ्क्तिः_ptr[0] << 2 | पङ्क्तिः_ptr[1] >> 4;
			encoded_len--;
			यदि (encoded_len == 0) {
				भङ्ग;
			}

			*dst++ = पङ्क्तिः_ptr[1] << 4 | पङ्क्तिः_ptr[2] >> 2;
			encoded_len--;
			यदि (encoded_len == 0) {
				भङ्ग;
			}

			*dst++ = पङ्क्तिः_ptr[2] << 6 | पङ्क्तिः_ptr[3];
			पङ्क्तिः_ptr += 4;
			encoded_len -= 2;
		} यावत् (encoded_len > 0);
		fwrite(पङ्क्तिः, 1, dst - पङ्क्तिः, dst_प्रवाहः);
		मुक्त(पङ्क्तिः);
	}
	bb_simple_error_सन्देशः_and_die("short file");
}

पूर्णाङ्क uudecode_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क uudecode_main(पूर्णाङ्क argc अप्रयुक्तम्_PARAM, वर्ण **argv)
{
	FILE *src_प्रवाहः;
	वर्ण *outname = रिक्त;
	वर्ण *पङ्क्तिः;

	getopt32(argv, "^" "o:" "\0" "?1"/* 1 arg max*/, &outname);
	argv += optind;

	यदि (!argv[0])
		*--argv = (वर्ण*)"-";
	src_प्रवाहः = xfopen_stdin(argv[0]);

	/* Search for the start of the encoding */
	यावत् ((पङ्क्तिः = xmalloc_fgetline(src_प्रवाहः)) != रिक्त) {
		शून्य FAST_FUNC (*decode_fn_ptr)(FILE *src, FILE *dst, पूर्णाङ्क flags);
		वर्ण *पङ्क्तिः_ptr;
		FILE *dst_प्रवाहः;
		पूर्णाङ्क बहुलकम्;

		यदि (is_prefixed_with(पङ्क्तिः, "begin-base64 ")) {
			पङ्क्तिः_ptr = पङ्क्तिः + 13;
			decode_fn_ptr = read_base64;
		} अन्यथा यदि (is_prefixed_with(पङ्क्तिः, "begin ")) {
			पङ्क्तिः_ptr = पङ्क्तिः + 6;
			decode_fn_ptr = read_stduu;
		} अन्यथा {
			मुक्त(पङ्क्तिः);
			अनुवर्त;
		}

		/* begin line found. decode and exit */
		बहुलकम् = bb_strtou(पङ्क्तिः_ptr, रिक्त, 8);
		यदि (outname == रिक्त) {
			outname = strchr(पङ्क्तिः_ptr, ' ');
			यदि (!outname)
				भङ्ग;
			outname++;
			trim(outname); /* remove trailing space (and '\r' for DOS text) */
			यदि (!outname[0])
				भङ्ग;
		}
		dst_प्रवाहः = stdout;
		यदि (NOT_LONE_DASH(outname)
/* https://pubs.opengroup.org/onlinepubs/9699919799/utilities/uudecode.html
 * https://pubs.opengroup.org/onlinepubs/9699919799/utilities/uuencode.html
 * The above says that output file name specified in input file
 * or overridden by -o OUTFILE can be special "/dev/stdout" string.
 * This usually works "implicitly": many systems have /dev/stdout.
 * If ENABLE_DESKTOP, support that explicitly:
 */
		 && (!ENABLE_DESKTOP || वाक्यं_तुलय(outname, "/dev/stdout") != 0)
		) {
			dst_प्रवाहः = xfopen_for_write(outname);
			fchmod(fileno(dst_प्रवाहः), बहुलकम् & (S_IRWXU | S_IRWXG | S_IRWXO));
		}
		मुक्त(पङ्क्तिः);
		decode_fn_ptr(src_प्रवाहः, dst_प्रवाहः, /*flags:*/ BASE64_FLAG_UU_STOP + BASE64_FLAG_NO_STOP_CHAR);
		/* fclose_if_not_stdin(src_stream); - redundant */
		प्रत्यागम EXIT_SUCCESS;
	}
	bb_simple_error_सन्देशः_and_die("no 'begin' line");
}
#यदि_अन्तः

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

#यदि ENABLE_BASE32 || ENABLE_BASE64

# यदि ENABLE_BASE32
स्थिर शून्य bb_b32encode(वर्ण *p, ध्रुव शून्य *src, पूर्णाङ्क दीर्घता)
{
#परिभाषय tbl bb_uuenc_tbl_base32
	ध्रुव अचिह्नित वर्ण *s = src;

	/* Transform 5x8 bits to 8x5 bits */
	यावत् (दीर्घता > 0) {
		अचिह्नित cur, अग्र;

		दीर्घता--;
		cur = *s++;
		*p++ = tbl[cur >> 3];			// xxxxx--- -------- -------- -------- --------
		cur &= 7;

		अग्र = 0;
		यदि (--दीर्घता >= 0)
			अग्र = *s++;
		*p++ = tbl[(cur << 2) + (अग्र >> 6)];	// -----xxx xx------ -------- -------- --------
		cur = अग्र & 0x3f;

		*p++ = tbl[cur >> 1];			// -------- --xxxxx- -------- -------- --------
		cur &= 1;

		अग्र = 0;
		यदि (--दीर्घता >= 0)
			अग्र = *s++;
		*p++ = tbl[(cur << 4) + (अग्र >> 4)];	// -------- -------x xxxx---- -------- --------
		cur = अग्र & 0xf;

		अग्र = 0;
		यदि (--दीर्घता >= 0)
			अग्र = *s++;
		*p++ = tbl[(cur << 1) + (अग्र >> 7)];	// -------- -------- ----xxxx x------- --------
		cur = अग्र & 0x7f;

		*p++ = tbl[cur >> 2];			// -------- -------- -------- -xxxxx-- --------
		cur &= 3;

		अग्र = 0;
		यदि (--दीर्घता >= 0)
			अग्र = *s++;
		*p++ = tbl[(cur << 3) + (अग्र >> 5)];	// -------- -------- -------- ------xx xxx-----
		cur = अग्र & 0x1f;

		*p++ = tbl[cur];			// -------- -------- -------- -------- ---xxxxx
	}
#अपरिभाषय tbl
	/* Zero-terminate */
	*p = '\0';
	/* Pad as necessary */
	दीर्घता = ((-दीर्घता) * 3) >> 1; /* -4 => 6 pad chars, -3 => 4, -2 => 3, -1 => 1 */
	यावत् (दीर्घता--) {
		*--p = '=';
	}
}
# अन्यथा
शून्य bb_b32encode(वर्ण *p, ध्रुव शून्य *src, पूर्णाङ्क दीर्घता); /* undefined */
# endif

पूर्णाङ्क आधारः_अङ्कः_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क आधारः_अङ्कः_main(पूर्णाङ्क argc अप्रयुक्तम्_PARAM, वर्ण **argv)
{
	FILE *src_प्रवाहः;
	अचिह्नित opts;
	अचिह्नित col = 76;

	opts = getopt32(argv, "^" "diw:+" "\0" "?1"/* 1 arg max*/, &col);
	argv += optind;

	यदि (!argv[0])
		*--argv = (वर्ण*)"-";
	src_प्रवाहः = xfopen_stdin(argv[0]);
	यदि (opts & 1) {
		/* -d: decode */
		पूर्णाङ्क flags = (अचिह्नित वर्ण)EOF;
		यदि (ENABLE_BASE32 && (!ENABLE_BASE64 || applet_name[4] == '3'))
			flags = ((अचिह्नित वर्ण)EOF) | BASE64_32;
		read_base64(src_प्रवाहः, stdout, flags);
	} अन्यथा {
		गणना {
			SRC_BUF_आकार = 3 * 5 * 32, /* this *MUST* be a multiple of 3 and 5 */
			DST_BUF_आकार = 8 * ((SRC_BUF_आकार + 4) / 5), /* max growth on encode (base32 case) */
		};
		/* Use one buffer for both input and output:
		 * encoding reads input "left-to-right",
		 * it's safe to place source at the end of the buffer and
		 * overwrite it while encoding, just be careful to have a gap.
		 */
		वर्ण dst_buf[((DST_BUF_आकार + /*gap:*/ 16) /*round up to 16:*/ | 0xf) + 1];
#परिभाषय src_buf (dst_buf + परिमाण(dst_buf) - SRC_BUF_आकार)
		पूर्णाङ्क src_fd, rem;

		src_fd = fileno(src_प्रवाहः);
		rem = 0;
		यावत् (1) {
			आकार_t आकार = पूर्णम्_read(src_fd, src_buf, SRC_BUF_आकार);
			यदि ((ssize_t)आकार < 0)
				bb_simple_perror_सन्देशः_and_die(bb_सन्देशः_read_error);
			यदि (आकार == 0) {
				यदि (rem != 0) bb_putchar('\n');
				भङ्ग;
			}

			/* Encode the buffer we just read in */
			यदि (ENABLE_BASE32 && (!ENABLE_BASE64 || applet_name[4] == '3')) {
				bb_b32encode(dst_buf, src_buf, आकार);
				आकार = 8 * ((आकार + 4) / 5);
			} अन्यथा {
				bb_uuencode(dst_buf, src_buf, आकार, bb_uuenc_tbl_base64);
				आकार = 4 * ((आकार + 2) / 3);
			}

			यदि (col == 0) {
				fputs_stdout(dst_buf);
			} अन्यथा {
				वर्ण *परिणाम = dst_buf;
				यदि (rem == 0)
					rem = col;
				यावत् (1) {
					पूर्णाङ्क बहिर्गमनम् = आकार < rem ? आकार : rem;
					rem -= बहिर्गमनम्;
					प्रारूप_लिख(rem != 0 ? "%.*s" : "%.*s\n", बहिर्गमनम्, परिणाम);
					यदि (rem != 0)
						भङ्ग;
					आकार -= बहिर्गमनम्;
					यदि (आकार == 0)
						भङ्ग;
					परिणाम += बहिर्गमनम्;
					rem = col;
				}
			}
		}
#अपरिभाषय src_buf
	}

	fflush_stdout_and_exit_SUCCESS();
}
#यदि_अन्तः

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
समाप्तम्
mkdir unpk_u unpk_m 2>/dev/null
for f in u_*; do
    ./uudecode <$f -o unpk_u/${f:2}
    diff -a ${f:2} unpk_u/${f:2} >/dev/null 2>&1
    echo uudecode $f: $?
समाप्तम्
for f in m_*; do
    ./uudecode <$f -o unpk_m/${f:2}
    diff -a ${f:2} unpk_m/${f:2} >/dev/null 2>&1
    echo uudecode $f: $?
समाप्तम्
*/
