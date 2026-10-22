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
#تعریف_کرو أكد(a) ((فراغ)0)


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

#تعریف_کرو OD_GETOPT32() getopt32long(argv, \
	"A:N:abcdDfhHiIj:lLoOBt:*vxXsS:w:+:", od_longopts, \
	/* -w with optional param */ \
	/* -S was -s and also had optional parameter */ \
	/* but in coreutils 6.3 it was renamed and now has */ \
	/* _mandatory_ parameter */ \
	&str_A, &str_N, &str_j, &lst_t, &str_S, &G.بائٹاں_فی_بلاک)


/* Check for 0x7f is a coreutils 6.3 addition */
#تعریف_کرو ISPRINT(c) (((c) >= ' ') && (c) < 0x7f)

تعريفنوع لما مزدوج longdouble_t;
تعريفنوع بنانشانی لما لما ulonglong_t;
تعريفنوع لما لما llong;

#إذا ENABLE_LFS
# define xstrtooff_sfx xstrtoull_sfx
#وإلا
# define xstrtooff_sfx xstrtoul_sfx
#جے_انت

/* The default number of input bytes per output line.  */
#تعریف_کرو DEFAULT_بائٹاں_فی_بلاک 16

/* The number of decimal digits of precision in a float.  */
#جے_بنا_تعریف FLT_DIG
# define FLT_DIG 7
#جے_انت

/* The number of decimal digits of precision in a double.  */
#جے_بنا_تعریف DBL_DIG
# define DBL_DIG 15
#جے_انت

/* The number of decimal digits of precision in a long double.  */
#جے_بنا_تعریف LDBL_DIG
# define LDBL_DIG DBL_DIG
#جے_انت

تعداد ناپ_spec {
	NO_ناپ,
	CHAR,
	SHORT,
	INT,
	LONG,
	LONG_LONG,
	FLOAT_SINGLE,
	FLOAT_DOUBLE,
	FLOAT_LONG_DOUBLE,
	N_ناپ_SPECS
};

تعداد output_format {
	SIGNED_DECIMAL,
	UNSIGNED_DECIMAL,
	OCTAL,
	HEXADECIMAL,
	FLOATING_نقطہ,
	NAMED_CHARACTER,
	CHARACTER
};

/* Each output format specification (from '-t spec' or from
   old-style options) is represented by one of these structures.  */
بنية tspec {
	تعداد output_format fmt;
	تعداد ناپ_spec ناپ;
	فراغ (*print_function) (ناپ_t, ثابت حرف *, ثابت حرف *);
	حرف *fmt_string;
	صحيح hexl_منوال_trailer;
	صحيح میدان_چوڑائی;
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

ساكن ثابت uint8_t بائٹاں_منزل_oct_digits[] ALIGN1 =
{0, 3, 6, 8, 11, 14, 16, 19, 22, 25, 27, 30, 32, 35, 38, 41, 43};

ساكن ثابت uint8_t بائٹاں_منزل_signed_dec_digits[] ALIGN1 =
{1, 4, 6, 8, 11, 13, 16, 18, 20, 23, 25, 28, 30, 33, 35, 37, 40};

ساكن ثابت uint8_t بائٹاں_منزل_unsigned_dec_digits[] ALIGN1 =
{0, 3, 5, 8, 10, 13, 15, 17, 20, 22, 25, 27, 29, 32, 34, 37, 39};

ساكن ثابت uint8_t بائٹاں_منزل_hex_digits[] ALIGN1 =
{0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32};

/* Convert enum size_spec to the size of the named type.  */
ساكن ثابت نشانیوالا حرف چوڑائی_بائٹاں[] ALIGN1 = {
	-1,
	حجم(حرف),
	حجم(چھوٹا),
	حجم(صحيح),
	حجم(لما),
	حجم(ulonglong_t),
	حجم(عائم),
	حجم(مزدوج),
	حجم(longdouble_t)
};
/* Ensure that for each member of 'enum size_spec' there is an
   initializer in the width_bytes array.  */
بنية ERR_چوڑائی_بائٹاں_has_bad_ناپ {
	حرف ERR_چوڑائی_بائٹاں_has_bad_ناپ[ARRAY_ناپ(چوڑائی_بائٹاں) == N_ناپ_SPECS ? 1 : -1];
};

بنية globals {
	smallint exit_کوڈ;

	بنانشانی string_min;

	/* An array of specs describing how to format each input block.  */
	بنانشانی n_specs;
	بنية tspec *spec;

	/* Function that accepts an address and an optional following char,
	   and prints the address and char to stdout.  */
	فراغ (*format_پتہ)(off_t, حرف);

	/* The difference between the old-style pseudo starting address and
	   the number of bytes to skip.  */
#إذا ENABLE_LONG_OPTS
	off_t pseudo_وتھ;
# define G_pseudo_وتھ G.pseudo_وتھ
#جے_انت
	/* When zero, MAX_BYTES_TO_FORMAT and END_OFFSET are ignored, and all
	   input is formatted.  */

	/* The number of input bytes formatted per output line.  It must be
	   a multiple of the least common multiple of the sizes associated with
	   the specified output types.  It should be as large as possible, but
	   no larger than 16 -- unless specified with the -w option.  */
	بنانشانی بائٹاں_فی_بلاک; /* have to use unsigned, not size_t */

	/* A NULL-terminated list of the file-arguments from the command line.  */
	ثابت حرف *ثابت *file_list;

	/* The input stream associated with the current file.  */
	FILE *وچ_دھارا;

	bool not_پہلا;
	bool prev_pair_equal;

	حرف پتہ_fmt[حجم("%0n"OFF_FMT"xc")];
} FIX_ALIASING;
/* Corresponds to 'x' above */
#تعریف_کرو پتہ_ادھار_char G.پتہ_fmt[حجم(G.پتہ_fmt)-3]
/* Corresponds to 'n' above */
#تعریف_کرو پتہ_pad_len_char G.پتہ_fmt[2]

#إذا !ENABLE_LONG_OPTS
تعداد { G_pseudo_وتھ = 0 };
#جے_انت
#تعریف_کرو G (*(بنية globals*)bb_common_bufsiz1)
#تعریف_کرو INIT_G() افعل { \
	تیاری_common_bufsiz(); \
	BUILD_BUG_ON(حجم(G) > COMMON_BUFSIZE); \
	G.بائٹاں_فی_بلاک = 32; \
	strcpy(G.پتہ_fmt, "%0n"OFF_FMT"xc"); \
} طالما (0)


