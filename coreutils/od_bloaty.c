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
#అన్తర్భావ "common_bufsiz.h"
#నిర్వచించు ప్రతిజ్ఞా(a) ((శూన్య)0)


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

గణన {
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

#నిర్వచించు OD_GETOPT32() getopt32long(argv, \
	"A:N:abcdDfhHiIj:lLoOBt:*vxXsS:w:+:", od_longopts, \
	/* -w with optional param */ \
	/* -S was -s and also had optional parameter */ \
	/* but in coreutils 6.3 it was renamed and now has */ \
	/* _mandatory_ parameter */ \
	&str_A, &str_N, &str_j, &lst_t, &str_S, &G.బైట్లు_చొప్పున_ఖండం)


/* Check for 0x7f is a coreutils 6.3 addition */
#నిర్వచించు ISPRINT(c) (((c) >= ' ') && (c) < 0x7f)

రకనిర్వచనం దీర్ఘ ద్విగుణ longdouble_t;
రకనిర్వచనం గుర్తులేని దీర్ఘ దీర్ఘ ulonglong_t;
రకనిర్వచనం దీర్ఘ దీర్ఘ llong;

#ఒకవేళ ENABLE_LFS
# define xstrtooff_sfx xstrtoull_sfx
#లేకపోతే
# define xstrtooff_sfx xstrtoul_sfx
#అయితే_ముగింపు

/* The default number of input bytes per output line.  */
#నిర్వచించు DEFAULT_బైట్లు_చొప్పున_ఖండం 16

/* The number of decimal digits of precision in a float.  */
#నిర్వచితం_కాకపోతే FLT_DIG
# define FLT_DIG 7
#అయితే_ముగింపు

/* The number of decimal digits of precision in a double.  */
#నిర్వచితం_కాకపోతే DBL_DIG
# define DBL_DIG 15
#అయితే_ముగింపు

/* The number of decimal digits of precision in a long double.  */
#నిర్వచితం_కాకపోతే LDBL_DIG
# define LDBL_DIG DBL_DIG
#అయితే_ముగింపు

గణన కొలత_spec {
	NO_కొలత,
	CHAR,
	SHORT,
	INT,
	LONG,
	LONG_LONG,
	FLOAT_SINGLE,
	FLOAT_DOUBLE,
	FLOAT_LONG_DOUBLE,
	N_కొలత_SPECS
};

గణన output_format {
	SIGNED_DECIMAL,
	UNSIGNED_DECIMAL,
	OCTAL,
	HEXADECIMAL,
	FLOATING_బిందువు,
	NAMED_CHARACTER,
	CHARACTER
};

/* Each output format specification (from '-t spec' or from
   old-style options) is represented by one of these structures.  */
నిర్మాణం tspec {
	గణన output_format fmt;
	గణన కొలత_spec కొలత;
	శూన్య (*print_function) (కొలత_t, స్థిరరాశి అక్షరం *, స్థిరరాశి అక్షరం *);
	అక్షరం *fmt_string;
	పూర్ణాంకం hexl_బహుళకం_trailer;
	పూర్ణాంకం క్షేత్రం_వెడల్పు;
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

స్థిర స్థిరరాశి uint8_t బైట్లు_గమ్యం_oct_digits[] ALIGN1 =
{0, 3, 6, 8, 11, 14, 16, 19, 22, 25, 27, 30, 32, 35, 38, 41, 43};

స్థిర స్థిరరాశి uint8_t బైట్లు_గమ్యం_signed_dec_digits[] ALIGN1 =
{1, 4, 6, 8, 11, 13, 16, 18, 20, 23, 25, 28, 30, 33, 35, 37, 40};

స్థిర స్థిరరాశి uint8_t బైట్లు_గమ్యం_unsigned_dec_digits[] ALIGN1 =
{0, 3, 5, 8, 10, 13, 15, 17, 20, 22, 25, 27, 29, 32, 34, 37, 39};

స్థిర స్థిరరాశి uint8_t బైట్లు_గమ్యం_hex_digits[] ALIGN1 =
{0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32};

/* Convert enum size_spec to the size of the named type.  */
స్థిర స్థిరరాశి గుర్తుగల అక్షరం వెడల్పు_బైట్లు[] ALIGN1 = {
	-1,
	పరిమాణం(అక్షరం),
	పరిమాణం(పొట్టి),
	పరిమాణం(పూర్ణాంకం),
	పరిమాణం(దీర్ఘ),
	పరిమాణం(ulonglong_t),
	పరిమాణం(దశాంశం),
	పరిమాణం(ద్విగుణ),
	పరిమాణం(longdouble_t)
};
/* Ensure that for each member of 'enum size_spec' there is an
   initializer in the width_bytes array.  */
నిర్మాణం ERR_వెడల్పు_బైట్లు_has_bad_కొలత {
	అక్షరం ERR_వెడల్పు_బైట్లు_has_bad_కొలత[ARRAY_కొలత(వెడల్పు_బైట్లు) == N_కొలత_SPECS ? 1 : -1];
};

నిర్మాణం globals {
	smallint exit_సంకేతం;

	గుర్తులేని string_min;

	/* An array of specs describing how to format each input block.  */
	గుర్తులేని n_specs;
	నిర్మాణం tspec *spec;

	/* Function that accepts an address and an optional following char,
	   and prints the address and char to stdout.  */
	శూన్య (*format_చిరునామా)(off_t, అక్షరం);

	/* The difference between the old-style pseudo starting address and
	   the number of bytes to skip.  */
#ఒకవేళ ENABLE_LONG_OPTS
	off_t pseudo_స్థానభ్రంశం;
# define G_pseudo_స్థానభ్రంశం G.pseudo_స్థానభ్రంశం
#అయితే_ముగింపు
	/* When zero, MAX_BYTES_TO_FORMAT and END_OFFSET are ignored, and all
	   input is formatted.  */

	/* The number of input bytes formatted per output line.  It must be
	   a multiple of the least common multiple of the sizes associated with
	   the specified output types.  It should be as large as possible, but
	   no larger than 16 -- unless specified with the -w option.  */
	గుర్తులేని బైట్లు_చొప్పున_ఖండం; /* have to use unsigned, not size_t */

	/* A NULL-terminated list of the file-arguments from the command line.  */
	స్థిరరాశి అక్షరం *స్థిరరాశి *file_list;

	/* The input stream associated with the current file.  */
	FILE *లోపల_ప్రవాహం;

	bool not_మొదటి;
	bool prev_pair_equal;

	అక్షరం చిరునామా_fmt[పరిమాణం("%0n"OFF_FMT"xc")];
} FIX_ALIASING;
/* Corresponds to 'x' above */
#నిర్వచించు చిరునామా_పునాది_char G.చిరునామా_fmt[పరిమాణం(G.చిరునామా_fmt)-3]
/* Corresponds to 'n' above */
#నిర్వచించు చిరునామా_pad_len_char G.చిరునామా_fmt[2]

#ఒకవేళ !ENABLE_LONG_OPTS
గణన { G_pseudo_స్థానభ్రంశం = 0 };
#అయితే_ముగింపు
#నిర్వచించు G (*(నిర్మాణం globals*)bb_common_bufsiz1)
#నిర్వచించు INIT_G() చేయి { \
	అమరిక_common_bufsiz(); \
	BUILD_BUG_ON(పరిమాణం(G) > COMMON_BUFSIZE); \
	G.బైట్లు_చొప్పున_ఖండం = 32; \
	strcpy(G.చిరునామా_fmt, "%0n"OFF_FMT"xc"); \
} అయితే (0)


