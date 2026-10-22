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

#अन्तर्भाव "libbb.h"

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
स्थिर शून्य shuffle_पङ्क्तयः(वर्ण **पङ्क्तयः, unsigned numlines, unsigned outlines)
{
	srand(monotonic_us());

	जाव (outlines != 0) {
		वर्ण *tmp;
		unsigned r = rand();
		/* RAND_MAX can be as small as 32767 */
		जइ (numlines > RAND_MAX)
			r ^= rand() << 15;
		r %= numlines;
//TODO: the above method is seriously non-uniform when numlines is very large.
//For example, with numlines of   0xf0000000,
//values of (r % numlines) in [0, 0x0fffffff] range
//are more likely: e.g. r=1 and r=0xf0000001 both map to 1,
//whereas only one value, r=0xefffffff, maps to 0xefffffff.
		numlines--;
		tmp = पङ्क्तयः[numlines];
		पङ्क्तयः[numlines] = पङ्क्तयः[r];
		पङ्क्तयः[r] = tmp;
		outlines--;
	}
}

/* We can handle insanity like this:
 * shuf -i 333333333333333333333333333333001-333333333333333333333333333333019
 * but do we want to have +200 bytes of code (~40% code growth)?
 */
#define COMMON_PREFIX_HACK 0

पूर्णाङ्क shuf_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क shuf_main(पूर्णाङ्क argc, वर्ण **argv)
{
	unsigned opts;
	वर्ण *opt_i_str, *opt_n_str, *opt_o_str;
	वर्ण **पङ्क्तयः;
	unsigned long long lo = lo;
	unsigned numlines, outlines;
	unsigned i;
	वर्ण eol;
#जइ COMMON_PREFIX_HACK
	unsigned pfx_len = 0;
	unsigned padding_width = padding_width;
	ध्रुव वर्ण *pfx = pfx;
#endif

	opts = getopt32(argv, "^"
			OPT_STR
			"\0" "e--i:i--e"/* mutually exclusive */,
			&opt_i_str, &opt_n_str, &opt_o_str
	);

	argc -= optind;
	argv += optind;

	/* Prepare lines for shuffling - either: */
	जइ (opts & OPT_e) {
		/* make lines from command-line arguments */
		numlines = argc;
		पङ्क्तयः = argv;
	} अण्णहा
	जइ (opts & OPT_i) {
		/* create a range of numbers */
		unsigned long long hi;
		वर्ण *dash;

		जइ (argv[0])
			bb_show_usage();

		dash = strchr(opt_i_str, '-');
		जइ (!dash) {
			bb_error_सन्देशः_and_die("bad range '%s'", opt_i_str);
		}
		*dash++ = '\0';
#जइ COMMON_PREFIX_HACK
		{
			ध्रुव वर्ण *a = opt_i_str;
			ध्रुव वर्ण *b = dash;
			/* Skip leading zeros (they may mask that common prefix does exist) */
			जाव (*a == '0') a++;
			जाव (*b == '0') b++;
			/* Do we have a common prefix (long enough to bother)? */
			padding_width = strlen(a);
			जइ (padding_width > 5 && padding_width == strlen(b)) {
				/* How long is it? */
				pfx = a;
				जाव (isdigit(*a) && *a == *b
				 && a[1] /* "111111-111111" case: avoid xatoull("") */
				) {
					a++;
					b++;
				}
				pfx_len = a - pfx; /* can end up being 0 */
				padding_width -= pfx_len;
			} अण्णहा {
				/* Undo leading zero 'eating' (think "0-9") */
				a = opt_i_str;
				b = dash;
			}
			lo = xatoull(a);
			hi = xatoull(b);
		}
#अण्णहा
		lo = xatoull(opt_i_str);
		hi = xatoull(dash);
#endif
		dash[-1] = '-';
		जइ (hi < lo)
			bb_error_सन्देशः_and_die("bad range '%s'", opt_i_str);
		hi -= lo;
		जइ (परिमाण(size_t) > परिमाण(numlines)) {
			जइ (hi >= UINT_MAX)
				bb_error_सन्देशः_and_die("bad range '%s'", opt_i_str);
		} अण्णहा {
			जइ (hi >= UINT_MAX / परिमाण(पङ्क्तयः[0]))
				bb_error_सन्देशः_and_die("bad range '%s'", opt_i_str);
		}

		numlines = hi + 1;
		पङ्क्तयः = xmalloc((size_t)numlines * परिमाण(पङ्क्तयः[0]));
		प्रति (i = 0; i < numlines; i++) {
			पङ्क्तयः[i] = (वर्ण*)(uintptr_t)i;
		}
	} अण्णहा {
		/* default - read lines from stdin or the input file */
		FILE *fp;
		ध्रुव वर्ण *fname = "-";

		जइ (argv[0]) {
			जइ (argv[1])
				bb_show_usage();
			fname = argv[0];
		}

		fp = xfopen_stdin(fname);
		पङ्क्तयः = रिक्त;
		numlines = 0;
		प्रति (;;) {
			वर्ण *पङ्क्तिः = xmalloc_fgetline(fp);
			जइ (!पङ्क्तिः)
				भङ्ग;
			पङ्क्तयः = xrealloc_vector(पङ्क्तयः, 6, numlines);
			पङ्क्तयः[numlines++] = पङ्क्तिः;
		}
		fclose_if_not_stdin(fp);
	}

	outlines = numlines;
	जइ (opts & OPT_n) {
		outlines = xatou(opt_n_str);
		जइ (outlines > numlines)
			outlines = numlines;
	}

	shuffle_पङ्क्तयः(पङ्क्तयः, numlines, outlines);

	जइ (opts & OPT_o)
		xmove_fd(xopen(opt_o_str, O_WRONLY|O_CREAT|O_TRUNC), STDOUT_FILENO);

	eol = '\n';
	जइ (opts & OPT_z)
		eol = '\0';

	प्रति (i = numlines - outlines; i < numlines; i++) {
		जइ (opts & OPT_i) {
#जइ COMMON_PREFIX_HACK
			जइ (pfx_len != 0)
				printf("%.*s%0*llu%c", pfx_len, pfx, padding_width, lo + (uintptr_t)पङ्क्तयः[i], eol);
			अण्णहा
#endif
				printf("%llu%c", lo + (uintptr_t)पङ्क्तयः[i], eol);
		} अण्णहा
			printf("%s%c", पङ्क्तयः[i], eol);
	}

	fflush_stdout_and_exit_SUCCESS();
}
