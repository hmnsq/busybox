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

#ضمن "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

#تعریف_کن OPT_e		(1 << 0)
#تعریف_کن OPT_i		(1 << 1)
#تعریف_کن OPT_n		(1 << 2)
#تعریف_کن OPT_o		(1 << 3)
#تعریف_کن OPT_z		(1 << 4)
#تعریف_کن OPT_STR		"ei:n:o:z"

/*
 * Use the Fisher-Yates shuffle algorithm on an array of lines.
 * If the required number of output lines is less than the total
 * we can stop shuffling early.
 */
ساكن فراغ shuffle_سطرها(حرف **سطرها, بیعلامت numlines, بیعلامت outlines)
{
	srand(monotonic_us());

	طالما (outlines != 0) {
		حرف *tmp;
		بیعلامت r = rand();
		/* RAND_MAX can be as small as 32767 */
		إذا (numlines > RAND_MAX)
			r ^= rand() << 15;
		r %= numlines;
//TODO: the above method is seriously non-uniform when numlines is very large.
//For example, with numlines of   0xf0000000,
//values of (r % numlines) in [0, 0x0fffffff] range
//are more likely: e.g. r=1 and r=0xf0000001 both map to 1,
//whereas only one value, r=0xefffffff, maps to 0xefffffff.
		numlines--;
		tmp = سطرها[numlines];
		سطرها[numlines] = سطرها[r];
		سطرها[r] = tmp;
		outlines--;
	}
}

/* We can handle insanity like this:
 * shuf -i 333333333333333333333333333333001-333333333333333333333333333333019
 * but do we want to have +200 bytes of code (~40% code growth)?
 */
#تعریف_کن COMMON_پیشوند_HACK 0

صحيح shuf_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح shuf_main(صحيح argc, حرف **argv)
{
	بیعلامت opts;
	حرف *opt_i_str, *opt_n_str, *opt_o_str;
	حرف **سطرها;
	بیعلامت دراز دراز lo = lo;
	بیعلامت numlines, outlines;
	بیعلامت i;
	حرف eol;
#إذا COMMON_پیشوند_HACK
	بیعلامت pfx_len = 0;
	بیعلامت padding_عرض = padding_عرض;
	ثابت حرف *pfx = pfx;
#پایان_اگر

	opts = getopt32(argv, "^"
			OPT_STR
			"\0" "e--i:i--e"/* mutually exclusive */,
			&opt_i_str, &opt_n_str, &opt_o_str
	);

	argc -= optind;
	argv += optind;

	/* Prepare lines for shuffling - either: */
	إذا (opts & OPT_e) {
		/* make lines from command-line arguments */
		numlines = argc;
		سطرها = argv;
	} وإلا
	إذا (opts & OPT_i) {
		/* create a range of numbers */
		بیعلامت دراز دراز hi;
		حرف *dash;

		إذا (argv[0])
			bb_show_usage();

		dash = strchr(opt_i_str, '-');
		إذا (!dash) {
			bb_error_پیام_and_die("bad range '%s'", opt_i_str);
		}
		*dash++ = '\0';
#إذا COMMON_پیشوند_HACK
		{
			ثابت حرف *a = opt_i_str;
			ثابت حرف *b = dash;
			/* Skip leading zeros (they may mask that common prefix does exist) */
			طالما (*a == '0') a++;
			طالما (*b == '0') b++;
			/* Do we have a common prefix (long enough to bother)? */
			padding_عرض = طول_رشته(a);
			إذا (padding_عرض > 5 && padding_عرض == طول_رشته(b)) {
				/* How long is it? */
				pfx = a;
				طالما (آیا_رقم(*a) && *a == *b
				 && a[1] /* "111111-111111" case: avoid xatoull("") */
				) {
					a++;
					b++;
				}
				pfx_len = a - pfx; /* can end up being 0 */
				padding_عرض -= pfx_len;
			} وإلا {
				/* Undo leading zero 'eating' (think "0-9") */
				a = opt_i_str;
				b = dash;
			}
			lo = xatoull(a);
			hi = xatoull(b);
		}
#وإلا
		lo = xatoull(opt_i_str);
		hi = xatoull(dash);
#پایان_اگر
		dash[-1] = '-';
		إذا (hi < lo)
			bb_error_پیام_and_die("bad range '%s'", opt_i_str);
		hi -= lo;
		إذا (حجم(اندازه_t) > حجم(numlines)) {
			إذا (hi >= UINT_MAX)
				bb_error_پیام_and_die("bad range '%s'", opt_i_str);
		} وإلا {
			إذا (hi >= UINT_MAX / حجم(سطرها[0]))
				bb_error_پیام_and_die("bad range '%s'", opt_i_str);
		}

		numlines = hi + 1;
		سطرها = xmalloc((اندازه_t)numlines * حجم(سطرها[0]));
		لكل (i = 0; i < numlines; i++) {
			سطرها[i] = (حرف*)(uintptr_t)i;
		}
	} وإلا {
		/* default - read lines from stdin or the input file */
		FILE *fp;
		ثابت حرف *fname = "-";

		إذا (argv[0]) {
			إذا (argv[1])
				bb_show_usage();
			fname = argv[0];
		}

		fp = xfopen_stdin(fname);
		سطرها = عدم;
		numlines = 0;
		لكل (;;) {
			حرف *سطر = xmalloc_fgetline(fp);
			إذا (!سطر)
				اكسر;
			سطرها = xrealloc_وکتور(سطرها, 6, numlines);
			سطرها[numlines++] = سطر;
		}
		fclose_if_not_stdin(fp);
	}

	outlines = numlines;
	إذا (opts & OPT_n) {
		outlines = xatou(opt_n_str);
		إذا (outlines > numlines)
			outlines = numlines;
	}

	shuffle_سطرها(سطرها, numlines, outlines);

	إذا (opts & OPT_o)
		xmove_fd(xopen(opt_o_str, O_WRONLY|O_CREAT|O_TRUNC), STDOUT_FILENO);

	eol = '\n';
	إذا (opts & OPT_z)
		eol = '\0';

	لكل (i = numlines - outlines; i < numlines; i++) {
		إذا (opts & OPT_i) {
#إذا COMMON_پیشوند_HACK
			إذا (pfx_len != 0)
				چاپ_کن("%.*s%0*llu%c", pfx_len, pfx, padding_عرض, lo + (uintptr_t)سطرها[i], eol);
			وإلا
#پایان_اگر
				چاپ_کن("%llu%c", lo + (uintptr_t)سطرها[i], eol);
		} وإلا
			چاپ_کن("%s%c", سطرها[i], eol);
	}

	fflush_stdout_and_exit_SUCCESS();
}