#నిర్వచించు MAX_INTEGRAL_TYPE_కొలత పరిమాణం(ulonglong_t)
స్థిర స్థిరరాశి గుర్తులేని అక్షరం integral_type_కొలత[MAX_INTEGRAL_TYPE_కొలత + 1] ALIGN1 = {
	[పరిమాణం(అక్షరం)] = CHAR,
#ఒకవేళ USHRT_MAX != UCHAR_MAX
	[పరిమాణం(పొట్టి)] = SHORT,
#అయితే_ముగింపు
#ఒకవేళ UINT_MAX != USHRT_MAX
	[పరిమాణం(పూర్ణాంకం)] = INT,
#అయితే_ముగింపు
#ఒకవేళ ULONG_MAX != UINT_MAX
	[పరిమాణం(దీర్ఘ)] = LONG,
#అయితే_ముగింపు
#ఒకవేళ ULLONG_MAX != ULONG_MAX
	[పరిమాణం(ulonglong_t)] = LONG_LONG,
#అయితే_ముగింపు
};

#నిర్వచించు MAX_FP_TYPE_కొలత పరిమాణం(longdouble_t)
స్థిర స్థిరరాశి గుర్తులేని అక్షరం fp_type_కొలత[MAX_FP_TYPE_కొలత + 1] ALIGN1 = {
	/* gcc seems to allow repeated indexes. Last one wins */
	[పరిమాణం(longdouble_t)] = FLOAT_LONG_DOUBLE,
	[పరిమాణం(ద్విగుణ)] = FLOAT_DOUBLE,
	[పరిమాణం(దశాంశం)] = FLOAT_SINGLE
};


స్థిర గుర్తులేని
గరిష్ఠ_సామాన్య_భాజకం(గుర్తులేని u, గుర్తులేని v)
{
	గుర్తులేని t;
	అయితే (v != 0) {
		t = u % v;
		u = v;
		v = t;
	}
	తిరిగి u;
}

/* Compute the least common multiple of U and V.  */
స్థిర గుర్తులేని
lcm(గుర్తులేని u, గుర్తులేని v) {
	గుర్తులేని t = గరిష్ఠ_సామాన్య_భాజకం(u, v);
	ఒకవేళ (t == 0)
		తిరిగి 0;
	తిరిగి u * v / t;
}

స్థిర శూన్య
print_s_char(కొలత_t n_బైట్లు, స్థిరరాశి అక్షరం *ఖండం, స్థిరరాశి అక్షరం *fmt_string)
{
	అయితే (n_బైట్లు--) {
		పూర్ణాంకం tmp = *(గుర్తుగల అక్షరం *) ఖండం;
		రాయి(fmt_string, tmp);
		ఖండం += పరిమాణం(గుర్తులేని అక్షరం);
	}
}

స్థిర శూన్య
print_char(కొలత_t n_బైట్లు, స్థిరరాశి అక్షరం *ఖండం, స్థిరరాశి అక్షరం *fmt_string)
{
	అయితే (n_బైట్లు--) {
		గుర్తులేని tmp = *(గుర్తులేని అక్షరం *) ఖండం;
		రాయి(fmt_string, tmp);
		ఖండం += పరిమాణం(గుర్తులేని అక్షరం);
	}
}

స్థిర శూన్య
print_s_short(కొలత_t n_బైట్లు, స్థిరరాశి అక్షరం *ఖండం, స్థిరరాశి అక్షరం *fmt_string)
{
	n_బైట్లు /= పరిమాణం(గుర్తుగల పొట్టి);
	అయితే (n_బైట్లు--) {
		పూర్ణాంకం tmp = *(గుర్తుగల పొట్టి *) ఖండం;
		రాయి(fmt_string, tmp);
		ఖండం += పరిమాణం(గుర్తులేని పొట్టి);
	}
}

స్థిర శూన్య
print_short(కొలత_t n_బైట్లు, స్థిరరాశి అక్షరం *ఖండం, స్థిరరాశి అక్షరం *fmt_string)
{
	n_బైట్లు /= పరిమాణం(గుర్తులేని పొట్టి);
	అయితే (n_బైట్లు--) {
		గుర్తులేని tmp = *(గుర్తులేని పొట్టి *) ఖండం;
		రాయి(fmt_string, tmp);
		ఖండం += పరిమాణం(గుర్తులేని పొట్టి);
	}
}

స్థిర శూన్య
print_int(కొలత_t n_బైట్లు, స్థిరరాశి అక్షరం *ఖండం, స్థిరరాశి అక్షరం *fmt_string)
{
	n_బైట్లు /= పరిమాణం(గుర్తులేని);
	అయితే (n_బైట్లు--) {
		గుర్తులేని tmp = *(గుర్తులేని *) ఖండం;
		రాయి(fmt_string, tmp);
		ఖండం += పరిమాణం(గుర్తులేని);
	}
}

#ఒకవేళ UINT_MAX == ULONG_MAX
# define print_long print_int
#లేకపోతే
స్థిర శూన్య
print_long(కొలత_t n_బైట్లు, స్థిరరాశి అక్షరం *ఖండం, స్థిరరాశి అక్షరం *fmt_string)
{
	n_బైట్లు /= పరిమాణం(గుర్తులేని దీర్ఘ);
	అయితే (n_బైట్లు--) {
		గుర్తులేని దీర్ఘ tmp = *(గుర్తులేని దీర్ఘ *) ఖండం;
		రాయి(fmt_string, tmp);
		ఖండం += పరిమాణం(గుర్తులేని దీర్ఘ);
	}
}
#అయితే_ముగింపు

#ఒకవేళ ULONG_MAX == ULLONG_MAX
# define print_long_long print_long
#లేకపోతే
స్థిర శూన్య
print_long_long(కొలత_t n_బైట్లు, స్థిరరాశి అక్షరం *ఖండం, స్థిరరాశి అక్షరం *fmt_string)
{
	n_బైట్లు /= పరిమాణం(ulonglong_t);
	అయితే (n_బైట్లు--) {
		ulonglong_t tmp = *(ulonglong_t *) ఖండం;
		రాయి(fmt_string, tmp);
		ఖండం += పరిమాణం(ulonglong_t);
	}
}
#అయితే_ముగింపు