#تعریف_کرو MAX_INTEGRAL_TYPE_ناپ حجم(ulonglong_t)
ساكن ثابت بنانشانی حرف integral_type_ناپ[MAX_INTEGRAL_TYPE_ناپ + 1] ALIGN1 = {
	[حجم(حرف)] = CHAR,
#إذا USHRT_MAX != UCHAR_MAX
	[حجم(چھوٹا)] = SHORT,
#جے_انت
#إذا UINT_MAX != USHRT_MAX
	[حجم(صحيح)] = INT,
#جے_انت
#إذا ULONG_MAX != UINT_MAX
	[حجم(لما)] = LONG,
#جے_انت
#إذا ULLONG_MAX != ULONG_MAX
	[حجم(ulonglong_t)] = LONG_LONG,
#جے_انت
};

#تعریف_کرو MAX_FP_TYPE_ناپ حجم(longdouble_t)
ساكن ثابت بنانشانی حرف fp_type_ناپ[MAX_FP_TYPE_ناپ + 1] ALIGN1 = {
	/* gcc seems to allow repeated indexes. Last one wins */
	[حجم(longdouble_t)] = FLOAT_LONG_DOUBLE,
	[حجم(مزدوج)] = FLOAT_DOUBLE,
	[حجم(عائم)] = FLOAT_SINGLE
};


ساكن بنانشانی
عاد_اعظم(بنانشانی u, بنانشانی v)
{
	بنانشانی t;
	طالما (v != 0) {
		t = u % v;
		u = v;
		v = t;
	}
	أرجع u;
}

/* Compute the least common multiple of U and V.  */
ساكن بنانشانی
lcm(بنانشانی u, بنانشانی v) {
	بنانشانی t = عاد_اعظم(u, v);
	إذا (t == 0)
		أرجع 0;
	أرجع u * v / t;
}

ساكن فراغ
print_s_char(ناپ_t n_بائٹاں, ثابت حرف *بلاک, ثابت حرف *fmt_string)
{
	طالما (n_بائٹاں--) {
		صحيح tmp = *(نشانیوالا حرف *) بلاک;
		لکھو(fmt_string, tmp);
		بلاک += حجم(بنانشانی حرف);
	}
}

ساكن فراغ
print_char(ناپ_t n_بائٹاں, ثابت حرف *بلاک, ثابت حرف *fmt_string)
{
	طالما (n_بائٹاں--) {
		بنانشانی tmp = *(بنانشانی حرف *) بلاک;
		لکھو(fmt_string, tmp);
		بلاک += حجم(بنانشانی حرف);
	}
}

ساكن فراغ
print_s_short(ناپ_t n_بائٹاں, ثابت حرف *بلاک, ثابت حرف *fmt_string)
{
	n_بائٹاں /= حجم(نشانیوالا چھوٹا);
	طالما (n_بائٹاں--) {
		صحيح tmp = *(نشانیوالا چھوٹا *) بلاک;
		لکھو(fmt_string, tmp);
		بلاک += حجم(بنانشانی چھوٹا);
	}
}

