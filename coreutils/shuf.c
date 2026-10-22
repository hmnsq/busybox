/* vi: set sw=4 ts=4: */
/*
 * shuf: Write a random permutation of the input lines to standard output.
 *
 * Copyright (C) 2014 by Bartosz Golaszewski <bartekgola@gmail.com>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config SHUF
//config:	bool "shuf (6 kb)"
//config:	default y
//config:	help
//config:	Generate random permutations

//applet:IF_SHUF(APPLET_NOEXEC(shuf, shuf, BB_DIR_USR_BIN, BB_SUID_DROP, shuf))

//kbuild:lib-$(CONFIG_SHUF) += shuf.o

//usage:#define shuf_trivial_usage
//usage:       "[-n NUM] [-o FILE] [-z] [FILE | -e [ARG...] | -i L-H]"
//usage:#define shuf_full_usage "\n\n"
//usage:       "Randomly permute lines\n"
//usage:     "\n	-n NUM	Output at most NUM lines"
//usage:     "\n	-o FILE	Write to FILE, not standard output"
//usage:     "\n	-z	NUL terminated output"
//usage:     "\n	-e	Treat ARGs as lines"
//usage:     "\n	-i L-H	Treat numbers L-H as lines"

#అన్తర్భావ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

#నిర్వచించు OPT_e		(1 << 0)
#నిర్వచించు OPT_i		(1 << 1)
#నిర్వచించు OPT_n		(1 << 2)
#నిర్వచించు OPT_o		(1 << 3)
#నిర్వచించు OPT_z		(1 << 4)
#నిర్వచించు OPT_STR		"ei:n:o:z"

/*
 * Use the Fisher-Yates shuffle algorithm on an array of lines.
 * If the required number of output lines is less than the total
 * we can stop shuffling early.
 */
స్థిర శూన్య shuffle_పంక్తులు(అక్షరం **పంక్తులు, గుర్తులేని numlines, గుర్తులేని outlines)
{
	srand(monotonic_us());

	అయితే (outlines != 0) {
		అక్షరం *tmp;
		గుర్తులేని r = rand();
		/* RAND_MAX can be as small as 32767 */
		ఒకవేళ (numlines > RAND_MAX)
			r ^= rand() << 15;
		r %= numlines;
//TODO: the above method is seriously non-uniform when numlines is very large.
//For example, with numlines of   0xf0000000,
//values of (r % numlines) in [0, 0x0fffffff] range
//are more likely: e.g. r=1 and r=0xf0000001 both map to 1,
//whereas only one value, r=0xefffffff, maps to 0xefffffff.
		numlines--;
		tmp = పంక్తులు[numlines];
		పంక్తులు[numlines] = పంక్తులు[r];
		పంక్తులు[r] = tmp;
		outlines--;
	}
}

/* We can handle insanity like this:
 * shuf -i 333333333333333333333333333333001-333333333333333333333333333333019
 * but do we want to have +200 bytes of code (~40% code growth)?
 */
#నిర్వచించు COMMON_ఉపసర్గ_HACK 0

