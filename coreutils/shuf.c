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
स्थिर शून्य shuffle_पंक्तियाँ(वर्ण **पंक्तियाँ, अचिन्हित numlines, अचिन्हित outlines)
{
	बेक्रम(monotonic_us());

	यावत् (outlines != 0) {
		वर्ण *tmp;
		अचिन्हित r = अक्रम();
		/* RAND_MAX can be as small as 32767 */
		यदि (numlines > अक्रम_उच्च)
			r ^= अक्रम() << 15;
		r %= numlines;
//TODO: the above method is seriously non-uniform when numlines is very large.
//For example, with numlines of   0xf0000000,
//values of (r % numlines) in [0, 0x0fffffff] range
//are more likely: e.g. r=1 and r=0xf0000001 both map to 1,
//whereas only one value, r=0xefffffff, maps to 0xefffffff.
		numlines--;
		tmp = पंक्तियाँ[numlines];
		पंक्तियाँ[numlines] = पंक्तियाँ[r];
		पंक्तियाँ[r] = tmp;
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
	अचिन्हित opts;
	वर्ण *opt_i_str, *opt_n_str, *opt_o_str;
	वर्ण **पंक्तियाँ;
	अचिन्हित दीर्घ दीर्घ lo = lo;
	अचिन्हित numlines, outlines;
	अचिन्हित i;
	वर्ण eol;
#यदि COMMON_PREFIX_HACK
	अचिन्हित pfx_len = 0;
	अचिन्हित padding_width = padding_width;
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
	यदि (opts & OPT_e) {
		/* make lines from command-line arguments */
		numlines = argc;
		पंक्तियाँ = argv;
	} अन्यथा
	यदि (opts & OPT_i) {
		/* create a range of numbers */
		अचिन्हित दीर्घ दीर्घ hi;
		वर्ण *dash;

		यदि (argv[0])
			bb_show_usage();

		dash = म_अक्षर(opt_i_str, '-');
		यदि (!dash) {
			bb_error_संदेश_and_die("bad range '%s'", opt_i_str);
		}
		*dash++ = '\0';
#यदि COMMON_PREFIX_HACK
		{
			ध्रुव वर्ण *a = opt_i_str;
			ध्रुव वर्ण *b = dash;
			/* Skip leading zeros (they may mask that common prefix does exist) */
			यावत् (*a == '0') a++;
			यावत् (*b == '0') b++;
			/* Do we have a common prefix (long enough to bother)? */
			padding_width = म_माप(a);
			यदि (padding_width > 5 && padding_width == म_माप(b)) {
				/* How long is it? */
				pfx = a;
				यावत् (है_अंक(*a) && *a == *b
				 && a[1] /* "111111-111111" case: avoid xatoull("") */
				) {
					a++;
					b++;
				}
				pfx_len = a - pfx; /* can end up being 0 */
				padding_width -= pfx_len;
			} अन्यथा {
				/* Undo leading zero 'eating' (think "0-9") */
				a = opt_i_str;
				b = dash;
			}
			lo = xatoull(a);
			hi = xatoull(b);
		}
#अन्यथा
		lo = xatoull(opt_i_str);
		hi = xatoull(dash);
#endif
		dash[-1] = '-';
		यदि (hi < lo)
			bb_error_संदेश_and_die("bad range '%s'", opt_i_str);
		hi -= lo;
		यदि (परिमाण(माप_प्रकार) > परिमाण(numlines)) {
			यदि (hi >= अच_पूर्णांक_उच्च)
				bb_error_संदेश_and_die("bad range '%s'", opt_i_str);
		} अन्यथा {
			यदि (hi >= अच_पूर्णांक_उच्च / परिमाण(पंक्तियाँ[0]))
				bb_error_संदेश_and_die("bad range '%s'", opt_i_str);
		}

		numlines = hi + 1;
		पंक्तियाँ = xmalloc((माप_प्रकार)numlines * परिमाण(पंक्तियाँ[0]));
		प्रति (i = 0; i < numlines; i++) {
			पंक्तियाँ[i] = (वर्ण*)(uintptr_t)i;
		}
	} अन्यथा {
		/* default - read lines from stdin or the input file */
		खाता *fp;
		ध्रुव वर्ण *fname = "-";

		यदि (argv[0]) {
			यदि (argv[1])
				bb_show_usage();
			fname = argv[0];
		}

		fp = xfopen_stdin(fname);
		पंक्तियाँ = रिक्त;
		numlines = 0;
		प्रति (;;) {
			वर्ण *पंक्ति = xmalloc_fgetline(fp);
			यदि (!पंक्ति)
				भङ्ग;
			पंक्तियाँ = xrealloc_vector(पंक्तियाँ, 6, numlines);
			पंक्तियाँ[numlines++] = पंक्ति;
		}
		fclose_if_not_stdin(fp);
	}

	outlines = numlines;
	यदि (opts & OPT_n) {
		outlines = xatou(opt_n_str);
		यदि (outlines > numlines)
			outlines = numlines;
	}

	shuffle_पंक्तियाँ(पंक्तियाँ, numlines, outlines);

	यदि (opts & OPT_o)
		xmove_fd(xopen(opt_o_str, O_WRONLY|O_CREAT|O_TRUNC), STDOUT_FILENO);

	eol = '\n';
	यदि (opts & OPT_z)
		eol = '\0';

	प्रति (i = numlines - outlines; i < numlines; i++) {
		यदि (opts & OPT_i) {
#यदि COMMON_PREFIX_HACK
			यदि (pfx_len != 0)
				म_लिखो("%.*s%0*llu%c", pfx_len, pfx, padding_width, lo + (uintptr_t)पंक्तियाँ[i], eol);
			अन्यथा
#endif
				म_लिखो("%llu%c", lo + (uintptr_t)पंक्तियाँ[i], eol);
		} अन्यथा
			म_लिखो("%s%c", पंक्तियाँ[i], eol);
	}

	fflush_stdout_and_exit_SUCCESS();
}
