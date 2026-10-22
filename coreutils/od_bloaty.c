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
#ਅਨ੍ਤਰ੍ਭਾਵ "common_bufsiz.h"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪ੍ਰਤਿਜ੍ਞਾ(a) ((ਖਾਲੀ)0)


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

ਗਿਣਤੀ {
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

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OD_GETOPT32() getopt32long(argv, \
	"A:N:abcdDfhHiIj:lLoOBt:*vxXsS:w:+:", od_longopts, \
	/* -w with optional param */ \
	/* -S was -s and also had optional parameter */ \
	/* but in coreutils 6.3 it was renamed and now has */ \
	/* _mandatory_ parameter */ \
	&str_A, &str_N, &str_j, &lst_t, &str_S, &G.ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਖੰਡ)


/* Check for 0x7f is a coreutils 6.3 addition */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ISPRINT(c) (((c) >= ' ') && (c) < 0x7f)

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਲੰਮਾ ਦੁੱਗਣਾ longdouble_t;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ ulonglong_t;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਲੰਮਾ ਲੰਮਾ llong;

#ਜੇ ENABLE_LFS
# define xstrtooff_sfx xstrtoull_sfx
#ਨਹੀਂਤਾਂ
# define xstrtooff_sfx xstrtoul_sfx
#ਜੇ_ਅੰਤ

/* The default number of input bytes per output line.  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEFAULT_ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਖੰਡ 16

/* The number of decimal digits of precision in a float.  */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ FLT_DIG
# define FLT_DIG 7
#ਜੇ_ਅੰਤ

/* The number of decimal digits of precision in a double.  */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ DBL_DIG
# define DBL_DIG 15
#ਜੇ_ਅੰਤ

/* The number of decimal digits of precision in a long double.  */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ LDBL_DIG
# define LDBL_DIG DBL_DIG
#ਜੇ_ਅੰਤ

ਗਿਣਤੀ ਮਾਪ_spec {
	NO_ਮਾਪ,
	CHAR,
	SHORT,
	INT,
	LONG,
	LONG_LONG,
	FLOAT_SINGLE,
	FLOAT_DOUBLE,
	FLOAT_LONG_DOUBLE,
	N_ਮਾਪ_SPECS
};

ਗਿਣਤੀ output_format {
	SIGNED_DECIMAL,
	UNSIGNED_DECIMAL,
	OCTAL,
	HEXADECIMAL,
	FLOATING_ਬਿੰਦੂ,
	NAMED_CHARACTER,
	CHARACTER
};

/* Each output format specification (from '-t spec' or from
   old-style options) is represented by one of these structures.  */
ਬਣਤਰ tspec {
	ਗਿਣਤੀ output_format fmt;
	ਗਿਣਤੀ ਮਾਪ_spec ਮਾਪ;
	ਖਾਲੀ (*print_function) (ਮਾਪ_t, ਅਚਲ ਅੱਖਰ *, ਅਚਲ ਅੱਖਰ *);
	ਅੱਖਰ *fmt_string;
	ਪੂਰਨ hexl_ਬਹੁਲਕ_trailer;
	ਪੂਰਨ ਖੇਤਰ_ਚੌੜਾਈ;
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

ਸਥਿਰ ਅਚਲ uint8_t ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_oct_digits[] ALIGN1 =
{0, 3, 6, 8, 11, 14, 16, 19, 22, 25, 27, 30, 32, 35, 38, 41, 43};

ਸਥਿਰ ਅਚਲ uint8_t ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_signed_dec_digits[] ALIGN1 =
{1, 4, 6, 8, 11, 13, 16, 18, 20, 23, 25, 28, 30, 33, 35, 37, 40};

ਸਥਿਰ ਅਚਲ uint8_t ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_unsigned_dec_digits[] ALIGN1 =
{0, 3, 5, 8, 10, 13, 15, 17, 20, 22, 25, 27, 29, 32, 34, 37, 39};

ਸਥਿਰ ਅਚਲ uint8_t ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_hex_digits[] ALIGN1 =
{0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32};

/* Convert enum size_spec to the size of the named type.  */
ਸਥਿਰ ਅਚਲ ਨਿਸ਼ਾਨੀਵਾਲਾ ਅੱਖਰ ਚੌੜਾਈ_ਬਾਈਟਾਂ[] ALIGN1 = {
	-1,
	ਆਕਾਰ(ਅੱਖਰ),
	ਆਕਾਰ(ਛੋਟਾ),
	ਆਕਾਰ(ਪੂਰਨ),
	ਆਕਾਰ(ਲੰਮਾ),
	ਆਕਾਰ(ulonglong_t),
	ਆਕਾਰ(ਦਸ਼ਮਲਵ),
	ਆਕਾਰ(ਦੁੱਗਣਾ),
	ਆਕਾਰ(longdouble_t)
};
/* Ensure that for each member of 'enum size_spec' there is an
   initializer in the width_bytes array.  */
ਬਣਤਰ ERR_ਚੌੜਾਈ_ਬਾਈਟਾਂ_has_bad_ਮਾਪ {
	ਅੱਖਰ ERR_ਚੌੜਾਈ_ਬਾਈਟਾਂ_has_bad_ਮਾਪ[ARRAY_ਮਾਪ(ਚੌੜਾਈ_ਬਾਈਟਾਂ) == N_ਮਾਪ_SPECS ? 1 : -1];
};

ਬਣਤਰ globals {
	smallint exit_ਕੋਡ;

	ਬਿਨਾਨਿਸ਼ਾਨੀ string_min;

	/* An array of specs describing how to format each input block.  */
	ਬਿਨਾਨਿਸ਼ਾਨੀ n_specs;
	ਬਣਤਰ tspec *spec;

	/* Function that accepts an address and an optional following char,
	   and prints the address and char to stdout.  */
	ਖਾਲੀ (*format_ਪਤਾ)(off_t, ਅੱਖਰ);

	/* The difference between the old-style pseudo starting address and
	   the number of bytes to skip.  */
#ਜੇ ENABLE_LONG_OPTS
	off_t pseudo_ਵਿੱਥ;
# define G_pseudo_ਵਿੱਥ G.pseudo_ਵਿੱਥ
#ਜੇ_ਅੰਤ
	/* When zero, MAX_BYTES_TO_FORMAT and END_OFFSET are ignored, and all
	   input is formatted.  */

	/* The number of input bytes formatted per output line.  It must be
	   a multiple of the least common multiple of the sizes associated with
	   the specified output types.  It should be as large as possible, but
	   no larger than 16 -- unless specified with the -w option.  */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਖੰਡ; /* have to use unsigned, not size_t */

	/* A NULL-terminated list of the file-arguments from the command line.  */
	ਅਚਲ ਅੱਖਰ *ਅਚਲ *file_list;

	/* The input stream associated with the current file.  */
	FILE *ਵਿੱਚ_ਧਾਰਾ;

	bool not_ਪਹਿਲਾ;
	bool prev_pair_equal;

	ਅੱਖਰ ਪਤਾ_fmt[ਆਕਾਰ("%0n"OFF_FMT"xc")];
} FIX_ALIASING;
/* Corresponds to 'x' above */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪਤਾ_ਅਧਾਰ_char G.ਪਤਾ_fmt[ਆਕਾਰ(G.ਪਤਾ_fmt)-3]
/* Corresponds to 'n' above */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪਤਾ_pad_len_char G.ਪਤਾ_fmt[2]

#ਜੇ !ENABLE_LONG_OPTS
ਗਿਣਤੀ { G_pseudo_ਵਿੱਥ = 0 };
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ G (*(ਬਣਤਰ globals*)bb_common_bufsiz1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INIT_G() ਕਰੋ { \
	ਤਿਆਰੀ_common_bufsiz(); \
	BUILD_BUG_ON(ਆਕਾਰ(G) > COMMON_BUFSIZE); \
	G.ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਖੰਡ = 32; \
	strcpy(G.ਪਤਾ_fmt, "%0n"OFF_FMT"xc"); \
} ਜਦੋਂ (0)


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_INTEGRAL_TYPE_ਮਾਪ ਆਕਾਰ(ulonglong_t)
ਸਥਿਰ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ integral_type_ਮਾਪ[MAX_INTEGRAL_TYPE_ਮਾਪ + 1] ALIGN1 = {
	[ਆਕਾਰ(ਅੱਖਰ)] = CHAR,
#ਜੇ USHRT_MAX != UCHAR_MAX
	[ਆਕਾਰ(ਛੋਟਾ)] = SHORT,
#ਜੇ_ਅੰਤ
#ਜੇ UINT_MAX != USHRT_MAX
	[ਆਕਾਰ(ਪੂਰਨ)] = INT,
#ਜੇ_ਅੰਤ
#ਜੇ ULONG_MAX != UINT_MAX
	[ਆਕਾਰ(ਲੰਮਾ)] = LONG,
#ਜੇ_ਅੰਤ
#ਜੇ ULLONG_MAX != ULONG_MAX
	[ਆਕਾਰ(ulonglong_t)] = LONG_LONG,
#ਜੇ_ਅੰਤ
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_FP_TYPE_ਮਾਪ ਆਕਾਰ(longdouble_t)
ਸਥਿਰ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ fp_type_ਮਾਪ[MAX_FP_TYPE_ਮਾਪ + 1] ALIGN1 = {
	/* gcc seems to allow repeated indexes. Last one wins */
	[ਆਕਾਰ(longdouble_t)] = FLOAT_LONG_DOUBLE,
	[ਆਕਾਰ(ਦੁੱਗਣਾ)] = FLOAT_DOUBLE,
	[ਆਕਾਰ(ਦਸ਼ਮਲਵ)] = FLOAT_SINGLE
};


ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ
ਮਹੱਤਮ_ਸਾਂਝਾ_ਭਾਜਕ(ਬਿਨਾਨਿਸ਼ਾਨੀ u, ਬਿਨਾਨਿਸ਼ਾਨੀ v)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ t;
	ਜਦੋਂ (v != 0) {
		t = u % v;
		u = v;
		v = t;
	}
	ਵਾਪਸ u;
}

/* Compute the least common multiple of U and V.  */
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ
lcm(ਬਿਨਾਨਿਸ਼ਾਨੀ u, ਬਿਨਾਨਿਸ਼ਾਨੀ v) {
	ਬਿਨਾਨਿਸ਼ਾਨੀ t = ਮਹੱਤਮ_ਸਾਂਝਾ_ਭਾਜਕ(u, v);
	ਜੇ (t == 0)
		ਵਾਪਸ 0;
	ਵਾਪਸ u * v / t;
}

ਸਥਿਰ ਖਾਲੀ
print_s_char(ਮਾਪ_t n_ਬਾਈਟਾਂ, ਅਚਲ ਅੱਖਰ *ਖੰਡ, ਅਚਲ ਅੱਖਰ *fmt_string)
{
	ਜਦੋਂ (n_ਬਾਈਟਾਂ--) {
		ਪੂਰਨ tmp = *(ਨਿਸ਼ਾਨੀਵਾਲਾ ਅੱਖਰ *) ਖੰਡ;
		ਲਿਖੋ(fmt_string, tmp);
		ਖੰਡ += ਆਕਾਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ);
	}
}

ਸਥਿਰ ਖਾਲੀ
print_char(ਮਾਪ_t n_ਬਾਈਟਾਂ, ਅਚਲ ਅੱਖਰ *ਖੰਡ, ਅਚਲ ਅੱਖਰ *fmt_string)
{
	ਜਦੋਂ (n_ਬਾਈਟਾਂ--) {
		ਬਿਨਾਨਿਸ਼ਾਨੀ tmp = *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *) ਖੰਡ;
		ਲਿਖੋ(fmt_string, tmp);
		ਖੰਡ += ਆਕਾਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ);
	}
}

