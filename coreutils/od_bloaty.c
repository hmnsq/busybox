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
#ಅನ್ತರ್ಭಾವ "common_bufsiz.h"
#ವ್ಯಾಖ್ಯಾನಿಸು ಪ್ರತಿಜ್ಞಾ(a) ((ಶೂನ್ಯ)0)


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

ಎಣಿಕೆ {
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

#ವ್ಯಾಖ್ಯಾನಿಸು OD_GETOPT32() getopt32long(argv, \
	"A:N:abcdDfhHiIj:lLoOBt:*vxXsS:w:+:", od_longopts, \
	/* -w with optional param */ \
	/* -S was -s and also had optional parameter */ \
	/* but in coreutils 6.3 it was renamed and now has */ \
	/* _mandatory_ parameter */ \
	&str_A, &str_N, &str_j, &lst_t, &str_S, &G.ಬೈಟುಗಳು_ಪ್ರತಿ_ಖಂಡ)


/* Check for 0x7f is a coreutils 6.3 addition */
#ವ್ಯಾಖ್ಯಾನಿಸು ISPRINT(c) (((c) >= ' ') && (c) < 0x7f)

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ದೀರ್ಘ ದ್ವಿಗುಣ longdouble_t;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ ulonglong_t;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ದೀರ್ಘ ದೀರ್ಘ llong;

#ಆದರೆ ENABLE_LFS
# define xstrtooff_sfx xstrtoull_sfx
#ಇಲ್ಲದಿದ್ದರೆ
# define xstrtooff_sfx xstrtoul_sfx
#ಆದರೆ_ಅಂತ್ಯ

/* The default number of input bytes per output line.  */
#ವ್ಯಾಖ್ಯಾನಿಸು DEFAULT_ಬೈಟುಗಳು_ಪ್ರತಿ_ಖಂಡ 16

/* The number of decimal digits of precision in a float.  */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ FLT_DIG
# define FLT_DIG 7
#ಆದರೆ_ಅಂತ್ಯ

/* The number of decimal digits of precision in a double.  */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ DBL_DIG
# define DBL_DIG 15
#ಆದರೆ_ಅಂತ್ಯ

/* The number of decimal digits of precision in a long double.  */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ LDBL_DIG
# define LDBL_DIG DBL_DIG
#ಆದರೆ_ಅಂತ್ಯ

ಎಣಿಕೆ ಗಾತ್ರ_spec {
	NO_ಗಾತ್ರ,
	CHAR,
	SHORT,
	INT,
	LONG,
	LONG_LONG,
	FLOAT_SINGLE,
	FLOAT_DOUBLE,
	FLOAT_LONG_DOUBLE,
	N_ಗಾತ್ರ_SPECS
};

ಎಣಿಕೆ output_format {
	SIGNED_DECIMAL,
	UNSIGNED_DECIMAL,
	OCTAL,
	HEXADECIMAL,
	FLOATING_ಬಿಂದು,
	NAMED_CHARACTER,
	CHARACTER
};

/* Each output format specification (from '-t spec' or from
   old-style options) is represented by one of these structures.  */
ರಚನೆ tspec {
	ಎಣಿಕೆ output_format fmt;
	ಎಣಿಕೆ ಗಾತ್ರ_spec ಗಾತ್ರ_2;
	ಶೂನ್ಯ (*print_function) (ಗಾತ್ರ_t, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *);
	ಅಕ್ಷರ *fmt_string;
	ಪೂರ್ಣಾಂಕ hexl_ಬಹುಲಕ_trailer;
	ಪೂರ್ಣಾಂಕ ಕ್ಷೇತ್ರ_ಅಗಲ;
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

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ uint8_t ಬೈಟುಗಳು_ಗಮ್ಯ_oct_digits[] ALIGN1 =
{0, 3, 6, 8, 11, 14, 16, 19, 22, 25, 27, 30, 32, 35, 38, 41, 43};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ uint8_t ಬೈಟುಗಳು_ಗಮ್ಯ_signed_dec_digits[] ALIGN1 =
{1, 4, 6, 8, 11, 13, 16, 18, 20, 23, 25, 28, 30, 33, 35, 37, 40};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ uint8_t ಬೈಟುಗಳು_ಗಮ್ಯ_unsigned_dec_digits[] ALIGN1 =
{0, 3, 5, 8, 10, 13, 15, 17, 20, 22, 25, 27, 29, 32, 34, 37, 39};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ uint8_t ಬೈಟುಗಳು_ಗಮ್ಯ_hex_digits[] ALIGN1 =
{0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32};

/* Convert enum size_spec to the size of the named type.  */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆಯುಳ್ಳ ಅಕ್ಷರ ಅಗಲ_ಬೈಟುಗಳು[] ALIGN1 = {
	-1,
	ಗಾತ್ರ(ಅಕ್ಷರ),
	ಗಾತ್ರ(ಗಿಡ್ಡ),
	ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ),
	ಗಾತ್ರ(ದೀರ್ಘ),
	ಗಾತ್ರ(ulonglong_t),
	ಗಾತ್ರ(ದಶಮಾಂಶ),
	ಗಾತ್ರ(ದ್ವಿಗುಣ),
	ಗಾತ್ರ(longdouble_t)
};
/* Ensure that for each member of 'enum size_spec' there is an
   initializer in the width_bytes array.  */
ರಚನೆ ERR_ಅಗಲ_ಬೈಟುಗಳು_has_bad_ಗಾತ್ರ {
	ಅಕ್ಷರ ERR_ಅಗಲ_ಬೈಟುಗಳು_has_bad_ಗಾತ್ರ[ARRAY_ಗಾತ್ರ(ಅಗಲ_ಬೈಟುಗಳು) == N_ಗಾತ್ರ_SPECS ? 1 : -1];
};

ರಚನೆ globals {
	smallint exit_ಸಂಕೇತ;

	ಚಿಹ್ನೆರಹಿತ string_min;

	/* An array of specs describing how to format each input block.  */
	ಚಿಹ್ನೆರಹಿತ n_specs;
	ರಚನೆ tspec *spec;

	/* Function that accepts an address and an optional following char,
	   and prints the address and char to stdout.  */
	ಶೂನ್ಯ (*format_ವಿಳಾಸ)(off_t, ಅಕ್ಷರ);

	/* The difference between the old-style pseudo starting address and
	   the number of bytes to skip.  */
#ಆದರೆ ENABLE_LONG_OPTS
	off_t pseudo_ಸ್ಥಾನಾಂತರ;
# define G_pseudo_ಸ್ಥಾನಾಂತರ G.pseudo_ಸ್ಥಾನಾಂತರ
#ಆದರೆ_ಅಂತ್ಯ
	/* When zero, MAX_BYTES_TO_FORMAT and END_OFFSET are ignored, and all
	   input is formatted.  */

	/* The number of input bytes formatted per output line.  It must be
	   a multiple of the least common multiple of the sizes associated with
	   the specified output types.  It should be as large as possible, but
	   no larger than 16 -- unless specified with the -w option.  */
	ಚಿಹ್ನೆರಹಿತ ಬೈಟುಗಳು_ಪ್ರತಿ_ಖಂಡ; /* have to use unsigned, not size_t */

	/* A NULL-terminated list of the file-arguments from the command line.  */
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ *file_list;

	/* The input stream associated with the current file.  */
	FILE *ಒಳ_ಹರಿವು;

	bool not_ಮೊದಲ;
	bool prev_pair_equal;

	ಅಕ್ಷರ ವಿಳಾಸ_fmt[ಗಾತ್ರ("%0n"OFF_FMT"xc")];
} FIX_ALIASING;
/* Corresponds to 'x' above */
#ವ್ಯಾಖ್ಯಾನಿಸು ವಿಳಾಸ_ತಳ_char G.ವಿಳಾಸ_fmt[ಗಾತ್ರ(G.ವಿಳಾಸ_fmt)-3]
/* Corresponds to 'n' above */
#ವ್ಯಾಖ್ಯಾನಿಸು ವಿಳಾಸ_pad_len_char G.ವಿಳಾಸ_fmt[2]

