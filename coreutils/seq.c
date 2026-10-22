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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

ਪੂਰਨ seq_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ seq_main(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
	ਗਿਣਤੀ {
		OPT_w = (1 << 0),
		OPT_s = (1 << 1),
	};
	ਦੁੱਗਣਾ ਪਹਿਲਾ, ਆਖਰੀ, increment, v;
	ਬਿਨਾਨਿਸ਼ਾਨੀ n;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਚੌੜਾਈ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ frac_part;
	ਅਚਲ ਅੱਖਰ *sep, *opt_s = "\n";
	ਅੱਖਰ *saved;
	ਬਿਨਾਨਿਸ਼ਾਨੀ opt;

#ਜੇ ENABLE_LOCALE_SUPPORT
	/* Undo busybox.c: on input, we want to use dot
	 * as fractional separator, regardless of current locale */
	setlocale(LC_NUMERIC, "C");
#ਜੇ_ਅੰਤ

	/* Cater for negative arguments: if we see one, truncate argv[] on it */
	n = 0;
	ਲਈ (;;) {
		ਅੱਖਰ c;
		saved = argv[++n];
		ਜੇ (!saved)
			ਰੋਕੋ;
		ਜੇ (saved[0] != '-') {
			// break; // "seq -s : -1 1" won't be treated correctly
			ਜਾਰੀ;
		}
// "seq -s -1 1 9" is not treated correctly, but such usage
// (delimiter string which looks like negative number) is very unlikely
		c = saved[1];
		ਜੇ (c == '.' || (c >= '0' && c <= '9')) {
			argv[n] = ਨਿਰਾ;
			ਰੋਕੋ;
		}
	}
	opt = getopt32(argv, "+ws:", &opt_s); /* "+": stop at first non-option */
	/* Restore possibly truncated argv[] */
	argv[n] = saved;

	argc -= optind;
	argv += optind;
	ਪਹਿਲਾ = increment = 1;
	errno = 0;
	ਚੋਣ (argc) {
			ਅੱਖਰ *pp;
		ਹਾਲਤ 3:
			increment = strtod(argv[1], &pp);
			errno |= *pp;
		ਹਾਲਤ 2:
			ਪਹਿਲਾ = strtod(argv[0], &pp);
			errno |= *pp;
		ਹਾਲਤ 1:
			ਆਖਰੀ = strtod(argv[argc-1], &pp);
			ਜੇ (!errno && *pp == '\0')
				ਰੋਕੋ;
		ਮੂਲ:
			bb_show_usage();
	}

#ਜੇ ENABLE_LOCALE_SUPPORT
	setlocale(LC_NUMERIC, "");
#ਜੇ_ਅੰਤ

	/* Last checked to be compatible with: coreutils-6.10 */
	ਚੌੜਾਈ = 0;
	frac_part = 0;
	ਜਦੋਂ (1) {
		ਅੱਖਰ *dot = strchrnul(*argv, '.');
		ਪੂਰਨ w = (dot - *argv);
		ਪੂਰਨ f = ਲੜੀ_ਲੰਬਾਈ(dot);
		ਜੇ (ਚੌੜਾਈ < w)
			ਚੌੜਾਈ = w;
		argv++;
		ਜੇ (!*argv)
			ਰੋਕੋ;
		/* Why do the above _before_ frac check below?
		 * Try "seq 1 2.0" and "seq 1.0 2.0":
		 * coreutils never pay attention to the number
		 * of fractional digits in last arg. */
		ਜੇ (frac_part < f)
			frac_part = f;
	}
	ਜੇ (frac_part) {
		frac_part--;
		ਜੇ (frac_part)
			ਚੌੜਾਈ += frac_part + 1;
	}
	ਜੇ (!(opt & OPT_w))
		ਚੌੜਾਈ = 0;

	sep = "";
	v = ਪਹਿਲਾ;
	n = 0;
	ਜਦੋਂ (increment >= 0 ? v <= ਆਖਰੀ : v >= ਆਖਰੀ) {
		ਜੇ (ਲਿਖੋ("%s%0*.*f", sep, ਚੌੜਾਈ, frac_part, v) < 0)
			ਰੋਕੋ; /* I/O error, bail out (yes, this really happens) */
		sep = opt_s;
		/* v += increment; - would accumulate floating point errors */
		n++;
		v = ਪਹਿਲਾ + n * increment;
	}
	ਜੇ (n) /* if while loop executed at least once */
		bb_putchar('\n');

	ਵਾਪਸ fflush_all();
}
