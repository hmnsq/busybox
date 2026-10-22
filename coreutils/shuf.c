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

#অন্তর্ভাব "libbb.h"

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
স্থির শূন্য shuffle_সারিগুলি(বর্ণ **সারিগুলি, unsigned numlines, unsigned outlines)
{
	srand(monotonic_us());

	যাবত্ (outlines != 0) {
		বর্ণ *tmp;
		unsigned r = rand();
		/* RAND_MAX can be as small as 32767 */
		যদি (numlines > RAND_MAX)
			r ^= rand() << 15;
		r %= numlines;
//TODO: the above method is seriously non-uniform when numlines is very large.
//For example, with numlines of   0xf0000000,
//values of (r % numlines) in [0, 0x0fffffff] range
//are more likely: e.g. r=1 and r=0xf0000001 both map to 1,
//whereas only one value, r=0xefffffff, maps to 0xefffffff.
		numlines--;
		tmp = সারিগুলি[numlines];
		সারিগুলি[numlines] = সারিগুলি[r];
		সারিগুলি[r] = tmp;
		outlines--;
	}
}

/* We can handle insanity like this:
 * shuf -i 333333333333333333333333333333001-333333333333333333333333333333019
 * but do we want to have +200 bytes of code (~40% code growth)?
 */
#define COMMON_PREFIX_HACK 0

পূর্ণাঙ্ক shuf_main(পূর্ণাঙ্ক argc, বর্ণ **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণাঙ্ক shuf_main(পূর্ণাঙ্ক argc, বর্ণ **argv)
{
	unsigned opts;
	বর্ণ *opt_i_str, *opt_n_str, *opt_o_str;
	বর্ণ **সারিগুলি;
	unsigned long long lo = lo;
	unsigned numlines, outlines;
	unsigned i;
	বর্ণ eol;
#যদি COMMON_PREFIX_HACK
	unsigned pfx_len = 0;
	unsigned padding_width = padding_width;
	ধ্রুব বর্ণ *pfx = pfx;
#endif

	opts = getopt32(argv, "^"
			OPT_STR
			"\0" "e--i:i--e"/* mutually exclusive */,
			&opt_i_str, &opt_n_str, &opt_o_str
	);

	argc -= optind;
	argv += optind;

	/* Prepare lines for shuffling - either: */
	যদি (opts & OPT_e) {
		/* make lines from command-line arguments */
		numlines = argc;
		সারিগুলি = argv;
	} অন্যথা
	যদি (opts & OPT_i) {
		/* create a range of numbers */
		unsigned long long hi;
		বর্ণ *dash;

		যদি (argv[0])
			bb_show_usage();

		dash = strchr(opt_i_str, '-');
		যদি (!dash) {
			bb_error_বার্তা_and_die("bad range '%s'", opt_i_str);
		}
		*dash++ = '\0';
#যদি COMMON_PREFIX_HACK
		{
			ধ্রুব বর্ণ *a = opt_i_str;
			ধ্রুব বর্ণ *b = dash;
			/* Skip leading zeros (they may mask that common prefix does exist) */
			যাবত্ (*a == '0') a++;
			যাবত্ (*b == '0') b++;
			/* Do we have a common prefix (long enough to bother)? */
			padding_width = strlen(a);
			যদি (padding_width > 5 && padding_width == strlen(b)) {
				/* How long is it? */
				pfx = a;
				যাবত্ (isdigit(*a) && *a == *b
				 && a[1] /* "111111-111111" case: avoid xatoull("") */
				) {
					a++;
					b++;
				}
				pfx_len = a - pfx; /* can end up being 0 */
				padding_width -= pfx_len;
			} অন্যথা {
				/* Undo leading zero 'eating' (think "0-9") */
				a = opt_i_str;
				b = dash;
			}
			lo = xatoull(a);
			hi = xatoull(b);
		}
#অন্যথা
		lo = xatoull(opt_i_str);
		hi = xatoull(dash);
#endif
		dash[-1] = '-';
		যদি (hi < lo)
			bb_error_বার্তা_and_die("bad range '%s'", opt_i_str);
		hi -= lo;
		যদি (পরিমাণ(size_t) > পরিমাণ(numlines)) {
			যদি (hi >= UINT_MAX)
				bb_error_বার্তা_and_die("bad range '%s'", opt_i_str);
		} অন্যথা {
			যদি (hi >= UINT_MAX / পরিমাণ(সারিগুলি[0]))
				bb_error_বার্তা_and_die("bad range '%s'", opt_i_str);
		}

		numlines = hi + 1;
		সারিগুলি = xmalloc((size_t)numlines * পরিমাণ(সারিগুলি[0]));
		প্রতি (i = 0; i < numlines; i++) {
			সারিগুলি[i] = (বর্ণ*)(uintptr_t)i;
		}
	} অন্যথা {
		/* default - read lines from stdin or the input file */
		FILE *fp;
		ধ্রুব বর্ণ *fname = "-";

		যদি (argv[0]) {
			যদি (argv[1])
				bb_show_usage();
			fname = argv[0];
		}

		fp = xfopen_stdin(fname);
		সারিগুলি = রিক্ত;
		numlines = 0;
		প্রতি (;;) {
			বর্ণ *সারি = xmalloc_fgetline(fp);
			যদি (!সারি)
				ভঙ্গ;
			সারিগুলি = xrealloc_vector(সারিগুলি, 6, numlines);
			সারিগুলি[numlines++] = সারি;
		}
		fclose_if_not_stdin(fp);
	}

	outlines = numlines;
	যদি (opts & OPT_n) {
		outlines = xatou(opt_n_str);
		যদি (outlines > numlines)
			outlines = numlines;
	}

	shuffle_সারিগুলি(সারিগুলি, numlines, outlines);

	যদি (opts & OPT_o)
		xmove_fd(xopen(opt_o_str, O_WRONLY|O_CREAT|O_TRUNC), STDOUT_FILENO);

	eol = '\n';
	যদি (opts & OPT_z)
		eol = '\0';

	প্রতি (i = numlines - outlines; i < numlines; i++) {
		যদি (opts & OPT_i) {
#যদি COMMON_PREFIX_HACK
			যদি (pfx_len != 0)
				printf("%.*s%0*llu%c", pfx_len, pfx, padding_width, lo + (uintptr_t)সারিগুলি[i], eol);
			অন্যথা
#endif
				printf("%llu%c", lo + (uintptr_t)সারিগুলি[i], eol);
		} অন্যথা
			printf("%s%c", সারিগুলি[i], eol);
	}

	fflush_stdout_and_exit_SUCCESS();
}