#ಆದರೆ !ENABLE_LONG_OPTS
ಎಣಿಕೆ { G_pseudo_ಸ್ಥಾನಾಂತರ = 0 };
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು G (*(ರಚನೆ globals*)bb_common_bufsiz1)
#ವ್ಯಾಖ್ಯಾನಿಸು INIT_G() ಮಾಡು { \
	ಸಿದ್ಧತೆ_common_bufsiz(); \
	BUILD_BUG_ON(ಗಾತ್ರ(G) > COMMON_BUFSIZE); \
	G.ಬೈಟುಗಳು_ಪ್ರತಿ_ಖಂಡ = 32; \
	strcpy(G.ವಿಳಾಸ_fmt, "%0n"OFF_FMT"xc"); \
} ಆಗ (0)


#ವ್ಯಾಖ್ಯಾನಿಸು MAX_INTEGRAL_TYPE_ಗಾತ್ರ ಗಾತ್ರ(ulonglong_t)
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ integral_type_ಗಾತ್ರ[MAX_INTEGRAL_TYPE_ಗಾತ್ರ + 1] ALIGN1 = {
	[ಗಾತ್ರ(ಅಕ್ಷರ)] = CHAR,
#ಆದರೆ USHRT_MAX != UCHAR_MAX
	[ಗಾತ್ರ(ಗಿಡ್ಡ)] = SHORT,
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ UINT_MAX != USHRT_MAX
	[ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ)] = INT,
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ULONG_MAX != UINT_MAX
	[ಗಾತ್ರ(ದೀರ್ಘ)] = LONG,
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ULLONG_MAX != ULONG_MAX
	[ಗಾತ್ರ(ulonglong_t)] = LONG_LONG,
#ಆದರೆ_ಅಂತ್ಯ
};

#ವ್ಯಾಖ್ಯಾನಿಸು MAX_FP_TYPE_ಗಾತ್ರ ಗಾತ್ರ(longdouble_t)
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ fp_type_ಗಾತ್ರ[MAX_FP_TYPE_ಗಾತ್ರ + 1] ALIGN1 = {
	/* gcc seems to allow repeated indexes. Last one wins */
	[ಗಾತ್ರ(longdouble_t)] = FLOAT_LONG_DOUBLE,
	[ಗಾತ್ರ(ದ್ವಿಗುಣ)] = FLOAT_DOUBLE,
	[ಗಾತ್ರ(ದಶಮಾಂಶ)] = FLOAT_SINGLE
};


ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ
ಮಹತ್ತಮ_ಸಾಮಾನ್ಯ_ಅಪವರ್ತನ(ಚಿಹ್ನೆರಹಿತ u, ಚಿಹ್ನೆರಹಿತ v)
{
	ಚಿಹ್ನೆರಹಿತ t;
	ಆಗ (v != 0) {
		t = u % v;
		u = v;
		v = t;
	}
	ಹಿಂತಿರುಗಿ u;
}

/* Compute the least common multiple of U and V.  */
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ
lcm(ಚಿಹ್ನೆರಹಿತ u, ಚಿಹ್ನೆರಹಿತ v) {
	ಚಿಹ್ನೆರಹಿತ t = ಮಹತ್ತಮ_ಸಾಮಾನ್ಯ_ಅಪವರ್ತನ(u, v);
	ಆದರೆ (t == 0)
		ಹಿಂತಿರುಗಿ 0;
	ಹಿಂತಿರುಗಿ u * v / t;
}

