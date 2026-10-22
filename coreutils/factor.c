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

#ಅನ್ತರ್ಭಾವ "libbb.h"
#ಅನ್ತರ್ಭಾವ "common_bufsiz.h"

#ಆದರೆ 0
# define dbg(...) bb_error_ಸಂದೇಶ(__VA_ARGS__)
#ಇಲ್ಲದಿದ್ದರೆ
# define dbg(...) ((ಶೂನ್ಯ)0)
#ಆದರೆ_ಅಂತ್ಯ

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ wide_t;

#ಆದರೆ ULLONG_MAX == (UINT_MAX * UINT_MAX + 2 * UINT_MAX)
/* "unsigned" is half as wide as ullong */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ half_t;
#ವ್ಯಾಖ್ಯಾನಿಸು HALF_MAX UINT_MAX
#ವ್ಯಾಖ್ಯಾನಿಸು HALF_FMT ""
#ಅಥವಾಆದರೆ ULLONG_MAX == (ULONG_MAX * ULONG_MAX + 2 * ULONG_MAX)
/* long is half as wide as ullong */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ half_t;
#ವ್ಯಾಖ್ಯಾನಿಸು HALF_MAX ULONG_MAX
#ವ್ಯಾಖ್ಯಾನಿಸು HALF_FMT "l"
#ಇಲ್ಲದಿದ್ದರೆ
#ದೋಷ Cant ಕಂಡುಹಿಡಿ an integer type which is half as wide as ullong
#ಆದರೆ_ಅಂತ್ಯ

/* The trial divisor increment wheel.  Use it to skip over divisors that
 * are composites of 2, 3, 5, 7, or 11.
 * Larger wheels improve sieving only slightly, but quickly grow in size
 * (adding just one prime, 13, results in 5766 element sieve).
 */
#ವ್ಯಾಖ್ಯಾನಿಸು R(a,b,c,d,e,f,g,h,i,j,A,B,C,D,E,F,G,H,I,J,x) \
	(((uint64_t)(a<<0) | (b<<3) | (c<<6) | (d<<9) | (e<<12) | (f<<15) | (g<<18) | (h<<21) | (i<<24) | (j<<27)) << 1)  | \
	(((uint64_t)(A<<0) | (B<<3) | (C<<6) | (D<<9) | (E<<12) | (F<<15) | (G<<18) | (H<<21) | (I<<24) | (J<<27)) << 31) | \
	((uint64_t)x << 61)