స్థిర శూన్య
print_float(కొలత_t n_బైట్లు, స్థిరరాశి అక్షరం *ఖండం, స్థిరరాశి అక్షరం *fmt_string)
{
	n_బైట్లు /= పరిమాణం(దశాంశం);
	అయితే (n_బైట్లు--) {
		దశాంశం tmp = *(దశాంశం *) ఖండం;
		రాయి(fmt_string, tmp);
		ఖండం += పరిమాణం(దశాంశం);
	}
}

స్థిర శూన్య
print_double(కొలత_t n_బైట్లు, స్థిరరాశి అక్షరం *ఖండం, స్థిరరాశి అక్షరం *fmt_string)
{
	n_బైట్లు /= పరిమాణం(ద్విగుణ);
	అయితే (n_బైట్లు--) {
		ద్విగుణ tmp = *(ద్విగుణ *) ఖండం;
		రాయి(fmt_string, tmp);
		ఖండం += పరిమాణం(ద్విగుణ);
	}
}

స్థిర శూన్య
print_long_double(కొలత_t n_బైట్లు, స్థిరరాశి అక్షరం *ఖండం, స్థిరరాశి అక్షరం *fmt_string)
{
	n_బైట్లు /= పరిమాణం(longdouble_t);
	అయితే (n_బైట్లు--) {
		longdouble_t tmp = *(longdouble_t *) ఖండం;
		రాయి(fmt_string, tmp);
		ఖండం += పరిమాణం(longdouble_t);
	}
}

/* print_[named]_ascii are optimized for speed.
 * Remember, someday you may want to pump gigabytes through this thing.
 * Saving a dozen of .text bytes here is counter-productive */

స్థిర శూన్య
print_named_ascii(కొలత_t n_బైట్లు, స్థిరరాశి అక్షరం *ఖండం,
		స్థిరరాశి అక్షరం *వాడని_fmt_string వాడని_PARAM)
{
	/* Names for some non-printing characters.  */
	స్థిర స్థిరరాశి అక్షరం charname[33][3] ALIGN1 = {
		"nul", "soh", "stx", "etx", "eot", "enq", "ack", "bel",
		" bs", " ht", " nl", " vt", " ff", " cr", " so", " si",
		"dle", "dc1", "dc2", "dc3", "dc4", "nak", "syn", "etb",
		"can", " em", "sub", "esc", " fs", " gs", " rs", " us",
		" sp"
	};
	// buf[N] pos:  01234 56789
	అక్షరం buf[12] = "   x\0 xxx\0";
	// [12] because we take three 32bit stack slots anyway, and
	// gcc is too dumb to initialize with constant stores,
	// it copies initializer from rodata. Oh well.
	// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=65410

	అయితే (n_బైట్లు--) {
		గుర్తులేని masked_c = *(గుర్తులేని అక్షరం *) ఖండం++;

		masked_c &= 0x7f;
		ఒకవేళ (masked_c == 0x7f) {
			fputs_stdout(" del");
			కొనసాగు;
		}
		ఒకవేళ (masked_c > ' ') {
			buf[3] = masked_c;
			fputs_stdout(buf);
			కొనసాగు;
		}
		/* Why? Because printf(" %3.3s") is much slower... */
		buf[6] = charname[masked_c][0];
		buf[7] = charname[masked_c][1];
		buf[8] = charname[masked_c][2];
		fputs_stdout(buf+5);
	}
}

