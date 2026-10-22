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
#অন্তর্ভাব "common_bufsiz.h"
#define প্রতিজ্ঞা(a) ((শূন্য)0)


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

গণনা {
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

#define OD_GETOPT32() getopt32long(argv, \
	"A:N:abcdDfhHiIj:lLoOBt:*vxXsS:w:+:", od_longopts, \
	/* -w with optional param */ \
	/* -S was -s and also had optional parameter */ \
	/* but in coreutils 6.3 it was renamed and now has */ \
	/* _mandatory_ parameter */ \
	&str_A, &str_N, &str_j, &lst_t, &str_S, &G.bytes_per_block)


/* Check for 0x7f is a coreutils 6.3 addition */
#define ISPRINT(c) (((c) >= ' ') && (c) < 0x7f)

প্রকারলেখ long দ্বিগুণ longdouble_t;
প্রকারলেখ unsigned long long ulonglong_t;
প্রকারলেখ long long llong;

#যদি ENABLE_LFS
# define xstrtooff_sfx xstrtoull_sfx
#অন্যথা
# define xstrtooff_sfx xstrtoul_sfx
#endif

/* The default number of input bytes per output line.  */
#define DEFAULT_BYTES_PER_BLOCK 16

/* The number of decimal digits of precision in a float.  */
#ifndef FLT_DIG
# define FLT_DIG 7
#endif

/* The number of decimal digits of precision in a double.  */
#ifndef DBL_DIG
# define DBL_DIG 15
#endif

/* The number of decimal digits of precision in a long double.  */
#ifndef LDBL_DIG
# define LDBL_DIG DBL_DIG
#endif

গণনা আকার_spec {
	NO_আকার,
	CHAR,
	SHORT,
	INT,
	LONG,
	LONG_LONG,
	FLOAT_SINGLE,
	FLOAT_DOUBLE,
	FLOAT_LONG_DOUBLE,
	N_আকার_SPECS
};

গণনা output_format {
	SIGNED_DECIMAL,
	UNSIGNED_DECIMAL,
	OCTAL,
	HEXADECIMAL,
	FLOATING_POINT,
	NAMED_CHARACTER,
	CHARACTER
};

/* Each output format specification (from '-t spec' or from
   old-style options) is represented by one of these structures.  */
কাঠামো tspec {
	গণনা output_format fmt;
	গণনা আকার_spec আকার_2;
	শূন্য (*print_function) (আকার_t, ধ্রুবক অক্ষর *, ধ্রুবক অক্ষর *);
	অক্ষর *fmt_string;
	পূর্ণসংখ্যা hexl_mode_trailer;
	পূর্ণসংখ্যা field_width;
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

স্থির ধ্রুবক uint8_t bytes_to_oct_digits[] ALIGN1 =
{0, 3, 6, 8, 11, 14, 16, 19, 22, 25, 27, 30, 32, 35, 38, 41, 43};

স্থির ধ্রুবক uint8_t bytes_to_signed_dec_digits[] ALIGN1 =
{1, 4, 6, 8, 11, 13, 16, 18, 20, 23, 25, 28, 30, 33, 35, 37, 40};

স্থির ধ্রুবক uint8_t bytes_to_unsigned_dec_digits[] ALIGN1 =
{0, 3, 5, 8, 10, 13, 15, 17, 20, 22, 25, 27, 29, 32, 34, 37, 39};

স্থির ধ্রুবক uint8_t bytes_to_hex_digits[] ALIGN1 =
{0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32};

/* Convert enum size_spec to the size of the named type.  */
স্থির ধ্রুবক signed অক্ষর width_bytes[] ALIGN1 = {
	-1,
	আকার(অক্ষর),
	আকার(short),
	আকার(পূর্ণসংখ্যা),
	আকার(long),
	আকার(ulonglong_t),
	আকার(দশমিক),
	আকার(দ্বিগুণ),
	আকার(longdouble_t)
};
/* Ensure that for each member of 'enum size_spec' there is an
   initializer in the width_bytes array.  */
কাঠামো ERR_width_bytes_has_bad_আকার {
	অক্ষর ERR_width_bytes_has_bad_আকার[ARRAY_আকার(width_bytes) == N_আকার_SPECS ? 1 : -1];
};

কাঠামো globals {
	smallint exit_code;

	unsigned string_min;

	/* An array of specs describing how to format each input block.  */
	unsigned n_specs;
	কাঠামো tspec *spec;

	/* Function that accepts an address and an optional following char,
	   and prints the address and char to stdout.  */
	শূন্য (*format_address)(off_t, অক্ষর);

	/* The difference between the old-style pseudo starting address and
	   the number of bytes to skip.  */
#যদি ENABLE_LONG_OPTS
	off_t pseudo_offset;
# define G_pseudo_offset G.pseudo_offset
#endif
	/* When zero, MAX_BYTES_TO_FORMAT and END_OFFSET are ignored, and all
	   input is formatted.  */

	/* The number of input bytes formatted per output line.  It must be
	   a multiple of the least common multiple of the sizes associated with
	   the specified output types.  It should be as large as possible, but
	   no larger than 16 -- unless specified with the -w option.  */
	unsigned bytes_per_block; /* have to use unsigned, not size_t */

	/* A NULL-terminated list of the file-arguments from the command line.  */
	ধ্রুবক অক্ষর *ধ্রুবক *file_list;

	/* The input stream associated with the current file.  */
	FILE *in_stream;

	bool not_first;
	bool পূর্ব_pair_equal;

	অক্ষর address_fmt[আকার("%0n"OFF_FMT"xc")];
} FIX_ALIASING;
/* Corresponds to 'x' above */
#define address_base_char G.address_fmt[আকার(G.address_fmt)-3]
/* Corresponds to 'n' above */
#define address_pad_len_char G.address_fmt[2]

#যদি !ENABLE_LONG_OPTS
গণনা { G_pseudo_offset = 0 };
#endif
#define G (*(কাঠামো globals*)bb_common_bufsiz1)
#define INIT_G() করো { \
	setup_common_bufsiz(); \
	BUILD_BUG_ON(আকার(G) > COMMON_BUFSIZE); \
	G.bytes_per_block = 32; \
	strcpy(G.address_fmt, "%0n"OFF_FMT"xc"); \
} যখন (0)


