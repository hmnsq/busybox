/* vi: set sw=4 ts=4: */
/*
 * cat implementation for busybox
 *
 * Copyright (C) 2003  Manuel Novoa III  <mjn3@codepoet.org>
 *
 * Licensed under GPLv2, see file LICENSE in this source tree.
 */
//config:config CAT
//config:	bool "cat (5.8 kb)"
//config:	default y
//config:	help
//config:	cat is used to concatenate files and print them to the standard
//config:	output. Enable this option if you wish to enable the 'cat' utility.
//config:
//config:config FEATURE_CATN
//config:	bool "Enable -n and -b options"
//config:	default y
//config:	depends on CAT
//config:	help
//config:	-n numbers all output lines while -b numbers nonempty output lines.
//config:
//config:config FEATURE_CATV
//config:	bool "cat -v[etA]"
//config:	default y
//config:	depends on CAT
//config:	help
//config:	Display nonprinting characters as escape sequences

//applet:IF_CAT(APPLET(cat, BB_DIR_BIN, BB_SUID_DROP))

//kbuild:lib-$(CONFIG_CAT) += cat.o

/* BB_AUDIT SUSv3 compliant */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/cat.html */

//usage:#if ENABLE_FEATURE_CATN || ENABLE_FEATURE_CATV
//usage:#define cat_trivial_usage
//usage:       "[-" IF_FEATURE_CATN("nb") IF_FEATURE_CATV("vteA") "] [FILE]..."
//usage:#else
//usage:#define cat_trivial_usage
//usage:       "[FILE]..."
//usage:#endif
//usage:#define cat_full_usage "\n\n"
//usage:       "Print FILEs to stdout\n"
//usage:	IF_FEATURE_CATN(
//usage:     "\n	-n	Number output lines"
//usage:     "\n	-b	Number nonempty lines"
//usage:	)
//usage:	IF_FEATURE_CATV(
//usage:     "\n	-v	Show nonprinting characters as ^x or M-x"
//usage:     "\n	-t	...and tabs as ^I"
//usage:     "\n	-e	...and end lines with $"
//usage:     "\n	-A	Same as -vte"
//usage:	)
/*
  Longopts not implemented yet:
      --number-nonblank    number nonempty output lines, overrides -n
      --number             number all output lines
      --show-nonprinting   use ^ and M- notation, except for LFD and TAB
      --show-all           equivalent to -vet
  Not implemented yet:
  -E, --show-ends          display $ at end of each line (-e sans -v)
  -T, --show-tabs          display TAB characters as ^I (-t sans -v)
  -s, --squeeze-blank      suppress repeated empty output lines
*/
//usage:
//usage:#define cat_example_usage
//usage:       "$ cat /proc/uptime\n"
//usage:       "110716.72 17.67"

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "common_bufsiz.h"

