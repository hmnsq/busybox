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
#அந்தர்பாவ "common_bufsiz.h"
#define ப்ரதிஜ்ஞா(a) ((வெறுமை)0)


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

எண்ணிக்கை {
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

வகைவரை long இரட்டை longdouble_t;
வகைவரை unsigned long long ulonglong_t;
வகைவரை long long llong;

#எனில் ENABLE_LFS
# define xstrtooff_sfx xstrtoull_sfx
#இல்லையெனில்
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

எண்ணிக்கை size_spec {
	NO_SIZE,
	CHAR,
	SHORT,
	INT,
	LONG,
	LONG_LONG,
	FLOAT_SINGLE,
	FLOAT_DOUBLE,
	FLOAT_LONG_DOUBLE,
	N_SIZE_SPECS
};

எண்ணிக்கை output_format {
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
கட்டமைப்பு tspec {
	எண்ணிக்கை output_format fmt;
	எண்ணிக்கை size_spec size;
	வெறுமை (*print_function) (size_t, மாறிலி எழுத்து *, மாறிலி எழுத்து *);
	எழுத்து *fmt_string;
	முழு hexl_mode_trailer;
	முழு field_width;
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

நிலையான மாறிலி uint8_t bytes_to_oct_digits[] ALIGN1 =
{0, 3, 6, 8, 11, 14, 16, 19, 22, 25, 27, 30, 32, 35, 38, 41, 43};

நிலையான மாறிலி uint8_t bytes_to_signed_dec_digits[] ALIGN1 =
{1, 4, 6, 8, 11, 13, 16, 18, 20, 23, 25, 28, 30, 33, 35, 37, 40};

நிலையான மாறிலி uint8_t bytes_to_unsigned_dec_digits[] ALIGN1 =
{0, 3, 5, 8, 10, 13, 15, 17, 20, 22, 25, 27, 29, 32, 34, 37, 39};

நிலையான மாறிலி uint8_t bytes_to_hex_digits[] ALIGN1 =
{0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32};

/* Convert enum size_spec to the size of the named type.  */
நிலையான மாறிலி signed எழுத்து width_bytes[] ALIGN1 = {
	-1,
	அளவு(எழுத்து),
	அளவு(short),
	அளவு(முழு),
	அளவு(long),
	அளவு(ulonglong_t),
	அளவு(தசம),
	அளவு(இரட்டை),
	அளவு(longdouble_t)
};
/* Ensure that for each member of 'enum size_spec' there is an
   initializer in the width_bytes array.  */
கட்டமைப்பு ERR_width_bytes_has_bad_size {
	எழுத்து ERR_width_bytes_has_bad_size[ARRAY_SIZE(width_bytes) == N_SIZE_SPECS ? 1 : -1];
};

கட்டமைப்பு globals {
	smallint exit_code;

	unsigned string_min;

	/* An array of specs describing how to format each input block.  */
	unsigned n_specs;
	கட்டமைப்பு tspec *spec;

	/* Function that accepts an address and an optional following char,
	   and prints the address and char to stdout.  */
	வெறுமை (*format_address)(off_t, எழுத்து);

	/* The difference between the old-style pseudo starting address and
	   the number of bytes to skip.  */
#எனில் ENABLE_LONG_OPTS
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
	மாறிலி எழுத்து *மாறிலி *file_list;

	/* The input stream associated with the current file.  */
	FILE *in_stream;

	bool not_first;
	bool prev_pair_equal;

	எழுத்து address_fmt[அளவு("%0n"OFF_FMT"xc")];
} FIX_ALIASING;
/* Corresponds to 'x' above */
#define address_base_char G.address_fmt[அளவு(G.address_fmt)-3]
/* Corresponds to 'n' above */
#define address_pad_len_char G.address_fmt[2]

#எனில் !ENABLE_LONG_OPTS
எண்ணிக்கை { G_pseudo_offset = 0 };
#endif
#define G (*(கட்டமைப்பு globals*)bb_common_bufsiz1)
#define INIT_G() செய் { \
	setup_common_bufsiz(); \
	BUILD_BUG_ON(அளவு(G) > COMMON_BUFSIZE); \
	G.bytes_per_block = 32; \
	strcpy(G.address_fmt, "%0n"OFF_FMT"xc"); \
} போது (0)


#define MAX_INTEGRAL_TYPE_SIZE அளவு(ulonglong_t)
நிலையான மாறிலி unsigned எழுத்து integral_type_size[MAX_INTEGRAL_TYPE_SIZE + 1] ALIGN1 = {
	[அளவு(எழுத்து)] = CHAR,
#எனில் USHRT_MAX != UCHAR_MAX
	[அளவு(short)] = SHORT,
#endif
#எனில் UINT_MAX != USHRT_MAX
	[அளவு(முழு)] = INT,
#endif
#எனில் ULONG_MAX != UINT_MAX
	[அளவு(long)] = LONG,
#endif
#எனில் ULLONG_MAX != ULONG_MAX
	[அளவு(ulonglong_t)] = LONG_LONG,
#endif
};

#define MAX_FP_TYPE_SIZE அளவு(longdouble_t)
நிலையான மாறிலி unsigned எழுத்து fp_type_size[MAX_FP_TYPE_SIZE + 1] ALIGN1 = {
	/* gcc seems to allow repeated indexes. Last one wins */
	[அளவு(longdouble_t)] = FLOAT_LONG_DOUBLE,
	[அளவு(இரட்டை)] = FLOAT_DOUBLE,
	[அளவு(தசம)] = FLOAT_SINGLE
};


நிலையான unsigned
gcd(unsigned u, unsigned v)
{
	unsigned t;
	போது (v != 0) {
		t = u % v;
		u = v;
		v = t;
	}
	திரும்பு u;
}

/* Compute the least common multiple of U and V.  */
நிலையான unsigned
lcm(unsigned u, unsigned v) {
	unsigned t = gcd(u, v);
	எனில் (t == 0)
		திரும்பு 0;
	திரும்பு u * v / t;
}

நிலையான வெறுமை
print_s_char(size_t n_bytes, மாறிலி எழுத்து *block, மாறிலி எழுத்து *fmt_string)
{
	போது (n_bytes--) {
		முழு tmp = *(signed எழுத்து *) block;
		printf(fmt_string, tmp);
		block += அளவு(unsigned எழுத்து);
	}
}

நிலையான வெறுமை
print_char(size_t n_bytes, மாறிலி எழுத்து *block, மாறிலி எழுத்து *fmt_string)
{
	போது (n_bytes--) {
		unsigned tmp = *(unsigned எழுத்து *) block;
		printf(fmt_string, tmp);
		block += அளவு(unsigned எழுத்து);
	}
}

நிலையான வெறுமை
print_s_short(size_t n_bytes, மாறிலி எழுத்து *block, மாறிலி எழுத்து *fmt_string)
{
	n_bytes /= அளவு(signed short);
	போது (n_bytes--) {
		முழு tmp = *(signed short *) block;
		printf(fmt_string, tmp);
		block += அளவு(unsigned short);
	}
}

நிலையான வெறுமை
print_short(size_t n_bytes, மாறிலி எழுத்து *block, மாறிலி எழுத்து *fmt_string)
{
	n_bytes /= அளவு(unsigned short);
	போது (n_bytes--) {
		unsigned tmp = *(unsigned short *) block;
		printf(fmt_string, tmp);
		block += அளவு(unsigned short);
	}
}

நிலையான வெறுமை
print_int(size_t n_bytes, மாறிலி எழுத்து *block, மாறிலி எழுத்து *fmt_string)
{
	n_bytes /= அளவு(unsigned);
	போது (n_bytes--) {
		unsigned tmp = *(unsigned *) block;
		printf(fmt_string, tmp);
		block += அளவு(unsigned);
	}
}

#எனில் UINT_MAX == ULONG_MAX
# define print_long print_int
#இல்லையெனில்
நிலையான வெறுமை
print_long(size_t n_bytes, மாறிலி எழுத்து *block, மாறிலி எழுத்து *fmt_string)
{
	n_bytes /= அளவு(unsigned long);
	போது (n_bytes--) {
		unsigned long tmp = *(unsigned long *) block;
		printf(fmt_string, tmp);
		block += அளவு(unsigned long);
	}
}
#endif

#எனில் ULONG_MAX == ULLONG_MAX
# define print_long_long print_long
#இல்லையெனில்
நிலையான வெறுமை
print_long_long(size_t n_bytes, மாறிலி எழுத்து *block, மாறிலி எழுத்து *fmt_string)
{
	n_bytes /= அளவு(ulonglong_t);
	போது (n_bytes--) {
		ulonglong_t tmp = *(ulonglong_t *) block;
		printf(fmt_string, tmp);
		block += அளவு(ulonglong_t);
	}
}
#endif

நிலையான வெறுமை
print_float(size_t n_bytes, மாறிலி எழுத்து *block, மாறிலி எழுத்து *fmt_string)
{
	n_bytes /= அளவு(தசம);
	போது (n_bytes--) {
		தசம tmp = *(தசம *) block;
		printf(fmt_string, tmp);
		block += அளவு(தசம);
	}
}

நிலையான வெறுமை
print_double(size_t n_bytes, மாறிலி எழுத்து *block, மாறிலி எழுத்து *fmt_string)
{
	n_bytes /= அளவு(இரட்டை);
	போது (n_bytes--) {
		இரட்டை tmp = *(இரட்டை *) block;
		printf(fmt_string, tmp);
		block += அளவு(இரட்டை);
	}
}

நிலையான வெறுமை
print_long_double(size_t n_bytes, மாறிலி எழுத்து *block, மாறிலி எழுத்து *fmt_string)
{
	n_bytes /= அளவு(longdouble_t);
	போது (n_bytes--) {
		longdouble_t tmp = *(longdouble_t *) block;
		printf(fmt_string, tmp);
		block += அளவு(longdouble_t);
	}
}

/* print_[named]_ascii are optimized for speed.
 * Remember, someday you may want to pump gigabytes through this thing.
 * Saving a dozen of .text bytes here is counter-productive */

நிலையான வெறுமை
print_named_ascii(size_t n_bytes, மாறிலி எழுத்து *block,
		மாறிலி எழுத்து *unused_fmt_string UNUSED_PARAM)
{
	/* Names for some non-printing characters.  */
	நிலையான மாறிலி எழுத்து charname[33][3] ALIGN1 = {
		"nul", "soh", "stx", "etx", "eot", "enq", "ack", "bel",
		" bs", " ht", " nl", " vt", " ff", " cr", " so", " si",
		"dle", "dc1", "dc2", "dc3", "dc4", "nak", "syn", "etb",
		"can", " em", "sub", "esc", " fs", " gs", " rs", " us",
		" sp"
	};
	// buf[N] pos:  01234 56789
	எழுத்து buf[12] = "   x\0 xxx\0";
	// [12] because we take three 32bit stack slots anyway, and
	// gcc is too dumb to initialize with constant stores,
	// it copies initializer from rodata. Oh well.
	// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=65410

	போது (n_bytes--) {
		unsigned masked_c = *(unsigned எழுத்து *) block++;

		masked_c &= 0x7f;
		எனில் (masked_c == 0x7f) {
			fputs_stdout(" del");
			தொடர்;
		}
		எனில் (masked_c > ' ') {
			buf[3] = masked_c;
			fputs_stdout(buf);
			தொடர்;
		}
		/* Why? Because printf(" %3.3s") is much slower... */
		buf[6] = charname[masked_c][0];
		buf[7] = charname[masked_c][1];
		buf[8] = charname[masked_c][2];
		fputs_stdout(buf+5);
	}
}

நிலையான வெறுமை
print_ascii(size_t n_bytes, மாறிலி எழுத்து *block,
		மாறிலி எழுத்து *unused_fmt_string UNUSED_PARAM)
{
	// buf[N] pos:  01234 56789
	எழுத்து buf[12] = "   x\0 xxx\0";

	போது (n_bytes--) {
		மாறிலி எழுத்து *s;
		unsigned c = *(unsigned எழுத்து *) block++;

		எனில் (ISPRINT(c)) {
			buf[3] = c;
			fputs_stdout(buf);
			தொடர்;
		}
		தேர்வு (c) {
		நிலை '\0':
			s = "  \\0";
			நிறுத்து;
		நிலை '\007':
			s = "  \\a";
			நிறுத்து;
		நிலை '\b':
			s = "  \\b";
			நிறுத்து;
		நிலை '\f':
			s = "  \\f";
			நிறுத்து;
		நிலை '\n':
			s = "  \\n";
			நிறுத்து;
		நிலை '\r':
			s = "  \\r";
			நிறுத்து;
		நிலை '\t':
			s = "  \\t";
			நிறுத்து;
		நிலை '\v':
			s = "  \\v";
			நிறுத்து;
		இயல்பு:
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

நிலையான வெறுமை
open_next_file(வெறுமை)
{
	போது (1) {
		எனில் (!*G.file_list)
			திரும்பு;
		G.in_stream = fopen_or_warn_stdin(*G.file_list++);
		எனில் (G.in_stream) {
			நிறுத்து;
		}
		G.exit_code = 1;
	}

	எனில் ((option_mask32 & (OPT_N|OPT_S)) == OPT_N)
		setbuf(G.in_stream, வெற்று);
}

/* Test whether there have been errors on in_stream, and close it if
   it is not standard input.  Return nonzero if there has been an error
   on in_stream or stdout; return zero otherwise.  This function will
   report more than one error only if both a read and a write error
   have occurred.  IN_ERRNO, if nonzero, is the error number
   corresponding to the most recent action for IN_STREAM.  */

நிலையான வெறுமை
check_and_close(வெறுமை)
{
	எனில் (G.in_stream) {
		எனில் (ferror(G.in_stream))	{
			bb_error_செய்தி("%s: read error", (G.in_stream == stdin)
					? bb_செய்தி_standard_உள்ளீடு
					: G.file_list[-1]
			);
			G.exit_code = 1;
		}
		fclose_if_not_stdin(G.in_stream);
		G.in_stream = வெற்று;
	}

	எனில் (ferror(stdout)) {
		bb_simple_error_செய்தி_and_die(bb_செய்தி_write_error);
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

நிலையான NOINLINE மாறிலி எழுத்து *
decode_one_format(மாறிலி எழுத்து *s_orig, மாறிலி எழுத்து *s, கட்டமைப்பு tspec *tspec)
{
	எண்ணிக்கை size_spec size_spec;
	unsigned size;
	எண்ணிக்கை output_format fmt;
	மாறிலி எழுத்து *p;
	எழுத்து *end;
	எழுத்து *fmt_string = வெற்று;
	வெறுமை (*print_function) (size_t, மாறிலி எழுத்து *, மாறிலி எழுத்து *);
	unsigned c;
	unsigned field_width = 0;
	முழு pos;

	தேர்வு (*s) {
	நிலை 'd':
	நிலை 'o':
	நிலை 'u':
	நிலை 'x': {
		நிலையான மாறிலி எழுத்து CSIL[] ALIGN1 = "CSIL";

		c = *s++;
		p = strchr(CSIL, *s);
		/* if *s == NUL, p != NULL! Testcase: "od -tx" */
		எனில் (!p || *p == '\0') {
			size = அளவு(முழு);
			எனில் (isdigit(s[0])) {
				size = bb_strtou(s, &end, 0);
				எனில் (errno == ERANGE
				 || MAX_INTEGRAL_TYPE_SIZE < size
				 || integral_type_size[size] == NO_SIZE
				) {
					bb_error_செய்தி_and_die("invalid type string '%s'; "
						"%u-byte %s type is not supported",
						s_orig, size, "integral");
				}
				s = end;
			}
		} இல்லையெனில் {
			நிலையான மாறிலி uint8_t CSIL_sizeof[4] = {
				அளவு(எழுத்து),
				அளவு(short),
				அளவு(முழு),
				அளவு(long),
			};
			size = CSIL_sizeof[p - CSIL];
			s++; /* skip C/S/I/L */
		}

#define ISPEC_TO_FORMAT(Spec, Min_format, Long_format, Max_format) \
	((Spec) == LONG_LONG ? (Max_format) \
	: ((Spec) == LONG ? (Long_format) : (Min_format)))

#define FMT_BYTES_ALLOCATED 9
		size_spec = integral_type_size[size];

		{
			நிலையான மாறிலி எழுத்து doux[] ALIGN1 = "doux";
			நிலையான மாறிலி எழுத்து doux_fmt_letter[][4] = {
				"lld", "llo", "llu", "llx"
			};
			நிலையான மாறிலி எண்ணிக்கை output_format doux_fmt[] = {
				SIGNED_DECIMAL,
				OCTAL,
				UNSIGNED_DECIMAL,
				HEXADECIMAL,
			};
			நிலையான மாறிலி uint8_t *மாறிலி doux_bytes_to_XXX[] = {
				bytes_to_signed_dec_digits,
				bytes_to_oct_digits,
				bytes_to_unsigned_dec_digits,
				bytes_to_hex_digits,
			};
			நிலையான மாறிலி எழுத்து doux_fmtstring[][அளவு(" %%0%u%s")] ALIGN1 = {
				" %%%u%s",
				" %%0%u%s",
				" %%%u%s",
				" %%0%u%s",
			};

			pos = strchr(doux, c) - doux;
			fmt = doux_fmt[pos];
			field_width = doux_bytes_to_XXX[pos][size];
			p = doux_fmt_letter[pos] + 2;
			எனில் (size_spec == LONG) p--;
			எனில் (size_spec == LONG_LONG) p -= 2;
			fmt_string = xasprintf(doux_fmtstring[pos], field_width, p);
		}

		தேர்வு (size_spec) {
		நிலை CHAR:
			print_function = (fmt == SIGNED_DECIMAL
				    ? print_s_char
				    : print_char);
			நிறுத்து;
		நிலை SHORT:
			print_function = (fmt == SIGNED_DECIMAL
				    ? print_s_short
				    : print_short);
			நிறுத்து;
		நிலை INT:
			print_function = print_int;
			நிறுத்து;
		நிலை LONG:
			print_function = print_long;
			நிறுத்து;
		இயல்பு: /* case LONG_LONG: */
			print_function = print_long_long;
			நிறுத்து;
		}
		நிறுத்து;
	}

	நிலை 'f': {
		நிலையான மாறிலி எழுத்து FDL[] ALIGN1 = "FDL";

		fmt = FLOATING_POINT;
		++s;
		p = strchr(FDL, *s);
		எனில் (!p || *p == '\0') {
			size = அளவு(இரட்டை);
			எனில் (isdigit(s[0])) {
				size = bb_strtou(s, &end, 0);
				எனில் (errno == ERANGE || size > MAX_FP_TYPE_SIZE
				 || fp_type_size[size] == NO_SIZE
				) {
					bb_error_செய்தி_and_die("invalid type string '%s'; "
						"%u-byte %s type is not supported",
						s_orig, size, "floating point");
				}
				s = end;
			}
		} இல்லையெனில் {
			நிலையான மாறிலி uint8_t FDL_sizeof[] = {
				அளவு(தசம),
				அளவு(இரட்டை),
				அளவு(longdouble_t),
			};

			size = FDL_sizeof[p - FDL];
			s++; /* skip F/D/L */
		}

		size_spec = fp_type_size[size];

		தேர்வு (size_spec) {
		நிலை FLOAT_SINGLE:
			print_function = print_float;
			field_width = FLT_DIG + 8;
			/* Don't use %#e; not all systems support it.  */
			fmt_string = xasprintf(" %%%d.%de", field_width, FLT_DIG);
			நிறுத்து;
		நிலை FLOAT_DOUBLE:
			print_function = print_double;
			field_width = DBL_DIG + 8;
			fmt_string = xasprintf(" %%%d.%de", field_width, DBL_DIG);
			நிறுத்து;
		இயல்பு: /* case FLOAT_LONG_DOUBLE: */
			print_function = print_long_double;
			field_width = LDBL_DIG + 8;
			fmt_string = xasprintf(" %%%d.%dLe", field_width, LDBL_DIG);
			நிறுத்து;
		}
		நிறுத்து;
	}

	நிலை 'a':
		++s;
		fmt = NAMED_CHARACTER;
		size_spec = CHAR;
		print_function = print_named_ascii;
		field_width = 3;
		நிறுத்து;
	நிலை 'c':
		++s;
		fmt = CHARACTER;
		size_spec = CHAR;
		print_function = print_ascii;
		field_width = 3;
		நிறுத்து;
	இயல்பு:
		bb_error_செய்தி_and_die("invalid character '%c' "
				"in type string '%s'", *s, s_orig);
	}

	tspec->size = size_spec;
	tspec->fmt = fmt;
	tspec->print_function = print_function;
	tspec->fmt_string = fmt_string;

	tspec->field_width = field_width;
	tspec->hexl_mode_trailer = (*s == 'z');
	எனில் (tspec->hexl_mode_trailer)
		s++;

	திரும்பு s;
}

/* Decode the modern od format string S.  Append the decoded
   representation to the global array SPEC, reallocating SPEC if
   necessary.  */

நிலையான வெறுமை
decode_format_string(மாறிலி எழுத்து *s)
{
	மாறிலி எழுத்து *s_orig = s;

	போது (*s != '\0') {
		கட்டமைப்பு tspec tspec;
		மாறிலி எழுத்து *next;

		next = decode_one_format(s_orig, s, &tspec);

		ப்ரதிஜ்ஞா(s != next);
		s = next;
		G.spec = xrealloc_vector(G.spec, 4, G.n_specs);
		memcpy(&G.spec[G.n_specs], &tspec, அளவு(G.spec[0]));
		G.n_specs++;
	}
}

/* Given a list of one or more input filenames FILE_LIST, set the global
   file pointer IN_STREAM to position N_SKIP in the concatenation of
   those files.  If any file operation fails or if there are fewer than
   N_SKIP bytes in the combined input, give an error message and return
   nonzero.  When possible, use seek rather than read operations to
   advance IN_STREAM.  */

நிலையான வெறுமை
skip(off_t n_skip)
{
	எனில் (n_skip == 0)
		திரும்பு;

	போது (G.in_stream) { /* !EOF */
		கட்டமைப்பு stat file_stats;

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
		எனில் (fstat(fileno(G.in_stream), &file_stats) == 0
		 && S_ISREG(file_stats.st_mode) && file_stats.st_size > 0
		) {
			எனில் (file_stats.st_size < n_skip) {
				n_skip -= file_stats.st_size;
				/* take "check & close / open_next" route */
			} இல்லையெனில் {
				எனில் (fseeko(G.in_stream, n_skip, SEEK_CUR) != 0)
					G.exit_code = 1;
				திரும்பு;
			}
		} இல்லையெனில் {
			/* If it's not a regular file with positive size,
			   position the file pointer by reading.  */
			எழுத்து buf[1024];
			size_t n_bytes_to_read = 1024;
			size_t n_bytes_read;

			போது (n_skip > 0) {
				எனில் (n_skip < n_bytes_to_read)
					n_bytes_to_read = n_skip;
				n_bytes_read = fread(buf, 1, n_bytes_to_read, G.in_stream);
				n_skip -= n_bytes_read;
				எனில் (n_bytes_read != n_bytes_to_read)
					நிறுத்து; /* EOF on this file or error */
			}
		}
		எனில் (n_skip == 0)
			திரும்பு;

		check_and_close();
		open_next_file();
	}

	எனில் (n_skip)
		bb_simple_error_செய்தி_and_die("can't skip past end of combined input");
}


வகைவரை வெறுமை FN_format_address(off_t address, எழுத்து c);

நிலையான வெறுமை
format_address_none(off_t address UNUSED_PARAM, எழுத்து c UNUSED_PARAM)
{
}

நிலையான வெறுமை
format_address_std(off_t address, எழுத்து c)
{
	/* Corresponds to 'c' */
	G.address_fmt[அளவு(G.address_fmt)-2] = c;
	printf(G.address_fmt, address);
}

#எனில் ENABLE_LONG_OPTS
/* only used with --traditional */
நிலையான வெறுமை
format_address_paren(off_t address, எழுத்து c)
{
	putchar('(');
	format_address_std(address, ')');
	எனில் (c) putchar(c);
}

நிலையான வெறுமை
format_address_label(off_t address, எழுத்து c)
{
	format_address_std(address, ' ');
	format_address_paren(address + G_pseudo_offset, c);
}
#endif

நிலையான வெறுமை
dump_hexl_mode_trailer(size_t n_bytes, மாறிலி எழுத்து *block)
{
	fputs_stdout("  >");
	போது (n_bytes--) {
		unsigned c = *(unsigned எழுத்து *) block++;
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

நிலையான வெறுமை
write_block(off_t current_offset, size_t n_bytes,
		மாறிலி எழுத்து *prev_block, மாறிலி எழுத்து *curr_block)
{
	unsigned i;

	எனில் (!(option_mask32 & OPT_v)
	 && G.not_first
	 && n_bytes == G.bytes_per_block
	 && memcmp(prev_block, curr_block, G.bytes_per_block) == 0
	) {
		எனில் (G.prev_pair_equal) {
			/* The two preceding blocks were equal, and the current
			   block is the same as the last one, so print nothing.  */
		} இல்லையெனில் {
			puts("*");
			G.prev_pair_equal = 1;
		}
	} இல்லையெனில் {
		G.not_first = 1;
		G.prev_pair_equal = 0;
		க்கு (i = 0; i < G.n_specs; i++) {
			எனில் (i == 0)
				G.format_address(current_offset, '\0');
			இல்லையெனில்
				printf("%*s", address_pad_len_char - '0', "");
			(*G.spec[i].print_function) (n_bytes, curr_block, G.spec[i].fmt_string);
			எனில் (G.spec[i].hexl_mode_trailer) {
				/* space-pad out to full line width, then dump the trailer */
				unsigned datum_width = width_bytes[G.spec[i].size];
				unsigned blank_fields = (G.bytes_per_block - n_bytes) / datum_width;
				unsigned field_width = G.spec[i].field_width + 1;
				printf("%*s", blank_fields * field_width, "");
				dump_hexl_mode_trailer(n_bytes, curr_block);
			}
			putchar('\n');
		}
	}
}

நிலையான வெறுமை
read_block(size_t n, எழுத்து *block, size_t *n_bytes_in_buffer)
{
	ப்ரதிஜ்ஞா(0 < n && n <= G.bytes_per_block);

	*n_bytes_in_buffer = 0;

	எனில் (n == 0)
		திரும்பு;

	போது (G.in_stream != வெற்று) { /* EOF.  */
		size_t n_needed;
		size_t n_read;

		n_needed = n - *n_bytes_in_buffer;
		n_read = fread(block + *n_bytes_in_buffer, 1, n_needed, G.in_stream);
		*n_bytes_in_buffer += n_read;
		எனில் (n_read == n_needed)
			நிறுத்து;
		/* error check is done in check_and_close */
		check_and_close();
		open_next_file();
	}
}

/* Return the least common multiple of the sizes associated
   with the format specs.  */

நிலையான முழு
get_lcm(வெறுமை)
{
	size_t i;
	முழு l_c_m = 1;

	க்கு (i = 0; i < G.n_specs; i++)
		l_c_m = lcm(l_c_m, width_bytes[(முழு) G.spec[i].size]);
	திரும்பு l_c_m;
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

நிலையான வெறுமை
dump(off_t current_offset, off_t end_offset)
{
	எழுத்து *block[2];
	முழு idx;
	size_t n_bytes_read;

	block[0] = xmalloc(2 * G.bytes_per_block);
	block[1] = block[0] + G.bytes_per_block;

	idx = 0;
	எனில் (option_mask32 & OPT_N) {
		போது (1) {
			size_t n_needed;
			எனில் (current_offset >= end_offset) {
				n_bytes_read = 0;
				நிறுத்து;
			}
			n_needed = MIN(end_offset - current_offset, (off_t) G.bytes_per_block);
			read_block(n_needed, block[idx], &n_bytes_read);
			எனில் (n_bytes_read < G.bytes_per_block)
				நிறுத்து;
			ப்ரதிஜ்ஞா(n_bytes_read == G.bytes_per_block);
			write_block(current_offset, n_bytes_read, block[idx ^ 1], block[idx]);
			current_offset += n_bytes_read;
			idx ^= 1;
		}
	} இல்லையெனில் {
		போது (1) {
			read_block(G.bytes_per_block, block[idx], &n_bytes_read);
			எனில் (n_bytes_read < G.bytes_per_block)
				நிறுத்து;
			ப்ரதிஜ்ஞா(n_bytes_read == G.bytes_per_block);
			write_block(current_offset, n_bytes_read, block[idx ^ 1], block[idx]);
			current_offset += n_bytes_read;
			idx ^= 1;
		}
	}

	எனில் (n_bytes_read > 0) {
		முழு l_c_m;
		size_t bytes_to_write;

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

	எனில் ((option_mask32 & OPT_N) && current_offset >= end_offset)
		check_and_close();

	விடுவி(block[0]);
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

நிலையான வெறுமை
dump_strings(off_t address, off_t end_offset)
{
	unsigned bufsize = MAX(100, G.string_min);
	unsigned எழுத்து *buf = xmalloc(bufsize);

	போது (1) {
		size_t i;
		முழு c;

		/* See if the next 'G.string_min' chars are all printing chars.  */
 tryline:
		எனில் ((option_mask32 & OPT_N) && (end_offset - G.string_min <= address))
			நிறுத்து;
		i = 0;
		போது (!(option_mask32 & OPT_N) || address < end_offset) {
			எனில் (i == bufsize) {
				bufsize += bufsize/8;
				buf = xrealloc(buf, bufsize);
			}

			போது (G.in_stream) { /* !EOF */
				c = fgetc(G.in_stream);
				எனில் (c != EOF)
					செல் got_char;
				check_and_close();
				open_next_file();
			}
			/* EOF */
			செல் ret;
 got_char:
			address++;
			எனில் (!c)
				நிறுத்து;
			எனில் (!ISPRINT(c))
				செல் tryline;	/* It isn't; give up on this string.  */
			buf[i++] = c;		/* String continues; store it all.  */
		}

		எனில் (i < G.string_min)		/* Too short! */
			செல் tryline;

		/* If we get here, the string is all printable and NUL-terminated */
		buf[i] = 0;
		G.format_address(address - i - 1, ' ');

		க்கு (i = 0; (c = buf[i]); i++) {
			தேர்வு (c) {
			நிலை '\007': fputs_stdout("\\a"); நிறுத்து;
			நிலை '\b': fputs_stdout("\\b"); நிறுத்து;
			நிலை '\f': fputs_stdout("\\f"); நிறுத்து;
			நிலை '\n': fputs_stdout("\\n"); நிறுத்து;
			நிலை '\r': fputs_stdout("\\r"); நிறுத்து;
			நிலை '\t': fputs_stdout("\\t"); நிறுத்து;
			நிலை '\v': fputs_stdout("\\v"); நிறுத்து;
			இயல்பு: putchar(c);
			}
		}
		putchar('\n');
	}

	/* We reach this point only if we search through
	   (max_bytes_to_format - G.string_min) bytes before reaching EOF.  */
	check_and_close();
 ret:
	விடுவி(buf);
}

#எனில் ENABLE_LONG_OPTS
/* If S is a valid traditional offset specification with an optional
   leading '+' return nonzero and set *OFFSET to the offset it denotes.  */

நிலையான முழு
parse_old_offset(மாறிலி எழுத்து *s, off_t *offset)
{
	நிலையான மாறிலி கட்டமைப்பு suffix_mult Bb[] ALIGN_SUFFIX = {
		{ "B", 1024 },
		{ "b", 512 },
		{ "", 0 }
	};
	எழுத்து *p;
	முழு radix;

	/* Skip over any leading '+'. */
	எனில் (s[0] == '+') ++s;
	எனில் (!isdigit(s[0])) திரும்பு 0; /* not a number */

	/* Determine the radix we'll use to interpret S.  If there is a '.',
	 * it's decimal, otherwise, if the string begins with '0X'or '0x',
	 * it's hexadecimal, else octal.  */
	p = strchr(s, '.');
	radix = 8;
	எனில் (p) {
		p[0] = '\0'; /* cheating */
		radix = 10;
	} இல்லையெனில் எனில் (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
		radix = 16;

	*offset = xstrtooff_sfx(s, radix, Bb);
	எனில் (p) p[0] = '.';

	திரும்பு (*offset >= 0);
}
#endif

முழு od_main(முழு argc, எழுத்து **argv) MAIN_EXTERNALLY_VISIBLE;
முழு od_main(முழு argc UNUSED_PARAM, எழுத்து **argv)
{
#எனில் ENABLE_LONG_OPTS
	நிலையான மாறிலி எழுத்து od_longopts[] ALIGN1 =
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
	மாறிலி எழுத்து *str_A, *str_N, *str_j, *str_S = "3";
	llist_t *lst_t = வெற்று;
	unsigned opt;
	முழு l_c_m;
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
	எனில் (opt & OPT_A) {
		நிலையான மாறிலி எழுத்து doxn[] ALIGN1 = "doxn";
		நிலையான மாறிலி எழுத்து doxn_address_base_char[] ALIGN1 = {
			'u', 'o', 'x', /* '?' fourth one is not important */
		};
		நிலையான மாறிலி uint8_t doxn_address_pad_len_char[] ALIGN1 = {
			'7', '7', '6', /* '?' */
		};
		எழுத்து *p;
		முழு pos;
		p = strchr(doxn, str_A[0]);
		எனில் (!p)
			bb_error_செய்தி_and_die("bad output address radix "
				"'%c' (must be [doxn])", str_A[0]);
		pos = p - doxn;
		எனில் (pos == 3) G.format_address = format_address_none;
		address_base_char = doxn_address_base_char[pos];
		address_pad_len_char = doxn_address_pad_len_char[pos];
	}
	எனில் (opt & OPT_N) {
		max_bytes_to_format = xstrtooff_sfx(str_N, 0, bkm_suffixes);
	}

	எனில் (opt & OPT_a) decode_format_string("a");
	எனில் (opt & OPT_b) decode_format_string("oC");
	எனில் (opt & OPT_c) decode_format_string("c");
	எனில் (opt & OPT_d) decode_format_string("u2");
	எனில் (opt & OPT_D) decode_format_string("uI");
	எனில் (opt & OPT_f) decode_format_string("fF");
	எனில் (opt & (OPT_h|OPT_x)) decode_format_string("x2");
	எனில் (opt & (OPT_H|OPT_X)) decode_format_string("xI");
	/* -I,L,l: depend on word width of the arch (what is "long"?) */
#எனில் ULONG_MAX > 0xffffffff
	எனில் (opt & OPT_i) decode_format_string("dI");
	எனில் (opt & (OPT_I|OPT_l|OPT_L)) decode_format_string("dL");
#இல்லையெனில்
	/* 32-bit arch: -I,L,l are the same as -i */
	எனில் (opt & (OPT_i|OPT_I|OPT_l|OPT_L)) decode_format_string("dI");
#endif
	எனில் (opt & OPT_j) n_bytes_to_skip = xstrtooff_sfx(str_j, 0, bkm_suffixes);
	எனில் (opt & (OPT_o|OPT_B)) decode_format_string("o2");
	எனில் (opt & OPT_O) decode_format_string("oI");
	போது (lst_t) {
		decode_format_string(llist_எடு(&lst_t));
	}
	எனில் (opt & OPT_s) decode_format_string("d2");
	எனில் (opt & OPT_S) {
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

#எனில் ENABLE_LONG_OPTS
	எனில் (opt & OPT_traditional) {
		எனில் (argv[0]) {
			off_t pseudo_start = -1;
			off_t o1, o2;

			எனில் (!argv[1]) { /* one arg */
				எனில் (parse_old_offset(argv[0], &o1)) {
					/* od --traditional OFFSET */
					n_bytes_to_skip = o1;
					argv++;
				}
				/* od --traditional FILE */
			} இல்லையெனில் எனில் (!argv[2]) { /* two args */
				எனில் (parse_old_offset(argv[0], &o1)
				 && parse_old_offset(argv[1], &o2)
				) {
					/* od --traditional OFFSET LABEL */
					n_bytes_to_skip = o1;
					pseudo_start = o2;
					argv += 2;
				} இல்லையெனில் எனில் (parse_old_offset(argv[1], &o2)) {
					/* od --traditional FILE OFFSET */
					n_bytes_to_skip = o2;
					argv[1] = வெற்று;
				} இல்லையெனில் {
					bb_error_செய்தி_and_die("invalid second argument '%s'", argv[1]);
				}
			} இல்லையெனில் எனில் (!argv[3]) { /* three args */
				எனில் (parse_old_offset(argv[1], &o1)
				 && parse_old_offset(argv[2], &o2)
				) {
					/* od --traditional FILE OFFSET LABEL */
					n_bytes_to_skip = o1;
					pseudo_start = o2;
					argv[1] = வெற்று;
				} இல்லையெனில் {
					bb_simple_error_செய்தி_and_die("the last two arguments must be offsets");
				}
			} இல்லையெனில் { /* >3 args */
				bb_simple_error_செய்தி_and_die("too many arguments");
			}

			எனில் (pseudo_start >= 0) {
				எனில் (G.format_address == format_address_none) {
					address_base_char = 'o';
					address_pad_len_char = '7';
					G.format_address = format_address_paren;
				} இல்லையெனில் {
					G.format_address = format_address_label;
				}
				G_pseudo_offset = pseudo_start - n_bytes_to_skip;
			}
		}
		/* else: od --traditional (without args) */
	}
#endif

	எனில் (option_mask32 & OPT_N) {
		end_offset = n_bytes_to_skip + max_bytes_to_format;
		எனில் (end_offset < n_bytes_to_skip)
			bb_simple_error_செய்தி_and_die("SKIP + SIZE is too large");
	}

	எனில் (G.n_specs == 0) {
		decode_format_string("o2");
		/*G.n_specs = 1; - done by decode_format_string */
	}

	/* If no files were listed on the command line,
	   set the global pointer FILE_LIST so that it
	   references the null-terminated list of one name: "-".  */
	G.file_list = bb_argv_dash;
	எனில் (argv[0]) {
		/* Set the global pointer FILE_LIST so that it
		   references the first file-argument on the command-line.  */
		G.file_list = (எழுத்து மாறிலி *மாறிலி *) argv;
	}

	/* Open the first input file */
	open_next_file();
	/* Skip over any unwanted header bytes */
	skip(n_bytes_to_skip);
	எனில் (!G.in_stream)
		திரும்பு EXIT_FAILURE;

	/* Compute output block length */
	l_c_m = get_lcm();

	எனில் (opt & OPT_w) { /* -w: width */
		எனில் (!G.bytes_per_block || G.bytes_per_block % l_c_m != 0) {
			bb_error_செய்தி("warning: invalid width %u; using %d instead",
					(unsigned)G.bytes_per_block, l_c_m);
			G.bytes_per_block = l_c_m;
		}
	} இல்லையெனில் {
		G.bytes_per_block = l_c_m;
		எனில் (l_c_m < DEFAULT_BYTES_PER_BLOCK)
			G.bytes_per_block *= DEFAULT_BYTES_PER_BLOCK / l_c_m;
	}

#ifdef DEBUG
	{
		முழு i;
		க்கு (i = 0; i < G.n_specs; i++) {
			printf("%d: fmt='%s' width=%d\n",
				i, G.spec[i].fmt_string,
				width_bytes[G.spec[i].size]);
		}
	}
#endif

	எனில் (option_mask32 & OPT_S)
		dump_strings(n_bytes_to_skip, end_offset);
	இல்லையெனில்
		dump(n_bytes_to_skip, end_offset);

	எனில் (fclose(stdin))
		bb_simple_perror_செய்தி_and_die(bb_செய்தி_standard_உள்ளீடு);

	திரும்பு G.exit_code;
}
