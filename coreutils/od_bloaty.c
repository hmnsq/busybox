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
#include "common_bufsiz.h"
#define assert(a) ((void)0)


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

enum {
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

typedef long double longdouble_t;
typedef unsigned long long ulonglong_t;
typedef long long llong;

#если ENABLE_LFS
# define xstrtooff_sfx xstrtoull_sfx
#иначе
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

enum size_spec {
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

enum output_format {
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
struct tspec {
	enum output_format fmt;
	enum size_spec size;
	void (*print_function) (size_t, const char *, const char *);
	char *fmt_string;
	целое hexl_mode_trailer;
	целое field_width;
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

static const uint8_t bytes_to_oct_digits[] ALIGN1 =
{0, 3, 6, 8, 11, 14, 16, 19, 22, 25, 27, 30, 32, 35, 38, 41, 43};

static const uint8_t bytes_to_signed_dec_digits[] ALIGN1 =
{1, 4, 6, 8, 11, 13, 16, 18, 20, 23, 25, 28, 30, 33, 35, 37, 40};

static const uint8_t bytes_to_unsigned_dec_digits[] ALIGN1 =
{0, 3, 5, 8, 10, 13, 15, 17, 20, 22, 25, 27, 29, 32, 34, 37, 39};

static const uint8_t bytes_to_hex_digits[] ALIGN1 =
{0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32};

/* Convert enum size_spec to the size of the named type.  */
static const signed char width_bytes[] ALIGN1 = {
	-1,
	sizeof(char),
	sizeof(short),
	sizeof(целое),
	sizeof(long),
	sizeof(ulonglong_t),
	sizeof(float),
	sizeof(double),
	sizeof(longdouble_t)
};
/* Ensure that for each member of 'enum size_spec' there is an
   initializer in the width_bytes array.  */
struct ERR_width_bytes_has_bad_size {
	char ERR_width_bytes_has_bad_size[ARRAY_SIZE(width_bytes) == N_SIZE_SPECS ? 1 : -1];
};

struct globals {
	smallint exit_code;

	unsigned string_min;

	/* An array of specs describing how to format each input block.  */
	unsigned n_specs;
	struct tspec *spec;

	/* Function that accepts an address and an optional following char,
	   and prints the address and char to stdout.  */
	void (*format_address)(off_t, char);

	/* The difference between the old-style pseudo starting address and
	   the number of bytes to skip.  */
#если ENABLE_LONG_OPTS
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
	const char *const *file_list;

	/* The input stream associated with the current file.  */
	FILE *in_stream;

	bool not_first;
	bool prev_pair_equal;

