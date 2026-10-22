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

#ಅನ್ತರ್ಭಾವ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

#ವ್ಯಾಖ್ಯಾನಿಸು OPT_e		(1 << 0)
#ವ್ಯಾಖ್ಯಾನಿಸು OPT_i		(1 << 1)
#ವ್ಯಾಖ್ಯಾನಿಸು OPT_n		(1 << 2)
#ವ್ಯಾಖ್ಯಾನಿಸು OPT_o		(1 << 3)
#ವ್ಯಾಖ್ಯಾನಿಸು OPT_z		(1 << 4)
#ವ್ಯಾಖ್ಯಾನಿಸು OPT_STR		"ei:n:o:z"

/*
 * Use the Fisher-Yates shuffle algorithm on an array of lines.
 * If the required number of output lines is less than the total
 * we can stop shuffling early.
 */
ಸ್ಥಿರ ಶೂನ್ಯ shuffle_ಸಾಲುಗಳು(ಅಕ್ಷರ **ಸಾಲುಗಳು, ಚಿಹ್ನೆರಹಿತ numlines, ಚಿಹ್ನೆರಹಿತ outlines)
{
	srand(monotonic_us());

	ಆಗ (outlines != 0) {
		ಅಕ್ಷರ *tmp;
		ಚಿಹ್ನೆರಹಿತ r = rand();
		/* RAND_MAX can be as small as 32767 */
		ಆದರೆ (numlines > RAND_MAX)
			r ^= rand() << 15;
		r %= numlines;
//TODO: the above method is seriously non-uniform when numlines is very large.
//For example, with numlines of   0xf0000000,
//values of (r % numlines) in [0, 0x0fffffff] range
//are more likely: e.g. r=1 and r=0xf0000001 both map to 1,
//whereas only one value, r=0xefffffff, maps to 0xefffffff.
		numlines--;
		tmp = ಸಾಲುಗಳು[numlines];
		ಸಾಲುಗಳು[numlines] = ಸಾಲುಗಳು[r];
		ಸಾಲುಗಳು[r] = tmp;
		outlines--;
	}
}

/* We can handle insanity like this:
 * shuf -i 333333333333333333333333333333001-333333333333333333333333333333019
 * but do we want to have +200 bytes of code (~40% code growth)?
 */
#ವ್ಯಾಖ್ಯಾನಿಸು COMMON_ಪೂರ್ವಪ್ರತ್ಯಯ_HACK 0

ಪೂರ್ಣಾಂಕ shuf_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ shuf_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
	ಚಿಹ್ನೆರಹಿತ opts;
	ಅಕ್ಷರ *opt_i_str, *opt_n_str, *opt_o_str;
	ಅಕ್ಷರ **ಸಾಲುಗಳು;
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ lo = lo;
	ಚಿಹ್ನೆರಹಿತ numlines, outlines;
	ಚಿಹ್ನೆರಹಿತ i;
	ಅಕ್ಷರ eol;
#ಆದರೆ COMMON_ಪೂರ್ವಪ್ರತ್ಯಯ_HACK
	ಚಿಹ್ನೆರಹಿತ pfx_len = 0;
	ಚಿಹ್ನೆರಹಿತ padding_ಅಗಲ = padding_ಅಗಲ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *pfx = pfx;
