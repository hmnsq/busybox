/* od -- dump files in octal and other formats
   Copyright (C) 92, 1995-2004 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
/* Written by Jim Meyering.  */
/* Busyboxed by Denys Vlasenko, based on od.c from coreutils-5.2.1 */


/* #include "libbb.h" - done in od.c */
#ضمن "common_bufsiz.h"
#تعریف_کن أكد(a) ((فراغ)0)


//usage:#if ENABLE_DESKTOP
//usage:#define od_trivial_usage
//usage:       "[-abcdfhilovxs] [-t TYPE] [-A RADIX] [-N SIZE] [-j SKIP] [-S MINSTR] [-w WIDTH] [FILE]..."
// We also support -BDOHXIL, but they are not documented in coreutils 9.1
// manpage/help, so don't show them either.
// We don't support:
// ... [FILE] [[+]OFFSET[.][b]]
// Support is buggy for:
// od --traditional [OPTION]... [FILE] [[+]OFFSET[.][b] [+][LABEL][.][b]]

//usage:#define od_full_usage "\n\n"
//usage:       "Print FILEs (or stdin) unambiguously, as octal bytes by default"
//usage:#endif

تعداد {
	OPT_A = 1 << 0,
	OPT_N = 1 << 1,
	OPT_a = 1 << 2,
	OPT_b = 1 << 3,
	OPT_c = 1 << 4,
	OPT_d = 1 << 5,
	OPT_D = 1 << 6, /* undocumented in coreutils 9.1 */
	OPT_f = 1 << 7,
	OPT_h = 1 << 8,
	OPT_H = 1 << 9, /* undocumented in coreutils 9.1 */
	OPT_i = 1 << 10,
	OPT_I = 1 << 11, /* undocumented in coreutils 9.1 */
	OPT_j = 1 << 12,
	OPT_l = 1 << 13,
	OPT_L = 1 << 14, /* undocumented in coreutils 9.1 */
	OPT_o = 1 << 15,
	OPT_O = 1 << 16, /* undocumented in coreutils 9.1 */
	OPT_B = 1 << 17, /* undocumented synonym to -o */
	OPT_t = 1 << 18,
	/* When zero and two or more consecutive blocks are equal, format
	   only the first block and output an asterisk alone on the following
	   line to indicate that identical blocks have been elided: */
	OPT_v = 1 << 19,
	OPT_x = 1 << 20,
	OPT_X = 1 << 21, /* undocumented in coreutils 9.1 */
	OPT_s = 1 << 22,
	OPT_S = 1 << 23,
	OPT_w = 1 << 24,
	OPT_traditional = (1 << 25) * ENABLE_LONG_OPTS,
};

#تعریف_کن OD_GETOPT32() getopt32long(argv, \
	"A:N:abcdDfhHiIj:lLoOBt:*vxXsS:w:+:", od_longopts, \
	/* -w with optional param */ \
	/* -S was -s and also had optional parameter */ \
	/* but in coreutils 6.3 it was renamed and now has */ \
	/* _mandatory_ parameter */ \
	&str_A, &str_N, &str_j, &lst_t, &str_S, &G.بایتها_در_هر_بلوک)


/* Check for 0x7f is a coreutils 6.3 addition */
#تعریف_کن ISPRINT(c) (((c) >= ' ') && (c) < 0x7f)

تعريفنوع دراز مزدوج longdouble_t;
تعريفنوع بیعلامت دراز دراز ulonglong_t;
تعريفنوع دراز دراز llong;

#إذا ENABLE_LFS
# define xstrtooff_sfx xstrtoull_sfx
#وإلا
# define xstrtooff_sfx xstrtoul_sfx
#پایان_اگر

/* The default number of input bytes per output line.  */
#تعریف_کن DEFAULT_بایتها_در_هر_بلوک 16

/* The number of decimal digits of precision in a float.  */
#اگر_تعریف_نشده FLT_DIG
# define FLT_DIG 7
#پایان_اگر

/* The number of decimal digits of precision in a double.  */
#اگر_تعریف_نشده DBL_DIG
# define DBL_DIG 15
#پایان_اگر

/* The number of decimal digits of precision in a long double.  */
#اگر_تعریف_نشده LDBL_DIG
# define LDBL_DIG DBL_DIG
#پایان_اگر

تعداد اندازه_spec {
	NO_اندازه,
	CHAR,
	SHORT,
	INT,
	LONG,
	LONG_LONG,
	FLOAT_SINGLE,
	FLOAT_DOUBLE,
	FLOAT_LONG_DOUBLE,
	N_اندازه_SPECS
};

تعداد output_format {
	SIGNED_DECIMAL,
	UNSIGNED_DECIMAL,
	OCTAL,
	HEXADECIMAL,
	FLOATING_نقطه,
	NAMED_CHARACTER,
	CHARACTER
};

/* Each output format specification (from '-t spec' or from
   old-style options) is represented by one of these structures.  */
بنية tspec {
	تعداد output_format fmt;
	تعداد اندازه_spec اندازه;
	فراغ (*print_function) (اندازه_t, ثابت حرف *, ثابت حرف *);
	حرف *fmt_string;
	صحيح hexl_نما_trailer;
	صحيح فیلد_عرض;
};

/* Convert the number of 8-bit bytes of a binary representation to
   the number of characters (digits + sign if the type is signed)
   required to represent the same quantity in the specified base/type.
   For example, a 32-bit (4-byte) quantity may require a field width
   as wide as the following for these types:
   11	unsigned octal
   11	signed decimal
   10	unsigned decimal
   8	unsigned hexadecimal  */

ساكن ثابت uint8_t بایتها_مقصد_oct_digits[] ALIGN1 =
{0, 3, 6, 8, 11, 14, 16, 19, 22, 25, 27, 30, 32, 35, 38, 41, 43};

ساكن ثابت uint8_t بایتها_مقصد_signed_dec_digits[] ALIGN1 =
{1, 4, 6, 8, 11, 13, 16, 18, 20, 23, 25, 28, 30, 33, 35, 37, 40};

ساكن ثابت uint8_t بایتها_مقصد_unsigned_dec_digits[] ALIGN1 =
{0, 3, 5, 8, 10, 13, 15, 17, 20, 22, 25, 27, 29, 32, 34, 37, 39};

ساكن ثابت uint8_t بایتها_مقصد_hex_digits[] ALIGN1 =
{0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32};

/* Convert enum size_spec to the size of the named type.  */
ساكن ثابت علامتدار حرف عرض_بایتها[] ALIGN1 = {
	-1,
	حجم(حرف),
	حجم(کوتاه),
	حجم(صحيح),
	حجم(دراز),
	حجم(ulonglong_t),
	حجم(عائم),
	حجم(مزدوج),
	حجم(longdouble_t)
};
/* Ensure that for each member of 'enum size_spec' there is an
   initializer in the width_bytes array.  */
