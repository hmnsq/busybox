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

#ಅನ್ತರ್ಭಾವ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

ಪೂರ್ಣಾಙ್ಕ seq_main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಙ್ಕ seq_main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv)
{
	ಗಣನಾ {
		OPT_w = (1 << 0),
		OPT_s = (1 << 1),
	};
	ದ್ವಿಗುಣ first, last, increment, v;
	unsigned n;
	unsigned width;
	unsigned frac_part;
	ಧ್ರುವ ವರ್ಣ *sep, *opt_s = "\n";
	ವರ್ಣ *saved;
	unsigned opt;

#ಯದಿ ENABLE_LOCALE_SUPPORT
	/* Undo busybox.c: on input, we want to use dot
	 * as fractional separator, regardless of current locale */
	setlocale(LC_NUMERIC, "C");
#endif

	/* Cater for negative arguments: if we see one, truncate argv[] on it */
	n = 0;
	ಪ್ರತಿ (;;) {
		ವರ್ಣ c;
		saved = argv[++n];
		ಯದಿ (!saved)
			ಭಙ್ಗ;
		ಯದಿ (saved[0] != '-') {
			// break; // "seq -s : -1 1" won't be treated correctly
			ಅನುವರ್ತ;
		}
// "seq -s -1 1 9" is not treated correctly, but such usage
// (delimiter string which looks like negative number) is very unlikely
		c = saved[1];
		ಯದಿ (c == '.' || (c >= '0' && c <= '9')) {
			argv[n] = ರಿಕ್ತ;
			ಭಙ್ಗ;
		}
	}
	opt = getopt32(argv, "+ws:", &opt_s); /* "+": stop at first non-option */
	/* Restore possibly truncated argv[] */
	argv[n] = saved;

	argc -= optind;
	argv += optind;
	first = increment = 1;
	errno = 0;
	ವಿಕಲ್ಪ (argc) {
			ವರ್ಣ *pp;
		ಪಕ್ಷ 3:
			increment = strtod(argv[1], &pp);
			errno |= *pp;
		ಪಕ್ಷ 2:
			first = strtod(argv[0], &pp);
			errno |= *pp;
		ಪಕ್ಷ 1:
			last = strtod(argv[argc-1], &pp);
			ಯದಿ (!errno && *pp == '\0')
				ಭಙ್ಗ;
		ಸ್ವಭಾವ:
			bb_show_usage();
	}

#ಯದಿ ENABLE_LOCALE_SUPPORT
	setlocale(LC_NUMERIC, "");
#endif

	/* Last checked to be compatible with: coreutils-6.10 */
	width = 0;
	frac_part = 0;
	ಯಾವತ್ (1) {
		ವರ್ಣ *dot = strchrnul(*argv, '.');
		ಪೂರ್ಣಾಙ್ಕ w = (dot - *argv);
		ಪೂರ್ಣಾಙ್ಕ f = strlen(dot);
		ಯದಿ (width < w)
			width = w;
		argv++;
		ಯದಿ (!*argv)
			ಭಙ್ಗ;
		/* Why do the above _before_ frac check below?
		 * Try "seq 1 2.0" and "seq 1.0 2.0":
		 * coreutils never pay attention to the number
		 * of fractional digits in last arg. */
		ಯದಿ (frac_part < f)
			frac_part = f;
	}
	ಯದಿ (frac_part) {
		frac_part--;
		ಯದಿ (frac_part)
			width += frac_part + 1;
	}
	ಯದಿ (!(opt & OPT_w))
		width = 0;

	sep = "";
	v = first;
	n = 0;
	ಯಾವತ್ (increment >= 0 ? v <= last : v >= last) {
		ಯದಿ (printf("%s%0*.*f", sep, width, frac_part, v) < 0)
			ಭಙ್ಗ; /* I/O error, bail out (yes, this really happens) */
		sep = opt_s;
		/* v += increment; - would accumulate floating point errors */
		n++;
		v = first + n * increment;
	}
	ಯದಿ (n) /* if while loop executed at least once */
		bb_putchar('\n');

	ಪ್ರತ್ಯಾಗಮ fflush_all();
}