#ਜੇ ENABLE_FEATURE_CATV
/*
 * cat -v implementation for busybox
 *
 * Copyright (C) 2006 Rob Landley <rob@landley.net>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
/* Rob had "cat -v" implemented as a separate applet, catv.
 * See "cat -v considered harmful" at
 * http://cm.bell-labs.com/cm/cs/doc/84/kp.ps.gz
 * From USENIX Summer Conference Proceedings, 1983
 * """
 * The talk reviews reasons for UNIX's popularity and shows, using UCB cat
 * as a primary example, how UNIX has grown fat. cat isn't for printing
 * files with line numbers, it isn't for compressing multiple blank lines,
 * it's not for looking at non-printing ASCII characters, it's for
 * concatenating files.
 * We are reminded that ls isn't the place for code to break a single column
 * into multiple ones, and that mailnews shouldn't have its own more
 * processing or joke encryption code.
 * """
 *
 * I agree with the argument. Unfortunately, this ship has sailed (1983...).
 * There are dozens of Linux distros and each of them has "cat" which supports -v.
 * It's unrealistic for us to "reeducate" them to use our, incompatible way
 * to achieve "cat -v" effect. The actual effect would be "users pissed off
 * by gratuitous incompatibility".
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CAT_OPT_e (1<<0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CAT_OPT_t (1<<1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CAT_OPT_v (1<<2)
/* -A occupies bit (1<<3) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CAT_OPT_n ((1<<4) * ENABLE_FEATURE_CATN)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CAT_OPT_b ((1<<5) * ENABLE_FEATURE_CATN)
ਸਥਿਰ ਪੂਰਨ catv(ਬਿਨਾਨਿਸ਼ਾਨੀ opts, ਅੱਖਰ **argv)
{
	ਪੂਰਨ retval = EXIT_SUCCESS;
	ਪੂਰਨ fd;
#ਜੇ ENABLE_FEATURE_CATN
	bool eol_ਵੇਖਿਆ = (opts & (CAT_OPT_n|CAT_OPT_b));
	ਬਿਨਾਨਿਸ਼ਾਨੀ eol_char = (eol_ਵੇਖਿਆ ? '\n' : 0x100);
	ਬਿਨਾਨਿਸ਼ਾਨੀ skip_ਅੰਕ_on = (opts & CAT_OPT_b) ? '\n' : 0x100;
	ਬਿਨਾਨਿਸ਼ਾਨੀ lineno = 0;
#ਜੇ_ਅੰਤ

	BUILD_BUG_ON(CAT_OPT_e != VISIBLE_ENDLINE);
	BUILD_BUG_ON(CAT_OPT_t != VISIBLE_SHOW_TABS);
#ਜੇ 0 /* These consts match, we can just pass "opts" to visible() */
	ਜੇ (opts & CAT_OPT_e)
		flags |= VISIBLE_ENDLINE;
	ਜੇ (opts & CAT_OPT_t)
		flags |= VISIBLE_SHOW_TABS;
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ read_buf bb_common_bufsiz1
	ਤਿਆਰੀ_common_bufsiz();
	ਕਰੋ {
		fd = open_or_warn_stdin(*argv);
		ਜੇ (fd < 0) {
			retval = EXIT_FAILURE;
			ਜਾਰੀ;
		}
		ਲਈ (;;) {
			ਪੂਰਨ i, res;

			res = ਪਠ(fd, read_buf, COMMON_BUFSIZE);
			ਜੇ (res < 0)
				retval = EXIT_FAILURE;
			ਜੇ (res <= 0)
				ਰੋਕੋ;
			ਲਈ (i = 0; i < res; i++) {
				ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ c = read_buf[i];
				ਅੱਖਰ buf[ਆਕਾਰ("M-^c")];
#ਜੇ ENABLE_FEATURE_CATN
				ਜੇ (eol_ਵੇਖਿਆ && c != skip_ਅੰਕ_on)
					ਲਿਖੋ("%6u  ", ++lineno);
				eol_ਵੇਖਿਆ = (c == eol_char);
#ਜੇ_ਅੰਤ
				visible(c, buf, opts);
				fputs_stdout(buf);
			}
		}
		ਜੇ (ENABLE_FEATURE_CLEAN_ਉੱਪਰ && fd)
			close(fd);
	} ਜਦੋਂ (*++argv);

	fflush_stdout_and_exit(retval);
}
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ CAT_OPT_n
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ CAT_OPT_b
#ਜੇ_ਅੰਤ

ਪੂਰਨ cat_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ cat_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
#ਜੇ ENABLE_FEATURE_CATV || ENABLE_FEATURE_CATN
	ਬਿਨਾਨਿਸ਼ਾਨੀ opts;

	opts =
#ਜੇ_ਅੰਤ
	getopt32(argv, IF_FEATURE_CATV("^")
		/* -u is ignored ("unbuffered") */
		IF_FEATURE_CATV("etvA")IF_FEATURE_CATN("nb")"u"
		IF_FEATURE_CATV("\0" "Aetv" /* -A == -vet */)
	);
	argv += optind;

	/* Read from stdin if there's nothing else to do. */
	ਜੇ (!argv[0])
		*--argv = (ਅੱਖਰ*)"-";

#ਜੇ ENABLE_FEATURE_CATV
	ਜੇ (opts & 7)
		ਵਾਪਸ catv(opts, argv);
	opts >>= 4;
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_FEATURE_CATN
# define CAT_OPT_n (1<<0)
# define CAT_OPT_b (1<<1)
	ਜੇ (opts & (CAT_OPT_n|CAT_OPT_b)) { /* -n or -b */
		ਬਣਤਰ ਸੰਖਿਆ_state ns;
		ਪੂਰਨ exitcode;

		ns.ਚੌੜਾਈ = 6;
		ns.ਸ਼ੁਰੂ = 1;
		ns.inc = 1;
		ns.sep = "\t";
		ns.ਖਾਲੀ_ਥਾਵਾਂ_str = ਨਿਰਾ;
		ns.all = !(opts & CAT_OPT_b); /* -n without -b */
		ns.nonempty = (opts & CAT_OPT_b); /* -b (with or without -n) */
		exitcode = EXIT_SUCCESS;
		ਕਰੋ {
			exitcode |= print_numbered_ਸਤਰਾਂ(&ns, *argv);
		} ਜਦੋਂ (*++argv);
		fflush_stdout_and_exit(exitcode);
	}
	/*opts >>= 2;*/
#ਜੇ_ਅੰਤ

	ਵਾਪਸ bb_cat(argv);
}
