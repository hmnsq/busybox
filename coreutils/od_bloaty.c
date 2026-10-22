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
#അന്തര്ഭാവ "common_bufsiz.h"
#define പ്രതിജ്ഞാ(a) ((ശൂന്യം)0)


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

എണ്ണൽ {
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

തരനിർവചനം long ഇരട്ട longdouble_t;
തരനിർവചനം unsigned long long ulonglong_t;
തരനിർവചനം long long llong;

#എങ്കിൽ ENABLE_LFS
# define xstrtooff_sfx xstrtoull_sfx
#അല്ലെങ്കിൽ
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

എണ്ണൽ size_spec {
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

എണ്ണൽ output_format {
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
ഘടന tspec {
	എണ്ണൽ output_format fmt;
	എണ്ണൽ size_spec size;
	ശൂന്യം (*print_function) (size_t, സ്ഥിരം അക്ഷരം *, സ്ഥിരം അക്ഷരം *);
	അക്ഷരം *fmt_string;
	പൂർണ്ണസംഖ്യ hexl_mode_trailer;
	പൂർണ്ണസംഖ്യ field_width;
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

സ്ഥിതം സ്ഥിരം uint8_t bytes_to_oct_digits[] ALIGN1 =
{0, 3, 6, 8, 11, 14, 16, 19, 22, 25, 27, 30, 32, 35, 38, 41, 43};

സ്ഥിതം സ്ഥിരം uint8_t bytes_to_signed_dec_digits[] ALIGN1 =
{1, 4, 6, 8, 11, 13, 16, 18, 20, 23, 25, 28, 30, 33, 35, 37, 40};

സ്ഥിതം സ്ഥിരം uint8_t bytes_to_unsigned_dec_digits[] ALIGN1 =
{0, 3, 5, 8, 10, 13, 15, 17, 20, 22, 25, 27, 29, 32, 34, 37, 39};

സ്ഥിതം സ്ഥിരം uint8_t bytes_to_hex_digits[] ALIGN1 =
{0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32};

/* Convert enum size_spec to the size of the named type.  */
സ്ഥിതം സ്ഥിരം signed അക്ഷരം width_bytes[] ALIGN1 = {
	-1,
	വലുപ്പം(അക്ഷരം),
	വലുപ്പം(short),
	വലുപ്പം(പൂർണ്ണസംഖ്യ),
	വലുപ്പം(long),
	വലുപ്പം(ulonglong_t),
	വലുപ്പം(ദശാംശം),
	വലുപ്പം(ഇരട്ട),
	വലുപ്പം(longdouble_t)
};
/* Ensure that for each member of 'enum size_spec' there is an
   initializer in the width_bytes array.  */
ഘടന ERR_width_bytes_has_bad_size {
	അക്ഷരം ERR_width_bytes_has_bad_size[ARRAY_SIZE(width_bytes) == N_SIZE_SPECS ? 1 : -1];
};

ഘടന globals {
	smallint exit_code;

	unsigned string_min;

	/* An array of specs describing how to format each input block.  */
	unsigned n_specs;
	ഘടന tspec *spec;

	/* Function that accepts an address and an optional following char,
	   and prints the address and char to stdout.  */
	ശൂന്യം (*format_address)(off_t, അക്ഷരം);

	/* The difference between the old-style pseudo starting address and
	   the number of bytes to skip.  */
#എങ്കിൽ ENABLE_LONG_OPTS
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
	സ്ഥിരം അക്ഷരം *സ്ഥിരം *file_list;

	/* The input stream associated with the current file.  */
	FILE *in_stream;

	bool not_first;
	bool prev_pair_equal;

	അക്ഷരം address_fmt[വലുപ്പം("%0n"OFF_FMT"xc")];
} FIX_ALIASING;
/* Corresponds to 'x' above */
#define address_base_char G.address_fmt[വലുപ്പം(G.address_fmt)-3]
/* Corresponds to 'n' above */
#define address_pad_len_char G.address_fmt[2]

#എങ്കിൽ !ENABLE_LONG_OPTS
എണ്ണൽ { G_pseudo_offset = 0 };
#endif
#define G (*(ഘടന globals*)bb_common_bufsiz1)
#define INIT_G() ചെയ്യുക { \
	setup_common_bufsiz(); \
	BUILD_BUG_ON(വലുപ്പം(G) > COMMON_BUFSIZE); \
	G.bytes_per_block = 32; \
	strcpy(G.address_fmt, "%0n"OFF_FMT"xc"); \
} ആകുമ്പോൾ (0)


#define MAX_INTEGRAL_TYPE_SIZE വലുപ്പം(ulonglong_t)
സ്ഥിതം സ്ഥിരം unsigned അക്ഷരം integral_type_size[MAX_INTEGRAL_TYPE_SIZE + 1] ALIGN1 = {
	[വലുപ്പം(അക്ഷരം)] = CHAR,
#എങ്കിൽ USHRT_MAX != UCHAR_MAX
	[വലുപ്പം(short)] = SHORT,
#endif
#എങ്കിൽ UINT_MAX != USHRT_MAX
	[വലുപ്പം(പൂർണ്ണസംഖ്യ)] = INT,
#endif
#എങ്കിൽ ULONG_MAX != UINT_MAX
	[വലുപ്പം(long)] = LONG,
#endif
#എങ്കിൽ ULLONG_MAX != ULONG_MAX
	[വലുപ്പം(ulonglong_t)] = LONG_LONG,
#endif
};

#define MAX_FP_TYPE_SIZE വലുപ്പം(longdouble_t)
സ്ഥിതം സ്ഥിരം unsigned അക്ഷരം fp_type_size[MAX_FP_TYPE_SIZE + 1] ALIGN1 = {
	/* gcc seems to allow repeated indexes. Last one wins */
	[വലുപ്പം(longdouble_t)] = FLOAT_LONG_DOUBLE,
	[വലുപ്പം(ഇരട്ട)] = FLOAT_DOUBLE,
	[വലുപ്പം(ദശാംശം)] = FLOAT_SINGLE
};


സ്ഥിതം unsigned
gcd(unsigned u, unsigned v)
{
	unsigned t;
	ആകുമ്പോൾ (v != 0) {
		t = u % v;
		u = v;
		v = t;
	}
	തിരികെ u;
}

/* Compute the least common multiple of U and V.  */
സ്ഥിതം unsigned
lcm(unsigned u, unsigned v) {
	unsigned t = gcd(u, v);
	എങ്കിൽ (t == 0)
		തിരികെ 0;
	തിരികെ u * v / t;
}

സ്ഥിതം ശൂന്യം
print_s_char(size_t n_bytes, സ്ഥിരം അക്ഷരം *block, സ്ഥിരം അക്ഷരം *fmt_string)
{
	ആകുമ്പോൾ (n_bytes--) {
		പൂർണ്ണസംഖ്യ tmp = *(signed അക്ഷരം *) block;
		printf(fmt_string, tmp);
		block += വലുപ്പം(unsigned അക്ഷരം);
	}
}

സ്ഥിതം ശൂന്യം
print_char(size_t n_bytes, സ്ഥിരം അക്ഷരം *block, സ്ഥിരം അക്ഷരം *fmt_string)
{
	ആകുമ്പോൾ (n_bytes--) {
		unsigned tmp = *(unsigned അക്ഷരം *) block;
		printf(fmt_string, tmp);
		block += വലുപ്പം(unsigned അക്ഷരം);
	}
}

സ്ഥിതം ശൂന്യം
print_s_short(size_t n_bytes, സ്ഥിരം അക്ഷരം *block, സ്ഥിരം അക്ഷരം *fmt_string)
{
	n_bytes /= വലുപ്പം(signed short);
	ആകുമ്പോൾ (n_bytes--) {
		പൂർണ്ണസംഖ്യ tmp = *(signed short *) block;
		printf(fmt_string, tmp);
		block += വലുപ്പം(unsigned short);
	}
}

സ്ഥിതം ശൂന്യം
print_short(size_t n_bytes, സ്ഥിരം അക്ഷരം *block, സ്ഥിരം അക്ഷരം *fmt_string)
{
	n_bytes /= വലുപ്പം(unsigned short);
	ആകുമ്പോൾ (n_bytes--) {
		unsigned tmp = *(unsigned short *) block;
		printf(fmt_string, tmp);
		block += വലുപ്പം(unsigned short);
	}
}

സ്ഥിതം ശൂന്യം
print_int(size_t n_bytes, സ്ഥിരം അക്ഷരം *block, സ്ഥിരം അക്ഷരം *fmt_string)
{
	n_bytes /= വലുപ്പം(unsigned);
	ആകുമ്പോൾ (n_bytes--) {
		unsigned tmp = *(unsigned *) block;
		printf(fmt_string, tmp);
		block += വലുപ്പം(unsigned);
	}
}

#എങ്കിൽ UINT_MAX == ULONG_MAX
# define print_long print_int
#അല്ലെങ്കിൽ
സ്ഥിതം ശൂന്യം
print_long(size_t n_bytes, സ്ഥിരം അക്ഷരം *block, സ്ഥിരം അക്ഷരം *fmt_string)
{
	n_bytes /= വലുപ്പം(unsigned long);
	ആകുമ്പോൾ (n_bytes--) {
		unsigned long tmp = *(unsigned long *) block;
		printf(fmt_string, tmp);
		block += വലുപ്പം(unsigned long);
	}
}
#endif

#എങ്കിൽ ULONG_MAX == ULLONG_MAX
# define print_long_long print_long
#അല്ലെങ്കിൽ
സ്ഥിതം ശൂന്യം
print_long_long(size_t n_bytes, സ്ഥിരം അക്ഷരം *block, സ്ഥിരം അക്ഷരം *fmt_string)
{
	n_bytes /= വലുപ്പം(ulonglong_t);
	ആകുമ്പോൾ (n_bytes--) {
		ulonglong_t tmp = *(ulonglong_t *) block;
		printf(fmt_string, tmp);
		block += വലുപ്പം(ulonglong_t);
	}
}
#endif

സ്ഥിതം ശൂന്യം
print_float(size_t n_bytes, സ്ഥിരം അക്ഷരം *block, സ്ഥിരം അക്ഷരം *fmt_string)
{
	n_bytes /= വലുപ്പം(ദശാംശം);
	ആകുമ്പോൾ (n_bytes--) {
		ദശാംശം tmp = *(ദശാംശം *) block;
		printf(fmt_string, tmp);
		block += വലുപ്പം(ദശാംശം);
	}
}

സ്ഥിതം ശൂന്യം
print_double(size_t n_bytes, സ്ഥിരം അക്ഷരം *block, സ്ഥിരം അക്ഷരം *fmt_string)
{
	n_bytes /= വലുപ്പം(ഇരട്ട);
	ആകുമ്പോൾ (n_bytes--) {
		ഇരട്ട tmp = *(ഇരട്ട *) block;
		printf(fmt_string, tmp);
		block += വലുപ്പം(ഇരട്ട);
	}
}

സ്ഥിതം ശൂന്യം
print_long_double(size_t n_bytes, സ്ഥിരം അക്ഷരം *block, സ്ഥിരം അക്ഷരം *fmt_string)
{
	n_bytes /= വലുപ്പം(longdouble_t);
	ആകുമ്പോൾ (n_bytes--) {
		longdouble_t tmp = *(longdouble_t *) block;
		printf(fmt_string, tmp);
		block += വലുപ്പം(longdouble_t);
	}
}

/* print_[named]_ascii are optimized for speed.
 * Remember, someday you may want to pump gigabytes through this thing.
 * Saving a dozen of .text bytes here is counter-productive */

സ്ഥിതം ശൂന്യം
print_named_ascii(size_t n_bytes, സ്ഥിരം അക്ഷരം *block,
		സ്ഥിരം അക്ഷരം *unused_fmt_string UNUSED_PARAM)
{
	/* Names for some non-printing characters.  */
	സ്ഥിതം സ്ഥിരം അക്ഷരം charname[33][3] ALIGN1 = {
		"nul", "soh", "stx", "etx", "eot", "enq", "ack", "bel",
		" bs", " ht", " nl", " vt", " ff", " cr", " so", " si",
		"dle", "dc1", "dc2", "dc3", "dc4", "nak", "syn", "etb",
		"can", " em", "sub", "esc", " fs", " gs", " rs", " us",
		" sp"
	};
	// buf[N] pos:  01234 56789
	അക്ഷരം buf[12] = "   x\0 xxx\0";
	// [12] because we take three 32bit stack slots anyway, and
	// gcc is too dumb to initialize with constant stores,
	// it copies initializer from rodata. Oh well.
	// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=65410

	ആകുമ്പോൾ (n_bytes--) {
		unsigned masked_c = *(unsigned അക്ഷരം *) block++;

		masked_c &= 0x7f;
		എങ്കിൽ (masked_c == 0x7f) {
			fputs_stdout(" del");
			തുടരുക;
		}
		എങ്കിൽ (masked_c > ' ') {
			buf[3] = masked_c;
			fputs_stdout(buf);
			തുടരുക;
		}
		/* Why? Because printf(" %3.3s") is much slower... */
		buf[6] = charname[masked_c][0];
		buf[7] = charname[masked_c][1];
		buf[8] = charname[masked_c][2];
		fputs_stdout(buf+5);
	}
}

സ്ഥിതം ശൂന്യം
print_ascii(size_t n_bytes, സ്ഥിരം അക്ഷരം *block,
		സ്ഥിരം അക്ഷരം *unused_fmt_string UNUSED_PARAM)
{
	// buf[N] pos:  01234 56789
	അക്ഷരം buf[12] = "   x\0 xxx\0";

	ആകുമ്പോൾ (n_bytes--) {
		സ്ഥിരം അക്ഷരം *s;
		unsigned c = *(unsigned അക്ഷരം *) block++;

		എങ്കിൽ (ISPRINT(c)) {
			buf[3] = c;
			fputs_stdout(buf);
			തുടരുക;
		}
		തിരഞ്ഞെടുപ്പ് (c) {
		അവസ്ഥ '\0':
			s = "  \\0";
			നിർത്തുക;
		അവസ്ഥ '\007':
			s = "  \\a";
			നിർത്തുക;
		അവസ്ഥ '\b':
			s = "  \\b";
			നിർത്തുക;
		അവസ്ഥ '\f':
			s = "  \\f";
			നിർത്തുക;
		അവസ്ഥ '\n':
			s = "  \\n";
			നിർത്തുക;
		അവസ്ഥ '\r':
			s = "  \\r";
			നിർത്തുക;
		അവസ്ഥ '\t':
			s = "  \\t";
			നിർത്തുക;
		അവസ്ഥ '\v':
			s = "  \\v";
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
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

സ്ഥിതം ശൂന്യം
open_next_file(ശൂന്യം)
{
	ആകുമ്പോൾ (1) {
		എങ്കിൽ (!*G.file_list)
			തിരികെ;
		G.in_stream = fopen_or_warn_stdin(*G.file_list++);
		എങ്കിൽ (G.in_stream) {
			നിർത്തുക;
		}
		G.exit_code = 1;
	}

	എങ്കിൽ ((option_mask32 & (OPT_N|OPT_S)) == OPT_N)
		setbuf(G.in_stream, ശൂന്യ);
}

/* Test whether there have been errors on in_stream, and close it if
   it is not standard input.  Return nonzero if there has been an error
   on in_stream or stdout; return zero otherwise.  This function will
   report more than one error only if both a read and a write error
   have occurred.  IN_ERRNO, if nonzero, is the error number
   corresponding to the most recent action for IN_STREAM.  */

സ്ഥിതം ശൂന്യം
check_and_close(ശൂന്യം)
{
	എങ്കിൽ (G.in_stream) {
		എങ്കിൽ (ferror(G.in_stream))	{
			bb_error_സന്ദേശം("%s: read error", (G.in_stream == stdin)
					? bb_സന്ദേശം_standard_നിവേശം
					: G.file_list[-1]
			);
			G.exit_code = 1;
		}
		fclose_if_not_stdin(G.in_stream);
		G.in_stream = ശൂന്യ;
	}

	എങ്കിൽ (ferror(stdout)) {
		bb_simple_error_സന്ദേശം_and_die(bb_സന്ദേശം_write_error);
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

സ്ഥിതം NOINLINE സ്ഥിരം അക്ഷരം *
decode_one_format(സ്ഥിരം അക്ഷരം *s_orig, സ്ഥിരം അക്ഷരം *s, ഘടന tspec *tspec)
{
	എണ്ണൽ size_spec size_spec;
	unsigned size;
	എണ്ണൽ output_format fmt;
	സ്ഥിരം അക്ഷരം *p;
	അക്ഷരം *end;
	അക്ഷരം *fmt_string = ശൂന്യ;
	ശൂന്യം (*print_function) (size_t, സ്ഥിരം അക്ഷരം *, സ്ഥിരം അക്ഷരം *);
	unsigned c;
	unsigned field_width = 0;
	പൂർണ്ണസംഖ്യ pos;

	തിരഞ്ഞെടുപ്പ് (*s) {
	അവസ്ഥ 'd':
	അവസ്ഥ 'o':
	അവസ്ഥ 'u':
	അവസ്ഥ 'x': {
		സ്ഥിതം സ്ഥിരം അക്ഷരം CSIL[] ALIGN1 = "CSIL";

		c = *s++;
		p = strchr(CSIL, *s);
		/* if *s == NUL, p != NULL! Testcase: "od -tx" */
		എങ്കിൽ (!p || *p == '\0') {
			size = വലുപ്പം(പൂർണ്ണസംഖ്യ);
			എങ്കിൽ (isdigit(s[0])) {
				size = bb_strtou(s, &end, 0);
				എങ്കിൽ (errno == ERANGE
				 || MAX_INTEGRAL_TYPE_SIZE < size
				 || integral_type_size[size] == NO_SIZE
				) {
					bb_error_സന്ദേശം_and_die("invalid type string '%s'; "
						"%u-byte %s type is not supported",
						s_orig, size, "integral");
				}
				s = end;
			}
		} അല്ലെങ്കിൽ {
			സ്ഥിതം സ്ഥിരം uint8_t CSIL_sizeof[4] = {
				വലുപ്പം(അക്ഷരം),
				വലുപ്പം(short),
				വലുപ്പം(പൂർണ്ണസംഖ്യ),
				വലുപ്പം(long),
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
			സ്ഥിതം സ്ഥിരം അക്ഷരം doux[] ALIGN1 = "doux";
			സ്ഥിതം സ്ഥിരം അക്ഷരം doux_fmt_letter[][4] = {
				"lld", "llo", "llu", "llx"
			};
			സ്ഥിതം സ്ഥിരം എണ്ണൽ output_format doux_fmt[] = {
				SIGNED_DECIMAL,
				OCTAL,
				UNSIGNED_DECIMAL,
				HEXADECIMAL,
			};
			സ്ഥിതം സ്ഥിരം uint8_t *സ്ഥിരം doux_bytes_to_XXX[] = {
				bytes_to_signed_dec_digits,
				bytes_to_oct_digits,
				bytes_to_unsigned_dec_digits,
				bytes_to_hex_digits,
			};
			സ്ഥിതം സ്ഥിരം അക്ഷരം doux_fmtstring[][വലുപ്പം(" %%0%u%s")] ALIGN1 = {
				" %%%u%s",
				" %%0%u%s",
				" %%%u%s",
				" %%0%u%s",
			};

			pos = strchr(doux, c) - doux;
			fmt = doux_fmt[pos];
			field_width = doux_bytes_to_XXX[pos][size];
			p = doux_fmt_letter[pos] + 2;
			എങ്കിൽ (size_spec == LONG) p--;
			എങ്കിൽ (size_spec == LONG_LONG) p -= 2;
			fmt_string = xasprintf(doux_fmtstring[pos], field_width, p);
		}

		തിരഞ്ഞെടുപ്പ് (size_spec) {
		അവസ്ഥ CHAR:
			print_function = (fmt == SIGNED_DECIMAL
				    ? print_s_char
				    : print_char);
			നിർത്തുക;
		അവസ്ഥ SHORT:
			print_function = (fmt == SIGNED_DECIMAL
				    ? print_s_short
				    : print_short);
			നിർത്തുക;
		അവസ്ഥ INT:
			print_function = print_int;
			നിർത്തുക;
		അവസ്ഥ LONG:
			print_function = print_long;
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി: /* case LONG_LONG: */
			print_function = print_long_long;
			നിർത്തുക;
		}
		നിർത്തുക;
	}

	അവസ്ഥ 'f': {
		സ്ഥിതം സ്ഥിരം അക്ഷരം FDL[] ALIGN1 = "FDL";

		fmt = FLOATING_POINT;
		++s;
		p = strchr(FDL, *s);
		എങ്കിൽ (!p || *p == '\0') {
			size = വലുപ്പം(ഇരട്ട);
			എങ്കിൽ (isdigit(s[0])) {
				size = bb_strtou(s, &end, 0);
				എങ്കിൽ (errno == ERANGE || size > MAX_FP_TYPE_SIZE
				 || fp_type_size[size] == NO_SIZE
				) {
					bb_error_സന്ദേശം_and_die("invalid type string '%s'; "
						"%u-byte %s type is not supported",
						s_orig, size, "floating point");
				}
				s = end;
			}
		} അല്ലെങ്കിൽ {
			സ്ഥിതം സ്ഥിരം uint8_t FDL_sizeof[] = {
				വലുപ്പം(ദശാംശം),
				വലുപ്പം(ഇരട്ട),
				വലുപ്പം(longdouble_t),
			};

			size = FDL_sizeof[p - FDL];
			s++; /* skip F/D/L */
		}

		size_spec = fp_type_size[size];

		തിരഞ്ഞെടുപ്പ് (size_spec) {
		അവസ്ഥ FLOAT_SINGLE:
			print_function = print_float;
			field_width = FLT_DIG + 8;
			/* Don't use %#e; not all systems support it.  */
			fmt_string = xasprintf(" %%%d.%de", field_width, FLT_DIG);
			നിർത്തുക;
		അവസ്ഥ FLOAT_DOUBLE:
			print_function = print_double;
			field_width = DBL_DIG + 8;
			fmt_string = xasprintf(" %%%d.%de", field_width, DBL_DIG);
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി: /* case FLOAT_LONG_DOUBLE: */
			print_function = print_long_double;
			field_width = LDBL_DIG + 8;
			fmt_string = xasprintf(" %%%d.%dLe", field_width, LDBL_DIG);
			നിർത്തുക;
		}
		നിർത്തുക;
	}

	അവസ്ഥ 'a':
		++s;
		fmt = NAMED_CHARACTER;
		size_spec = CHAR;
		print_function = print_named_ascii;
		field_width = 3;
		നിർത്തുക;
	അവസ്ഥ 'c':
		++s;
		fmt = CHARACTER;
		size_spec = CHAR;
		print_function = print_ascii;
		field_width = 3;
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		bb_error_സന്ദേശം_and_die("invalid character '%c' "
				"in type string '%s'", *s, s_orig);
	}

	tspec->size = size_spec;
	tspec->fmt = fmt;
	tspec->print_function = print_function;
	tspec->fmt_string = fmt_string;

	tspec->field_width = field_width;
	tspec->hexl_mode_trailer = (*s == 'z');
	എങ്കിൽ (tspec->hexl_mode_trailer)
		s++;

	തിരികെ s;
}

/* Decode the modern od format string S.  Append the decoded
   representation to the global array SPEC, reallocating SPEC if
   necessary.  */

സ്ഥിതം ശൂന്യം
decode_format_string(സ്ഥിരം അക്ഷരം *s)
{
	സ്ഥിരം അക്ഷരം *s_orig = s;

	ആകുമ്പോൾ (*s != '\0') {
		ഘടന tspec tspec;
		സ്ഥിരം അക്ഷരം *next;

		next = decode_one_format(s_orig, s, &tspec);

		പ്രതിജ്ഞാ(s != next);
		s = next;
		G.spec = xrealloc_vector(G.spec, 4, G.n_specs);
		memcpy(&G.spec[G.n_specs], &tspec, വലുപ്പം(G.spec[0]));
		G.n_specs++;
	}
}

/* Given a list of one or more input filenames FILE_LIST, set the global
   file pointer IN_STREAM to position N_SKIP in the concatenation of
   those files.  If any file operation fails or if there are fewer than
   N_SKIP bytes in the combined input, give an error message and return
   nonzero.  When possible, use seek rather than read operations to
   advance IN_STREAM.  */

സ്ഥിതം ശൂന്യം
skip(off_t n_skip)
{
	എങ്കിൽ (n_skip == 0)
		തിരികെ;

	ആകുമ്പോൾ (G.in_stream) { /* !EOF */
		ഘടന stat file_stats;

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
		എങ്കിൽ (fstat(fileno(G.in_stream), &file_stats) == 0
		 && S_ISREG(file_stats.st_mode) && file_stats.st_size > 0
		) {
			എങ്കിൽ (file_stats.st_size < n_skip) {
				n_skip -= file_stats.st_size;
				/* take "check & close / open_next" route */
			} അല്ലെങ്കിൽ {
				എങ്കിൽ (fseeko(G.in_stream, n_skip, SEEK_CUR) != 0)
					G.exit_code = 1;
				തിരികെ;
			}
		} അല്ലെങ്കിൽ {
			/* If it's not a regular file with positive size,
			   position the file pointer by reading.  */
			അക്ഷരം buf[1024];
			size_t n_bytes_to_read = 1024;
			size_t n_bytes_read;

			ആകുമ്പോൾ (n_skip > 0) {
				എങ്കിൽ (n_skip < n_bytes_to_read)
					n_bytes_to_read = n_skip;
				n_bytes_read = fread(buf, 1, n_bytes_to_read, G.in_stream);
				n_skip -= n_bytes_read;
				എങ്കിൽ (n_bytes_read != n_bytes_to_read)
					നിർത്തുക; /* EOF on this file or error */
			}
		}
		എങ്കിൽ (n_skip == 0)
			തിരികെ;

		check_and_close();
		open_next_file();
	}

	എങ്കിൽ (n_skip)
		bb_simple_error_സന്ദേശം_and_die("can't skip past end of combined input");
}


തരനിർവചനം ശൂന്യം FN_format_address(off_t address, അക്ഷരം c);

സ്ഥിതം ശൂന്യം
format_address_none(off_t address UNUSED_PARAM, അക്ഷരം c UNUSED_PARAM)
{
}

സ്ഥിതം ശൂന്യം
format_address_std(off_t address, അക്ഷരം c)
{
	/* Corresponds to 'c' */
	G.address_fmt[വലുപ്പം(G.address_fmt)-2] = c;
	printf(G.address_fmt, address);
}

#എങ്കിൽ ENABLE_LONG_OPTS
/* only used with --traditional */
സ്ഥിതം ശൂന്യം
format_address_paren(off_t address, അക്ഷരം c)
{
	putchar('(');
	format_address_std(address, ')');
	എങ്കിൽ (c) putchar(c);
}

സ്ഥിതം ശൂന്യം
format_address_label(off_t address, അക്ഷരം c)
{
	format_address_std(address, ' ');
	format_address_paren(address + G_pseudo_offset, c);
}
#endif

സ്ഥിതം ശൂന്യം
dump_hexl_mode_trailer(size_t n_bytes, സ്ഥിരം അക്ഷരം *block)
{
	fputs_stdout("  >");
	ആകുമ്പോൾ (n_bytes--) {
		unsigned c = *(unsigned അക്ഷരം *) block++;
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

സ്ഥിതം ശൂന്യം
write_block(off_t current_offset, size_t n_bytes,
		സ്ഥിരം അക്ഷരം *prev_block, സ്ഥിരം അക്ഷരം *curr_block)
{
	unsigned i;

	എങ്കിൽ (!(option_mask32 & OPT_v)
	 && G.not_first
	 && n_bytes == G.bytes_per_block
	 && memcmp(prev_block, curr_block, G.bytes_per_block) == 0
	) {
		എങ്കിൽ (G.prev_pair_equal) {
			/* The two preceding blocks were equal, and the current
			   block is the same as the last one, so print nothing.  */
		} അല്ലെങ്കിൽ {
			puts("*");
			G.prev_pair_equal = 1;
		}
	} അല്ലെങ്കിൽ {
		G.not_first = 1;
		G.prev_pair_equal = 0;
		വേണ്ടി (i = 0; i < G.n_specs; i++) {
			എങ്കിൽ (i == 0)
				G.format_address(current_offset, '\0');
			അല്ലെങ്കിൽ
				printf("%*s", address_pad_len_char - '0', "");
			(*G.spec[i].print_function) (n_bytes, curr_block, G.spec[i].fmt_string);
			എങ്കിൽ (G.spec[i].hexl_mode_trailer) {
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

സ്ഥിതം ശൂന്യം
read_block(size_t n, അക്ഷരം *block, size_t *n_bytes_in_buffer)
{
	പ്രതിജ്ഞാ(0 < n && n <= G.bytes_per_block);

	*n_bytes_in_buffer = 0;

	എങ്കിൽ (n == 0)
		തിരികെ;

	ആകുമ്പോൾ (G.in_stream != ശൂന്യ) { /* EOF.  */
		size_t n_needed;
		size_t n_read;

		n_needed = n - *n_bytes_in_buffer;
		n_read = fread(block + *n_bytes_in_buffer, 1, n_needed, G.in_stream);
		*n_bytes_in_buffer += n_read;
		എങ്കിൽ (n_read == n_needed)
			നിർത്തുക;
		/* error check is done in check_and_close */
		check_and_close();
		open_next_file();
	}
}

/* Return the least common multiple of the sizes associated
   with the format specs.  */

സ്ഥിതം പൂർണ്ണസംഖ്യ
get_lcm(ശൂന്യം)
{
	size_t i;
	പൂർണ്ണസംഖ്യ l_c_m = 1;

	വേണ്ടി (i = 0; i < G.n_specs; i++)
		l_c_m = lcm(l_c_m, width_bytes[(പൂർണ്ണസംഖ്യ) G.spec[i].size]);
	തിരികെ l_c_m;
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

സ്ഥിതം ശൂന്യം
dump(off_t current_offset, off_t end_offset)
{
	അക്ഷരം *block[2];
	പൂർണ്ണസംഖ്യ idx;
	size_t n_bytes_read;

	block[0] = xmalloc(2 * G.bytes_per_block);
	block[1] = block[0] + G.bytes_per_block;

	idx = 0;
	എങ്കിൽ (option_mask32 & OPT_N) {
		ആകുമ്പോൾ (1) {
			size_t n_needed;
			എങ്കിൽ (current_offset >= end_offset) {
				n_bytes_read = 0;
				നിർത്തുക;
			}
			n_needed = MIN(end_offset - current_offset, (off_t) G.bytes_per_block);
			read_block(n_needed, block[idx], &n_bytes_read);
			എങ്കിൽ (n_bytes_read < G.bytes_per_block)
				നിർത്തുക;
			പ്രതിജ്ഞാ(n_bytes_read == G.bytes_per_block);
			write_block(current_offset, n_bytes_read, block[idx ^ 1], block[idx]);
			current_offset += n_bytes_read;
			idx ^= 1;
		}
	} അല്ലെങ്കിൽ {
		ആകുമ്പോൾ (1) {
			read_block(G.bytes_per_block, block[idx], &n_bytes_read);
			എങ്കിൽ (n_bytes_read < G.bytes_per_block)
				നിർത്തുക;
			പ്രതിജ്ഞാ(n_bytes_read == G.bytes_per_block);
			write_block(current_offset, n_bytes_read, block[idx ^ 1], block[idx]);
			current_offset += n_bytes_read;
			idx ^= 1;
		}
	}

	എങ്കിൽ (n_bytes_read > 0) {
		പൂർണ്ണസംഖ്യ l_c_m;
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

	എങ്കിൽ ((option_mask32 & OPT_N) && current_offset >= end_offset)
		check_and_close();

	സ്വതന്ത്രം(block[0]);
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

സ്ഥിതം ശൂന്യം
dump_strings(off_t address, off_t end_offset)
{
	unsigned bufsize = MAX(100, G.string_min);
	unsigned അക്ഷരം *buf = xmalloc(bufsize);

	ആകുമ്പോൾ (1) {
		size_t i;
		പൂർണ്ണസംഖ്യ c;

		/* See if the next 'G.string_min' chars are all printing chars.  */
 tryline:
		എങ്കിൽ ((option_mask32 & OPT_N) && (end_offset - G.string_min <= address))
			നിർത്തുക;
		i = 0;
		ആകുമ്പോൾ (!(option_mask32 & OPT_N) || address < end_offset) {
			എങ്കിൽ (i == bufsize) {
				bufsize += bufsize/8;
				buf = xrealloc(buf, bufsize);
			}

			ആകുമ്പോൾ (G.in_stream) { /* !EOF */
				c = fgetc(G.in_stream);
				എങ്കിൽ (c != EOF)
					പോകുക got_char;
				check_and_close();
				open_next_file();
			}
			/* EOF */
			പോകുക ret;
 got_char:
			address++;
			എങ്കിൽ (!c)
				നിർത്തുക;
			എങ്കിൽ (!ISPRINT(c))
				പോകുക tryline;	/* It isn't; give up on this string.  */
			buf[i++] = c;		/* String continues; store it all.  */
		}

		എങ്കിൽ (i < G.string_min)		/* Too short! */
			പോകുക tryline;

		/* If we get here, the string is all printable and NUL-terminated */
		buf[i] = 0;
		G.format_address(address - i - 1, ' ');

		വേണ്ടി (i = 0; (c = buf[i]); i++) {
			തിരഞ്ഞെടുപ്പ് (c) {
			അവസ്ഥ '\007': fputs_stdout("\\a"); നിർത്തുക;
			അവസ്ഥ '\b': fputs_stdout("\\b"); നിർത്തുക;
			അവസ്ഥ '\f': fputs_stdout("\\f"); നിർത്തുക;
			അവസ്ഥ '\n': fputs_stdout("\\n"); നിർത്തുക;
			അവസ്ഥ '\r': fputs_stdout("\\r"); നിർത്തുക;
			അവസ്ഥ '\t': fputs_stdout("\\t"); നിർത്തുക;
			അവസ്ഥ '\v': fputs_stdout("\\v"); നിർത്തുക;
			സ്ഥിരസ്ഥിതി: putchar(c);
			}
		}
		putchar('\n');
	}

	/* We reach this point only if we search through
	   (max_bytes_to_format - G.string_min) bytes before reaching EOF.  */
	check_and_close();
 ret:
	സ്വതന്ത്രം(buf);
}

#എങ്കിൽ ENABLE_LONG_OPTS
/* If S is a valid traditional offset specification with an optional
   leading '+' return nonzero and set *OFFSET to the offset it denotes.  */

സ്ഥിതം പൂർണ്ണസംഖ്യ
parse_old_offset(സ്ഥിരം അക്ഷരം *s, off_t *offset)
{
	സ്ഥിതം സ്ഥിരം ഘടന suffix_mult Bb[] ALIGN_SUFFIX = {
		{ "B", 1024 },
		{ "b", 512 },
		{ "", 0 }
	};
	അക്ഷരം *p;
	പൂർണ്ണസംഖ്യ radix;

	/* Skip over any leading '+'. */
	എങ്കിൽ (s[0] == '+') ++s;
	എങ്കിൽ (!isdigit(s[0])) തിരികെ 0; /* not a number */

	/* Determine the radix we'll use to interpret S.  If there is a '.',
	 * it's decimal, otherwise, if the string begins with '0X'or '0x',
	 * it's hexadecimal, else octal.  */
	p = strchr(s, '.');
	radix = 8;
	എങ്കിൽ (p) {
		p[0] = '\0'; /* cheating */
		radix = 10;
	} അല്ലെങ്കിൽ എങ്കിൽ (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
		radix = 16;

	*offset = xstrtooff_sfx(s, radix, Bb);
	എങ്കിൽ (p) p[0] = '.';

	തിരികെ (*offset >= 0);
}
#endif

പൂർണ്ണസംഖ്യ od_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) MAIN_EXTERNALLY_VISIBLE;
പൂർണ്ണസംഖ്യ od_main(പൂർണ്ണസംഖ്യ argc UNUSED_PARAM, അക്ഷരം **argv)
{
#എങ്കിൽ ENABLE_LONG_OPTS
	സ്ഥിതം സ്ഥിരം അക്ഷരം od_longopts[] ALIGN1 =
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
	സ്ഥിരം അക്ഷരം *str_A, *str_N, *str_j, *str_S = "3";
	llist_t *lst_t = ശൂന്യ;
	unsigned opt;
	പൂർണ്ണസംഖ്യ l_c_m;
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
	എങ്കിൽ (opt & OPT_A) {
		സ്ഥിതം സ്ഥിരം അക്ഷരം doxn[] ALIGN1 = "doxn";
		സ്ഥിതം സ്ഥിരം അക്ഷരം doxn_address_base_char[] ALIGN1 = {
			'u', 'o', 'x', /* '?' fourth one is not important */
		};
		സ്ഥിതം സ്ഥിരം uint8_t doxn_address_pad_len_char[] ALIGN1 = {
			'7', '7', '6', /* '?' */
		};
		അക്ഷരം *p;
		പൂർണ്ണസംഖ്യ pos;
		p = strchr(doxn, str_A[0]);
		എങ്കിൽ (!p)
			bb_error_സന്ദേശം_and_die("bad output address radix "
				"'%c' (must be [doxn])", str_A[0]);
		pos = p - doxn;
		എങ്കിൽ (pos == 3) G.format_address = format_address_none;
		address_base_char = doxn_address_base_char[pos];
		address_pad_len_char = doxn_address_pad_len_char[pos];
	}
	എങ്കിൽ (opt & OPT_N) {
		max_bytes_to_format = xstrtooff_sfx(str_N, 0, bkm_suffixes);
	}

	എങ്കിൽ (opt & OPT_a) decode_format_string("a");
	എങ്കിൽ (opt & OPT_b) decode_format_string("oC");
	എങ്കിൽ (opt & OPT_c) decode_format_string("c");
	എങ്കിൽ (opt & OPT_d) decode_format_string("u2");
	എങ്കിൽ (opt & OPT_D) decode_format_string("uI");
	എങ്കിൽ (opt & OPT_f) decode_format_string("fF");
	എങ്കിൽ (opt & (OPT_h|OPT_x)) decode_format_string("x2");
	എങ്കിൽ (opt & (OPT_H|OPT_X)) decode_format_string("xI");
	/* -I,L,l: depend on word width of the arch (what is "long"?) */
#എങ്കിൽ ULONG_MAX > 0xffffffff
	എങ്കിൽ (opt & OPT_i) decode_format_string("dI");
	എങ്കിൽ (opt & (OPT_I|OPT_l|OPT_L)) decode_format_string("dL");
#അല്ലെങ്കിൽ
	/* 32-bit arch: -I,L,l are the same as -i */
	എങ്കിൽ (opt & (OPT_i|OPT_I|OPT_l|OPT_L)) decode_format_string("dI");
#endif
	എങ്കിൽ (opt & OPT_j) n_bytes_to_skip = xstrtooff_sfx(str_j, 0, bkm_suffixes);
	എങ്കിൽ (opt & (OPT_o|OPT_B)) decode_format_string("o2");
	എങ്കിൽ (opt & OPT_O) decode_format_string("oI");
	ആകുമ്പോൾ (lst_t) {
		decode_format_string(llist_എടുക്കുക(&lst_t));
	}
	എങ്കിൽ (opt & OPT_s) decode_format_string("d2");
	എങ്കിൽ (opt & OPT_S) {
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

#എങ്കിൽ ENABLE_LONG_OPTS
	എങ്കിൽ (opt & OPT_traditional) {
		എങ്കിൽ (argv[0]) {
			off_t pseudo_start = -1;
			off_t o1, o2;

			എങ്കിൽ (!argv[1]) { /* one arg */
				എങ്കിൽ (parse_old_offset(argv[0], &o1)) {
					/* od --traditional OFFSET */
					n_bytes_to_skip = o1;
					argv++;
				}
				/* od --traditional FILE */
			} അല്ലെങ്കിൽ എങ്കിൽ (!argv[2]) { /* two args */
				എങ്കിൽ (parse_old_offset(argv[0], &o1)
				 && parse_old_offset(argv[1], &o2)
				) {
					/* od --traditional OFFSET LABEL */
					n_bytes_to_skip = o1;
					pseudo_start = o2;
					argv += 2;
				} അല്ലെങ്കിൽ എങ്കിൽ (parse_old_offset(argv[1], &o2)) {
					/* od --traditional FILE OFFSET */
					n_bytes_to_skip = o2;
					argv[1] = ശൂന്യ;
				} അല്ലെങ്കിൽ {
					bb_error_സന്ദേശം_and_die("invalid second argument '%s'", argv[1]);
				}
			} അല്ലെങ്കിൽ എങ്കിൽ (!argv[3]) { /* three args */
				എങ്കിൽ (parse_old_offset(argv[1], &o1)
				 && parse_old_offset(argv[2], &o2)
				) {
					/* od --traditional FILE OFFSET LABEL */
					n_bytes_to_skip = o1;
					pseudo_start = o2;
					argv[1] = ശൂന്യ;
				} അല്ലെങ്കിൽ {
					bb_simple_error_സന്ദേശം_and_die("the last two arguments must be offsets");
				}
			} അല്ലെങ്കിൽ { /* >3 args */
				bb_simple_error_സന്ദേശം_and_die("too many arguments");
			}

			എങ്കിൽ (pseudo_start >= 0) {
				എങ്കിൽ (G.format_address == format_address_none) {
					address_base_char = 'o';
					address_pad_len_char = '7';
					G.format_address = format_address_paren;
				} അല്ലെങ്കിൽ {
					G.format_address = format_address_label;
				}
				G_pseudo_offset = pseudo_start - n_bytes_to_skip;
			}
		}
		/* else: od --traditional (without args) */
	}
#endif

	എങ്കിൽ (option_mask32 & OPT_N) {
		end_offset = n_bytes_to_skip + max_bytes_to_format;
		എങ്കിൽ (end_offset < n_bytes_to_skip)
			bb_simple_error_സന്ദേശം_and_die("SKIP + SIZE is too large");
	}

	എങ്കിൽ (G.n_specs == 0) {
		decode_format_string("o2");
		/*G.n_specs = 1; - done by decode_format_string */
	}

	/* If no files were listed on the command line,
	   set the global pointer FILE_LIST so that it
	   references the null-terminated list of one name: "-".  */
	G.file_list = bb_argv_dash;
	എങ്കിൽ (argv[0]) {
		/* Set the global pointer FILE_LIST so that it
		   references the first file-argument on the command-line.  */
		G.file_list = (അക്ഷരം സ്ഥിരം *സ്ഥിരം *) argv;
	}

	/* Open the first input file */
	open_next_file();
	/* Skip over any unwanted header bytes */
	skip(n_bytes_to_skip);
	എങ്കിൽ (!G.in_stream)
		തിരികെ EXIT_FAILURE;

	/* Compute output block length */
	l_c_m = get_lcm();

	എങ്കിൽ (opt & OPT_w) { /* -w: width */
		എങ്കിൽ (!G.bytes_per_block || G.bytes_per_block % l_c_m != 0) {
			bb_error_സന്ദേശം("warning: invalid width %u; using %d instead",
					(unsigned)G.bytes_per_block, l_c_m);
			G.bytes_per_block = l_c_m;
		}
	} അല്ലെങ്കിൽ {
		G.bytes_per_block = l_c_m;
		എങ്കിൽ (l_c_m < DEFAULT_BYTES_PER_BLOCK)
			G.bytes_per_block *= DEFAULT_BYTES_PER_BLOCK / l_c_m;
	}

#ifdef DEBUG
	{
		പൂർണ്ണസംഖ്യ i;
		വേണ്ടി (i = 0; i < G.n_specs; i++) {
			printf("%d: fmt='%s' width=%d\n",
				i, G.spec[i].fmt_string,
				width_bytes[G.spec[i].size]);
		}
	}
#endif

	എങ്കിൽ (option_mask32 & OPT_S)
		dump_strings(n_bytes_to_skip, end_offset);
	അല്ലെങ്കിൽ
		dump(n_bytes_to_skip, end_offset);

	എങ്കിൽ (fclose(stdin))
		bb_simple_perror_സന്ദേശം_and_die(bb_സന്ദേശം_standard_നിവേശം);

	തിരികെ G.exit_code;
}