#ಆದರೆ_ಅಂತ್ಯ

	opts = getopt32(argv, "^"
			OPT_STR
			"\0" "e--i:i--e"/* mutually exclusive */,
			&opt_i_str, &opt_n_str, &opt_o_str
	);

	argc -= optind;
	argv += optind;

	/* Prepare lines for shuffling - either: */
	ಆದರೆ (opts & OPT_e) {
		/* make lines from command-line arguments */
		numlines = argc;
		ಸಾಲುಗಳು = argv;
	} ಇಲ್ಲದಿದ್ದರೆ
	ಆದರೆ (opts & OPT_i) {
		/* create a range of numbers */
		ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ hi;
		ಅಕ್ಷರ *dash;

		ಆದರೆ (argv[0])
			bb_show_usage();

		dash = strchr(opt_i_str, '-');
		ಆದರೆ (!dash) {
			bb_error_ಸಂದೇಶ_and_die("bad range '%s'", opt_i_str);
		}
		*dash++ = '\0';
#ಆದರೆ COMMON_ಪೂರ್ವಪ್ರತ್ಯಯ_HACK
		{
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *a = opt_i_str;
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *b = dash;
			/* Skip leading zeros (they may mask that common prefix does exist) */
			ಆಗ (*a == '0') a++;
			ಆಗ (*b == '0') b++;
			/* Do we have a common prefix (long enough to bother)? */
			padding_ಅಗಲ = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(a);
			ಆದರೆ (padding_ಅಗಲ > 5 && padding_ಅಗಲ == ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(b)) {
				/* How long is it? */
				pfx = a;
				ಆಗ (ಅಂಕಿಯೇ(*a) && *a == *b
				 && a[1] /* "111111-111111" case: avoid xatoull("") */
				) {
					a++;
					b++;
				}
				pfx_len = a - pfx; /* can end up being 0 */
				padding_ಅಗಲ -= pfx_len;
			} ಇಲ್ಲದಿದ್ದರೆ {
				/* Undo leading zero 'eating' (think "0-9") */
				a = opt_i_str;
				b = dash;
			}
			lo = xatoull(a);
			hi = xatoull(b);
		}
#ಇಲ್ಲದಿದ್ದರೆ
		lo = xatoull(opt_i_str);
		hi = xatoull(dash);
#ಆದರೆ_ಅಂತ್ಯ
		dash[-1] = '-';
		ಆದರೆ (hi < lo)
			bb_error_ಸಂದೇಶ_and_die("bad range '%s'", opt_i_str);
		hi -= lo;
		ಆದರೆ (ಗಾತ್ರ(ಗಾತ್ರ_t) > ಗಾತ್ರ(numlines)) {
			ಆದರೆ (hi >= UINT_MAX)
				bb_error_ಸಂದೇಶ_and_die("bad range '%s'", opt_i_str);
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (hi >= UINT_MAX / ಗಾತ್ರ(ಸಾಲುಗಳು[0]))
				bb_error_ಸಂದೇಶ_and_die("bad range '%s'", opt_i_str);
		}

		numlines = hi + 1;
		ಸಾಲುಗಳು = xmalloc((ಗಾತ್ರ_t)numlines * ಗಾತ್ರ(ಸಾಲುಗಳು[0]));
		ಗಾಗಿ (i = 0; i < numlines; i++) {
			ಸಾಲುಗಳು[i] = (ಅಕ್ಷರ*)(uintptr_t)i;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* default - read lines from stdin or the input file */
		FILE *fp;
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname = "-";

		ಆದರೆ (argv[0]) {
			ಆದರೆ (argv[1])
				bb_show_usage();
			fname = argv[0];
		}

		fp = xfopen_stdin(fname);
		ಸಾಲುಗಳು = ಖಾಲಿ;
		numlines = 0;
		ಗಾಗಿ (;;) {
			ಅಕ್ಷರ *ಸಾಲು = xmalloc_fgetline(fp);
			ಆದರೆ (!ಸಾಲು)
				ನಿಲ್ಲಿಸು;
			ಸಾಲುಗಳು = xrealloc_ಸದಿಶ(ಸಾಲುಗಳು, 6, numlines);
			ಸಾಲುಗಳು[numlines++] = ಸಾಲು;
		}
		fclose_if_not_stdin(fp);
	}

	outlines = numlines;
	ಆದರೆ (opts & OPT_n) {
		outlines = xatou(opt_n_str);
		ಆದರೆ (outlines > numlines)
			outlines = numlines;
	}

	shuffle_ಸಾಲುಗಳು(ಸಾಲುಗಳು, numlines, outlines);

	ಆದರೆ (opts & OPT_o)
		xmove_fd(xopen(opt_o_str, O_WRONLY|O_CREAT|O_TRUNC), STDOUT_FILENO);

	eol = '\n';
	ಆದರೆ (opts & OPT_z)
		eol = '\0';

	ಗಾಗಿ (i = numlines - outlines; i < numlines; i++) {
		ಆದರೆ (opts & OPT_i) {
#ಆದರೆ COMMON_ಪೂರ್ವಪ್ರತ್ಯಯ_HACK
			ಆದರೆ (pfx_len != 0)
				ಬರೆ("%.*s%0*llu%c", pfx_len, pfx, padding_ಅಗಲ, lo + (uintptr_t)ಸಾಲುಗಳು[i], eol);
			ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
				ಬರೆ("%llu%c", lo + (uintptr_t)ಸಾಲುಗಳು[i], eol);
		} ಇಲ್ಲದಿದ್ದರೆ
			ಬರೆ("%s%c", ಸಾಲುಗಳು[i], eol);
	}

	fflush_stdout_and_exit_SUCCESS();
}