بنية ERR_عرض_بایتها_has_bad_اندازه {
	حرف ERR_عرض_بایتها_has_bad_اندازه[ARRAY_اندازه(عرض_بایتها) == N_اندازه_SPECS ? 1 : -1];
};

بنية globals {
	smallint exit_کد;

	بیعلامت string_min;

	/* An array of specs describing how to format each input block.  */
	بیعلامت n_specs;
	بنية tspec *spec;

	/* Function that accepts an address and an optional following char,
	   and prints the address and char to stdout.  */
	فراغ (*format_آدرس)(off_t, حرف);

	/* The difference between the old-style pseudo starting address and
	   the number of bytes to skip.  */
#إذا ENABLE_LONG_OPTS
	off_t pseudo_جابه_جایی;
# define G_pseudo_جابه_جایی G.pseudo_جابه_جایی
#پایان_اگر
	/* When zero, MAX_BYTES_TO_FORMAT and END_OFFSET are ignored, and all
	   input is formatted.  */

	/* The number of input bytes formatted per output line.  It must be
	   a multiple of the least common multiple of the sizes associated with
	   the specified output types.  It should be as large as possible, but
	   no larger than 16 -- unless specified with the -w option.  */
	بیعلامت بایتها_در_هر_بلوک; /* have to use unsigned, not size_t */

	/* A NULL-terminated list of the file-arguments from the command line.  */
	ثابت حرف *ثابت *file_list;

	/* The input stream associated with the current file.  */
	FILE *درون_جریان;

	bool not_اولی;
	bool prev_pair_equal;

	حرف آدرس_fmt[حجم("%0n"OFF_FMT"xc")];
} FIX_ALIASING;
/* Corresponds to 'x' above */
#تعریف_کن آدرس_پایه_char G.آدرس_fmt[حجم(G.آدرس_fmt)-3]
/* Corresponds to 'n' above */
#تعریف_کن آدرس_pad_len_char G.آدرس_fmt[2]

#إذا !ENABLE_LONG_OPTS
تعداد { G_pseudo_جابه_جایی = 0 };
#پایان_اگر
#تعریف_کن G (*(بنية globals*)bb_common_bufsiz1)
#تعریف_کن INIT_G() افعل { \
	راهاندازی_common_bufsiz(); \
	BUILD_BUG_ON(حجم(G) > COMMON_BUFSIZE); \
	G.بایتها_در_هر_بلوک = 32; \
	strcpy(G.آدرس_fmt, "%0n"OFF_FMT"xc"); \
} طالما (0)


#تعریف_کن MAX_INTEGRAL_TYPE_اندازه حجم(ulonglong_t)
ساكن ثابت بیعلامت حرف integral_type_اندازه[MAX_INTEGRAL_TYPE_اندازه + 1] ALIGN1 = {
	[حجم(حرف)] = CHAR,
#إذا USHRT_MAX != UCHAR_MAX
	[حجم(کوتاه)] = SHORT,
#پایان_اگر
#إذا UINT_MAX != USHRT_MAX
	[حجم(صحيح)] = INT,
#پایان_اگر
#إذا ULONG_MAX != UINT_MAX
	[حجم(دراز)] = LONG,
#پایان_اگر
#إذا ULLONG_MAX != ULONG_MAX
	[حجم(ulonglong_t)] = LONG_LONG,
#پایان_اگر
};

#تعریف_کن MAX_FP_TYPE_اندازه حجم(longdouble_t)
ساكن ثابت بیعلامت حرف fp_type_اندازه[MAX_FP_TYPE_اندازه + 1] ALIGN1 = {
	/* gcc seems to allow repeated indexes. Last one wins */
	[حجم(longdouble_t)] = FLOAT_LONG_DOUBLE,
	[حجم(مزدوج)] = FLOAT_DOUBLE,
	[حجم(عائم)] = FLOAT_SINGLE
};


ساكن بیعلامت
بزرگترین_مقسوم_علیه(بیعلامت u, بیعلامت v)
{
	بیعلامت t;
	طالما (v != 0) {
		t = u % v;
		u = v;
		v = t;
	}
	أرجع u;
}

/* Compute the least common multiple of U and V.  */
ساكن بیعلامت
lcm(بیعلامت u, بیعلامت v) {
	بیعلامت t = بزرگترین_مقسوم_علیه(u, v);
	إذا (t == 0)
		أرجع 0;
	أرجع u * v / t;
}

ساكن فراغ
print_s_char(اندازه_t n_بایتها, ثابت حرف *بلوک, ثابت حرف *fmt_string)
{
	طالما (n_بایتها--) {
		صحيح tmp = *(علامتدار حرف *) بلوک;
		چاپ_کن(fmt_string, tmp);
		بلوک += حجم(بیعلامت حرف);
	}
}

ساكن فراغ
print_char(اندازه_t n_بایتها, ثابت حرف *بلوک, ثابت حرف *fmt_string)
{
	طالما (n_بایتها--) {
		بیعلامت tmp = *(بیعلامت حرف *) بلوک;
		چاپ_کن(fmt_string, tmp);
		بلوک += حجم(بیعلامت حرف);
	}
}

ساكن فراغ
print_s_short(اندازه_t n_بایتها, ثابت حرف *بلوک, ثابت حرف *fmt_string)
{
	n_بایتها /= حجم(علامتدار کوتاه);
	طالما (n_بایتها--) {
		صحيح tmp = *(علامتدار کوتاه *) بلوک;
		چاپ_کن(fmt_string, tmp);
		بلوک += حجم(بیعلامت کوتاه);
	}
}

ساكن فراغ
print_short(اندازه_t n_بایتها, ثابت حرف *بلوک, ثابت حرف *fmt_string)
{
	n_بایتها /= حجم(بیعلامت کوتاه);
	طالما (n_بایتها--) {
		بیعلامت tmp = *(بیعلامت کوتاه *) بلوک;
		چاپ_کن(fmt_string, tmp);
		بلوک += حجم(بیعلامت کوتاه);
	}
}

ساكن فراغ
print_int(اندازه_t n_بایتها, ثابت حرف *بلوک, ثابت حرف *fmt_string)
{
	n_بایتها /= حجم(بیعلامت);
	طالما (n_بایتها--) {
		بیعلامت tmp = *(بیعلامت *) بلوک;
		چاپ_کن(fmt_string, tmp);
		بلوک += حجم(بیعلامت);
	}
}

