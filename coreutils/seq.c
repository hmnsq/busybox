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

ಪೂರ್ಣಾಂಕ seq_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ seq_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
	ಎಣಿಕೆ {
		OPT_w = (1 << 0),
		OPT_s = (1 << 1),
	};
	ದ್ವಿಗುಣ ಮೊದಲ, ಕೊನೆಯ, increment, v;
	ಚಿಹ್ನೆರಹಿತ n;
	ಚಿಹ್ನೆರಹಿತ ಅಗಲ;
	ಚಿಹ್ನೆರಹಿತ frac_part;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sep, *opt_s = "\n";
	ಅಕ್ಷರ *saved;
	ಚಿಹ್ನೆರಹಿತ opt;

#ಆದರೆ ENABLE_LOCALE_SUPPORT
	/* Undo busybox.c: on input, we want to use dot
	 * as fractional separator, regardless of current locale */
	setlocale(LC_NUMERIC, "C");
#ಆದರೆ_ಅಂತ್ಯ

	/* Cater for negative arguments: if we see one, truncate argv[] on it */
	n = 0;
	ಗಾಗಿ (;;) {
		ಅಕ್ಷರ c;
		saved = argv[++n];
		ಆದರೆ (!saved)
			ನಿಲ್ಲಿಸು;
		ಆದರೆ (saved[0] != '-') {
			// break; // "seq -s : -1 1" won't be treated correctly
			ಮುಂದುವರಿ;
		}
// "seq -s -1 1 9" is not treated correctly, but such usage
// (delimiter string which looks like negative number) is very unlikely
		c = saved[1];
		ಆದರೆ (c == '.' || (c >= '0' && c <= '9')) {
			argv[n] = ಖಾಲಿ;
			ನಿಲ್ಲಿಸು;
		}
	}
	opt = getopt32(argv, "+ws:", &opt_s); /* "+": stop at first non-option */
	/* Restore possibly truncated argv[] */
	argv[n] = saved;

	argc -= optind;
	argv += optind;
	ಮೊದಲ = increment = 1;
	errno = 0;
	ಆಯ್ಕೆ (argc) {
			ಅಕ್ಷರ *pp;
		ಸ್ಥಿತಿ 3:
			increment = strtod(argv[1], &pp);
			errno |= *pp;
		ಸ್ಥಿತಿ 2:
			ಮೊದಲ = strtod(argv[0], &pp);
			errno |= *pp;
		ಸ್ಥಿತಿ 1:
			ಕೊನೆಯ = strtod(argv[argc-1], &pp);
			ಆದರೆ (!errno && *pp == '\0')
				ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			bb_show_usage();
	}

#ಆದರೆ ENABLE_LOCALE_SUPPORT
	setlocale(LC_NUMERIC, "");
#ಆದರೆ_ಅಂತ್ಯ

	/* Last checked to be compatible with: coreutils-6.10 */
	ಅಗಲ = 0;
	frac_part = 0;
	ಆಗ (1) {
		ಅಕ್ಷರ *dot = strchrnul(*argv, '.');
		ಪೂರ್ಣಾಂಕ w = (dot - *argv);
		ಪೂರ್ಣಾಂಕ f = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(dot);
		ಆದರೆ (ಅಗಲ < w)
			ಅಗಲ = w;
		argv++;
		ಆದರೆ (!*argv)
			ನಿಲ್ಲಿಸು;
		/* Why do the above _before_ frac check below?
		 * Try "seq 1 2.0" and "seq 1.0 2.0":
		 * coreutils never pay attention to the number
		 * of fractional digits in last arg. */
		ಆದರೆ (frac_part < f)
			frac_part = f;
	}
	ಆದರೆ (frac_part) {
		frac_part--;
		ಆದರೆ (frac_part)
			ಅಗಲ += frac_part + 1;
	}
	ಆದರೆ (!(opt & OPT_w))
		ಅಗಲ = 0;

	sep = "";
	v = ಮೊದಲ;
	n = 0;
	ಆಗ (increment >= 0 ? v <= ಕೊನೆಯ : v >= ಕೊನೆಯ) {
		ಆದರೆ (ಬರೆ("%s%0*.*f", sep, ಅಗಲ, frac_part, v) < 0)
			ನಿಲ್ಲಿಸು; /* I/O error, bail out (yes, this really happens) */
		sep = opt_s;
		/* v += increment; - would accumulate floating point errors */
		n++;
		v = ಮೊದಲ + n * increment;
	}
	ಆದರೆ (n) /* if while loop executed at least once */
		bb_putchar('\n');

	ಹಿಂತಿರುಗಿ fflush_all();
}
