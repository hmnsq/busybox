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

#అన్తర్భావ "libbb.h"
#అన్తర్భావ "common_bufsiz.h"

#ఒకవేళ 0
# define dbg(...) bb_error_సందేశం(__VA_ARGS__)
#లేకపోతే
# define dbg(...) ((శూన్య)0)
#అయితే_ముగింపు

రకనిర్వచనం గుర్తులేని దీర్ఘ దీర్ఘ wide_t;

#ఒకవేళ ULLONG_MAX == (UINT_MAX * UINT_MAX + 2 * UINT_MAX)
/* "unsigned" is half as wide as ullong */
రకనిర్వచనం గుర్తులేని half_t;
#నిర్వచించు HALF_MAX UINT_MAX
#నిర్వచించు HALF_FMT ""
#లేదావేళ ULLONG_MAX == (ULONG_MAX * ULONG_MAX + 2 * ULONG_MAX)
/* long is half as wide as ullong */
రకనిర్వచనం గుర్తులేని దీర్ఘ half_t;
#నిర్వచించు HALF_MAX ULONG_MAX
#నిర్వచించు HALF_FMT "l"
#లేకపోతే
#దోషం Cant కనుగొను an integer type which is half as wide as ullong
#అయితే_ముగింపు

/* The trial divisor increment wheel.  Use it to skip over divisors that
 * are composites of 2, 3, 5, 7, or 11.
 * Larger wheels improve sieving only slightly, but quickly grow in size
 * (adding just one prime, 13, results in 5766 element sieve).
 */
#నిర్వచించు R(a,b,c,d,e,f,g,h,i,j,A,B,C,D,E,F,G,H,I,J,x) \
	(((uint64_t)(a<<0) | (b<<3) | (c<<6) | (d<<9) | (e<<12) | (f<<15) | (g<<18) | (h<<21) | (i<<24) | (j<<27)) << 1)  | \
	(((uint64_t)(A<<0) | (B<<3) | (C<<6) | (D<<9) | (E<<12) | (F<<15) | (G<<18) | (H<<21) | (I<<24) | (J<<27)) << 31) | \
	((uint64_t)x << 61)
#నిర్వచించు P(a,b,c,d,e,f,g,h,i,j,A,B,C,D,E,F,G,H,I,J,x) \
	R(	(a/2),(b/2),(c/2),(d/2),(e/2),(f/2),(g/2),(h/2),(i/2),(j/2), \
		(A/2),(B/2),(C/2),(D/2),(E/2),(F/2),(G/2),(H/2),(I/2),(J/2), \
		(x/2) \
	)
