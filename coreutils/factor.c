/*
 * Copyright (C) 2017 Denys Vlasenko <vda.linux@googlemail.com>
 *
 * Licensed under GPLv2, see file LICENSE in this source tree.
 */
//config:config FACTOR
//config:	bool "factor (3.2 kb)"
//config:	default y
//config:	help
//config:	factor factorizes integers

//applet:IF_FACTOR(APPLET(factor, BB_DIR_USR_BIN, BB_SUID_DROP))

//kbuild:lib-$(CONFIG_FACTOR) += factor.o

//usage:#define factor_trivial_usage
//usage:       "[NUMBER]..."
//usage:#define factor_full_usage "\n\n"
//usage:       "Print prime factors"

#ضمن "libbb.h"
#ضمن "common_bufsiz.h"

#إذا 0
# define dbg(...) bb_error_پیام(__VA_ARGS__)
#وإلا
# define dbg(...) ((فراغ)0)
#پایان_اگر

تعريفنوع بیعلامت دراز دراز wide_t;

#إذا ULLONG_MAX == (UINT_MAX * UINT_MAX + 2 * UINT_MAX)
/* "unsigned" is half as wide as ullong */
تعريفنوع بیعلامت half_t;
#تعریف_کن HALF_MAX UINT_MAX
#تعریف_کن HALF_FMT ""
#وإلاإذا ULLONG_MAX == (ULONG_MAX * ULONG_MAX + 2 * ULONG_MAX)
/* long is half as wide as ullong */
تعريفنوع بیعلامت دراز half_t;
#تعریف_کن HALF_MAX ULONG_MAX
#تعریف_کن HALF_FMT "l"
#وإلا
#خطا Cant پیدا_کن an integer type which is half as wide as ullong
#پایان_اگر

/* The trial divisor increment wheel.  Use it to skip over divisors that
 * are composites of 2, 3, 5, 7, or 11.
 * Larger wheels improve sieving only slightly, but quickly grow in size
 * (adding just one prime, 13, results in 5766 element sieve).
 */
#تعریف_کن R(a,b,c,d,e,f,g,h,i,j,A,B,C,D,E,F,G,H,I,J,x) \
	(((uint64_t)(a<<0) | (b<<3) | (c<<6) | (d<<9) | (e<<12) | (f<<15) | (g<<18) | (h<<21) | (i<<24) | (j<<27)) << 1)  | \
	(((uint64_t)(A<<0) | (B<<3) | (C<<6) | (D<<9) | (E<<12) | (F<<15) | (G<<18) | (H<<21) | (I<<24) | (J<<27)) << 31) | \
	((uint64_t)x << 61)
#تعریف_کن P(a,b,c,d,e,f,g,h,i,j,A,B,C,D,E,F,G,H,I,J,x) \
	R(	(a/2),(b/2),(c/2),(d/2),(e/2),(f/2),(g/2),(h/2),(i/2),(j/2), \
		(A/2),(B/2),(C/2),(D/2),(E/2),(F/2),(G/2),(H/2),(I/2),(J/2), \
		(x/2) \
	)