#إذا UINT_MAX == ULONG_MAX
# define print_long print_int
#وإلا
ساكن فراغ
print_long(اندازه_t n_بایتها, ثابت حرف *بلوک, ثابت حرف *fmt_string)
{
	n_بایتها /= حجم(بیعلامت دراز);
	طالما (n_بایتها--) {
		بیعلامت دراز tmp = *(بیعلامت دراز *) بلوک;
		چاپ_کن(fmt_string, tmp);
		بلوک += حجم(بیعلامت دراز);
	}
}
#پایان_اگر

#إذا ULONG_MAX == ULLONG_MAX
# define print_long_long print_long
#وإلا
ساكن فراغ
print_long_long(اندازه_t n_بایتها, ثابت حرف *بلوک, ثابت حرف *fmt_string)
{
	n_بایتها /= حجم(ulonglong_t);
	طالما (n_بایتها--) {
		ulonglong_t tmp = *(ulonglong_t *) بلوک;
		چاپ_کن(fmt_string, tmp);
		بلوک += حجم(ulonglong_t);
	}
}
#پایان_اگر

ساكن فراغ
print_float(اندازه_t n_بایتها, ثابت حرف *بلوک, ثابت حرف *fmt_string)
{
	n_بایتها /= حجم(عائم);
	طالما (n_بایتها--) {
		عائم tmp = *(عائم *) بلوک;
		چاپ_کن(fmt_string, tmp);
		بلوک += حجم(عائم);
	}
}

ساكن فراغ
print_double(اندازه_t n_بایتها, ثابت حرف *بلوک, ثابت حرف *fmt_string)
{
	n_بایتها /= حجم(مزدوج);
	طالما (n_بایتها--) {
		مزدوج tmp = *(مزدوج *) بلوک;
		چاپ_کن(fmt_string, tmp);
		بلوک += حجم(مزدوج);
	}
}

ساكن فراغ
print_long_double(اندازه_t n_بایتها, ثابت حرف *بلوک, ثابت حرف *fmt_string)
{
	n_بایتها /= حجم(longdouble_t);
	طالما (n_بایتها--) {
		longdouble_t tmp = *(longdouble_t *) بلوک;
		چاپ_کن(fmt_string, tmp);
		بلوک += حجم(longdouble_t);
	}
}

/* print_[named]_ascii are optimized for speed.
 * Remember, someday you may want to pump gigabytes through this thing.
 * Saving a dozen of .text bytes here is counter-productive */

ساكن فراغ
print_named_ascii(اندازه_t n_بایتها, ثابت حرف *بلوک,
		ثابت حرف *استفادهنشده_fmt_string استفادهنشده_PARAM)
{
	/* Names for some non-printing characters.  */
	ساكن ثابت حرف charname[33][3] ALIGN1 = {
		"nul", "soh", "stx", "etx", "eot", "enq", "ack", "bel",
		" bs", " ht", " nl", " vt", " ff", " cr", " so", " si",
		"dle", "dc1", "dc2", "dc3", "dc4", "nak", "syn", "etb",
		"can", " em", "sub", "esc", " fs", " gs", " rs", " us",
		" sp"
	};
	// buf[N] pos:  01234 56789
	حرف buf[12] = "   x\0 xxx\0";
	// [12] because we take three 32bit stack slots anyway, and
	// gcc is too dumb to initialize with constant stores,
	// it copies initializer from rodata. Oh well.
	// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=65410

	طالما (n_بایتها--) {
		بیعلامت masked_c = *(بیعلامت حرف *) بلوک++;

		masked_c &= 0x7f;
		إذا (masked_c == 0x7f) {
			fputs_stdout(" del");
			استمر;
		}
		إذا (masked_c > ' ') {
			buf[3] = masked_c;
			fputs_stdout(buf);
			استمر;
		}
		/* Why? Because printf(" %3.3s") is much slower... */
		buf[6] = charname[masked_c][0];
		buf[7] = charname[masked_c][1];
		buf[8] = charname[masked_c][2];
		fputs_stdout(buf+5);
	}
}

ساكن فراغ
print_ascii(اندازه_t n_بایتها, ثابت حرف *بلوک,
		ثابت حرف *استفادهنشده_fmt_string استفادهنشده_PARAM)
{
	// buf[N] pos:  01234 56789
	حرف buf[12] = "   x\0 xxx\0";

	طالما (n_بایتها--) {
		ثابت حرف *s;
		بیعلامت c = *(بیعلامت حرف *) بلوک++;

		إذا (ISPRINT(c)) {
			buf[3] = c;
			fputs_stdout(buf);
			استمر;
		}
		بدل (c) {
		حالة '\0':
			s = "  \\0";
			اكسر;
		حالة '\007':
			s = "  \\a";
			اكسر;
		حالة '\b':
			s = "  \\b";
			اكسر;
		حالة '\f':
			s = "  \\f";
			اكسر;
		حالة '\n':
			s = "  \\n";
			اكسر;
		حالة '\r':
			s = "  \\r";
			اكسر;
		حالة '\t':
			s = "  \\t";
			اكسر;
		حالة '\v':
			s = "  \\v";
			اكسر;
		افتراضي:
			buf[6] = (c >> 6 & 3) + '0';
			buf[7] = (c >> 3 & 7) + '0';
			buf[8] = (c & 7) + '0';
			s = buf + 5;
		}
		fputs_stdout(s);
	}
}

/* Given a list of one or more input filenames FILE_LIST, set the global
   file pointer IN_STREAM and the global string INPUT_FILENAME to the
   first one that can be successfully opened. Modify FILE_LIST to
   reference the next filename in the list.  A file name of "-" is
   interpreted as standard input.  If any file open fails, give an error
   message and return nonzero.  */

ساكن فراغ
open_بعدی_file(فراغ)
{
	طالما (1) {
		إذا (!*G.file_list)
			أرجع;
		G.درون_جریان = fopen_or_warn_stdin(*G.file_list++);
		إذا (G.درون_جریان) {
			اكسر;
		}
		G.exit_کد = 1;
	}

	إذا ((option_mask32 & (OPT_N|OPT_S)) == OPT_N)
		setbuf(G.درون_جریان, عدم);
}

/* Test whether there have been errors on in_stream, and close it if
   it is not standard input.  Return nonzero if there has been an error
   on in_stream or stdout; return zero otherwise.  This function will
   report more than one error only if both a read and a write error
   have occurred.  IN_ERRNO, if nonzero, is the error number
   corresponding to the most recent action for IN_STREAM.  */

ساكن فراغ
check_and_close(فراغ)
{
	إذا (G.درون_جریان) {
		إذا (ferror(G.درون_جریان))	{
			bb_error_پیام("%s: read error", (G.درون_جریان == stdin)
					? bb_پیام_standard_درون_داد
					: G.file_list[-1]
			);
			G.exit_کد = 1;
		}
		fclose_if_not_stdin(G.درون_جریان);
		G.درون_جریان = عدم;
	}

	إذا (ferror(stdout)) {
		bb_simple_error_پیام_and_die(bb_پیام_write_error);
	}
}