స్థిర స్థిరరాశి uint64_t packed_wheel[] = {
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
#నిర్వచనం_రద్దు P
#నిర్వచనం_రద్దు R
#నిర్వచించు WHEEL_ప్రారంభం 5
#నిర్వచించు WHEEL_కొలత (5 + 24 * 20)
#నిర్వచించు వర్గం_లెక్క (((uint8_t*)&bb_common_bufsiz1)[0])
#నిర్వచించు wheel_tab    (((uint8_t*)&bb_common_bufsiz1) + 1)
/*
 * Why, you ask?
 * plain byte array:
 *	function                old     new   delta
 *	wheel_tab                 -     485    +485
 * 3-bit-packed insanity:
 *	packed_wheel              -     184    +184
 *	factor_main             108     163     +55
 */
స్థిర శూన్య unpack_wheel(శూన్య)
{
	పూర్ణాంకం i;
	uint8_t *p;

	అమరిక_common_bufsiz();
	wheel_tab[0] = 1;
	wheel_tab[1] = 2;
	p = &wheel_tab[2];
	కొరకు (i = 0; i < ARRAY_కొలత(packed_wheel); i++) {
		uint64_t v = packed_wheel[i];
		అయితే ((v & 0xe) != 0) {
			*p = v & 0xe;
			//printf("%2u,", *p);
			p++;
			v >>= 3;
		}
		//printf("\n");
	}
}

/* Prevent inlining, factorize() needs all help it can get with reducing register pressure */
స్థిర NOINLINE శూన్య print_w(wide_t n)
{
	గుర్తులేని rep = వర్గం_లెక్క;
	చేయి
		రాయి(" %llu", n);
	అయితే (--rep != 0);
}
స్థిర NOINLINE శూన్య print_h(half_t n)
{
	print_w(n);
}

స్థిర శూన్య factorize(wide_t N);

స్థిర half_t isqrt_odd(wide_t N)
{
	half_t s = isqrt(N);
	/* s^2 is <= N, (s+1)^2 > N */

	/* If s^2 in fact is EQUAL to N, it's very lucky.
	 * Examples:
	 * factor 18446743988964486098 = 2 * 3037000493 * 3037000493
	 * factor 18446743902517389507 = 3 * 2479700513 * 2479700513
	 */
	ఒకవేళ ((wide_t)s * s == N) {
		/* factorize sqrt(N), printing each factor twice */
		వర్గం_లెక్క *= 2;
		factorize(s);
		/* Let caller know we recursed */
		తిరిగి 0;
	}

	/* Subtract 1 from even s, odd s won't change: */
	/* (doesnt work for zero, but we know that s != 0 here) */
	s = (s - 1) | 1;
	తిరిగి s;
}

స్థిర NOINLINE శూన్య factorize(wide_t N)
{
	గుర్తులేని w;
	half_t గుణకం;
	half_t max_గుణకం;

	ఒకవేళ (N < 4)
		వెళ్ళు చివర;

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
	max_గుణకం = isqrt_odd(N);
	ఒకవేళ (!max_గుణకం)
		తిరిగి; /* square was detected and recursively factored */
	గుణకం = 2;
	w = 0;
	కొరకు (;;) {
		half_t fw;

		/* The division is the most costly part of the loop.
		 * On 64bit CPUs, takes at best 12 cycles, often ~20.
		 */
		అయితే ((N % గుణకం) == 0) { /* not likely */
			N = N / గుణకం;
			print_h(గుణకం);
			max_గుణకం = isqrt_odd(N);
			ఒకవేళ (!max_గుణకం)
				తిరిగి; /* square was detected */
		}
		ఒకవేళ (గుణకం >= max_గుణకం)
			ఆపు;
		fw = గుణకం + wheel_tab[w];
		ఒకవేళ (fw < గుణకం)
			ఆపు; /*అధికప్రవాహం*/
		గుణకం = fw;
		w++;
		ఒకవేళ (w < WHEEL_కొలత)
			కొనసాగు;
		w = WHEEL_ప్రారంభం;
	}
 చివర:
	ఒకవేళ (N > 1)
		print_w(N);
	bb_putchar('\n');
}

స్థిర శూన్య factorize_numstr(స్థిరరాశి అక్షరం *numstr)
{
	wide_t N;

	/* Leading + is ok (coreutils compat) */
	ఒకవేళ (*numstr == '+')
		numstr++;
	N = bb_strtoull(numstr, ఖాళీ, 10);
	ఒకవేళ (errno)
		bb_show_usage();
	రాయి("%llu:", N);
	వర్గం_లెక్క = 1;
	factorize(N);
}

పూర్ణాంకం గుణకం_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం గుణకం_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	unpack_wheel();

	//// coreutils has undocumented option ---debug (three dashes)
	//getopt32(argv, "");
	//argv += optind;
	argv++;

	ఒకవేళ (!*argv) {
		/* Read from stdin, several numbers per line are accepted */
		కొరకు (;;) {
			అక్షరం *numstr, *పంక్తి;
			పంక్తి = xmalloc_fgetline(stdin);
			ఒకవేళ (!పంక్తి)
				తిరిగి EXIT_SUCCESS;
			numstr = పంక్తి;
			కొరకు (;;) {
				అక్షరం *చివర;
				numstr = skip_whitespace(numstr);
				ఒకవేళ (!numstr[0])
					ఆపు;
				చివర = skip_non_whitespace(numstr);
				ఒకవేళ (*చివర != '\0')
					*చివర++ = '\0';
				factorize_numstr(numstr);
				numstr = చివర;
			}
			విడుదల(పంక్తి);
		}
	}

	చేయి {
		/* Leading spaces are ok (coreutils compat) */
		factorize_numstr(skip_whitespace(*argv));
	} అయితే (*++argv);

	తిరిగి EXIT_SUCCESS;
}
