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
#شامل "common_bufsiz.h"
#تعریف_کرو أكد(a) ((خالی)0)


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

شمار {
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
	&str_A, &str_N, &str_j, &lst_t, &str_S, &G.بائٹیں_فی_بلاک)


/* Check for 0x7f is a coreutils 6.3 addition */
#تعریف_کرو ISPRINT(c) (((c) >= ' ') && (c) < 0x7f)

قسمتعریف طویل دوہرا longdouble_t;
قسمتعریف غیرعلامتی طویل طویل ulonglong_t;
قسمتعریف طویل طویل llong;

#اگر ENABLE_LFS
# define xstrtooff_sfx xstrtoull_sfx
#ورنہ
# define xstrtooff_sfx xstrtoul_sfx
#اگر_ختم

/* The default number of input bytes per output line.  */
#تعریف_کرو DEFAULT_بائٹیں_فی_بلاک 16

/* The number of decimal digits of precision in a float.  */
#اگر_غیر_تعریف_شدہ FLT_DIG
# define FLT_DIG 7
#اگر_ختم

/* The number of decimal digits of precision in a double.  */
#اگر_غیر_تعریف_شدہ DBL_DIG
# define DBL_DIG 15
#اگر_ختم

/* The number of decimal digits of precision in a long double.  */
#اگر_غیر_تعریف_شدہ LDBL_DIG
# define LDBL_DIG DBL_DIG
#اگر_ختم

شمار سائز_spec {
	NO_سائز,
	CHAR,
	SHORT,
	INT,
	LONG,
	LONG_LONG,
	FLOAT_SINGLE,
	FLOAT_DOUBLE,
	FLOAT_LONG_DOUBLE,
	N_سائز_SPECS
};