	char address_fmt[sizeof("%0n"OFF_FMT"xc")];
} FIX_ALIASING;
/* Corresponds to 'x' above */
#define address_base_char G.address_fmt[sizeof(G.address_fmt)-3]
/* Corresponds to 'n' above */
#define address_pad_len_char G.address_fmt[2]

#если !ENABLE_LONG_OPTS
enum { G_pseudo_offset = 0 };
#endif
#define G (*(struct globals*)bb_common_bufsiz1)
#define INIT_G() do { \
	setup_common_bufsiz(); \
	BUILD_BUG_ON(sizeof(G) > COMMON_BUFSIZE); \
	G.bytes_per_block = 32; \
	strcpy(G.address_fmt, "%0n"OFF_FMT"xc"); \
} пока (0)


#define MAX_INTEGRAL_TYPE_SIZE sizeof(ulonglong_t)
static const unsigned char integral_type_size[MAX_INTEGRAL_TYPE_SIZE + 1] ALIGN1 = {
	[sizeof(char)] = CHAR,
#если USHRT_MAX != UCHAR_MAX
	[sizeof(short)] = SHORT,
#endif
#если UINT_MAX != USHRT_MAX
	[sizeof(целое)] = INT,
#endif
#если ULONG_MAX != UINT_MAX
	[sizeof(long)] = LONG,
#endif
#если ULLONG_MAX != ULONG_MAX
	[sizeof(ulonglong_t)] = LONG_LONG,
#endif
};

#define MAX_FP_TYPE_SIZE sizeof(longdouble_t)
static const unsigned char fp_type_size[MAX_FP_TYPE_SIZE + 1] ALIGN1 = {
	/* gcc seems to allow repeated indexes. Last one wins */
	[sizeof(longdouble_t)] = FLOAT_LONG_DOUBLE,
	[sizeof(double)] = FLOAT_DOUBLE,
	[sizeof(float)] = FLOAT_SINGLE
};


static unsigned
gcd(unsigned u, unsigned v)
{
	unsigned t;
	пока (v != 0) {
		t = u % v;
		u = v;
		v = t;
	}
	вернуть u;
}

/* Compute the least common multiple of U and V.  */
static unsigned
lcm(unsigned u, unsigned v) {
	unsigned t = gcd(u, v);
	если (t == 0)
		вернуть 0;
	вернуть u * v / t;
}

static void
print_s_char(size_t n_bytes, const char *block, const char *fmt_string)
{
	пока (n_bytes--) {
		целое tmp = *(signed char *) block;
		printf(fmt_string, tmp);
		block += sizeof(unsigned char);
	}
}

static void
print_char(size_t n_bytes, const char *block, const char *fmt_string)
{
	пока (n_bytes--) {
		unsigned tmp = *(unsigned char *) block;
		printf(fmt_string, tmp);
		block += sizeof(unsigned char);
	}
}

static void
print_s_short(size_t n_bytes, const char *block, const char *fmt_string)
{
	n_bytes /= sizeof(signed short);
	пока (n_bytes--) {
		целое tmp = *(signed short *) block;
		printf(fmt_string, tmp);
		block += sizeof(unsigned short);
	}
}

static void
print_short(size_t n_bytes, const char *block, const char *fmt_string)
{
	n_bytes /= sizeof(unsigned short);
	пока (n_bytes--) {
		unsigned tmp = *(unsigned short *) block;
		printf(fmt_string, tmp);
		block += sizeof(unsigned short);
	}
}

static void
print_int(size_t n_bytes, const char *block, const char *fmt_string)
{
	n_bytes /= sizeof(unsigned);
	пока (n_bytes--) {
		unsigned tmp = *(unsigned *) block;
		printf(fmt_string, tmp);
		block += sizeof(unsigned);
	}
}

#если UINT_MAX == ULONG_MAX
# define print_long print_int
#иначе
static void
print_long(size_t n_bytes, const char *block, const char *fmt_string)
{
	n_bytes /= sizeof(unsigned long);
	пока (n_bytes--) {
		unsigned long tmp = *(unsigned long *) block;
		printf(fmt_string, tmp);
		block += sizeof(unsigned long);
	}
}
#endif

#если ULONG_MAX == ULLONG_MAX
# define print_long_long print_long
#иначе
static void
print_long_long(size_t n_bytes, const char *block, const char *fmt_string)
{
	n_bytes /= sizeof(ulonglong_t);
	пока (n_bytes--) {
		ulonglong_t tmp = *(ulonglong_t *) block;
		printf(fmt_string, tmp);
		block += sizeof(ulonglong_t);
	}
}
#endif

static void
print_float(size_t n_bytes, const char *block, const char *fmt_string)
{
	n_bytes /= sizeof(float);
	пока (n_bytes--) {
		float tmp = *(float *) block;
		printf(fmt_string, tmp);
		block += sizeof(float);
	}
}

static void
print_double(size_t n_bytes, const char *block, const char *fmt_string)
{
	n_bytes /= sizeof(double);
	пока (n_bytes--) {
		double tmp = *(double *) block;
		printf(fmt_string, tmp);
		block += sizeof(double);
	}
}

static void
print_long_double(size_t n_bytes, const char *block, const char *fmt_string)
{
	n_bytes /= sizeof(longdouble_t);
	пока (n_bytes--) {
		longdouble_t tmp = *(longdouble_t *) block;
		printf(fmt_string, tmp);
		block += sizeof(longdouble_t);
	}
}

/* print_[named]_ascii are optimized for speed.
 * Remember, someday you may want to pump gigabytes through this thing.
 * Saving a dozen of .text bytes here is counter-productive */

static void
print_named_ascii(size_t n_bytes, const char *block,
		const char *unused_fmt_string UNUSED_PARAM)
{
	/* Names for some non-printing characters.  */
	static const char charname[33][3] ALIGN1 = {
		"nul", "soh", "stx", "etx", "eot", "enq", "ack", "bel",
		" bs", " ht", " nl", " vt", " ff", " cr", " so", " si",
		"dle", "dc1", "dc2", "dc3", "dc4", "nak", "syn", "etb",
		"can", " em", "sub", "esc", " fs", " gs", " rs", " us",
		" sp"
	};
	// buf[N] pos:  01234 56789
	char buf[12] = "   x\0 xxx\0";
	// [12] because we take three 32bit stack slots anyway, and
	// gcc is too dumb to initialize with constant stores,
	// it copies initializer from rodata. Oh well.
	// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=65410

	пока (n_bytes--) {
		unsigned masked_c = *(unsigned char *) block++;

		masked_c &= 0x7f;
		если (masked_c == 0x7f) {
			fputs_stdout(" del");
			continue;
		}
		если (masked_c > ' ') {
			buf[3] = masked_c;
			fputs_stdout(buf);
			continue;
		}
		/* Why? Because printf(" %3.3s") is much slower... */
		buf[6] = charname[masked_c][0];
		buf[7] = charname[masked_c][1];
		buf[8] = charname[masked_c][2];
		fputs_stdout(buf+5);
	}
}

static void
print_ascii(size_t n_bytes, const char *block,
		const char *unused_fmt_string UNUSED_PARAM)
{
	// buf[N] pos:  01234 56789
	char buf[12] = "   x\0 xxx\0";

	пока (n_bytes--) {
		const char *s;
		unsigned c = *(unsigned char *) block++;

		если (ISPRINT(c)) {
			buf[3] = c;
			fputs_stdout(buf);
			continue;
		}
		switch (c) {
		case '\0':
			s = "  \\0";
			break;
		case '\007':
			s = "  \\a";
			break;
		case '\b':
			s = "  \\b";
			break;
		case '\f':
			s = "  \\f";
			break;
		case '\n':
			s = "  \\n";
			break;
		case '\r':
			s = "  \\r";
			break;
		case '\t':
			s = "  \\t";
			break;
		case '\v':
			s = "  \\v";
			break;
		default:
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

static void
open_next_file(void)
{
	пока (1) {
		если (!*G.file_list)
			вернуть;
		G.in_stream = fopen_or_warn_stdin(*G.file_list++);
		если (G.in_stream) {
			break;
		}
		G.exit_code = 1;
	}

	если ((option_mask32 & (OPT_N|OPT_S)) == OPT_N)
		setbuf(G.in_stream, NULL);
}

/* Test whether there have been errors on in_stream, and close it if
   it is not standard input.  Return nonzero if there has been an error
   on in_stream or stdout; return zero otherwise.  This function will
   report more than one error only if both a read and a write error
   have occurred.  IN_ERRNO, if nonzero, is the error number
   corresponding to the most recent action for IN_STREAM.  */

static void
check_and_close(void)
{
	если (G.in_stream) {
		если (ferror(G.in_stream))	{
			bb_error_msg("%s: read error", (G.in_stream == stdin)
					? bb_msg_standard_input
					: G.file_list[-1]
			);
			G.exit_code = 1;
		}
		fclose_if_not_stdin(G.in_stream);
		G.in_stream = NULL;
	}

	если (ferror(stdout)) {
		bb_simple_error_msg_and_die(bb_msg_write_error);
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

static NOINLINE const char *
decode_one_format(const char *s_orig, const char *s, struct tspec *tspec)
{
	enum size_spec size_spec;
	unsigned size;
	enum output_format fmt;
	const char *p;
	char *end;
	char *fmt_string = NULL;
	void (*print_function) (size_t, const char *, const char *);
	unsigned c;
	unsigned field_width = 0;
	целое pos;

	switch (*s) {
	case 'd':
	case 'o':
	case 'u':
	case 'x': {
		static const char CSIL[] ALIGN1 = "CSIL";

		c = *s++;
		p = strchr(CSIL, *s);
		/* if *s == NUL, p != NULL! Testcase: "od -tx" */
		если (!p || *p == '\0') {
			size = sizeof(целое);
			если (isdigit(s[0])) {
				size = bb_strtou(s, &end, 0);
				если (errno == ERANGE
				 || MAX_INTEGRAL_TYPE_SIZE < size
				 || integral_type_size[size] == NO_SIZE
				) {
					bb_error_msg_and_die("invalid type string '%s'; "
						"%u-byte %s type is not supported",
						s_orig, size, "integral");
				}
				s = end;
			}
		} иначе {
			static const uint8_t CSIL_sizeof[4] = {
				sizeof(char),
				sizeof(short),
				sizeof(целое),
				sizeof(long),
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
			static const char doux[] ALIGN1 = "doux";
			static const char doux_fmt_letter[][4] = {
				"lld", "llo", "llu", "llx"
			};
			static const enum output_format doux_fmt[] = {
				SIGNED_DECIMAL,
				OCTAL,
				UNSIGNED_DECIMAL,
				HEXADECIMAL,
			};
			static const uint8_t *const doux_bytes_to_XXX[] = {
				bytes_to_signed_dec_digits,
				bytes_to_oct_digits,
				bytes_to_unsigned_dec_digits,
				bytes_to_hex_digits,
			};
			static const char doux_fmtstring[][sizeof(" %%0%u%s")] ALIGN1 = {
				" %%%u%s",
				" %%0%u%s",
				" %%%u%s",
				" %%0%u%s",
			};

			pos = strchr(doux, c) - doux;
			fmt = doux_fmt[pos];
			field_width = doux_bytes_to_XXX[pos][size];
			p = doux_fmt_letter[pos] + 2;
			если (size_spec == LONG) p--;
			если (size_spec == LONG_LONG) p -= 2;
			fmt_string = xasprintf(doux_fmtstring[pos], field_width, p);
		}

		switch (size_spec) {
		case CHAR:
			print_function = (fmt == SIGNED_DECIMAL
				    ? print_s_char
				    : print_char);
			break;
		case SHORT:
			print_function = (fmt == SIGNED_DECIMAL
				    ? print_s_short
				    : print_short);
			break;
		case INT:
			print_function = print_int;
			break;
		case LONG:
			print_function = print_long;
			break;
		default: /* case LONG_LONG: */
			print_function = print_long_long;
			break;
		}
		break;
	}

	case 'f': {
		static const char FDL[] ALIGN1 = "FDL";

		fmt = FLOATING_POINT;
		++s;
		p = strchr(FDL, *s);
		если (!p || *p == '\0') {
			size = sizeof(double);
			если (isdigit(s[0])) {
				size = bb_strtou(s, &end, 0);
				если (errno == ERANGE || size > MAX_FP_TYPE_SIZE
				 || fp_type_size[size] == NO_SIZE
				) {
					bb_error_msg_and_die("invalid type string '%s'; "
						"%u-byte %s type is not supported",
						s_orig, size, "floating point");
				}
				s = end;
			}
		} иначе {
			static const uint8_t FDL_sizeof[] = {
				sizeof(float),
				sizeof(double),
				sizeof(longdouble_t),
			};

			size = FDL_sizeof[p - FDL];
			s++; /* skip F/D/L */
		}

		size_spec = fp_type_size[size];

		switch (size_spec) {
		case FLOAT_SINGLE:
			print_function = print_float;
			field_width = FLT_DIG + 8;
			/* Don't use %#e; not all systems support it.  */
			fmt_string = xasprintf(" %%%d.%de", field_width, FLT_DIG);
			break;
		case FLOAT_DOUBLE:
			print_function = print_double;
			field_width = DBL_DIG + 8;
			fmt_string = xasprintf(" %%%d.%de", field_width, DBL_DIG);
			break;
		default: /* case FLOAT_LONG_DOUBLE: */
			print_function = print_long_double;
			field_width = LDBL_DIG + 8;
			fmt_string = xasprintf(" %%%d.%dLe", field_width, LDBL_DIG);
			break;
		}
		break;
	}

	case 'a':
		++s;
		fmt = NAMED_CHARACTER;
		size_spec = CHAR;
		print_function = print_named_ascii;
		field_width = 3;
		break;
	case 'c':
		++s;
		fmt = CHARACTER;
		size_spec = CHAR;
		print_function = print_ascii;
		field_width = 3;
		break;
	default:
		bb_error_msg_and_die("invalid character '%c' "
				"in type string '%s'", *s, s_orig);
	}

	tspec->size = size_spec;
	tspec->fmt = fmt;
	tspec->print_function = print_function;
	tspec->fmt_string = fmt_string;

	tspec->field_width = field_width;
	tspec->hexl_mode_trailer = (*s == 'z');
	если (tspec->hexl_mode_trailer)
		s++;

	вернуть s;
}

/* Decode the modern od format string S.  Append the decoded
   representation to the global array SPEC, reallocating SPEC if
   necessary.  */

static void
decode_format_string(const char *s)
{
	const char *s_orig = s;

	пока (*s != '\0') {
		struct tspec tspec;
		const char *next;

		next = decode_one_format(s_orig, s, &tspec);

		assert(s != next);
		s = next;
		G.spec = xrealloc_vector(G.spec, 4, G.n_specs);
		memcpy(&G.spec[G.n_specs], &tspec, sizeof(G.spec[0]));
		G.n_specs++;
	}
}

/* Given a list of one or more input filenames FILE_LIST, set the global
   file pointer IN_STREAM to position N_SKIP in the concatenation of
   those files.  If any file operation fails or if there are fewer than
   N_SKIP bytes in the combined input, give an error message and return
   nonzero.  When possible, use seek rather than read operations to
   advance IN_STREAM.  */

static void
skip(off_t n_skip)
{
	если (n_skip == 0)
		вернуть;

	пока (G.in_stream) { /* !EOF */
		struct stat file_stats;

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
		если (fstat(fileno(G.in_stream), &file_stats) == 0
		 && S_ISREG(file_stats.st_mode) && file_stats.st_size > 0
		) {
			если (file_stats.st_size < n_skip) {
				n_skip -= file_stats.st_size;
				/* take "check & close / open_next" route */
			} иначе {
				если (fseeko(G.in_stream, n_skip, SEEK_CUR) != 0)
					G.exit_code = 1;
				вернуть;
			}
		} иначе {
			/* If it's not a regular file with positive size,
			   position the file pointer by reading.  */
			char buf[1024];
			size_t n_bytes_to_read = 1024;
			size_t n_bytes_read;

			пока (n_skip > 0) {
				если (n_skip < n_bytes_to_read)
					n_bytes_to_read = n_skip;
				n_bytes_read = fread(buf, 1, n_bytes_to_read, G.in_stream);
				n_skip -= n_bytes_read;
				если (n_bytes_read != n_bytes_to_read)
					break; /* EOF on this file or error */
			}
		}
		если (n_skip == 0)
			вернуть;

		check_and_close();
		open_next_file();
	}

	если (n_skip)
		bb_simple_error_msg_and_die("can't skip past end of combined input");
}


typedef void FN_format_address(off_t address, char c);

static void
format_address_none(off_t address UNUSED_PARAM, char c UNUSED_PARAM)
{
}

static void
format_address_std(off_t address, char c)
{
	/* Corresponds to 'c' */
	G.address_fmt[sizeof(G.address_fmt)-2] = c;
	printf(G.address_fmt, address);
}

#если ENABLE_LONG_OPTS
/* only used with --traditional */
static void
format_address_paren(off_t address, char c)
{
	putchar('(');
	format_address_std(address, ')');
	если (c) putchar(c);
}

static void
format_address_label(off_t address, char c)
{
	format_address_std(address, ' ');
	format_address_paren(address + G_pseudo_offset, c);
}
#endif

static void
dump_hexl_mode_trailer(size_t n_bytes, const char *block)
{
	fputs_stdout("  >");
	пока (n_bytes--) {
		unsigned c = *(unsigned char *) block++;
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

static void
write_block(off_t current_offset, size_t n_bytes,
		const char *prev_block, const char *curr_block)
{
	unsigned i;

	если (!(option_mask32 & OPT_v)
	 && G.not_first
	 && n_bytes == G.bytes_per_block
	 && memcmp(prev_block, curr_block, G.bytes_per_block) == 0
	) {
		если (G.prev_pair_equal) {
			/* The two preceding blocks were equal, and the current
			   block is the same as the last one, so print nothing.  */
		} иначе {
			puts("*");
			G.prev_pair_equal = 1;
		}
	} иначе {
		G.not_first = 1;
		G.prev_pair_equal = 0;
		для (i = 0; i < G.n_specs; i++) {
			если (i == 0)
				G.format_address(current_offset, '\0');
			иначе
				printf("%*s", address_pad_len_char - '0', "");
			(*G.spec[i].print_function) (n_bytes, curr_block, G.spec[i].fmt_string);
			если (G.spec[i].hexl_mode_trailer) {
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

static void
read_block(size_t n, char *block, size_t *n_bytes_in_buffer)
{
	assert(0 < n && n <= G.bytes_per_block);

	*n_bytes_in_buffer = 0;

	если (n == 0)
		вернуть;

	пока (G.in_stream != NULL) { /* EOF.  */
		size_t n_needed;
		size_t n_read;

		n_needed = n - *n_bytes_in_buffer;
		n_read = fread(block + *n_bytes_in_buffer, 1, n_needed, G.in_stream);
		*n_bytes_in_buffer += n_read;
		если (n_read == n_needed)
			break;
		/* error check is done in check_and_close */
		check_and_close();
		open_next_file();
	}
}

/* Return the least common multiple of the sizes associated
   with the format specs.  */

static целое
get_lcm(void)
{
	size_t i;
	целое l_c_m = 1;

	для (i = 0; i < G.n_specs; i++)
		l_c_m = lcm(l_c_m, width_bytes[(целое) G.spec[i].size]);
	вернуть l_c_m;
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

static void
dump(off_t current_offset, off_t end_offset)
{
	char *block[2];
	целое idx;
	size_t n_bytes_read;

	block[0] = xmalloc(2 * G.bytes_per_block);
	block[1] = block[0] + G.bytes_per_block;

	idx = 0;
	если (option_mask32 & OPT_N) {
		пока (1) {
			size_t n_needed;
			если (current_offset >= end_offset) {
				n_bytes_read = 0;
				break;
			}
			n_needed = MIN(end_offset - current_offset, (off_t) G.bytes_per_block);
			read_block(n_needed, block[idx], &n_bytes_read);
			если (n_bytes_read < G.bytes_per_block)
				break;
			assert(n_bytes_read == G.bytes_per_block);
			write_block(current_offset, n_bytes_read, block[idx ^ 1], block[idx]);
			current_offset += n_bytes_read;
			idx ^= 1;
		}
	} иначе {
		пока (1) {
			read_block(G.bytes_per_block, block[idx], &n_bytes_read);
			если (n_bytes_read < G.bytes_per_block)
				break;
			assert(n_bytes_read == G.bytes_per_block);
			write_block(current_offset, n_bytes_read, block[idx ^ 1], block[idx]);
			current_offset += n_bytes_read;
			idx ^= 1;
		}
	}

	если (n_bytes_read > 0) {
		целое l_c_m;
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

	если ((option_mask32 & OPT_N) && current_offset >= end_offset)
		check_and_close();

	free(block[0]);
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

static void
dump_strings(off_t address, off_t end_offset)
{
	unsigned bufsize = MAX(100, G.string_min);
	unsigned char *buf = xmalloc(bufsize);

	пока (1) {
		size_t i;
		целое c;

		/* See if the next 'G.string_min' chars are all printing chars.  */
 tryline:
		если ((option_mask32 & OPT_N) && (end_offset - G.string_min <= address))
			break;
		i = 0;
		пока (!(option_mask32 & OPT_N) || address < end_offset) {
			если (i == bufsize) {
				bufsize += bufsize/8;
				buf = xrealloc(buf, bufsize);
			}

			пока (G.in_stream) { /* !EOF */
				c = fgetc(G.in_stream);
				если (c != EOF)
					goto got_char;
				check_and_close();
				open_next_file();
			}
			/* EOF */
			goto ret;
 got_char:
			address++;
			если (!c)
				break;
			если (!ISPRINT(c))
				goto tryline;	/* It isn't; give up on this string.  */
			buf[i++] = c;		/* String continues; store it all.  */
		}

		если (i < G.string_min)		/* Too short! */
			goto tryline;

		/* If we get here, the string is all printable and NUL-terminated */
		buf[i] = 0;
		G.format_address(address - i - 1, ' ');

		для (i = 0; (c = buf[i]); i++) {
			switch (c) {
			case '\007': fputs_stdout("\\a"); break;
			case '\b': fputs_stdout("\\b"); break;
			case '\f': fputs_stdout("\\f"); break;
			case '\n': fputs_stdout("\\n"); break;
			case '\r': fputs_stdout("\\r"); break;
			case '\t': fputs_stdout("\\t"); break;
			case '\v': fputs_stdout("\\v"); break;
			default: putchar(c);
			}
		}
		putchar('\n');
	}

	/* We reach this point only if we search through
	   (max_bytes_to_format - G.string_min) bytes before reaching EOF.  */
	check_and_close();
 ret:
	free(buf);
}

#если ENABLE_LONG_OPTS
/* If S is a valid traditional offset specification with an optional
   leading '+' return nonzero and set *OFFSET to the offset it denotes.  */

static целое
parse_old_offset(const char *s, off_t *offset)
{
	static const struct suffix_mult Bb[] ALIGN_SUFFIX = {
		{ "B", 1024 },
		{ "b", 512 },
		{ "", 0 }
	};
	char *p;
	целое radix;

	/* Skip over any leading '+'. */
	если (s[0] == '+') ++s;
	если (!isdigit(s[0])) вернуть 0; /* not a number */

	/* Determine the radix we'll use to interpret S.  If there is a '.',
	 * it's decimal, otherwise, if the string begins with '0X'or '0x',
	 * it's hexadecimal, else octal.  */
	p = strchr(s, '.');
	radix = 8;
	если (p) {
		p[0] = '\0'; /* cheating */
		radix = 10;
	} иначе если (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
		radix = 16;

	*offset = xstrtooff_sfx(s, radix, Bb);
	если (p) p[0] = '.';

	вернуть (*offset >= 0);
}
#endif

целое od_main(целое argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
целое od_main(целое argc UNUSED_PARAM, char **argv)
{
#если ENABLE_LONG_OPTS
	static const char od_longopts[] ALIGN1 =
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
	const char *str_A, *str_N, *str_j, *str_S = "3";
	llist_t *lst_t = NULL;
	unsigned opt;
	целое l_c_m;
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
	если (opt & OPT_A) {
		static const char doxn[] ALIGN1 = "doxn";
		static const char doxn_address_base_char[] ALIGN1 = {
			'u', 'o', 'x', /* '?' fourth one is not important */
		};
		static const uint8_t doxn_address_pad_len_char[] ALIGN1 = {
			'7', '7', '6', /* '?' */
		};
		char *p;
		целое pos;
		p = strchr(doxn, str_A[0]);
		если (!p)
			bb_error_msg_and_die("bad output address radix "
				"'%c' (must be [doxn])", str_A[0]);
		pos = p - doxn;
		если (pos == 3) G.format_address = format_address_none;
		address_base_char = doxn_address_base_char[pos];
		address_pad_len_char = doxn_address_pad_len_char[pos];
	}
	если (opt & OPT_N) {
		max_bytes_to_format = xstrtooff_sfx(str_N, 0, bkm_suffixes);
	}

	если (opt & OPT_a) decode_format_string("a");
	если (opt & OPT_b) decode_format_string("oC");
	если (opt & OPT_c) decode_format_string("c");
	если (opt & OPT_d) decode_format_string("u2");
	если (opt & OPT_D) decode_format_string("uI");
	если (opt & OPT_f) decode_format_string("fF");
	если (opt & (OPT_h|OPT_x)) decode_format_string("x2");
	если (opt & (OPT_H|OPT_X)) decode_format_string("xI");
	/* -I,L,l: depend on word width of the arch (what is "long"?) */
#если ULONG_MAX > 0xffffffff
	если (opt & OPT_i) decode_format_string("dI");
	если (opt & (OPT_I|OPT_l|OPT_L)) decode_format_string("dL");
#иначе
	/* 32-bit arch: -I,L,l are the same as -i */
	если (opt & (OPT_i|OPT_I|OPT_l|OPT_L)) decode_format_string("dI");
#endif
	если (opt & OPT_j) n_bytes_to_skip = xstrtooff_sfx(str_j, 0, bkm_suffixes);
	если (opt & (OPT_o|OPT_B)) decode_format_string("o2");
	если (opt & OPT_O) decode_format_string("oI");
	пока (lst_t) {
		decode_format_string(llist_pop(&lst_t));
	}
	если (opt & OPT_s) decode_format_string("d2");
	если (opt & OPT_S) {
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

#если ENABLE_LONG_OPTS
	если (opt & OPT_traditional) {
		если (argv[0]) {
			off_t pseudo_start = -1;
			off_t o1, o2;

			если (!argv[1]) { /* one arg */
				если (parse_old_offset(argv[0], &o1)) {
					/* od --traditional OFFSET */
					n_bytes_to_skip = o1;
					argv++;
				}
				/* od --traditional FILE */
			} иначе если (!argv[2]) { /* two args */
				если (parse_old_offset(argv[0], &o1)
				 && parse_old_offset(argv[1], &o2)
				) {
					/* od --traditional OFFSET LABEL */
					n_bytes_to_skip = o1;
					pseudo_start = o2;
					argv += 2;
				} иначе если (parse_old_offset(argv[1], &o2)) {
					/* od --traditional FILE OFFSET */
					n_bytes_to_skip = o2;
					argv[1] = NULL;
				} иначе {
					bb_error_msg_and_die("invalid second argument '%s'", argv[1]);
				}
			} иначе если (!argv[3]) { /* three args */
				если (parse_old_offset(argv[1], &o1)
				 && parse_old_offset(argv[2], &o2)
				) {
					/* od --traditional FILE OFFSET LABEL */
					n_bytes_to_skip = o1;
					pseudo_start = o2;
					argv[1] = NULL;
				} иначе {
					bb_simple_error_msg_and_die("the last two arguments must be offsets");
				}
			} иначе { /* >3 args */
				bb_simple_error_msg_and_die("too many arguments");
			}

			если (pseudo_start >= 0) {
				если (G.format_address == format_address_none) {
					address_base_char = 'o';
					address_pad_len_char = '7';
					G.format_address = format_address_paren;
				} иначе {
					G.format_address = format_address_label;
				}
				G_pseudo_offset = pseudo_start - n_bytes_to_skip;
			}
		}
		/* else: od --traditional (without args) */
	}
#endif

	если (option_mask32 & OPT_N) {
		end_offset = n_bytes_to_skip + max_bytes_to_format;
		если (end_offset < n_bytes_to_skip)
			bb_simple_error_msg_and_die("SKIP + SIZE is too large");
	}

	если (G.n_specs == 0) {
		decode_format_string("o2");
		/*G.n_specs = 1; - done by decode_format_string */
	}

	/* If no files were listed on the command line,
	   set the global pointer FILE_LIST so that it
	   references the null-terminated list of one name: "-".  */
	G.file_list = bb_argv_dash;
	если (argv[0]) {
		/* Set the global pointer FILE_LIST so that it
		   references the first file-argument on the command-line.  */
		G.file_list = (char const *const *) argv;
	}

	/* Open the first input file */
	open_next_file();
	/* Skip over any unwanted header bytes */
	skip(n_bytes_to_skip);
	если (!G.in_stream)
		вернуть EXIT_FAILURE;

	/* Compute output block length */
	l_c_m = get_lcm();

	если (opt & OPT_w) { /* -w: width */
		если (!G.bytes_per_block || G.bytes_per_block % l_c_m != 0) {
			bb_error_msg("warning: invalid width %u; using %d instead",
					(unsigned)G.bytes_per_block, l_c_m);
			G.bytes_per_block = l_c_m;
		}
	} иначе {
		G.bytes_per_block = l_c_m;
		если (l_c_m < DEFAULT_BYTES_PER_BLOCK)
			G.bytes_per_block *= DEFAULT_BYTES_PER_BLOCK / l_c_m;
	}

#ifdef DEBUG
	{
		целое i;
		для (i = 0; i < G.n_specs; i++) {
			printf("%d: fmt='%s' width=%d\n",
				i, G.spec[i].fmt_string,
				width_bytes[G.spec[i].size]);
		}
	}
#endif

	если (option_mask32 & OPT_S)
		dump_strings(n_bytes_to_skip, end_offset);
	иначе
		dump(n_bytes_to_skip, end_offset);

	если (fclose(stdin))
		bb_simple_perror_msg_and_die(bb_msg_standard_input);

	вернуть G.exit_code;
}
