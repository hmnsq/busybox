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

#অন্তর্ভাব "libbb.h"
#অন্তর্ভাব "common_bufsiz.h"

#যদি 0
# define dbg(...) bb_error_বার্তা(__VA_ARGS__)
#অন্যথা
# define dbg(...) ((শূন্য)0)
#endif

প্রকারনির্দেশ unsigned long long wide_t;

#যদি ULLONG_MAX == (UINT_MAX * UINT_MAX + 2 * UINT_MAX)
/* "unsigned" is half as wide as ullong */
প্রকারনির্দেশ unsigned half_t;
#define HALF_MAX UINT_MAX
#define HALF_FMT ""
#নোচেদ্যদি ULLONG_MAX == (ULONG_MAX * ULONG_MAX + 2 * ULONG_MAX)
/* long is half as wide as ullong */
প্রকারনির্দেশ unsigned long half_t;
#define HALF_MAX ULONG_MAX
#define HALF_FMT "l"
#অন্যথা
#error Cant find an integer type which is half as wide as ullong
#endif

/* The trial divisor increment wheel.  Use it to skip over divisors that
 * are composites of 2, 3, 5, 7, or 11.
 * Larger wheels improve sieving only slightly, but quickly grow in size
 * (adding just one prime, 13, results in 5766 element sieve).
 */
#define R(a,b,c,d,e,f,g,h,i,j,A,B,C,D,E,F,G,H,I,J,x) \
	(((uint64_t)(a<<0) | (b<<3) | (c<<6) | (d<<9) | (e<<12) | (f<<15) | (g<<18) | (h<<21) | (i<<24) | (j<<27)) << 1)  | \
	(((uint64_t)(A<<0) | (B<<3) | (C<<6) | (D<<9) | (E<<12) | (F<<15) | (G<<18) | (H<<21) | (I<<24) | (J<<27)) << 31) | \
	((uint64_t)x << 61)
#define P(a,b,c,d,e,f,g,h,i,j,A,B,C,D,E,F,G,H,I,J,x) \
	R(	(a/2),(b/2),(c/2),(d/2),(e/2),(f/2),(g/2),(h/2),(i/2),(j/2), \
		(A/2),(B/2),(C/2),(D/2),(E/2),(F/2),(G/2),(H/2),(I/2),(J/2), \
		(x/2) \
	)