شمار output_format {
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
ساخت tspec {
	شمار output_format fmt;
	شمار سائز_spec سائز;
	خالی (*print_function) (سائز_t, ثابت حرف *, ثابت حرف *);
	حرف *fmt_string;
	صحیح hexl_منوال_trailer;
	صحیح میدان_چوڑائی;
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

ساکن ثابت uint8_t بائٹیں_مقصد_oct_digits[] ALIGN1 =
{0, 3, 6, 8, 11, 14, 16, 19, 22, 25, 27, 30, 32, 35, 38, 41, 43};

ساکن ثابت uint8_t بائٹیں_مقصد_signed_dec_digits[] ALIGN1 =
{1, 4, 6, 8, 11, 13, 16, 18, 20, 23, 25, 28, 30, 33, 35, 37, 40};

ساکن ثابت uint8_t بائٹیں_مقصد_unsigned_dec_digits[] ALIGN1 =
{0, 3, 5, 8, 10, 13, 15, 17, 20, 22, 25, 27, 29, 32, 34, 37, 39};

ساکن ثابت uint8_t بائٹیں_مقصد_hex_digits[] ALIGN1 =
{0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32};

/* Convert enum size_spec to the size of the named type.  */
ساکن ثابت علامتی حرف چوڑائی_بائٹیں[] ALIGN1 = {
	-1,
	حجم(حرف),
	حجم(مختصر),
	حجم(صحیح),
	حجم(طویل),
	حجم(ulonglong_t),
	حجم(اعشاری),
	حجم(دوہرا),
	حجم(longdouble_t)
};
/* Ensure that for each member of 'enum size_spec' there is an
   initializer in the width_bytes array.  */
ساخت ERR_چوڑائی_بائٹیں_has_bad_سائز {
	حرف ERR_چوڑائی_بائٹیں_has_bad_سائز[ARRAY_سائز(چوڑائی_بائٹیں) == N_سائز_SPECS ? 1 : -1];
};

ساخت globals {
	smallint exit_رمز;

	غیرعلامتی string_min;

	/* An array of specs describing how to format each input block.  */
	غیرعلامتی n_specs;
	ساخت tspec *spec;

	/* Function that accepts an address and an optional following char,
	   and prints the address and char to stdout.  */
	خالی (*format_پتہ)(off_t, حرف);

	/* The difference between the old-style pseudo starting address and
	   the number of bytes to skip.  */
#اگر ENABLE_LONG_OPTS
	off_t pseudo_انحراف;
# define G_pseudo_انحراف G.pseudo_انحراف
#اگر_ختم
	/* When zero, MAX_BYTES_TO_FORMAT and END_OFFSET are ignored, and all
	   input is formatted.  */

	/* The number of input bytes formatted per output line.  It must be
	   a multiple of the least common multiple of the sizes associated with
	   the specified output types.  It should be as large as possible, but
	   no larger than 16 -- unless specified with the -w option.  */
	غیرعلامتی بائٹیں_فی_بلاک; /* have to use unsigned, not size_t */

	/* A NULL-terminated list of the file-arguments from the command line.  */
	ثابت حرف *ثابت *file_list;

	/* The input stream associated with the current file.  */
	FILE *اندر_دھارا;

	bool not_پہلا;
	bool پچھلا_pair_equal;

	حرف پتہ_fmt[حجم("%0n"OFF_FMT"xc")];
} FIX_ALIASING;
/* Corresponds to 'x' above */
#تعریف_کرو پتہ_بنیاد_char G.پتہ_fmt[حجم(G.پتہ_fmt)-3]
/* Corresponds to 'n' above */
#تعریف_کرو پتہ_pad_len_char G.پتہ_fmt[2]

#اگر !ENABLE_LONG_OPTS
شمار { G_pseudo_انحراف = 0 };
#اگر_ختم
#تعریف_کرو G (*(ساخت globals*)bb_common_bufsiz1)
#تعریف_کرو INIT_G() کرو { \
	تیاری_common_bufsiz(); \
	BUILD_BUG_ON(حجم(G) > COMMON_BUFSIZE); \
	G.بائٹیں_فی_بلاک = 32; \
	strcpy(G.پتہ_fmt, "%0n"OFF_FMT"xc"); \
} جب (0)


#تعریف_کرو MAX_INTEGRAL_TYPE_سائز حجم(ulonglong_t)
ساکن ثابت غیرعلامتی حرف integral_type_سائز[MAX_INTEGRAL_TYPE_سائز + 1] ALIGN1 = {
	[حجم(حرف)] = CHAR,
#اگر USHRT_MAX != UCHAR_MAX
	[حجم(مختصر)] = SHORT,
#اگر_ختم
#اگر UINT_MAX != USHRT_MAX
	[حجم(صحیح)] = INT,
#اگر_ختم
#اگر ULONG_MAX != UINT_MAX
	[حجم(طویل)] = LONG,
#اگر_ختم
#اگر ULLONG_MAX != ULONG_MAX
	[حجم(ulonglong_t)] = LONG_LONG,
#اگر_ختم
};

#تعریف_کرو MAX_FP_TYPE_سائز حجم(longdouble_t)
ساکن ثابت غیرعلامتی حرف fp_type_سائز[MAX_FP_TYPE_سائز + 1] ALIGN1 = {
	/* gcc seems to allow repeated indexes. Last one wins */
	[حجم(longdouble_t)] = FLOAT_LONG_DOUBLE,
	[حجم(دوہرا)] = FLOAT_DOUBLE,
	[حجم(اعشاری)] = FLOAT_SINGLE
};


ساکن غیرعلامتی
عاد_اعظم(غیرعلامتی u, غیرعلامتی v)
{
	غیرعلامتی t;
	جب (v != 0) {
		t = u % v;
		u = v;
		v = t;
	}
	واپس u;
}

/* Compute the least common multiple of U and V.  */
ساکن غیرعلامتی
lcm(غیرعلامتی u, غیرعلامتی v) {
	غیرعلامتی t = عاد_اعظم(u, v);
	اگر (t == 0)
		واپس 0;
	واپس u * v / t;
}

ساکن خالی
print_s_char(سائز_t n_بائٹیں, ثابت حرف *بلاک, ثابت حرف *fmt_string)
{
	جب (n_بائٹیں--) {
		صحیح tmp = *(علامتی حرف *) بلاک;
		لکھو(fmt_string, tmp);
		بلاک += حجم(غیرعلامتی حرف);
	}
}

ساکن خالی
print_char(سائز_t n_بائٹیں, ثابت حرف *بلاک, ثابت حرف *fmt_string)
{
	جب (n_بائٹیں--) {
		غیرعلامتی tmp = *(غیرعلامتی حرف *) بلاک;
		لکھو(fmt_string, tmp);
		بلاک += حجم(غیرعلامتی حرف);
	}
}

ساکن خالی
print_s_short(سائز_t n_بائٹیں, ثابت حرف *بلاک, ثابت حرف *fmt_string)
{
	n_بائٹیں /= حجم(علامتی مختصر);
	جب (n_بائٹیں--) {
		صحیح tmp = *(علامتی مختصر *) بلاک;
		لکھو(fmt_string, tmp);
		بلاک += حجم(غیرعلامتی مختصر);
	}
}

ساکن خالی
print_short(سائز_t n_بائٹیں, ثابت حرف *بلاک, ثابت حرف *fmt_string)
{
	n_بائٹیں /= حجم(غیرعلامتی مختصر);
	جب (n_بائٹیں--) {
		غیرعلامتی tmp = *(غیرعلامتی مختصر *) بلاک;
		لکھو(fmt_string, tmp);
		بلاک += حجم(غیرعلامتی مختصر);
	}
}

ساکن خالی
print_int(سائز_t n_بائٹیں, ثابت حرف *بلاک, ثابت حرف *fmt_string)
{
	n_بائٹیں /= حجم(غیرعلامتی);
	جب (n_بائٹیں--) {
		غیرعلامتی tmp = *(غیرعلامتی *) بلاک;
		لکھو(fmt_string, tmp);
		بلاک += حجم(غیرعلامتی);
	}
}

#اگر UINT_MAX == ULONG_MAX
# define print_long print_int
#ورنہ
ساکن خالی
print_long(سائز_t n_بائٹیں, ثابت حرف *بلاک, ثابت حرف *fmt_string)
{
	n_بائٹیں /= حجم(غیرعلامتی طویل);
	جب (n_بائٹیں--) {
		غیرعلامتی طویل tmp = *(غیرعلامتی طویل *) بلاک;
		لکھو(fmt_string, tmp);
		بلاک += حجم(غیرعلامتی طویل);
	}
}
#اگر_ختم

#اگر ULONG_MAX == ULLONG_MAX
# define print_long_long print_long
#ورنہ
ساکن خالی
print_long_long(سائز_t n_بائٹیں, ثابت حرف *بلاک, ثابت حرف *fmt_string)
{
	n_بائٹیں /= حجم(ulonglong_t);
	جب (n_بائٹیں--) {
		ulonglong_t tmp = *(ulonglong_t *) بلاک;
		لکھو(fmt_string, tmp);
		بلاک += حجم(ulonglong_t);
	}
}
#اگر_ختم

ساکن خالی
print_float(سائز_t n_بائٹیں, ثابت حرف *بلاک, ثابت حرف *fmt_string)
{
	n_بائٹیں /= حجم(اعشاری);
	جب (n_بائٹیں--) {
		اعشاری tmp = *(اعشاری *) بلاک;
		لکھو(fmt_string, tmp);
		بلاک += حجم(اعشاری);
	}
}

ساکن خالی
print_double(سائز_t n_بائٹیں, ثابت حرف *بلاک, ثابت حرف *fmt_string)
{
	n_بائٹیں /= حجم(دوہرا);
	جب (n_بائٹیں--) {
		دوہرا tmp = *(دوہرا *) بلاک;
		لکھو(fmt_string, tmp);
		بلاک += حجم(دوہرا);
	}
}

ساکن خالی
print_long_double(سائز_t n_بائٹیں, ثابت حرف *بلاک, ثابت حرف *fmt_string)
{
	n_بائٹیں /= حجم(longdouble_t);
	جب (n_بائٹیں--) {
		longdouble_t tmp = *(longdouble_t *) بلاک;
		لکھو(fmt_string, tmp);
		بلاک += حجم(longdouble_t);
	}
}

/* print_[named]_ascii are optimized for speed.
 * Remember, someday you may want to pump gigabytes through this thing.
 * Saving a dozen of .text bytes here is counter-productive */

ساکن خالی
print_named_ascii(سائز_t n_بائٹیں, ثابت حرف *بلاک,
		ثابت حرف *غیر_مستعمل_fmt_string غیر_مستعمل_PARAM)
{
	/* Names for some non-printing characters.  */
	ساکن ثابت حرف charname[33][3] ALIGN1 = {
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

	جب (n_بائٹیں--) {
		غیرعلامتی masked_c = *(غیرعلامتی حرف *) بلاک++;

		masked_c &= 0x7f;
		اگر (masked_c == 0x7f) {
			fputs_stdout(" del");
			جاری;
		}
		اگر (masked_c > ' ') {
			buf[3] = masked_c;
			fputs_stdout(buf);
			جاری;
		}
		/* Why? Because printf(" %3.3s") is much slower... */
		buf[6] = charname[masked_c][0];
		buf[7] = charname[masked_c][1];
		buf[8] = charname[masked_c][2];
		fputs_stdout(buf+5);
	}
}

ساکن خالی
print_ascii(سائز_t n_بائٹیں, ثابت حرف *بلاک,
		ثابت حرف *غیر_مستعمل_fmt_string غیر_مستعمل_PARAM)
{
	// buf[N] pos:  01234 56789
	حرف buf[12] = "   x\0 xxx\0";

	جب (n_بائٹیں--) {
		ثابت حرف *s;
		غیرعلامتی c = *(غیرعلامتی حرف *) بلاک++;

		اگر (ISPRINT(c)) {
			buf[3] = c;
			fputs_stdout(buf);
			جاری;
		}
		انتخاب (c) {
		حالت '\0':
			s = "  \\0";
			توڑو;
		حالت '\007':
			s = "  \\a";
			توڑو;
		حالت '\b':
			s = "  \\b";
			توڑو;
		حالت '\f':
			s = "  \\f";
			توڑو;
		حالت '\n':
			s = "  \\n";
			توڑو;
		حالت '\r':
			s = "  \\r";
			توڑو;
		حالت '\t':
			s = "  \\t";
			توڑو;
		حالت '\v':
			s = "  \\v";
			توڑو;
		طےشدہ:
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

ساکن خالی
open_اگلا_file(خالی)
{
	جب (1) {
		اگر (!*G.file_list)
			واپس;
		G.اندر_دھارا = fopen_or_warn_stdin(*G.file_list++);
		اگر (G.اندر_دھارا) {
			توڑو;
		}
		G.exit_رمز = 1;
	}

	اگر ((option_mask32 & (OPT_N|OPT_S)) == OPT_N)
		setbuf(G.اندر_دھارا, تہی);
}

/* Test whether there have been errors on in_stream, and close it if
   it is not standard input.  Return nonzero if there has been an error
   on in_stream or stdout; return zero otherwise.  This function will
   report more than one error only if both a read and a write error
   have occurred.  IN_ERRNO, if nonzero, is the error number
   corresponding to the most recent action for IN_STREAM.  */

ساکن خالی
check_and_close(خالی)
{
	اگر (G.اندر_دھارا) {
		اگر (ferror(G.اندر_دھارا))	{
			bb_error_پیغام("%s: read error", (G.اندر_دھارا == stdin)
					? bb_پیغام_standard_داخلہ
					: G.file_list[-1]
			);
			G.exit_رمز = 1;
		}
		fclose_if_not_stdin(G.اندر_دھارا);
		G.اندر_دھارا = تہی;
	}

	اگر (ferror(stdout)) {
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

ساکن NOINLINE ثابت حرف *
decode_one_format(ثابت حرف *s_orig, ثابت حرف *s, ساخت tspec *tspec)
{
	شمار سائز_spec سائز_spec;
	غیرعلامتی سائز;
	شمار output_format fmt;
	ثابت حرف *p;
	حرف *انتہا;
	حرف *fmt_string = تہی;
	خالی (*print_function) (سائز_t, ثابت حرف *, ثابت حرف *);
	غیرعلامتی c;
	غیرعلامتی میدان_چوڑائی = 0;
	صحیح pos;

	انتخاب (*s) {
	حالت 'd':
	حالت 'o':
	حالت 'u':
	حالت 'x': {
		ساکن ثابت حرف CSIL[] ALIGN1 = "CSIL";

		c = *s++;
		p = strchr(CSIL, *s);
		/* if *s == NUL, p != NULL! Testcase: "od -tx" */
		اگر (!p || *p == '\0') {
			سائز = حجم(صحیح);
			اگر (ہندسہ_ہے(s[0])) {
				سائز = bb_strtou(s, &انتہا, 0);
				اگر (errno == ERANGE
				 || MAX_INTEGRAL_TYPE_سائز < سائز
				 || integral_type_سائز[سائز] == NO_سائز
				) {
					bb_error_پیغام_and_die("invalid type string '%s'; "
						"%u-byte %s type is not supported",
						s_orig, سائز, "integral");
				}
				s = انتہا;
			}
		} ورنہ {
			ساکن ثابت uint8_t CSIL_sizeof[4] = {
				حجم(حرف),
				حجم(مختصر),
				حجم(صحیح),
				حجم(طویل),
			};
			سائز = CSIL_sizeof[p - CSIL];
			s++; /* skip C/S/I/L */
		}

#تعریف_کرو ISPEC_مقصد_FORMAT(Spec, Min_format, Long_format, Max_format) \
	((Spec) == LONG_LONG ? (Max_format) \
	: ((Spec) == LONG ? (Long_format) : (Min_format)))

#تعریف_کرو FMT_بائٹیں_ALLOCATED 9
		سائز_spec = integral_type_سائز[سائز];

		{
			ساکن ثابت حرف doux[] ALIGN1 = "doux";
			ساکن ثابت حرف doux_fmt_letter[][4] = {
				"lld", "llo", "llu", "llx"
			};
			ساکن ثابت شمار output_format doux_fmt[] = {
				SIGNED_DECIMAL,
				OCTAL,
				UNSIGNED_DECIMAL,
				HEXADECIMAL,
			};
			ساکن ثابت uint8_t *ثابت doux_بائٹیں_مقصد_XXX[] = {
				بائٹیں_مقصد_signed_dec_digits,
				بائٹیں_مقصد_oct_digits,
				بائٹیں_مقصد_unsigned_dec_digits,
				بائٹیں_مقصد_hex_digits,
			};
			ساکن ثابت حرف doux_fmtstring[][حجم(" %%0%u%s")] ALIGN1 = {
				" %%%u%s",
				" %%0%u%s",
				" %%%u%s",
				" %%0%u%s",
			};

			pos = strchr(doux, c) - doux;
			fmt = doux_fmt[pos];
			میدان_چوڑائی = doux_بائٹیں_مقصد_XXX[pos][سائز];
			p = doux_fmt_letter[pos] + 2;
			اگر (سائز_spec == LONG) p--;
			اگر (سائز_spec == LONG_LONG) p -= 2;
			fmt_string = xasprintf(doux_fmtstring[pos], میدان_چوڑائی, p);
		}

		انتخاب (سائز_spec) {
		حالت CHAR:
			print_function = (fmt == SIGNED_DECIMAL
				    ? print_s_char
				    : print_char);
			توڑو;
		حالت SHORT:
			print_function = (fmt == SIGNED_DECIMAL
				    ? print_s_short
				    : print_short);
			توڑو;
		حالت INT:
			print_function = print_int;
			توڑو;
		حالت LONG:
			print_function = print_long;
			توڑو;
		طےشدہ: /* case LONG_LONG: */
			print_function = print_long_long;
			توڑو;
		}
		توڑو;
	}

	حالت 'f': {
		ساکن ثابت حرف FDL[] ALIGN1 = "FDL";

		fmt = FLOATING_نقطہ;
		++s;
		p = strchr(FDL, *s);
		اگر (!p || *p == '\0') {
			سائز = حجم(دوہرا);
			اگر (ہندسہ_ہے(s[0])) {
				سائز = bb_strtou(s, &انتہا, 0);
				اگر (errno == ERANGE || سائز > MAX_FP_TYPE_سائز
				 || fp_type_سائز[سائز] == NO_سائز
				) {
					bb_error_پیغام_and_die("invalid type string '%s'; "
						"%u-byte %s type is not supported",
						s_orig, سائز, "floating point");
				}
				s = انتہا;
			}
		} ورنہ {
			ساکن ثابت uint8_t FDL_sizeof[] = {
				حجم(اعشاری),
				حجم(دوہرا),
				حجم(longdouble_t),
			};

			سائز = FDL_sizeof[p - FDL];
			s++; /* skip F/D/L */
		}

		سائز_spec = fp_type_سائز[سائز];

		انتخاب (سائز_spec) {
		حالت FLOAT_SINGLE:
			print_function = print_float;
			میدان_چوڑائی = FLT_DIG + 8;
			/* Don't use %#e; not all systems support it.  */
			fmt_string = xasprintf(" %%%d.%de", میدان_چوڑائی, FLT_DIG);
			توڑو;
		حالت FLOAT_DOUBLE:
			print_function = print_double;
			میدان_چوڑائی = DBL_DIG + 8;
			fmt_string = xasprintf(" %%%d.%de", میدان_چوڑائی, DBL_DIG);
			توڑو;
		طےشدہ: /* case FLOAT_LONG_DOUBLE: */
			print_function = print_long_double;
			میدان_چوڑائی = LDBL_DIG + 8;
			fmt_string = xasprintf(" %%%d.%dLe", میدان_چوڑائی, LDBL_DIG);
			توڑو;
		}
		توڑو;
	}

	حالت 'a':
		++s;
		fmt = NAMED_CHARACTER;
		سائز_spec = CHAR;
		print_function = print_named_ascii;
		میدان_چوڑائی = 3;
		توڑو;
	حالت 'c':
		++s;
		fmt = CHARACTER;
		سائز_spec = CHAR;
		print_function = print_ascii;
		میدان_چوڑائی = 3;
		توڑو;
	طےشدہ:
		bb_error_پیغام_and_die("invalid character '%c' "
				"in type string '%s'", *s, s_orig);
	}

	tspec->سائز = سائز_spec;
	tspec->fmt = fmt;
	tspec->print_function = print_function;
	tspec->fmt_string = fmt_string;

	tspec->میدان_چوڑائی = میدان_چوڑائی;
	tspec->hexl_منوال_trailer = (*s == 'z');
	اگر (tspec->hexl_منوال_trailer)
		s++;

	واپس s;
}

/* Decode the modern od format string S.  Append the decoded
   representation to the global array SPEC, reallocating SPEC if
   necessary.  */

ساکن خالی
decode_format_string(ثابت حرف *s)
{
	ثابت حرف *s_orig = s;

	جب (*s != '\0') {
		ساخت tspec tspec;
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

ساکن خالی
skip(off_t n_skip)
{
	اگر (n_skip == 0)
		واپس;

	جب (G.اندر_دھارا) { /* !EOF */
		ساخت stat file_stats;

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
		اگر (fstat(fileno(G.اندر_دھارا), &file_stats) == 0
		 && S_ISREG(file_stats.st_منوال) && file_stats.st_سائز > 0
		) {
			اگر (file_stats.st_سائز < n_skip) {
				n_skip -= file_stats.st_سائز;
				/* take "check & close / open_next" route */
			} ورنہ {
				اگر (fseeko(G.اندر_دھارا, n_skip, SEEK_CUR) != 0)
					G.exit_رمز = 1;
				واپس;
			}
		} ورنہ {
			/* If it's not a regular file with positive size,
			   position the file pointer by reading.  */
			حرف buf[1024];
			سائز_t n_بائٹیں_مقصد_read = 1024;
			سائز_t n_بائٹیں_read;

			جب (n_skip > 0) {
				اگر (n_skip < n_بائٹیں_مقصد_read)
					n_بائٹیں_مقصد_read = n_skip;
				n_بائٹیں_read = fread(buf, 1, n_بائٹیں_مقصد_read, G.اندر_دھارا);
				n_skip -= n_بائٹیں_read;
				اگر (n_بائٹیں_read != n_بائٹیں_مقصد_read)
					توڑو; /* EOF on this file or error */
			}
		}
		اگر (n_skip == 0)
			واپس;

		check_and_close();
		open_اگلا_file();
	}

	اگر (n_skip)
		bb_simple_error_پیغام_and_die("can't skip past end of combined input");
}


قسمتعریف خالی FN_format_پتہ(off_t پتہ, حرف c);

ساکن خالی
format_پتہ_none(off_t پتہ غیر_مستعمل_PARAM, حرف c غیر_مستعمل_PARAM)
{
}

ساکن خالی
format_پتہ_std(off_t پتہ, حرف c)
{
	/* Corresponds to 'c' */
	G.پتہ_fmt[حجم(G.پتہ_fmt)-2] = c;
	لکھو(G.پتہ_fmt, پتہ);
}

#اگر ENABLE_LONG_OPTS
/* only used with --traditional */
ساکن خالی
format_پتہ_paren(off_t پتہ, حرف c)
{
	حرف_لکھو('(');
	format_پتہ_std(پتہ, ')');
	اگر (c) حرف_لکھو(c);
}

ساکن خالی
format_پتہ_label(off_t پتہ, حرف c)
{
	format_پتہ_std(پتہ, ' ');
	format_پتہ_paren(پتہ + G_pseudo_انحراف, c);
}
#اگر_ختم

ساکن خالی
dump_hexl_منوال_trailer(سائز_t n_بائٹیں, ثابت حرف *بلاک)
{
	fputs_stdout("  >");
	جب (n_بائٹیں--) {
		غیرعلامتی c = *(غیرعلامتی حرف *) بلاک++;
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

ساکن خالی
write_بلاک(off_t current_انحراف, سائز_t n_بائٹیں,
		ثابت حرف *پچھلا_بلاک, ثابت حرف *curr_بلاک)
{
	غیرعلامتی i;

	اگر (!(option_mask32 & OPT_v)
	 && G.not_پہلا
	 && n_بائٹیں == G.بائٹیں_فی_بلاک
	 && memcmp(پچھلا_بلاک, curr_بلاک, G.بائٹیں_فی_بلاک) == 0
	) {
		اگر (G.پچھلا_pair_equal) {
			/* The two preceding blocks were equal, and the current
			   block is the same as the last one, so print nothing.  */
		} ورنہ {
			puts("*");
			G.پچھلا_pair_equal = 1;
		}
	} ورنہ {
		G.not_پہلا = 1;
		G.پچھلا_pair_equal = 0;
		برائے (i = 0; i < G.n_specs; i++) {
			اگر (i == 0)
				G.format_پتہ(current_انحراف, '\0');
			ورنہ
				لکھو("%*s", پتہ_pad_len_char - '0', "");
			(*G.spec[i].print_function) (n_بائٹیں, curr_بلاک, G.spec[i].fmt_string);
			اگر (G.spec[i].hexl_منوال_trailer) {
				/* space-pad out to full line width, then dump the trailer */
				غیرعلامتی datum_چوڑائی = چوڑائی_بائٹیں[G.spec[i].سائز];
				غیرعلامتی blank_fields = (G.بائٹیں_فی_بلاک - n_بائٹیں) / datum_چوڑائی;
				غیرعلامتی میدان_چوڑائی = G.spec[i].میدان_چوڑائی + 1;
				لکھو("%*s", blank_fields * میدان_چوڑائی, "");
				dump_hexl_منوال_trailer(n_بائٹیں, curr_بلاک);
			}
			حرف_لکھو('\n');
		}
	}
}

ساکن خالی
read_بلاک(سائز_t n, حرف *بلاک, سائز_t *n_بائٹیں_اندر_بفر)
{
	أكد(0 < n && n <= G.بائٹیں_فی_بلاک);

	*n_بائٹیں_اندر_بفر = 0;

	اگر (n == 0)
		واپس;

	جب (G.اندر_دھارا != تہی) { /* EOF.  */
		سائز_t n_needed;
		سائز_t n_read;

		n_needed = n - *n_بائٹیں_اندر_بفر;
		n_read = fread(بلاک + *n_بائٹیں_اندر_بفر, 1, n_needed, G.اندر_دھارا);
		*n_بائٹیں_اندر_بفر += n_read;
		اگر (n_read == n_needed)
			توڑو;
		/* error check is done in check_and_close */
		check_and_close();
		open_اگلا_file();
	}
}

/* Return the least common multiple of the sizes associated
   with the format specs.  */

ساکن صحیح
get_lcm(خالی)
{
	سائز_t i;
	صحیح l_c_m = 1;

	برائے (i = 0; i < G.n_specs; i++)
		l_c_m = lcm(l_c_m, چوڑائی_بائٹیں[(صحیح) G.spec[i].سائز]);
	واپس l_c_m;
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

ساکن خالی
dump(off_t current_انحراف, off_t انتہا_انحراف)
{
	حرف *بلاک[2];
	صحیح idx;
	سائز_t n_بائٹیں_read;

	بلاک[0] = xmalloc(2 * G.بائٹیں_فی_بلاک);
	بلاک[1] = بلاک[0] + G.بائٹیں_فی_بلاک;

	idx = 0;
	اگر (option_mask32 & OPT_N) {
		جب (1) {
			سائز_t n_needed;
			اگر (current_انحراف >= انتہا_انحراف) {
				n_بائٹیں_read = 0;
				توڑو;
			}
			n_needed = MIN(انتہا_انحراف - current_انحراف, (off_t) G.بائٹیں_فی_بلاک);
			read_بلاک(n_needed, بلاک[idx], &n_بائٹیں_read);
			اگر (n_بائٹیں_read < G.بائٹیں_فی_بلاک)
				توڑو;
			أكد(n_بائٹیں_read == G.بائٹیں_فی_بلاک);
			write_بلاک(current_انحراف, n_بائٹیں_read, بلاک[idx ^ 1], بلاک[idx]);
			current_انحراف += n_بائٹیں_read;
			idx ^= 1;
		}
	} ورنہ {
		جب (1) {
			read_بلاک(G.بائٹیں_فی_بلاک, بلاک[idx], &n_بائٹیں_read);
			اگر (n_بائٹیں_read < G.بائٹیں_فی_بلاک)
				توڑو;
			أكد(n_بائٹیں_read == G.بائٹیں_فی_بلاک);
			write_بلاک(current_انحراف, n_بائٹیں_read, بلاک[idx ^ 1], بلاک[idx]);
			current_انحراف += n_بائٹیں_read;
			idx ^= 1;
		}
	}

	اگر (n_بائٹیں_read > 0) {
		صحیح l_c_m;
		سائز_t بائٹیں_مقصد_write;

		l_c_m = get_lcm();

		/* Make bytes_to_write the smallest multiple of l_c_m that
		   is at least as large as n_bytes_read.  */
		بائٹیں_مقصد_write = l_c_m * ((n_بائٹیں_read + l_c_m - 1) / l_c_m);

		memset(بلاک[idx] + n_بائٹیں_read, 0, بائٹیں_مقصد_write - n_بائٹیں_read);
		write_بلاک(current_انحراف, بائٹیں_مقصد_write,
				بلاک[idx ^ 1], بلاک[idx]);
		current_انحراف += n_بائٹیں_read;
	}

	G.format_پتہ(current_انحراف, '\n');

	اگر ((option_mask32 & OPT_N) && current_انحراف >= انتہا_انحراف)
		check_and_close();

	آزاد(بلاک[0]);
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

ساکن خالی
dump_لڑیاں(off_t پتہ, off_t انتہا_انحراف)
{
	غیرعلامتی bufsize = MAX(100, G.string_min);
	غیرعلامتی حرف *buf = xmalloc(bufsize);

	جب (1) {
		سائز_t i;
		صحیح c;

		/* See if the next 'G.string_min' chars are all printing chars.  */
 tryline:
		اگر ((option_mask32 & OPT_N) && (انتہا_انحراف - G.string_min <= پتہ))
			توڑو;
		i = 0;
		جب (!(option_mask32 & OPT_N) || پتہ < انتہا_انحراف) {
			اگر (i == bufsize) {
				bufsize += bufsize/8;
				buf = xrealloc(buf, bufsize);
			}

			جب (G.اندر_دھارا) { /* !EOF */
				c = fgetc(G.اندر_دھارا);
				اگر (c != EOF)
					جاؤ got_char;
				check_and_close();
				open_اگلا_file();
			}
			/* EOF */
			جاؤ ret;
 got_char:
			پتہ++;
			اگر (!c)
				توڑو;
			اگر (!ISPRINT(c))
				جاؤ tryline;	/* It isn't; give up on this string.  */
			buf[i++] = c;		/* String continues; store it all.  */
		}

		اگر (i < G.string_min)		/* Too short! */
			جاؤ tryline;

		/* If we get here, the string is all printable and NUL-terminated */
		buf[i] = 0;
		G.format_پتہ(پتہ - i - 1, ' ');

		برائے (i = 0; (c = buf[i]); i++) {
			انتخاب (c) {
			حالت '\007': fputs_stdout("\\a"); توڑو;
			حالت '\b': fputs_stdout("\\b"); توڑو;
			حالت '\f': fputs_stdout("\\f"); توڑو;
			حالت '\n': fputs_stdout("\\n"); توڑو;
			حالت '\r': fputs_stdout("\\r"); توڑو;
			حالت '\t': fputs_stdout("\\t"); توڑو;
			حالت '\v': fputs_stdout("\\v"); توڑو;
			طےشدہ: حرف_لکھو(c);
			}
		}
		حرف_لکھو('\n');
	}

	/* We reach this point only if we search through
	   (max_bytes_to_format - G.string_min) bytes before reaching EOF.  */
	check_and_close();
 ret:
	آزاد(buf);
}

#اگر ENABLE_LONG_OPTS
/* If S is a valid traditional offset specification with an optional
   leading '+' return nonzero and set *OFFSET to the offset it denotes.  */

ساکن صحیح
parse_old_انحراف(ثابت حرف *s, off_t *انحراف)
{
	ساکن ثابت ساخت suffix_mult Bb[] ALIGN_SUFFIX = {
		{ "B", 1024 },
		{ "b", 512 },
		{ "", 0 }
	};
	حرف *p;
	صحیح radix;

	/* Skip over any leading '+'. */
	اگر (s[0] == '+') ++s;
	اگر (!ہندسہ_ہے(s[0])) واپس 0; /* not a number */

	/* Determine the radix we'll use to interpret S.  If there is a '.',
	 * it's decimal, otherwise, if the string begins with '0X'or '0x',
	 * it's hexadecimal, else octal.  */
	p = strchr(s, '.');
	radix = 8;
	اگر (p) {
		p[0] = '\0'; /* cheating */
		radix = 10;
	} ورنہ اگر (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
		radix = 16;

	*انحراف = xstrtooff_sfx(s, radix, Bb);
	اگر (p) p[0] = '.';

	واپس (*انحراف >= 0);
}
#اگر_ختم

صحیح od_main(صحیح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح od_main(صحیح argc غیر_مستعمل_PARAM, حرف **argv)
{
#اگر ENABLE_LONG_OPTS
	ساکن ثابت حرف od_longopts[] ALIGN1 =
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
#اگر_ختم
	ثابت حرف *str_A, *str_N, *str_j, *str_S = "3";
	llist_t *lst_t = تہی;
	غیرعلامتی opt;
	صحیح l_c_m;
	/* The number of input bytes to skip before formatting and writing.  */
	off_t n_بائٹیں_مقصد_skip = 0;
	/* The offset of the first byte after the last byte to be formatted.  */
	off_t انتہا_انحراف = 0;
	/* The maximum number of bytes that will be formatted.  */
	off_t max_بائٹیں_مقصد_format = 0;

	INIT_G();

	/*G.spec = NULL; - already is */
	G.format_پتہ = format_پتہ_std;
	پتہ_بنیاد_char = 'o';
	پتہ_pad_len_char = '7';

	/* Parse command line */
	opt = OD_GETOPT32();
	argv += optind;
	اگر (opt & OPT_A) {
		ساکن ثابت حرف doxn[] ALIGN1 = "doxn";
		ساکن ثابت حرف doxn_پتہ_بنیاد_char[] ALIGN1 = {
			'u', 'o', 'x', /* '?' fourth one is not important */
		};
		ساکن ثابت uint8_t doxn_پتہ_pad_len_char[] ALIGN1 = {
			'7', '7', '6', /* '?' */
		};
		حرف *p;
		صحیح pos;
		p = strchr(doxn, str_A[0]);
		اگر (!p)
			bb_error_پیغام_and_die("bad output address radix "
				"'%c' (must be [doxn])", str_A[0]);
		pos = p - doxn;
		اگر (pos == 3) G.format_پتہ = format_پتہ_none;
		پتہ_بنیاد_char = doxn_پتہ_بنیاد_char[pos];
		پتہ_pad_len_char = doxn_پتہ_pad_len_char[pos];
	}
	اگر (opt & OPT_N) {
		max_بائٹیں_مقصد_format = xstrtooff_sfx(str_N, 0, bkm_suffixes);
	}

	اگر (opt & OPT_a) decode_format_string("a");
	اگر (opt & OPT_b) decode_format_string("oC");
	اگر (opt & OPT_c) decode_format_string("c");
	اگر (opt & OPT_d) decode_format_string("u2");
	اگر (opt & OPT_D) decode_format_string("uI");
	اگر (opt & OPT_f) decode_format_string("fF");
	اگر (opt & (OPT_h|OPT_x)) decode_format_string("x2");
	اگر (opt & (OPT_H|OPT_X)) decode_format_string("xI");
	/* -I,L,l: depend on word width of the arch (what is "long"?) */
#اگر ULONG_MAX > 0xffffffff
	اگر (opt & OPT_i) decode_format_string("dI");
	اگر (opt & (OPT_I|OPT_l|OPT_L)) decode_format_string("dL");
#ورنہ
	/* 32-bit arch: -I,L,l are the same as -i */
	اگر (opt & (OPT_i|OPT_I|OPT_l|OPT_L)) decode_format_string("dI");
#اگر_ختم
	اگر (opt & OPT_j) n_بائٹیں_مقصد_skip = xstrtooff_sfx(str_j, 0, bkm_suffixes);
	اگر (opt & (OPT_o|OPT_B)) decode_format_string("o2");
	اگر (opt & OPT_O) decode_format_string("oI");
	جب (lst_t) {
		decode_format_string(llist_نکالو(&lst_t));
	}
	اگر (opt & OPT_s) decode_format_string("d2");
	اگر (opt & OPT_S) {
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

#اگر ENABLE_LONG_OPTS
	اگر (opt & OPT_traditional) {
		اگر (argv[0]) {
			off_t pseudo_آغاز = -1;
			off_t o1, o2;

			اگر (!argv[1]) { /* one arg */
				اگر (parse_old_انحراف(argv[0], &o1)) {
					/* od --traditional OFFSET */
					n_بائٹیں_مقصد_skip = o1;
					argv++;
				}
				/* od --traditional FILE */
			} ورنہ اگر (!argv[2]) { /* two args */
				اگر (parse_old_انحراف(argv[0], &o1)
				 && parse_old_انحراف(argv[1], &o2)
				) {
					/* od --traditional OFFSET LABEL */
					n_بائٹیں_مقصد_skip = o1;
					pseudo_آغاز = o2;
					argv += 2;
				} ورنہ اگر (parse_old_انحراف(argv[1], &o2)) {
					/* od --traditional FILE OFFSET */
					n_بائٹیں_مقصد_skip = o2;
					argv[1] = تہی;
				} ورنہ {
					bb_error_پیغام_and_die("invalid second argument '%s'", argv[1]);
				}
			} ورنہ اگر (!argv[3]) { /* three args */
				اگر (parse_old_انحراف(argv[1], &o1)
				 && parse_old_انحراف(argv[2], &o2)
				) {
					/* od --traditional FILE OFFSET LABEL */
					n_بائٹیں_مقصد_skip = o1;
					pseudo_آغاز = o2;
					argv[1] = تہی;
				} ورنہ {
					bb_simple_error_پیغام_and_die("the last two arguments must be offsets");
				}
			} ورنہ { /* >3 args */
				bb_simple_error_پیغام_and_die("too many arguments");
			}

			اگر (pseudo_آغاز >= 0) {
				اگر (G.format_پتہ == format_پتہ_none) {
					پتہ_بنیاد_char = 'o';
					پتہ_pad_len_char = '7';
					G.format_پتہ = format_پتہ_paren;
				} ورنہ {
					G.format_پتہ = format_پتہ_label;
				}
				G_pseudo_انحراف = pseudo_آغاز - n_بائٹیں_مقصد_skip;
			}
		}
		/* else: od --traditional (without args) */
	}
#اگر_ختم

	اگر (option_mask32 & OPT_N) {
		انتہا_انحراف = n_بائٹیں_مقصد_skip + max_بائٹیں_مقصد_format;
		اگر (انتہا_انحراف < n_بائٹیں_مقصد_skip)
			bb_simple_error_پیغام_and_die("SKIP + SIZE is too large");
	}

	اگر (G.n_specs == 0) {
		decode_format_string("o2");
		/*G.n_specs = 1; - done by decode_format_string */
	}

	/* If no files were listed on the command line,
	   set the global pointer FILE_LIST so that it
	   references the null-terminated list of one name: "-".  */
	G.file_list = bb_argv_dash;
	اگر (argv[0]) {
		/* Set the global pointer FILE_LIST so that it
		   references the first file-argument on the command-line.  */
		G.file_list = (حرف ثابت *ثابت *) argv;
	}

	/* Open the first input file */
	open_اگلا_file();
	/* Skip over any unwanted header bytes */
	skip(n_بائٹیں_مقصد_skip);
	اگر (!G.اندر_دھارا)
		واپس EXIT_FAILURE;

	/* Compute output block length */
	l_c_m = get_lcm();

	اگر (opt & OPT_w) { /* -w: width */
		اگر (!G.بائٹیں_فی_بلاک || G.بائٹیں_فی_بلاک % l_c_m != 0) {
			bb_error_پیغام("warning: invalid width %u; using %d instead",
					(غیرعلامتی)G.بائٹیں_فی_بلاک, l_c_m);
			G.بائٹیں_فی_بلاک = l_c_m;
		}
	} ورنہ {
		G.بائٹیں_فی_بلاک = l_c_m;
		اگر (l_c_m < DEFAULT_بائٹیں_فی_بلاک)
			G.بائٹیں_فی_بلاک *= DEFAULT_بائٹیں_فی_بلاک / l_c_m;
	}

#اگر_تعریف_شدہ DEBUG
	{
		صحیح i;
		برائے (i = 0; i < G.n_specs; i++) {
			لکھو("%d: fmt='%s' width=%d\n",
				i, G.spec[i].fmt_string,
				چوڑائی_بائٹیں[G.spec[i].سائز]);
		}
	}
#اگر_ختم

	اگر (option_mask32 & OPT_S)
		dump_لڑیاں(n_بائٹیں_مقصد_skip, انتہا_انحراف);
	ورنہ
		dump(n_بائٹیں_مقصد_skip, انتہا_انحراف);

	اگر (fclose(stdin))
		bb_simple_perror_پیغام_and_die(bb_پیغام_standard_داخلہ);

	واپس G.exit_رمز;
}
