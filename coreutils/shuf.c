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

#ଅନ୍ତର୍ଭାଵ "libbb.h"

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
ସ୍ଥିର ଶୂନ୍ୟ shuffle_ଧାଡ଼ିଗୁଡ଼ିକ(ଅକ୍ଷର **ଧାଡ଼ିଗୁଡ଼ିକ, unsigned numlines, unsigned outlines)
{
	srand(monotonic_us());

	ଯେତେବେଳେ (outlines != 0) {
		ଅକ୍ଷର *tmp;
		unsigned r = rand();
		/* RAND_MAX can be as small as 32767 */
		ଯଦି (numlines > RAND_MAX)
			r ^= rand() << 15;
		r %= numlines;
//TODO: the above method is seriously non-uniform when numlines is very large.
//For example, with numlines of   0xf0000000,
//values of (r % numlines) in [0, 0x0fffffff] range
//are more likely: e.g. r=1 and r=0xf0000001 both map to 1,
//whereas only one value, r=0xefffffff, maps to 0xefffffff.
		numlines--;
		tmp = ଧାଡ଼ିଗୁଡ଼ିକ[numlines];
		ଧାଡ଼ିଗୁଡ଼ିକ[numlines] = ଧାଡ଼ିଗୁଡ଼ିକ[r];
		ଧାଡ଼ିଗୁଡ଼ିକ[r] = tmp;
		outlines--;
	}
}

/* We can handle insanity like this:
 * shuf -i 333333333333333333333333333333001-333333333333333333333333333333019
 * but do we want to have +200 bytes of code (~40% code growth)?
 */
#define COMMON_PREFIX_HACK 0