ਸਥਿਰ ਖਾਲੀ
print_s_short(ਮਾਪ_t n_ਬਾਈਟਾਂ, ਅਚਲ ਅੱਖਰ *ਖੰਡ, ਅਚਲ ਅੱਖਰ *fmt_string)
{
	n_ਬਾਈਟਾਂ /= ਆਕਾਰ(ਨਿਸ਼ਾਨੀਵਾਲਾ ਛੋਟਾ);
	ਜਦੋਂ (n_ਬਾਈਟਾਂ--) {
		ਪੂਰਨ tmp = *(ਨਿਸ਼ਾਨੀਵਾਲਾ ਛੋਟਾ *) ਖੰਡ;
		ਲਿਖੋ(fmt_string, tmp);
		ਖੰਡ += ਆਕਾਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ);
	}
}

ਸਥਿਰ ਖਾਲੀ
print_short(ਮਾਪ_t n_ਬਾਈਟਾਂ, ਅਚਲ ਅੱਖਰ *ਖੰਡ, ਅਚਲ ਅੱਖਰ *fmt_string)
{
	n_ਬਾਈਟਾਂ /= ਆਕਾਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ);
	ਜਦੋਂ (n_ਬਾਈਟਾਂ--) {
		ਬਿਨਾਨਿਸ਼ਾਨੀ tmp = *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ *) ਖੰਡ;
		ਲਿਖੋ(fmt_string, tmp);
		ਖੰਡ += ਆਕਾਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ);
	}
}

ਸਥਿਰ ਖਾਲੀ
print_int(ਮਾਪ_t n_ਬਾਈਟਾਂ, ਅਚਲ ਅੱਖਰ *ਖੰਡ, ਅਚਲ ਅੱਖਰ *fmt_string)
{
	n_ਬਾਈਟਾਂ /= ਆਕਾਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ);
	ਜਦੋਂ (n_ਬਾਈਟਾਂ--) {
		ਬਿਨਾਨਿਸ਼ਾਨੀ tmp = *(ਬਿਨਾਨਿਸ਼ਾਨੀ *) ਖੰਡ;
		ਲਿਖੋ(fmt_string, tmp);
		ਖੰਡ += ਆਕਾਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ);
	}
}

#ਜੇ UINT_MAX == ULONG_MAX
# define print_long print_int
#ਨਹੀਂਤਾਂ
ਸਥਿਰ ਖਾਲੀ
print_long(ਮਾਪ_t n_ਬਾਈਟਾਂ, ਅਚਲ ਅੱਖਰ *ਖੰਡ, ਅਚਲ ਅੱਖਰ *fmt_string)
{
	n_ਬਾਈਟਾਂ /= ਆਕਾਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ);
	ਜਦੋਂ (n_ਬਾਈਟਾਂ--) {
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ tmp = *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *) ਖੰਡ;
		ਲਿਖੋ(fmt_string, tmp);
		ਖੰਡ += ਆਕਾਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ);
	}
}
#ਜੇ_ਅੰਤ

