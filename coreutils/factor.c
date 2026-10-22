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
# define dbg(...) bb_error_نياپو(__VA_ARGS__)
#وإلا
# define dbg(...) ((فراغ)0)
#جيڪڏهن_پورو

تعريفنوع بنانشان ڊگهو ڊگهو wide_t;

#إذا ULLONG_MAX == (UINT_MAX * UINT_MAX + 2 * UINT_MAX)
/* "unsigned" is half as wide as ullong */
تعريفنوع بنانشان half_t;
#وصف_ڪريو HALF_MAX UINT_MAX
#وصف_ڪريو HALF_FMT ""
#وإلاإذا ULLONG_MAX == (ULONG_MAX * ULONG_MAX + 2 * ULONG_MAX)
/* long is half as wide as ullong */
تعريفنوع بنانشان ڊگهو half_t;
#وصف_ڪريو HALF_MAX ULONG_MAX
#وصف_ڪريو HALF_FMT "l"
#وإلا
#غلطي Cant ڳوليو an integer type which is half as wide as ullong
#جيڪڏهن_پورو

/* The trial divisor increment wheel.  Use it to skip over divisors that
 * are composites of 2, 3, 5, 7, or 11.
 * Larger wheels improve sieving only slightly, but quickly grow in size
 * (adding just one prime, 13, results in 5766 element sieve).
 */
#وصف_ڪريو R(a,b,c,d,e,f,g,h,i,j,A,B,C,D,E,F,G,H,I,J,x) \
	(((uint64_t)(a<<0) | (b<<3) | (c<<6) | (d<<9) | (e<<12) | (f<<15) | (g<<18) | (h<<21) | (i<<24) | (j<<27)) << 1)  | \
	(((uint64_t)(A<<0) | (B<<3) | (C<<6) | (D<<9) | (E<<12) | (F<<15) | (G<<18) | (H<<21) | (I<<24) | (J<<27)) << 31) | \
	((uint64_t)x << 61)
#وصف_ڪريو P(a,b,c,d,e,f,g,h,i,j,A,B,C,D,E,F,G,H,I,J,x) \
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
#وصف_رد P
#وصف_رد R
#وصف_ڪريو WHEEL_شروعات 5
#وصف_ڪريو WHEEL_سائيز (5 + 24 * 20)
#وصف_ڪريو چورس_ڳڻپ (((uint8_t*)&bb_common_bufsiz1)[0])
#وصف_ڪريو wheel_tab    (((uint8_t*)&bb_common_bufsiz1) + 1)
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

	تياري_common_bufsiz();
	wheel_tab[0] = 1;
	wheel_tab[1] = 2;
	p = &wheel_tab[2];
	لكل (i = 0; i < ARRAY_سائيز(packed_wheel); i++) {
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
	بنانشان rep = چورس_ڳڻپ;
	افعل
		لکو(" %llu", n);
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
		چورس_ڳڻپ *= 2;
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
	بنانشان w;
	half_t ضرب_ڪندڙ;
	half_t max_ضرب_ڪندڙ;

	إذا (N < 4)
		اذهب پڇاڙي;

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
	max_ضرب_ڪندڙ = isqrt_odd(N);
	إذا (!max_ضرب_ڪندڙ)
		أرجع; /* square was detected and recursively factored */
	ضرب_ڪندڙ = 2;
	w = 0;
	لكل (;;) {
		half_t fw;

		/* The division is the most costly part of the loop.
		 * On 64bit CPUs, takes at best 12 cycles, often ~20.
		 */
		طالما ((N % ضرب_ڪندڙ) == 0) { /* not likely */
			N = N / ضرب_ڪندڙ;
			print_h(ضرب_ڪندڙ);
			max_ضرب_ڪندڙ = isqrt_odd(N);
			إذا (!max_ضرب_ڪندڙ)
				أرجع; /* square was detected */
		}
		إذا (ضرب_ڪندڙ >= max_ضرب_ڪندڙ)
			اكسر;
		fw = ضرب_ڪندڙ + wheel_tab[w];
		إذا (fw < ضرب_ڪندڙ)
			اكسر; /*حد_کان_مٿي*/
		ضرب_ڪندڙ = fw;
		w++;
		إذا (w < WHEEL_سائيز)
			استمر;
		w = WHEEL_شروعات;
	}
 پڇاڙي:
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
	لکو("%llu:", N);
	چورس_ڳڻپ = 1;
	factorize(N);
}

صحيح ضرب_ڪندڙ_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح ضرب_ڪندڙ_main(صحيح argc اڻ_استعمال_ٿيل_PARAM, حرف **argv)
{
	unpack_wheel();

	//// coreutils has undocumented option ---debug (three dashes)
	//getopt32(argv, "");
	//argv += optind;
	argv++;

	إذا (!*argv) {
		/* Read from stdin, several numbers per line are accepted */
		لكل (;;) {
			حرف *numstr, *سٽ;
			سٽ = xmalloc_fgetline(stdin);
			إذا (!سٽ)
				أرجع EXIT_SUCCESS;
			numstr = سٽ;
			لكل (;;) {
				حرف *پڇاڙي;
				numstr = skip_whitespace(numstr);
				إذا (!numstr[0])
					اكسر;
				پڇاڙي = skip_non_whitespace(numstr);
				إذا (*پڇاڙي != '\0')
					*پڇاڙي++ = '\0';
				factorize_numstr(numstr);
				numstr = پڇاڙي;
			}
			حرر(سٽ);
		}
	}

	افعل {
		/* Leading spaces are ok (coreutils compat) */
		factorize_numstr(skip_whitespace(*argv));
	} طالما (*++argv);

	أرجع EXIT_SUCCESS;
}
