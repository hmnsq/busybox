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

#समावेश "libbb.h"

#अगर ENABLE_UUDECODE
अटल व्योम FAST_FUNC read_stduu(खाता *src_stream, खाता *dst_stream, पूर्णांक flags UNUSED_PARAM)
{
	अक्षर *पंक्ति;

	क्रम (;;) {
		पूर्णांक encoded_len, str_len;
		अक्षर *पंक्ति_ptr, *dst;
		माप_प्रकार पंक्ति_len;

		पंक्ति_len = 64 * 1024;
		पंक्ति = xmalloc_fgets_str_len(src_stream, "\n", &पंक्ति_len);
		अगर (!पंक्ति)
			अवरोध;
		/* Handle both Unix and MSDOS text.
		 * Note: space should not be trimmed, some encoders use it instead of "`"
		 * for padding of last incomplete 4-char block.
		 */
		str_len = पंक्ति_len;
		जबतक (--str_len >= 0
		 && (पंक्ति[str_len] == '\n' || पंक्ति[str_len] == '\r')
		) {
			पंक्ति[str_len] = '\0';
		}

		अगर (म_भेद(पंक्ति, "end") == 0) {
			वापस; /* the only non-error exit */
		}

		पंक्ति_ptr = पंक्ति;
		जबतक (*पंक्ति_ptr) {
			*पंक्ति_ptr = (*पंक्ति_ptr - 0x20) & 0x3f;
			पंक्ति_ptr++;
		}
		str_len = पंक्ति_ptr - पंक्ति;

		encoded_len = पंक्ति[0] * 4 / 3;
		/* Check that line is not too short. (we tolerate
		 * overly _long_ line to accommodate possible extra "`").
		 * Empty line case is also caught here. */
		अगर (str_len <= encoded_len) {
			अवरोध; /* go to bb_error_msg_and_die("short file"); */
		}
		अगर (encoded_len <= 0) {
			/* Ignore the "`\n" line, why is it even in the encode file ? */
			मुक्त(पंक्ति);
			जारी;
		}
		अगर (encoded_len > 60) {
			bb_simple_error_संदेश_and_die("line too long");
		}

		dst = पंक्ति;
		पंक्ति_ptr = पंक्ति + 1;
		करो {
			/* Merge four 6 bit chars to three 8 bit chars */
			*dst++ = पंक्ति_ptr[0] << 2 | पंक्ति_ptr[1] >> 4;
			encoded_len--;
			अगर (encoded_len == 0) {
				अवरोध;
			}

			*dst++ = पंक्ति_ptr[1] << 4 | पंक्ति_ptr[2] >> 2;
			encoded_len--;
			अगर (encoded_len == 0) {
				अवरोध;
			}

			*dst++ = पंक्ति_ptr[2] << 6 | पंक्ति_ptr[3];
			पंक्ति_ptr += 4;
			encoded_len -= 2;
		} जबतक (encoded_len > 0);
		ख_डालो(पंक्ति, 1, dst - पंक्ति, dst_stream);
		मुक्त(पंक्ति);
	}
	bb_simple_error_संदेश_and_die("short file");
}

