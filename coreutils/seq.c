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

#अन्तर्भाव "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

पूर्णाङ्क seq_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क seq_main(पूर्णाङ्क argc, वर्ण **argv)
{
	गणना {
		OPT_w = (1 << 0),
		OPT_s = (1 << 1),
	};
	द्विगुण first, last, increment, v;
	unsigned n;
	unsigned width;
	unsigned frac_part;
	ध्रुव वर्ण *sep, *opt_s = "\n";
	वर्ण *saved;
	unsigned opt;

#यदि ENABLE_LOCALE_SUPPORT
	/* Undo busybox.c: on input, we want to use dot
	 * as fractional separator, regardless of current locale */
	setlocale(LC_NUMERIC, "C");
#endif

	/* Cater for negative arguments: if we see one, truncate argv[] on it */
	n = 0;
	प्रति (;;) {
		वर्ण c;
		saved = argv[++n];
		यदि (!saved)
			भङ्ग;
		यदि (saved[0] != '-') {
			// break; // "seq -s : -1 1" won't be treated correctly
			अनुवर्त;
		}
// "seq -s -1 1 9" is not treated correctly, but such usage
// (delimiter string which looks like negative number) is very unlikely
		c = saved[1];
		यदि (c == '.' || (c >= '0' && c <= '9')) {
			argv[n] = रिक्त;
			भङ्ग;
		}
	}
	opt = getopt32(argv, "+ws:", &opt_s); /* "+": stop at first non-option */
	/* Restore possibly truncated argv[] */
	argv[n] = saved;

	argc -= optind;
	argv += optind;
	first = increment = 1;
	errno = 0;
	विकल्प (argc) {
			वर्ण *pp;
		पक्ष 3:
			increment = strtod(argv[1], &pp);
			errno |= *pp;
		पक्ष 2:
			first = strtod(argv[0], &pp);
			errno |= *pp;
		पक्ष 1:
			last = strtod(argv[argc-1], &pp);
			यदि (!errno && *pp == '\0')
				भङ्ग;
		स्वभाव:
			bb_show_usage();
	}

#यदि ENABLE_LOCALE_SUPPORT
	setlocale(LC_NUMERIC, "");
#endif

	/* Last checked to be compatible with: coreutils-6.10 */
	width = 0;
	frac_part = 0;
	यावत् (1) {
		वर्ण *dot = strchrnul(*argv, '.');
		पूर्णाङ्क w = (dot - *argv);
		पूर्णाङ्क f = strlen(dot);
		यदि (width < w)
			width = w;
		argv++;
		यदि (!*argv)
			भङ्ग;
		/* Why do the above _before_ frac check below?
		 * Try "seq 1 2.0" and "seq 1.0 2.0":
		 * coreutils never pay attention to the number
		 * of fractional digits in last arg. */
		यदि (frac_part < f)
			frac_part = f;
	}
	यदि (frac_part) {
		frac_part--;
		यदि (frac_part)
			width += frac_part + 1;
	}
	यदि (!(opt & OPT_w))
		width = 0;

	sep = "";
	v = first;
	n = 0;
	यावत् (increment >= 0 ? v <= last : v >= last) {
		यदि (printf("%s%0*.*f", sep, width, frac_part, v) < 0)
			भङ्ग; /* I/O error, bail out (yes, this really happens) */
		sep = opt_s;
		/* v += increment; - would accumulate floating point errors */
		n++;
		v = first + n * increment;
	}
	यदि (n) /* if while loop executed at least once */
		bb_putchar('\n');

	प्रत्यागम fflush_all();
}
