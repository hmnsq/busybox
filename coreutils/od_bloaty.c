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
#अन्तर्भाव "common_bufsiz.h"
#परिभाषय प्रतिज्ञा(a) ((शून्य)0)


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

गणना {
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

#परिभाषय OD_GETOPT32() getopt32long(argv, \
	"A:N:abcdDfhHiIj:lLoOBt:*vxXsS:w:+:", od_longopts, \
	/* -w with optional param */ \
	/* -S was -s and also had optional parameter */ \
	/* but in coreutils 6.3 it was renamed and now has */ \
	/* _mandatory_ parameter */ \
	&str_A, &str_N, &str_j, &lst_t, &str_S, &G.अष्टकानि_प्रत्येकम्_खण्डम्)


/* Check for 0x7f is a coreutils 6.3 addition */
#परिभाषय ISPRINT(c) (((c) >= ' ') && (c) < 0x7f)

प्रकारनिर्देश दीर्घ द्विगुण longdouble_t;
प्रकारनिर्देश अचिह्नित दीर्घ दीर्घ ulonglong_t;
प्रकारनिर्देश दीर्घ दीर्घ llong;

#यदि ENABLE_LFS
# define xstrtooff_sfx xstrtoull_sfx
#अन्यथा
# define xstrtooff_sfx xstrtoul_sfx
#यदि_अन्तः

/* The default number of input bytes per output line.  */
#परिभाषय DEFAULT_अष्टकानि_प्रत्येकम्_खण्डम् 16

/* The number of decimal digits of precision in a float.  */
#यदि_अपरिभाषितम् FLT_DIG
# define FLT_DIG 7
#यदि_अन्तः

/* The number of decimal digits of precision in a double.  */
#यदि_अपरिभाषितम् DBL_DIG
# define DBL_DIG 15
#यदि_अन्तः

/* The number of decimal digits of precision in a long double.  */
#यदि_अपरिभाषितम् LDBL_DIG
# define LDBL_DIG DBL_DIG
#यदि_अन्तः

गणना आकार_spec {
	NO_आकार,
	CHAR,
	SHORT,
	INT,
	LONG,
	LONG_LONG,
	FLOAT_SINGLE,
	FLOAT_DOUBLE,
	FLOAT_LONG_DOUBLE,
	N_आकार_SPECS
};

