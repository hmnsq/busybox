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

#define OPT_e		(1 << 0)
#define OPT_i		(1 << 1)
#define OPT_n		(1 << 2)
#define OPT_o		(1 << 3)
#define OPT_z		(1 << 4)
#define OPT_STR		"ei:n:o:z"

/*
 * Use the Fisher-Yates shuffle algorithm on an array of lines.
 * If the required number of output lines is less than the total
 * we can stop shuffling early.
 */
ಸ್ಥಿರ ಶೂನ್ಯ shuffle_ಸಾಲುಗಳು(ವರ್ಣ **ಸಾಲುಗಳು, unsigned numlines, unsigned outlines)
{
	srand(monotonic_us());

	ಯಾವತ್ (outlines != 0) {
		ವರ್ಣ *tmp;
		unsigned r = rand();
		/* RAND_MAX can be as small as 32767 */
		ಯದಿ (numlines > RAND_MAX)
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
#define COMMON_PREFIX_HACK 0

ಪೂರ್ಣಾಙ್ಕ shuf_main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಙ್ಕ shuf_main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv)
{
	unsigned opts;
	ವರ್ಣ *opt_i_str, *opt_n_str, *opt_o_str;
	ವರ್ಣ **ಸಾಲುಗಳು;
	unsigned long long lo = lo;
	unsigned numlines, outlines;
	unsigned i;
	ವರ್ಣ eol;
#ಯದಿ COMMON_PREFIX_HACK
	unsigned pfx_len = 0;
	unsigned padding_width = padding_width;
	ಧ್ರುವ ವರ್ಣ *pfx = pfx;
#endif

	opts = getopt32(argv, "^"
			OPT_STR
			"\0" "e--i:i--e"/* mutually exclusive */,
			&opt_i_str, &opt_n_str, &opt_o_str
	);

	argc -= optind;
	argv += optind;

	/* Prepare lines for shuffling - either: */
	ಯದಿ (opts & OPT_e) {
		/* make lines from command-line arguments */
		numlines = argc;
		ಸಾಲುಗಳು = argv;
	} ಅನ್ಯಥಾ
	ಯದಿ (opts & OPT_i) {
		/* create a range of numbers */
		unsigned long long hi;
		ವರ್ಣ *dash;

		ಯದಿ (argv[0])
			bb_show_usage();

		dash = strchr(opt_i_str, '-');
		ಯದಿ (!dash) {
			bb_error_ಸಂದೇಶ_and_die("bad range '%s'", opt_i_str);
		}
		*dash++ = '\0';
#ಯದಿ COMMON_PREFIX_HACK
		{
			ಧ್ರುವ ವರ್ಣ *a = opt_i_str;
			ಧ್ರುವ ವರ್ಣ *b = dash;
			/* Skip leading zeros (they may mask that common prefix does exist) */
			ಯಾವತ್ (*a == '0') a++;
			ಯಾವತ್ (*b == '0') b++;
			/* Do we have a common prefix (long enough to bother)? */
			padding_width = strlen(a);
			ಯದಿ (padding_width > 5 && padding_width == strlen(b)) {
				/* How long is it? */
				pfx = a;
				ಯಾವತ್ (isdigit(*a) && *a == *b
				 && a[1] /* "111111-111111" case: avoid xatoull("") */
				) {
					a++;
					b++;
				}
				pfx_len = a - pfx; /* can end up being 0 */
				padding_width -= pfx_len;
			} ಅನ್ಯಥಾ {
				/* Undo leading zero 'eating' (think "0-9") */
				a = opt_i_str;
				b = dash;
			}
			lo = xatoull(a);
			hi = xatoull(b);
		}
#ಅನ್ಯಥಾ
		lo = xatoull(opt_i_str);
		hi = xatoull(dash);
#endif
		dash[-1] = '-';
		ಯದಿ (hi < lo)
			bb_error_ಸಂದೇಶ_and_die("bad range '%s'", opt_i_str);
		hi -= lo;
		ಯದಿ (ಪರಿಮಾಣ(size_t) > ಪರಿಮಾಣ(numlines)) {
			ಯದಿ (hi >= UINT_MAX)
				bb_error_ಸಂದೇಶ_and_die("bad range '%s'", opt_i_str);
		} ಅನ್ಯಥಾ {
			ಯದಿ (hi >= UINT_MAX / ಪರಿಮಾಣ(ಸಾಲುಗಳು[0]))
				bb_error_ಸಂದೇಶ_and_die("bad range '%s'", opt_i_str);
		}

		numlines = hi + 1;
		ಸಾಲುಗಳು = xmalloc((size_t)numlines * ಪರಿಮಾಣ(ಸಾಲುಗಳು[0]));
		ಪ್ರತಿ (i = 0; i < numlines; i++) {
			ಸಾಲುಗಳು[i] = (ವರ್ಣ*)(uintptr_t)i;
		}
	} ಅನ್ಯಥಾ {
		/* default - read lines from stdin or the input file */
		FILE *fp;
		ಧ್ರುವ ವರ್ಣ *fname = "-";

		ಯದಿ (argv[0]) {
			ಯದಿ (argv[1])
				bb_show_usage();
			fname = argv[0];
		}

		fp = xfopen_stdin(fname);
		ಸಾಲುಗಳು = ರಿಕ್ತ;
		numlines = 0;
		ಪ್ರತಿ (;;) {
			ವರ್ಣ *ಸಾಲು = xmalloc_fgetline(fp);
			ಯದಿ (!ಸಾಲು)
				ಭಙ್ಗ;
			ಸಾಲುಗಳು = xrealloc_vector(ಸಾಲುಗಳು, 6, numlines);
			ಸಾಲುಗಳು[numlines++] = ಸಾಲು;
		}
		fclose_if_not_stdin(fp);
	}

	outlines = numlines;
	ಯದಿ (opts & OPT_n) {
		outlines = xatou(opt_n_str);
		ಯದಿ (outlines > numlines)
			outlines = numlines;
	}

	shuffle_ಸಾಲುಗಳು(ಸಾಲುಗಳು, numlines, outlines);

	ಯದಿ (opts & OPT_o)
		xmove_fd(xopen(opt_o_str, O_WRONLY|O_CREAT|O_TRUNC), STDOUT_FILENO);

	eol = '\n';
	ಯದಿ (opts & OPT_z)
		eol = '\0';

	ಪ್ರತಿ (i = numlines - outlines; i < numlines; i++) {
		ಯದಿ (opts & OPT_i) {
#ಯದಿ COMMON_PREFIX_HACK
			ಯದಿ (pfx_len != 0)
				printf("%.*s%0*llu%c", pfx_len, pfx, padding_width, lo + (uintptr_t)ಸಾಲುಗಳು[i], eol);
			ಅನ್ಯಥಾ
#endif
				printf("%llu%c", lo + (uintptr_t)ಸಾಲುಗಳು[i], eol);
		} ಅನ್ಯಥಾ
			printf("%s%c", ಸಾಲುಗಳು[i], eol);
	}

	fflush_stdout_and_exit_SUCCESS();
}
