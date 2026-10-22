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

#அந்தர்பாவ "libbb.h"

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
நிலையான வெறுமை shuffle_வரிகள்(எழுத்து **வரிகள், unsigned numlines, unsigned outlines)
{
	srand(monotonic_us());

	போது (outlines != 0) {
		எழுத்து *tmp;
		unsigned r = rand();
		/* RAND_MAX can be as small as 32767 */
		எனில் (numlines > RAND_MAX)
			r ^= rand() << 15;
		r %= numlines;
//TODO: the above method is seriously non-uniform when numlines is very large.
//For example, with numlines of   0xf0000000,
//values of (r % numlines) in [0, 0x0fffffff] range
//are more likely: e.g. r=1 and r=0xf0000001 both map to 1,
//whereas only one value, r=0xefffffff, maps to 0xefffffff.
		numlines--;
		tmp = வரிகள்[numlines];
		வரிகள்[numlines] = வரிகள்[r];
		வரிகள்[r] = tmp;
		outlines--;
	}
}

/* We can handle insanity like this:
 * shuf -i 333333333333333333333333333333001-333333333333333333333333333333019
 * but do we want to have +200 bytes of code (~40% code growth)?
 */
#define COMMON_PREFIX_HACK 0

முழு shuf_main(முழு argc, எழுத்து **argv) MAIN_EXTERNALLY_VISIBLE;
முழு shuf_main(முழு argc, எழுத்து **argv)
{
	unsigned opts;
	எழுத்து *opt_i_str, *opt_n_str, *opt_o_str;
	எழுத்து **வரிகள்;
	unsigned long long lo = lo;
	unsigned numlines, outlines;
	unsigned i;
	எழுத்து eol;
#எனில் COMMON_PREFIX_HACK
	unsigned pfx_len = 0;
	unsigned padding_width = padding_width;
	மாறிலி எழுத்து *pfx = pfx;
#endif

	opts = getopt32(argv, "^"
			OPT_STR
			"\0" "e--i:i--e"/* mutually exclusive */,
			&opt_i_str, &opt_n_str, &opt_o_str
	);

	argc -= optind;
	argv += optind;

	/* Prepare lines for shuffling - either: */
	எனில் (opts & OPT_e) {
		/* make lines from command-line arguments */
		numlines = argc;
		வரிகள் = argv;
	} இல்லையெனில்
	எனில் (opts & OPT_i) {
		/* create a range of numbers */
		unsigned long long hi;
		எழுத்து *dash;

		எனில் (argv[0])
			bb_show_usage();

		dash = strchr(opt_i_str, '-');
		எனில் (!dash) {
			bb_error_செய்தி_and_die("bad range '%s'", opt_i_str);
		}
		*dash++ = '\0';
#எனில் COMMON_PREFIX_HACK
		{
			மாறிலி எழுத்து *a = opt_i_str;
			மாறிலி எழுத்து *b = dash;
			/* Skip leading zeros (they may mask that common prefix does exist) */
			போது (*a == '0') a++;
			போது (*b == '0') b++;
			/* Do we have a common prefix (long enough to bother)? */
			padding_width = strlen(a);
			எனில் (padding_width > 5 && padding_width == strlen(b)) {
				/* How long is it? */
				pfx = a;
				போது (isdigit(*a) && *a == *b
				 && a[1] /* "111111-111111" case: avoid xatoull("") */
				) {
					a++;
					b++;
				}
				pfx_len = a - pfx; /* can end up being 0 */
				padding_width -= pfx_len;
			} இல்லையெனில் {
				/* Undo leading zero 'eating' (think "0-9") */
				a = opt_i_str;
				b = dash;
			}
			lo = xatoull(a);
			hi = xatoull(b);
		}
#இல்லையெனில்
		lo = xatoull(opt_i_str);
		hi = xatoull(dash);
#endif
		dash[-1] = '-';
		எனில் (hi < lo)
			bb_error_செய்தி_and_die("bad range '%s'", opt_i_str);
		hi -= lo;
		எனில் (அளவு(size_t) > அளவு(numlines)) {
			எனில் (hi >= UINT_MAX)
				bb_error_செய்தி_and_die("bad range '%s'", opt_i_str);
		} இல்லையெனில் {
			எனில் (hi >= UINT_MAX / அளவு(வரிகள்[0]))
				bb_error_செய்தி_and_die("bad range '%s'", opt_i_str);
		}

		numlines = hi + 1;
		வரிகள் = xmalloc((size_t)numlines * அளவு(வரிகள்[0]));
		க்கு (i = 0; i < numlines; i++) {
			வரிகள்[i] = (எழுத்து*)(uintptr_t)i;
		}
	} இல்லையெனில் {
		/* default - read lines from stdin or the input file */
		FILE *fp;
		மாறிலி எழுத்து *fname = "-";

		எனில் (argv[0]) {
			எனில் (argv[1])
				bb_show_usage();
			fname = argv[0];
		}

		fp = xfopen_stdin(fname);
		வரிகள் = வெற்று;
		numlines = 0;
		க்கு (;;) {
			எழுத்து *வரி = xmalloc_fgetline(fp);
			எனில் (!வரி)
				நிறுத்து;
			வரிகள் = xrealloc_vector(வரிகள், 6, numlines);
			வரிகள்[numlines++] = வரி;
		}
		fclose_if_not_stdin(fp);
	}

	outlines = numlines;
	எனில் (opts & OPT_n) {
		outlines = xatou(opt_n_str);
		எனில் (outlines > numlines)
			outlines = numlines;
	}

	shuffle_வரிகள்(வரிகள், numlines, outlines);

	எனில் (opts & OPT_o)
		xmove_fd(xopen(opt_o_str, O_WRONLY|O_CREAT|O_TRUNC), STDOUT_FILENO);

	eol = '\n';
	எனில் (opts & OPT_z)
		eol = '\0';

	க்கு (i = numlines - outlines; i < numlines; i++) {
		எனில் (opts & OPT_i) {
#எனில் COMMON_PREFIX_HACK
			எனில் (pfx_len != 0)
				printf("%.*s%0*llu%c", pfx_len, pfx, padding_width, lo + (uintptr_t)வரிகள்[i], eol);
			இல்லையெனில்
#endif
				printf("%llu%c", lo + (uintptr_t)வரிகள்[i], eol);
		} இல்லையெனில்
			printf("%s%c", வரிகள்[i], eol);
	}

	fflush_stdout_and_exit_SUCCESS();
}
