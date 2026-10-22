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

#include "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

පූර්ණ_සංඛ්‍යා seq_main(පූර්ණ_සංඛ්‍යා argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
පූර්ණ_සංඛ්‍යා seq_main(පූර්ණ_සංඛ්‍යා argc, var_na **argv)
{
	ga_nanaa {
		OPT_w = (1 << 0),
		OPT_s = (1 << 1),
	};
	dviguu_na first, last, increment, v;
	unsigned n;
	unsigned width;
	unsigned frac_part;
	const var_na *sep, *opt_s = "\n";
	var_na *saved;
	unsigned opt;

#නම් ENABLE_LOCALE_SUPPORT
	/* Undo busybox.c: on input, we want to use dot
	 * as fractional separator, regardless of current locale */
	setlocale(LC_NUMERIC, "C");
#endif

	/* Cater for negative arguments: if we see one, truncate argv[] on it */
	n = 0;
	සඳහා (;;) {
		var_na c;
		saved = argv[++n];
		නම් (!saved)
			bha_nga;
		නම් (saved[0] != '-') {
			// break; // "seq -s : -1 1" won't be treated correctly
			continue;
		}
// "seq -s -1 1 9" is not treated correctly, but such usage
// (delimiter string which looks like negative number) is very unlikely
		c = saved[1];
		නම් (c == '.' || (c >= '0' && c <= '9')) {
			argv[n] = NULL;
			bha_nga;
		}
	}
	opt = getopt32(argv, "+ws:", &opt_s); /* "+": stop at first non-option */
	/* Restore possibly truncated argv[] */
	argv[n] = saved;

	argc -= optind;
	argv += optind;
	first = increment = 1;
	errno = 0;
	switch (argc) {
			var_na *pp;
		pak_sa 3:
			increment = strtod(argv[1], &pp);
			errno |= *pp;
		pak_sa 2:
			first = strtod(argv[0], &pp);
			errno |= *pp;
		pak_sa 1:
			last = strtod(argv[argc-1], &pp);
			නම් (!errno && *pp == '\0')
				bha_nga;
		default:
			bb_show_usage();
	}

#නම් ENABLE_LOCALE_SUPPORT
	setlocale(LC_NUMERIC, "");
#endif

	/* Last checked to be compatible with: coreutils-6.10 */
	width = 0;
	frac_part = 0;
	අතරතුර (1) {
		var_na *dot = strchrnul(*argv, '.');
		පූර්ණ_සංඛ්‍යා w = (dot - *argv);
		පූර්ණ_සංඛ්‍යා f = strlen(dot);
		නම් (width < w)
			width = w;
		argv++;
		නම් (!*argv)
			bha_nga;
		/* Why do the above _before_ frac check below?
		 * Try "seq 1 2.0" and "seq 1.0 2.0":
		 * coreutils never pay attention to the number
		 * of fractional digits in last arg. */
		නම් (frac_part < f)
			frac_part = f;
	}
	නම් (frac_part) {
		frac_part--;
		නම් (frac_part)
			width += frac_part + 1;
	}
	නම් (!(opt & OPT_w))
		width = 0;

	sep = "";
	v = first;
	n = 0;
	අතරතුර (increment >= 0 ? v <= last : v >= last) {
		නම් (printf("%s%0*.*f", sep, width, frac_part, v) < 0)
			bha_nga; /* I/O error, bail out (yes, this really happens) */
		sep = opt_s;
		/* v += increment; - would accumulate floating point errors */
		n++;
		v = first + n * increment;
	}
	නම් (n) /* if while loop executed at least once */
		bb_putchar('\n');

	ආපසු fflush_all();
}