ساكن فراغ
print_short(ناپ_t n_بائٹاں, ثابت حرف *بلاک, ثابت حرف *fmt_string)
{
	n_بائٹاں /= حجم(بنانشانی چھوٹا);
	طالما (n_بائٹاں--) {
		بنانشانی tmp = *(بنانشانی چھوٹا *) بلاک;
		لکھو(fmt_string, tmp);
		بلاک += حجم(بنانشانی چھوٹا);
	}
}

ساكن فراغ
print_int(ناپ_t n_بائٹاں, ثابت حرف *بلاک, ثابت حرف *fmt_string)
{
	n_بائٹاں /= حجم(بنانشانی);
	طالما (n_بائٹاں--) {
		بنانشانی tmp = *(بنانشانی *) بلاک;
		لکھو(fmt_string, tmp);
		بلاک += حجم(بنانشانی);
	}
}

#إذا UINT_MAX == ULONG_MAX
# define print_long print_int
#وإلا
ساكن فراغ
print_long(ناپ_t n_بائٹاں, ثابت حرف *بلاک, ثابت حرف *fmt_string)
{
	n_بائٹاں /= حجم(بنانشانی لما);
	طالما (n_بائٹاں--) {
		بنانشانی لما tmp = *(بنانشانی لما *) بلاک;
		لکھو(fmt_string, tmp);
		بلاک += حجم(بنانشانی لما);
	}
}
#جے_انت

#إذا ULONG_MAX == ULLONG_MAX
# define print_long_long print_long
#وإلا
ساكن فراغ
print_long_long(ناپ_t n_بائٹاں, ثابت حرف *بلاک, ثابت حرف *fmt_string)
{
	n_بائٹاں /= حجم(ulonglong_t);
	طالما (n_بائٹاں--) {
		ulonglong_t tmp = *(ulonglong_t *) بلاک;
		لکھو(fmt_string, tmp);
		بلاک += حجم(ulonglong_t);
	}
}
#جے_انت

ساكن فراغ
print_float(ناپ_t n_بائٹاں, ثابت حرف *بلاک, ثابت حرف *fmt_string)
{
	n_بائٹاں /= حجم(عائم);
	طالما (n_بائٹاں--) {
		عائم tmp = *(عائم *) بلاک;
		لکھو(fmt_string, tmp);
		بلاک += حجم(عائم);
	}
}

ساكن فراغ
print_double(ناپ_t n_بائٹاں, ثابت حرف *بلاک, ثابت حرف *fmt_string)
{
	n_بائٹاں /= حجم(مزدوج);
	طالما (n_بائٹاں--) {
		مزدوج tmp = *(مزدوج *) بلاک;
		لکھو(fmt_string, tmp);
		بلاک += حجم(مزدوج);
	}
}

ساكن فراغ
print_long_double(ناپ_t n_بائٹاں, ثابت حرف *بلاک, ثابت حرف *fmt_string)
{
	n_بائٹاں /= حجم(longdouble_t);
	طالما (n_بائٹاں--) {
		longdouble_t tmp = *(longdouble_t *) بلاک;
		لکھو(fmt_string, tmp);
		بلاک += حجم(longdouble_t);
	}
}

/* print_[named]_ascii are optimized for speed.
 * Remember, someday you may want to pump gigabytes through this thing.
 * Saving a dozen of .text bytes here is counter-productive */