पूर्णांक uudecode_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक uudecode_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	खाता *src_stream;
	अक्षर *outname = शून्य;
	अक्षर *पंक्ति;

	getopt32(argv, "^" "o:" "\0" "?1"/* 1 arg max*/, &outname);
	argv += optind;

	अगर (!argv[0])
		*--argv = (अक्षर*)"-";
	src_stream = xfopen_stdin(argv[0]);

	/* Search for the start of the encoding */
	जबतक ((पंक्ति = xmalloc_fgetline(src_stream)) != शून्य) {
		व्योम FAST_FUNC (*decode_fn_ptr)(खाता *src, खाता *dst, पूर्णांक flags);
		अक्षर *पंक्ति_ptr;
		खाता *dst_stream;
		पूर्णांक mode;

		अगर (is_prefixed_with(पंक्ति, "begin-base64 ")) {
			पंक्ति_ptr = पंक्ति + 13;
			decode_fn_ptr = read_base64;
		} अन्यथा अगर (is_prefixed_with(पंक्ति, "begin ")) {
			पंक्ति_ptr = पंक्ति + 6;
			decode_fn_ptr = read_stduu;
		} अन्यथा {
			मुक्त(पंक्ति);
			जारी;
		}

		/* begin line found. decode and exit */
		mode = bb_strtou(पंक्ति_ptr, शून्य, 8);
		अगर (outname == शून्य) {
			outname = म_अक्षर(पंक्ति_ptr, ' ');
			अगर (!outname)
				अवरोध;
			outname++;
			trim(outname); /* remove trailing space (and '\r' for DOS text) */
			अगर (!outname[0])
				अवरोध;
		}
		dst_stream = मानक_निकास;
		अगर (NOT_LONE_DASH(outname)
/* https://pubs.opengroup.org/onlinepubs/9699919799/utilities/uudecode.html
 * https://pubs.opengroup.org/onlinepubs/9699919799/utilities/uuencode.html
 * The above says that output file name specified in input file
 * or overridden by -o OUTFILE can be special "/dev/stdout" string.
 * This usually works "implicitly": many systems have /dev/stdout.
 * If ENABLE_DESKTOP, support that explicitly:
 */
		 && (!ENABLE_DESKTOP || म_भेद(outname, "/dev/stdout") != 0)
		) {
			dst_stream = xfopen_for_write(outname);
			fchmod(fileno(dst_stream), mode & (S_IRWXU | S_IRWXG | S_IRWXO));
		}
		मुक्त(पंक्ति);
		decode_fn_ptr(src_stream, dst_stream, /*flags:*/ BASE64_FLAG_UU_STOP + BASE64_FLAG_NO_STOP_CHAR);
		/* fclose_if_not_stdin(src_stream); - redundant */
		वापस निकास_सफल;
	}
	bb_simple_error_संदेश_and_die("no 'begin' line");
}
#पूर्ण_अगर

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

#अगर ENABLE_BASE32 || ENABLE_BASE64

# अगर ENABLE_BASE32
अटल व्योम bb_b32encode(अक्षर *p, स्थिर व्योम *src, पूर्णांक लंबाई)
{
#घोषणा tbl bb_uuenc_tbl_base32
	स्थिर अचिन्हित अक्षर *s = src;

	/* Transform 5x8 bits to 8x5 bits */
	जबतक (लंबाई > 0) {
		अचिन्हित cur, अग्र;

		लंबाई--;
		cur = *s++;
		*p++ = tbl[cur >> 3];			// xxxxx--- -------- -------- -------- --------
		cur &= 7;

		अग्र = 0;
		अगर (--लंबाई >= 0)
			अग्र = *s++;
		*p++ = tbl[(cur << 2) + (अग्र >> 6)];	// -----xxx xx------ -------- -------- --------
		cur = अग्र & 0x3f;

		*p++ = tbl[cur >> 1];			// -------- --xxxxx- -------- -------- --------
		cur &= 1;

		अग्र = 0;
		अगर (--लंबाई >= 0)
			अग्र = *s++;
		*p++ = tbl[(cur << 4) + (अग्र >> 4)];	// -------- -------x xxxx---- -------- --------
		cur = अग्र & 0xf;

		अग्र = 0;
		अगर (--लंबाई >= 0)
			अग्र = *s++;
		*p++ = tbl[(cur << 1) + (अग्र >> 7)];	// -------- -------- ----xxxx x------- --------
		cur = अग्र & 0x7f;

		*p++ = tbl[cur >> 2];			// -------- -------- -------- -xxxxx-- --------
		cur &= 3;

		अग्र = 0;
		अगर (--लंबाई >= 0)
			अग्र = *s++;
		*p++ = tbl[(cur << 3) + (अग्र >> 5)];	// -------- -------- -------- ------xx xxx-----
		cur = अग्र & 0x1f;

		*p++ = tbl[cur];			// -------- -------- -------- -------- ---xxxxx
	}
#अघोषित tbl
	/* Zero-terminate */
	*p = '\0';
	/* Pad as necessary */
	लंबाई = ((-लंबाई) * 3) >> 1; /* -4 => 6 pad chars, -3 => 4, -2 => 3, -1 => 1 */
	जबतक (लंबाई--) {
		*--p = '=';
	}
}
# अन्यथा
व्योम bb_b32encode(अक्षर *p, स्थिर व्योम *src, पूर्णांक लंबाई); /* undefined */
# endif