/* If S points to a single valid modern od format string, put
   a description of that format in *TSPEC, return pointer to
   character following the just-decoded format.
   For example, if S were "d4afL", we will return a rtp to "afL"
   and *TSPEC would be
	{
		fmt = SIGNED_DECIMAL;
		size = INT or LONG; (whichever integral_type_size[4] resolves to)
		print_function = print_int; (assuming size == INT)
		fmt_string = "%011d%c";
	}
   S_ORIG is solely for reporting errors.  It should be the full format
   string argument. */

ساكن NOINLINE ثابت حرف *
decode_one_format(ثابت حرف *s_orig, ثابت حرف *s, بنية tspec *tspec)
{
	تعداد اندازه_spec اندازه_spec;
	بیعلامت اندازه;
	تعداد output_format fmt;
	ثابت حرف *p;
	حرف *انتها;
	حرف *fmt_string = عدم;
	فراغ (*print_function) (اندازه_t, ثابت حرف *, ثابت حرف *);
	بیعلامت c;
	بیعلامت فیلد_عرض = 0;
	صحيح pos;

	بدل (*s) {
	حالة 'd':
	حالة 'o':
	حالة 'u':
	حالة 'x': {
		ساكن ثابت حرف CSIL[] ALIGN1 = "CSIL";

		c = *s++;
		p = strchr(CSIL, *s);
		/* if *s == NUL, p != NULL! Testcase: "od -tx" */
		إذا (!p || *p == '\0') {
			اندازه = حجم(صحيح);
			إذا (آیا_رقم(s[0])) {
				اندازه = bb_strtou(s, &انتها, 0);
				إذا (errno == ERANGE
				 || MAX_INTEGRAL_TYPE_اندازه < اندازه
				 || integral_type_اندازه[اندازه] == NO_اندازه
				) {
					bb_error_پیام_and_die("invalid type string '%s'; "
						"%u-byte %s type is not supported",
						s_orig, اندازه, "integral");
				}
				s = انتها;
			}
		} وإلا {
			ساكن ثابت uint8_t CSIL_sizeof[4] = {
				حجم(حرف),
				حجم(کوتاه),
				حجم(صحيح),
				حجم(دراز),
			};
			اندازه = CSIL_sizeof[p - CSIL];
			s++; /* skip C/S/I/L */
		}

#تعریف_کن ISPEC_مقصد_FORMAT(Spec, Min_format, Long_format, Max_format) \
	((Spec) == LONG_LONG ? (Max_format) \
	: ((Spec) == LONG ? (Long_format) : (Min_format)))

#تعریف_کن FMT_بایتها_ALLOCATED 9
		اندازه_spec = integral_type_اندازه[اندازه];

		{
			ساكن ثابت حرف doux[] ALIGN1 = "doux";
			ساكن ثابت حرف doux_fmt_letter[][4] = {
				"lld", "llo", "llu", "llx"
			};
			ساكن ثابت تعداد output_format doux_fmt[] = {
				SIGNED_DECIMAL,
				OCTAL,
				UNSIGNED_DECIMAL,
				HEXADECIMAL,
			};
			ساكن ثابت uint8_t *ثابت doux_بایتها_مقصد_XXX[] = {
				بایتها_مقصد_signed_dec_digits,
				بایتها_مقصد_oct_digits,
				بایتها_مقصد_unsigned_dec_digits,
				بایتها_مقصد_hex_digits,
			};
			ساكن ثابت حرف doux_fmtstring[][حجم(" %%0%u%s")] ALIGN1 = {
				" %%%u%s",
				" %%0%u%s",
				" %%%u%s",
				" %%0%u%s",
			};

			pos = strchr(doux, c) - doux;
			fmt = doux_fmt[pos];
			فیلد_عرض = doux_بایتها_مقصد_XXX[pos][اندازه];
			p = doux_fmt_letter[pos] + 2;
			إذا (اندازه_spec == LONG) p--;
			إذا (اندازه_spec == LONG_LONG) p -= 2;
			fmt_string = xasprintf(doux_fmtstring[pos], فیلد_عرض, p);
		}

		بدل (اندازه_spec) {
		حالة CHAR:
			print_function = (fmt == SIGNED_DECIMAL
				    ? print_s_char
				    : print_char);
			اكسر;
		حالة SHORT:
			print_function = (fmt == SIGNED_DECIMAL
				    ? print_s_short
				    : print_short);
			اكسر;
		حالة INT:
			print_function = print_int;
			اكسر;
		حالة LONG:
			print_function = print_long;
			اكسر;
		افتراضي: /* case LONG_LONG: */
			print_function = print_long_long;
			اكسر;
		}
		اكسر;
	}

	حالة 'f': {
		ساكن ثابت حرف FDL[] ALIGN1 = "FDL";

		fmt = FLOATING_نقطه;
		++s;
		p = strchr(FDL, *s);
		إذا (!p || *p == '\0') {
			اندازه = حجم(مزدوج);
			إذا (آیا_رقم(s[0])) {
				اندازه = bb_strtou(s, &انتها, 0);
				إذا (errno == ERANGE || اندازه > MAX_FP_TYPE_اندازه
				 || fp_type_اندازه[اندازه] == NO_اندازه
				) {
					bb_error_پیام_and_die("invalid type string '%s'; "
						"%u-byte %s type is not supported",
						s_orig, اندازه, "floating point");
				}
				s = انتها;
			}
		} وإلا {
			ساكن ثابت uint8_t FDL_sizeof[] = {
				حجم(عائم),
				حجم(مزدوج),
				حجم(longdouble_t),
			};

			اندازه = FDL_sizeof[p - FDL];
			s++; /* skip F/D/L */
		}

		اندازه_spec = fp_type_اندازه[اندازه];

		بدل (اندازه_spec) {
		حالة FLOAT_SINGLE:
			print_function = print_float;
			فیلد_عرض = FLT_DIG + 8;
			/* Don't use %#e; not all systems support it.  */
			fmt_string = xasprintf(" %%%d.%de", فیلد_عرض, FLT_DIG);
			اكسر;
		حالة FLOAT_DOUBLE:
			print_function = print_double;
			فیلد_عرض = DBL_DIG + 8;
			fmt_string = xasprintf(" %%%d.%de", فیلد_عرض, DBL_DIG);
			اكسر;
		افتراضي: /* case FLOAT_LONG_DOUBLE: */
			print_function = print_long_double;
			فیلد_عرض = LDBL_DIG + 8;
			fmt_string = xasprintf(" %%%d.%dLe", فیلد_عرض, LDBL_DIG);
			اكسر;
		}
		اكسر;
	}

	حالة 'a':
		++s;
		fmt = NAMED_CHARACTER;
		اندازه_spec = CHAR;
		print_function = print_named_ascii;
		فیلد_عرض = 3;
		اكسر;
	حالة 'c':
		++s;
		fmt = CHARACTER;
		اندازه_spec = CHAR;
		print_function = print_ascii;
		فیلد_عرض = 3;
		اكسر;
	افتراضي:
		bb_error_پیام_and_die("invalid character '%c' "
				"in type string '%s'", *s, s_orig);
	}

	tspec->اندازه = اندازه_spec;
	tspec->fmt = fmt;
	tspec->print_function = print_function;
	tspec->fmt_string = fmt_string;

	tspec->فیلد_عرض = فیلد_عرض;
	tspec->hexl_نما_trailer = (*s == 'z');
	إذا (tspec->hexl_نما_trailer)
		s++;

	أرجع s;
}

