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

#అన్తర్భావ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

పూర్ణాంకం seq_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం seq_main(పూర్ణాంకం argc, అక్షరం **argv)
{
	గణన {
		OPT_w = (1 << 0),
		OPT_s = (1 << 1),
	};
	ద్విగుణ మొదటి, చివరి, increment, v;
	గుర్తులేని n;
	గుర్తులేని వెడల్పు;
	గుర్తులేని frac_part;
	స్థిరరాశి అక్షరం *sep, *opt_s = "\n";
	అక్షరం *saved;
	గుర్తులేని opt;

#ఒకవేళ ENABLE_LOCALE_SUPPORT
	/* Undo busybox.c: on input, we want to use dot
	 * as fractional separator, regardless of current locale */
	setlocale(LC_NUMERIC, "C");
#అయితే_ముగింపు

	/* Cater for negative arguments: if we see one, truncate argv[] on it */
	n = 0;
	కొరకు (;;) {
		అక్షరం c;
		saved = argv[++n];
		ఒకవేళ (!saved)
			ఆపు;
		ఒకవేళ (saved[0] != '-') {
			// break; // "seq -s : -1 1" won't be treated correctly
			కొనసాగు;
		}
// "seq -s -1 1 9" is not treated correctly, but such usage
// (delimiter string which looks like negative number) is very unlikely
		c = saved[1];
		ఒకవేళ (c == '.' || (c >= '0' && c <= '9')) {
			argv[n] = ఖాళీ;
			ఆపు;
		}
	}
	opt = getopt32(argv, "+ws:", &opt_s); /* "+": stop at first non-option */
	/* Restore possibly truncated argv[] */
	argv[n] = saved;

	argc -= optind;
	argv += optind;
	మొదటి = increment = 1;
	errno = 0;
	ఎంపిక (argc) {
			అక్షరం *pp;
		స్థితి 3:
			increment = strtod(argv[1], &pp);
			errno |= *pp;
		స్థితి 2:
			మొదటి = strtod(argv[0], &pp);
			errno |= *pp;
		స్థితి 1:
			చివరి = strtod(argv[argc-1], &pp);
			ఒకవేళ (!errno && *pp == '\0')
				ఆపు;
		అప్రమేయ:
			bb_show_usage();
	}

#ఒకవేళ ENABLE_LOCALE_SUPPORT
	setlocale(LC_NUMERIC, "");
#అయితే_ముగింపు

	/* Last checked to be compatible with: coreutils-6.10 */
	వెడల్పు = 0;
	frac_part = 0;
	అయితే (1) {
		అక్షరం *dot = strchrnul(*argv, '.');
		పూర్ణాంకం w = (dot - *argv);
		పూర్ణాంకం f = అక్షరశ్రేణి_పొడవు(dot);
		ఒకవేళ (వెడల్పు < w)
			వెడల్పు = w;
		argv++;
		ఒకవేళ (!*argv)
			ఆపు;
		/* Why do the above _before_ frac check below?
		 * Try "seq 1 2.0" and "seq 1.0 2.0":
		 * coreutils never pay attention to the number
		 * of fractional digits in last arg. */
		ఒకవేళ (frac_part < f)
			frac_part = f;
	}
	ఒకవేళ (frac_part) {
		frac_part--;
		ఒకవేళ (frac_part)
			వెడల్పు += frac_part + 1;
	}
	ఒకవేళ (!(opt & OPT_w))
		వెడల్పు = 0;

	sep = "";
	v = మొదటి;
	n = 0;
	అయితే (increment >= 0 ? v <= చివరి : v >= చివరి) {
		ఒకవేళ (రాయి("%s%0*.*f", sep, వెడల్పు, frac_part, v) < 0)
			ఆపు; /* I/O error, bail out (yes, this really happens) */
		sep = opt_s;
		/* v += increment; - would accumulate floating point errors */
		n++;
		v = మొదటి + n * increment;
	}
	ఒకవేళ (n) /* if while loop executed at least once */
		bb_putchar('\n');

	తిరిగి fflush_all();
}