పూర్ణాంకం shuf_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం shuf_main(పూర్ణాంకం argc, అక్షరం **argv)
{
	గుర్తులేని opts;
	అక్షరం *opt_i_str, *opt_n_str, *opt_o_str;
	అక్షరం **పంక్తులు;
	గుర్తులేని దీర్ఘ దీర్ఘ lo = lo;
	గుర్తులేని numlines, outlines;
	గుర్తులేని i;
	అక్షరం eol;
#ఒకవేళ COMMON_ఉపసర్గ_HACK
	గుర్తులేని pfx_len = 0;
	గుర్తులేని padding_వెడల్పు = padding_వెడల్పు;
	స్థిరరాశి అక్షరం *pfx = pfx;
#అయితే_ముగింపు

	opts = getopt32(argv, "^"
			OPT_STR
			"\0" "e--i:i--e"/* mutually exclusive */,
			&opt_i_str, &opt_n_str, &opt_o_str
	);

	argc -= optind;
	argv += optind;

	/* Prepare lines for shuffling - either: */
	ఒకవేళ (opts & OPT_e) {
		/* make lines from command-line arguments */
		numlines = argc;
		పంక్తులు = argv;
	} లేకపోతే
	ఒకవేళ (opts & OPT_i) {
		/* create a range of numbers */
		గుర్తులేని దీర్ఘ దీర్ఘ hi;
		అక్షరం *dash;

		ఒకవేళ (argv[0])
			bb_show_usage();

		dash = strchr(opt_i_str, '-');
		ఒకవేళ (!dash) {
			bb_error_సందేశం_and_die("bad range '%s'", opt_i_str);
		}
		*dash++ = '\0';
#ఒకవేళ COMMON_ఉపసర్గ_HACK
		{
			స్థిరరాశి అక్షరం *a = opt_i_str;
			స్థిరరాశి అక్షరం *b = dash;
			/* Skip leading zeros (they may mask that common prefix does exist) */
			అయితే (*a == '0') a++;
			అయితే (*b == '0') b++;
			/* Do we have a common prefix (long enough to bother)? */
			padding_వెడల్పు = అక్షరశ్రేణి_పొడవు(a);
			ఒకవేళ (padding_వెడల్పు > 5 && padding_వెడల్పు == అక్షరశ్రేణి_పొడవు(b)) {
				/* How long is it? */
				pfx = a;
				అయితే (అంకెనా(*a) && *a == *b
				 && a[1] /* "111111-111111" case: avoid xatoull("") */
				) {
					a++;
					b++;
				}
				pfx_len = a - pfx; /* can end up being 0 */
				padding_వెడల్పు -= pfx_len;
			} లేకపోతే {
				/* Undo leading zero 'eating' (think "0-9") */
				a = opt_i_str;
				b = dash;
			}
			lo = xatoull(a);
			hi = xatoull(b);
		}
#లేకపోతే
		lo = xatoull(opt_i_str);
		hi = xatoull(dash);
#అయితే_ముగింపు
		dash[-1] = '-';
		ఒకవేళ (hi < lo)
			bb_error_సందేశం_and_die("bad range '%s'", opt_i_str);
		hi -= lo;
		ఒకవేళ (పరిమాణం(కొలత_t) > పరిమాణం(numlines)) {
			ఒకవేళ (hi >= UINT_MAX)
				bb_error_సందేశం_and_die("bad range '%s'", opt_i_str);
		} లేకపోతే {
			ఒకవేళ (hi >= UINT_MAX / పరిమాణం(పంక్తులు[0]))
				bb_error_సందేశం_and_die("bad range '%s'", opt_i_str);
		}

		numlines = hi + 1;
		పంక్తులు = xmalloc((కొలత_t)numlines * పరిమాణం(పంక్తులు[0]));
		కొరకు (i = 0; i < numlines; i++) {
			పంక్తులు[i] = (అక్షరం*)(uintptr_t)i;
		}
	} లేకపోతే {
		/* default - read lines from stdin or the input file */
		FILE *fp;
		స్థిరరాశి అక్షరం *fname = "-";

		ఒకవేళ (argv[0]) {
			ఒకవేళ (argv[1])
				bb_show_usage();
			fname = argv[0];
		}

		fp = xfopen_stdin(fname);
		పంక్తులు = ఖాళీ;
		numlines = 0;
		కొరకు (;;) {
			అక్షరం *పంక్తి = xmalloc_fgetline(fp);
			ఒకవేళ (!పంక్తి)
				ఆపు;
			పంక్తులు = xrealloc_సదిశ(పంక్తులు, 6, numlines);
			పంక్తులు[numlines++] = పంక్తి;
		}
		fclose_if_not_stdin(fp);
	}

	outlines = numlines;
	ఒకవేళ (opts & OPT_n) {
		outlines = xatou(opt_n_str);
		ఒకవేళ (outlines > numlines)
			outlines = numlines;
	}

	shuffle_పంక్తులు(పంక్తులు, numlines, outlines);

	ఒకవేళ (opts & OPT_o)
		xmove_fd(xopen(opt_o_str, O_WRONLY|O_CREAT|O_TRUNC), STDOUT_FILENO);

	eol = '\n';
	ఒకవేళ (opts & OPT_z)
		eol = '\0';

	కొరకు (i = numlines - outlines; i < numlines; i++) {
		ఒకవేళ (opts & OPT_i) {
#ఒకవేళ COMMON_ఉపసర్గ_HACK
			ఒకవేళ (pfx_len != 0)
				రాయి("%.*s%0*llu%c", pfx_len, pfx, padding_వెడల్పు, lo + (uintptr_t)పంక్తులు[i], eol);
			లేకపోతే
#అయితే_ముగింపు
				రాయి("%llu%c", lo + (uintptr_t)పంక్తులు[i], eol);
		} లేకపోతే
			రాయి("%s%c", పంక్తులు[i], eol);
	}

	fflush_stdout_and_exit_SUCCESS();
}