#ਜੇ ULONG_MAX == ULLONG_MAX
# define print_long_long print_long
#ਨਹੀਂਤਾਂ
ਸਥਿਰ ਖਾਲੀ
print_long_long(ਮਾਪ_t n_ਬਾਈਟਾਂ, ਅਚਲ ਅੱਖਰ *ਖੰਡ, ਅਚਲ ਅੱਖਰ *fmt_string)
{
	n_ਬਾਈਟਾਂ /= ਆਕਾਰ(ulonglong_t);
	ਜਦੋਂ (n_ਬਾਈਟਾਂ--) {
		ulonglong_t tmp = *(ulonglong_t *) ਖੰਡ;
		ਲਿਖੋ(fmt_string, tmp);
		ਖੰਡ += ਆਕਾਰ(ulonglong_t);
	}
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਖਾਲੀ
print_float(ਮਾਪ_t n_ਬਾਈਟਾਂ, ਅਚਲ ਅੱਖਰ *ਖੰਡ, ਅਚਲ ਅੱਖਰ *fmt_string)
{
	n_ਬਾਈਟਾਂ /= ਆਕਾਰ(ਦਸ਼ਮਲਵ);
	ਜਦੋਂ (n_ਬਾਈਟਾਂ--) {
		ਦਸ਼ਮਲਵ tmp = *(ਦਸ਼ਮਲਵ *) ਖੰਡ;
		ਲਿਖੋ(fmt_string, tmp);
		ਖੰਡ += ਆਕਾਰ(ਦਸ਼ਮਲਵ);
	}
}

ਸਥਿਰ ਖਾਲੀ
print_double(ਮਾਪ_t n_ਬਾਈਟਾਂ, ਅਚਲ ਅੱਖਰ *ਖੰਡ, ਅਚਲ ਅੱਖਰ *fmt_string)
{
	n_ਬਾਈਟਾਂ /= ਆਕਾਰ(ਦੁੱਗਣਾ);
	ਜਦੋਂ (n_ਬਾਈਟਾਂ--) {
		ਦੁੱਗਣਾ tmp = *(ਦੁੱਗਣਾ *) ਖੰਡ;
		ਲਿਖੋ(fmt_string, tmp);
		ਖੰਡ += ਆਕਾਰ(ਦੁੱਗਣਾ);
	}
}

ਸਥਿਰ ਖਾਲੀ
print_long_double(ਮਾਪ_t n_ਬਾਈਟਾਂ, ਅਚਲ ਅੱਖਰ *ਖੰਡ, ਅਚਲ ਅੱਖਰ *fmt_string)
{
	n_ਬਾਈਟਾਂ /= ਆਕਾਰ(longdouble_t);
	ਜਦੋਂ (n_ਬਾਈਟਾਂ--) {
		longdouble_t tmp = *(longdouble_t *) ਖੰਡ;
		ਲਿਖੋ(fmt_string, tmp);
		ਖੰਡ += ਆਕਾਰ(longdouble_t);
	}
}

/* print_[named]_ascii are optimized for speed.
 * Remember, someday you may want to pump gigabytes through this thing.
 * Saving a dozen of .text bytes here is counter-productive */

ਸਥਿਰ ਖਾਲੀ
print_named_ascii(ਮਾਪ_t n_ਬਾਈਟਾਂ, ਅਚਲ ਅੱਖਰ *ਖੰਡ,
		ਅਚਲ ਅੱਖਰ *ਅਣਵਰਤਿਆ_fmt_string ਅਣਵਰਤਿਆ_PARAM)
{
	/* Names for some non-printing characters.  */
	ਸਥਿਰ ਅਚਲ ਅੱਖਰ charname[33][3] ALIGN1 = {
		"nul", "soh", "stx", "etx", "eot", "enq", "ack", "bel",
		" bs", " ht", " nl", " vt", " ff", " cr", " so", " si",
		"dle", "dc1", "dc2", "dc3", "dc4", "nak", "syn", "etb",
		"can", " em", "sub", "esc", " fs", " gs", " rs", " us",
		" sp"
	};
	// buf[N] pos:  01234 56789
	ਅੱਖਰ buf[12] = "   x\0 xxx\0";
	// [12] because we take three 32bit stack slots anyway, and
	// gcc is too dumb to initialize with constant stores,
	// it copies initializer from rodata. Oh well.
	// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=65410

	ਜਦੋਂ (n_ਬਾਈਟਾਂ--) {
		ਬਿਨਾਨਿਸ਼ਾਨੀ masked_c = *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *) ਖੰਡ++;

		masked_c &= 0x7f;
		ਜੇ (masked_c == 0x7f) {
			fputs_stdout(" del");
			ਜਾਰੀ;
		}
		ਜੇ (masked_c > ' ') {
			buf[3] = masked_c;
			fputs_stdout(buf);
			ਜਾਰੀ;
		}
		/* Why? Because printf(" %3.3s") is much slower... */
		buf[6] = charname[masked_c][0];
		buf[7] = charname[masked_c][1];
		buf[8] = charname[masked_c][2];
		fputs_stdout(buf+5);
	}
}