ساكن فراغ
print_named_ascii(ناپ_t n_بائٹاں, ثابت حرف *بلاک,
		ثابت حرف *بن_ورتیا_fmt_string بن_ورتیا_PARAM)
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

	طالما (n_بائٹاں--) {
		بنانشانی masked_c = *(بنانشانی حرف *) بلاک++;

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
print_ascii(ناپ_t n_بائٹاں, ثابت حرف *بلاک,
		ثابت حرف *بن_ورتیا_fmt_string بن_ورتیا_PARAM)
{
	// buf[N] pos:  01234 56789
	حرف buf[12] = "   x\0 xxx\0";

	طالما (n_بائٹاں--) {
		ثابت حرف *s;
		بنانشانی c = *(بنانشانی حرف *) بلاک++;

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
open_اگلا_file(فراغ)
{
	طالما (1) {
		إذا (!*G.file_list)
			أرجع;
		G.وچ_دھارا = fopen_or_warn_stdin(*G.file_list++);
		إذا (G.وچ_دھارا) {
			اكسر;
		}
		G.exit_کوڈ = 1;
	}

	إذا ((option_mask32 & (OPT_N|OPT_S)) == OPT_N)
		setbuf(G.وچ_دھارا, عدم);
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
	إذا (G.وچ_دھارا) {
		إذا (ferror(G.وچ_دھارا))	{
			bb_error_پیغام("%s: read error", (G.وچ_دھارا == stdin)
					? bb_پیغام_standard_داخلہ
					: G.file_list[-1]
			);
			G.exit_کوڈ = 1;
		}
		fclose_if_not_stdin(G.وچ_دھارا);
		G.وچ_دھارا = عدم;
	}

	إذا (ferror(stdout)) {
		bb_simple_error_پیغام_and_die(bb_پیغام_write_error);
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
	تعداد ناپ_spec ناپ_spec;
	بنانشانی ناپ;
	تعداد output_format fmt;
	ثابت حرف *p;
	حرف *سرا;
	حرف *fmt_string = عدم;
	فراغ (*print_function) (ناپ_t, ثابت حرف *, ثابت حرف *);
	بنانشانی c;
	بنانشانی میدان_چوڑائی = 0;
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
			ناپ = حجم(صحيح);
			إذا (ہندسہ_اے(s[0])) {
				ناپ = bb_strtou(s, &سرا, 0);
				إذا (errno == ERANGE
				 || MAX_INTEGRAL_TYPE_ناپ < ناپ
				 || integral_type_ناپ[ناپ] == NO_ناپ
				) {
					bb_error_پیغام_and_die("invalid type string '%s'; "
						"%u-byte %s type is not supported",
						s_orig, ناپ, "integral");
				}
				s = سرا;
			}
		} وإلا {
			ساكن ثابت uint8_t CSIL_sizeof[4] = {
				حجم(حرف),
				حجم(چھوٹا),
				حجم(صحيح),
				حجم(لما),
			};
			ناپ = CSIL_sizeof[p - CSIL];
			s++; /* skip C/S/I/L */
		}

#تعریف_کرو ISPEC_منزل_FORMAT(Spec, Min_format, Long_format, Max_format) \
	((Spec) == LONG_LONG ? (Max_format) \
	: ((Spec) == LONG ? (Long_format) : (Min_format)))

#تعریف_کرو FMT_بائٹاں_ALLOCATED 9
		ناپ_spec = integral_type_ناپ[ناپ];

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
			ساكن ثابت uint8_t *ثابت doux_بائٹاں_منزل_XXX[] = {
				بائٹاں_منزل_signed_dec_digits,
				بائٹاں_منزل_oct_digits,
				بائٹاں_منزل_unsigned_dec_digits,
				بائٹاں_منزل_hex_digits,
			};
			ساكن ثابت حرف doux_fmtstring[][حجم(" %%0%u%s")] ALIGN1 = {
				" %%%u%s",
				" %%0%u%s",
				" %%%u%s",
				" %%0%u%s",
			};

			pos = strchr(doux, c) - doux;
			fmt = doux_fmt[pos];
			میدان_چوڑائی = doux_بائٹاں_منزل_XXX[pos][ناپ];
			p = doux_fmt_letter[pos] + 2;
			إذا (ناپ_spec == LONG) p--;
			إذا (ناپ_spec == LONG_LONG) p -= 2;
			fmt_string = xasprintf(doux_fmtstring[pos], میدان_چوڑائی, p);
		}

		بدل (ناپ_spec) {
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

		fmt = FLOATING_نقطہ;
		++s;
		p = strchr(FDL, *s);
		إذا (!p || *p == '\0') {
			ناپ = حجم(مزدوج);
			إذا (ہندسہ_اے(s[0])) {
				ناپ = bb_strtou(s, &سرا, 0);
				إذا (errno == ERANGE || ناپ > MAX_FP_TYPE_ناپ
				 || fp_type_ناپ[ناپ] == NO_ناپ
				) {
					bb_error_پیغام_and_die("invalid type string '%s'; "
						"%u-byte %s type is not supported",
						s_orig, ناپ, "floating point");
				}
				s = سرا;
			}
		} وإلا {
			ساكن ثابت uint8_t FDL_sizeof[] = {
				حجم(عائم),
				حجم(مزدوج),
				حجم(longdouble_t),
			};

			ناپ = FDL_sizeof[p - FDL];
			s++; /* skip F/D/L */
		}

		ناپ_spec = fp_type_ناپ[ناپ];

		بدل (ناپ_spec) {
		حالة FLOAT_SINGLE:
			print_function = print_float;
			میدان_چوڑائی = FLT_DIG + 8;
			/* Don't use %#e; not all systems support it.  */
			fmt_string = xasprintf(" %%%d.%de", میدان_چوڑائی, FLT_DIG);
			اكسر;
		حالة FLOAT_DOUBLE:
			print_function = print_double;
			میدان_چوڑائی = DBL_DIG + 8;
			fmt_string = xasprintf(" %%%d.%de", میدان_چوڑائی, DBL_DIG);
			اكسر;
		افتراضي: /* case FLOAT_LONG_DOUBLE: */
			print_function = print_long_double;
			میدان_چوڑائی = LDBL_DIG + 8;
			fmt_string = xasprintf(" %%%d.%dLe", میدان_چوڑائی, LDBL_DIG);
			اكسر;
		}
		اكسر;
	}

	حالة 'a':
		++s;
		fmt = NAMED_CHARACTER;
		ناپ_spec = CHAR;
		print_function = print_named_ascii;
		میدان_چوڑائی = 3;
		اكسر;
	حالة 'c':
		++s;
		fmt = CHARACTER;
		ناپ_spec = CHAR;
		print_function = print_ascii;
		میدان_چوڑائی = 3;
		اكسر;
	افتراضي:
		bb_error_پیغام_and_die("invalid character '%c' "
				"in type string '%s'", *s, s_orig);
	}

	tspec->ناپ = ناپ_spec;
	tspec->fmt = fmt;
	tspec->print_function = print_function;
	tspec->fmt_string = fmt_string;

	tspec->میدان_چوڑائی = میدان_چوڑائی;
	tspec->hexl_منوال_trailer = (*s == 'z');
	إذا (tspec->hexl_منوال_trailer)
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
		ثابت حرف *اگلا;

		اگلا = decode_one_format(s_orig, s, &tspec);

		أكد(s != اگلا);
		s = اگلا;
		G.spec = xrealloc_سمتیہ(G.spec, 4, G.n_specs);
		حافظہ_نقل(&G.spec[G.n_specs], &tspec, حجم(G.spec[0]));
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

	طالما (G.وچ_دھارا) { /* !EOF */
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
		إذا (fstat(fileno(G.وچ_دھارا), &file_stats) == 0
		 && S_ISREG(file_stats.st_منوال) && file_stats.st_ناپ > 0
		) {
			إذا (file_stats.st_ناپ < n_skip) {
				n_skip -= file_stats.st_ناپ;
				/* take "check & close / open_next" route */
			} وإلا {
				إذا (fseeko(G.وچ_دھارا, n_skip, SEEK_CUR) != 0)
					G.exit_کوڈ = 1;
				أرجع;
			}
		} وإلا {
			/* If it's not a regular file with positive size,
			   position the file pointer by reading.  */
			حرف buf[1024];
			ناپ_t n_بائٹاں_منزل_read = 1024;
			ناپ_t n_بائٹاں_read;

			طالما (n_skip > 0) {
				إذا (n_skip < n_بائٹاں_منزل_read)
					n_بائٹاں_منزل_read = n_skip;
				n_بائٹاں_read = fread(buf, 1, n_بائٹاں_منزل_read, G.وچ_دھارا);
				n_skip -= n_بائٹاں_read;
				إذا (n_بائٹاں_read != n_بائٹاں_منزل_read)
					اكسر; /* EOF on this file or error */
			}
		}
		إذا (n_skip == 0)
			أرجع;

		check_and_close();
		open_اگلا_file();
	}

	إذا (n_skip)
		bb_simple_error_پیغام_and_die("can't skip past end of combined input");
}


تعريفنوع فراغ FN_format_پتہ(off_t پتہ, حرف c);

ساكن فراغ
format_پتہ_none(off_t پتہ بن_ورتیا_PARAM, حرف c بن_ورتیا_PARAM)
{
}

ساكن فراغ
format_پتہ_std(off_t پتہ, حرف c)
{
	/* Corresponds to 'c' */
	G.پتہ_fmt[حجم(G.پتہ_fmt)-2] = c;
	لکھو(G.پتہ_fmt, پتہ);
}

#إذا ENABLE_LONG_OPTS
/* only used with --traditional */
ساكن فراغ
format_پتہ_paren(off_t پتہ, حرف c)
{
	حرف_لکھو('(');
	format_پتہ_std(پتہ, ')');
	إذا (c) حرف_لکھو(c);
}

ساكن فراغ
format_پتہ_label(off_t پتہ, حرف c)
{
	format_پتہ_std(پتہ, ' ');
	format_پتہ_paren(پتہ + G_pseudo_وتھ, c);
}
#جے_انت

ساكن فراغ
dump_hexl_منوال_trailer(ناپ_t n_بائٹاں, ثابت حرف *بلاک)
{
	fputs_stdout("  >");
	طالما (n_بائٹاں--) {
		بنانشانی c = *(بنانشانی حرف *) بلاک++;
		c = (ISPRINT(c) ? c : '.');
		حرف_لکھو(c);
	}
	حرف_لکھو('<');
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
write_بلاک(off_t current_وتھ, ناپ_t n_بائٹاں,
		ثابت حرف *prev_بلاک, ثابت حرف *curr_بلاک)
{
	بنانشانی i;

	إذا (!(option_mask32 & OPT_v)
	 && G.not_پہلا
	 && n_بائٹاں == G.بائٹاں_فی_بلاک
	 && memcmp(prev_بلاک, curr_بلاک, G.بائٹاں_فی_بلاک) == 0
	) {
		إذا (G.prev_pair_equal) {
			/* The two preceding blocks were equal, and the current
			   block is the same as the last one, so print nothing.  */
		} وإلا {
			puts("*");
			G.prev_pair_equal = 1;
		}
	} وإلا {
		G.not_پہلا = 1;
		G.prev_pair_equal = 0;
		لكل (i = 0; i < G.n_specs; i++) {
			إذا (i == 0)
				G.format_پتہ(current_وتھ, '\0');
			وإلا
				لکھو("%*s", پتہ_pad_len_char - '0', "");
			(*G.spec[i].print_function) (n_بائٹاں, curr_بلاک, G.spec[i].fmt_string);
			إذا (G.spec[i].hexl_منوال_trailer) {
				/* space-pad out to full line width, then dump the trailer */
				بنانشانی datum_چوڑائی = چوڑائی_بائٹاں[G.spec[i].ناپ];
				بنانشانی blank_fields = (G.بائٹاں_فی_بلاک - n_بائٹاں) / datum_چوڑائی;
				بنانشانی میدان_چوڑائی = G.spec[i].میدان_چوڑائی + 1;
				لکھو("%*s", blank_fields * میدان_چوڑائی, "");
				dump_hexl_منوال_trailer(n_بائٹاں, curr_بلاک);
			}
			حرف_لکھو('\n');
		}
	}
}

ساكن فراغ
read_بلاک(ناپ_t n, حرف *بلاک, ناپ_t *n_بائٹاں_وچ_بفر)
{
	أكد(0 < n && n <= G.بائٹاں_فی_بلاک);

	*n_بائٹاں_وچ_بفر = 0;

	إذا (n == 0)
		أرجع;

	طالما (G.وچ_دھارا != عدم) { /* EOF.  */
		ناپ_t n_needed;
		ناپ_t n_read;

		n_needed = n - *n_بائٹاں_وچ_بفر;
		n_read = fread(بلاک + *n_بائٹاں_وچ_بفر, 1, n_needed, G.وچ_دھارا);
		*n_بائٹاں_وچ_بفر += n_read;
		إذا (n_read == n_needed)
			اكسر;
		/* error check is done in check_and_close */
		check_and_close();
		open_اگلا_file();
	}
}

/* Return the least common multiple of the sizes associated
   with the format specs.  */

ساكن صحيح
get_lcm(فراغ)
{
	ناپ_t i;
	صحيح l_c_m = 1;

	لكل (i = 0; i < G.n_specs; i++)
		l_c_m = lcm(l_c_m, چوڑائی_بائٹاں[(صحيح) G.spec[i].ناپ]);
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
dump(off_t current_وتھ, off_t سرا_وتھ)
{
	حرف *بلاک[2];
	صحيح idx;
	ناپ_t n_بائٹاں_read;

	بلاک[0] = xmalloc(2 * G.بائٹاں_فی_بلاک);
	بلاک[1] = بلاک[0] + G.بائٹاں_فی_بلاک;

	idx = 0;
	إذا (option_mask32 & OPT_N) {
		طالما (1) {
			ناپ_t n_needed;
			إذا (current_وتھ >= سرا_وتھ) {
				n_بائٹاں_read = 0;
				اكسر;
			}
			n_needed = MIN(سرا_وتھ - current_وتھ, (off_t) G.بائٹاں_فی_بلاک);
			read_بلاک(n_needed, بلاک[idx], &n_بائٹاں_read);
			إذا (n_بائٹاں_read < G.بائٹاں_فی_بلاک)
				اكسر;
			أكد(n_بائٹاں_read == G.بائٹاں_فی_بلاک);
			write_بلاک(current_وتھ, n_بائٹاں_read, بلاک[idx ^ 1], بلاک[idx]);
			current_وتھ += n_بائٹاں_read;
			idx ^= 1;
		}
	} وإلا {
		طالما (1) {
			read_بلاک(G.بائٹاں_فی_بلاک, بلاک[idx], &n_بائٹاں_read);
			إذا (n_بائٹاں_read < G.بائٹاں_فی_بلاک)
				اكسر;
			أكد(n_بائٹاں_read == G.بائٹاں_فی_بلاک);
			write_بلاک(current_وتھ, n_بائٹاں_read, بلاک[idx ^ 1], بلاک[idx]);
			current_وتھ += n_بائٹاں_read;
			idx ^= 1;
		}
	}

	إذا (n_بائٹاں_read > 0) {
		صحيح l_c_m;
		ناپ_t بائٹاں_منزل_write;

		l_c_m = get_lcm();

		/* Make bytes_to_write the smallest multiple of l_c_m that
		   is at least as large as n_bytes_read.  */
		بائٹاں_منزل_write = l_c_m * ((n_بائٹاں_read + l_c_m - 1) / l_c_m);

		memset(بلاک[idx] + n_بائٹاں_read, 0, بائٹاں_منزل_write - n_بائٹاں_read);
		write_بلاک(current_وتھ, بائٹاں_منزل_write,
				بلاک[idx ^ 1], بلاک[idx]);
		current_وتھ += n_بائٹاں_read;
	}

	G.format_پتہ(current_وتھ, '\n');

	إذا ((option_mask32 & OPT_N) && current_وتھ >= سرا_وتھ)
		check_and_close();

	حرر(بلاک[0]);
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
dump_لڑیاں(off_t پتہ, off_t سرا_وتھ)
{
	بنانشانی bufsize = MAX(100, G.string_min);
	بنانشانی حرف *buf = xmalloc(bufsize);

	طالما (1) {
		ناپ_t i;
		صحيح c;

		/* See if the next 'G.string_min' chars are all printing chars.  */
 tryline:
		إذا ((option_mask32 & OPT_N) && (سرا_وتھ - G.string_min <= پتہ))
			اكسر;
		i = 0;
		طالما (!(option_mask32 & OPT_N) || پتہ < سرا_وتھ) {
			إذا (i == bufsize) {
				bufsize += bufsize/8;
				buf = xrealloc(buf, bufsize);
			}

			طالما (G.وچ_دھارا) { /* !EOF */
				c = fgetc(G.وچ_دھارا);
				إذا (c != EOF)
					اذهب got_char;
				check_and_close();
				open_اگلا_file();
			}
			/* EOF */
			اذهب ret;
 got_char:
			پتہ++;
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
		G.format_پتہ(پتہ - i - 1, ' ');

		لكل (i = 0; (c = buf[i]); i++) {
			بدل (c) {
			حالة '\007': fputs_stdout("\\a"); اكسر;
			حالة '\b': fputs_stdout("\\b"); اكسر;
			حالة '\f': fputs_stdout("\\f"); اكسر;
			حالة '\n': fputs_stdout("\\n"); اكسر;
			حالة '\r': fputs_stdout("\\r"); اكسر;
			حالة '\t': fputs_stdout("\\t"); اكسر;
			حالة '\v': fputs_stdout("\\v"); اكسر;
			افتراضي: حرف_لکھو(c);
			}
		}
		حرف_لکھو('\n');
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
parse_old_وتھ(ثابت حرف *s, off_t *وتھ)
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
	إذا (!ہندسہ_اے(s[0])) أرجع 0; /* not a number */

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

	*وتھ = xstrtooff_sfx(s, radix, Bb);
	إذا (p) p[0] = '.';

	أرجع (*وتھ >= 0);
}
#جے_انت

صحيح od_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح od_main(صحيح argc بن_ورتیا_PARAM, حرف **argv)
{
#إذا ENABLE_LONG_OPTS
	ساكن ثابت حرف od_longopts[] ALIGN1 =
		"skip-bytes\0"        Required_دلیل "j"
		"address-radix\0"     Required_دلیل "A"
		"read-bytes\0"        Required_دلیل "N"
		"format\0"            Required_دلیل "t"
		"output-duplicates\0" No_دلیل       "v"
		/* Yes, it's true: -S NUM, but --strings[=NUM]!
		 * that is, NUM is mandatory for -S but optional for --strings!
		 */
		"strings\0"           Optional_دلیل "S"
		"width\0"             Optional_دلیل "w"
		"traditional\0"       No_دلیل       "\xff"
		;
#جے_انت
	ثابت حرف *str_A, *str_N, *str_j, *str_S = "3";
	llist_t *lst_t = عدم;
	بنانشانی opt;
	صحيح l_c_m;
	/* The number of input bytes to skip before formatting and writing.  */
	off_t n_بائٹاں_منزل_skip = 0;
	/* The offset of the first byte after the last byte to be formatted.  */
	off_t سرا_وتھ = 0;
	/* The maximum number of bytes that will be formatted.  */
	off_t max_بائٹاں_منزل_format = 0;

	INIT_G();

	/*G.spec = NULL; - already is */
	G.format_پتہ = format_پتہ_std;
	پتہ_ادھار_char = 'o';
	پتہ_pad_len_char = '7';

	/* Parse command line */
	opt = OD_GETOPT32();
	argv += optind;
	إذا (opt & OPT_A) {
		ساكن ثابت حرف doxn[] ALIGN1 = "doxn";
		ساكن ثابت حرف doxn_پتہ_ادھار_char[] ALIGN1 = {
			'u', 'o', 'x', /* '?' fourth one is not important */
		};
		ساكن ثابت uint8_t doxn_پتہ_pad_len_char[] ALIGN1 = {
			'7', '7', '6', /* '?' */
		};
		حرف *p;
		صحيح pos;
		p = strchr(doxn, str_A[0]);
		إذا (!p)
			bb_error_پیغام_and_die("bad output address radix "
				"'%c' (must be [doxn])", str_A[0]);
		pos = p - doxn;
		إذا (pos == 3) G.format_پتہ = format_پتہ_none;
		پتہ_ادھار_char = doxn_پتہ_ادھار_char[pos];
		پتہ_pad_len_char = doxn_پتہ_pad_len_char[pos];
	}
	إذا (opt & OPT_N) {
		max_بائٹاں_منزل_format = xstrtooff_sfx(str_N, 0, bkm_suffixes);
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
#جے_انت
	إذا (opt & OPT_j) n_بائٹاں_منزل_skip = xstrtooff_sfx(str_j, 0, bkm_suffixes);
	إذا (opt & (OPT_o|OPT_B)) decode_format_string("o2");
	إذا (opt & OPT_O) decode_format_string("oI");
	طالما (lst_t) {
		decode_format_string(llist_نکالو(&lst_t));
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
				إذا (parse_old_وتھ(argv[0], &o1)) {
					/* od --traditional OFFSET */
					n_بائٹاں_منزل_skip = o1;
					argv++;
				}
				/* od --traditional FILE */
			} وإلا إذا (!argv[2]) { /* two args */
				إذا (parse_old_وتھ(argv[0], &o1)
				 && parse_old_وتھ(argv[1], &o2)
				) {
					/* od --traditional OFFSET LABEL */
					n_بائٹاں_منزل_skip = o1;
					pseudo_شروع = o2;
					argv += 2;
				} وإلا إذا (parse_old_وتھ(argv[1], &o2)) {
					/* od --traditional FILE OFFSET */
					n_بائٹاں_منزل_skip = o2;
					argv[1] = عدم;
				} وإلا {
					bb_error_پیغام_and_die("invalid second argument '%s'", argv[1]);
				}
			} وإلا إذا (!argv[3]) { /* three args */
				إذا (parse_old_وتھ(argv[1], &o1)
				 && parse_old_وتھ(argv[2], &o2)
				) {
					/* od --traditional FILE OFFSET LABEL */
					n_بائٹاں_منزل_skip = o1;
					pseudo_شروع = o2;
					argv[1] = عدم;
				} وإلا {
					bb_simple_error_پیغام_and_die("the last two arguments must be offsets");
				}
			} وإلا { /* >3 args */
				bb_simple_error_پیغام_and_die("too many arguments");
			}

			إذا (pseudo_شروع >= 0) {
				إذا (G.format_پتہ == format_پتہ_none) {
					پتہ_ادھار_char = 'o';
					پتہ_pad_len_char = '7';
					G.format_پتہ = format_پتہ_paren;
				} وإلا {
					G.format_پتہ = format_پتہ_label;
				}
				G_pseudo_وتھ = pseudo_شروع - n_بائٹاں_منزل_skip;
			}
		}
		/* else: od --traditional (without args) */
	}
#جے_انت

	إذا (option_mask32 & OPT_N) {
		سرا_وتھ = n_بائٹاں_منزل_skip + max_بائٹاں_منزل_format;
		إذا (سرا_وتھ < n_بائٹاں_منزل_skip)
			bb_simple_error_پیغام_and_die("SKIP + SIZE is too large");
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
	open_اگلا_file();
	/* Skip over any unwanted header bytes */
	skip(n_بائٹاں_منزل_skip);
	إذا (!G.وچ_دھارا)
		أرجع EXIT_FAILURE;

	/* Compute output block length */
	l_c_m = get_lcm();

	إذا (opt & OPT_w) { /* -w: width */
		إذا (!G.بائٹاں_فی_بلاک || G.بائٹاں_فی_بلاک % l_c_m != 0) {
			bb_error_پیغام("warning: invalid width %u; using %d instead",
					(بنانشانی)G.بائٹاں_فی_بلاک, l_c_m);
			G.بائٹاں_فی_بلاک = l_c_m;
		}
	} وإلا {
		G.بائٹاں_فی_بلاک = l_c_m;
		إذا (l_c_m < DEFAULT_بائٹاں_فی_بلاک)
			G.بائٹاں_فی_بلاک *= DEFAULT_بائٹاں_فی_بلاک / l_c_m;
	}

#جے_تعریف_شدہ DEBUG
	{
		صحيح i;
		لكل (i = 0; i < G.n_specs; i++) {
			لکھو("%d: fmt='%s' width=%d\n",
				i, G.spec[i].fmt_string,
				چوڑائی_بائٹاں[G.spec[i].ناپ]);
		}
	}
#جے_انت

	إذا (option_mask32 & OPT_S)
		dump_لڑیاں(n_بائٹاں_منزل_skip, سرا_وتھ);
	وإلا
		dump(n_بائٹاں_منزل_skip, سرا_وتھ);

	إذا (fclose(stdin))
		bb_simple_perror_پیغام_and_die(bb_پیغام_standard_داخلہ);

	أرجع G.exit_کوڈ;
}