స్థిర శూన్య
print_ascii(కొలత_t n_బైట్లు, స్థిరరాశి అక్షరం *ఖండం,
		స్థిరరాశి అక్షరం *వాడని_fmt_string వాడని_PARAM)
{
	// buf[N] pos:  01234 56789
	అక్షరం buf[12] = "   x\0 xxx\0";

	అయితే (n_బైట్లు--) {
		స్థిరరాశి అక్షరం *s;
		గుర్తులేని c = *(గుర్తులేని అక్షరం *) ఖండం++;

		ఒకవేళ (ISPRINT(c)) {
			buf[3] = c;
			fputs_stdout(buf);
			కొనసాగు;
		}
		ఎంపిక (c) {
		స్థితి '\0':
			s = "  \\0";
			ఆపు;
		స్థితి '\007':
			s = "  \\a";
			ఆపు;
		స్థితి '\b':
			s = "  \\b";
			ఆపు;
		స్థితి '\f':
			s = "  \\f";
			ఆపు;
		స్థితి '\n':
			s = "  \\n";
			ఆపు;
		స్థితి '\r':
			s = "  \\r";
			ఆపు;
		స్థితి '\t':
			s = "  \\t";
			ఆపు;
		స్థితి '\v':
			s = "  \\v";
			ఆపు;
		అప్రమేయ:
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

స్థిర శూన్య
open_తదుపరి_file(శూన్య)
{
	అయితే (1) {
		ఒకవేళ (!*G.file_list)
			తిరిగి;
		G.లోపల_ప్రవాహం = fopen_or_warn_stdin(*G.file_list++);
		ఒకవేళ (G.లోపల_ప్రవాహం) {
			ఆపు;
		}
		G.exit_సంకేతం = 1;
	}

	ఒకవేళ ((option_mask32 & (OPT_N|OPT_S)) == OPT_N)
		setbuf(G.లోపల_ప్రవాహం, ఖాళీ);
}

/* Test whether there have been errors on in_stream, and close it if
   it is not standard input.  Return nonzero if there has been an error
   on in_stream or stdout; return zero otherwise.  This function will
   report more than one error only if both a read and a write error
   have occurred.  IN_ERRNO, if nonzero, is the error number
   corresponding to the most recent action for IN_STREAM.  */

స్థిర శూన్య
check_and_close(శూన్య)
{
	ఒకవేళ (G.లోపల_ప్రవాహం) {
		ఒకవేళ (ferror(G.లోపల_ప్రవాహం))	{
			bb_error_సందేశం("%s: read error", (G.లోపల_ప్రవాహం == stdin)
					? bb_సందేశం_standard_ఆదానం
					: G.file_list[-1]
			);
			G.exit_సంకేతం = 1;
		}
		fclose_if_not_stdin(G.లోపల_ప్రవాహం);
		G.లోపల_ప్రవాహం = ఖాళీ;
	}

	ఒకవేళ (ferror(stdout)) {
		bb_simple_error_సందేశం_and_die(bb_సందేశం_write_error);
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

స్థిర NOINLINE స్థిరరాశి అక్షరం *
decode_one_format(స్థిరరాశి అక్షరం *s_orig, స్థిరరాశి అక్షరం *s, నిర్మాణం tspec *tspec)
{
	గణన కొలత_spec కొలత_spec;
	గుర్తులేని కొలత;
	గణన output_format fmt;
	స్థిరరాశి అక్షరం *p;
	అక్షరం *చివర;
	అక్షరం *fmt_string = ఖాళీ;
	శూన్య (*print_function) (కొలత_t, స్థిరరాశి అక్షరం *, స్థిరరాశి అక్షరం *);
	గుర్తులేని c;
	గుర్తులేని క్షేత్రం_వెడల్పు = 0;
	పూర్ణాంకం pos;

	ఎంపిక (*s) {
	స్థితి 'd':
	స్థితి 'o':
	స్థితి 'u':
	స్థితి 'x': {
		స్థిర స్థిరరాశి అక్షరం CSIL[] ALIGN1 = "CSIL";

		c = *s++;
		p = strchr(CSIL, *s);
		/* if *s == NUL, p != NULL! Testcase: "od -tx" */
		ఒకవేళ (!p || *p == '\0') {
			కొలత = పరిమాణం(పూర్ణాంకం);
			ఒకవేళ (అంకెనా(s[0])) {
				కొలత = bb_strtou(s, &చివర, 0);
				ఒకవేళ (errno == ERANGE
				 || MAX_INTEGRAL_TYPE_కొలత < కొలత
				 || integral_type_కొలత[కొలత] == NO_కొలత
				) {
					bb_error_సందేశం_and_die("invalid type string '%s'; "
						"%u-byte %s type is not supported",
						s_orig, కొలత, "integral");
				}
				s = చివర;
			}
		} లేకపోతే {
			స్థిర స్థిరరాశి uint8_t CSIL_sizeof[4] = {
				పరిమాణం(అక్షరం),
				పరిమాణం(పొట్టి),
				పరిమాణం(పూర్ణాంకం),
				పరిమాణం(దీర్ఘ),
			};
			కొలత = CSIL_sizeof[p - CSIL];
			s++; /* skip C/S/I/L */
		}

#నిర్వచించు ISPEC_గమ్యం_FORMAT(Spec, Min_format, Long_format, Max_format) \
	((Spec) == LONG_LONG ? (Max_format) \
	: ((Spec) == LONG ? (Long_format) : (Min_format)))

#నిర్వచించు FMT_బైట్లు_ALLOCATED 9
		కొలత_spec = integral_type_కొలత[కొలత];

		{
			స్థిర స్థిరరాశి అక్షరం doux[] ALIGN1 = "doux";
			స్థిర స్థిరరాశి అక్షరం doux_fmt_letter[][4] = {
				"lld", "llo", "llu", "llx"
			};
			స్థిర స్థిరరాశి గణన output_format doux_fmt[] = {
				SIGNED_DECIMAL,
				OCTAL,
				UNSIGNED_DECIMAL,
				HEXADECIMAL,
			};
			స్థిర స్థిరరాశి uint8_t *స్థిరరాశి doux_బైట్లు_గమ్యం_XXX[] = {
				బైట్లు_గమ్యం_signed_dec_digits,
				బైట్లు_గమ్యం_oct_digits,
				బైట్లు_గమ్యం_unsigned_dec_digits,
				బైట్లు_గమ్యం_hex_digits,
			};
			స్థిర స్థిరరాశి అక్షరం doux_fmtstring[][పరిమాణం(" %%0%u%s")] ALIGN1 = {
				" %%%u%s",
				" %%0%u%s",
				" %%%u%s",
				" %%0%u%s",
			};

			pos = strchr(doux, c) - doux;
			fmt = doux_fmt[pos];
			క్షేత్రం_వెడల్పు = doux_బైట్లు_గమ్యం_XXX[pos][కొలత];
			p = doux_fmt_letter[pos] + 2;
			ఒకవేళ (కొలత_spec == LONG) p--;
			ఒకవేళ (కొలత_spec == LONG_LONG) p -= 2;
			fmt_string = xasprintf(doux_fmtstring[pos], క్షేత్రం_వెడల్పు, p);
		}

		ఎంపిక (కొలత_spec) {
		స్థితి CHAR:
			print_function = (fmt == SIGNED_DECIMAL
				    ? print_s_char
				    : print_char);
			ఆపు;
		స్థితి SHORT:
			print_function = (fmt == SIGNED_DECIMAL
				    ? print_s_short
				    : print_short);
			ఆపు;
		స్థితి INT:
			print_function = print_int;
			ఆపు;
		స్థితి LONG:
			print_function = print_long;
			ఆపు;
		అప్రమేయ: /* case LONG_LONG: */
			print_function = print_long_long;
			ఆపు;
		}
		ఆపు;
	}

	స్థితి 'f': {
		స్థిర స్థిరరాశి అక్షరం FDL[] ALIGN1 = "FDL";

		fmt = FLOATING_బిందువు;
		++s;
		p = strchr(FDL, *s);
		ఒకవేళ (!p || *p == '\0') {
			కొలత = పరిమాణం(ద్విగుణ);
			ఒకవేళ (అంకెనా(s[0])) {
				కొలత = bb_strtou(s, &చివర, 0);
				ఒకవేళ (errno == ERANGE || కొలత > MAX_FP_TYPE_కొలత
				 || fp_type_కొలత[కొలత] == NO_కొలత
				) {
					bb_error_సందేశం_and_die("invalid type string '%s'; "
						"%u-byte %s type is not supported",
						s_orig, కొలత, "floating point");
				}
				s = చివర;
			}
		} లేకపోతే {
			స్థిర స్థిరరాశి uint8_t FDL_sizeof[] = {
				పరిమాణం(దశాంశం),
				పరిమాణం(ద్విగుణ),
				పరిమాణం(longdouble_t),
			};

			కొలత = FDL_sizeof[p - FDL];
			s++; /* skip F/D/L */
		}

		కొలత_spec = fp_type_కొలత[కొలత];

		ఎంపిక (కొలత_spec) {
		స్థితి FLOAT_SINGLE:
			print_function = print_float;
			క్షేత్రం_వెడల్పు = FLT_DIG + 8;
			/* Don't use %#e; not all systems support it.  */
			fmt_string = xasprintf(" %%%d.%de", క్షేత్రం_వెడల్పు, FLT_DIG);
			ఆపు;
		స్థితి FLOAT_DOUBLE:
			print_function = print_double;
			క్షేత్రం_వెడల్పు = DBL_DIG + 8;
			fmt_string = xasprintf(" %%%d.%de", క్షేత్రం_వెడల్పు, DBL_DIG);
			ఆపు;
		అప్రమేయ: /* case FLOAT_LONG_DOUBLE: */
			print_function = print_long_double;
			క్షేత్రం_వెడల్పు = LDBL_DIG + 8;
			fmt_string = xasprintf(" %%%d.%dLe", క్షేత్రం_వెడల్పు, LDBL_DIG);
			ఆపు;
		}
		ఆపు;
	}

	స్థితి 'a':
		++s;
		fmt = NAMED_CHARACTER;
		కొలత_spec = CHAR;
		print_function = print_named_ascii;
		క్షేత్రం_వెడల్పు = 3;
		ఆపు;
	స్థితి 'c':
		++s;
		fmt = CHARACTER;
		కొలత_spec = CHAR;
		print_function = print_ascii;
		క్షేత్రం_వెడల్పు = 3;
		ఆపు;
	అప్రమేయ:
		bb_error_సందేశం_and_die("invalid character '%c' "
				"in type string '%s'", *s, s_orig);
	}

	tspec->కొలత = కొలత_spec;
	tspec->fmt = fmt;
	tspec->print_function = print_function;
	tspec->fmt_string = fmt_string;

	tspec->క్షేత్రం_వెడల్పు = క్షేత్రం_వెడల్పు;
	tspec->hexl_బహుళకం_trailer = (*s == 'z');
	ఒకవేళ (tspec->hexl_బహుళకం_trailer)
		s++;

	తిరిగి s;
}

/* Decode the modern od format string S.  Append the decoded
   representation to the global array SPEC, reallocating SPEC if
   necessary.  */

స్థిర శూన్య
decode_format_string(స్థిరరాశి అక్షరం *s)
{
	స్థిరరాశి అక్షరం *s_orig = s;

	అయితే (*s != '\0') {
		నిర్మాణం tspec tspec;
		స్థిరరాశి అక్షరం *తదుపరి;

		తదుపరి = decode_one_format(s_orig, s, &tspec);

		ప్రతిజ్ఞా(s != తదుపరి);
		s = తదుపరి;
		G.spec = xrealloc_సదిశ(G.spec, 4, G.n_specs);
		జ్ఞాపకం_నకలు(&G.spec[G.n_specs], &tspec, పరిమాణం(G.spec[0]));
		G.n_specs++;
	}
}

/* Given a list of one or more input filenames FILE_LIST, set the global
   file pointer IN_STREAM to position N_SKIP in the concatenation of
   those files.  If any file operation fails or if there are fewer than
   N_SKIP bytes in the combined input, give an error message and return
   nonzero.  When possible, use seek rather than read operations to
   advance IN_STREAM.  */

స్థిర శూన్య
skip(off_t n_skip)
{
	ఒకవేళ (n_skip == 0)
		తిరిగి;

	అయితే (G.లోపల_ప్రవాహం) { /* !EOF */
		నిర్మాణం stat file_stats;

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
		ఒకవేళ (fstat(fileno(G.లోపల_ప్రవాహం), &file_stats) == 0
		 && S_ISREG(file_stats.st_బహుళకం) && file_stats.st_కొలత > 0
		) {
			ఒకవేళ (file_stats.st_కొలత < n_skip) {
				n_skip -= file_stats.st_కొలత;
				/* take "check & close / open_next" route */
			} లేకపోతే {
				ఒకవేళ (fseeko(G.లోపల_ప్రవాహం, n_skip, SEEK_CUR) != 0)
					G.exit_సంకేతం = 1;
				తిరిగి;
			}
		} లేకపోతే {
			/* If it's not a regular file with positive size,
			   position the file pointer by reading.  */
			అక్షరం buf[1024];
			కొలత_t n_బైట్లు_గమ్యం_read = 1024;
			కొలత_t n_బైట్లు_read;

			అయితే (n_skip > 0) {
				ఒకవేళ (n_skip < n_బైట్లు_గమ్యం_read)
					n_బైట్లు_గమ్యం_read = n_skip;
				n_బైట్లు_read = fread(buf, 1, n_బైట్లు_గమ్యం_read, G.లోపల_ప్రవాహం);
				n_skip -= n_బైట్లు_read;
				ఒకవేళ (n_బైట్లు_read != n_బైట్లు_గమ్యం_read)
					ఆపు; /* EOF on this file or error */
			}
		}
		ఒకవేళ (n_skip == 0)
			తిరిగి;

		check_and_close();
		open_తదుపరి_file();
	}

	ఒకవేళ (n_skip)
		bb_simple_error_సందేశం_and_die("can't skip past end of combined input");
}


రకనిర్వచనం శూన్య FN_format_చిరునామా(off_t చిరునామా, అక్షరం c);

స్థిర శూన్య
format_చిరునామా_none(off_t చిరునామా వాడని_PARAM, అక్షరం c వాడని_PARAM)
{
}

స్థిర శూన్య
format_చిరునామా_std(off_t చిరునామా, అక్షరం c)
{
	/* Corresponds to 'c' */
	G.చిరునామా_fmt[పరిమాణం(G.చిరునామా_fmt)-2] = c;
	రాయి(G.చిరునామా_fmt, చిరునామా);
}

#ఒకవేళ ENABLE_LONG_OPTS
/* only used with --traditional */
స్థిర శూన్య
format_చిరునామా_paren(off_t చిరునామా, అక్షరం c)
{
	అక్షరం_రాయి('(');
	format_చిరునామా_std(చిరునామా, ')');
	ఒకవేళ (c) అక్షరం_రాయి(c);
}

స్థిర శూన్య
format_చిరునామా_label(off_t చిరునామా, అక్షరం c)
{
	format_చిరునామా_std(చిరునామా, ' ');
	format_చిరునామా_paren(చిరునామా + G_pseudo_స్థానభ్రంశం, c);
}
#అయితే_ముగింపు

స్థిర శూన్య
dump_hexl_బహుళకం_trailer(కొలత_t n_బైట్లు, స్థిరరాశి అక్షరం *ఖండం)
{
	fputs_stdout("  >");
	అయితే (n_బైట్లు--) {
		గుర్తులేని c = *(గుర్తులేని అక్షరం *) ఖండం++;
		c = (ISPRINT(c) ? c : '.');
		అక్షరం_రాయి(c);
	}
	అక్షరం_రాయి('<');
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

స్థిర శూన్య
write_ఖండం(off_t current_స్థానభ్రంశం, కొలత_t n_బైట్లు,
		స్థిరరాశి అక్షరం *prev_ఖండం, స్థిరరాశి అక్షరం *curr_ఖండం)
{
	గుర్తులేని i;

	ఒకవేళ (!(option_mask32 & OPT_v)
	 && G.not_మొదటి
	 && n_బైట్లు == G.బైట్లు_చొప్పున_ఖండం
	 && memcmp(prev_ఖండం, curr_ఖండం, G.బైట్లు_చొప్పున_ఖండం) == 0
	) {
		ఒకవేళ (G.prev_pair_equal) {
			/* The two preceding blocks were equal, and the current
			   block is the same as the last one, so print nothing.  */
		} లేకపోతే {
			puts("*");
			G.prev_pair_equal = 1;
		}
	} లేకపోతే {
		G.not_మొదటి = 1;
		G.prev_pair_equal = 0;
		కొరకు (i = 0; i < G.n_specs; i++) {
			ఒకవేళ (i == 0)
				G.format_చిరునామా(current_స్థానభ్రంశం, '\0');
			లేకపోతే
				రాయి("%*s", చిరునామా_pad_len_char - '0', "");
			(*G.spec[i].print_function) (n_బైట్లు, curr_ఖండం, G.spec[i].fmt_string);
			ఒకవేళ (G.spec[i].hexl_బహుళకం_trailer) {
				/* space-pad out to full line width, then dump the trailer */
				గుర్తులేని datum_వెడల్పు = వెడల్పు_బైట్లు[G.spec[i].కొలత];
				గుర్తులేని blank_fields = (G.బైట్లు_చొప్పున_ఖండం - n_బైట్లు) / datum_వెడల్పు;
				గుర్తులేని క్షేత్రం_వెడల్పు = G.spec[i].క్షేత్రం_వెడల్పు + 1;
				రాయి("%*s", blank_fields * క్షేత్రం_వెడల్పు, "");
				dump_hexl_బహుళకం_trailer(n_బైట్లు, curr_ఖండం);
			}
			అక్షరం_రాయి('\n');
		}
	}
}

స్థిర శూన్య
read_ఖండం(కొలత_t n, అక్షరం *ఖండం, కొలత_t *n_బైట్లు_లోపల_బఫరు)
{
	ప్రతిజ్ఞా(0 < n && n <= G.బైట్లు_చొప్పున_ఖండం);

	*n_బైట్లు_లోపల_బఫరు = 0;

	ఒకవేళ (n == 0)
		తిరిగి;

	అయితే (G.లోపల_ప్రవాహం != ఖాళీ) { /* EOF.  */
		కొలత_t n_needed;
		కొలత_t n_read;

		n_needed = n - *n_బైట్లు_లోపల_బఫరు;
		n_read = fread(ఖండం + *n_బైట్లు_లోపల_బఫరు, 1, n_needed, G.లోపల_ప్రవాహం);
		*n_బైట్లు_లోపల_బఫరు += n_read;
		ఒకవేళ (n_read == n_needed)
			ఆపు;
		/* error check is done in check_and_close */
		check_and_close();
		open_తదుపరి_file();
	}
}

/* Return the least common multiple of the sizes associated
   with the format specs.  */

స్థిర పూర్ణాంకం
get_lcm(శూన్య)
{
	కొలత_t i;
	పూర్ణాంకం l_c_m = 1;

	కొరకు (i = 0; i < G.n_specs; i++)
		l_c_m = lcm(l_c_m, వెడల్పు_బైట్లు[(పూర్ణాంకం) G.spec[i].కొలత]);
	తిరిగి l_c_m;
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

స్థిర శూన్య
dump(off_t current_స్థానభ్రంశం, off_t చివర_స్థానభ్రంశం)
{
	అక్షరం *ఖండం[2];
	పూర్ణాంకం idx;
	కొలత_t n_బైట్లు_read;

	ఖండం[0] = xmalloc(2 * G.బైట్లు_చొప్పున_ఖండం);
	ఖండం[1] = ఖండం[0] + G.బైట్లు_చొప్పున_ఖండం;

	idx = 0;
	ఒకవేళ (option_mask32 & OPT_N) {
		అయితే (1) {
			కొలత_t n_needed;
			ఒకవేళ (current_స్థానభ్రంశం >= చివర_స్థానభ్రంశం) {
				n_బైట్లు_read = 0;
				ఆపు;
			}
			n_needed = MIN(చివర_స్థానభ్రంశం - current_స్థానభ్రంశం, (off_t) G.బైట్లు_చొప్పున_ఖండం);
			read_ఖండం(n_needed, ఖండం[idx], &n_బైట్లు_read);
			ఒకవేళ (n_బైట్లు_read < G.బైట్లు_చొప్పున_ఖండం)
				ఆపు;
			ప్రతిజ్ఞా(n_బైట్లు_read == G.బైట్లు_చొప్పున_ఖండం);
			write_ఖండం(current_స్థానభ్రంశం, n_బైట్లు_read, ఖండం[idx ^ 1], ఖండం[idx]);
			current_స్థానభ్రంశం += n_బైట్లు_read;
			idx ^= 1;
		}
	} లేకపోతే {
		అయితే (1) {
			read_ఖండం(G.బైట్లు_చొప్పున_ఖండం, ఖండం[idx], &n_బైట్లు_read);
			ఒకవేళ (n_బైట్లు_read < G.బైట్లు_చొప్పున_ఖండం)
				ఆపు;
			ప్రతిజ్ఞా(n_బైట్లు_read == G.బైట్లు_చొప్పున_ఖండం);
			write_ఖండం(current_స్థానభ్రంశం, n_బైట్లు_read, ఖండం[idx ^ 1], ఖండం[idx]);
			current_స్థానభ్రంశం += n_బైట్లు_read;
			idx ^= 1;
		}
	}

	ఒకవేళ (n_బైట్లు_read > 0) {
		పూర్ణాంకం l_c_m;
		కొలత_t బైట్లు_గమ్యం_write;

		l_c_m = get_lcm();

		/* Make bytes_to_write the smallest multiple of l_c_m that
		   is at least as large as n_bytes_read.  */
		బైట్లు_గమ్యం_write = l_c_m * ((n_బైట్లు_read + l_c_m - 1) / l_c_m);

		memset(ఖండం[idx] + n_బైట్లు_read, 0, బైట్లు_గమ్యం_write - n_బైట్లు_read);
		write_ఖండం(current_స్థానభ్రంశం, బైట్లు_గమ్యం_write,
				ఖండం[idx ^ 1], ఖండం[idx]);
		current_స్థానభ్రంశం += n_బైట్లు_read;
	}

	G.format_చిరునామా(current_స్థానభ్రంశం, '\n');

	ఒకవేళ ((option_mask32 & OPT_N) && current_స్థానభ్రంశం >= చివర_స్థానభ్రంశం)
		check_and_close();

	విడుదల(ఖండం[0]);
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

స్థిర శూన్య
dump_అక్షరశ్రేణులు(off_t చిరునామా, off_t చివర_స్థానభ్రంశం)
{
	గుర్తులేని bufsize = MAX(100, G.string_min);
	గుర్తులేని అక్షరం *buf = xmalloc(bufsize);

	అయితే (1) {
		కొలత_t i;
		పూర్ణాంకం c;

		/* See if the next 'G.string_min' chars are all printing chars.  */
 tryline:
		ఒకవేళ ((option_mask32 & OPT_N) && (చివర_స్థానభ్రంశం - G.string_min <= చిరునామా))
			ఆపు;
		i = 0;
		అయితే (!(option_mask32 & OPT_N) || చిరునామా < చివర_స్థానభ్రంశం) {
			ఒకవేళ (i == bufsize) {
				bufsize += bufsize/8;
				buf = xrealloc(buf, bufsize);
			}

			అయితే (G.లోపల_ప్రవాహం) { /* !EOF */
				c = fgetc(G.లోపల_ప్రవాహం);
				ఒకవేళ (c != EOF)
					వెళ్ళు got_char;
				check_and_close();
				open_తదుపరి_file();
			}
			/* EOF */
			వెళ్ళు ret;
 got_char:
			చిరునామా++;
			ఒకవేళ (!c)
				ఆపు;
			ఒకవేళ (!ISPRINT(c))
				వెళ్ళు tryline;	/* It isn't; give up on this string.  */
			buf[i++] = c;		/* String continues; store it all.  */
		}

		ఒకవేళ (i < G.string_min)		/* Too short! */
			వెళ్ళు tryline;

		/* If we get here, the string is all printable and NUL-terminated */
		buf[i] = 0;
		G.format_చిరునామా(చిరునామా - i - 1, ' ');

		కొరకు (i = 0; (c = buf[i]); i++) {
			ఎంపిక (c) {
			స్థితి '\007': fputs_stdout("\\a"); ఆపు;
			స్థితి '\b': fputs_stdout("\\b"); ఆపు;
			స్థితి '\f': fputs_stdout("\\f"); ఆపు;
			స్థితి '\n': fputs_stdout("\\n"); ఆపు;
			స్థితి '\r': fputs_stdout("\\r"); ఆపు;
			స్థితి '\t': fputs_stdout("\\t"); ఆపు;
			స్థితి '\v': fputs_stdout("\\v"); ఆపు;
			అప్రమేయ: అక్షరం_రాయి(c);
			}
		}
		అక్షరం_రాయి('\n');
	}

	/* We reach this point only if we search through
	   (max_bytes_to_format - G.string_min) bytes before reaching EOF.  */
	check_and_close();
 ret:
	విడుదల(buf);
}

#ఒకవేళ ENABLE_LONG_OPTS
/* If S is a valid traditional offset specification with an optional
   leading '+' return nonzero and set *OFFSET to the offset it denotes.  */

స్థిర పూర్ణాంకం
parse_old_స్థానభ్రంశం(స్థిరరాశి అక్షరం *s, off_t *స్థానభ్రంశం)
{
	స్థిర స్థిరరాశి నిర్మాణం suffix_mult Bb[] ALIGN_SUFFIX = {
		{ "B", 1024 },
		{ "b", 512 },
		{ "", 0 }
	};
	అక్షరం *p;
	పూర్ణాంకం radix;

	/* Skip over any leading '+'. */
	ఒకవేళ (s[0] == '+') ++s;
	ఒకవేళ (!అంకెనా(s[0])) తిరిగి 0; /* not a number */

	/* Determine the radix we'll use to interpret S.  If there is a '.',
	 * it's decimal, otherwise, if the string begins with '0X'or '0x',
	 * it's hexadecimal, else octal.  */
	p = strchr(s, '.');
	radix = 8;
	ఒకవేళ (p) {
		p[0] = '\0'; /* cheating */
		radix = 10;
	} లేకపోతే ఒకవేళ (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
		radix = 16;

	*స్థానభ్రంశం = xstrtooff_sfx(s, radix, Bb);
	ఒకవేళ (p) p[0] = '.';

	తిరిగి (*స్థానభ్రంశం >= 0);
}
#అయితే_ముగింపు

పూర్ణాంకం od_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం od_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
#ఒకవేళ ENABLE_LONG_OPTS
	స్థిర స్థిరరాశి అక్షరం od_longopts[] ALIGN1 =
		"skip-bytes\0"        Required_వాదం "j"
		"address-radix\0"     Required_వాదం "A"
		"read-bytes\0"        Required_వాదం "N"
		"format\0"            Required_వాదం "t"
		"output-duplicates\0" No_వాదం       "v"
		/* Yes, it's true: -S NUM, but --strings[=NUM]!
		 * that is, NUM is mandatory for -S but optional for --strings!
		 */
		"strings\0"           Optional_వాదం "S"
		"width\0"             Optional_వాదం "w"
		"traditional\0"       No_వాదం       "\xff"
		;
#అయితే_ముగింపు
	స్థిరరాశి అక్షరం *str_A, *str_N, *str_j, *str_S = "3";
	llist_t *lst_t = ఖాళీ;
	గుర్తులేని opt;
	పూర్ణాంకం l_c_m;
	/* The number of input bytes to skip before formatting and writing.  */
	off_t n_బైట్లు_గమ్యం_skip = 0;
	/* The offset of the first byte after the last byte to be formatted.  */
	off_t చివర_స్థానభ్రంశం = 0;
	/* The maximum number of bytes that will be formatted.  */
	off_t max_బైట్లు_గమ్యం_format = 0;

	INIT_G();

	/*G.spec = NULL; - already is */
	G.format_చిరునామా = format_చిరునామా_std;
	చిరునామా_పునాది_char = 'o';
	చిరునామా_pad_len_char = '7';

	/* Parse command line */
	opt = OD_GETOPT32();
	argv += optind;
	ఒకవేళ (opt & OPT_A) {
		స్థిర స్థిరరాశి అక్షరం doxn[] ALIGN1 = "doxn";
		స్థిర స్థిరరాశి అక్షరం doxn_చిరునామా_పునాది_char[] ALIGN1 = {
			'u', 'o', 'x', /* '?' fourth one is not important */
		};
		స్థిర స్థిరరాశి uint8_t doxn_చిరునామా_pad_len_char[] ALIGN1 = {
			'7', '7', '6', /* '?' */
		};
		అక్షరం *p;
		పూర్ణాంకం pos;
		p = strchr(doxn, str_A[0]);
		ఒకవేళ (!p)
			bb_error_సందేశం_and_die("bad output address radix "
				"'%c' (must be [doxn])", str_A[0]);
		pos = p - doxn;
		ఒకవేళ (pos == 3) G.format_చిరునామా = format_చిరునామా_none;
		చిరునామా_పునాది_char = doxn_చిరునామా_పునాది_char[pos];
		చిరునామా_pad_len_char = doxn_చిరునామా_pad_len_char[pos];
	}
	ఒకవేళ (opt & OPT_N) {
		max_బైట్లు_గమ్యం_format = xstrtooff_sfx(str_N, 0, bkm_suffixes);
	}

	ఒకవేళ (opt & OPT_a) decode_format_string("a");
	ఒకవేళ (opt & OPT_b) decode_format_string("oC");
	ఒకవేళ (opt & OPT_c) decode_format_string("c");
	ఒకవేళ (opt & OPT_d) decode_format_string("u2");
	ఒకవేళ (opt & OPT_D) decode_format_string("uI");
	ఒకవేళ (opt & OPT_f) decode_format_string("fF");
	ఒకవేళ (opt & (OPT_h|OPT_x)) decode_format_string("x2");
	ఒకవేళ (opt & (OPT_H|OPT_X)) decode_format_string("xI");
	/* -I,L,l: depend on word width of the arch (what is "long"?) */
#ఒకవేళ ULONG_MAX > 0xffffffff
	ఒకవేళ (opt & OPT_i) decode_format_string("dI");
	ఒకవేళ (opt & (OPT_I|OPT_l|OPT_L)) decode_format_string("dL");
#లేకపోతే
	/* 32-bit arch: -I,L,l are the same as -i */
	ఒకవేళ (opt & (OPT_i|OPT_I|OPT_l|OPT_L)) decode_format_string("dI");
#అయితే_ముగింపు
	ఒకవేళ (opt & OPT_j) n_బైట్లు_గమ్యం_skip = xstrtooff_sfx(str_j, 0, bkm_suffixes);
	ఒకవేళ (opt & (OPT_o|OPT_B)) decode_format_string("o2");
	ఒకవేళ (opt & OPT_O) decode_format_string("oI");
	అయితే (lst_t) {
		decode_format_string(llist_తీయి(&lst_t));
	}
	ఒకవేళ (opt & OPT_s) decode_format_string("d2");
	ఒకవేళ (opt & OPT_S) {
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

#ఒకవేళ ENABLE_LONG_OPTS
	ఒకవేళ (opt & OPT_traditional) {
		ఒకవేళ (argv[0]) {
			off_t pseudo_ప్రారంభం = -1;
			off_t o1, o2;

			ఒకవేళ (!argv[1]) { /* one arg */
				ఒకవేళ (parse_old_స్థానభ్రంశం(argv[0], &o1)) {
					/* od --traditional OFFSET */
					n_బైట్లు_గమ్యం_skip = o1;
					argv++;
				}
				/* od --traditional FILE */
			} లేకపోతే ఒకవేళ (!argv[2]) { /* two args */
				ఒకవేళ (parse_old_స్థానభ్రంశం(argv[0], &o1)
				 && parse_old_స్థానభ్రంశం(argv[1], &o2)
				) {
					/* od --traditional OFFSET LABEL */
					n_బైట్లు_గమ్యం_skip = o1;
					pseudo_ప్రారంభం = o2;
					argv += 2;
				} లేకపోతే ఒకవేళ (parse_old_స్థానభ్రంశం(argv[1], &o2)) {
					/* od --traditional FILE OFFSET */
					n_బైట్లు_గమ్యం_skip = o2;
					argv[1] = ఖాళీ;
				} లేకపోతే {
					bb_error_సందేశం_and_die("invalid second argument '%s'", argv[1]);
				}
			} లేకపోతే ఒకవేళ (!argv[3]) { /* three args */
				ఒకవేళ (parse_old_స్థానభ్రంశం(argv[1], &o1)
				 && parse_old_స్థానభ్రంశం(argv[2], &o2)
				) {
					/* od --traditional FILE OFFSET LABEL */
					n_బైట్లు_గమ్యం_skip = o1;
					pseudo_ప్రారంభం = o2;
					argv[1] = ఖాళీ;
				} లేకపోతే {
					bb_simple_error_సందేశం_and_die("the last two arguments must be offsets");
				}
			} లేకపోతే { /* >3 args */
				bb_simple_error_సందేశం_and_die("too many arguments");
			}

			ఒకవేళ (pseudo_ప్రారంభం >= 0) {
				ఒకవేళ (G.format_చిరునామా == format_చిరునామా_none) {
					చిరునామా_పునాది_char = 'o';
					చిరునామా_pad_len_char = '7';
					G.format_చిరునామా = format_చిరునామా_paren;
				} లేకపోతే {
					G.format_చిరునామా = format_చిరునామా_label;
				}
				G_pseudo_స్థానభ్రంశం = pseudo_ప్రారంభం - n_బైట్లు_గమ్యం_skip;
			}
		}
		/* else: od --traditional (without args) */
	}
#అయితే_ముగింపు

	ఒకవేళ (option_mask32 & OPT_N) {
		చివర_స్థానభ్రంశం = n_బైట్లు_గమ్యం_skip + max_బైట్లు_గమ్యం_format;
		ఒకవేళ (చివర_స్థానభ్రంశం < n_బైట్లు_గమ్యం_skip)
			bb_simple_error_సందేశం_and_die("SKIP + SIZE is too large");
	}

	ఒకవేళ (G.n_specs == 0) {
		decode_format_string("o2");
		/*G.n_specs = 1; - done by decode_format_string */
	}

	/* If no files were listed on the command line,
	   set the global pointer FILE_LIST so that it
	   references the null-terminated list of one name: "-".  */
	G.file_list = bb_argv_dash;
	ఒకవేళ (argv[0]) {
		/* Set the global pointer FILE_LIST so that it
		   references the first file-argument on the command-line.  */
		G.file_list = (అక్షరం స్థిరరాశి *స్థిరరాశి *) argv;
	}

	/* Open the first input file */
	open_తదుపరి_file();
	/* Skip over any unwanted header bytes */
	skip(n_బైట్లు_గమ్యం_skip);
	ఒకవేళ (!G.లోపల_ప్రవాహం)
		తిరిగి EXIT_FAILURE;

	/* Compute output block length */
	l_c_m = get_lcm();

	ఒకవేళ (opt & OPT_w) { /* -w: width */
		ఒకవేళ (!G.బైట్లు_చొప్పున_ఖండం || G.బైట్లు_చొప్పున_ఖండం % l_c_m != 0) {
			bb_error_సందేశం("warning: invalid width %u; using %d instead",
					(గుర్తులేని)G.బైట్లు_చొప్పున_ఖండం, l_c_m);
			G.బైట్లు_చొప్పున_ఖండం = l_c_m;
		}
	} లేకపోతే {
		G.బైట్లు_చొప్పున_ఖండం = l_c_m;
		ఒకవేళ (l_c_m < DEFAULT_బైట్లు_చొప్పున_ఖండం)
			G.బైట్లు_చొప్పున_ఖండం *= DEFAULT_బైట్లు_చొప్పున_ఖండం / l_c_m;
	}

#నిర్వచితమైతే DEBUG
	{
		పూర్ణాంకం i;
		కొరకు (i = 0; i < G.n_specs; i++) {
			రాయి("%d: fmt='%s' width=%d\n",
				i, G.spec[i].fmt_string,
				వెడల్పు_బైట్లు[G.spec[i].కొలత]);
		}
	}
#అయితే_ముగింపు

	ఒకవేళ (option_mask32 & OPT_S)
		dump_అక్షరశ్రేణులు(n_బైట్లు_గమ్యం_skip, చివర_స్థానభ్రంశం);
	లేకపోతే
		dump(n_బైట్లు_గమ్యం_skip, చివర_స్థానభ్రంశం);

	ఒకవేళ (fclose(stdin))
		bb_simple_perror_సందేశం_and_die(bb_సందేశం_standard_ఆదానం);

	తిరిగి G.exit_సంకేతం;
}
