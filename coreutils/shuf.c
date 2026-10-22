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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPT_e		(1 << 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPT_i		(1 << 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPT_n		(1 << 2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPT_o		(1 << 3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPT_z		(1 << 4)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPT_STR		"ei:n:o:z"

/*
 * Use the Fisher-Yates shuffle algorithm on an array of lines.
 * If the required number of output lines is less than the total
 * we can stop shuffling early.
 */
ਸਥਿਰ ਖਾਲੀ shuffle_ਸਤਰਾਂ(ਅੱਖਰ **ਸਤਰਾਂ, ਬਿਨਾਨਿਸ਼ਾਨੀ numlines, ਬਿਨਾਨਿਸ਼ਾਨੀ outlines)
{
	srand(monotonic_us());

	ਜਦੋਂ (outlines != 0) {
		ਅੱਖਰ *tmp;
		ਬਿਨਾਨਿਸ਼ਾਨੀ r = rand();
		/* RAND_MAX can be as small as 32767 */
		ਜੇ (numlines > RAND_MAX)
			r ^= rand() << 15;
		r %= numlines;
//TODO: the above method is seriously non-uniform when numlines is very large.
//For example, with numlines of   0xf0000000,
//values of (r % numlines) in [0, 0x0fffffff] range
//are more likely: e.g. r=1 and r=0xf0000001 both map to 1,
//whereas only one value, r=0xefffffff, maps to 0xefffffff.
		numlines--;
		tmp = ਸਤਰਾਂ[numlines];
		ਸਤਰਾਂ[numlines] = ਸਤਰਾਂ[r];
		ਸਤਰਾਂ[r] = tmp;
		outlines--;
	}
}

/* We can handle insanity like this:
 * shuf -i 333333333333333333333333333333001-333333333333333333333333333333019
 * but do we want to have +200 bytes of code (~40% code growth)?
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COMMON_ਅਗੇਤਰ_HACK 0

ਪੂਰਨ shuf_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ shuf_main(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ opts;
	ਅੱਖਰ *opt_i_str, *opt_n_str, *opt_o_str;
	ਅੱਖਰ **ਸਤਰਾਂ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ lo = lo;
	ਬਿਨਾਨਿਸ਼ਾਨੀ numlines, outlines;
	ਬਿਨਾਨਿਸ਼ਾਨੀ i;
	ਅੱਖਰ eol;
#ਜੇ COMMON_ਅਗੇਤਰ_HACK
	ਬਿਨਾਨਿਸ਼ਾਨੀ pfx_len = 0;
	ਬਿਨਾਨਿਸ਼ਾਨੀ padding_ਚੌੜਾਈ = padding_ਚੌੜਾਈ;
	ਅਚਲ ਅੱਖਰ *pfx = pfx;
#ਜੇ_ਅੰਤ

	opts = getopt32(argv, "^"
			OPT_STR
			"\0" "e--i:i--e"/* mutually exclusive */,
			&opt_i_str, &opt_n_str, &opt_o_str
	);

	argc -= optind;
	argv += optind;

	/* Prepare lines for shuffling - either: */
	ਜੇ (opts & OPT_e) {
		/* make lines from command-line arguments */
		numlines = argc;
		ਸਤਰਾਂ = argv;
	} ਨਹੀਂਤਾਂ
	ਜੇ (opts & OPT_i) {
		/* create a range of numbers */
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ hi;
		ਅੱਖਰ *dash;

		ਜੇ (argv[0])
			bb_show_usage();

		dash = strchr(opt_i_str, '-');
		ਜੇ (!dash) {
			bb_error_ਸੁਨੇਹਾ_and_die("bad range '%s'", opt_i_str);
		}
		*dash++ = '\0';
#ਜੇ COMMON_ਅਗੇਤਰ_HACK
		{
			ਅਚਲ ਅੱਖਰ *a = opt_i_str;
			ਅਚਲ ਅੱਖਰ *b = dash;
			/* Skip leading zeros (they may mask that common prefix does exist) */
			ਜਦੋਂ (*a == '0') a++;
			ਜਦੋਂ (*b == '0') b++;
			/* Do we have a common prefix (long enough to bother)? */
			padding_ਚੌੜਾਈ = ਲੜੀ_ਲੰਬਾਈ(a);
			ਜੇ (padding_ਚੌੜਾਈ > 5 && padding_ਚੌੜਾਈ == ਲੜੀ_ਲੰਬਾਈ(b)) {
				/* How long is it? */
				pfx = a;
				ਜਦੋਂ (ਅੰਕ_ਹੈ(*a) && *a == *b
				 && a[1] /* "111111-111111" case: avoid xatoull("") */
				) {
					a++;
					b++;
				}
				pfx_len = a - pfx; /* can end up being 0 */
				padding_ਚੌੜਾਈ -= pfx_len;
			} ਨਹੀਂਤਾਂ {
				/* Undo leading zero 'eating' (think "0-9") */
				a = opt_i_str;
				b = dash;
			}
			lo = xatoull(a);
			hi = xatoull(b);
		}
#ਨਹੀਂਤਾਂ
		lo = xatoull(opt_i_str);
		hi = xatoull(dash);
#ਜੇ_ਅੰਤ
		dash[-1] = '-';
		ਜੇ (hi < lo)
			bb_error_ਸੁਨੇਹਾ_and_die("bad range '%s'", opt_i_str);
		hi -= lo;
		ਜੇ (ਆਕਾਰ(ਮਾਪ_t) > ਆਕਾਰ(numlines)) {
			ਜੇ (hi >= UINT_MAX)
				bb_error_ਸੁਨੇਹਾ_and_die("bad range '%s'", opt_i_str);
		} ਨਹੀਂਤਾਂ {
			ਜੇ (hi >= UINT_MAX / ਆਕਾਰ(ਸਤਰਾਂ[0]))
				bb_error_ਸੁਨੇਹਾ_and_die("bad range '%s'", opt_i_str);
		}

		numlines = hi + 1;
		ਸਤਰਾਂ = xmalloc((ਮਾਪ_t)numlines * ਆਕਾਰ(ਸਤਰਾਂ[0]));
		ਲਈ (i = 0; i < numlines; i++) {
			ਸਤਰਾਂ[i] = (ਅੱਖਰ*)(uintptr_t)i;
		}
	} ਨਹੀਂਤਾਂ {
		/* default - read lines from stdin or the input file */
		FILE *fp;
		ਅਚਲ ਅੱਖਰ *fname = "-";

		ਜੇ (argv[0]) {
			ਜੇ (argv[1])
				bb_show_usage();
			fname = argv[0];
		}

		fp = xfopen_stdin(fname);
		ਸਤਰਾਂ = ਨਿਰਾ;
		numlines = 0;
		ਲਈ (;;) {
			ਅੱਖਰ *ਸਤਰ = xmalloc_fgetline(fp);
			ਜੇ (!ਸਤਰ)
				ਰੋਕੋ;
			ਸਤਰਾਂ = xrealloc_ਸਦਿਸ਼(ਸਤਰਾਂ, 6, numlines);
			ਸਤਰਾਂ[numlines++] = ਸਤਰ;
		}
		fclose_if_not_stdin(fp);
	}

	outlines = numlines;
	ਜੇ (opts & OPT_n) {
		outlines = xatou(opt_n_str);
		ਜੇ (outlines > numlines)
			outlines = numlines;
	}

	shuffle_ਸਤਰਾਂ(ਸਤਰਾਂ, numlines, outlines);

	ਜੇ (opts & OPT_o)
		xmove_fd(xopen(opt_o_str, O_WRONLY|O_CREAT|O_TRUNC), STDOUT_FILENO);

	eol = '\n';
	ਜੇ (opts & OPT_z)
		eol = '\0';

	ਲਈ (i = numlines - outlines; i < numlines; i++) {
		ਜੇ (opts & OPT_i) {
#ਜੇ COMMON_ਅਗੇਤਰ_HACK
			ਜੇ (pfx_len != 0)
				ਲਿਖੋ("%.*s%0*llu%c", pfx_len, pfx, padding_ਚੌੜਾਈ, lo + (uintptr_t)ਸਤਰਾਂ[i], eol);
			ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
				ਲਿਖੋ("%llu%c", lo + (uintptr_t)ਸਤਰਾਂ[i], eol);
		} ਨਹੀਂਤਾਂ
			ਲਿਖੋ("%s%c", ਸਤਰਾਂ[i], eol);
	}

	fflush_stdout_and_exit_SUCCESS();
}
