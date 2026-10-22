/* vi: set sw=4 ts=4: */
/*
 * seq implementation for busybox
 *
 * Copyright (C) 2004, Glenn McGrath
 *
 * Licensed under GPLv2, see file LICENSE in this source tree.
 */
//config:config SEQ
//config:	bool "seq (4 kb)"
//config:	default y
//config:	help
//config:	print a sequence of numbers

//applet:IF_SEQ(APPLET_NOEXEC(seq, seq, BB_DIR_USR_BIN, BB_SUID_DROP, seq))
/* was NOFORK, but then "seq 1 999999999" can't be ^C'ed if run by hush */

//kbuild:lib-$(CONFIG_SEQ) += seq.o

//usage:#define seq_trivial_usage
//usage:       "[-w] [-s SEP] [FIRST [INC]] LAST"
//usage:#define seq_full_usage "\n\n"
//usage:       "Print numbers from FIRST to LAST, in steps of INC.\n"
//usage:       "FIRST, INC default to 1.\n"
//usage:     "\n	-w	Pad with leading zeros"
//usage:     "\n	-s SEP	String separator"

#ଅନ୍ତର୍ଭାଵ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା seq_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv) MAIN_EXTERNALLY_VISIBLE;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା seq_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv)
{
	ଗଣନା {
		OPT_w = (1 << 0),
		OPT_s = (1 << 1),
	};
	ଦ୍ୱିଗୁଣ first, last, increment, v;
	unsigned n;
	unsigned width;
	unsigned frac_part;
	ଅଚଳ ଅକ୍ଷର *sep, *opt_s = "\n";
	ଅକ୍ଷର *saved;
	unsigned opt;

#ଯଦି ENABLE_LOCALE_SUPPORT
	/* Undo busybox.c: on input, we want to use dot
	 * as fractional separator, regardless of current locale */
	setlocale(LC_NUMERIC, "C");
#endif

	/* Cater for negative arguments: if we see one, truncate argv[] on it */
	n = 0;
	ପାଇଁ (;;) {
		ଅକ୍ଷର c;
		saved = argv[++n];
		ଯଦି (!saved)
			ଭାଙ୍ଗ;
		ଯଦି (saved[0] != '-') {
			// break; // "seq -s : -1 1" won't be treated correctly
			ଜାରି;
		}
// "seq -s -1 1 9" is not treated correctly, but such usage
// (delimiter string which looks like negative number) is very unlikely
		c = saved[1];
		ଯଦି (c == '.' || (c >= '0' && c <= '9')) {
			argv[n] = ଖାଲି;
			ଭାଙ୍ଗ;
		}
	}
	opt = getopt32(argv, "+ws:", &opt_s); /* "+": stop at first non-option */
	/* Restore possibly truncated argv[] */
	argv[n] = saved;

	argc -= optind;
	argv += optind;
	first = increment = 1;
	errno = 0;
	ବାଛ (argc) {
			ଅକ୍ଷର *pp;
		ସ୍ଥିତି 3:
			increment = strtod(argv[1], &pp);
			errno |= *pp;
		ସ୍ଥିତି 2:
			first = strtod(argv[0], &pp);
			errno |= *pp;
		ସ୍ଥିତି 1:
			last = strtod(argv[argc-1], &pp);
			ଯଦି (!errno && *pp == '\0')
				ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			bb_show_usage();
	}

#ଯଦି ENABLE_LOCALE_SUPPORT
	setlocale(LC_NUMERIC, "");
#endif

	/* Last checked to be compatible with: coreutils-6.10 */
	width = 0;
	frac_part = 0;
	ଯେତେବେଳେ (1) {
		ଅକ୍ଷର *dot = strchrnul(*argv, '.');
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା w = (dot - *argv);
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା f = strlen(dot);
		ଯଦି (width < w)
			width = w;
		argv++;
		ଯଦି (!*argv)
			ଭାଙ୍ଗ;
		/* Why do the above _before_ frac check below?
		 * Try "seq 1 2.0" and "seq 1.0 2.0":
		 * coreutils never pay attention to the number
		 * of fractional digits in last arg. */
		ଯଦି (frac_part < f)
			frac_part = f;
	}
	ଯଦି (frac_part) {
		frac_part--;
		ଯଦି (frac_part)
			width += frac_part + 1;
	}
	ଯଦି (!(opt & OPT_w))
		width = 0;

	sep = "";
	v = first;
	n = 0;
	ଯେତେବେଳେ (increment >= 0 ? v <= last : v >= last) {
		ଯଦି (printf("%s%0*.*f", sep, width, frac_part, v) < 0)
			ଭାଙ୍ଗ; /* I/O error, bail out (yes, this really happens) */
		sep = opt_s;
		/* v += increment; - would accumulate floating point errors */
		n++;
		v = first + n * increment;
	}
	ଯଦି (n) /* if while loop executed at least once */
		bb_putchar('\n');

	ଫେରନ୍ତୁ fflush_all();
}