#define MAX_INTEGRAL_TYPE_আকার আকার(ulonglong_t)
স্থির ধ্রুবক unsigned অক্ষর integral_type_আকার[MAX_INTEGRAL_TYPE_আকার + 1] ALIGN1 = {
	[আকার(অক্ষর)] = CHAR,
#যদি USHRT_MAX != UCHAR_MAX
	[আকার(short)] = SHORT,
#endif
#যদি UINT_MAX != USHRT_MAX
	[আকার(পূর্ণসংখ্যা)] = INT,
#endif
#যদি ULONG_MAX != UINT_MAX
	[আকার(long)] = LONG,
#endif
#যদি ULLONG_MAX != ULONG_MAX
	[আকার(ulonglong_t)] = LONG_LONG,
#endif
};

#define MAX_FP_TYPE_আকার আকার(longdouble_t)
স্থির ধ্রুবক unsigned অক্ষর fp_type_আকার[MAX_FP_TYPE_আকার + 1] ALIGN1 = {
	/* gcc seems to allow repeated indexes. Last one wins */
	[আকার(longdouble_t)] = FLOAT_LONG_DOUBLE,
	[আকার(দ্বিগুণ)] = FLOAT_DOUBLE,
	[আকার(দশমিক)] = FLOAT_SINGLE
};


স্থির unsigned
gcd(unsigned u, unsigned v)
{
	unsigned t;
	যখন (v != 0) {
		t = u % v;
		u = v;
		v = t;
	}
	ফেরত u;
}

/* Compute the least common multiple of U and V.  */
স্থির unsigned
lcm(unsigned u, unsigned v) {
	unsigned t = gcd(u, v);
	যদি (t == 0)
		ফেরত 0;
	ফেরত u * v / t;
}

স্থির শূন্য
print_s_char(আকার_t n_bytes, ধ্রুবক অক্ষর *block, ধ্রুবক অক্ষর *fmt_string)
{
	যখন (n_bytes--) {
		পূর্ণসংখ্যা tmp = *(signed অক্ষর *) block;
		printf(fmt_string, tmp);
		block += আকার(unsigned অক্ষর);
	}
}

স্থির শূন্য
print_char(আকার_t n_bytes, ধ্রুবক অক্ষর *block, ধ্রুবক অক্ষর *fmt_string)
{
	যখন (n_bytes--) {
		unsigned tmp = *(unsigned অক্ষর *) block;
		printf(fmt_string, tmp);
		block += আকার(unsigned অক্ষর);
	}
}

স্থির শূন্য
print_s_short(আকার_t n_bytes, ধ্রুবক অক্ষর *block, ধ্রুবক অক্ষর *fmt_string)
{
	n_bytes /= আকার(signed short);
	যখন (n_bytes--) {
		পূর্ণসংখ্যা tmp = *(signed short *) block;
		printf(fmt_string, tmp);
		block += আকার(unsigned short);
	}
}

স্থির শূন্য
print_short(আকার_t n_bytes, ধ্রুবক অক্ষর *block, ধ্রুবক অক্ষর *fmt_string)
{
	n_bytes /= আকার(unsigned short);
	যখন (n_bytes--) {
		unsigned tmp = *(unsigned short *) block;
		printf(fmt_string, tmp);
		block += আকার(unsigned short);
	}
}

স্থির শূন্য
print_int(আকার_t n_bytes, ধ্রুবক অক্ষর *block, ধ্রুবক অক্ষর *fmt_string)
{
	n_bytes /= আকার(unsigned);
	যখন (n_bytes--) {
		unsigned tmp = *(unsigned *) block;
		printf(fmt_string, tmp);
		block += আকার(unsigned);
	}
}

#যদি UINT_MAX == ULONG_MAX
# define print_long print_int
#অন্যথা
স্থির শূন্য
print_long(আকার_t n_bytes, ধ্রুবক অক্ষর *block, ধ্রুবক অক্ষর *fmt_string)
{
	n_bytes /= আকার(unsigned long);
	যখন (n_bytes--) {
		unsigned long tmp = *(unsigned long *) block;
		printf(fmt_string, tmp);
		block += আকার(unsigned long);
	}
}
#endif

#যদি ULONG_MAX == ULLONG_MAX
# define print_long_long print_long
#অন্যথা
স্থির শূন্য
print_long_long(আকার_t n_bytes, ধ্রুবক অক্ষর *block, ধ্রুবক অক্ষর *fmt_string)
{
	n_bytes /= আকার(ulonglong_t);
	যখন (n_bytes--) {
		ulonglong_t tmp = *(ulonglong_t *) block;
		printf(fmt_string, tmp);
		block += আকার(ulonglong_t);
	}
}
#endif

স্থির শূন্য
print_float(আকার_t n_bytes, ধ্রুবক অক্ষর *block, ধ্রুবক অক্ষর *fmt_string)
{
	n_bytes /= আকার(দশমিক);
	যখন (n_bytes--) {
		দশমিক tmp = *(দশমিক *) block;
		printf(fmt_string, tmp);
		block += আকার(দশমিক);
	}
}