#ವ್ಯಾಖ್ಯಾನಿಸು P(a,b,c,d,e,f,g,h,i,j,A,B,C,D,E,F,G,H,I,J,x) \
	R(	(a/2),(b/2),(c/2),(d/2),(e/2),(f/2),(g/2),(h/2),(i/2),(j/2), \
		(A/2),(B/2),(C/2),(D/2),(E/2),(F/2),(G/2),(H/2),(I/2),(J/2), \
		(x/2) \
	)
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ uint64_t packed_wheel[] = {
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
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು P
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು R
#ವ್ಯಾಖ್ಯಾನಿಸು WHEEL_ಆರಂಭ 5
#ವ್ಯಾಖ್ಯಾನಿಸು WHEEL_ಗಾತ್ರ (5 + 24 * 20)
#ವ್ಯಾಖ್ಯಾನಿಸು ವರ್ಗಫಲ_ಎಣಿಕೆ (((uint8_t*)&bb_common_bufsiz1)[0])
#ವ್ಯಾಖ್ಯಾನಿಸು wheel_tab    (((uint8_t*)&bb_common_bufsiz1) + 1)
/*
 * Why, you ask?
 * plain byte array:
 *	function                old     new   delta
 *	wheel_tab                 -     485    +485
 * 3-bit-packed insanity:
 *	packed_wheel              -     184    +184
 *	factor_main             108     163     +55
 */
ಸ್ಥಿರ ಶೂನ್ಯ unpack_wheel(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ i;
	uint8_t *p;

	ಸಿದ್ಧತೆ_common_bufsiz();
	wheel_tab[0] = 1;
	wheel_tab[1] = 2;
	p = &wheel_tab[2];
	ಗಾಗಿ (i = 0; i < ARRAY_ಗಾತ್ರ(packed_wheel); i++) {
		uint64_t v = packed_wheel[i];
		ಆಗ ((v & 0xe) != 0) {
			*p = v & 0xe;
			//printf("%2u,", *p);
			p++;
			v >>= 3;
		}
		//printf("\n");
	}
}

/* Prevent inlining, factorize() needs all help it can get with reducing register pressure */
ಸ್ಥಿರ NOINLINE ಶೂನ್ಯ print_w(wide_t n)
{
	ಚಿಹ್ನೆರಹಿತ rep = ವರ್ಗಫಲ_ಎಣಿಕೆ;
	ಮಾಡು
		ಬರೆ(" %llu", n);
	ಆಗ (--rep != 0);
}
ಸ್ಥಿರ NOINLINE ಶೂನ್ಯ print_h(half_t n)
{
	print_w(n);
}

ಸ್ಥಿರ ಶೂನ್ಯ factorize(wide_t N);

ಸ್ಥಿರ half_t isqrt_odd(wide_t N)
{
	half_t s = isqrt(N);
	/* s^2 is <= N, (s+1)^2 > N */

	/* If s^2 in fact is EQUAL to N, it's very lucky.
	 * Examples:
	 * factor 18446743988964486098 = 2 * 3037000493 * 3037000493
	 * factor 18446743902517389507 = 3 * 2479700513 * 2479700513
	 */
	ಆದರೆ ((wide_t)s * s == N) {
		/* factorize sqrt(N), printing each factor twice */
		ವರ್ಗಫಲ_ಎಣಿಕೆ *= 2;
		factorize(s);
		/* Let caller know we recursed */
		ಹಿಂತಿರುಗಿ 0;
	}

	/* Subtract 1 from even s, odd s won't change: */
	/* (doesnt work for zero, but we know that s != 0 here) */
	s = (s - 1) | 1;
	ಹಿಂತಿರುಗಿ s;
}

ಸ್ಥಿರ NOINLINE ಶೂನ್ಯ factorize(wide_t N)
{
	ಚಿಹ್ನೆರಹಿತ w;
	half_t ಗುಣಕ;
	half_t max_ಗುಣಕ;

	ಆದರೆ (N < 4)
		ಹೋಗು ಕೊನೆ;

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
	max_ಗುಣಕ = isqrt_odd(N);
	ಆದರೆ (!max_ಗುಣಕ)
		ಹಿಂತಿರುಗಿ; /* square was detected and recursively factored */
	ಗುಣಕ = 2;
	w = 0;
	ಗಾಗಿ (;;) {
		half_t fw;

		/* The division is the most costly part of the loop.
		 * On 64bit CPUs, takes at best 12 cycles, often ~20.
		 */
		ಆಗ ((N % ಗುಣಕ) == 0) { /* not likely */
			N = N / ಗುಣಕ;
			print_h(ಗುಣಕ);
			max_ಗುಣಕ = isqrt_odd(N);
			ಆದರೆ (!max_ಗುಣಕ)
				ಹಿಂತಿರುಗಿ; /* square was detected */
		}
		ಆದರೆ (ಗುಣಕ >= max_ಗುಣಕ)
			ನಿಲ್ಲಿಸು;
		fw = ಗುಣಕ + wheel_tab[w];
		ಆದರೆ (fw < ಗುಣಕ)
			ನಿಲ್ಲಿಸು; /*ಅತಿಪ್ರವಾಹ*/
		ಗುಣಕ = fw;
		w++;
		ಆದರೆ (w < WHEEL_ಗಾತ್ರ)
			ಮುಂದುವರಿ;
		w = WHEEL_ಆರಂಭ;
	}
 ಕೊನೆ:
	ಆದರೆ (N > 1)
		print_w(N);
	bb_putchar('\n');
}

ಸ್ಥಿರ ಶೂನ್ಯ factorize_numstr(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *numstr)
{
	wide_t N;

	/* Leading + is ok (coreutils compat) */
	ಆದರೆ (*numstr == '+')
		numstr++;
	N = bb_strtoull(numstr, ಖಾಲಿ, 10);
	ಆದರೆ (errno)
		bb_show_usage();
	ಬರೆ("%llu:", N);
	ವರ್ಗಫಲ_ಎಣಿಕೆ = 1;
	factorize(N);
}

ಪೂರ್ಣಾಂಕ ಗುಣಕ_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ ಗುಣಕ_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	unpack_wheel();

	//// coreutils has undocumented option ---debug (three dashes)
	//getopt32(argv, "");
	//argv += optind;
	argv++;

	ಆದರೆ (!*argv) {
		/* Read from stdin, several numbers per line are accepted */
		ಗಾಗಿ (;;) {
			ಅಕ್ಷರ *numstr, *ಸಾಲು;
			ಸಾಲು = xmalloc_fgetline(stdin);
			ಆದರೆ (!ಸಾಲು)
				ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
			numstr = ಸಾಲು;
			ಗಾಗಿ (;;) {
				ಅಕ್ಷರ *ಕೊನೆ;
				numstr = skip_whitespace(numstr);
				ಆದರೆ (!numstr[0])
					ನಿಲ್ಲಿಸು;
				ಕೊನೆ = skip_non_whitespace(numstr);
				ಆದರೆ (*ಕೊನೆ != '\0')
					*ಕೊನೆ++ = '\0';
				factorize_numstr(numstr);
				numstr = ಕೊನೆ;
			}
			ಬಿಡುಗಡೆ(ಸಾಲು);
		}
	}

	ಮಾಡು {
		/* Leading spaces are ok (coreutils compat) */
		factorize_numstr(skip_whitespace(*argv));
	} ಆಗ (*++argv);

	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}