/* Decode the modern od format string S.  Append the decoded
   representation to the global array SPEC, reallocating SPEC if
   necessary.  */

ساكن فراغ
decode_format_string(ثابت حرف *s)
{
	ثابت حرف *s_orig = s;

	طالما (*s != '\0') {
		بنية tspec tspec;
		ثابت حرف *بعدی;

		بعدی = decode_one_format(s_orig, s, &tspec);

		أكد(s != بعدی);
		s = بعدی;
		G.spec = xrealloc_وکتور(G.spec, 4, G.n_specs);
		کپی_حافظه(&G.spec[G.n_specs], &tspec, حجم(G.spec[0]));
		G.n_specs++;
	}
}

/* Given a list of one or more input filenames FILE_LIST, set the global
   file pointer IN_STREAM to position N_SKIP in the concatenation of
   those files.  If any file operation fails or if there are fewer than
   N_SKIP bytes in the combined input, give an error message and return
   nonzero.  When possible, use seek rather than read operations to
   advance IN_STREAM.  */

ساكن فراغ
skip(off_t n_skip)
{
	إذا (n_skip == 0)
		أرجع;

	طالما (G.درون_جریان) { /* !EOF */
		بنية stat file_stats;

		/* First try seeking.  For large offsets, this extra work is
		   worthwhile.  If the offset is below some threshold it may be
		   more efficient to move the pointer by reading.  There are two
		   issues when trying to seek:
			- the file must be seekable.
			- before seeking to the specified position, make sure
			  that the new position is in the current file.
			  Try to do that by getting file's size using fstat.
			  But that will work only for regular files.  */

			/* The st_size field is valid only for regular files
			   (and for symbolic links, which cannot occur here).
			   If the number of bytes left to skip is at least
			   as large as the size of the current file, we can
			   decrement n_skip and go on to the next file.  */
		إذا (fstat(fileno(G.درون_جریان), &file_stats) == 0
		 && S_ISREG(file_stats.st_نما) && file_stats.st_اندازه > 0
		) {
			إذا (file_stats.st_اندازه < n_skip) {
				n_skip -= file_stats.st_اندازه;
				/* take "check & close / open_next" route */
			} وإلا {
				إذا (fseeko(G.درون_جریان, n_skip, SEEK_CUR) != 0)
					G.exit_کد = 1;
				أرجع;
			}
		} وإلا {
			/* If it's not a regular file with positive size,
			   position the file pointer by reading.  */
			حرف buf[1024];
			اندازه_t n_بایتها_مقصد_read = 1024;
			اندازه_t n_بایتها_read;

			طالما (n_skip > 0) {
				إذا (n_skip < n_بایتها_مقصد_read)
					n_بایتها_مقصد_read = n_skip;
				n_بایتها_read = fread(buf, 1, n_بایتها_مقصد_read, G.درون_جریان);
				n_skip -= n_بایتها_read;
				إذا (n_بایتها_read != n_بایتها_مقصد_read)
					اكسر; /* EOF on this file or error */
			}
		}
		إذا (n_skip == 0)
			أرجع;

		check_and_close();
		open_بعدی_file();
	}

	إذا (n_skip)
		bb_simple_error_پیام_and_die("can't skip past end of combined input");
}


تعريفنوع فراغ FN_format_آدرس(off_t آدرس, حرف c);

ساكن فراغ
format_آدرس_none(off_t آدرس استفادهنشده_PARAM, حرف c استفادهنشده_PARAM)
{
}

ساكن فراغ
format_آدرس_std(off_t آدرس, حرف c)
{
	/* Corresponds to 'c' */
	G.آدرس_fmt[حجم(G.آدرس_fmt)-2] = c;
	چاپ_کن(G.آدرس_fmt, آدرس);
}

#إذا ENABLE_LONG_OPTS
/* only used with --traditional */
ساكن فراغ
format_آدرس_paren(off_t آدرس, حرف c)
{
	حرف_چاپ_کن('(');
	format_آدرس_std(آدرس, ')');
	إذا (c) حرف_چاپ_کن(c);
}

ساكن فراغ
format_آدرس_label(off_t آدرس, حرف c)
{
	format_آدرس_std(آدرس, ' ');
	format_آدرس_paren(آدرس + G_pseudo_جابه_جایی, c);
}
#پایان_اگر

ساكن فراغ
dump_hexl_نما_trailer(اندازه_t n_بایتها, ثابت حرف *بلوک)
{
	fputs_stdout("  >");
	طالما (n_بایتها--) {
		بیعلامت c = *(بیعلامت حرف *) بلوک++;
		c = (ISPRINT(c) ? c : '.');
		حرف_چاپ_کن(c);
	}
	حرف_چاپ_کن('<');
}

/* Write N_BYTES bytes from CURR_BLOCK to standard output once for each
   of the N_SPEC format specs.  CURRENT_OFFSET is the byte address of
   CURR_BLOCK in the concatenation of input files, and it is printed
   (optionally) only before the output line associated with the first
   format spec.  When duplicate blocks are being abbreviated, the output
   for a sequence of identical input blocks is the output for the first
   block followed by an asterisk alone on a line.  It is valid to compare
   the blocks PREV_BLOCK and CURR_BLOCK only when N_BYTES == BYTES_PER_BLOCK.
   That condition may be false only for the last input block -- and then
   only when it has not been padded to length BYTES_PER_BLOCK.  */