স্থির শূন্য
print_double(আকার_t n_bytes, ধ্রুবক অক্ষর *block, ধ্রুবক অক্ষর *fmt_string)
{
	n_bytes /= আকার(দ্বিগুণ);
	যখন (n_bytes--) {
		দ্বিগুণ tmp = *(দ্বিগুণ *) block;
		printf(fmt_string, tmp);
		block += আকার(দ্বিগুণ);
	}
}

স্থির শূন্য
print_long_double(আকার_t n_bytes, ধ্রুবক অক্ষর *block, ধ্রুবক অক্ষর *fmt_string)
{
	n_bytes /= আকার(longdouble_t);
	যখন (n_bytes--) {
		longdouble_t tmp = *(longdouble_t *) block;
		printf(fmt_string, tmp);
		block += আকার(longdouble_t);
	}
}

/* print_[named]_ascii are optimized for speed.
 * Remember, someday you may want to pump gigabytes through this thing.
 * Saving a dozen of .text bytes here is counter-productive */

স্থির শূন্য
print_named_ascii(আকার_t n_bytes, ধ্রুবক অক্ষর *block,
		ধ্রুবক অক্ষর *unused_fmt_string UNUSED_PARAM)
{
	/* Names for some non-printing characters.  */
	স্থির ধ্রুবক অক্ষর charname[33][3] ALIGN1 = {
		"nul", "soh", "stx", "etx", "eot", "enq", "ack", "bel",
		" bs", " ht", " nl", " vt", " ff", " cr", " so", " si",
		"dle", "dc1", "dc2", "dc3", "dc4", "nak", "syn", "etb",
		"can", " em", "sub", "esc", " fs", " gs", " rs", " us",
		" sp"
	};
	// buf[N] pos:  01234 56789
	অক্ষর buf[12] = "   x\0 xxx\0";
	// [12] because we take three 32bit stack slots anyway, and
	// gcc is too dumb to initialize with constant stores,
	// it copies initializer from rodata. Oh well.
	// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=65410

	যখন (n_bytes--) {
		unsigned masked_c = *(unsigned অক্ষর *) block++;

		masked_c &= 0x7f;
		যদি (masked_c == 0x7f) {
			fputs_stdout(" del");
			চালু;
		}
		যদি (masked_c > ' ') {
			buf[3] = masked_c;
			fputs_stdout(buf);
			চালু;
		}
		/* Why? Because printf(" %3.3s") is much slower... */
		buf[6] = charname[masked_c][0];
		buf[7] = charname[masked_c][1];
		buf[8] = charname[masked_c][2];
		fputs_stdout(buf+5);
	}
}