गणना output_format {
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
संरचना tspec {
	गणना output_format fmt;
	गणना आकार_spec आकार;
	शून्य (*print_function) (आकार_t, ध्रुव वर्ण *, ध्रुव वर्ण *);
	वर्ण *fmt_string;
	पूर्णाङ्क hexl_बहुलकम्_trailer;
	पूर्णाङ्क क्षेत्रम्_विस्तारः;
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

स्थिर ध्रुव uint8_t अष्टकानि_गन्तव्यम्_oct_digits[] ALIGN1 =
{0, 3, 6, 8, 11, 14, 16, 19, 22, 25, 27, 30, 32, 35, 38, 41, 43};

स्थिर ध्रुव uint8_t अष्टकानि_गन्तव्यम्_signed_dec_digits[] ALIGN1 =
{1, 4, 6, 8, 11, 13, 16, 18, 20, 23, 25, 28, 30, 33, 35, 37, 40};

स्थिर ध्रुव uint8_t अष्टकानि_गन्तव्यम्_unsigned_dec_digits[] ALIGN1 =
{0, 3, 5, 8, 10, 13, 15, 17, 20, 22, 25, 27, 29, 32, 34, 37, 39};

स्थिर ध्रुव uint8_t अष्टकानि_गन्तव्यम्_hex_digits[] ALIGN1 =
{0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32};

/* Convert enum size_spec to the size of the named type.  */
स्थिर ध्रुव चिह्नित वर्ण विस्तारः_अष्टकानि[] ALIGN1 = {
	-1,
	परिमाण(वर्ण),
	परिमाण(ह्रस्व),
	परिमाण(पूर्णाङ्क),
	परिमाण(दीर्घ),
	परिमाण(ulonglong_t),
	परिमाण(भिन्न),
	परिमाण(द्विगुण),
	परिमाण(longdouble_t)
};
/* Ensure that for each member of 'enum size_spec' there is an
   initializer in the width_bytes array.  */
संरचना ERR_विस्तारः_अष्टकानि_has_bad_आकार {
	वर्ण ERR_विस्तारः_अष्टकानि_has_bad_आकार[ARRAY_आकार(विस्तारः_अष्टकानि) == N_आकार_SPECS ? 1 : -1];
};

संरचना globals {
	smallint exit_संहिता;

	अचिह्नित string_min;

	/* An array of specs describing how to format each input block.  */
	अचिह्नित n_specs;
	संरचना tspec *spec;

	/* Function that accepts an address and an optional following char,
	   and prints the address and char to stdout.  */
	शून्य (*format_स्थानाङ्कः)(off_t, वर्ण);

	/* The difference between the old-style pseudo starting address and
	   the number of bytes to skip.  */
#यदि ENABLE_LONG_OPTS
	off_t pseudo_विस्थापनम्;
# define G_pseudo_विस्थापनम् G.pseudo_विस्थापनम्
#यदि_अन्तः
	/* When zero, MAX_BYTES_TO_FORMAT and END_OFFSET are ignored, and all
	   input is formatted.  */

	/* The number of input bytes formatted per output line.  It must be
	   a multiple of the least common multiple of the sizes associated with
	   the specified output types.  It should be as large as possible, but
	   no larger than 16 -- unless specified with the -w option.  */
	अचिह्नित अष्टकानि_प्रत्येकम्_खण्डम्; /* have to use unsigned, not size_t */

	/* A NULL-terminated list of the file-arguments from the command line.  */
	ध्रुव वर्ण *ध्रुव *file_list;

	/* The input stream associated with the current file.  */
	FILE *अन्तर्_प्रवाहः;

	bool not_first;
	bool पूर्व_pair_equal;

	वर्ण स्थानाङ्कः_fmt[परिमाण("%0n"OFF_FMT"xc")];
} FIX_ALIASING;
/* Corresponds to 'x' above */
#परिभाषय स्थानाङ्कः_आधारः_char G.स्थानाङ्कः_fmt[परिमाण(G.स्थानाङ्कः_fmt)-3]
/* Corresponds to 'n' above */
#परिभाषय स्थानाङ्कः_pad_len_char G.स्थानाङ्कः_fmt[2]

#यदि !ENABLE_LONG_OPTS
गणना { G_pseudo_विस्थापनम् = 0 };
#यदि_अन्तः
#परिभाषय G (*(संरचना globals*)bb_common_bufsiz1)
#परिभाषय INIT_G() कुरु { \
	सज्जता_common_bufsiz(); \
	BUILD_BUG_ON(परिमाण(G) > COMMON_BUFSIZE); \
	G.अष्टकानि_प्रत्येकम्_खण्डम् = 32; \
	वाक्यं_प्रतिलिख(G.स्थानाङ्कः_fmt, "%0n"OFF_FMT"xc"); \
} यावत् (0)


#परिभाषय MAX_INTEGRAL_TYPE_आकार परिमाण(ulonglong_t)
स्थिर ध्रुव अचिह्नित वर्ण integral_type_आकार[MAX_INTEGRAL_TYPE_आकार + 1] ALIGN1 = {
	[परिमाण(वर्ण)] = CHAR,
#यदि USHRT_MAX != UCHAR_MAX
	[परिमाण(ह्रस्व)] = SHORT,
#यदि_अन्तः
#यदि UINT_MAX != USHRT_MAX
	[परिमाण(पूर्णाङ्क)] = INT,
#यदि_अन्तः
#यदि ULONG_MAX != UINT_MAX
	[परिमाण(दीर्घ)] = LONG,
#यदि_अन्तः
#यदि ULLONG_MAX != ULONG_MAX
	[परिमाण(ulonglong_t)] = LONG_LONG,
#यदि_अन्तः
};

#परिभाषय MAX_FP_TYPE_आकार परिमाण(longdouble_t)
स्थिर ध्रुव अचिह्नित वर्ण fp_type_आकार[MAX_FP_TYPE_आकार + 1] ALIGN1 = {
	/* gcc seems to allow repeated indexes. Last one wins */
	[परिमाण(longdouble_t)] = FLOAT_LONG_DOUBLE,
	[परिमाण(द्विगुण)] = FLOAT_DOUBLE,
	[परिमाण(भिन्न)] = FLOAT_SINGLE
};


स्थिर अचिह्नित
gcd(अचिह्नित u, अचिह्नित v)
{
	अचिह्नित t;
	यावत् (v != 0) {
		t = u % v;
		u = v;
		v = t;
	}
	प्रत्यागम u;
}

/* Compute the least common multiple of U and V.  */
स्थिर अचिह्नित
lcm(अचिह्नित u, अचिह्नित v) {
	अचिह्नित t = gcd(u, v);
	यदि (t == 0)
		प्रत्यागम 0;
	प्रत्यागम u * v / t;
}

स्थिर शून्य
print_s_char(आकार_t n_अष्टकानि, ध्रुव वर्ण *खण्डम्, ध्रुव वर्ण *fmt_string)
{
	यावत् (n_अष्टकानि--) {
		पूर्णाङ्क tmp = *(चिह्नित वर्ण *) खण्डम्;
		प्रारूप_लिख(fmt_string, tmp);
		खण्डम् += परिमाण(अचिह्नित वर्ण);
	}
}

स्थिर शून्य
print_char(आकार_t n_अष्टकानि, ध्रुव वर्ण *खण्डम्, ध्रुव वर्ण *fmt_string)
{
	यावत् (n_अष्टकानि--) {
		अचिह्नित tmp = *(अचिह्नित वर्ण *) खण्डम्;
		प्रारूप_लिख(fmt_string, tmp);
		खण्डम् += परिमाण(अचिह्नित वर्ण);
	}
}

स्थिर शून्य
print_s_short(आकार_t n_अष्टकानि, ध्रुव वर्ण *खण्डम्, ध्रुव वर्ण *fmt_string)
{
	n_अष्टकानि /= परिमाण(चिह्नित ह्रस्व);
	यावत् (n_अष्टकानि--) {
		पूर्णाङ्क tmp = *(चिह्नित ह्रस्व *) खण्डम्;
		प्रारूप_लिख(fmt_string, tmp);
		खण्डम् += परिमाण(अचिह्नित ह्रस्व);
	}
}

स्थिर शून्य
print_short(आकार_t n_अष्टकानि, ध्रुव वर्ण *खण्डम्, ध्रुव वर्ण *fmt_string)
{
	n_अष्टकानि /= परिमाण(अचिह्नित ह्रस्व);
	यावत् (n_अष्टकानि--) {
		अचिह्नित tmp = *(अचिह्नित ह्रस्व *) खण्डम्;
		प्रारूप_लिख(fmt_string, tmp);
		खण्डम् += परिमाण(अचिह्नित ह्रस्व);
	}
}

स्थिर शून्य
print_int(आकार_t n_अष्टकानि, ध्रुव वर्ण *खण्डम्, ध्रुव वर्ण *fmt_string)
{
	n_अष्टकानि /= परिमाण(अचिह्नित);
	यावत् (n_अष्टकानि--) {
		अचिह्नित tmp = *(अचिह्नित *) खण्डम्;
		प्रारूप_लिख(fmt_string, tmp);
		खण्डम् += परिमाण(अचिह्नित);
	}
}

#यदि UINT_MAX == ULONG_MAX
# define print_long print_int
#अन्यथा
स्थिर शून्य
print_long(आकार_t n_अष्टकानि, ध्रुव वर्ण *खण्डम्, ध्रुव वर्ण *fmt_string)
{
	n_अष्टकानि /= परिमाण(अचिह्नित दीर्घ);
	यावत् (n_अष्टकानि--) {
		अचिह्नित दीर्घ tmp = *(अचिह्नित दीर्घ *) खण्डम्;
		प्रारूप_लिख(fmt_string, tmp);
		खण्डम् += परिमाण(अचिह्नित दीर्घ);
	}
}
#यदि_अन्तः

#यदि ULONG_MAX == ULLONG_MAX
# define print_long_long print_long
#अन्यथा
स्थिर शून्य
print_long_long(आकार_t n_अष्टकानि, ध्रुव वर्ण *खण्डम्, ध्रुव वर्ण *fmt_string)
{
	n_अष्टकानि /= परिमाण(ulonglong_t);
	यावत् (n_अष्टकानि--) {
		ulonglong_t tmp = *(ulonglong_t *) खण्डम्;
		प्रारूप_लिख(fmt_string, tmp);
		खण्डम् += परिमाण(ulonglong_t);
	}
}
#यदि_अन्तः

स्थिर शून्य
print_float(आकार_t n_अष्टकानि, ध्रुव वर्ण *खण्डम्, ध्रुव वर्ण *fmt_string)
{
	n_अष्टकानि /= परिमाण(भिन्न);
	यावत् (n_अष्टकानि--) {
		भिन्न tmp = *(भिन्न *) खण्डम्;
		प्रारूप_लिख(fmt_string, tmp);
		खण्डम् += परिमाण(भिन्न);
	}
}

स्थिर शून्य
print_double(आकार_t n_अष्टकानि, ध्रुव वर्ण *खण्डम्, ध्रुव वर्ण *fmt_string)
{
	n_अष्टकानि /= परिमाण(द्विगुण);
	यावत् (n_अष्टकानि--) {
		द्विगुण tmp = *(द्विगुण *) खण्डम्;
		प्रारूप_लिख(fmt_string, tmp);
		खण्डम् += परिमाण(द्विगुण);
	}
}

स्थिर शून्य
print_long_double(आकार_t n_अष्टकानि, ध्रुव वर्ण *खण्डम्, ध्रुव वर्ण *fmt_string)
{
	n_अष्टकानि /= परिमाण(longdouble_t);
	यावत् (n_अष्टकानि--) {
		longdouble_t tmp = *(longdouble_t *) खण्डम्;
		प्रारूप_लिख(fmt_string, tmp);
		खण्डम् += परिमाण(longdouble_t);
	}
}

/* print_[named]_ascii are optimized for speed.
 * Remember, someday you may want to pump gigabytes through this thing.
 * Saving a dozen of .text bytes here is counter-productive */

स्थिर शून्य
print_named_ascii(आकार_t n_अष्टकानि, ध्रुव वर्ण *खण्डम्,
		ध्रुव वर्ण *अप्रयुक्तम्_fmt_string अप्रयुक्तम्_PARAM)
{
	/* Names for some non-printing characters.  */
	स्थिर ध्रुव वर्ण charname[33][3] ALIGN1 = {
		"nul", "soh", "stx", "etx", "eot", "enq", "ack", "bel",
		" bs", " ht", " nl", " vt", " ff", " cr", " so", " si",
		"dle", "dc1", "dc2", "dc3", "dc4", "nak", "syn", "etb",
		"can", " em", "sub", "esc", " fs", " gs", " rs", " us",
		" sp"
	};
	// buf[N] pos:  01234 56789
	वर्ण buf[12] = "   x\0 xxx\0";
	// [12] because we take three 32bit stack slots anyway, and
	// gcc is too dumb to initialize with constant stores,
	// it copies initializer from rodata. Oh well.
	// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=65410

	यावत् (n_अष्टकानि--) {
		अचिह्नित masked_c = *(अचिह्नित वर्ण *) खण्डम्++;

		masked_c &= 0x7f;
		यदि (masked_c == 0x7f) {
			fputs_stdout(" del");
			अनुवर्त;
		}
		यदि (masked_c > ' ') {
			buf[3] = masked_c;
			fputs_stdout(buf);
			अनुवर्त;
		}
		/* Why? Because printf(" %3.3s") is much slower... */
		buf[6] = charname[masked_c][0];
		buf[7] = charname[masked_c][1];
		buf[8] = charname[masked_c][2];
		fputs_stdout(buf+5);
	}
}

स्थिर शून्य
print_ascii(आकार_t n_अष्टकानि, ध्रुव वर्ण *खण्डम्,
		ध्रुव वर्ण *अप्रयुक्तम्_fmt_string अप्रयुक्तम्_PARAM)
{
	// buf[N] pos:  01234 56789
	वर्ण buf[12] = "   x\0 xxx\0";

	यावत् (n_अष्टकानि--) {
		ध्रुव वर्ण *s;
		अचिह्नित c = *(अचिह्नित वर्ण *) खण्डम्++;

		यदि (ISPRINT(c)) {
			buf[3] = c;
			fputs_stdout(buf);
			अनुवर्त;
		}
		विकल्प (c) {
		पक्ष '\0':
			s = "  \\0";
			भङ्ग;
		पक्ष '\007':
			s = "  \\a";
			भङ्ग;
		पक्ष '\b':
			s = "  \\b";
			भङ्ग;
		पक्ष '\f':
			s = "  \\f";
			भङ्ग;
		पक्ष '\n':
			s = "  \\n";
			भङ्ग;
		पक्ष '\r':
			s = "  \\r";
			भङ्ग;
		पक्ष '\t':
			s = "  \\t";
			भङ्ग;
		पक्ष '\v':
			s = "  \\v";
			भङ्ग;
		स्वभाव:
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

स्थिर शून्य
open_अग्र_file(शून्य)
{
	यावत् (1) {
		यदि (!*G.file_list)
			प्रत्यागम;
		G.अन्तर्_प्रवाहः = fopen_or_warn_stdin(*G.file_list++);
		यदि (G.अन्तर्_प्रवाहः) {
			भङ्ग;
		}
		G.exit_संहिता = 1;
	}

	यदि ((option_mask32 & (OPT_N|OPT_S)) == OPT_N)
		setbuf(G.अन्तर्_प्रवाहः, रिक्त);
}

/* Test whether there have been errors on in_stream, and close it if
   it is not standard input.  Return nonzero if there has been an error
   on in_stream or stdout; return zero otherwise.  This function will
   report more than one error only if both a read and a write error
   have occurred.  IN_ERRNO, if nonzero, is the error number
   corresponding to the most recent action for IN_STREAM.  */

स्थिर शून्य
check_and_close(शून्य)
{
	यदि (G.अन्तर्_प्रवाहः) {
		यदि (ferror(G.अन्तर्_प्रवाहः))	{
			bb_error_सन्देशः("%s: read error", (G.अन्तर्_प्रवाहः == stdin)
					? bb_सन्देशः_standard_निवेशः
					: G.file_list[-1]
			);
			G.exit_संहिता = 1;
		}
		fclose_if_not_stdin(G.अन्तर्_प्रवाहः);
		G.अन्तर्_प्रवाहः = रिक्त;
	}

	यदि (ferror(stdout)) {
		bb_simple_error_सन्देशः_and_die(bb_सन्देशः_write_error);
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

स्थिर NOINLINE ध्रुव वर्ण *
decode_one_format(ध्रुव वर्ण *s_orig, ध्रुव वर्ण *s, संरचना tspec *tspec)
{
	गणना आकार_spec आकार_spec;
	अचिह्नित आकार;
	गणना output_format fmt;
	ध्रुव वर्ण *p;
	वर्ण *अन्तः;
	वर्ण *fmt_string = रिक्त;
	शून्य (*print_function) (आकार_t, ध्रुव वर्ण *, ध्रुव वर्ण *);
	अचिह्नित c;
	अचिह्नित क्षेत्रम्_विस्तारः = 0;
	पूर्णाङ्क pos;

	विकल्प (*s) {
	पक्ष 'd':
	पक्ष 'o':
	पक्ष 'u':
	पक्ष 'x': {
		स्थिर ध्रुव वर्ण CSIL[] ALIGN1 = "CSIL";

		c = *s++;
		p = strchr(CSIL, *s);
		/* if *s == NUL, p != NULL! Testcase: "od -tx" */
		यदि (!p || *p == '\0') {
			आकार = परिमाण(पूर्णाङ्क);
			यदि (अङ्कः_वा(s[0])) {
				आकार = bb_strtou(s, &अन्तः, 0);
				यदि (errno == ERANGE
				 || MAX_INTEGRAL_TYPE_आकार < आकार
				 || integral_type_आकार[आकार] == NO_आकार
				) {
					bb_error_सन्देशः_and_die("invalid type string '%s'; "
						"%u-byte %s type is not supported",
						s_orig, आकार, "integral");
				}
				s = अन्तः;
			}
		} अन्यथा {
			स्थिर ध्रुव uint8_t CSIL_sizeof[4] = {
				परिमाण(वर्ण),
				परिमाण(ह्रस्व),
				परिमाण(पूर्णाङ्क),
				परिमाण(दीर्घ),
			};
			आकार = CSIL_sizeof[p - CSIL];
			s++; /* skip C/S/I/L */
		}

#परिभाषय ISPEC_गन्तव्यम्_FORMAT(Spec, Min_format, Long_format, Max_format) \
	((Spec) == LONG_LONG ? (Max_format) \
	: ((Spec) == LONG ? (Long_format) : (Min_format)))

#परिभाषय FMT_अष्टकानि_ALLOCATED 9
		आकार_spec = integral_type_आकार[आकार];

		{
			स्थिर ध्रुव वर्ण doux[] ALIGN1 = "doux";
			स्थिर ध्रुव वर्ण doux_fmt_letter[][4] = {
				"lld", "llo", "llu", "llx"
			};
			स्थिर ध्रुव गणना output_format doux_fmt[] = {
				SIGNED_DECIMAL,
				OCTAL,
				UNSIGNED_DECIMAL,
				HEXADECIMAL,
			};
			स्थिर ध्रुव uint8_t *ध्रुव doux_अष्टकानि_गन्तव्यम्_XXX[] = {
				अष्टकानि_गन्तव्यम्_signed_dec_digits,
				अष्टकानि_गन्तव्यम्_oct_digits,
				अष्टकानि_गन्तव्यम्_unsigned_dec_digits,
				अष्टकानि_गन्तव्यम्_hex_digits,
			};
			स्थिर ध्रुव वर्ण doux_fmtstring[][परिमाण(" %%0%u%s")] ALIGN1 = {
				" %%%u%s",
				" %%0%u%s",
				" %%%u%s",
				" %%0%u%s",
			};

			pos = strchr(doux, c) - doux;
			fmt = doux_fmt[pos];
			क्षेत्रम्_विस्तारः = doux_अष्टकानि_गन्तव्यम्_XXX[pos][आकार];
			p = doux_fmt_letter[pos] + 2;
			यदि (आकार_spec == LONG) p--;
			यदि (आकार_spec == LONG_LONG) p -= 2;
			fmt_string = xasprintf(doux_fmtstring[pos], क्षेत्रम्_विस्तारः, p);
		}

		विकल्प (आकार_spec) {
		पक्ष CHAR:
			print_function = (fmt == SIGNED_DECIMAL
				    ? print_s_char
				    : print_char);
			भङ्ग;
		पक्ष SHORT:
			print_function = (fmt == SIGNED_DECIMAL
				    ? print_s_short
				    : print_short);
			भङ्ग;
		पक्ष INT:
			print_function = print_int;
			भङ्ग;
		पक्ष LONG:
			print_function = print_long;
			भङ्ग;
		स्वभाव: /* case LONG_LONG: */
			print_function = print_long_long;
			भङ्ग;
		}
		भङ्ग;
	}

	पक्ष 'f': {
		स्थिर ध्रुव वर्ण FDL[] ALIGN1 = "FDL";

		fmt = FLOATING_POINT;
		++s;
		p = strchr(FDL, *s);
		यदि (!p || *p == '\0') {
			आकार = परिमाण(द्विगुण);
			यदि (अङ्कः_वा(s[0])) {
				आकार = bb_strtou(s, &अन्तः, 0);
				यदि (errno == ERANGE || आकार > MAX_FP_TYPE_आकार
				 || fp_type_आकार[आकार] == NO_आकार
				) {
					bb_error_सन्देशः_and_die("invalid type string '%s'; "
						"%u-byte %s type is not supported",
						s_orig, आकार, "floating point");
				}
				s = अन्तः;
			}
		} अन्यथा {
			स्थिर ध्रुव uint8_t FDL_sizeof[] = {
				परिमाण(भिन्न),
				परिमाण(द्विगुण),
				परिमाण(longdouble_t),
			};

			आकार = FDL_sizeof[p - FDL];
			s++; /* skip F/D/L */
		}

		आकार_spec = fp_type_आकार[आकार];

		विकल्प (आकार_spec) {
		पक्ष FLOAT_SINGLE:
			print_function = print_float;
			क्षेत्रम्_विस्तारः = FLT_DIG + 8;
			/* Don't use %#e; not all systems support it.  */
			fmt_string = xasprintf(" %%%d.%de", क्षेत्रम्_विस्तारः, FLT_DIG);
			भङ्ग;
		पक्ष FLOAT_DOUBLE:
			print_function = print_double;
			क्षेत्रम्_विस्तारः = DBL_DIG + 8;
			fmt_string = xasprintf(" %%%d.%de", क्षेत्रम्_विस्तारः, DBL_DIG);
			भङ्ग;
		स्वभाव: /* case FLOAT_LONG_DOUBLE: */
			print_function = print_long_double;
			क्षेत्रम्_विस्तारः = LDBL_DIG + 8;
			fmt_string = xasprintf(" %%%d.%dLe", क्षेत्रम्_विस्तारः, LDBL_DIG);
			भङ्ग;
		}
		भङ्ग;
	}

	पक्ष 'a':
		++s;
		fmt = NAMED_CHARACTER;
		आकार_spec = CHAR;
		print_function = print_named_ascii;
		क्षेत्रम्_विस्तारः = 3;
		भङ्ग;
	पक्ष 'c':
		++s;
		fmt = CHARACTER;
		आकार_spec = CHAR;
		print_function = print_ascii;
		क्षेत्रम्_विस्तारः = 3;
		भङ्ग;
	स्वभाव:
		bb_error_सन्देशः_and_die("invalid character '%c' "
				"in type string '%s'", *s, s_orig);
	}

	tspec->आकार = आकार_spec;
	tspec->fmt = fmt;
	tspec->print_function = print_function;
	tspec->fmt_string = fmt_string;

	tspec->क्षेत्रम्_विस्तारः = क्षेत्रम्_विस्तारः;
	tspec->hexl_बहुलकम्_trailer = (*s == 'z');
	यदि (tspec->hexl_बहुलकम्_trailer)
		s++;

	प्रत्यागम s;
}

/* Decode the modern od format string S.  Append the decoded
   representation to the global array SPEC, reallocating SPEC if
   necessary.  */

स्थिर शून्य
decode_format_string(ध्रुव वर्ण *s)
{
	ध्रुव वर्ण *s_orig = s;

	यावत् (*s != '\0') {
		संरचना tspec tspec;
		ध्रुव वर्ण *अग्र;

		अग्र = decode_one_format(s_orig, s, &tspec);

		प्रतिज्ञा(s != अग्र);
		s = अग्र;
		G.spec = xrealloc_सदिशम्(G.spec, 4, G.n_specs);
		स्मृतिं_प्रतिलिख(&G.spec[G.n_specs], &tspec, परिमाण(G.spec[0]));
		G.n_specs++;
	}
}

/* Given a list of one or more input filenames FILE_LIST, set the global
   file pointer IN_STREAM to position N_SKIP in the concatenation of
   those files.  If any file operation fails or if there are fewer than
   N_SKIP bytes in the combined input, give an error message and return
   nonzero.  When possible, use seek rather than read operations to
   advance IN_STREAM.  */

स्थिर शून्य
skip(off_t n_skip)
{
	यदि (n_skip == 0)
		प्रत्यागम;

	यावत् (G.अन्तर्_प्रवाहः) { /* !EOF */
		संरचना stat file_stats;

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
		यदि (fstat(fileno(G.अन्तर्_प्रवाहः), &file_stats) == 0
		 && S_ISREG(file_stats.st_बहुलकम्) && file_stats.st_आकार > 0
		) {
			यदि (file_stats.st_आकार < n_skip) {
				n_skip -= file_stats.st_आकार;
				/* take "check & close / open_next" route */
			} अन्यथा {
				यदि (fseeko(G.अन्तर्_प्रवाहः, n_skip, SEEK_CUR) != 0)
					G.exit_संहिता = 1;
				प्रत्यागम;
			}
		} अन्यथा {
			/* If it's not a regular file with positive size,
			   position the file pointer by reading.  */
			वर्ण buf[1024];
			आकार_t n_अष्टकानि_गन्तव्यम्_read = 1024;
			आकार_t n_अष्टकानि_read;

			यावत् (n_skip > 0) {
				यदि (n_skip < n_अष्टकानि_गन्तव्यम्_read)
					n_अष्टकानि_गन्तव्यम्_read = n_skip;
				n_अष्टकानि_read = fread(buf, 1, n_अष्टकानि_गन्तव्यम्_read, G.अन्तर्_प्रवाहः);
				n_skip -= n_अष्टकानि_read;
				यदि (n_अष्टकानि_read != n_अष्टकानि_गन्तव्यम्_read)
					भङ्ग; /* EOF on this file or error */
			}
		}
		यदि (n_skip == 0)
			प्रत्यागम;

		check_and_close();
		open_अग्र_file();
	}

	यदि (n_skip)
		bb_simple_error_सन्देशः_and_die("can't skip past end of combined input");
}


प्रकारनिर्देश शून्य FN_format_स्थानाङ्कः(off_t स्थानाङ्कः, वर्ण c);

स्थिर शून्य
format_स्थानाङ्कः_none(off_t स्थानाङ्कः अप्रयुक्तम्_PARAM, वर्ण c अप्रयुक्तम्_PARAM)
{
}

स्थिर शून्य
format_स्थानाङ्कः_std(off_t स्थानाङ्कः, वर्ण c)
{
	/* Corresponds to 'c' */
	G.स्थानाङ्कः_fmt[परिमाण(G.स्थानाङ्कः_fmt)-2] = c;
	प्रारूप_लिख(G.स्थानाङ्कः_fmt, स्थानाङ्कः);
}

#यदि ENABLE_LONG_OPTS
/* only used with --traditional */
स्थिर शून्य
format_स्थानाङ्कः_paren(off_t स्थानाङ्कः, वर्ण c)
{
	अक्षरं_लिख('(');
	format_स्थानाङ्कः_std(स्थानाङ्कः, ')');
	यदि (c) अक्षरं_लिख(c);
}

स्थिर शून्य
format_स्थानाङ्कः_label(off_t स्थानाङ्कः, वर्ण c)
{
	format_स्थानाङ्कः_std(स्थानाङ्कः, ' ');
	format_स्थानाङ्कः_paren(स्थानाङ्कः + G_pseudo_विस्थापनम्, c);
}
#यदि_अन्तः

स्थिर शून्य
dump_hexl_बहुलकम्_trailer(आकार_t n_अष्टकानि, ध्रुव वर्ण *खण्डम्)
{
	fputs_stdout("  >");
	यावत् (n_अष्टकानि--) {
		अचिह्नित c = *(अचिह्नित वर्ण *) खण्डम्++;
		c = (ISPRINT(c) ? c : '.');
		अक्षरं_लिख(c);
	}
	अक्षरं_लिख('<');
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

स्थिर शून्य
write_खण्डम्(off_t current_विस्थापनम्, आकार_t n_अष्टकानि,
		ध्रुव वर्ण *पूर्व_खण्डम्, ध्रुव वर्ण *curr_खण्डम्)
{
	अचिह्नित i;

	यदि (!(option_mask32 & OPT_v)
	 && G.not_first
	 && n_अष्टकानि == G.अष्टकानि_प्रत्येकम्_खण्डम्
	 && memcmp(पूर्व_खण्डम्, curr_खण्डम्, G.अष्टकानि_प्रत्येकम्_खण्डम्) == 0
	) {
		यदि (G.पूर्व_pair_equal) {
			/* The two preceding blocks were equal, and the current
			   block is the same as the last one, so print nothing.  */
		} अन्यथा {
			पङ्क्तिं_लिख("*");
			G.पूर्व_pair_equal = 1;
		}
	} अन्यथा {
		G.not_first = 1;
		G.पूर्व_pair_equal = 0;
		प्रति (i = 0; i < G.n_specs; i++) {
			यदि (i == 0)
				G.format_स्थानाङ्कः(current_विस्थापनम्, '\0');
			अन्यथा
				प्रारूप_लिख("%*s", स्थानाङ्कः_pad_len_char - '0', "");
			(*G.spec[i].print_function) (n_अष्टकानि, curr_खण्डम्, G.spec[i].fmt_string);
			यदि (G.spec[i].hexl_बहुलकम्_trailer) {
				/* space-pad out to full line width, then dump the trailer */
				अचिह्नित datum_विस्तारः = विस्तारः_अष्टकानि[G.spec[i].आकार];
				अचिह्नित blank_fields = (G.अष्टकानि_प्रत्येकम्_खण्डम् - n_अष्टकानि) / datum_विस्तारः;
				अचिह्नित क्षेत्रम्_विस्तारः = G.spec[i].क्षेत्रम्_विस्तारः + 1;
				प्रारूप_लिख("%*s", blank_fields * क्षेत्रम्_विस्तारः, "");
				dump_hexl_बहुलकम्_trailer(n_अष्टकानि, curr_खण्डम्);
			}
			अक्षरं_लिख('\n');
		}
	}
}

स्थिर शून्य
read_खण्डम्(आकार_t n, वर्ण *खण्डम्, आकार_t *n_अष्टकानि_अन्तर्_बफर)
{
	प्रतिज्ञा(0 < n && n <= G.अष्टकानि_प्रत्येकम्_खण्डम्);

	*n_अष्टकानि_अन्तर्_बफर = 0;

	यदि (n == 0)
		प्रत्यागम;

	यावत् (G.अन्तर्_प्रवाहः != रिक्त) { /* EOF.  */
		आकार_t n_needed;
		आकार_t n_read;

		n_needed = n - *n_अष्टकानि_अन्तर्_बफर;
		n_read = fread(खण्डम् + *n_अष्टकानि_अन्तर्_बफर, 1, n_needed, G.अन्तर्_प्रवाहः);
		*n_अष्टकानि_अन्तर्_बफर += n_read;
		यदि (n_read == n_needed)
			भङ्ग;
		/* error check is done in check_and_close */
		check_and_close();
		open_अग्र_file();
	}
}

/* Return the least common multiple of the sizes associated
   with the format specs.  */

स्थिर पूर्णाङ्क
get_lcm(शून्य)
{
	आकार_t i;
	पूर्णाङ्क l_c_m = 1;

	प्रति (i = 0; i < G.n_specs; i++)
		l_c_m = lcm(l_c_m, विस्तारः_अष्टकानि[(पूर्णाङ्क) G.spec[i].आकार]);
	प्रत्यागम l_c_m;
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

स्थिर शून्य
dump(off_t current_विस्थापनम्, off_t अन्तः_विस्थापनम्)
{
	वर्ण *खण्डम्[2];
	पूर्णाङ्क idx;
	आकार_t n_अष्टकानि_read;

	खण्डम्[0] = xmalloc(2 * G.अष्टकानि_प्रत्येकम्_खण्डम्);
	खण्डम्[1] = खण्डम्[0] + G.अष्टकानि_प्रत्येकम्_खण्डम्;

	idx = 0;
	यदि (option_mask32 & OPT_N) {
		यावत् (1) {
			आकार_t n_needed;
			यदि (current_विस्थापनम् >= अन्तः_विस्थापनम्) {
				n_अष्टकानि_read = 0;
				भङ्ग;
			}
			n_needed = MIN(अन्तः_विस्थापनम् - current_विस्थापनम्, (off_t) G.अष्टकानि_प्रत्येकम्_खण्डम्);
			read_खण्डम्(n_needed, खण्डम्[idx], &n_अष्टकानि_read);
			यदि (n_अष्टकानि_read < G.अष्टकानि_प्रत्येकम्_खण्डम्)
				भङ्ग;
			प्रतिज्ञा(n_अष्टकानि_read == G.अष्टकानि_प्रत्येकम्_खण्डम्);
			write_खण्डम्(current_विस्थापनम्, n_अष्टकानि_read, खण्डम्[idx ^ 1], खण्डम्[idx]);
			current_विस्थापनम् += n_अष्टकानि_read;
			idx ^= 1;
		}
	} अन्यथा {
		यावत् (1) {
			read_खण्डम्(G.अष्टकानि_प्रत्येकम्_खण्डम्, खण्डम्[idx], &n_अष्टकानि_read);
			यदि (n_अष्टकानि_read < G.अष्टकानि_प्रत्येकम्_खण्डम्)
				भङ्ग;
			प्रतिज्ञा(n_अष्टकानि_read == G.अष्टकानि_प्रत्येकम्_खण्डम्);
			write_खण्डम्(current_विस्थापनम्, n_अष्टकानि_read, खण्डम्[idx ^ 1], खण्डम्[idx]);
			current_विस्थापनम् += n_अष्टकानि_read;
			idx ^= 1;
		}
	}

	यदि (n_अष्टकानि_read > 0) {
		पूर्णाङ्क l_c_m;
		आकार_t अष्टकानि_गन्तव्यम्_write;

		l_c_m = get_lcm();

		/* Make bytes_to_write the smallest multiple of l_c_m that
		   is at least as large as n_bytes_read.  */
		अष्टकानि_गन्तव्यम्_write = l_c_m * ((n_अष्टकानि_read + l_c_m - 1) / l_c_m);

		स्मृतिं_पूरय(खण्डम्[idx] + n_अष्टकानि_read, 0, अष्टकानि_गन्तव्यम्_write - n_अष्टकानि_read);
		write_खण्डम्(current_विस्थापनम्, अष्टकानि_गन्तव्यम्_write,
				खण्डम्[idx ^ 1], खण्डम्[idx]);
		current_विस्थापनम् += n_अष्टकानि_read;
	}

	G.format_स्थानाङ्कः(current_विस्थापनम्, '\n');

	यदि ((option_mask32 & OPT_N) && current_विस्थापनम् >= अन्तः_विस्थापनम्)
		check_and_close();

	मुक्त(खण्डम्[0]);
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

स्थिर शून्य
dump_वाक्यानि(off_t स्थानाङ्कः, off_t अन्तः_विस्थापनम्)
{
	अचिह्नित bufsize = MAX(100, G.string_min);
	अचिह्नित वर्ण *buf = xmalloc(bufsize);

	यावत् (1) {
		आकार_t i;
		पूर्णाङ्क c;

		/* See if the next 'G.string_min' chars are all printing chars.  */
 tryline:
		यदि ((option_mask32 & OPT_N) && (अन्तः_विस्थापनम् - G.string_min <= स्थानाङ्कः))
			भङ्ग;
		i = 0;
		यावत् (!(option_mask32 & OPT_N) || स्थानाङ्कः < अन्तः_विस्थापनम्) {
			यदि (i == bufsize) {
				bufsize += bufsize/8;
				buf = xrealloc(buf, bufsize);
			}

			यावत् (G.अन्तर्_प्रवाहः) { /* !EOF */
				c = fgetc(G.अन्तर्_प्रवाहः);
				यदि (c != EOF)
					गच्छ got_char;
				check_and_close();
				open_अग्र_file();
			}
			/* EOF */
			गच्छ ret;
 got_char:
			स्थानाङ्कः++;
			यदि (!c)
				भङ्ग;
			यदि (!ISPRINT(c))
				गच्छ tryline;	/* It isn't; give up on this string.  */
			buf[i++] = c;		/* String continues; store it all.  */
		}

		यदि (i < G.string_min)		/* Too short! */
			गच्छ tryline;

		/* If we get here, the string is all printable and NUL-terminated */
		buf[i] = 0;
		G.format_स्थानाङ्कः(स्थानाङ्कः - i - 1, ' ');

		प्रति (i = 0; (c = buf[i]); i++) {
			विकल्प (c) {
			पक्ष '\007': fputs_stdout("\\a"); भङ्ग;
			पक्ष '\b': fputs_stdout("\\b"); भङ्ग;
			पक्ष '\f': fputs_stdout("\\f"); भङ्ग;
			पक्ष '\n': fputs_stdout("\\n"); भङ्ग;
			पक्ष '\r': fputs_stdout("\\r"); भङ्ग;
			पक्ष '\t': fputs_stdout("\\t"); भङ्ग;
			पक्ष '\v': fputs_stdout("\\v"); भङ्ग;
			स्वभाव: अक्षरं_लिख(c);
			}
		}
		अक्षरं_लिख('\n');
	}

	/* We reach this point only if we search through
	   (max_bytes_to_format - G.string_min) bytes before reaching EOF.  */
	check_and_close();
 ret:
	मुक्त(buf);
}

#यदि ENABLE_LONG_OPTS
/* If S is a valid traditional offset specification with an optional
   leading '+' return nonzero and set *OFFSET to the offset it denotes.  */

स्थिर पूर्णाङ्क
parse_old_विस्थापनम्(ध्रुव वर्ण *s, off_t *विस्थापनम्)
{
	स्थिर ध्रुव संरचना suffix_mult Bb[] ALIGN_SUFFIX = {
		{ "B", 1024 },
		{ "b", 512 },
		{ "", 0 }
	};
	वर्ण *p;
	पूर्णाङ्क radix;

	/* Skip over any leading '+'. */
	यदि (s[0] == '+') ++s;
	यदि (!अङ्कः_वा(s[0])) प्रत्यागम 0; /* not a number */

	/* Determine the radix we'll use to interpret S.  If there is a '.',
	 * it's decimal, otherwise, if the string begins with '0X'or '0x',
	 * it's hexadecimal, else octal.  */
	p = strchr(s, '.');
	radix = 8;
	यदि (p) {
		p[0] = '\0'; /* cheating */
		radix = 10;
	} अन्यथा यदि (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
		radix = 16;

	*विस्थापनम् = xstrtooff_sfx(s, radix, Bb);
	यदि (p) p[0] = '.';

	प्रत्यागम (*विस्थापनम् >= 0);
}
#यदि_अन्तः

पूर्णाङ्क od_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क od_main(पूर्णाङ्क argc अप्रयुक्तम्_PARAM, वर्ण **argv)
{
#यदि ENABLE_LONG_OPTS
	स्थिर ध्रुव वर्ण od_longopts[] ALIGN1 =
		"skip-bytes\0"        Required_तर्कः "j"
		"address-radix\0"     Required_तर्कः "A"
		"read-bytes\0"        Required_तर्कः "N"
		"format\0"            Required_तर्कः "t"
		"output-duplicates\0" No_तर्कः       "v"
		/* Yes, it's true: -S NUM, but --strings[=NUM]!
		 * that is, NUM is mandatory for -S but optional for --strings!
		 */
		"strings\0"           Optional_तर्कः "S"
		"width\0"             Optional_तर्कः "w"
		"traditional\0"       No_तर्कः       "\xff"
		;
#यदि_अन्तः
	ध्रुव वर्ण *str_A, *str_N, *str_j, *str_S = "3";
	llist_t *lst_t = रिक्त;
	अचिह्नित opt;
	पूर्णाङ्क l_c_m;
	/* The number of input bytes to skip before formatting and writing.  */
	off_t n_अष्टकानि_गन्तव्यम्_skip = 0;
	/* The offset of the first byte after the last byte to be formatted.  */
	off_t अन्तः_विस्थापनम् = 0;
	/* The maximum number of bytes that will be formatted.  */
	off_t max_अष्टकानि_गन्तव्यम्_format = 0;

	INIT_G();

	/*G.spec = NULL; - already is */
	G.format_स्थानाङ्कः = format_स्थानाङ्कः_std;
	स्थानाङ्कः_आधारः_char = 'o';
	स्थानाङ्कः_pad_len_char = '7';

	/* Parse command line */
	opt = OD_GETOPT32();
	argv += optind;
	यदि (opt & OPT_A) {
		स्थिर ध्रुव वर्ण doxn[] ALIGN1 = "doxn";
		स्थिर ध्रुव वर्ण doxn_स्थानाङ्कः_आधारः_char[] ALIGN1 = {
			'u', 'o', 'x', /* '?' fourth one is not important */
		};
		स्थिर ध्रुव uint8_t doxn_स्थानाङ्कः_pad_len_char[] ALIGN1 = {
			'7', '7', '6', /* '?' */
		};
		वर्ण *p;
		पूर्णाङ्क pos;
		p = strchr(doxn, str_A[0]);
		यदि (!p)
			bb_error_सन्देशः_and_die("bad output address radix "
				"'%c' (must be [doxn])", str_A[0]);
		pos = p - doxn;
		यदि (pos == 3) G.format_स्थानाङ्कः = format_स्थानाङ्कः_none;
		स्थानाङ्कः_आधारः_char = doxn_स्थानाङ्कः_आधारः_char[pos];
		स्थानाङ्कः_pad_len_char = doxn_स्थानाङ्कः_pad_len_char[pos];
	}
	यदि (opt & OPT_N) {
		max_अष्टकानि_गन्तव्यम्_format = xstrtooff_sfx(str_N, 0, bkm_suffixes);
	}

	यदि (opt & OPT_a) decode_format_string("a");
	यदि (opt & OPT_b) decode_format_string("oC");
	यदि (opt & OPT_c) decode_format_string("c");
	यदि (opt & OPT_d) decode_format_string("u2");
	यदि (opt & OPT_D) decode_format_string("uI");
	यदि (opt & OPT_f) decode_format_string("fF");
	यदि (opt & (OPT_h|OPT_x)) decode_format_string("x2");
	यदि (opt & (OPT_H|OPT_X)) decode_format_string("xI");
	/* -I,L,l: depend on word width of the arch (what is "long"?) */
#यदि ULONG_MAX > 0xffffffff
	यदि (opt & OPT_i) decode_format_string("dI");
	यदि (opt & (OPT_I|OPT_l|OPT_L)) decode_format_string("dL");
#अन्यथा
	/* 32-bit arch: -I,L,l are the same as -i */
	यदि (opt & (OPT_i|OPT_I|OPT_l|OPT_L)) decode_format_string("dI");
#यदि_अन्तः
	यदि (opt & OPT_j) n_अष्टकानि_गन्तव्यम्_skip = xstrtooff_sfx(str_j, 0, bkm_suffixes);
	यदि (opt & (OPT_o|OPT_B)) decode_format_string("o2");
	यदि (opt & OPT_O) decode_format_string("oI");
	यावत् (lst_t) {
		decode_format_string(llist_निष्कासय(&lst_t));
	}
	यदि (opt & OPT_s) decode_format_string("d2");
	यदि (opt & OPT_S) {
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

#यदि ENABLE_LONG_OPTS
	यदि (opt & OPT_traditional) {
		यदि (argv[0]) {
			off_t pseudo_start = -1;
			off_t o1, o2;

			यदि (!argv[1]) { /* one arg */
				यदि (parse_old_विस्थापनम्(argv[0], &o1)) {
					/* od --traditional OFFSET */
					n_अष्टकानि_गन्तव्यम्_skip = o1;
					argv++;
				}
				/* od --traditional FILE */
			} अन्यथा यदि (!argv[2]) { /* two args */
				यदि (parse_old_विस्थापनम्(argv[0], &o1)
				 && parse_old_विस्थापनम्(argv[1], &o2)
				) {
					/* od --traditional OFFSET LABEL */
					n_अष्टकानि_गन्तव्यम्_skip = o1;
					pseudo_start = o2;
					argv += 2;
				} अन्यथा यदि (parse_old_विस्थापनम्(argv[1], &o2)) {
					/* od --traditional FILE OFFSET */
					n_अष्टकानि_गन्तव्यम्_skip = o2;
					argv[1] = रिक्त;
				} अन्यथा {
					bb_error_सन्देशः_and_die("invalid second argument '%s'", argv[1]);
				}
			} अन्यथा यदि (!argv[3]) { /* three args */
				यदि (parse_old_विस्थापनम्(argv[1], &o1)
				 && parse_old_विस्थापनम्(argv[2], &o2)
				) {
					/* od --traditional FILE OFFSET LABEL */
					n_अष्टकानि_गन्तव्यम्_skip = o1;
					pseudo_start = o2;
					argv[1] = रिक्त;
				} अन्यथा {
					bb_simple_error_सन्देशः_and_die("the last two arguments must be offsets");
				}
			} अन्यथा { /* >3 args */
				bb_simple_error_सन्देशः_and_die("too many arguments");
			}

			यदि (pseudo_start >= 0) {
				यदि (G.format_स्थानाङ्कः == format_स्थानाङ्कः_none) {
					स्थानाङ्कः_आधारः_char = 'o';
					स्थानाङ्कः_pad_len_char = '7';
					G.format_स्थानाङ्कः = format_स्थानाङ्कः_paren;
				} अन्यथा {
					G.format_स्थानाङ्कः = format_स्थानाङ्कः_label;
				}
				G_pseudo_विस्थापनम् = pseudo_start - n_अष्टकानि_गन्तव्यम्_skip;
			}
		}
		/* else: od --traditional (without args) */
	}
#यदि_अन्तः

	यदि (option_mask32 & OPT_N) {
		अन्तः_विस्थापनम् = n_अष्टकानि_गन्तव्यम्_skip + max_अष्टकानि_गन्तव्यम्_format;
		यदि (अन्तः_विस्थापनम् < n_अष्टकानि_गन्तव्यम्_skip)
			bb_simple_error_सन्देशः_and_die("SKIP + SIZE is too large");
	}

	यदि (G.n_specs == 0) {
		decode_format_string("o2");
		/*G.n_specs = 1; - done by decode_format_string */
	}

	/* If no files were listed on the command line,
	   set the global pointer FILE_LIST so that it
	   references the null-terminated list of one name: "-".  */
	G.file_list = bb_argv_dash;
	यदि (argv[0]) {
		/* Set the global pointer FILE_LIST so that it
		   references the first file-argument on the command-line.  */
		G.file_list = (वर्ण ध्रुव *ध्रुव *) argv;
	}

	/* Open the first input file */
	open_अग्र_file();
	/* Skip over any unwanted header bytes */
	skip(n_अष्टकानि_गन्तव्यम्_skip);
	यदि (!G.अन्तर्_प्रवाहः)
		प्रत्यागम EXIT_FAILURE;

	/* Compute output block length */
	l_c_m = get_lcm();

	यदि (opt & OPT_w) { /* -w: width */
		यदि (!G.अष्टकानि_प्रत्येकम्_खण्डम् || G.अष्टकानि_प्रत्येकम्_खण्डम् % l_c_m != 0) {
			bb_error_सन्देशः("warning: invalid width %u; using %d instead",
					(अचिह्नित)G.अष्टकानि_प्रत्येकम्_खण्डम्, l_c_m);
			G.अष्टकानि_प्रत्येकम्_खण्डम् = l_c_m;
		}
	} अन्यथा {
		G.अष्टकानि_प्रत्येकम्_खण्डम् = l_c_m;
		यदि (l_c_m < DEFAULT_अष्टकानि_प्रत्येकम्_खण्डम्)
			G.अष्टकानि_प्रत्येकम्_खण्डम् *= DEFAULT_अष्टकानि_प्रत्येकम्_खण्डम् / l_c_m;
	}

#यदि_परिभाषितम् DEBUG
	{
		पूर्णाङ्क i;
		प्रति (i = 0; i < G.n_specs; i++) {
			प्रारूप_लिख("%d: fmt='%s' width=%d\n",
				i, G.spec[i].fmt_string,
				विस्तारः_अष्टकानि[G.spec[i].आकार]);
		}
	}
#यदि_अन्तः

	यदि (option_mask32 & OPT_S)
		dump_वाक्यानि(n_अष्टकानि_गन्तव्यम्_skip, अन्तः_विस्थापनम्);
	अन्यथा
		dump(n_अष्टकानि_गन्तव्यम्_skip, अन्तः_विस्थापनम्);

	यदि (fclose(stdin))
		bb_simple_perror_सन्देशः_and_die(bb_सन्देशः_standard_निवेशः);

	प्रत्यागम G.exit_संहिता;
}
