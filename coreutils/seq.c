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

#شامل "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

صحیح seq_main(صحیح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح seq_main(صحیح argc, حرف **argv)
{
	شمار {
		OPT_w = (1 << 0),
		OPT_s = (1 << 1),
	};
	دوہرا پہلا, آخری, increment, v;
	غیرعلامتی n;
	غیرعلامتی چوڑائی;
	غیرعلامتی frac_part;
	ثابت حرف *sep, *opt_s = "\n";
	حرف *saved;
	غیرعلامتی opt;

#اگر ENABLE_LOCALE_SUPPORT
	/* Undo busybox.c: on input, we want to use dot
	 * as fractional separator, regardless of current locale */
	setlocale(LC_NUMERIC, "C");
#اگر_ختم

	/* Cater for negative arguments: if we see one, truncate argv[] on it */
	n = 0;
	برائے (;;) {
		حرف c;
		saved = argv[++n];
		اگر (!saved)
			توڑو;
		اگر (saved[0] != '-') {
			// break; // "seq -s : -1 1" won't be treated correctly
			جاری;
		}
// "seq -s -1 1 9" is not treated correctly, but such usage
// (delimiter string which looks like negative number) is very unlikely
		c = saved[1];
		اگر (c == '.' || (c >= '0' && c <= '9')) {
			argv[n] = تہی;
			توڑو;
		}
	}
	opt = getopt32(argv, "+ws:", &opt_s); /* "+": stop at first non-option */
	/* Restore possibly truncated argv[] */
	argv[n] = saved;

	argc -= optind;
	argv += optind;
	پہلا = increment = 1;
	errno = 0;
	انتخاب (argc) {
			حرف *pp;
		حالت 3:
			increment = strtod(argv[1], &pp);
			errno |= *pp;
		حالت 2:
			پہلا = strtod(argv[0], &pp);
			errno |= *pp;
		حالت 1:
			آخری = strtod(argv[argc-1], &pp);
			اگر (!errno && *pp == '\0')
				توڑو;
		طےشدہ:
			bb_show_usage();
	}

#اگر ENABLE_LOCALE_SUPPORT
	setlocale(LC_NUMERIC, "");
#اگر_ختم

	/* Last checked to be compatible with: coreutils-6.10 */
	چوڑائی = 0;
	frac_part = 0;
	جب (1) {
		حرف *dot = strchrnul(*argv, '.');
		صحیح w = (dot - *argv);
		صحیح f = لڑی_لمبائی(dot);
		اگر (چوڑائی < w)
			چوڑائی = w;
		argv++;
		اگر (!*argv)
			توڑو;
		/* Why do the above _before_ frac check below?
		 * Try "seq 1 2.0" and "seq 1.0 2.0":
		 * coreutils never pay attention to the number
		 * of fractional digits in last arg. */
		اگر (frac_part < f)
			frac_part = f;
	}
	اگر (frac_part) {
		frac_part--;
		اگر (frac_part)
			چوڑائی += frac_part + 1;
	}
	اگر (!(opt & OPT_w))
		چوڑائی = 0;

	sep = "";
	v = پہلا;
	n = 0;
	جب (increment >= 0 ? v <= آخری : v >= آخری) {
		اگر (لکھو("%s%0*.*f", sep, چوڑائی, frac_part, v) < 0)
			توڑو; /* I/O error, bail out (yes, this really happens) */
		sep = opt_s;
		/* v += increment; - would accumulate floating point errors */
		n++;
		v = پہلا + n * increment;
	}
	اگر (n) /* if while loop executed at least once */
		bb_putchar('\n');

	واپس fflush_all();
}