ساكن فراغ
write_بلوک(off_t current_جابه_جایی, اندازه_t n_بایتها,
		ثابت حرف *prev_بلوک, ثابت حرف *curr_بلوک)
{
	بیعلامت i;

	إذا (!(option_mask32 & OPT_v)
	 && G.not_اولی
	 && n_بایتها == G.بایتها_در_هر_بلوک
	 && memcmp(prev_بلوک, curr_بلوک, G.بایتها_در_هر_بلوک) == 0
	) {
		إذا (G.prev_pair_equal) {
			/* The two preceding blocks were equal, and the current
			   block is the same as the last one, so print nothing.  */
		} وإلا {
			puts("*");
			G.prev_pair_equal = 1;
		}
	} وإلا {
		G.not_اولی = 1;
		G.prev_pair_equal = 0;
		لكل (i = 0; i < G.n_specs; i++) {
			إذا (i == 0)
				G.format_آدرس(current_جابه_جایی, '\0');
			وإلا
				چاپ_کن("%*s", آدرس_pad_len_char - '0', "");
			(*G.spec[i].print_function) (n_بایتها, curr_بلوک, G.spec[i].fmt_string);
			إذا (G.spec[i].hexl_نما_trailer) {
				/* space-pad out to full line width, then dump the trailer */
				بیعلامت datum_عرض = عرض_بایتها[G.spec[i].اندازه];
				بیعلامت blank_fields = (G.بایتها_در_هر_بلوک - n_بایتها) / datum_عرض;
				بیعلامت فیلد_عرض = G.spec[i].فیلد_عرض + 1;
				چاپ_کن("%*s", blank_fields * فیلد_عرض, "");
				dump_hexl_نما_trailer(n_بایتها, curr_بلوک);
			}
			حرف_چاپ_کن('\n');
		}
	}
}

ساكن فراغ
read_بلوک(اندازه_t n, حرف *بلوک, اندازه_t *n_بایتها_درون_میانگیر)
{
	أكد(0 < n && n <= G.بایتها_در_هر_بلوک);

	*n_بایتها_درون_میانگیر = 0;

	إذا (n == 0)
		أرجع;

	طالما (G.درون_جریان != عدم) { /* EOF.  */
		اندازه_t n_needed;
		اندازه_t n_read;

		n_needed = n - *n_بایتها_درون_میانگیر;
		n_read = fread(بلوک + *n_بایتها_درون_میانگیر, 1, n_needed, G.درون_جریان);
		*n_بایتها_درون_میانگیر += n_read;
		إذا (n_read == n_needed)
			اكسر;
		/* error check is done in check_and_close */
		check_and_close();
		open_بعدی_file();
	}
}

/* Return the least common multiple of the sizes associated
   with the format specs.  */

ساكن صحيح
get_lcm(فراغ)
{
	اندازه_t i;
	صحيح l_c_m = 1;

	لكل (i = 0; i < G.n_specs; i++)
		l_c_m = lcm(l_c_m, عرض_بایتها[(صحيح) G.spec[i].اندازه]);
	أرجع l_c_m;
}

/* Read a chunk of size BYTES_PER_BLOCK from the input files, write the
   formatted block to standard output, and repeat until the specified
   maximum number of bytes has been read or until all input has been
   processed.  If the last block read is smaller than BYTES_PER_BLOCK
   and its size is not a multiple of the size associated with a format
   spec, extend the input block with zero bytes until its length is a
   multiple of all format spec sizes.  Write the final block.  Finally,
   write on a line by itself the offset of the byte after the last byte
   read.  */

ساكن فراغ
dump(off_t current_جابه_جایی, off_t انتها_جابه_جایی)
{
	حرف *بلوک[2];
	صحيح idx;
	اندازه_t n_بایتها_read;

	بلوک[0] = xmalloc(2 * G.بایتها_در_هر_بلوک);
	بلوک[1] = بلوک[0] + G.بایتها_در_هر_بلوک;

	idx = 0;
	إذا (option_mask32 & OPT_N) {
		طالما (1) {
			اندازه_t n_needed;
			إذا (current_جابه_جایی >= انتها_جابه_جایی) {
				n_بایتها_read = 0;
				اكسر;
			}
			n_needed = MIN(انتها_جابه_جایی - current_جابه_جایی, (off_t) G.بایتها_در_هر_بلوک);
			read_بلوک(n_needed, بلوک[idx], &n_بایتها_read);
			إذا (n_بایتها_read < G.بایتها_در_هر_بلوک)
				اكسر;
			أكد(n_بایتها_read == G.بایتها_در_هر_بلوک);
			write_بلوک(current_جابه_جایی, n_بایتها_read, بلوک[idx ^ 1], بلوک[idx]);
			current_جابه_جایی += n_بایتها_read;
			idx ^= 1;
		}
	} وإلا {
		طالما (1) {
			read_بلوک(G.بایتها_در_هر_بلوک, بلوک[idx], &n_بایتها_read);
			إذا (n_بایتها_read < G.بایتها_در_هر_بلوک)
				اكسر;
			أكد(n_بایتها_read == G.بایتها_در_هر_بلوک);
			write_بلوک(current_جابه_جایی, n_بایتها_read, بلوک[idx ^ 1], بلوک[idx]);
			current_جابه_جایی += n_بایتها_read;
			idx ^= 1;
		}
	}

	إذا (n_بایتها_read > 0) {
		صحيح l_c_m;
		اندازه_t بایتها_مقصد_write;

		l_c_m = get_lcm();

		/* Make bytes_to_write the smallest multiple of l_c_m that
		   is at least as large as n_bytes_read.  */
		بایتها_مقصد_write = l_c_m * ((n_بایتها_read + l_c_m - 1) / l_c_m);

		memset(بلوک[idx] + n_بایتها_read, 0, بایتها_مقصد_write - n_بایتها_read);
		write_بلوک(current_جابه_جایی, بایتها_مقصد_write,
				بلوک[idx ^ 1], بلوک[idx]);
		current_جابه_جایی += n_بایتها_read;
	}

	G.format_آدرس(current_جابه_جایی, '\n');

	إذا ((option_mask32 & OPT_N) && current_جابه_جایی >= انتها_جابه_جایی)
		check_and_close();

	حرر(بلوک[0]);
}

/* Read N bytes into BLOCK from the concatenation of the input files
   named in the global array FILE_LIST.  On the first call to this
   function, the global variable IN_STREAM is expected to be an open
   stream associated with the input file INPUT_FILENAME.  If all N
   bytes cannot be read from IN_STREAM, close IN_STREAM and update
   the global variables IN_STREAM and INPUT_FILENAME.  Then try to
   read the remaining bytes from the newly opened file.  Repeat if
   necessary until EOF is reached for the last file in FILE_LIST.
   On subsequent calls, don't modify BLOCK and return zero.  Set
   *N_BYTES_IN_BUFFER to the number of bytes read.  If an error occurs,
   it will be detected through ferror when the stream is about to be
   closed.  If there is an error, give a message but continue reading
   as usual and return nonzero.  Otherwise return zero.  */