ಸ್ಥಿರ ಶೂನ್ಯ
print_s_char(ಗಾತ್ರ_t n_ಬೈಟುಗಳು, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಖಂಡ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt_string)
{
	ಆಗ (n_ಬೈಟುಗಳು--) {
		ಪೂರ್ಣಾಂಕ tmp = *(ಚಿಹ್ನೆಯುಳ್ಳ ಅಕ್ಷರ *) ಖಂಡ;
		ಬರೆ(fmt_string, tmp);
		ಖಂಡ += ಗಾತ್ರ(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ
print_char(ಗಾತ್ರ_t n_ಬೈಟುಗಳು, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಖಂಡ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt_string)
{
	ಆಗ (n_ಬೈಟುಗಳು--) {
		ಚಿಹ್ನೆರಹಿತ tmp = *(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *) ಖಂಡ;
		ಬರೆ(fmt_string, tmp);
		ಖಂಡ += ಗಾತ್ರ(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ
print_s_short(ಗಾತ್ರ_t n_ಬೈಟುಗಳು, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಖಂಡ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt_string)
{
	n_ಬೈಟುಗಳು /= ಗಾತ್ರ(ಚಿಹ್ನೆಯುಳ್ಳ ಗಿಡ್ಡ);
	ಆಗ (n_ಬೈಟುಗಳು--) {
		ಪೂರ್ಣಾಂಕ tmp = *(ಚಿಹ್ನೆಯುಳ್ಳ ಗಿಡ್ಡ *) ಖಂಡ;
		ಬರೆ(fmt_string, tmp);
		ಖಂಡ += ಗಾತ್ರ(ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ
print_short(ಗಾತ್ರ_t n_ಬೈಟುಗಳು, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಖಂಡ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt_string)
{
	n_ಬೈಟುಗಳು /= ಗಾತ್ರ(ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ);
	ಆಗ (n_ಬೈಟುಗಳು--) {
		ಚಿಹ್ನೆರಹಿತ tmp = *(ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ *) ಖಂಡ;
		ಬರೆ(fmt_string, tmp);
		ಖಂಡ += ಗಾತ್ರ(ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ
print_int(ಗಾತ್ರ_t n_ಬೈಟುಗಳು, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಖಂಡ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt_string)
{
	n_ಬೈಟುಗಳು /= ಗಾತ್ರ(ಚಿಹ್ನೆರಹಿತ);
	ಆಗ (n_ಬೈಟುಗಳು--) {
		ಚಿಹ್ನೆರಹಿತ tmp = *(ಚಿಹ್ನೆರಹಿತ *) ಖಂಡ;
		ಬರೆ(fmt_string, tmp);
		ಖಂಡ += ಗಾತ್ರ(ಚಿಹ್ನೆರಹಿತ);
	}
}

#ಆದರೆ UINT_MAX == ULONG_MAX
# define print_long print_int
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ ಶೂನ್ಯ
print_long(ಗಾತ್ರ_t n_ಬೈಟುಗಳು, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಖಂಡ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt_string)
{
	n_ಬೈಟುಗಳು /= ಗಾತ್ರ(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ);
	ಆಗ (n_ಬೈಟುಗಳು--) {
		ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ tmp = *(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *) ಖಂಡ;
		ಬರೆ(fmt_string, tmp);
		ಖಂಡ += ಗಾತ್ರ(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ);
	}
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ULONG_MAX == ULLONG_MAX
# define print_long_long print_long
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ ಶೂನ್ಯ
print_long_long(ಗಾತ್ರ_t n_ಬೈಟುಗಳು, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಖಂಡ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt_string)
{
	n_ಬೈಟುಗಳು /= ಗಾತ್ರ(ulonglong_t);
	ಆಗ (n_ಬೈಟುಗಳು--) {
		ulonglong_t tmp = *(ulonglong_t *) ಖಂಡ;
		ಬರೆ(fmt_string, tmp);
		ಖಂಡ += ಗಾತ್ರ(ulonglong_t);
	}
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಶೂನ್ಯ
print_float(ಗಾತ್ರ_t n_ಬೈಟುಗಳು, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಖಂಡ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt_string)
{
	n_ಬೈಟುಗಳು /= ಗಾತ್ರ(ದಶಮಾಂಶ);
	ಆಗ (n_ಬೈಟುಗಳು--) {
		ದಶಮಾಂಶ tmp = *(ದಶಮಾಂಶ *) ಖಂಡ;
		ಬರೆ(fmt_string, tmp);
		ಖಂಡ += ಗಾತ್ರ(ದಶಮಾಂಶ);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ
print_double(ಗಾತ್ರ_t n_ಬೈಟುಗಳು, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಖಂಡ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt_string)
{
	n_ಬೈಟುಗಳು /= ಗಾತ್ರ(ದ್ವಿಗುಣ);
	ಆಗ (n_ಬೈಟುಗಳು--) {
		ದ್ವಿಗುಣ tmp = *(ದ್ವಿಗುಣ *) ಖಂಡ;
		ಬರೆ(fmt_string, tmp);
		ಖಂಡ += ಗಾತ್ರ(ದ್ವಿಗುಣ);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ
print_long_double(ಗಾತ್ರ_t n_ಬೈಟುಗಳು, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಖಂಡ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt_string)
{
	n_ಬೈಟುಗಳು /= ಗಾತ್ರ(longdouble_t);
	ಆಗ (n_ಬೈಟುಗಳು--) {
		longdouble_t tmp = *(longdouble_t *) ಖಂಡ;
		ಬರೆ(fmt_string, tmp);
		ಖಂಡ += ಗಾತ್ರ(longdouble_t);
	}
}

/* print_[named]_ascii are optimized for speed.
 * Remember, someday you may want to pump gigabytes through this thing.
 * Saving a dozen of .text bytes here is counter-productive */

ಸ್ಥಿರ ಶೂನ್ಯ
print_named_ascii(ಗಾತ್ರ_t n_ಬೈಟುಗಳು, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಖಂಡ,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಳಸದ_fmt_string ಬಳಸದ_PARAM)
{
	/* Names for some non-printing characters.  */
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ charname[33][3] ALIGN1 = {
		"nul", "soh", "stx", "etx", "eot", "enq", "ack", "bel",
		" bs", " ht", " nl", " vt", " ff", " cr", " so", " si",
		"dle", "dc1", "dc2", "dc3", "dc4", "nak", "syn", "etb",
		"can", " em", "sub", "esc", " fs", " gs", " rs", " us",
		" sp"
	};
	// buf[N] pos:  01234 56789
	ಅಕ್ಷರ buf[12] = "   x\0 xxx\0";
	// [12] because we take three 32bit stack slots anyway, and
	// gcc is too dumb to initialize with constant stores,
	// it copies initializer from rodata. Oh well.
	// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=65410

	ಆಗ (n_ಬೈಟುಗಳು--) {
		ಚಿಹ್ನೆರಹಿತ masked_c = *(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *) ಖಂಡ++;

		masked_c &= 0x7f;
		ಆದರೆ (masked_c == 0x7f) {
			fputs_stdout(" del");
			ಮುಂದುವರಿ;
		}
		ಆದರೆ (masked_c > ' ') {
			buf[3] = masked_c;
			fputs_stdout(buf);
			ಮುಂದುವರಿ;
		}
		/* Why? Because printf(" %3.3s") is much slower... */
		buf[6] = charname[masked_c][0];
		buf[7] = charname[masked_c][1];
		buf[8] = charname[masked_c][2];
		fputs_stdout(buf+5);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ
print_ascii(ಗಾತ್ರ_t n_ಬೈಟುಗಳು, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಖಂಡ,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಳಸದ_fmt_string ಬಳಸದ_PARAM)
{
	// buf[N] pos:  01234 56789
	ಅಕ್ಷರ buf[12] = "   x\0 xxx\0";

	ಆಗ (n_ಬೈಟುಗಳು--) {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s;
		ಚಿಹ್ನೆರಹಿತ c = *(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *) ಖಂಡ++;

		ಆದರೆ (ISPRINT(c)) {
			buf[3] = c;
			fputs_stdout(buf);
			ಮುಂದುವರಿ;
		}
		ಆಯ್ಕೆ (c) {
		ಸ್ಥಿತಿ '\0':
			s = "  \\0";
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ '\007':
			s = "  \\a";
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ '\b':
			s = "  \\b";
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ '\f':
			s = "  \\f";
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ '\n':
			s = "  \\n";
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ '\r':
			s = "  \\r";
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ '\t':
			s = "  \\t";
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ '\v':
			s = "  \\v";
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
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

ಸ್ಥಿರ ಶೂನ್ಯ
open_ಮುಂದಿನ_file(ಶೂನ್ಯ)
{
	ಆಗ (1) {
		ಆದರೆ (!*G.file_list)
			ಹಿಂತಿರುಗಿ;
		G.ಒಳ_ಹರಿವು = fopen_or_warn_stdin(*G.file_list++);
		ಆದರೆ (G.ಒಳ_ಹರಿವು) {
			ನಿಲ್ಲಿಸು;
		}
		G.exit_ಸಂಕೇತ = 1;
	}

	ಆದರೆ ((option_mask32 & (OPT_N|OPT_S)) == OPT_N)
		setbuf(G.ಒಳ_ಹರಿವು, ಖಾಲಿ);
}

/* Test whether there have been errors on in_stream, and close it if
   it is not standard input.  Return nonzero if there has been an error
   on in_stream or stdout; return zero otherwise.  This function will
   report more than one error only if both a read and a write error
   have occurred.  IN_ERRNO, if nonzero, is the error number
   corresponding to the most recent action for IN_STREAM.  */

ಸ್ಥಿರ ಶೂನ್ಯ
check_and_close(ಶೂನ್ಯ)
{
	ಆದರೆ (G.ಒಳ_ಹರಿವು) {
		ಆದರೆ (ferror(G.ಒಳ_ಹರಿವು))	{
			bb_error_ಸಂದೇಶ("%s: read error", (G.ಒಳ_ಹರಿವು == stdin)
					? bb_ಸಂದೇಶ_standard_ಒಳಹರಿವು
					: G.file_list[-1]
			);
			G.exit_ಸಂಕೇತ = 1;
		}
		fclose_if_not_stdin(G.ಒಳ_ಹರಿವು);
		G.ಒಳ_ಹರಿವು = ಖಾಲಿ;
	}

	ಆದರೆ (ferror(stdout)) {
		bb_simple_error_ಸಂದೇಶ_and_die(bb_ಸಂದೇಶ_write_error);
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

ಸ್ಥಿರ NOINLINE ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *
decode_one_format(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s_orig, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, ರಚನೆ tspec *tspec)
{
	ಎಣಿಕೆ ಗಾತ್ರ_spec ಗಾತ್ರ_spec;
	ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_2;
	ಎಣಿಕೆ output_format fmt;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p;
	ಅಕ್ಷರ *ಕೊನೆ;
	ಅಕ್ಷರ *fmt_string = ಖಾಲಿ;
	ಶೂನ್ಯ (*print_function) (ಗಾತ್ರ_t, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *);
	ಚಿಹ್ನೆರಹಿತ c;
	ಚಿಹ್ನೆರಹಿತ ಕ್ಷೇತ್ರ_ಅಗಲ = 0;
	ಪೂರ್ಣಾಂಕ pos;

	ಆಯ್ಕೆ (*s) {
	ಸ್ಥಿತಿ 'd':
	ಸ್ಥಿತಿ 'o':
	ಸ್ಥಿತಿ 'u':
	ಸ್ಥಿತಿ 'x': {
		ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ CSIL[] ALIGN1 = "CSIL";

		c = *s++;
		p = strchr(CSIL, *s);
		/* if *s == NUL, p != NULL! Testcase: "od -tx" */
		ಆದರೆ (!p || *p == '\0') {
			ಗಾತ್ರ_2 = ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ);
			ಆದರೆ (ಅಂಕಿಯೇ(s[0])) {
				ಗಾತ್ರ_2 = bb_strtou(s, &ಕೊನೆ, 0);
				ಆದರೆ (errno == ERANGE
				 || MAX_INTEGRAL_TYPE_ಗಾತ್ರ < ಗಾತ್ರ_2
				 || integral_type_ಗಾತ್ರ[ಗಾತ್ರ_2] == NO_ಗಾತ್ರ
				) {
					bb_error_ಸಂದೇಶ_and_die("invalid type string '%s'; "
						"%u-byte %s type is not supported",
						s_orig, ಗಾತ್ರ_2, "integral");
				}
				s = ಕೊನೆ;
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ uint8_t CSIL_sizeof[4] = {
				ಗಾತ್ರ(ಅಕ್ಷರ),
				ಗಾತ್ರ(ಗಿಡ್ಡ),
				ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ),
				ಗಾತ್ರ(ದೀರ್ಘ),
			};
			ಗಾತ್ರ_2 = CSIL_sizeof[p - CSIL];
			s++; /* skip C/S/I/L */
		}

#ವ್ಯಾಖ್ಯಾನಿಸು ISPEC_ಗಮ್ಯ_FORMAT(Spec, Min_format, Long_format, Max_format) \
	((Spec) == LONG_LONG ? (Max_format) \
	: ((Spec) == LONG ? (Long_format) : (Min_format)))

#ವ್ಯಾಖ್ಯಾನಿಸು FMT_ಬೈಟುಗಳು_ALLOCATED 9
		ಗಾತ್ರ_spec = integral_type_ಗಾತ್ರ[ಗಾತ್ರ_2];

		{
			ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ doux[] ALIGN1 = "doux";
			ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ doux_fmt_letter[][4] = {
				"lld", "llo", "llu", "llx"
			};
			ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಎಣಿಕೆ output_format doux_fmt[] = {
				SIGNED_DECIMAL,
				OCTAL,
				UNSIGNED_DECIMAL,
				HEXADECIMAL,
			};
			ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ uint8_t *ಸ್ಥಿರಾಂಕ doux_ಬೈಟುಗಳು_ಗಮ್ಯ_XXX[] = {
				ಬೈಟುಗಳು_ಗಮ್ಯ_signed_dec_digits,
				ಬೈಟುಗಳು_ಗಮ್ಯ_oct_digits,
				ಬೈಟುಗಳು_ಗಮ್ಯ_unsigned_dec_digits,
				ಬೈಟುಗಳು_ಗಮ್ಯ_hex_digits,
			};
			ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ doux_fmtstring[][ಗಾತ್ರ(" %%0%u%s")] ALIGN1 = {
				" %%%u%s",
				" %%0%u%s",
				" %%%u%s",
				" %%0%u%s",
			};

			pos = strchr(doux, c) - doux;
			fmt = doux_fmt[pos];
			ಕ್ಷೇತ್ರ_ಅಗಲ = doux_ಬೈಟುಗಳು_ಗಮ್ಯ_XXX[pos][ಗಾತ್ರ_2];
			p = doux_fmt_letter[pos] + 2;
			ಆದರೆ (ಗಾತ್ರ_spec == LONG) p--;
			ಆದರೆ (ಗಾತ್ರ_spec == LONG_LONG) p -= 2;
			fmt_string = xasprintf(doux_fmtstring[pos], ಕ್ಷೇತ್ರ_ಅಗಲ, p);
		}

		ಆಯ್ಕೆ (ಗಾತ್ರ_spec) {
		ಸ್ಥಿತಿ CHAR:
			print_function = (fmt == SIGNED_DECIMAL
				    ? print_s_char
				    : print_char);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ SHORT:
			print_function = (fmt == SIGNED_DECIMAL
				    ? print_s_short
				    : print_short);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ INT:
			print_function = print_int;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ LONG:
			print_function = print_long;
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ: /* case LONG_LONG: */
			print_function = print_long_long;
			ನಿಲ್ಲಿಸು;
		}
		ನಿಲ್ಲಿಸು;
	}

	ಸ್ಥಿತಿ 'f': {
		ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ FDL[] ALIGN1 = "FDL";

		fmt = FLOATING_ಬಿಂದು;
		++s;
		p = strchr(FDL, *s);
		ಆದರೆ (!p || *p == '\0') {
			ಗಾತ್ರ_2 = ಗಾತ್ರ(ದ್ವಿಗುಣ);
			ಆದರೆ (ಅಂಕಿಯೇ(s[0])) {
				ಗಾತ್ರ_2 = bb_strtou(s, &ಕೊನೆ, 0);
				ಆದರೆ (errno == ERANGE || ಗಾತ್ರ_2 > MAX_FP_TYPE_ಗಾತ್ರ
				 || fp_type_ಗಾತ್ರ[ಗಾತ್ರ_2] == NO_ಗಾತ್ರ
				) {
					bb_error_ಸಂದೇಶ_and_die("invalid type string '%s'; "
						"%u-byte %s type is not supported",
						s_orig, ಗಾತ್ರ_2, "floating point");
				}
				s = ಕೊನೆ;
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ uint8_t FDL_sizeof[] = {
				ಗಾತ್ರ(ದಶಮಾಂಶ),
				ಗಾತ್ರ(ದ್ವಿಗುಣ),
				ಗಾತ್ರ(longdouble_t),
			};

			ಗಾತ್ರ_2 = FDL_sizeof[p - FDL];
			s++; /* skip F/D/L */
		}

		ಗಾತ್ರ_spec = fp_type_ಗಾತ್ರ[ಗಾತ್ರ_2];

		ಆಯ್ಕೆ (ಗಾತ್ರ_spec) {
		ಸ್ಥಿತಿ FLOAT_SINGLE:
			print_function = print_float;
			ಕ್ಷೇತ್ರ_ಅಗಲ = FLT_DIG + 8;
			/* Don't use %#e; not all systems support it.  */
			fmt_string = xasprintf(" %%%d.%de", ಕ್ಷೇತ್ರ_ಅಗಲ, FLT_DIG);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ FLOAT_DOUBLE:
			print_function = print_double;
			ಕ್ಷೇತ್ರ_ಅಗಲ = DBL_DIG + 8;
			fmt_string = xasprintf(" %%%d.%de", ಕ್ಷೇತ್ರ_ಅಗಲ, DBL_DIG);
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ: /* case FLOAT_LONG_DOUBLE: */
			print_function = print_long_double;
			ಕ್ಷೇತ್ರ_ಅಗಲ = LDBL_DIG + 8;
			fmt_string = xasprintf(" %%%d.%dLe", ಕ್ಷೇತ್ರ_ಅಗಲ, LDBL_DIG);
			ನಿಲ್ಲಿಸು;
		}
		ನಿಲ್ಲಿಸು;
	}

	ಸ್ಥಿತಿ 'a':
		++s;
		fmt = NAMED_CHARACTER;
		ಗಾತ್ರ_spec = CHAR;
		print_function = print_named_ascii;
		ಕ್ಷೇತ್ರ_ಅಗಲ = 3;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ 'c':
		++s;
		fmt = CHARACTER;
		ಗಾತ್ರ_spec = CHAR;
		print_function = print_ascii;
		ಕ್ಷೇತ್ರ_ಅಗಲ = 3;
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		bb_error_ಸಂದೇಶ_and_die("invalid character '%c' "
				"in type string '%s'", *s, s_orig);
	}

	tspec->ಗಾತ್ರ_2 = ಗಾತ್ರ_spec;
	tspec->fmt = fmt;
	tspec->print_function = print_function;
	tspec->fmt_string = fmt_string;

	tspec->ಕ್ಷೇತ್ರ_ಅಗಲ = ಕ್ಷೇತ್ರ_ಅಗಲ;
	tspec->hexl_ಬಹುಲಕ_trailer = (*s == 'z');
	ಆದರೆ (tspec->hexl_ಬಹುಲಕ_trailer)
		s++;

	ಹಿಂತಿರುಗಿ s;
}

/* Decode the modern od format string S.  Append the decoded
   representation to the global array SPEC, reallocating SPEC if
   necessary.  */

ಸ್ಥಿರ ಶೂನ್ಯ
decode_format_string(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s_orig = s;

	ಆಗ (*s != '\0') {
		ರಚನೆ tspec tspec;
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮುಂದಿನ;

		ಮುಂದಿನ = decode_one_format(s_orig, s, &tspec);

		ಪ್ರತಿಜ್ಞಾ(s != ಮುಂದಿನ);
		s = ಮುಂದಿನ;
		G.spec = xrealloc_ಸದಿಶ(G.spec, 4, G.n_specs);
		ಸ್ಮರಣೆ_ನಕಲು(&G.spec[G.n_specs], &tspec, ಗಾತ್ರ(G.spec[0]));
		G.n_specs++;
	}
}

/* Given a list of one or more input filenames FILE_LIST, set the global
   file pointer IN_STREAM to position N_SKIP in the concatenation of
   those files.  If any file operation fails or if there are fewer than
   N_SKIP bytes in the combined input, give an error message and return
   nonzero.  When possible, use seek rather than read operations to
   advance IN_STREAM.  */

ಸ್ಥಿರ ಶೂನ್ಯ
skip(off_t n_skip)
{
	ಆದರೆ (n_skip == 0)
		ಹಿಂತಿರುಗಿ;

	ಆಗ (G.ಒಳ_ಹರಿವು) { /* !EOF */
		ರಚನೆ stat file_stats;

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
		ಆದರೆ (fstat(fileno(G.ಒಳ_ಹರಿವು), &file_stats) == 0
		 && S_ISREG(file_stats.st_ಬಹುಲಕ) && file_stats.st_ಗಾತ್ರ > 0
		) {
			ಆದರೆ (file_stats.st_ಗಾತ್ರ < n_skip) {
				n_skip -= file_stats.st_ಗಾತ್ರ;
				/* take "check & close / open_next" route */
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಆದರೆ (fseeko(G.ಒಳ_ಹರಿವು, n_skip, SEEK_CUR) != 0)
					G.exit_ಸಂಕೇತ = 1;
				ಹಿಂತಿರುಗಿ;
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* If it's not a regular file with positive size,
			   position the file pointer by reading.  */
			ಅಕ್ಷರ buf[1024];
			ಗಾತ್ರ_t n_ಬೈಟುಗಳು_ಗಮ್ಯ_read = 1024;
			ಗಾತ್ರ_t n_ಬೈಟುಗಳು_read;

			ಆಗ (n_skip > 0) {
				ಆದರೆ (n_skip < n_ಬೈಟುಗಳು_ಗಮ್ಯ_read)
					n_ಬೈಟುಗಳು_ಗಮ್ಯ_read = n_skip;
				n_ಬೈಟುಗಳು_read = fread(buf, 1, n_ಬೈಟುಗಳು_ಗಮ್ಯ_read, G.ಒಳ_ಹರಿವು);
				n_skip -= n_ಬೈಟುಗಳು_read;
				ಆದರೆ (n_ಬೈಟುಗಳು_read != n_ಬೈಟುಗಳು_ಗಮ್ಯ_read)
					ನಿಲ್ಲಿಸು; /* EOF on this file or error */
			}
		}
		ಆದರೆ (n_skip == 0)
			ಹಿಂತಿರುಗಿ;

		check_and_close();
		open_ಮುಂದಿನ_file();
	}

	ಆದರೆ (n_skip)
		bb_simple_error_ಸಂದೇಶ_and_die("can't skip past end of combined input");
}


ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಶೂನ್ಯ FN_format_ವಿಳಾಸ(off_t ವಿಳಾಸ, ಅಕ್ಷರ c);

ಸ್ಥಿರ ಶೂನ್ಯ
format_ವಿಳಾಸ_none(off_t ವಿಳಾಸ ಬಳಸದ_PARAM, ಅಕ್ಷರ c ಬಳಸದ_PARAM)
{
}

ಸ್ಥಿರ ಶೂನ್ಯ
format_ವಿಳಾಸ_std(off_t ವಿಳಾಸ, ಅಕ್ಷರ c)
{
	/* Corresponds to 'c' */
	G.ವಿಳಾಸ_fmt[ಗಾತ್ರ(G.ವಿಳಾಸ_fmt)-2] = c;
	ಬರೆ(G.ವಿಳಾಸ_fmt, ವಿಳಾಸ);
}

#ಆದರೆ ENABLE_LONG_OPTS
/* only used with --traditional */
ಸ್ಥಿರ ಶೂನ್ಯ
format_ವಿಳಾಸ_paren(off_t ವಿಳಾಸ, ಅಕ್ಷರ c)
{
	ಅಕ್ಷರ_ಬರೆ('(');
	format_ವಿಳಾಸ_std(ವಿಳಾಸ, ')');
	ಆದರೆ (c) ಅಕ್ಷರ_ಬರೆ(c);
}

ಸ್ಥಿರ ಶೂನ್ಯ
format_ವಿಳಾಸ_label(off_t ವಿಳಾಸ, ಅಕ್ಷರ c)
{
	format_ವಿಳಾಸ_std(ವಿಳಾಸ, ' ');
	format_ವಿಳಾಸ_paren(ವಿಳಾಸ + G_pseudo_ಸ್ಥಾನಾಂತರ, c);
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಶೂನ್ಯ
dump_hexl_ಬಹುಲಕ_trailer(ಗಾತ್ರ_t n_ಬೈಟುಗಳು, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಖಂಡ)
{
	fputs_stdout("  >");
	ಆಗ (n_ಬೈಟುಗಳು--) {
		ಚಿಹ್ನೆರಹಿತ c = *(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *) ಖಂಡ++;
		c = (ISPRINT(c) ? c : '.');
		ಅಕ್ಷರ_ಬರೆ(c);
	}
	ಅಕ್ಷರ_ಬರೆ('<');
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

ಸ್ಥಿರ ಶೂನ್ಯ
write_ಖಂಡ(off_t current_ಸ್ಥಾನಾಂತರ, ಗಾತ್ರ_t n_ಬೈಟುಗಳು,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *prev_ಖಂಡ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *curr_ಖಂಡ)
{
	ಚಿಹ್ನೆರಹಿತ i;

	ಆದರೆ (!(option_mask32 & OPT_v)
	 && G.not_ಮೊದಲ
	 && n_ಬೈಟುಗಳು == G.ಬೈಟುಗಳು_ಪ್ರತಿ_ಖಂಡ
	 && memcmp(prev_ಖಂಡ, curr_ಖಂಡ, G.ಬೈಟುಗಳು_ಪ್ರತಿ_ಖಂಡ) == 0
	) {
		ಆದರೆ (G.prev_pair_equal) {
			/* The two preceding blocks were equal, and the current
			   block is the same as the last one, so print nothing.  */
		} ಇಲ್ಲದಿದ್ದರೆ {
			puts("*");
			G.prev_pair_equal = 1;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		G.not_ಮೊದಲ = 1;
		G.prev_pair_equal = 0;
		ಗಾಗಿ (i = 0; i < G.n_specs; i++) {
			ಆದರೆ (i == 0)
				G.format_ವಿಳಾಸ(current_ಸ್ಥಾನಾಂತರ, '\0');
			ಇಲ್ಲದಿದ್ದರೆ
				ಬರೆ("%*s", ವಿಳಾಸ_pad_len_char - '0', "");
			(*G.spec[i].print_function) (n_ಬೈಟುಗಳು, curr_ಖಂಡ, G.spec[i].fmt_string);
			ಆದರೆ (G.spec[i].hexl_ಬಹುಲಕ_trailer) {
				/* space-pad out to full line width, then dump the trailer */
				ಚಿಹ್ನೆರಹಿತ datum_ಅಗಲ = ಅಗಲ_ಬೈಟುಗಳು[G.spec[i].ಗಾತ್ರ_2];
				ಚಿಹ್ನೆರಹಿತ blank_fields = (G.ಬೈಟುಗಳು_ಪ್ರತಿ_ಖಂಡ - n_ಬೈಟುಗಳು) / datum_ಅಗಲ;
				ಚಿಹ್ನೆರಹಿತ ಕ್ಷೇತ್ರ_ಅಗಲ = G.spec[i].ಕ್ಷೇತ್ರ_ಅಗಲ + 1;
				ಬರೆ("%*s", blank_fields * ಕ್ಷೇತ್ರ_ಅಗಲ, "");
				dump_hexl_ಬಹುಲಕ_trailer(n_ಬೈಟುಗಳು, curr_ಖಂಡ);
			}
			ಅಕ್ಷರ_ಬರೆ('\n');
		}
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ
read_ಖಂಡ(ಗಾತ್ರ_t n, ಅಕ್ಷರ *ಖಂಡ, ಗಾತ್ರ_t *n_ಬೈಟುಗಳು_ಒಳ_ಬಫರ್)
{
	ಪ್ರತಿಜ್ಞಾ(0 < n && n <= G.ಬೈಟುಗಳು_ಪ್ರತಿ_ಖಂಡ);

	*n_ಬೈಟುಗಳು_ಒಳ_ಬಫರ್ = 0;

	ಆದರೆ (n == 0)
		ಹಿಂತಿರುಗಿ;

	ಆಗ (G.ಒಳ_ಹರಿವು != ಖಾಲಿ) { /* EOF.  */
		ಗಾತ್ರ_t n_needed;
		ಗಾತ್ರ_t n_read;

		n_needed = n - *n_ಬೈಟುಗಳು_ಒಳ_ಬಫರ್;
		n_read = fread(ಖಂಡ + *n_ಬೈಟುಗಳು_ಒಳ_ಬಫರ್, 1, n_needed, G.ಒಳ_ಹರಿವು);
		*n_ಬೈಟುಗಳು_ಒಳ_ಬಫರ್ += n_read;
		ಆದರೆ (n_read == n_needed)
			ನಿಲ್ಲಿಸು;
		/* error check is done in check_and_close */
		check_and_close();
		open_ಮುಂದಿನ_file();
	}
}

/* Return the least common multiple of the sizes associated
   with the format specs.  */

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
get_lcm(ಶೂನ್ಯ)
{
	ಗಾತ್ರ_t i;
	ಪೂರ್ಣಾಂಕ l_c_m = 1;

	ಗಾಗಿ (i = 0; i < G.n_specs; i++)
		l_c_m = lcm(l_c_m, ಅಗಲ_ಬೈಟುಗಳು[(ಪೂರ್ಣಾಂಕ) G.spec[i].ಗಾತ್ರ_2]);
	ಹಿಂತಿರುಗಿ l_c_m;
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

ಸ್ಥಿರ ಶೂನ್ಯ
dump(off_t current_ಸ್ಥಾನಾಂತರ, off_t ಕೊನೆ_ಸ್ಥಾನಾಂತರ)
{
	ಅಕ್ಷರ *ಖಂಡ[2];
	ಪೂರ್ಣಾಂಕ idx;
	ಗಾತ್ರ_t n_ಬೈಟುಗಳು_read;

	ಖಂಡ[0] = xmalloc(2 * G.ಬೈಟುಗಳು_ಪ್ರತಿ_ಖಂಡ);
	ಖಂಡ[1] = ಖಂಡ[0] + G.ಬೈಟುಗಳು_ಪ್ರತಿ_ಖಂಡ;

	idx = 0;
	ಆದರೆ (option_mask32 & OPT_N) {
		ಆಗ (1) {
			ಗಾತ್ರ_t n_needed;
			ಆದರೆ (current_ಸ್ಥಾನಾಂತರ >= ಕೊನೆ_ಸ್ಥಾನಾಂತರ) {
				n_ಬೈಟುಗಳು_read = 0;
				ನಿಲ್ಲಿಸು;
			}
			n_needed = MIN(ಕೊನೆ_ಸ್ಥಾನಾಂತರ - current_ಸ್ಥಾನಾಂತರ, (off_t) G.ಬೈಟುಗಳು_ಪ್ರತಿ_ಖಂಡ);
			read_ಖಂಡ(n_needed, ಖಂಡ[idx], &n_ಬೈಟುಗಳು_read);
			ಆದರೆ (n_ಬೈಟುಗಳು_read < G.ಬೈಟುಗಳು_ಪ್ರತಿ_ಖಂಡ)
				ನಿಲ್ಲಿಸು;
			ಪ್ರತಿಜ್ಞಾ(n_ಬೈಟುಗಳು_read == G.ಬೈಟುಗಳು_ಪ್ರತಿ_ಖಂಡ);
			write_ಖಂಡ(current_ಸ್ಥಾನಾಂತರ, n_ಬೈಟುಗಳು_read, ಖಂಡ[idx ^ 1], ಖಂಡ[idx]);
			current_ಸ್ಥಾನಾಂತರ += n_ಬೈಟುಗಳು_read;
			idx ^= 1;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಆಗ (1) {
			read_ಖಂಡ(G.ಬೈಟುಗಳು_ಪ್ರತಿ_ಖಂಡ, ಖಂಡ[idx], &n_ಬೈಟುಗಳು_read);
			ಆದರೆ (n_ಬೈಟುಗಳು_read < G.ಬೈಟುಗಳು_ಪ್ರತಿ_ಖಂಡ)
				ನಿಲ್ಲಿಸು;
			ಪ್ರತಿಜ್ಞಾ(n_ಬೈಟುಗಳು_read == G.ಬೈಟುಗಳು_ಪ್ರತಿ_ಖಂಡ);
			write_ಖಂಡ(current_ಸ್ಥಾನಾಂತರ, n_ಬೈಟುಗಳು_read, ಖಂಡ[idx ^ 1], ಖಂಡ[idx]);
			current_ಸ್ಥಾನಾಂತರ += n_ಬೈಟುಗಳು_read;
			idx ^= 1;
		}
	}

	ಆದರೆ (n_ಬೈಟುಗಳು_read > 0) {
		ಪೂರ್ಣಾಂಕ l_c_m;
		ಗಾತ್ರ_t ಬೈಟುಗಳು_ಗಮ್ಯ_write;

		l_c_m = get_lcm();

		/* Make bytes_to_write the smallest multiple of l_c_m that
		   is at least as large as n_bytes_read.  */
		ಬೈಟುಗಳು_ಗಮ್ಯ_write = l_c_m * ((n_ಬೈಟುಗಳು_read + l_c_m - 1) / l_c_m);

		memset(ಖಂಡ[idx] + n_ಬೈಟುಗಳು_read, 0, ಬೈಟುಗಳು_ಗಮ್ಯ_write - n_ಬೈಟುಗಳು_read);
		write_ಖಂಡ(current_ಸ್ಥಾನಾಂತರ, ಬೈಟುಗಳು_ಗಮ್ಯ_write,
				ಖಂಡ[idx ^ 1], ಖಂಡ[idx]);
		current_ಸ್ಥಾನಾಂತರ += n_ಬೈಟುಗಳು_read;
	}

	G.format_ವಿಳಾಸ(current_ಸ್ಥಾನಾಂತರ, '\n');

	ಆದರೆ ((option_mask32 & OPT_N) && current_ಸ್ಥಾನಾಂತರ >= ಕೊನೆ_ಸ್ಥಾನಾಂತರ)
		check_and_close();

	ಬಿಡುಗಡೆ(ಖಂಡ[0]);
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

ಸ್ಥಿರ ಶೂನ್ಯ
dump_ಅಕ್ಷರಸರಣಿಗಳು(off_t ವಿಳಾಸ, off_t ಕೊನೆ_ಸ್ಥಾನಾಂತರ)
{
	ಚಿಹ್ನೆರಹಿತ bufsize = MAX(100, G.string_min);
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *buf = xmalloc(bufsize);

	ಆಗ (1) {
		ಗಾತ್ರ_t i;
		ಪೂರ್ಣಾಂಕ c;

		/* See if the next 'G.string_min' chars are all printing chars.  */
 tryline:
		ಆದರೆ ((option_mask32 & OPT_N) && (ಕೊನೆ_ಸ್ಥಾನಾಂತರ - G.string_min <= ವಿಳಾಸ))
			ನಿಲ್ಲಿಸು;
		i = 0;
		ಆಗ (!(option_mask32 & OPT_N) || ವಿಳಾಸ < ಕೊನೆ_ಸ್ಥಾನಾಂತರ) {
			ಆದರೆ (i == bufsize) {
				bufsize += bufsize/8;
				buf = xrealloc(buf, bufsize);
			}

			ಆಗ (G.ಒಳ_ಹರಿವು) { /* !EOF */
				c = fgetc(G.ಒಳ_ಹರಿವು);
				ಆದರೆ (c != EOF)
					ಹೋಗು got_char;
				check_and_close();
				open_ಮುಂದಿನ_file();
			}
			/* EOF */
			ಹೋಗು ret;
 got_char:
			ವಿಳಾಸ++;
			ಆದರೆ (!c)
				ನಿಲ್ಲಿಸು;
			ಆದರೆ (!ISPRINT(c))
				ಹೋಗು tryline;	/* It isn't; give up on this string.  */
			buf[i++] = c;		/* String continues; store it all.  */
		}

		ಆದರೆ (i < G.string_min)		/* Too short! */
			ಹೋಗು tryline;

		/* If we get here, the string is all printable and NUL-terminated */
		buf[i] = 0;
		G.format_ವಿಳಾಸ(ವಿಳಾಸ - i - 1, ' ');

		ಗಾಗಿ (i = 0; (c = buf[i]); i++) {
			ಆಯ್ಕೆ (c) {
			ಸ್ಥಿತಿ '\007': fputs_stdout("\\a"); ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ '\b': fputs_stdout("\\b"); ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ '\f': fputs_stdout("\\f"); ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ '\n': fputs_stdout("\\n"); ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ '\r': fputs_stdout("\\r"); ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ '\t': fputs_stdout("\\t"); ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ '\v': fputs_stdout("\\v"); ನಿಲ್ಲಿಸು;
			ಪೂರ್ವನಿಯೋಜಿತ: ಅಕ್ಷರ_ಬರೆ(c);
			}
		}
		ಅಕ್ಷರ_ಬರೆ('\n');
	}

	/* We reach this point only if we search through
	   (max_bytes_to_format - G.string_min) bytes before reaching EOF.  */
	check_and_close();
 ret:
	ಬಿಡುಗಡೆ(buf);
}

#ಆದರೆ ENABLE_LONG_OPTS
/* If S is a valid traditional offset specification with an optional
   leading '+' return nonzero and set *OFFSET to the offset it denotes.  */

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
parse_old_ಸ್ಥಾನಾಂತರ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, off_t *ಸ್ಥಾನಾಂತರ)
{
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ suffix_mult Bb[] ALIGN_SUFFIX = {
		{ "B", 1024 },
		{ "b", 512 },
		{ "", 0 }
	};
	ಅಕ್ಷರ *p;
	ಪೂರ್ಣಾಂಕ radix;

	/* Skip over any leading '+'. */
	ಆದರೆ (s[0] == '+') ++s;
	ಆದರೆ (!ಅಂಕಿಯೇ(s[0])) ಹಿಂತಿರುಗಿ 0; /* not a number */

	/* Determine the radix we'll use to interpret S.  If there is a '.',
	 * it's decimal, otherwise, if the string begins with '0X'or '0x',
	 * it's hexadecimal, else octal.  */
	p = strchr(s, '.');
	radix = 8;
	ಆದರೆ (p) {
		p[0] = '\0'; /* cheating */
		radix = 10;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
		radix = 16;

	*ಸ್ಥಾನಾಂತರ = xstrtooff_sfx(s, radix, Bb);
	ಆದರೆ (p) p[0] = '.';

	ಹಿಂತಿರುಗಿ (*ಸ್ಥಾನಾಂತರ >= 0);
}
#ಆದರೆ_ಅಂತ್ಯ

ಪೂರ್ಣಾಂಕ od_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ od_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
#ಆದರೆ ENABLE_LONG_OPTS
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ od_longopts[] ALIGN1 =
		"skip-bytes\0"        Required_ವಾದ "j"
		"address-radix\0"     Required_ವಾದ "A"
		"read-bytes\0"        Required_ವಾದ "N"
		"format\0"            Required_ವಾದ "t"
		"output-duplicates\0" No_ವಾದ       "v"
		/* Yes, it's true: -S NUM, but --strings[=NUM]!
		 * that is, NUM is mandatory for -S but optional for --strings!
		 */
		"strings\0"           Optional_ವಾದ "S"
		"width\0"             Optional_ವಾದ "w"
		"traditional\0"       No_ವಾದ       "\xff"
		;
#ಆದರೆ_ಅಂತ್ಯ
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str_A, *str_N, *str_j, *str_S = "3";
	llist_t *lst_t = ಖಾಲಿ;
	ಚಿಹ್ನೆರಹಿತ opt;
	ಪೂರ್ಣಾಂಕ l_c_m;
	/* The number of input bytes to skip before formatting and writing.  */
	off_t n_ಬೈಟುಗಳು_ಗಮ್ಯ_skip = 0;
	/* The offset of the first byte after the last byte to be formatted.  */
	off_t ಕೊನೆ_ಸ್ಥಾನಾಂತರ = 0;
	/* The maximum number of bytes that will be formatted.  */
	off_t max_ಬೈಟುಗಳು_ಗಮ್ಯ_format = 0;

	INIT_G();

	/*G.spec = NULL; - already is */
	G.format_ವಿಳಾಸ = format_ವಿಳಾಸ_std;
	ವಿಳಾಸ_ತಳ_char = 'o';
	ವಿಳಾಸ_pad_len_char = '7';

	/* Parse command line */
	opt = OD_GETOPT32();
	argv += optind;
	ಆದರೆ (opt & OPT_A) {
		ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ doxn[] ALIGN1 = "doxn";
		ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ doxn_ವಿಳಾಸ_ತಳ_char[] ALIGN1 = {
			'u', 'o', 'x', /* '?' fourth one is not important */
		};
		ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ uint8_t doxn_ವಿಳಾಸ_pad_len_char[] ALIGN1 = {
			'7', '7', '6', /* '?' */
		};
		ಅಕ್ಷರ *p;
		ಪೂರ್ಣಾಂಕ pos;
		p = strchr(doxn, str_A[0]);
		ಆದರೆ (!p)
			bb_error_ಸಂದೇಶ_and_die("bad output address radix "
				"'%c' (must be [doxn])", str_A[0]);
		pos = p - doxn;
		ಆದರೆ (pos == 3) G.format_ವಿಳಾಸ = format_ವಿಳಾಸ_none;
		ವಿಳಾಸ_ತಳ_char = doxn_ವಿಳಾಸ_ತಳ_char[pos];
		ವಿಳಾಸ_pad_len_char = doxn_ವಿಳಾಸ_pad_len_char[pos];
	}
	ಆದರೆ (opt & OPT_N) {
		max_ಬೈಟುಗಳು_ಗಮ್ಯ_format = xstrtooff_sfx(str_N, 0, bkm_suffixes);
	}

	ಆದರೆ (opt & OPT_a) decode_format_string("a");
	ಆದರೆ (opt & OPT_b) decode_format_string("oC");
	ಆದರೆ (opt & OPT_c) decode_format_string("c");
	ಆದರೆ (opt & OPT_d) decode_format_string("u2");
	ಆದರೆ (opt & OPT_D) decode_format_string("uI");
	ಆದರೆ (opt & OPT_f) decode_format_string("fF");
	ಆದರೆ (opt & (OPT_h|OPT_x)) decode_format_string("x2");
	ಆದರೆ (opt & (OPT_H|OPT_X)) decode_format_string("xI");
	/* -I,L,l: depend on word width of the arch (what is "long"?) */
#ಆದರೆ ULONG_MAX > 0xffffffff
	ಆದರೆ (opt & OPT_i) decode_format_string("dI");
	ಆದರೆ (opt & (OPT_I|OPT_l|OPT_L)) decode_format_string("dL");
#ಇಲ್ಲದಿದ್ದರೆ
	/* 32-bit arch: -I,L,l are the same as -i */
	ಆದರೆ (opt & (OPT_i|OPT_I|OPT_l|OPT_L)) decode_format_string("dI");
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (opt & OPT_j) n_ಬೈಟುಗಳು_ಗಮ್ಯ_skip = xstrtooff_sfx(str_j, 0, bkm_suffixes);
	ಆದರೆ (opt & (OPT_o|OPT_B)) decode_format_string("o2");
	ಆದರೆ (opt & OPT_O) decode_format_string("oI");
	ಆಗ (lst_t) {
		decode_format_string(llist_ತೆಗೆ(&lst_t));
	}
	ಆದರೆ (opt & OPT_s) decode_format_string("d2");
	ಆದರೆ (opt & OPT_S) {
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

#ಆದರೆ ENABLE_LONG_OPTS
	ಆದರೆ (opt & OPT_traditional) {
		ಆದರೆ (argv[0]) {
			off_t pseudo_ಆರಂಭ = -1;
			off_t o1, o2;

			ಆದರೆ (!argv[1]) { /* one arg */
				ಆದರೆ (parse_old_ಸ್ಥಾನಾಂತರ(argv[0], &o1)) {
					/* od --traditional OFFSET */
					n_ಬೈಟುಗಳು_ಗಮ್ಯ_skip = o1;
					argv++;
				}
				/* od --traditional FILE */
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!argv[2]) { /* two args */
				ಆದರೆ (parse_old_ಸ್ಥಾನಾಂತರ(argv[0], &o1)
				 && parse_old_ಸ್ಥಾನಾಂತರ(argv[1], &o2)
				) {
					/* od --traditional OFFSET LABEL */
					n_ಬೈಟುಗಳು_ಗಮ್ಯ_skip = o1;
					pseudo_ಆರಂಭ = o2;
					argv += 2;
				} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (parse_old_ಸ್ಥಾನಾಂತರ(argv[1], &o2)) {
					/* od --traditional FILE OFFSET */
					n_ಬೈಟುಗಳು_ಗಮ್ಯ_skip = o2;
					argv[1] = ಖಾಲಿ;
				} ಇಲ್ಲದಿದ್ದರೆ {
					bb_error_ಸಂದೇಶ_and_die("invalid second argument '%s'", argv[1]);
				}
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!argv[3]) { /* three args */
				ಆದರೆ (parse_old_ಸ್ಥಾನಾಂತರ(argv[1], &o1)
				 && parse_old_ಸ್ಥಾನಾಂತರ(argv[2], &o2)
				) {
					/* od --traditional FILE OFFSET LABEL */
					n_ಬೈಟುಗಳು_ಗಮ್ಯ_skip = o1;
					pseudo_ಆರಂಭ = o2;
					argv[1] = ಖಾಲಿ;
				} ಇಲ್ಲದಿದ್ದರೆ {
					bb_simple_error_ಸಂದೇಶ_and_die("the last two arguments must be offsets");
				}
			} ಇಲ್ಲದಿದ್ದರೆ { /* >3 args */
				bb_simple_error_ಸಂದೇಶ_and_die("too many arguments");
			}

			ಆದರೆ (pseudo_ಆರಂಭ >= 0) {
				ಆದರೆ (G.format_ವಿಳಾಸ == format_ವಿಳಾಸ_none) {
					ವಿಳಾಸ_ತಳ_char = 'o';
					ವಿಳಾಸ_pad_len_char = '7';
					G.format_ವಿಳಾಸ = format_ವಿಳಾಸ_paren;
				} ಇಲ್ಲದಿದ್ದರೆ {
					G.format_ವಿಳಾಸ = format_ವಿಳಾಸ_label;
				}
				G_pseudo_ಸ್ಥಾನಾಂತರ = pseudo_ಆರಂಭ - n_ಬೈಟುಗಳು_ಗಮ್ಯ_skip;
			}
		}
		/* else: od --traditional (without args) */
	}
#ಆದರೆ_ಅಂತ್ಯ

	ಆದರೆ (option_mask32 & OPT_N) {
		ಕೊನೆ_ಸ್ಥಾನಾಂತರ = n_ಬೈಟುಗಳು_ಗಮ್ಯ_skip + max_ಬೈಟುಗಳು_ಗಮ್ಯ_format;
		ಆದರೆ (ಕೊನೆ_ಸ್ಥಾನಾಂತರ < n_ಬೈಟುಗಳು_ಗಮ್ಯ_skip)
			bb_simple_error_ಸಂದೇಶ_and_die("SKIP + SIZE is too large");
	}

	ಆದರೆ (G.n_specs == 0) {
		decode_format_string("o2");
		/*G.n_specs = 1; - done by decode_format_string */
	}

	/* If no files were listed on the command line,
	   set the global pointer FILE_LIST so that it
	   references the null-terminated list of one name: "-".  */
	G.file_list = bb_argv_dash;
	ಆದರೆ (argv[0]) {
		/* Set the global pointer FILE_LIST so that it
		   references the first file-argument on the command-line.  */
		G.file_list = (ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ *ಸ್ಥಿರಾಂಕ *) argv;
	}

	/* Open the first input file */
	open_ಮುಂದಿನ_file();
	/* Skip over any unwanted header bytes */
	skip(n_ಬೈಟುಗಳು_ಗಮ್ಯ_skip);
	ಆದರೆ (!G.ಒಳ_ಹರಿವು)
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;

	/* Compute output block length */
	l_c_m = get_lcm();

	ಆದರೆ (opt & OPT_w) { /* -w: width */
		ಆದರೆ (!G.ಬೈಟುಗಳು_ಪ್ರತಿ_ಖಂಡ || G.ಬೈಟುಗಳು_ಪ್ರತಿ_ಖಂಡ % l_c_m != 0) {
			bb_error_ಸಂದೇಶ("warning: invalid width %u; using %d instead",
					(ಚಿಹ್ನೆರಹಿತ)G.ಬೈಟುಗಳು_ಪ್ರತಿ_ಖಂಡ, l_c_m);
			G.ಬೈಟುಗಳು_ಪ್ರತಿ_ಖಂಡ = l_c_m;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		G.ಬೈಟುಗಳು_ಪ್ರತಿ_ಖಂಡ = l_c_m;
		ಆದರೆ (l_c_m < DEFAULT_ಬೈಟುಗಳು_ಪ್ರತಿ_ಖಂಡ)
			G.ಬೈಟುಗಳು_ಪ್ರತಿ_ಖಂಡ *= DEFAULT_ಬೈಟುಗಳು_ಪ್ರತಿ_ಖಂಡ / l_c_m;
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DEBUG
	{
		ಪೂರ್ಣಾಂಕ i;
		ಗಾಗಿ (i = 0; i < G.n_specs; i++) {
			ಬರೆ("%d: fmt='%s' width=%d\n",
				i, G.spec[i].fmt_string,
				ಅಗಲ_ಬೈಟುಗಳು[G.spec[i].ಗಾತ್ರ_2]);
		}
	}
#ಆದರೆ_ಅಂತ್ಯ

	ಆದರೆ (option_mask32 & OPT_S)
		dump_ಅಕ್ಷರಸರಣಿಗಳು(n_ಬೈಟುಗಳು_ಗಮ್ಯ_skip, ಕೊನೆ_ಸ್ಥಾನಾಂತರ);
	ಇಲ್ಲದಿದ್ದರೆ
		dump(n_ಬೈಟುಗಳು_ಗಮ್ಯ_skip, ಕೊನೆ_ಸ್ಥಾನಾಂತರ);

	ಆದರೆ (fclose(stdin))
		bb_simple_perror_ಸಂದೇಶ_and_die(bb_ಸಂದೇಶ_standard_ಒಳಹರಿವು);

	ಹಿಂತಿರುಗಿ G.exit_ಸಂಕೇತ;
}