ساكن ثابت uint64_t packed_wheel[] = {
	/* 1, 2, */
	P( 2, 4, 2, 4, 2, 4, 6, 2, 6, 4, 2, 4, 6, 6, 2, 6, 4, 2, 6, 4, 6),
	P( 8, 4, 2, 4, 2, 4,14, 4, 6, 2,10, 2, 6, 6, 4, 2, 4, 6, 2,10, 2),
	P( 4, 2,12,10, 2, 4, 2, 4, 6, 2, 6, 4, 6, 6, 6, 2, 6, 4, 2, 6, 4),
	P( 6, 8, 4, 2, 4, 6, 8, 6,10, 2, 4, 6, 2, 6, 6, 4, 2, 4, 6, 2, 6),
	P( 4, 2, 6,10, 2,10, 2, 4, 2, 4, 6, 8, 4, 2, 4,12, 2, 6, 4, 2, 6),
	P( 4, 6,12, 2, 4, 2, 4, 8, 6, 4, 6, 2, 4, 6, 2, 6,10, 2, 4, 6, 2),
	P( 6, 4, 2, 4, 2,10, 2,10, 2, 4, 6, 6, 2, 6, 6, 4, 6, 6, 2, 6, 4),
	P( 2, 6, 4, 6, 8, 4, 2, 6, 4, 8, 6, 4, 6, 2, 4, 6, 8, 6, 4, 2,10),
	P( 2, 6, 4, 2, 4, 2,10, 2,10, 2, 4, 2, 4, 8, 6, 4, 2, 4, 6, 6, 2),
	P( 6, 4, 8, 4, 6, 8, 4, 2, 4, 2, 4, 8, 6, 4, 6, 6, 6, 2, 6, 6, 4),
	P( 2, 4, 6, 2, 6, 4, 2, 4, 2,10, 2,10, 2, 6, 4, 6, 2, 6, 4, 2, 4),
	P( 6, 6, 8, 4, 2, 6,10, 8, 4, 2, 4, 2, 4, 8,10, 6, 2, 4, 8, 6, 6),
	P( 4, 2, 4, 6, 2, 6, 4, 6, 2,10, 2,10, 2, 4, 2, 4, 6, 2, 6, 4, 2),
	P( 4, 6, 6, 2, 6, 6, 6, 4, 6, 8, 4, 2, 4, 2, 4, 8, 6, 4, 8, 4, 6),
	P( 2, 6, 6, 4, 2, 4, 6, 8, 4, 2, 4, 2,10, 2,10, 2, 4, 2, 4, 6, 2),
	P(10, 2, 4, 6, 8, 6, 4, 2, 6, 4, 6, 8, 4, 6, 2, 4, 8, 6, 4, 6, 2),
	P( 4, 6, 2, 6, 6, 4, 6, 6, 2, 6, 6, 4, 2,10, 2,10, 2, 4, 2, 4, 6),
	P( 2, 6, 4, 2,10, 6, 2, 6, 4, 2, 6, 4, 6, 8, 4, 2, 4, 2,12, 6, 4),
	P( 6, 2, 4, 6, 2,12, 4, 2, 4, 8, 6, 4, 2, 4, 2,10, 2,10, 6, 2, 4),
	P( 6, 2, 6, 4, 2, 4, 6, 6, 2, 6, 4, 2,10, 6, 8, 6, 4, 2, 4, 8, 6),
	P( 4, 6, 2, 4, 6, 2, 6, 6, 6, 4, 6, 2, 6, 4, 2, 4, 2,10,12, 2, 4),
	P( 2,10, 2, 6, 4, 2, 4, 6, 6, 2,10, 2, 6, 4,14, 4, 2, 4, 2, 4, 8),
	P( 6, 4, 6, 2, 4, 6, 2, 6, 6, 4, 2, 4, 6, 2, 6, 4, 2, 4,12, 2,12),
};
#لغو_تعریف P
#لغو_تعریف R
#تعریف_کن WHEEL_شروع 5
#تعریف_کن WHEEL_اندازه (5 + 24 * 20)
#تعریف_کن مربع_شمار (((uint8_t*)&bb_common_bufsiz1)[0])
#تعریف_کن wheel_tab    (((uint8_t*)&bb_common_bufsiz1) + 1)
/*
 * Why, you ask?
 * plain byte array:
 *	function                old     new   delta
 *	wheel_tab                 -     485    +485
 * 3-bit-packed insanity:
 *	packed_wheel              -     184    +184
 *	factor_main             108     163     +55
 */
ساكن فراغ unpack_wheel(فراغ)
{
	صحيح i;
	uint8_t *p;

	راهاندازی_common_bufsiz();
	wheel_tab[0] = 1;
	wheel_tab[1] = 2;
	p = &wheel_tab[2];
	لكل (i = 0; i < ARRAY_اندازه(packed_wheel); i++) {
		uint64_t v = packed_wheel[i];
		طالما ((v & 0xe) != 0) {
			*p = v & 0xe;
			//printf("%2u,", *p);
			p++;
			v >>= 3;
		}
		//printf("\n");
	}
}

/* Prevent inlining, factorize() needs all help it can get with reducing register pressure */
ساكن NOINLINE فراغ print_w(wide_t n)
{
	بیعلامت rep = مربع_شمار;
	افعل
		چاپ_کن(" %llu", n);
	طالما (--rep != 0);
}
ساكن NOINLINE فراغ print_h(half_t n)
{
	print_w(n);
}

ساكن فراغ factorize(wide_t N);

