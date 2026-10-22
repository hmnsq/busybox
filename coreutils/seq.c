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

#അന്തര്ഭാവ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

പൂർണ്ണസംഖ്യ seq_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) MAIN_EXTERNALLY_VISIBLE;
പൂർണ്ണസംഖ്യ seq_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv)
{
	എണ്ണൽ {
		OPT_w = (1 << 0),
		OPT_s = (1 << 1),
	};
	ഇരട്ട first, last, increment, v;
	unsigned n;
	unsigned width;
	unsigned frac_part;
	സ്ഥിരം അക്ഷരം *sep, *opt_s = "\n";
	അക്ഷരം *saved;
	unsigned opt;

#എങ്കിൽ ENABLE_LOCALE_SUPPORT
	/* Undo busybox.c: on input, we want to use dot
	 * as fractional separator, regardless of current locale */
	setlocale(LC_NUMERIC, "C");
#endif

	/* Cater for negative arguments: if we see one, truncate argv[] on it */
	n = 0;
	വേണ്ടി (;;) {
		അക്ഷരം c;
		saved = argv[++n];
		എങ്കിൽ (!saved)
			നിർത്തുക;
		എങ്കിൽ (saved[0] != '-') {
			// break; // "seq -s : -1 1" won't be treated correctly
			തുടരുക;
		}
// "seq -s -1 1 9" is not treated correctly, but such usage
// (delimiter string which looks like negative number) is very unlikely
		c = saved[1];
		എങ്കിൽ (c == '.' || (c >= '0' && c <= '9')) {
			argv[n] = ശൂന്യ;
			നിർത്തുക;
		}
	}
	opt = getopt32(argv, "+ws:", &opt_s); /* "+": stop at first non-option */
	/* Restore possibly truncated argv[] */
	argv[n] = saved;

	argc -= optind;
	argv += optind;
	first = increment = 1;
	errno = 0;
	തിരഞ്ഞെടുപ്പ് (argc) {
			അക്ഷരം *pp;
		അവസ്ഥ 3:
			increment = strtod(argv[1], &pp);
			errno |= *pp;
		അവസ്ഥ 2:
			first = strtod(argv[0], &pp);
			errno |= *pp;
		അവസ്ഥ 1:
			last = strtod(argv[argc-1], &pp);
			എങ്കിൽ (!errno && *pp == '\0')
				നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
			bb_show_usage();
	}

#എങ്കിൽ ENABLE_LOCALE_SUPPORT
	setlocale(LC_NUMERIC, "");
#endif

	/* Last checked to be compatible with: coreutils-6.10 */
	width = 0;
	frac_part = 0;
	ആകുമ്പോൾ (1) {
		അക്ഷരം *dot = strchrnul(*argv, '.');
		പൂർണ്ണസംഖ്യ w = (dot - *argv);
		പൂർണ്ണസംഖ്യ f = strlen(dot);
		എങ്കിൽ (width < w)
			width = w;
		argv++;
		എങ്കിൽ (!*argv)
			നിർത്തുക;
		/* Why do the above _before_ frac check below?
		 * Try "seq 1 2.0" and "seq 1.0 2.0":
		 * coreutils never pay attention to the number
		 * of fractional digits in last arg. */
		എങ്കിൽ (frac_part < f)
			frac_part = f;
	}
	എങ്കിൽ (frac_part) {
		frac_part--;
		എങ്കിൽ (frac_part)
			width += frac_part + 1;
	}
	എങ്കിൽ (!(opt & OPT_w))
		width = 0;

	sep = "";
	v = first;
	n = 0;
	ആകുമ്പോൾ (increment >= 0 ? v <= last : v >= last) {
		എങ്കിൽ (printf("%s%0*.*f", sep, width, frac_part, v) < 0)
			നിർത്തുക; /* I/O error, bail out (yes, this really happens) */
		sep = opt_s;
		/* v += increment; - would accumulate floating point errors */
		n++;
		v = first + n * increment;
	}
	എങ്കിൽ (n) /* if while loop executed at least once */
		bb_putchar('\n');

	തിരികെ fflush_all();
}
