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

#समावेश "libbb.h"
#समावेश "common_bufsiz.h"

#अगर ENABLE_FEATURE_CATV
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
#घोषणा CAT_OPT_e (1<<0)
#घोषणा CAT_OPT_t (1<<1)
#घोषणा CAT_OPT_v (1<<2)
/* -A occupies bit (1<<3) */
#घोषणा CAT_OPT_n ((1<<4) * ENABLE_FEATURE_CATN)
#घोषणा CAT_OPT_b ((1<<5) * ENABLE_FEATURE_CATN)
अटल पूर्णांक catv(अचिन्हित opts, अक्षर **argv)
{
	पूर्णांक retval = निकास_सफल;
	पूर्णांक fd;
#अगर ENABLE_FEATURE_CATN
	bool eol_seen = (opts & (CAT_OPT_n|CAT_OPT_b));
	अचिन्हित eol_char = (eol_seen ? '\n' : 0x100);
	अचिन्हित skip_अंक_on = (opts & CAT_OPT_b) ? '\n' : 0x100;
	अचिन्हित lineno = 0;
#पूर्ण_अगर

	BUILD_BUG_ON(CAT_OPT_e != VISIBLE_ENDLINE);
	BUILD_BUG_ON(CAT_OPT_t != VISIBLE_SHOW_TABS);
#अगर 0 /* These consts match, we can just pass "opts" to visible() */
	अगर (opts & CAT_OPT_e)
		flags |= VISIBLE_ENDLINE;
	अगर (opts & CAT_OPT_t)
		flags |= VISIBLE_SHOW_TABS;
#पूर्ण_अगर

#घोषणा read_buf bb_common_bufsiz1
	setup_common_bufsiz();
	करो {
		fd = open_or_warn_stdin(*argv);
		अगर (fd < 0) {
			retval = निकास_त्रुटि;
			जारी;
		}
		क्रम (;;) {
			पूर्णांक i, res;

			res = पढ़ो(fd, read_buf, COMMON_BUFSIZE);
			अगर (res < 0)
				retval = निकास_त्रुटि;
			अगर (res <= 0)
				अवरोध;
			क्रम (i = 0; i < res; i++) {
				अचिन्हित अक्षर c = read_buf[i];
				अक्षर buf[माप("M-^c")];
#अगर ENABLE_FEATURE_CATN
				अगर (eol_seen && c != skip_अंक_on)
					म_लिखो("%6u  ", ++lineno);
				eol_seen = (c == eol_char);
#पूर्ण_अगर
				visible(c, buf, opts);
				fputs_stdout(buf);
			}
		}
		अगर (ENABLE_FEATURE_CLEAN_UP && fd)
			बंद(fd);
	} जबतक (*++argv);

	fflush_stdout_and_exit(retval);
}
#अघोषित CAT_OPT_n
#अघोषित CAT_OPT_b
#पूर्ण_अगर

पूर्णांक cat_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक cat_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
#अगर ENABLE_FEATURE_CATV || ENABLE_FEATURE_CATN
	अचिन्हित opts;

	opts =
#पूर्ण_अगर
	getopt32(argv, IF_FEATURE_CATV("^")
		/* -u is ignored ("unbuffered") */
		IF_FEATURE_CATV("etvA")IF_FEATURE_CATN("nb")"u"
		IF_FEATURE_CATV("\0" "Aetv" /* -A == -vet */)
	);
	argv += optind;

	/* Read from stdin if there's nothing else to do. */
	अगर (!argv[0])
		*--argv = (अक्षर*)"-";

#अगर ENABLE_FEATURE_CATV
	अगर (opts & 7)
		वापस catv(opts, argv);
	opts >>= 4;
#पूर्ण_अगर

#अगर ENABLE_FEATURE_CATN
# define CAT_OPT_n (1<<0)
# define CAT_OPT_b (1<<1)
	अगर (opts & (CAT_OPT_n|CAT_OPT_b)) { /* -n or -b */
		काष्ठा संख्या_state ns;
		पूर्णांक exitcode;

		ns.width = 6;
		ns.start = 1;
		ns.inc = 1;
		ns.sep = "\t";
		ns.empty_str = शून्य;
		ns.all = !(opts & CAT_OPT_b); /* -n without -b */
		ns.nonempty = (opts & CAT_OPT_b); /* -b (with or without -n) */
		exitcode = निकास_सफल;
		करो {
			exitcode |= print_numbered_पंक्तियाँ(&ns, *argv);
		} जबतक (*++argv);
		fflush_stdout_and_exit(exitcode);
	}
	/*opts >>= 2;*/
#पूर्ण_अगर

	वापस bb_cat(argv);
}