ਸਥਿਰ ਖਾਲੀ
print_ascii(ਮਾਪ_t n_ਬਾਈਟਾਂ, ਅਚਲ ਅੱਖਰ *ਖੰਡ,
		ਅਚਲ ਅੱਖਰ *ਅਣਵਰਤਿਆ_fmt_string ਅਣਵਰਤਿਆ_PARAM)
{
	// buf[N] pos:  01234 56789
	ਅੱਖਰ buf[12] = "   x\0 xxx\0";

	ਜਦੋਂ (n_ਬਾਈਟਾਂ--) {
		ਅਚਲ ਅੱਖਰ *s;
		ਬਿਨਾਨਿਸ਼ਾਨੀ c = *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *) ਖੰਡ++;

		ਜੇ (ISPRINT(c)) {
			buf[3] = c;
			fputs_stdout(buf);
			ਜਾਰੀ;
		}
		ਚੋਣ (c) {
		ਹਾਲਤ '\0':
			s = "  \\0";
			ਰੋਕੋ;
		ਹਾਲਤ '\007':
			s = "  \\a";
			ਰੋਕੋ;
		ਹਾਲਤ '\b':
			s = "  \\b";
			ਰੋਕੋ;
		ਹਾਲਤ '\f':
			s = "  \\f";
			ਰੋਕੋ;
		ਹਾਲਤ '\n':
			s = "  \\n";
			ਰੋਕੋ;
		ਹਾਲਤ '\r':
			s = "  \\r";
			ਰੋਕੋ;
		ਹਾਲਤ '\t':
			s = "  \\t";
			ਰੋਕੋ;
		ਹਾਲਤ '\v':
			s = "  \\v";
			ਰੋਕੋ;
		ਮੂਲ:
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

ਸਥਿਰ ਖਾਲੀ
open_ਅਗਲਾ_file(ਖਾਲੀ)
{
	ਜਦੋਂ (1) {
		ਜੇ (!*G.file_list)
			ਵਾਪਸ;
		G.ਵਿੱਚ_ਧਾਰਾ = fopen_or_warn_stdin(*G.file_list++);
		ਜੇ (G.ਵਿੱਚ_ਧਾਰਾ) {
			ਰੋਕੋ;
		}
		G.exit_ਕੋਡ = 1;
	}

	ਜੇ ((option_mask32 & (OPT_N|OPT_S)) == OPT_N)
		setbuf(G.ਵਿੱਚ_ਧਾਰਾ, ਨਿਰਾ);
}

/* Test whether there have been errors on in_stream, and close it if
   it is not standard input.  Return nonzero if there has been an error
   on in_stream or stdout; return zero otherwise.  This function will
   report more than one error only if both a read and a write error
   have occurred.  IN_ERRNO, if nonzero, is the error number
   corresponding to the most recent action for IN_STREAM.  */

ਸਥਿਰ ਖਾਲੀ
check_and_close(ਖਾਲੀ)
{
	ਜੇ (G.ਵਿੱਚ_ਧਾਰਾ) {
		ਜੇ (ferror(G.ਵਿੱਚ_ਧਾਰਾ))	{
			bb_error_ਸੁਨੇਹਾ("%s: read error", (G.ਵਿੱਚ_ਧਾਰਾ == stdin)
					? bb_ਸੁਨੇਹਾ_standard_ਨਿਵੇਸ਼
					: G.file_list[-1]
			);
			G.exit_ਕੋਡ = 1;
		}
		fclose_if_not_stdin(G.ਵਿੱਚ_ਧਾਰਾ);
		G.ਵਿੱਚ_ਧਾਰਾ = ਨਿਰਾ;
	}

	ਜੇ (ferror(stdout)) {
		bb_simple_error_ਸੁਨੇਹਾ_and_die(bb_ਸੁਨੇਹਾ_write_error);
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

ਸਥਿਰ NOINLINE ਅਚਲ ਅੱਖਰ *
decode_one_format(ਅਚਲ ਅੱਖਰ *s_orig, ਅਚਲ ਅੱਖਰ *s, ਬਣਤਰ tspec *tspec)
{
	ਗਿਣਤੀ ਮਾਪ_spec ਮਾਪ_spec;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ;
	ਗਿਣਤੀ output_format fmt;
	ਅਚਲ ਅੱਖਰ *p;
	ਅੱਖਰ *ਸਿਰਾ;
	ਅੱਖਰ *fmt_string = ਨਿਰਾ;
	ਖਾਲੀ (*print_function) (ਮਾਪ_t, ਅਚਲ ਅੱਖਰ *, ਅਚਲ ਅੱਖਰ *);
	ਬਿਨਾਨਿਸ਼ਾਨੀ c;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਖੇਤਰ_ਚੌੜਾਈ = 0;
	ਪੂਰਨ pos;

	ਚੋਣ (*s) {
	ਹਾਲਤ 'd':
	ਹਾਲਤ 'o':
	ਹਾਲਤ 'u':
	ਹਾਲਤ 'x': {
		ਸਥਿਰ ਅਚਲ ਅੱਖਰ CSIL[] ALIGN1 = "CSIL";

		c = *s++;
		p = strchr(CSIL, *s);
		/* if *s == NUL, p != NULL! Testcase: "od -tx" */
		ਜੇ (!p || *p == '\0') {
			ਮਾਪ = ਆਕਾਰ(ਪੂਰਨ);
			ਜੇ (ਅੰਕ_ਹੈ(s[0])) {
				ਮਾਪ = bb_strtou(s, &ਸਿਰਾ, 0);
				ਜੇ (errno == ERANGE
				 || MAX_INTEGRAL_TYPE_ਮਾਪ < ਮਾਪ
				 || integral_type_ਮਾਪ[ਮਾਪ] == NO_ਮਾਪ
				) {
					bb_error_ਸੁਨੇਹਾ_and_die("invalid type string '%s'; "
						"%u-byte %s type is not supported",
						s_orig, ਮਾਪ, "integral");
				}
				s = ਸਿਰਾ;
			}
		} ਨਹੀਂਤਾਂ {
			ਸਥਿਰ ਅਚਲ uint8_t CSIL_sizeof[4] = {
				ਆਕਾਰ(ਅੱਖਰ),
				ਆਕਾਰ(ਛੋਟਾ),
				ਆਕਾਰ(ਪੂਰਨ),
				ਆਕਾਰ(ਲੰਮਾ),
			};
			ਮਾਪ = CSIL_sizeof[p - CSIL];
			s++; /* skip C/S/I/L */
		}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ISPEC_ਮੰਜ਼ਿਲ_FORMAT(Spec, Min_format, Long_format, Max_format) \
	((Spec) == LONG_LONG ? (Max_format) \
	: ((Spec) == LONG ? (Long_format) : (Min_format)))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FMT_ਬਾਈਟਾਂ_ALLOCATED 9
		ਮਾਪ_spec = integral_type_ਮਾਪ[ਮਾਪ];

		{
			ਸਥਿਰ ਅਚਲ ਅੱਖਰ doux[] ALIGN1 = "doux";
			ਸਥਿਰ ਅਚਲ ਅੱਖਰ doux_fmt_letter[][4] = {
				"lld", "llo", "llu", "llx"
			};
			ਸਥਿਰ ਅਚਲ ਗਿਣਤੀ output_format doux_fmt[] = {
				SIGNED_DECIMAL,
				OCTAL,
				UNSIGNED_DECIMAL,
				HEXADECIMAL,
			};
			ਸਥਿਰ ਅਚਲ uint8_t *ਅਚਲ doux_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_XXX[] = {
				ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_signed_dec_digits,
				ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_oct_digits,
				ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_unsigned_dec_digits,
				ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_hex_digits,
			};
			ਸਥਿਰ ਅਚਲ ਅੱਖਰ doux_fmtstring[][ਆਕਾਰ(" %%0%u%s")] ALIGN1 = {
				" %%%u%s",
				" %%0%u%s",
				" %%%u%s",
				" %%0%u%s",
			};

			pos = strchr(doux, c) - doux;
			fmt = doux_fmt[pos];
			ਖੇਤਰ_ਚੌੜਾਈ = doux_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_XXX[pos][ਮਾਪ];
			p = doux_fmt_letter[pos] + 2;
			ਜੇ (ਮਾਪ_spec == LONG) p--;
			ਜੇ (ਮਾਪ_spec == LONG_LONG) p -= 2;
			fmt_string = xasprintf(doux_fmtstring[pos], ਖੇਤਰ_ਚੌੜਾਈ, p);
		}

		ਚੋਣ (ਮਾਪ_spec) {
		ਹਾਲਤ CHAR:
			print_function = (fmt == SIGNED_DECIMAL
				    ? print_s_char
				    : print_char);
			ਰੋਕੋ;
		ਹਾਲਤ SHORT:
			print_function = (fmt == SIGNED_DECIMAL
				    ? print_s_short
				    : print_short);
			ਰੋਕੋ;
		ਹਾਲਤ INT:
			print_function = print_int;
			ਰੋਕੋ;
		ਹਾਲਤ LONG:
			print_function = print_long;
			ਰੋਕੋ;
		ਮੂਲ: /* case LONG_LONG: */
			print_function = print_long_long;
			ਰੋਕੋ;
		}
		ਰੋਕੋ;
	}

	ਹਾਲਤ 'f': {
		ਸਥਿਰ ਅਚਲ ਅੱਖਰ FDL[] ALIGN1 = "FDL";

		fmt = FLOATING_ਬਿੰਦੂ;
		++s;
		p = strchr(FDL, *s);
		ਜੇ (!p || *p == '\0') {
			ਮਾਪ = ਆਕਾਰ(ਦੁੱਗਣਾ);
			ਜੇ (ਅੰਕ_ਹੈ(s[0])) {
				ਮਾਪ = bb_strtou(s, &ਸਿਰਾ, 0);
				ਜੇ (errno == ERANGE || ਮਾਪ > MAX_FP_TYPE_ਮਾਪ
				 || fp_type_ਮਾਪ[ਮਾਪ] == NO_ਮਾਪ
				) {
					bb_error_ਸੁਨੇਹਾ_and_die("invalid type string '%s'; "
						"%u-byte %s type is not supported",
						s_orig, ਮਾਪ, "floating point");
				}
				s = ਸਿਰਾ;
			}
		} ਨਹੀਂਤਾਂ {
			ਸਥਿਰ ਅਚਲ uint8_t FDL_sizeof[] = {
				ਆਕਾਰ(ਦਸ਼ਮਲਵ),
				ਆਕਾਰ(ਦੁੱਗਣਾ),
				ਆਕਾਰ(longdouble_t),
			};

			ਮਾਪ = FDL_sizeof[p - FDL];
			s++; /* skip F/D/L */
		}

		ਮਾਪ_spec = fp_type_ਮਾਪ[ਮਾਪ];

		ਚੋਣ (ਮਾਪ_spec) {
		ਹਾਲਤ FLOAT_SINGLE:
			print_function = print_float;
			ਖੇਤਰ_ਚੌੜਾਈ = FLT_DIG + 8;
			/* Don't use %#e; not all systems support it.  */
			fmt_string = xasprintf(" %%%d.%de", ਖੇਤਰ_ਚੌੜਾਈ, FLT_DIG);
			ਰੋਕੋ;
		ਹਾਲਤ FLOAT_DOUBLE:
			print_function = print_double;
			ਖੇਤਰ_ਚੌੜਾਈ = DBL_DIG + 8;
			fmt_string = xasprintf(" %%%d.%de", ਖੇਤਰ_ਚੌੜਾਈ, DBL_DIG);
			ਰੋਕੋ;
		ਮੂਲ: /* case FLOAT_LONG_DOUBLE: */
			print_function = print_long_double;
			ਖੇਤਰ_ਚੌੜਾਈ = LDBL_DIG + 8;
			fmt_string = xasprintf(" %%%d.%dLe", ਖੇਤਰ_ਚੌੜਾਈ, LDBL_DIG);
			ਰੋਕੋ;
		}
		ਰੋਕੋ;
	}

	ਹਾਲਤ 'a':
		++s;
		fmt = NAMED_CHARACTER;
		ਮਾਪ_spec = CHAR;
		print_function = print_named_ascii;
		ਖੇਤਰ_ਚੌੜਾਈ = 3;
		ਰੋਕੋ;
	ਹਾਲਤ 'c':
		++s;
		fmt = CHARACTER;
		ਮਾਪ_spec = CHAR;
		print_function = print_ascii;
		ਖੇਤਰ_ਚੌੜਾਈ = 3;
		ਰੋਕੋ;
	ਮੂਲ:
		bb_error_ਸੁਨੇਹਾ_and_die("invalid character '%c' "
				"in type string '%s'", *s, s_orig);
	}

	tspec->ਮਾਪ = ਮਾਪ_spec;
	tspec->fmt = fmt;
	tspec->print_function = print_function;
	tspec->fmt_string = fmt_string;

	tspec->ਖੇਤਰ_ਚੌੜਾਈ = ਖੇਤਰ_ਚੌੜਾਈ;
	tspec->hexl_ਬਹੁਲਕ_trailer = (*s == 'z');
	ਜੇ (tspec->hexl_ਬਹੁਲਕ_trailer)
		s++;

	ਵਾਪਸ s;
}

/* Decode the modern od format string S.  Append the decoded
   representation to the global array SPEC, reallocating SPEC if
   necessary.  */

ਸਥਿਰ ਖਾਲੀ
decode_format_string(ਅਚਲ ਅੱਖਰ *s)
{
	ਅਚਲ ਅੱਖਰ *s_orig = s;

	ਜਦੋਂ (*s != '\0') {
		ਬਣਤਰ tspec tspec;
		ਅਚਲ ਅੱਖਰ *ਅਗਲਾ;

		ਅਗਲਾ = decode_one_format(s_orig, s, &tspec);

		ਪ੍ਰਤਿਜ੍ਞਾ(s != ਅਗਲਾ);
		s = ਅਗਲਾ;
		G.spec = xrealloc_ਸਦਿਸ਼(G.spec, 4, G.n_specs);
		ਸਮ੍ਰਿਤੀ_ਨਕਲ(&G.spec[G.n_specs], &tspec, ਆਕਾਰ(G.spec[0]));
		G.n_specs++;
	}
}

/* Given a list of one or more input filenames FILE_LIST, set the global
   file pointer IN_STREAM to position N_SKIP in the concatenation of
   those files.  If any file operation fails or if there are fewer than
   N_SKIP bytes in the combined input, give an error message and return
   nonzero.  When possible, use seek rather than read operations to
   advance IN_STREAM.  */

ਸਥਿਰ ਖਾਲੀ
skip(off_t n_skip)
{
	ਜੇ (n_skip == 0)
		ਵਾਪਸ;

	ਜਦੋਂ (G.ਵਿੱਚ_ਧਾਰਾ) { /* !EOF */
		ਬਣਤਰ stat file_stats;

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
		ਜੇ (fstat(fileno(G.ਵਿੱਚ_ਧਾਰਾ), &file_stats) == 0
		 && S_ISREG(file_stats.st_ਬਹੁਲਕ) && file_stats.st_ਮਾਪ > 0
		) {
			ਜੇ (file_stats.st_ਮਾਪ < n_skip) {
				n_skip -= file_stats.st_ਮਾਪ;
				/* take "check & close / open_next" route */
			} ਨਹੀਂਤਾਂ {
				ਜੇ (fseeko(G.ਵਿੱਚ_ਧਾਰਾ, n_skip, SEEK_CUR) != 0)
					G.exit_ਕੋਡ = 1;
				ਵਾਪਸ;
			}
		} ਨਹੀਂਤਾਂ {
			/* If it's not a regular file with positive size,
			   position the file pointer by reading.  */
			ਅੱਖਰ buf[1024];
			ਮਾਪ_t n_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_read = 1024;
			ਮਾਪ_t n_ਬਾਈਟਾਂ_read;

			ਜਦੋਂ (n_skip > 0) {
				ਜੇ (n_skip < n_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_read)
					n_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_read = n_skip;
				n_ਬਾਈਟਾਂ_read = fread(buf, 1, n_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_read, G.ਵਿੱਚ_ਧਾਰਾ);
				n_skip -= n_ਬਾਈਟਾਂ_read;
				ਜੇ (n_ਬਾਈਟਾਂ_read != n_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_read)
					ਰੋਕੋ; /* EOF on this file or error */
			}
		}
		ਜੇ (n_skip == 0)
			ਵਾਪਸ;

		check_and_close();
		open_ਅਗਲਾ_file();
	}

	ਜੇ (n_skip)
		bb_simple_error_ਸੁਨੇਹਾ_and_die("can't skip past end of combined input");
}


ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਖਾਲੀ FN_format_ਪਤਾ(off_t ਪਤਾ, ਅੱਖਰ c);

ਸਥਿਰ ਖਾਲੀ
format_ਪਤਾ_none(off_t ਪਤਾ ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ c ਅਣਵਰਤਿਆ_PARAM)
{
}

ਸਥਿਰ ਖਾਲੀ
format_ਪਤਾ_std(off_t ਪਤਾ, ਅੱਖਰ c)
{
	/* Corresponds to 'c' */
	G.ਪਤਾ_fmt[ਆਕਾਰ(G.ਪਤਾ_fmt)-2] = c;
	ਲਿਖੋ(G.ਪਤਾ_fmt, ਪਤਾ);
}

#ਜੇ ENABLE_LONG_OPTS
/* only used with --traditional */
ਸਥਿਰ ਖਾਲੀ
format_ਪਤਾ_paren(off_t ਪਤਾ, ਅੱਖਰ c)
{
	ਅੱਖਰ_ਲਿਖੋ('(');
	format_ਪਤਾ_std(ਪਤਾ, ')');
	ਜੇ (c) ਅੱਖਰ_ਲਿਖੋ(c);
}

ਸਥਿਰ ਖਾਲੀ
format_ਪਤਾ_label(off_t ਪਤਾ, ਅੱਖਰ c)
{
	format_ਪਤਾ_std(ਪਤਾ, ' ');
	format_ਪਤਾ_paren(ਪਤਾ + G_pseudo_ਵਿੱਥ, c);
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਖਾਲੀ
dump_hexl_ਬਹੁਲਕ_trailer(ਮਾਪ_t n_ਬਾਈਟਾਂ, ਅਚਲ ਅੱਖਰ *ਖੰਡ)
{
	fputs_stdout("  >");
	ਜਦੋਂ (n_ਬਾਈਟਾਂ--) {
		ਬਿਨਾਨਿਸ਼ਾਨੀ c = *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *) ਖੰਡ++;
		c = (ISPRINT(c) ? c : '.');
		ਅੱਖਰ_ਲਿਖੋ(c);
	}
	ਅੱਖਰ_ਲਿਖੋ('<');
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

ਸਥਿਰ ਖਾਲੀ
write_ਖੰਡ(off_t current_ਵਿੱਥ, ਮਾਪ_t n_ਬਾਈਟਾਂ,
		ਅਚਲ ਅੱਖਰ *prev_ਖੰਡ, ਅਚਲ ਅੱਖਰ *curr_ਖੰਡ)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ i;

	ਜੇ (!(option_mask32 & OPT_v)
	 && G.not_ਪਹਿਲਾ
	 && n_ਬਾਈਟਾਂ == G.ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਖੰਡ
	 && memcmp(prev_ਖੰਡ, curr_ਖੰਡ, G.ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਖੰਡ) == 0
	) {
		ਜੇ (G.prev_pair_equal) {
			/* The two preceding blocks were equal, and the current
			   block is the same as the last one, so print nothing.  */
		} ਨਹੀਂਤਾਂ {
			puts("*");
			G.prev_pair_equal = 1;
		}
	} ਨਹੀਂਤਾਂ {
		G.not_ਪਹਿਲਾ = 1;
		G.prev_pair_equal = 0;
		ਲਈ (i = 0; i < G.n_specs; i++) {
			ਜੇ (i == 0)
				G.format_ਪਤਾ(current_ਵਿੱਥ, '\0');
			ਨਹੀਂਤਾਂ
				ਲਿਖੋ("%*s", ਪਤਾ_pad_len_char - '0', "");
			(*G.spec[i].print_function) (n_ਬਾਈਟਾਂ, curr_ਖੰਡ, G.spec[i].fmt_string);
			ਜੇ (G.spec[i].hexl_ਬਹੁਲਕ_trailer) {
				/* space-pad out to full line width, then dump the trailer */
				ਬਿਨਾਨਿਸ਼ਾਨੀ datum_ਚੌੜਾਈ = ਚੌੜਾਈ_ਬਾਈਟਾਂ[G.spec[i].ਮਾਪ];
				ਬਿਨਾਨਿਸ਼ਾਨੀ blank_fields = (G.ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਖੰਡ - n_ਬਾਈਟਾਂ) / datum_ਚੌੜਾਈ;
				ਬਿਨਾਨਿਸ਼ਾਨੀ ਖੇਤਰ_ਚੌੜਾਈ = G.spec[i].ਖੇਤਰ_ਚੌੜਾਈ + 1;
				ਲਿਖੋ("%*s", blank_fields * ਖੇਤਰ_ਚੌੜਾਈ, "");
				dump_hexl_ਬਹੁਲਕ_trailer(n_ਬਾਈਟਾਂ, curr_ਖੰਡ);
			}
			ਅੱਖਰ_ਲਿਖੋ('\n');
		}
	}
}

ਸਥਿਰ ਖਾਲੀ
read_ਖੰਡ(ਮਾਪ_t n, ਅੱਖਰ *ਖੰਡ, ਮਾਪ_t *n_ਬਾਈਟਾਂ_ਵਿੱਚ_ਬਫਰ)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(0 < n && n <= G.ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਖੰਡ);

	*n_ਬਾਈਟਾਂ_ਵਿੱਚ_ਬਫਰ = 0;

	ਜੇ (n == 0)
		ਵਾਪਸ;

	ਜਦੋਂ (G.ਵਿੱਚ_ਧਾਰਾ != ਨਿਰਾ) { /* EOF.  */
		ਮਾਪ_t n_needed;
		ਮਾਪ_t n_read;

		n_needed = n - *n_ਬਾਈਟਾਂ_ਵਿੱਚ_ਬਫਰ;
		n_read = fread(ਖੰਡ + *n_ਬਾਈਟਾਂ_ਵਿੱਚ_ਬਫਰ, 1, n_needed, G.ਵਿੱਚ_ਧਾਰਾ);
		*n_ਬਾਈਟਾਂ_ਵਿੱਚ_ਬਫਰ += n_read;
		ਜੇ (n_read == n_needed)
			ਰੋਕੋ;
		/* error check is done in check_and_close */
		check_and_close();
		open_ਅਗਲਾ_file();
	}
}

/* Return the least common multiple of the sizes associated
   with the format specs.  */

ਸਥਿਰ ਪੂਰਨ
get_lcm(ਖਾਲੀ)
{
	ਮਾਪ_t i;
	ਪੂਰਨ l_c_m = 1;

	ਲਈ (i = 0; i < G.n_specs; i++)
		l_c_m = lcm(l_c_m, ਚੌੜਾਈ_ਬਾਈਟਾਂ[(ਪੂਰਨ) G.spec[i].ਮਾਪ]);
	ਵਾਪਸ l_c_m;
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

ਸਥਿਰ ਖਾਲੀ
dump(off_t current_ਵਿੱਥ, off_t ਸਿਰਾ_ਵਿੱਥ)
{
	ਅੱਖਰ *ਖੰਡ[2];
	ਪੂਰਨ idx;
	ਮਾਪ_t n_ਬਾਈਟਾਂ_read;

	ਖੰਡ[0] = xmalloc(2 * G.ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਖੰਡ);
	ਖੰਡ[1] = ਖੰਡ[0] + G.ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਖੰਡ;

	idx = 0;
	ਜੇ (option_mask32 & OPT_N) {
		ਜਦੋਂ (1) {
			ਮਾਪ_t n_needed;
			ਜੇ (current_ਵਿੱਥ >= ਸਿਰਾ_ਵਿੱਥ) {
				n_ਬਾਈਟਾਂ_read = 0;
				ਰੋਕੋ;
			}
			n_needed = MIN(ਸਿਰਾ_ਵਿੱਥ - current_ਵਿੱਥ, (off_t) G.ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਖੰਡ);
			read_ਖੰਡ(n_needed, ਖੰਡ[idx], &n_ਬਾਈਟਾਂ_read);
			ਜੇ (n_ਬਾਈਟਾਂ_read < G.ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਖੰਡ)
				ਰੋਕੋ;
			ਪ੍ਰਤਿਜ੍ਞਾ(n_ਬਾਈਟਾਂ_read == G.ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਖੰਡ);
			write_ਖੰਡ(current_ਵਿੱਥ, n_ਬਾਈਟਾਂ_read, ਖੰਡ[idx ^ 1], ਖੰਡ[idx]);
			current_ਵਿੱਥ += n_ਬਾਈਟਾਂ_read;
			idx ^= 1;
		}
	} ਨਹੀਂਤਾਂ {
		ਜਦੋਂ (1) {
			read_ਖੰਡ(G.ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਖੰਡ, ਖੰਡ[idx], &n_ਬਾਈਟਾਂ_read);
			ਜੇ (n_ਬਾਈਟਾਂ_read < G.ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਖੰਡ)
				ਰੋਕੋ;
			ਪ੍ਰਤਿਜ੍ਞਾ(n_ਬਾਈਟਾਂ_read == G.ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਖੰਡ);
			write_ਖੰਡ(current_ਵਿੱਥ, n_ਬਾਈਟਾਂ_read, ਖੰਡ[idx ^ 1], ਖੰਡ[idx]);
			current_ਵਿੱਥ += n_ਬਾਈਟਾਂ_read;
			idx ^= 1;
		}
	}

	ਜੇ (n_ਬਾਈਟਾਂ_read > 0) {
		ਪੂਰਨ l_c_m;
		ਮਾਪ_t ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_write;

		l_c_m = get_lcm();

		/* Make bytes_to_write the smallest multiple of l_c_m that
		   is at least as large as n_bytes_read.  */
		ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_write = l_c_m * ((n_ਬਾਈਟਾਂ_read + l_c_m - 1) / l_c_m);

		memset(ਖੰਡ[idx] + n_ਬਾਈਟਾਂ_read, 0, ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_write - n_ਬਾਈਟਾਂ_read);
		write_ਖੰਡ(current_ਵਿੱਥ, ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_write,
				ਖੰਡ[idx ^ 1], ਖੰਡ[idx]);
		current_ਵਿੱਥ += n_ਬਾਈਟਾਂ_read;
	}

	G.format_ਪਤਾ(current_ਵਿੱਥ, '\n');

	ਜੇ ((option_mask32 & OPT_N) && current_ਵਿੱਥ >= ਸਿਰਾ_ਵਿੱਥ)
		check_and_close();

	ਮੁਕਤ(ਖੰਡ[0]);
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

ਸਥਿਰ ਖਾਲੀ
dump_ਲੜੀਆਂ(off_t ਪਤਾ, off_t ਸਿਰਾ_ਵਿੱਥ)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ bufsize = MAX(100, G.string_min);
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *buf = xmalloc(bufsize);

	ਜਦੋਂ (1) {
		ਮਾਪ_t i;
		ਪੂਰਨ c;

		/* See if the next 'G.string_min' chars are all printing chars.  */
 tryline:
		ਜੇ ((option_mask32 & OPT_N) && (ਸਿਰਾ_ਵਿੱਥ - G.string_min <= ਪਤਾ))
			ਰੋਕੋ;
		i = 0;
		ਜਦੋਂ (!(option_mask32 & OPT_N) || ਪਤਾ < ਸਿਰਾ_ਵਿੱਥ) {
			ਜੇ (i == bufsize) {
				bufsize += bufsize/8;
				buf = xrealloc(buf, bufsize);
			}

			ਜਦੋਂ (G.ਵਿੱਚ_ਧਾਰਾ) { /* !EOF */
				c = fgetc(G.ਵਿੱਚ_ਧਾਰਾ);
				ਜੇ (c != EOF)
					ਜਾਓ got_char;
				check_and_close();
				open_ਅਗਲਾ_file();
			}
			/* EOF */
			ਜਾਓ ret;
 got_char:
			ਪਤਾ++;
			ਜੇ (!c)
				ਰੋਕੋ;
			ਜੇ (!ISPRINT(c))
				ਜਾਓ tryline;	/* It isn't; give up on this string.  */
			buf[i++] = c;		/* String continues; store it all.  */
		}

		ਜੇ (i < G.string_min)		/* Too short! */
			ਜਾਓ tryline;

		/* If we get here, the string is all printable and NUL-terminated */
		buf[i] = 0;
		G.format_ਪਤਾ(ਪਤਾ - i - 1, ' ');

		ਲਈ (i = 0; (c = buf[i]); i++) {
			ਚੋਣ (c) {
			ਹਾਲਤ '\007': fputs_stdout("\\a"); ਰੋਕੋ;
			ਹਾਲਤ '\b': fputs_stdout("\\b"); ਰੋਕੋ;
			ਹਾਲਤ '\f': fputs_stdout("\\f"); ਰੋਕੋ;
			ਹਾਲਤ '\n': fputs_stdout("\\n"); ਰੋਕੋ;
			ਹਾਲਤ '\r': fputs_stdout("\\r"); ਰੋਕੋ;
			ਹਾਲਤ '\t': fputs_stdout("\\t"); ਰੋਕੋ;
			ਹਾਲਤ '\v': fputs_stdout("\\v"); ਰੋਕੋ;
			ਮੂਲ: ਅੱਖਰ_ਲਿਖੋ(c);
			}
		}
		ਅੱਖਰ_ਲਿਖੋ('\n');
	}

	/* We reach this point only if we search through
	   (max_bytes_to_format - G.string_min) bytes before reaching EOF.  */
	check_and_close();
 ret:
	ਮੁਕਤ(buf);
}

#ਜੇ ENABLE_LONG_OPTS
/* If S is a valid traditional offset specification with an optional
   leading '+' return nonzero and set *OFFSET to the offset it denotes.  */

ਸਥਿਰ ਪੂਰਨ
parse_old_ਵਿੱਥ(ਅਚਲ ਅੱਖਰ *s, off_t *ਵਿੱਥ)
{
	ਸਥਿਰ ਅਚਲ ਬਣਤਰ suffix_mult Bb[] ALIGN_SUFFIX = {
		{ "B", 1024 },
		{ "b", 512 },
		{ "", 0 }
	};
	ਅੱਖਰ *p;
	ਪੂਰਨ radix;

	/* Skip over any leading '+'. */
	ਜੇ (s[0] == '+') ++s;
	ਜੇ (!ਅੰਕ_ਹੈ(s[0])) ਵਾਪਸ 0; /* not a number */

	/* Determine the radix we'll use to interpret S.  If there is a '.',
	 * it's decimal, otherwise, if the string begins with '0X'or '0x',
	 * it's hexadecimal, else octal.  */
	p = strchr(s, '.');
	radix = 8;
	ਜੇ (p) {
		p[0] = '\0'; /* cheating */
		radix = 10;
	} ਨਹੀਂਤਾਂ ਜੇ (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
		radix = 16;

	*ਵਿੱਥ = xstrtooff_sfx(s, radix, Bb);
	ਜੇ (p) p[0] = '.';

	ਵਾਪਸ (*ਵਿੱਥ >= 0);
}
#ਜੇ_ਅੰਤ

ਪੂਰਨ od_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ od_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
#ਜੇ ENABLE_LONG_OPTS
	ਸਥਿਰ ਅਚਲ ਅੱਖਰ od_longopts[] ALIGN1 =
		"skip-bytes\0"        Required_ਦਲੀਲ "j"
		"address-radix\0"     Required_ਦਲੀਲ "A"
		"read-bytes\0"        Required_ਦਲੀਲ "N"
		"format\0"            Required_ਦਲੀਲ "t"
		"output-duplicates\0" No_ਦਲੀਲ       "v"
		/* Yes, it's true: -S NUM, but --strings[=NUM]!
		 * that is, NUM is mandatory for -S but optional for --strings!
		 */
		"strings\0"           Optional_ਦਲੀਲ "S"
		"width\0"             Optional_ਦਲੀਲ "w"
		"traditional\0"       No_ਦਲੀਲ       "\xff"
		;
#ਜੇ_ਅੰਤ
	ਅਚਲ ਅੱਖਰ *str_A, *str_N, *str_j, *str_S = "3";
	llist_t *lst_t = ਨਿਰਾ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ opt;
	ਪੂਰਨ l_c_m;
	/* The number of input bytes to skip before formatting and writing.  */
	off_t n_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_skip = 0;
	/* The offset of the first byte after the last byte to be formatted.  */
	off_t ਸਿਰਾ_ਵਿੱਥ = 0;
	/* The maximum number of bytes that will be formatted.  */
	off_t max_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_format = 0;

	INIT_G();

	/*G.spec = NULL; - already is */
	G.format_ਪਤਾ = format_ਪਤਾ_std;
	ਪਤਾ_ਅਧਾਰ_char = 'o';
	ਪਤਾ_pad_len_char = '7';

	/* Parse command line */
	opt = OD_GETOPT32();
	argv += optind;
	ਜੇ (opt & OPT_A) {
		ਸਥਿਰ ਅਚਲ ਅੱਖਰ doxn[] ALIGN1 = "doxn";
		ਸਥਿਰ ਅਚਲ ਅੱਖਰ doxn_ਪਤਾ_ਅਧਾਰ_char[] ALIGN1 = {
			'u', 'o', 'x', /* '?' fourth one is not important */
		};
		ਸਥਿਰ ਅਚਲ uint8_t doxn_ਪਤਾ_pad_len_char[] ALIGN1 = {
			'7', '7', '6', /* '?' */
		};
		ਅੱਖਰ *p;
		ਪੂਰਨ pos;
		p = strchr(doxn, str_A[0]);
		ਜੇ (!p)
			bb_error_ਸੁਨੇਹਾ_and_die("bad output address radix "
				"'%c' (must be [doxn])", str_A[0]);
		pos = p - doxn;
		ਜੇ (pos == 3) G.format_ਪਤਾ = format_ਪਤਾ_none;
		ਪਤਾ_ਅਧਾਰ_char = doxn_ਪਤਾ_ਅਧਾਰ_char[pos];
		ਪਤਾ_pad_len_char = doxn_ਪਤਾ_pad_len_char[pos];
	}
	ਜੇ (opt & OPT_N) {
		max_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_format = xstrtooff_sfx(str_N, 0, bkm_suffixes);
	}

	ਜੇ (opt & OPT_a) decode_format_string("a");
	ਜੇ (opt & OPT_b) decode_format_string("oC");
	ਜੇ (opt & OPT_c) decode_format_string("c");
	ਜੇ (opt & OPT_d) decode_format_string("u2");
	ਜੇ (opt & OPT_D) decode_format_string("uI");
	ਜੇ (opt & OPT_f) decode_format_string("fF");
	ਜੇ (opt & (OPT_h|OPT_x)) decode_format_string("x2");
	ਜੇ (opt & (OPT_H|OPT_X)) decode_format_string("xI");
	/* -I,L,l: depend on word width of the arch (what is "long"?) */
#ਜੇ ULONG_MAX > 0xffffffff
	ਜੇ (opt & OPT_i) decode_format_string("dI");
	ਜੇ (opt & (OPT_I|OPT_l|OPT_L)) decode_format_string("dL");
#ਨਹੀਂਤਾਂ
	/* 32-bit arch: -I,L,l are the same as -i */
	ਜੇ (opt & (OPT_i|OPT_I|OPT_l|OPT_L)) decode_format_string("dI");
#ਜੇ_ਅੰਤ
	ਜੇ (opt & OPT_j) n_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_skip = xstrtooff_sfx(str_j, 0, bkm_suffixes);
	ਜੇ (opt & (OPT_o|OPT_B)) decode_format_string("o2");
	ਜੇ (opt & OPT_O) decode_format_string("oI");
	ਜਦੋਂ (lst_t) {
		decode_format_string(llist_ਕੱਢੋ(&lst_t));
	}
	ਜੇ (opt & OPT_s) decode_format_string("d2");
	ਜੇ (opt & OPT_S) {
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

#ਜੇ ENABLE_LONG_OPTS
	ਜੇ (opt & OPT_traditional) {
		ਜੇ (argv[0]) {
			off_t pseudo_ਸ਼ੁਰੂ = -1;
			off_t o1, o2;

			ਜੇ (!argv[1]) { /* one arg */
				ਜੇ (parse_old_ਵਿੱਥ(argv[0], &o1)) {
					/* od --traditional OFFSET */
					n_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_skip = o1;
					argv++;
				}
				/* od --traditional FILE */
			} ਨਹੀਂਤਾਂ ਜੇ (!argv[2]) { /* two args */
				ਜੇ (parse_old_ਵਿੱਥ(argv[0], &o1)
				 && parse_old_ਵਿੱਥ(argv[1], &o2)
				) {
					/* od --traditional OFFSET LABEL */
					n_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_skip = o1;
					pseudo_ਸ਼ੁਰੂ = o2;
					argv += 2;
				} ਨਹੀਂਤਾਂ ਜੇ (parse_old_ਵਿੱਥ(argv[1], &o2)) {
					/* od --traditional FILE OFFSET */
					n_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_skip = o2;
					argv[1] = ਨਿਰਾ;
				} ਨਹੀਂਤਾਂ {
					bb_error_ਸੁਨੇਹਾ_and_die("invalid second argument '%s'", argv[1]);
				}
			} ਨਹੀਂਤਾਂ ਜੇ (!argv[3]) { /* three args */
				ਜੇ (parse_old_ਵਿੱਥ(argv[1], &o1)
				 && parse_old_ਵਿੱਥ(argv[2], &o2)
				) {
					/* od --traditional FILE OFFSET LABEL */
					n_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_skip = o1;
					pseudo_ਸ਼ੁਰੂ = o2;
					argv[1] = ਨਿਰਾ;
				} ਨਹੀਂਤਾਂ {
					bb_simple_error_ਸੁਨੇਹਾ_and_die("the last two arguments must be offsets");
				}
			} ਨਹੀਂਤਾਂ { /* >3 args */
				bb_simple_error_ਸੁਨੇਹਾ_and_die("too many arguments");
			}

			ਜੇ (pseudo_ਸ਼ੁਰੂ >= 0) {
				ਜੇ (G.format_ਪਤਾ == format_ਪਤਾ_none) {
					ਪਤਾ_ਅਧਾਰ_char = 'o';
					ਪਤਾ_pad_len_char = '7';
					G.format_ਪਤਾ = format_ਪਤਾ_paren;
				} ਨਹੀਂਤਾਂ {
					G.format_ਪਤਾ = format_ਪਤਾ_label;
				}
				G_pseudo_ਵਿੱਥ = pseudo_ਸ਼ੁਰੂ - n_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_skip;
			}
		}
		/* else: od --traditional (without args) */
	}
#ਜੇ_ਅੰਤ

	ਜੇ (option_mask32 & OPT_N) {
		ਸਿਰਾ_ਵਿੱਥ = n_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_skip + max_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_format;
		ਜੇ (ਸਿਰਾ_ਵਿੱਥ < n_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_skip)
			bb_simple_error_ਸੁਨੇਹਾ_and_die("SKIP + SIZE is too large");
	}

	ਜੇ (G.n_specs == 0) {
		decode_format_string("o2");
		/*G.n_specs = 1; - done by decode_format_string */
	}

	/* If no files were listed on the command line,
	   set the global pointer FILE_LIST so that it
	   references the null-terminated list of one name: "-".  */
	G.file_list = bb_argv_dash;
	ਜੇ (argv[0]) {
		/* Set the global pointer FILE_LIST so that it
		   references the first file-argument on the command-line.  */
		G.file_list = (ਅੱਖਰ ਅਚਲ *ਅਚਲ *) argv;
	}

	/* Open the first input file */
	open_ਅਗਲਾ_file();
	/* Skip over any unwanted header bytes */
	skip(n_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_skip);
	ਜੇ (!G.ਵਿੱਚ_ਧਾਰਾ)
		ਵਾਪਸ EXIT_FAILURE;

	/* Compute output block length */
	l_c_m = get_lcm();

	ਜੇ (opt & OPT_w) { /* -w: width */
		ਜੇ (!G.ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਖੰਡ || G.ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਖੰਡ % l_c_m != 0) {
			bb_error_ਸੁਨੇਹਾ("warning: invalid width %u; using %d instead",
					(ਬਿਨਾਨਿਸ਼ਾਨੀ)G.ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਖੰਡ, l_c_m);
			G.ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਖੰਡ = l_c_m;
		}
	} ਨਹੀਂਤਾਂ {
		G.ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਖੰਡ = l_c_m;
		ਜੇ (l_c_m < DEFAULT_ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਖੰਡ)
			G.ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਖੰਡ *= DEFAULT_ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਖੰਡ / l_c_m;
	}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DEBUG
	{
		ਪੂਰਨ i;
		ਲਈ (i = 0; i < G.n_specs; i++) {
			ਲਿਖੋ("%d: fmt='%s' width=%d\n",
				i, G.spec[i].fmt_string,
				ਚੌੜਾਈ_ਬਾਈਟਾਂ[G.spec[i].ਮਾਪ]);
		}
	}
#ਜੇ_ਅੰਤ

	ਜੇ (option_mask32 & OPT_S)
		dump_ਲੜੀਆਂ(n_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_skip, ਸਿਰਾ_ਵਿੱਥ);
	ਨਹੀਂਤਾਂ
		dump(n_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_skip, ਸਿਰਾ_ਵਿੱਥ);

	ਜੇ (fclose(stdin))
		bb_simple_perror_ਸੁਨੇਹਾ_and_die(bb_ਸੁਨੇਹਾ_standard_ਨਿਵੇਸ਼);

	ਵਾਪਸ G.exit_ਕੋਡ;
}
