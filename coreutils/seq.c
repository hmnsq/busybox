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

#அந்தர்பாவ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

முழு seq_main(முழு argc, எழுத்து **argv) MAIN_EXTERNALLY_VISIBLE;
முழு seq_main(முழு argc, எழுத்து **argv)
{
	எண்ணிக்கை {
		OPT_w = (1 << 0),
		OPT_s = (1 << 1),
	};
	இரட்டை first, last, increment, v;
	unsigned n;
	unsigned width;
	unsigned frac_part;
	மாறிலி எழுத்து *sep, *opt_s = "\n";
	எழுத்து *saved;
	unsigned opt;

#எனில் ENABLE_LOCALE_SUPPORT
	/* Undo busybox.c: on input, we want to use dot
	 * as fractional separator, regardless of current locale */
	setlocale(LC_NUMERIC, "C");
#endif

	/* Cater for negative arguments: if we see one, truncate argv[] on it */
	n = 0;
	க்கு (;;) {
		எழுத்து c;
		saved = argv[++n];
		எனில் (!saved)
			நிறுத்து;
		எனில் (saved[0] != '-') {
			// break; // "seq -s : -1 1" won't be treated correctly
			தொடர்;
		}
// "seq -s -1 1 9" is not treated correctly, but such usage
// (delimiter string which looks like negative number) is very unlikely
		c = saved[1];
		எனில் (c == '.' || (c >= '0' && c <= '9')) {
			argv[n] = வெற்று;
			நிறுத்து;
		}
	}
	opt = getopt32(argv, "+ws:", &opt_s); /* "+": stop at first non-option */
	/* Restore possibly truncated argv[] */
	argv[n] = saved;

	argc -= optind;
	argv += optind;
	first = increment = 1;
	errno = 0;
	தேர்வு (argc) {
			எழுத்து *pp;
		நிலை 3:
			increment = strtod(argv[1], &pp);
			errno |= *pp;
		நிலை 2:
			first = strtod(argv[0], &pp);
			errno |= *pp;
		நிலை 1:
			last = strtod(argv[argc-1], &pp);
			எனில் (!errno && *pp == '\0')
				நிறுத்து;
		இயல்பு:
			bb_show_usage();
	}

#எனில் ENABLE_LOCALE_SUPPORT
	setlocale(LC_NUMERIC, "");
#endif

	/* Last checked to be compatible with: coreutils-6.10 */
	width = 0;
	frac_part = 0;
	போது (1) {
		எழுத்து *dot = strchrnul(*argv, '.');
		முழு w = (dot - *argv);
		முழு f = strlen(dot);
		எனில் (width < w)
			width = w;
		argv++;
		எனில் (!*argv)
			நிறுத்து;
		/* Why do the above _before_ frac check below?
		 * Try "seq 1 2.0" and "seq 1.0 2.0":
		 * coreutils never pay attention to the number
		 * of fractional digits in last arg. */
		எனில் (frac_part < f)
			frac_part = f;
	}
	எனில் (frac_part) {
		frac_part--;
		எனில் (frac_part)
			width += frac_part + 1;
	}
	எனில் (!(opt & OPT_w))
		width = 0;

	sep = "";
	v = first;
	n = 0;
	போது (increment >= 0 ? v <= last : v >= last) {
		எனில் (printf("%s%0*.*f", sep, width, frac_part, v) < 0)
			நிறுத்து; /* I/O error, bail out (yes, this really happens) */
		sep = opt_s;
		/* v += increment; - would accumulate floating point errors */
		n++;
		v = first + n * increment;
	}
	எனில் (n) /* if while loop executed at least once */
		bb_putchar('\n');

	திரும்பு fflush_all();
}