ପୂର୍ଣ୍ଣସଂଖ୍ୟା shuf_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv) MAIN_EXTERNALLY_VISIBLE;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା shuf_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv)
{
	unsigned opts;
	ଅକ୍ଷର *opt_i_str, *opt_n_str, *opt_o_str;
	ଅକ୍ଷର **ଧାଡ଼ିଗୁଡ଼ିକ;
	unsigned long long lo = lo;
	unsigned numlines, outlines;
	unsigned i;
	ଅକ୍ଷର eol;
#ଯଦି COMMON_PREFIX_HACK
	unsigned pfx_len = 0;
	unsigned padding_width = padding_width;
	ଅଚଳ ଅକ୍ଷର *pfx = pfx;
#endif

	opts = getopt32(argv, "^"
			OPT_STR
			"\0" "e--i:i--e"/* mutually exclusive */,
			&opt_i_str, &opt_n_str, &opt_o_str
	);

	argc -= optind;
	argv += optind;

	/* Prepare lines for shuffling - either: */
	ଯଦି (opts & OPT_e) {
		/* make lines from command-line arguments */
		numlines = argc;
		ଧାଡ଼ିଗୁଡ଼ିକ = argv;
	} ଅନ୍ୟଥା
	ଯଦି (opts & OPT_i) {
		/* create a range of numbers */
		unsigned long long hi;
		ଅକ୍ଷର *dash;

		ଯଦି (argv[0])
			bb_show_usage();

		dash = strchr(opt_i_str, '-');
		ଯଦି (!dash) {
			bb_error_ସନ୍ଦେଶ_and_die("bad range '%s'", opt_i_str);
		}
		*dash++ = '\0';
#ଯଦି COMMON_PREFIX_HACK
		{
			ଅଚଳ ଅକ୍ଷର *a = opt_i_str;
			ଅଚଳ ଅକ୍ଷର *b = dash;
			/* Skip leading zeros (they may mask that common prefix does exist) */
			ଯେତେବେଳେ (*a == '0') a++;
			ଯେତେବେଳେ (*b == '0') b++;
			/* Do we have a common prefix (long enough to bother)? */
			padding_width = strlen(a);
			ଯଦି (padding_width > 5 && padding_width == strlen(b)) {
				/* How long is it? */
				pfx = a;
				ଯେତେବେଳେ (isdigit(*a) && *a == *b
				 && a[1] /* "111111-111111" case: avoid xatoull("") */
				) {
					a++;
					b++;
				}
				pfx_len = a - pfx; /* can end up being 0 */
				padding_width -= pfx_len;
			} ଅନ୍ୟଥା {
				/* Undo leading zero 'eating' (think "0-9") */
				a = opt_i_str;
				b = dash;
			}
			lo = xatoull(a);
			hi = xatoull(b);
		}
#ଅନ୍ୟଥା
		lo = xatoull(opt_i_str);
		hi = xatoull(dash);
#endif
		dash[-1] = '-';
		ଯଦି (hi < lo)
			bb_error_ସନ୍ଦେଶ_and_die("bad range '%s'", opt_i_str);
		hi -= lo;
		ଯଦି (ଆକାର(size_t) > ଆକାର(numlines)) {
			ଯଦି (hi >= UINT_MAX)
				bb_error_ସନ୍ଦେଶ_and_die("bad range '%s'", opt_i_str);
		} ଅନ୍ୟଥା {
			ଯଦି (hi >= UINT_MAX / ଆକାର(ଧାଡ଼ିଗୁଡ଼ିକ[0]))
				bb_error_ସନ୍ଦେଶ_and_die("bad range '%s'", opt_i_str);
		}

		numlines = hi + 1;
		ଧାଡ଼ିଗୁଡ଼ିକ = xmalloc((size_t)numlines * ଆକାର(ଧାଡ଼ିଗୁଡ଼ିକ[0]));
		ପାଇଁ (i = 0; i < numlines; i++) {
			ଧାଡ଼ିଗୁଡ଼ିକ[i] = (ଅକ୍ଷର*)(uintptr_t)i;
		}
	} ଅନ୍ୟଥା {
		/* default - read lines from stdin or the input file */
		FILE *fp;
		ଅଚଳ ଅକ୍ଷର *fname = "-";

		ଯଦି (argv[0]) {
			ଯଦି (argv[1])
				bb_show_usage();
			fname = argv[0];
		}

		fp = xfopen_stdin(fname);
		ଧାଡ଼ିଗୁଡ଼ିକ = ଖାଲି;
		numlines = 0;
		ପାଇଁ (;;) {
			ଅକ୍ଷର *ଧାଡ଼ି = xmalloc_fgetline(fp);
			ଯଦି (!ଧାଡ଼ି)
				ଭାଙ୍ଗ;
			ଧାଡ଼ିଗୁଡ଼ିକ = xrealloc_vector(ଧାଡ଼ିଗୁଡ଼ିକ, 6, numlines);
			ଧାଡ଼ିଗୁଡ଼ିକ[numlines++] = ଧାଡ଼ି;
		}
		fclose_if_not_stdin(fp);
	}

	outlines = numlines;
	ଯଦି (opts & OPT_n) {
		outlines = xatou(opt_n_str);
		ଯଦି (outlines > numlines)
			outlines = numlines;
	}

	shuffle_ଧାଡ଼ିଗୁଡ଼ିକ(ଧାଡ଼ିଗୁଡ଼ିକ, numlines, outlines);

	ଯଦି (opts & OPT_o)
		xmove_fd(xopen(opt_o_str, O_WRONLY|O_CREAT|O_TRUNC), STDOUT_FILENO);

	eol = '\n';
	ଯଦି (opts & OPT_z)
		eol = '\0';

	ପାଇଁ (i = numlines - outlines; i < numlines; i++) {
		ଯଦି (opts & OPT_i) {
#ଯଦି COMMON_PREFIX_HACK
			ଯଦି (pfx_len != 0)
				printf("%.*s%0*llu%c", pfx_len, pfx, padding_width, lo + (uintptr_t)ଧାଡ଼ିଗୁଡ଼ିକ[i], eol);
			ଅନ୍ୟଥା
#endif
				printf("%llu%c", lo + (uintptr_t)ଧାଡ଼ିଗୁଡ଼ିକ[i], eol);
		} ଅନ୍ୟଥା
			printf("%s%c", ଧାଡ଼ିଗୁଡ଼ିକ[i], eol);
	}

	fflush_stdout_and_exit_SUCCESS();
}