/* STRINGS mode.  Find each "string constant" in the input.
   A string constant is a run of at least 'string_min' ASCII
   graphic (or formatting) characters terminated by a null.
   Based on a function written by Richard Stallman for a
   traditional version of od.  */

ساكن فراغ
dump_رشتهها(off_t آدرس, off_t انتها_جابه_جایی)
{
	بیعلامت bufsize = MAX(100, G.string_min);
	بیعلامت حرف *buf = xmalloc(bufsize);

	طالما (1) {
		اندازه_t i;
		صحيح c;

		/* See if the next 'G.string_min' chars are all printing chars.  */
 tryline:
		إذا ((option_mask32 & OPT_N) && (انتها_جابه_جایی - G.string_min <= آدرس))
			اكسر;
		i = 0;
		طالما (!(option_mask32 & OPT_N) || آدرس < انتها_جابه_جایی) {
			إذا (i == bufsize) {
				bufsize += bufsize/8;
				buf = xrealloc(buf, bufsize);
			}

			طالما (G.درون_جریان) { /* !EOF */
				c = fgetc(G.درون_جریان);
				إذا (c != EOF)
					اذهب got_char;
				check_and_close();
				open_بعدی_file();
			}
			/* EOF */
			اذهب ret;
 got_char:
			آدرس++;
			إذا (!c)
				اكسر;
			إذا (!ISPRINT(c))
				اذهب tryline;	/* It isn't; give up on this string.  */
			buf[i++] = c;		/* String continues; store it all.  */
		}

		إذا (i < G.string_min)		/* Too short! */
			اذهب tryline;

		/* If we get here, the string is all printable and NUL-terminated */
		buf[i] = 0;
		G.format_آدرس(آدرس - i - 1, ' ');

		لكل (i = 0; (c = buf[i]); i++) {
			بدل (c) {
			حالة '\007': fputs_stdout("\\a"); اكسر;
			حالة '\b': fputs_stdout("\\b"); اكسر;
			حالة '\f': fputs_stdout("\\f"); اكسر;
			حالة '\n': fputs_stdout("\\n"); اكسر;
			حالة '\r': fputs_stdout("\\r"); اكسر;
			حالة '\t': fputs_stdout("\\t"); اكسر;
			حالة '\v': fputs_stdout("\\v"); اكسر;
			افتراضي: حرف_چاپ_کن(c);
			}
		}
		حرف_چاپ_کن('\n');
	}

	/* We reach this point only if we search through
	   (max_bytes_to_format - G.string_min) bytes before reaching EOF.  */
	check_and_close();
 ret:
	حرر(buf);
}

#إذا ENABLE_LONG_OPTS
/* If S is a valid traditional offset specification with an optional
   leading '+' return nonzero and set *OFFSET to the offset it denotes.  */