पूर्णांक base_अंक_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक base_अंक_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	खाता *src_stream;
	अचिन्हित opts;
	अचिन्हित col = 76;

	opts = getopt32(argv, "^" "diw:+" "\0" "?1"/* 1 arg max*/, &col);
	argv += optind;

	अगर (!argv[0])
		*--argv = (अक्षर*)"-";
	src_stream = xfopen_stdin(argv[0]);
	अगर (opts & 1) {
		/* -d: decode */
		पूर्णांक flags = (अचिन्हित अक्षर)खातापूर्ण;
		अगर (ENABLE_BASE32 && (!ENABLE_BASE64 || applet_name[4] == '3'))
			flags = ((अचिन्हित अक्षर)खातापूर्ण) | BASE64_32;
		read_base64(src_stream, मानक_निकास, flags);
	} अन्यथा {
		क्रमागत {
			SRC_BUF_आकार = 3 * 5 * 32, /* this *MUST* be a multiple of 3 and 5 */
			DST_BUF_आकार = 8 * ((SRC_BUF_आकार + 4) / 5), /* max growth on encode (base32 case) */
		};
		/* Use one buffer for both input and output:
		 * encoding reads input "left-to-right",
		 * it's safe to place source at the end of the buffer and
		 * overwrite it while encoding, just be careful to have a gap.
		 */
		अक्षर dst_buf[((DST_BUF_आकार + /*gap:*/ 16) /*round up to 16:*/ | 0xf) + 1];
#घोषणा src_buf (dst_buf + माप(dst_buf) - SRC_BUF_आकार)
		पूर्णांक src_fd, rem;

		src_fd = fileno(src_stream);
		rem = 0;
		जबतक (1) {
			माप_प्रकार आकार = full_read(src_fd, src_buf, SRC_BUF_आकार);
			अगर ((ssize_t)आकार < 0)
				bb_simple_perror_संदेश_and_die(bb_संदेश_read_error);
			अगर (आकार == 0) {
				अगर (rem != 0) bb_putchar('\n');
				अवरोध;
			}

			/* Encode the buffer we just read in */
			अगर (ENABLE_BASE32 && (!ENABLE_BASE64 || applet_name[4] == '3')) {
				bb_b32encode(dst_buf, src_buf, आकार);
				आकार = 8 * ((आकार + 4) / 5);
			} अन्यथा {
				bb_uuencode(dst_buf, src_buf, आकार, bb_uuenc_tbl_base64);
				आकार = 4 * ((आकार + 2) / 3);
			}

			अगर (col == 0) {
				fputs_stdout(dst_buf);
			} अन्यथा {
				अक्षर *परिणाम = dst_buf;
				अगर (rem == 0)
					rem = col;
				जबतक (1) {
					पूर्णांक out = आकार < rem ? आकार : rem;
					rem -= out;
					म_लिखो(rem != 0 ? "%.*s" : "%.*s\n", out, परिणाम);
					अगर (rem != 0)
						अवरोध;
					आकार -= out;
					अगर (आकार == 0)
						अवरोध;
					परिणाम += out;
					rem = col;
				}
			}
		}
#अघोषित src_buf
	}

	fflush_stdout_and_exit_SUCCESS();
}
#पूर्ण_अगर

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