স্থির শূন্য
print_ascii(আকার_t n_bytes, ধ্রুবক অক্ষর *block,
		ধ্রুবক অক্ষর *unused_fmt_string UNUSED_PARAM)
{
	// buf[N] pos:  01234 56789
	অক্ষর buf[12] = "   x\0 xxx\0";

	যখন (n_bytes--) {
		ধ্রুবক অক্ষর *s;
		unsigned c = *(unsigned অক্ষর *) block++;

		যদি (ISPRINT(c)) {
			buf[3] = c;
			fputs_stdout(buf);
			চালু;
		}
		নির্বাচন (c) {
		ক্ষেত্র '\0':
			s = "  \\0";
			বিরতি;
		ক্ষেত্র '\007':
			s = "  \\a";
			বিরতি;
		ক্ষেত্র '\b':
			s = "  \\b";
			বিরতি;
		ক্ষেত্র '\f':
			s = "  \\f";
			বিরতি;
		ক্ষেত্র '\n':
			s = "  \\n";
			বিরতি;
		ক্ষেত্র '\r':
			s = "  \\r";
			বিরতি;
		ক্ষেত্র '\t':
			s = "  \\t";
			বিরতি;
		ক্ষেত্র '\v':
			s = "  \\v";
			বিরতি;
		পূর্বনির্ধারিত:
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

স্থির শূন্য
open_পরবর্তী_file(শূন্য)
{
	যখন (1) {
		যদি (!*G.file_list)
			ফেরত;
		G.in_stream = fopen_or_warn_stdin(*G.file_list++);
		যদি (G.in_stream) {
			বিরতি;
		}
		G.exit_code = 1;
	}

	যদি ((option_mask32 & (OPT_N|OPT_S)) == OPT_N)
		setbuf(G.in_stream, ফাঁকা);
}

/* Test whether there have been errors on in_stream, and close it if
   it is not standard input.  Return nonzero if there has been an error
   on in_stream or stdout; return zero otherwise.  This function will
   report more than one error only if both a read and a write error
   have occurred.  IN_ERRNO, if nonzero, is the error number
   corresponding to the most recent action for IN_STREAM.  */

স্থির শূন্য
check_and_close(শূন্য)
{
	যদি (G.in_stream) {
		যদি (ferror(G.in_stream))	{
			bb_error_বার্তা("%s: read error", (G.in_stream == stdin)
					? bb_বার্তা_standard_প্রবেশ
					: G.file_list[-1]
			);
			G.exit_code = 1;
		}
		fclose_if_not_stdin(G.in_stream);
		G.in_stream = ফাঁকা;
	}

	যদি (ferror(stdout)) {
		bb_simple_error_বার্তা_and_die(bb_বার্তা_write_error);
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

স্থির NOINLINE ধ্রুবক অক্ষর *
decode_one_format(ধ্রুবক অক্ষর *s_orig, ধ্রুবক অক্ষর *s, কাঠামো tspec *tspec)
{
	গণনা আকার_spec আকার_spec;
	unsigned আকার_2;
	গণনা output_format fmt;
	ধ্রুবক অক্ষর *p;
	অক্ষর *end;
	অক্ষর *fmt_string = ফাঁকা;
	শূন্য (*print_function) (আকার_t, ধ্রুবক অক্ষর *, ধ্রুবক অক্ষর *);
	unsigned c;
	unsigned field_width = 0;
	পূর্ণসংখ্যা pos;

	নির্বাচন (*s) {
	ক্ষেত্র 'd':
	ক্ষেত্র 'o':
	ক্ষেত্র 'u':
	ক্ষেত্র 'x': {
		স্থির ধ্রুবক অক্ষর CSIL[] ALIGN1 = "CSIL";

		c = *s++;
		p = strchr(CSIL, *s);
		/* if *s == NUL, p != NULL! Testcase: "od -tx" */
		যদি (!p || *p == '\0') {
			আকার_2 = আকার(পূর্ণসংখ্যা);
			যদি (isdigit(s[0])) {
				আকার_2 = bb_strtou(s, &end, 0);
				যদি (errno == ERANGE
				 || MAX_INTEGRAL_TYPE_আকার < আকার_2
				 || integral_type_আকার[আকার_2] == NO_আকার
				) {
					bb_error_বার্তা_and_die("invalid type string '%s'; "
						"%u-byte %s type is not supported",
						s_orig, আকার_2, "integral");
				}
				s = end;
			}
		} অন্যথা {
			স্থির ধ্রুবক uint8_t CSIL_sizeof[4] = {
				আকার(অক্ষর),
				আকার(short),
				আকার(পূর্ণসংখ্যা),
				আকার(long),
			};
			আকার_2 = CSIL_sizeof[p - CSIL];
			s++; /* skip C/S/I/L */
		}

#define ISPEC_TO_FORMAT(Spec, Min_format, Long_format, Max_format) \
	((Spec) == LONG_LONG ? (Max_format) \
	: ((Spec) == LONG ? (Long_format) : (Min_format)))

#define FMT_BYTES_ALLOCATED 9
		আকার_spec = integral_type_আকার[আকার_2];

		{
			স্থির ধ্রুবক অক্ষর doux[] ALIGN1 = "doux";
			স্থির ধ্রুবক অক্ষর doux_fmt_letter[][4] = {
				"lld", "llo", "llu", "llx"
			};
			স্থির ধ্রুবক গণনা output_format doux_fmt[] = {
				SIGNED_DECIMAL,
				OCTAL,
				UNSIGNED_DECIMAL,
				HEXADECIMAL,
			};
			স্থির ধ্রুবক uint8_t *ধ্রুবক doux_bytes_to_XXX[] = {
				bytes_to_signed_dec_digits,
				bytes_to_oct_digits,
				bytes_to_unsigned_dec_digits,
				bytes_to_hex_digits,
			};
			স্থির ধ্রুবক অক্ষর doux_fmtstring[][আকার(" %%0%u%s")] ALIGN1 = {
				" %%%u%s",
				" %%0%u%s",
				" %%%u%s",
				" %%0%u%s",
			};

			pos = strchr(doux, c) - doux;
			fmt = doux_fmt[pos];
			field_width = doux_bytes_to_XXX[pos][আকার_2];
			p = doux_fmt_letter[pos] + 2;
			যদি (আকার_spec == LONG) p--;
			যদি (আকার_spec == LONG_LONG) p -= 2;
			fmt_string = xasprintf(doux_fmtstring[pos], field_width, p);
		}

		নির্বাচন (আকার_spec) {
		ক্ষেত্র CHAR:
			print_function = (fmt == SIGNED_DECIMAL
				    ? print_s_char
				    : print_char);
			বিরতি;
		ক্ষেত্র SHORT:
			print_function = (fmt == SIGNED_DECIMAL
				    ? print_s_short
				    : print_short);
			বিরতি;
		ক্ষেত্র INT:
			print_function = print_int;
			বিরতি;
		ক্ষেত্র LONG:
			print_function = print_long;
			বিরতি;
		পূর্বনির্ধারিত: /* case LONG_LONG: */
			print_function = print_long_long;
			বিরতি;
		}
		বিরতি;
	}

	ক্ষেত্র 'f': {
		স্থির ধ্রুবক অক্ষর FDL[] ALIGN1 = "FDL";

		fmt = FLOATING_POINT;
		++s;
		p = strchr(FDL, *s);
		যদি (!p || *p == '\0') {
			আকার_2 = আকার(দ্বিগুণ);
			যদি (isdigit(s[0])) {
				আকার_2 = bb_strtou(s, &end, 0);
				যদি (errno == ERANGE || আকার_2 > MAX_FP_TYPE_আকার
				 || fp_type_আকার[আকার_2] == NO_আকার
				) {
					bb_error_বার্তা_and_die("invalid type string '%s'; "
						"%u-byte %s type is not supported",
						s_orig, আকার_2, "floating point");
				}
				s = end;
			}
		} অন্যথা {
			স্থির ধ্রুবক uint8_t FDL_sizeof[] = {
				আকার(দশমিক),
				আকার(দ্বিগুণ),
				আকার(longdouble_t),
			};

			আকার_2 = FDL_sizeof[p - FDL];
			s++; /* skip F/D/L */
		}

		আকার_spec = fp_type_আকার[আকার_2];

		নির্বাচন (আকার_spec) {
		ক্ষেত্র FLOAT_SINGLE:
			print_function = print_float;
			field_width = FLT_DIG + 8;
			/* Don't use %#e; not all systems support it.  */
			fmt_string = xasprintf(" %%%d.%de", field_width, FLT_DIG);
			বিরতি;
		ক্ষেত্র FLOAT_DOUBLE:
			print_function = print_double;
			field_width = DBL_DIG + 8;
			fmt_string = xasprintf(" %%%d.%de", field_width, DBL_DIG);
			বিরতি;
		পূর্বনির্ধারিত: /* case FLOAT_LONG_DOUBLE: */
			print_function = print_long_double;
			field_width = LDBL_DIG + 8;
			fmt_string = xasprintf(" %%%d.%dLe", field_width, LDBL_DIG);
			বিরতি;
		}
		বিরতি;
	}

	ক্ষেত্র 'a':
		++s;
		fmt = NAMED_CHARACTER;
		আকার_spec = CHAR;
		print_function = print_named_ascii;
		field_width = 3;
		বিরতি;
	ক্ষেত্র 'c':
		++s;
		fmt = CHARACTER;
		আকার_spec = CHAR;
		print_function = print_ascii;
		field_width = 3;
		বিরতি;
	পূর্বনির্ধারিত:
		bb_error_বার্তা_and_die("invalid character '%c' "
				"in type string '%s'", *s, s_orig);
	}

	tspec->আকার_2 = আকার_spec;
	tspec->fmt = fmt;
	tspec->print_function = print_function;
	tspec->fmt_string = fmt_string;

	tspec->field_width = field_width;
	tspec->hexl_mode_trailer = (*s == 'z');
	যদি (tspec->hexl_mode_trailer)
		s++;

	ফেরত s;
}

/* Decode the modern od format string S.  Append the decoded
   representation to the global array SPEC, reallocating SPEC if
   necessary.  */

স্থির শূন্য
decode_format_string(ধ্রুবক অক্ষর *s)
{
	ধ্রুবক অক্ষর *s_orig = s;

	যখন (*s != '\0') {
		কাঠামো tspec tspec;
		ধ্রুবক অক্ষর *পরবর্তী;

		পরবর্তী = decode_one_format(s_orig, s, &tspec);

		প্রতিজ্ঞা(s != পরবর্তী);
		s = পরবর্তী;
		G.spec = xrealloc_vector(G.spec, 4, G.n_specs);
		memcpy(&G.spec[G.n_specs], &tspec, আকার(G.spec[0]));
		G.n_specs++;
	}
}

/* Given a list of one or more input filenames FILE_LIST, set the global
   file pointer IN_STREAM to position N_SKIP in the concatenation of
   those files.  If any file operation fails or if there are fewer than
   N_SKIP bytes in the combined input, give an error message and return
   nonzero.  When possible, use seek rather than read operations to
   advance IN_STREAM.  */

স্থির শূন্য
skip(off_t n_skip)
{
	যদি (n_skip == 0)
		ফেরত;

	যখন (G.in_stream) { /* !EOF */
		কাঠামো stat file_stats;

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
		যদি (fstat(fileno(G.in_stream), &file_stats) == 0
		 && S_ISREG(file_stats.st_mode) && file_stats.st_আকার > 0
		) {
			যদি (file_stats.st_আকার < n_skip) {
				n_skip -= file_stats.st_আকার;
				/* take "check & close / open_next" route */
			} অন্যথা {
				যদি (fseeko(G.in_stream, n_skip, SEEK_CUR) != 0)
					G.exit_code = 1;
				ফেরত;
			}
		} অন্যথা {
			/* If it's not a regular file with positive size,
			   position the file pointer by reading.  */
			অক্ষর buf[1024];
			আকার_t n_bytes_to_read = 1024;
			আকার_t n_bytes_read;

			যখন (n_skip > 0) {
				যদি (n_skip < n_bytes_to_read)
					n_bytes_to_read = n_skip;
				n_bytes_read = fread(buf, 1, n_bytes_to_read, G.in_stream);
				n_skip -= n_bytes_read;
				যদি (n_bytes_read != n_bytes_to_read)
					বিরতি; /* EOF on this file or error */
			}
		}
		যদি (n_skip == 0)
			ফেরত;

		check_and_close();
		open_পরবর্তী_file();
	}

	যদি (n_skip)
		bb_simple_error_বার্তা_and_die("can't skip past end of combined input");
}


প্রকারলেখ শূন্য FN_format_address(off_t address, অক্ষর c);

স্থির শূন্য
format_address_none(off_t address UNUSED_PARAM, অক্ষর c UNUSED_PARAM)
{
}

স্থির শূন্য
format_address_std(off_t address, অক্ষর c)
{
	/* Corresponds to 'c' */
	G.address_fmt[আকার(G.address_fmt)-2] = c;
	printf(G.address_fmt, address);
}

#যদি ENABLE_LONG_OPTS
/* only used with --traditional */
স্থির শূন্য
format_address_paren(off_t address, অক্ষর c)
{
	putchar('(');
	format_address_std(address, ')');
	যদি (c) putchar(c);
}

স্থির শূন্য
format_address_label(off_t address, অক্ষর c)
{
	format_address_std(address, ' ');
	format_address_paren(address + G_pseudo_offset, c);
}
#endif

স্থির শূন্য
dump_hexl_mode_trailer(আকার_t n_bytes, ধ্রুবক অক্ষর *block)
{
	fputs_stdout("  >");
	যখন (n_bytes--) {
		unsigned c = *(unsigned অক্ষর *) block++;
		c = (ISPRINT(c) ? c : '.');
		putchar(c);
	}
	putchar('<');
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

স্থির শূন্য
write_block(off_t current_offset, আকার_t n_bytes,
		ধ্রুবক অক্ষর *পূর্ব_block, ধ্রুবক অক্ষর *curr_block)
{
	unsigned i;

	যদি (!(option_mask32 & OPT_v)
	 && G.not_first
	 && n_bytes == G.bytes_per_block
	 && memcmp(পূর্ব_block, curr_block, G.bytes_per_block) == 0
	) {
		যদি (G.পূর্ব_pair_equal) {
			/* The two preceding blocks were equal, and the current
			   block is the same as the last one, so print nothing.  */
		} অন্যথা {
			puts("*");
			G.পূর্ব_pair_equal = 1;
		}
	} অন্যথা {
		G.not_first = 1;
		G.পূর্ব_pair_equal = 0;
		জন্য (i = 0; i < G.n_specs; i++) {
			যদি (i == 0)
				G.format_address(current_offset, '\0');
			অন্যথা
				printf("%*s", address_pad_len_char - '0', "");
			(*G.spec[i].print_function) (n_bytes, curr_block, G.spec[i].fmt_string);
			যদি (G.spec[i].hexl_mode_trailer) {
				/* space-pad out to full line width, then dump the trailer */
				unsigned datum_width = width_bytes[G.spec[i].আকার_2];
				unsigned blank_fields = (G.bytes_per_block - n_bytes) / datum_width;
				unsigned field_width = G.spec[i].field_width + 1;
				printf("%*s", blank_fields * field_width, "");
				dump_hexl_mode_trailer(n_bytes, curr_block);
			}
			putchar('\n');
		}
	}
}

স্থির শূন্য
read_block(আকার_t n, অক্ষর *block, আকার_t *n_bytes_in_বাফার)
{
	প্রতিজ্ঞা(0 < n && n <= G.bytes_per_block);

	*n_bytes_in_বাফার = 0;

	যদি (n == 0)
		ফেরত;

	যখন (G.in_stream != ফাঁকা) { /* EOF.  */
		আকার_t n_needed;
		আকার_t n_read;

		n_needed = n - *n_bytes_in_বাফার;
		n_read = fread(block + *n_bytes_in_বাফার, 1, n_needed, G.in_stream);
		*n_bytes_in_বাফার += n_read;
		যদি (n_read == n_needed)
			বিরতি;
		/* error check is done in check_and_close */
		check_and_close();
		open_পরবর্তী_file();
	}
}

/* Return the least common multiple of the sizes associated
   with the format specs.  */

স্থির পূর্ণসংখ্যা
get_lcm(শূন্য)
{
	আকার_t i;
	পূর্ণসংখ্যা l_c_m = 1;

	জন্য (i = 0; i < G.n_specs; i++)
		l_c_m = lcm(l_c_m, width_bytes[(পূর্ণসংখ্যা) G.spec[i].আকার_2]);
	ফেরত l_c_m;
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

স্থির শূন্য
dump(off_t current_offset, off_t end_offset)
{
	অক্ষর *block[2];
	পূর্ণসংখ্যা idx;
	আকার_t n_bytes_read;

	block[0] = xmalloc(2 * G.bytes_per_block);
	block[1] = block[0] + G.bytes_per_block;

	idx = 0;
	যদি (option_mask32 & OPT_N) {
		যখন (1) {
			আকার_t n_needed;
			যদি (current_offset >= end_offset) {
				n_bytes_read = 0;
				বিরতি;
			}
			n_needed = MIN(end_offset - current_offset, (off_t) G.bytes_per_block);
			read_block(n_needed, block[idx], &n_bytes_read);
			যদি (n_bytes_read < G.bytes_per_block)
				বিরতি;
			প্রতিজ্ঞা(n_bytes_read == G.bytes_per_block);
			write_block(current_offset, n_bytes_read, block[idx ^ 1], block[idx]);
			current_offset += n_bytes_read;
			idx ^= 1;
		}
	} অন্যথা {
		যখন (1) {
			read_block(G.bytes_per_block, block[idx], &n_bytes_read);
			যদি (n_bytes_read < G.bytes_per_block)
				বিরতি;
			প্রতিজ্ঞা(n_bytes_read == G.bytes_per_block);
			write_block(current_offset, n_bytes_read, block[idx ^ 1], block[idx]);
			current_offset += n_bytes_read;
			idx ^= 1;
		}
	}

	যদি (n_bytes_read > 0) {
		পূর্ণসংখ্যা l_c_m;
		আকার_t bytes_to_write;

		l_c_m = get_lcm();

		/* Make bytes_to_write the smallest multiple of l_c_m that
		   is at least as large as n_bytes_read.  */
		bytes_to_write = l_c_m * ((n_bytes_read + l_c_m - 1) / l_c_m);

		memset(block[idx] + n_bytes_read, 0, bytes_to_write - n_bytes_read);
		write_block(current_offset, bytes_to_write,
				block[idx ^ 1], block[idx]);
		current_offset += n_bytes_read;
	}

	G.format_address(current_offset, '\n');

	যদি ((option_mask32 & OPT_N) && current_offset >= end_offset)
		check_and_close();

	মুক্ত(block[0]);
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

স্থির শূন্য
dump_strings(off_t address, off_t end_offset)
{
	unsigned bufsize = MAX(100, G.string_min);
	unsigned অক্ষর *buf = xmalloc(bufsize);

	যখন (1) {
		আকার_t i;
		পূর্ণসংখ্যা c;

		/* See if the next 'G.string_min' chars are all printing chars.  */
 tryline:
		যদি ((option_mask32 & OPT_N) && (end_offset - G.string_min <= address))
			বিরতি;
		i = 0;
		যখন (!(option_mask32 & OPT_N) || address < end_offset) {
			যদি (i == bufsize) {
				bufsize += bufsize/8;
				buf = xrealloc(buf, bufsize);
			}

			যখন (G.in_stream) { /* !EOF */
				c = fgetc(G.in_stream);
				যদি (c != EOF)
					যাও got_char;
				check_and_close();
				open_পরবর্তী_file();
			}
			/* EOF */
			যাও ret;
 got_char:
			address++;
			যদি (!c)
				বিরতি;
			যদি (!ISPRINT(c))
				যাও tryline;	/* It isn't; give up on this string.  */
			buf[i++] = c;		/* String continues; store it all.  */
		}

		যদি (i < G.string_min)		/* Too short! */
			যাও tryline;

		/* If we get here, the string is all printable and NUL-terminated */
		buf[i] = 0;
		G.format_address(address - i - 1, ' ');

		জন্য (i = 0; (c = buf[i]); i++) {
			নির্বাচন (c) {
			ক্ষেত্র '\007': fputs_stdout("\\a"); বিরতি;
			ক্ষেত্র '\b': fputs_stdout("\\b"); বিরতি;
			ক্ষেত্র '\f': fputs_stdout("\\f"); বিরতি;
			ক্ষেত্র '\n': fputs_stdout("\\n"); বিরতি;
			ক্ষেত্র '\r': fputs_stdout("\\r"); বিরতি;
			ক্ষেত্র '\t': fputs_stdout("\\t"); বিরতি;
			ক্ষেত্র '\v': fputs_stdout("\\v"); বিরতি;
			পূর্বনির্ধারিত: putchar(c);
			}
		}
		putchar('\n');
	}

	/* We reach this point only if we search through
	   (max_bytes_to_format - G.string_min) bytes before reaching EOF.  */
	check_and_close();
 ret:
	মুক্ত(buf);
}

#যদি ENABLE_LONG_OPTS
/* If S is a valid traditional offset specification with an optional
   leading '+' return nonzero and set *OFFSET to the offset it denotes.  */

স্থির পূর্ণসংখ্যা
parse_old_offset(ধ্রুবক অক্ষর *s, off_t *offset)
{
	স্থির ধ্রুবক কাঠামো suffix_mult Bb[] ALIGN_SUFFIX = {
		{ "B", 1024 },
		{ "b", 512 },
		{ "", 0 }
	};
	অক্ষর *p;
	পূর্ণসংখ্যা radix;

	/* Skip over any leading '+'. */
	যদি (s[0] == '+') ++s;
	যদি (!isdigit(s[0])) ফেরত 0; /* not a number */

	/* Determine the radix we'll use to interpret S.  If there is a '.',
	 * it's decimal, otherwise, if the string begins with '0X'or '0x',
	 * it's hexadecimal, else octal.  */
	p = strchr(s, '.');
	radix = 8;
	যদি (p) {
		p[0] = '\0'; /* cheating */
		radix = 10;
	} অন্যথা যদি (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
		radix = 16;

	*offset = xstrtooff_sfx(s, radix, Bb);
	যদি (p) p[0] = '.';

	ফেরত (*offset >= 0);
}
#endif

পূর্ণসংখ্যা od_main(পূর্ণসংখ্যা argc, অক্ষর **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণসংখ্যা od_main(পূর্ণসংখ্যা argc UNUSED_PARAM, অক্ষর **argv)
{
#যদি ENABLE_LONG_OPTS
	স্থির ধ্রুবক অক্ষর od_longopts[] ALIGN1 =
		"skip-bytes\0"        Required_argument "j"
		"address-radix\0"     Required_argument "A"
		"read-bytes\0"        Required_argument "N"
		"format\0"            Required_argument "t"
		"output-duplicates\0" No_argument       "v"
		/* Yes, it's true: -S NUM, but --strings[=NUM]!
		 * that is, NUM is mandatory for -S but optional for --strings!
		 */
		"strings\0"           Optional_argument "S"
		"width\0"             Optional_argument "w"
		"traditional\0"       No_argument       "\xff"
		;
#endif
	ধ্রুবক অক্ষর *str_A, *str_N, *str_j, *str_S = "3";
	llist_t *lst_t = ফাঁকা;
	unsigned opt;
	পূর্ণসংখ্যা l_c_m;
	/* The number of input bytes to skip before formatting and writing.  */
	off_t n_bytes_to_skip = 0;
	/* The offset of the first byte after the last byte to be formatted.  */
	off_t end_offset = 0;
	/* The maximum number of bytes that will be formatted.  */
	off_t max_bytes_to_format = 0;

	INIT_G();

	/*G.spec = NULL; - already is */
	G.format_address = format_address_std;
	address_base_char = 'o';
	address_pad_len_char = '7';

	/* Parse command line */
	opt = OD_GETOPT32();
	argv += optind;
	যদি (opt & OPT_A) {
		স্থির ধ্রুবক অক্ষর doxn[] ALIGN1 = "doxn";
		স্থির ধ্রুবক অক্ষর doxn_address_base_char[] ALIGN1 = {
			'u', 'o', 'x', /* '?' fourth one is not important */
		};
		স্থির ধ্রুবক uint8_t doxn_address_pad_len_char[] ALIGN1 = {
			'7', '7', '6', /* '?' */
		};
		অক্ষর *p;
		পূর্ণসংখ্যা pos;
		p = strchr(doxn, str_A[0]);
		যদি (!p)
			bb_error_বার্তা_and_die("bad output address radix "
				"'%c' (must be [doxn])", str_A[0]);
		pos = p - doxn;
		যদি (pos == 3) G.format_address = format_address_none;
		address_base_char = doxn_address_base_char[pos];
		address_pad_len_char = doxn_address_pad_len_char[pos];
	}
	যদি (opt & OPT_N) {
		max_bytes_to_format = xstrtooff_sfx(str_N, 0, bkm_suffixes);
	}

	যদি (opt & OPT_a) decode_format_string("a");
	যদি (opt & OPT_b) decode_format_string("oC");
	যদি (opt & OPT_c) decode_format_string("c");
	যদি (opt & OPT_d) decode_format_string("u2");
	যদি (opt & OPT_D) decode_format_string("uI");
	যদি (opt & OPT_f) decode_format_string("fF");
	যদি (opt & (OPT_h|OPT_x)) decode_format_string("x2");
	যদি (opt & (OPT_H|OPT_X)) decode_format_string("xI");
	/* -I,L,l: depend on word width of the arch (what is "long"?) */
#যদি ULONG_MAX > 0xffffffff
	যদি (opt & OPT_i) decode_format_string("dI");
	যদি (opt & (OPT_I|OPT_l|OPT_L)) decode_format_string("dL");
#অন্যথা
	/* 32-bit arch: -I,L,l are the same as -i */
	যদি (opt & (OPT_i|OPT_I|OPT_l|OPT_L)) decode_format_string("dI");
#endif
	যদি (opt & OPT_j) n_bytes_to_skip = xstrtooff_sfx(str_j, 0, bkm_suffixes);
	যদি (opt & (OPT_o|OPT_B)) decode_format_string("o2");
	যদি (opt & OPT_O) decode_format_string("oI");
	যখন (lst_t) {
		decode_format_string(llist_বের_করো(&lst_t));
	}
	যদি (opt & OPT_s) decode_format_string("d2");
	যদি (opt & OPT_S) {
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

#যদি ENABLE_LONG_OPTS
	যদি (opt & OPT_traditional) {
		যদি (argv[0]) {
			off_t pseudo_start = -1;
			off_t o1, o2;

			যদি (!argv[1]) { /* one arg */
				যদি (parse_old_offset(argv[0], &o1)) {
					/* od --traditional OFFSET */
					n_bytes_to_skip = o1;
					argv++;
				}
				/* od --traditional FILE */
			} অন্যথা যদি (!argv[2]) { /* two args */
				যদি (parse_old_offset(argv[0], &o1)
				 && parse_old_offset(argv[1], &o2)
				) {
					/* od --traditional OFFSET LABEL */
					n_bytes_to_skip = o1;
					pseudo_start = o2;
					argv += 2;
				} অন্যথা যদি (parse_old_offset(argv[1], &o2)) {
					/* od --traditional FILE OFFSET */
					n_bytes_to_skip = o2;
					argv[1] = ফাঁকা;
				} অন্যথা {
					bb_error_বার্তা_and_die("invalid second argument '%s'", argv[1]);
				}
			} অন্যথা যদি (!argv[3]) { /* three args */
				যদি (parse_old_offset(argv[1], &o1)
				 && parse_old_offset(argv[2], &o2)
				) {
					/* od --traditional FILE OFFSET LABEL */
					n_bytes_to_skip = o1;
					pseudo_start = o2;
					argv[1] = ফাঁকা;
				} অন্যথা {
					bb_simple_error_বার্তা_and_die("the last two arguments must be offsets");
				}
			} অন্যথা { /* >3 args */
				bb_simple_error_বার্তা_and_die("too many arguments");
			}

			যদি (pseudo_start >= 0) {
				যদি (G.format_address == format_address_none) {
					address_base_char = 'o';
					address_pad_len_char = '7';
					G.format_address = format_address_paren;
				} অন্যথা {
					G.format_address = format_address_label;
				}
				G_pseudo_offset = pseudo_start - n_bytes_to_skip;
			}
		}
		/* else: od --traditional (without args) */
	}
#endif

	যদি (option_mask32 & OPT_N) {
		end_offset = n_bytes_to_skip + max_bytes_to_format;
		যদি (end_offset < n_bytes_to_skip)
			bb_simple_error_বার্তা_and_die("SKIP + SIZE is too large");
	}

	যদি (G.n_specs == 0) {
		decode_format_string("o2");
		/*G.n_specs = 1; - done by decode_format_string */
	}

	/* If no files were listed on the command line,
	   set the global pointer FILE_LIST so that it
	   references the null-terminated list of one name: "-".  */
	G.file_list = bb_argv_dash;
	যদি (argv[0]) {
		/* Set the global pointer FILE_LIST so that it
		   references the first file-argument on the command-line.  */
		G.file_list = (অক্ষর ধ্রুবক *ধ্রুবক *) argv;
	}

	/* Open the first input file */
	open_পরবর্তী_file();
	/* Skip over any unwanted header bytes */
	skip(n_bytes_to_skip);
	যদি (!G.in_stream)
		ফেরত EXIT_FAILURE;

	/* Compute output block length */
	l_c_m = get_lcm();

	যদি (opt & OPT_w) { /* -w: width */
		যদি (!G.bytes_per_block || G.bytes_per_block % l_c_m != 0) {
			bb_error_বার্তা("warning: invalid width %u; using %d instead",
					(unsigned)G.bytes_per_block, l_c_m);
			G.bytes_per_block = l_c_m;
		}
	} অন্যথা {
		G.bytes_per_block = l_c_m;
		যদি (l_c_m < DEFAULT_BYTES_PER_BLOCK)
			G.bytes_per_block *= DEFAULT_BYTES_PER_BLOCK / l_c_m;
	}

#ifdef DEBUG
	{
		পূর্ণসংখ্যা i;
		জন্য (i = 0; i < G.n_specs; i++) {
			printf("%d: fmt='%s' width=%d\n",
				i, G.spec[i].fmt_string,
				width_bytes[G.spec[i].আকার_2]);
		}
	}
#endif

	যদি (option_mask32 & OPT_S)
		dump_strings(n_bytes_to_skip, end_offset);
	অন্যথা
		dump(n_bytes_to_skip, end_offset);

	যদি (fclose(stdin))
		bb_simple_perror_বার্তা_and_die(bb_বার্তা_standard_প্রবেশ);

	ফেরত G.exit_code;
}