ساكن صحيح
parse_old_جابه_جایی(ثابت حرف *s, off_t *جابه_جایی)
{
	ساكن ثابت بنية suffix_mult Bb[] ALIGN_SUFFIX = {
		{ "B", 1024 },
		{ "b", 512 },
		{ "", 0 }
	};
	حرف *p;
	صحيح radix;

	/* Skip over any leading '+'. */
	إذا (s[0] == '+') ++s;
	إذا (!آیا_رقم(s[0])) أرجع 0; /* not a number */

	/* Determine the radix we'll use to interpret S.  If there is a '.',
	 * it's decimal, otherwise, if the string begins with '0X'or '0x',
	 * it's hexadecimal, else octal.  */
	p = strchr(s, '.');
	radix = 8;
	إذا (p) {
		p[0] = '\0'; /* cheating */
		radix = 10;
	} وإلا إذا (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
		radix = 16;

	*جابه_جایی = xstrtooff_sfx(s, radix, Bb);
	إذا (p) p[0] = '.';

	أرجع (*جابه_جایی >= 0);
}
#پایان_اگر

صحيح od_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح od_main(صحيح argc استفادهنشده_PARAM, حرف **argv)
{
#إذا ENABLE_LONG_OPTS
	ساكن ثابت حرف od_longopts[] ALIGN1 =
		"skip-bytes\0"        Required_آرگومان "j"
		"address-radix\0"     Required_آرگومان "A"
		"read-bytes\0"        Required_آرگومان "N"
		"format\0"            Required_آرگومان "t"
		"output-duplicates\0" No_آرگومان       "v"
		/* Yes, it's true: -S NUM, but --strings[=NUM]!
		 * that is, NUM is mandatory for -S but optional for --strings!
		 */
		"strings\0"           Optional_آرگومان "S"
		"width\0"             Optional_آرگومان "w"
		"traditional\0"       No_آرگومان       "\xff"
		;
#پایان_اگر
	ثابت حرف *str_A, *str_N, *str_j, *str_S = "3";
	llist_t *lst_t = عدم;
	بیعلامت opt;
	صحيح l_c_m;
	/* The number of input bytes to skip before formatting and writing.  */
	off_t n_بایتها_مقصد_skip = 0;
	/* The offset of the first byte after the last byte to be formatted.  */
	off_t انتها_جابه_جایی = 0;
	/* The maximum number of bytes that will be formatted.  */
	off_t max_بایتها_مقصد_format = 0;

	INIT_G();

	/*G.spec = NULL; - already is */
	G.format_آدرس = format_آدرس_std;
	آدرس_پایه_char = 'o';
	آدرس_pad_len_char = '7';

	/* Parse command line */
	opt = OD_GETOPT32();
	argv += optind;
	إذا (opt & OPT_A) {
		ساكن ثابت حرف doxn[] ALIGN1 = "doxn";
		ساكن ثابت حرف doxn_آدرس_پایه_char[] ALIGN1 = {
			'u', 'o', 'x', /* '?' fourth one is not important */
		};
		ساكن ثابت uint8_t doxn_آدرس_pad_len_char[] ALIGN1 = {
			'7', '7', '6', /* '?' */
		};
		حرف *p;
		صحيح pos;
		p = strchr(doxn, str_A[0]);
		إذا (!p)
			bb_error_پیام_and_die("bad output address radix "
				"'%c' (must be [doxn])", str_A[0]);
		pos = p - doxn;
		إذا (pos == 3) G.format_آدرس = format_آدرس_none;
		آدرس_پایه_char = doxn_آدرس_پایه_char[pos];
		آدرس_pad_len_char = doxn_آدرس_pad_len_char[pos];
	}
	إذا (opt & OPT_N) {
		max_بایتها_مقصد_format = xstrtooff_sfx(str_N, 0, bkm_suffixes);
	}

	إذا (opt & OPT_a) decode_format_string("a");
	إذا (opt & OPT_b) decode_format_string("oC");
	إذا (opt & OPT_c) decode_format_string("c");
	إذا (opt & OPT_d) decode_format_string("u2");
	إذا (opt & OPT_D) decode_format_string("uI");
	إذا (opt & OPT_f) decode_format_string("fF");
	إذا (opt & (OPT_h|OPT_x)) decode_format_string("x2");
	إذا (opt & (OPT_H|OPT_X)) decode_format_string("xI");
	/* -I,L,l: depend on word width of the arch (what is "long"?) */
#إذا ULONG_MAX > 0xffffffff
	إذا (opt & OPT_i) decode_format_string("dI");
	إذا (opt & (OPT_I|OPT_l|OPT_L)) decode_format_string("dL");
#وإلا
	/* 32-bit arch: -I,L,l are the same as -i */
	إذا (opt & (OPT_i|OPT_I|OPT_l|OPT_L)) decode_format_string("dI");
#پایان_اگر
	إذا (opt & OPT_j) n_بایتها_مقصد_skip = xstrtooff_sfx(str_j, 0, bkm_suffixes);
	إذا (opt & (OPT_o|OPT_B)) decode_format_string("o2");
	إذا (opt & OPT_O) decode_format_string("oI");
	طالما (lst_t) {
		decode_format_string(llist_برداشت(&lst_t));
	}
	إذا (opt & OPT_s) decode_format_string("d2");
	إذا (opt & OPT_S) {
		G.string_min = xstrtou_sfx(str_S, 0, bkm_suffixes);
	}

	// Bloat:
	//if ((option_mask32 & OPT_S) && G.n_specs > 0)
	//	bb_error_msg_and_die("no type may be specified when dumping strings");

	/* If the --traditional option is used, there may be from
	 * 0 to 3 remaining command line arguments;  handle each case
	 * separately.
	 * od [FILE] [[+]OFFSET[.][b] [[+]LABEL[.][b]]]
	 * The offset and pseudo_start have the same syntax.
	 *
	 * FIXME: POSIX 1003.1-2001 with XSI requires support for the
	 * traditional syntax even if --traditional is not given.  */

#إذا ENABLE_LONG_OPTS
	إذا (opt & OPT_traditional) {
		إذا (argv[0]) {
			off_t pseudo_شروع = -1;
			off_t o1, o2;

			إذا (!argv[1]) { /* one arg */
				إذا (parse_old_جابه_جایی(argv[0], &o1)) {
					/* od --traditional OFFSET */
					n_بایتها_مقصد_skip = o1;
					argv++;
				}
				/* od --traditional FILE */
			} وإلا إذا (!argv[2]) { /* two args */
				إذا (parse_old_جابه_جایی(argv[0], &o1)
				 && parse_old_جابه_جایی(argv[1], &o2)
				) {
					/* od --traditional OFFSET LABEL */
					n_بایتها_مقصد_skip = o1;
					pseudo_شروع = o2;
					argv += 2;
				} وإلا إذا (parse_old_جابه_جایی(argv[1], &o2)) {
					/* od --traditional FILE OFFSET */
					n_بایتها_مقصد_skip = o2;
					argv[1] = عدم;
				} وإلا {
					bb_error_پیام_and_die("invalid second argument '%s'", argv[1]);
				}
			} وإلا إذا (!argv[3]) { /* three args */
				إذا (parse_old_جابه_جایی(argv[1], &o1)
				 && parse_old_جابه_جایی(argv[2], &o2)
				) {
					/* od --traditional FILE OFFSET LABEL */
					n_بایتها_مقصد_skip = o1;
					pseudo_شروع = o2;
					argv[1] = عدم;
				} وإلا {
					bb_simple_error_پیام_and_die("the last two arguments must be offsets");
				}
			} وإلا { /* >3 args */
				bb_simple_error_پیام_and_die("too many arguments");
			}

			إذا (pseudo_شروع >= 0) {
				إذا (G.format_آدرس == format_آدرس_none) {
					آدرس_پایه_char = 'o';
					آدرس_pad_len_char = '7';
					G.format_آدرس = format_آدرس_paren;
				} وإلا {
					G.format_آدرس = format_آدرس_label;
				}
				G_pseudo_جابه_جایی = pseudo_شروع - n_بایتها_مقصد_skip;
			}
		}
		/* else: od --traditional (without args) */
	}
#پایان_اگر

	إذا (option_mask32 & OPT_N) {
		انتها_جابه_جایی = n_بایتها_مقصد_skip + max_بایتها_مقصد_format;
		إذا (انتها_جابه_جایی < n_بایتها_مقصد_skip)
			bb_simple_error_پیام_and_die("SKIP + SIZE is too large");
	}

	إذا (G.n_specs == 0) {
		decode_format_string("o2");
		/*G.n_specs = 1; - done by decode_format_string */
	}

	/* If no files were listed on the command line,
	   set the global pointer FILE_LIST so that it
	   references the null-terminated list of one name: "-".  */
	G.file_list = bb_argv_dash;
	إذا (argv[0]) {
		/* Set the global pointer FILE_LIST so that it
		   references the first file-argument on the command-line.  */
		G.file_list = (حرف ثابت *ثابت *) argv;
	}

	/* Open the first input file */
	open_بعدی_file();
	/* Skip over any unwanted header bytes */
	skip(n_بایتها_مقصد_skip);
	إذا (!G.درون_جریان)
		أرجع EXIT_FAILURE;

	/* Compute output block length */
	l_c_m = get_lcm();

	إذا (opt & OPT_w) { /* -w: width */
		إذا (!G.بایتها_در_هر_بلوک || G.بایتها_در_هر_بلوک % l_c_m != 0) {
			bb_error_پیام("warning: invalid width %u; using %d instead",
					(بیعلامت)G.بایتها_در_هر_بلوک, l_c_m);
			G.بایتها_در_هر_بلوک = l_c_m;
		}
	} وإلا {
		G.بایتها_در_هر_بلوک = l_c_m;
		إذا (l_c_m < DEFAULT_بایتها_در_هر_بلوک)
			G.بایتها_در_هر_بلوک *= DEFAULT_بایتها_در_هر_بلوک / l_c_m;
	}

#اگر_تعریف_شده DEBUG
	{
		صحيح i;
		لكل (i = 0; i < G.n_specs; i++) {
			چاپ_کن("%d: fmt='%s' width=%d\n",
				i, G.spec[i].fmt_string,
				عرض_بایتها[G.spec[i].اندازه]);
		}
	}
#پایان_اگر

	إذا (option_mask32 & OPT_S)
		dump_رشتهها(n_بایتها_مقصد_skip, انتها_جابه_جایی);
	وإلا
		dump(n_بایتها_مقصد_skip, انتها_جابه_جایی);

	إذا (fclose(stdin))
		bb_simple_perror_پیام_and_die(bb_پیام_standard_درون_داد);

	أرجع G.exit_کد;
}