ساكن half_t isqrt_odd(wide_t N)
{
	half_t s = isqrt(N);
	/* s^2 is <= N, (s+1)^2 > N */

	/* If s^2 in fact is EQUAL to N, it's very lucky.
	 * Examples:
	 * factor 18446743988964486098 = 2 * 3037000493 * 3037000493
	 * factor 18446743902517389507 = 3 * 2479700513 * 2479700513
	 */
	إذا ((wide_t)s * s == N) {
		/* factorize sqrt(N), printing each factor twice */
		مربع_شمار *= 2;
		factorize(s);
		/* Let caller know we recursed */
		أرجع 0;
	}

	/* Subtract 1 from even s, odd s won't change: */
	/* (doesnt work for zero, but we know that s != 0 here) */
	s = (s - 1) | 1;
	أرجع s;
}

ساكن NOINLINE فراغ factorize(wide_t N)
{
	بیعلامت w;
	half_t ضریب;
	half_t max_ضریب;

	إذا (N < 4)
		اذهب انتها;

	/* The code needs to be optimized for the case where
	 * there are large prime factors. For example,
	 * this is not hard:
	 * 8262075252869367027 = 3 7 17 23 47 101 113 127 131 137 823
	 *  (the largest divisor to test for largest factor 823
	 *  is only ~sqrt(823) = 28, the entire factorization needs
	 *  only ~33 trial divisions)
	 * but this is:
	 * 18446744073709551601 = 53 348051774975651917
	 * the last factor requires testing up to
	 * 589959129 - about 100 million iterations.
	 * The slowest case (largest prime) for N < 2^64 is
	 * factor 18446744073709551557 (0xffffffffffffffc5).
	 */
	max_ضریب = isqrt_odd(N);
	إذا (!max_ضریب)
		أرجع; /* square was detected and recursively factored */
	ضریب = 2;
	w = 0;
	لكل (;;) {
		half_t fw;

		/* The division is the most costly part of the loop.
		 * On 64bit CPUs, takes at best 12 cycles, often ~20.
		 */
		طالما ((N % ضریب) == 0) { /* not likely */
			N = N / ضریب;
			print_h(ضریب);
			max_ضریب = isqrt_odd(N);
			إذا (!max_ضریب)
				أرجع; /* square was detected */
		}
		إذا (ضریب >= max_ضریب)
			اكسر;
		fw = ضریب + wheel_tab[w];
		إذا (fw < ضریب)
			اكسر; /*سرریز*/
		ضریب = fw;
		w++;
		إذا (w < WHEEL_اندازه)
			استمر;
		w = WHEEL_شروع;
	}
 انتها:
	إذا (N > 1)
		print_w(N);
	bb_putchar('\n');
}

ساكن فراغ factorize_numstr(ثابت حرف *numstr)
{
	wide_t N;

	/* Leading + is ok (coreutils compat) */
	إذا (*numstr == '+')
		numstr++;
	N = bb_strtoull(numstr, عدم, 10);
	إذا (errno)
		bb_show_usage();
	چاپ_کن("%llu:", N);
	مربع_شمار = 1;
	factorize(N);
}

صحيح ضریب_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح ضریب_main(صحيح argc استفادهنشده_PARAM, حرف **argv)
{
	unpack_wheel();

	//// coreutils has undocumented option ---debug (three dashes)
	//getopt32(argv, "");
	//argv += optind;
	argv++;

	إذا (!*argv) {
		/* Read from stdin, several numbers per line are accepted */
		لكل (;;) {
			حرف *numstr, *سطر;
			سطر = xmalloc_fgetline(stdin);
			إذا (!سطر)
				أرجع EXIT_SUCCESS;
			numstr = سطر;
			لكل (;;) {
				حرف *انتها;
				numstr = skip_whitespace(numstr);
				إذا (!numstr[0])
					اكسر;
				انتها = skip_non_whitespace(numstr);
				إذا (*انتها != '\0')
					*انتها++ = '\0';
				factorize_numstr(numstr);
				numstr = انتها;
			}
			حرر(سطر);
		}
	}

	افعل {
		/* Leading spaces are ok (coreutils compat) */
		factorize_numstr(skip_whitespace(*argv));
	} طالما (*++argv);

	أرجع EXIT_SUCCESS;
}