স্থির ধ্রুব uint64_t packed_wheel[] = {
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
#undef P
#undef R
#define WHEEL_START 5
#define WHEEL_SIZE (5 + 24 * 20)
#define square_গুনতি (((uint8_t*)&bb_common_bufsiz1)[0])
#define wheel_tab    (((uint8_t*)&bb_common_bufsiz1) + 1)
/*
 * Why, you ask?
 * plain byte array:
 *	function                old     new   delta
 *	wheel_tab                 -     485    +485
 * 3-bit-packed insanity:
 *	packed_wheel              -     184    +184
 *	factor_main             108     163     +55
 */
স্থির শূন্য unpack_wheel(শূন্য)
{
	পূর্ণাঙ্ক i;
	uint8_t *p;

	setup_common_bufsiz();
	wheel_tab[0] = 1;
	wheel_tab[1] = 2;
	p = &wheel_tab[2];
	প্রতি (i = 0; i < ARRAY_SIZE(packed_wheel); i++) {
		uint64_t v = packed_wheel[i];
		যাবত্ ((v & 0xe) != 0) {
			*p = v & 0xe;
			//printf("%2u,", *p);
			p++;
			v >>= 3;
		}
		//printf("\n");
	}
}

/* Prevent inlining, factorize() needs all help it can get with reducing register pressure */
স্থির NOINLINE শূন্য print_w(wide_t n)
{
	unsigned rep = square_গুনতি;
	কুরু
		printf(" %llu", n);
	যাবত্ (--rep != 0);
}
স্থির NOINLINE শূন্য print_h(half_t n)
{
	print_w(n);
}

স্থির শূন্য factorize(wide_t N);

স্থির half_t isqrt_odd(wide_t N)
{
	half_t s = isqrt(N);
	/* s^2 is <= N, (s+1)^2 > N */

	/* If s^2 in fact is EQUAL to N, it's very lucky.
	 * Examples:
	 * factor 18446743988964486098 = 2 * 3037000493 * 3037000493
	 * factor 18446743902517389507 = 3 * 2479700513 * 2479700513
	 */
	যদি ((wide_t)s * s == N) {
		/* factorize sqrt(N), printing each factor twice */
		square_গুনতি *= 2;
		factorize(s);
		/* Let caller know we recursed */
		প্রত্যাগম 0;
	}

	/* Subtract 1 from even s, odd s won't change: */
	/* (doesnt work for zero, but we know that s != 0 here) */
	s = (s - 1) | 1;
	প্রত্যাগম s;
}

স্থির NOINLINE শূন্য factorize(wide_t N)
{
	unsigned w;
	half_t factor;
	half_t max_factor;

	যদি (N < 4)
		গচ্ছ end;

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
	max_factor = isqrt_odd(N);
	যদি (!max_factor)
		প্রত্যাগম; /* square was detected and recursively factored */
	factor = 2;
	w = 0;
	প্রতি (;;) {
		half_t fw;

		/* The division is the most costly part of the loop.
		 * On 64bit CPUs, takes at best 12 cycles, often ~20.
		 */
		যাবত্ ((N % factor) == 0) { /* not likely */
			N = N / factor;
			print_h(factor);
			max_factor = isqrt_odd(N);
			যদি (!max_factor)
				প্রত্যাগম; /* square was detected */
		}
		যদি (factor >= max_factor)
			ভঙ্গ;
		fw = factor + wheel_tab[w];
		যদি (fw < factor)
			ভঙ্গ; /* overflow */
		factor = fw;
		w++;
		যদি (w < WHEEL_SIZE)
			অনুবর্ত;
		w = WHEEL_START;
	}
 end:
	যদি (N > 1)
		print_w(N);
	bb_putchar('\n');
}

স্থির শূন্য factorize_numstr(ধ্রুব বর্ণ *numstr)
{
	wide_t N;

	/* Leading + is ok (coreutils compat) */
	যদি (*numstr == '+')
		numstr++;
	N = bb_strtoull(numstr, রিক্ত, 10);
	যদি (errno)
		bb_show_usage();
	printf("%llu:", N);
	square_গুনতি = 1;
	factorize(N);
}

পূর্ণাঙ্ক factor_main(পূর্ণাঙ্ক argc, বর্ণ **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণাঙ্ক factor_main(পূর্ণাঙ্ক argc UNUSED_PARAM, বর্ণ **argv)
{
	unpack_wheel();

	//// coreutils has undocumented option ---debug (three dashes)
	//getopt32(argv, "");
	//argv += optind;
	argv++;

	যদি (!*argv) {
		/* Read from stdin, several numbers per line are accepted */
		প্রতি (;;) {
			বর্ণ *numstr, *সারি;
			সারি = xmalloc_fgetline(stdin);
			যদি (!সারি)
				প্রত্যাগম EXIT_SUCCESS;
			numstr = সারি;
			প্রতি (;;) {
				বর্ণ *end;
				numstr = skip_whitespace(numstr);
				যদি (!numstr[0])
					ভঙ্গ;
				end = skip_non_whitespace(numstr);
				যদি (*end != '\0')
					*end++ = '\0';
				factorize_numstr(numstr);
				numstr = end;
			}
			মুক্ত(সারি);
		}
	}

	কুরু {
		/* Leading spaces are ok (coreutils compat) */
		factorize_numstr(skip_whitespace(*argv));
	} যাবত্ (*++argv);

	প্রত্যাগম EXIT_SUCCESS;
}
