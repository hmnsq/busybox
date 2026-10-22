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

#समावेश "libbb.h"
#समावेश "common_bufsiz.h"

#अगर 0
# define dbg(...) bb_error_संदेश(__VA_ARGS__)
#अन्यथा
# define dbg(...) ((व्योम)0)
#पूर्ण_अगर

प्रकार अचिन्हित दीर्घ दीर्घ wide_t;

#अगर ULLONG_MAX == (अच_पूर्णांक_उच्च * अच_पूर्णांक_उच्च + 2 * अच_पूर्णांक_उच्च)
/* "unsigned" is half as wide as ullong */
प्रकार अचिन्हित half_t;
#घोषणा HALF_MAX अच_पूर्णांक_उच्च
#घोषणा HALF_FMT ""
#या_अगर ULLONG_MAX == (अच_दीर्घ_उच्च * अच_दीर्घ_उच्च + 2 * अच_दीर्घ_उच्च)
/* long is half as wide as ullong */
प्रकार अचिन्हित दीर्घ half_t;
#घोषणा HALF_MAX अच_दीर्घ_उच्च
#घोषणा HALF_FMT "l"
#अन्यथा
#त्रुटि Cant find an integer type which is half as wide as ullong
#पूर्ण_अगर

/* The trial divisor increment wheel.  Use it to skip over divisors that
 * are composites of 2, 3, 5, 7, or 11.
 * Larger wheels improve sieving only slightly, but quickly grow in size
 * (adding just one prime, 13, results in 5766 element sieve).
 */
#घोषणा R(a,b,c,d,e,f,g,h,i,j,A,B,C,D,E,F,G,H,I,J,x) \
	(((uint64_t)(a<<0) | (b<<3) | (c<<6) | (d<<9) | (e<<12) | (f<<15) | (g<<18) | (h<<21) | (i<<24) | (j<<27)) << 1)  | \
	(((uint64_t)(A<<0) | (B<<3) | (C<<6) | (D<<9) | (E<<12) | (F<<15) | (G<<18) | (H<<21) | (I<<24) | (J<<27)) << 31) | \
	((uint64_t)x << 61)
#घोषणा P(a,b,c,d,e,f,g,h,i,j,A,B,C,D,E,F,G,H,I,J,x) \
	R(	(a/2),(b/2),(c/2),(d/2),(e/2),(f/2),(g/2),(h/2),(i/2),(j/2), \
		(A/2),(B/2),(C/2),(D/2),(E/2),(F/2),(G/2),(H/2),(I/2),(J/2), \
		(x/2) \
	)
अटल स्थिर uint64_t packed_wheel[] = {
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
#अघोषित P
#अघोषित R
#घोषणा WHEEL_START 5
#घोषणा WHEEL_आकार (5 + 24 * 20)
#घोषणा square_गिनती (((uint8_t*)&bb_common_bufsiz1)[0])
#घोषणा wheel_tab    (((uint8_t*)&bb_common_bufsiz1) + 1)
/*
 * Why, you ask?
 * plain byte array:
 *	function                old     new   delta
 *	wheel_tab                 -     485    +485
 * 3-bit-packed insanity:
 *	packed_wheel              -     184    +184
 *	factor_main             108     163     +55
 */
अटल व्योम unpack_wheel(व्योम)
{
	पूर्णांक i;
	uint8_t *p;

	setup_common_bufsiz();
	wheel_tab[0] = 1;
	wheel_tab[1] = 2;
	p = &wheel_tab[2];
	क्रम (i = 0; i < ARRAY_आकार(packed_wheel); i++) {
		uint64_t v = packed_wheel[i];
		जबतक ((v & 0xe) != 0) {
			*p = v & 0xe;
			//printf("%2u,", *p);
			p++;
			v >>= 3;
		}
		//printf("\n");
	}
}

/* Prevent inlining, factorize() needs all help it can get with reducing register pressure */
अटल NOINLINE व्योम print_w(wide_t n)
{
	अचिन्हित rep = square_गिनती;
	करो
		म_लिखो(" %llu", n);
	जबतक (--rep != 0);
}
अटल NOINLINE व्योम print_h(half_t n)
{
	print_w(n);
}

अटल व्योम factorize(wide_t N);

अटल half_t isqrt_odd(wide_t N)
{
	half_t s = isqrt(N);
	/* s^2 is <= N, (s+1)^2 > N */

	/* If s^2 in fact is EQUAL to N, it's very lucky.
	 * Examples:
	 * factor 18446743988964486098 = 2 * 3037000493 * 3037000493
	 * factor 18446743902517389507 = 3 * 2479700513 * 2479700513
	 */
	अगर ((wide_t)s * s == N) {
		/* factorize sqrt(N), printing each factor twice */
		square_गिनती *= 2;
		factorize(s);
		/* Let caller know we recursed */
		वापस 0;
	}

	/* Subtract 1 from even s, odd s won't change: */
	/* (doesnt work for zero, but we know that s != 0 here) */
	s = (s - 1) | 1;
	वापस s;
}

अटल NOINLINE व्योम factorize(wide_t N)
{
	अचिन्हित w;
	half_t factor;
	half_t max_factor;

	अगर (N < 4)
		जाओ end;

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
	अगर (!max_factor)
		वापस; /* square was detected and recursively factored */
	factor = 2;
	w = 0;
	क्रम (;;) {
		half_t fw;

		/* The division is the most costly part of the loop.
		 * On 64bit CPUs, takes at best 12 cycles, often ~20.
		 */
		जबतक ((N % factor) == 0) { /* not likely */
			N = N / factor;
			print_h(factor);
			max_factor = isqrt_odd(N);
			अगर (!max_factor)
				वापस; /* square was detected */
		}
		अगर (factor >= max_factor)
			अवरोध;
		fw = factor + wheel_tab[w];
		अगर (fw < factor)
			अवरोध; /* overflow */
		factor = fw;
		w++;
		अगर (w < WHEEL_आकार)
			जारी;
		w = WHEEL_START;
	}
 end:
	अगर (N > 1)
		print_w(N);
	bb_putchar('\n');
}

अटल व्योम factorize_numstr(स्थिर अक्षर *numstr)
{
	wide_t N;

	/* Leading + is ok (coreutils compat) */
	अगर (*numstr == '+')
		numstr++;
	N = bb_strtoull(numstr, शून्य, 10);
	अगर (त्रुटि_सं)
		bb_show_usage();
	म_लिखो("%llu:", N);
	square_गिनती = 1;
	factorize(N);
}

पूर्णांक factor_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक factor_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	unpack_wheel();

	//// coreutils has undocumented option ---debug (three dashes)
	//getopt32(argv, "");
	//argv += optind;
	argv++;

	अगर (!*argv) {
		/* Read from stdin, several numbers per line are accepted */
		क्रम (;;) {
			अक्षर *numstr, *पंक्ति;
			पंक्ति = xmalloc_fgetline(मानक_निवेश);
			अगर (!पंक्ति)
				वापस निकास_सफल;
			numstr = पंक्ति;
			क्रम (;;) {
				अक्षर *end;
				numstr = skip_whitespace(numstr);
				अगर (!numstr[0])
					अवरोध;
				end = skip_non_whitespace(numstr);
				अगर (*end != '\0')
					*end++ = '\0';
				factorize_numstr(numstr);
				numstr = end;
			}
			मुक्त(पंक्ति);
		}
	}

	करो {
		/* Leading spaces are ok (coreutils compat) */
		factorize_numstr(skip_whitespace(*argv));
	} जबतक (*++argv);

	वापस निकास_सफल;
}
