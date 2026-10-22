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
	अचिन्हित n;
	अचिन्हित width;
	अचिन्हित frac_part;
	अचर वर्ण *sep, *opt_s = "\n";
	वर्ण *saved;
	अचिन्हित opt;

#यदि ENABLE_LOCALE_SUPPORT
	/* Undo busybox.c: on input, we want to use dot
	 * as fractional separator, regardless of current locale */
	रखो_क्षेत्र(LC_NUMERIC, "C");
#endif

	/* Cater for negative arguments: if we see one, truncate argv[] on it */
	n = 0;
	लागि (;;) {
		वर्ण c;
		saved = argv[++n];
		यदि (!saved)
			रोक;
		यदि (saved[0] != '-') {
			// break; // "seq -s : -1 1" won't be treated correctly
			जारी;
		}
// "seq -s -1 1 9" is not treated correctly, but such usage
// (delimiter string which looks like negative number) is very unlikely
		c = saved[1];
		यदि (c == '.' || (c >= '0' && c <= '9')) {
			argv[n] = रिक्त;
			रोक;
		}
	}
	opt = getopt32(argv, "+ws:", &opt_s); /* "+": stop at first non-option */
	/* Restore possibly truncated argv[] */
	argv[n] = saved;

	argc -= optind;
	argv += optind;
	first = increment = 1;
	त्रुटि_सं = 0;
	विकल्प (argc) {
			वर्ण *pp;
		पक्ष 3:
			increment = म_से_भग्न(argv[1], &pp);
			त्रुटि_सं |= *pp;
		पक्ष 2:
			first = म_से_भग्न(argv[0], &pp);
			त्रुटि_सं |= *pp;
		पक्ष 1:
			last = म_से_भग्न(argv[argc-1], &pp);
			यदि (!त्रुटि_सं && *pp == '\0')
				रोक;
		स्वभाव:
			bb_show_usage();
	}

#यदि ENABLE_LOCALE_SUPPORT
	रखो_क्षेत्र(LC_NUMERIC, "");
#endif

	/* Last checked to be compatible with: coreutils-6.10 */
	width = 0;
	frac_part = 0;
	जबसम्म (1) {
		वर्ण *dot = strchrnul(*argv, '.');
		पूर्णाङ्क w = (dot - *argv);
		पूर्णाङ्क f = म_माप(dot);
		यदि (width < w)
			width = w;
		argv++;
		यदि (!*argv)
			रोक;
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
	जबसम्म (increment >= 0 ? v <= last : v >= last) {
		यदि (म_लिखो("%s%0*.*f", sep, width, frac_part, v) < 0)
			रोक; /* I/O error, bail out (yes, this really happens) */
		sep = opt_s;
		/* v += increment; - would accumulate floating point errors */
		n++;
		v = first + n * increment;
	}
	यदि (n) /* if while loop executed at least once */
		bb_putchar('\n');

	फर्काऊ fflush_all();
}
