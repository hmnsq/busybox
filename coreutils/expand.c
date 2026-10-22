/* expand - convert tabs to spaces
 * unexpand - convert spaces to tabs
 *
 * Copyright (C) 89, 91, 1995-2006 Free Software Foundation, Inc.
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 *
 * David MacKenzie <djm@gnu.ai.mit.edu>
 *
 * Options for expand:
 * -t num  --tabs NUM      Convert tabs to num spaces (default 8 spaces).
 * -i      --initial       Only convert initial tabs on each line to spaces.
 *
 * Options for unexpand:
 * -a      --all           Convert all blanks, instead of just initial blanks.
 * -f      --first-only    Convert only leading sequences of blanks (default).
 * -t num  --tabs NUM      Have tabs num characters apart instead of 8.
 *
 *  Busybox version (C) 2007 by Tito Ragusa <farmatito@tiscali.it>
 *
 *  Caveat: this versions of expand and unexpand don't accept tab lists.
 */
//config:config EXPAND
//config:	bool "expand (5.3 kb)"
//config:	default y
//config:	help
//config:	By default, convert all tabs to spaces.
//config:
//config:config UNEXPAND
//config:	bool "unexpand (5.5 kb)"
//config:	default y
//config:	help
//config:	By default, convert only leading sequences of blanks to tabs.

//applet:IF_EXPAND(APPLET(expand, BB_DIR_USR_BIN, BB_SUID_DROP))
//                   APPLET_ODDNAME:name      main    location        suid_type     help
//applet:IF_UNEXPAND(APPLET_ODDNAME(unexpand, expand, BB_DIR_USR_BIN, BB_SUID_DROP, unexpand))

//kbuild:lib-$(CONFIG_EXPAND) += expand.o
//kbuild:lib-$(CONFIG_UNEXPAND) += expand.o

//usage:#define expand_trivial_usage
//usage:       "[-i] [-t N] [FILE]..."
//usage:#define expand_full_usage "\n\n"
//usage:       "Convert tabs to spaces, writing to stdout\n"
//usage:     "\n	-i	Don't convert tabs after non blanks"
//usage:     "\n	-t	Tabstops every N chars"

//usage:#define unexpand_trivial_usage
//usage:       "[-fa][-t N] [FILE]..."
//usage:#define unexpand_full_usage "\n\n"
//usage:       "Convert spaces to tabs, writing to stdout\n"
//usage:     "\n	-a	Convert all blanks"
//usage:     "\n	-f	Convert only leading blanks"
//usage:     "\n	-t N	Tabstops every N chars"

#ضمن "libbb.h"
#ضمن "unicode.h"

تعداد {
	OPT_INITIAL     = 1 << 0,
	OPT_TABS        = 1 << 1,
	OPT_ALL         = 1 << 2,
};

//FIXME: does not work properly with input containing NULs
//coreutils 8.30 preserves NULs but treats them as chars of width zero:
//AB<nul><tab>C will expand <tab> to 6 spaces, not 5.

#إذا ENABLE_EXPAND
ساكن فراغ expand(FILE *file, بینښه tab_اندازه, بینښه opt)
{

	لكل (;;) {
		حرف *کرښه;
		حرف *ptr;
		حرف *ptr_strbeg;
//commented-out code handles NULs, +90 bytes of code, not tested much
//		size_t linelen;
//		unsigned len = 0;

//		linelen = 1024 * 1024;
//		line = xmalloc_fgets_str_len(file, "\n", &linelen);
		کرښه = xmalloc_fgets(file); //
		إذا (!کرښه)
			اكسر;
		ptr = ptr_strbeg = کرښه;
		لكل (;;) {
			بینښه حرف c = *ptr;
			إذا (c == '\0') {
//				size_t rem = line + linelen - ptr;
//				if (rem > 0) {
//# if ENABLE_UNICODE_SUPPORT
//					len += unicode_strwidth(ptr_strbeg);
//# else
//					len += ptr - ptr_strbeg;
//# endif
//					printf("%s%c", ptr_strbeg, '\0');
//					memmove(ptr, ptr + 1, rem + 1);
//					ptr_strbeg = ptr;
//					linelen--;
//					continue;
//				}
				اكسر;
			}
			إذا ((opt & OPT_INITIAL) && !isblank(c)) {
				/* not space or tab */
				اكسر;
			}
			إذا (c == '\t') {
				بینښه len = 0; //
				*ptr = '\0';
# إذا ENABLE_UNICODE_SUPPORT
				len += unicode_strwidth(ptr_strbeg);
# وإلا
				len += ptr - ptr_strbeg;
# endif
				len = tab_اندازه - (len % tab_اندازه);
				/*while (ptr[1] == '\t') { ptr++; len += tab_size; } - can handle many tabs at once */
				چاپ_کړه("%s%*s", ptr_strbeg, len, "");
//				len = 0;
				ptr_strbeg = ptr + 1;
			}
			ptr++;
		}
		fputs_stdout(ptr_strbeg);
		حرر(کرښه);
	}
}
#د_که_پای

#إذا ENABLE_UNEXPAND
ساكن فراغ unexpand(FILE *file, بینښه tab_اندازه, بینښه opt)
{
	حرف *کرښه;

	طالما ((کرښه = xmalloc_fgets(file)) != عدم) {
		حرف *ptr = کرښه;
		بینښه ستنه = 0;

		طالما (*ptr) {
			بینښه n;
			بینښه len = 0;

			طالما (*ptr == ' ') {
				ptr++;
				len++;
			}
			ستنه += len;
			إذا (*ptr == '\t') {
				ستنه += tab_اندازه - (ستنه % tab_اندازه);
				ptr++;
				استمر;
			}

			n = ستنه / tab_اندازه;
			إذا (n) {
				len = ستنه = ستنه % tab_اندازه;
				طالما (n--)
					توری_چاپ_کړه('\t');
			}

			إذا (!(opt & OPT_ALL) && ptr != کرښه) {
				چاپ_کړه("%*s%s", len, "", ptr);
				اكسر;
			}
			n = strcspn(ptr, "\t ");
			چاپ_کړه("%*s%.*s", len, "", n, ptr);
# إذا ENABLE_UNICODE_SUPPORT
			{
				حرف c = ptr[n];
				ptr[n] = '\0';
				len = unicode_strwidth(ptr);
				ptr[n] = c;
			}
# وإلا
			len = n;
# endif
			ptr += n;
			ستنه = (ستنه + len) % tab_اندازه;
		}
		حرر(کرښه);
	}
}
#د_که_پای

صحيح expand_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح expand_main(صحيح argc ناکارول_شوی_PARAM, حرف **argv)
{
	/* Default 8 spaces for 1 tab */
	ثابت حرف *opt_t = "8";
	FILE *file;
	بینښه tab_اندازه;
	بینښه opt;
	exitcode_t exit_حالت = EXIT_SUCCESS;

	init_unicode();

	إذا (ENABLE_EXPAND && (!ENABLE_UNEXPAND || applet_name[0] == 'e')) {
		opt = getopt32long(argv, "it:",
				"initial\0"          No_دلیل       "i"
				"tabs\0"             Required_دلیل "t"
				, &opt_t
		);
	} وإلا {
		opt = getopt32long(argv, "^"
				"ft:a"
				"\0"
				"ta" /* -t NUM sets -a */,
				"first-only\0"       No_دلیل       "f"
				"tabs\0"             Required_دلیل "t"
				"all\0"              No_دلیل       "a"
				, &opt_t
		);
		/* -t implies -a, but an explicit -f overrides */
		إذا (opt & OPT_INITIAL) opt &= ~OPT_ALL;
	}
	tab_اندازه = xatou_range(opt_t, 1, UINT_MAX);

	argv += optind;

	إذا (!*argv) {
		*--argv = (حرف*)bb_پیغام_standard_ننوت;
	}
	افعل {
		file = fopen_or_warn_stdin(*argv);
		إذا (!file) {
			exit_حالت = EXIT_FAILURE;
			استمر;
		}

		إذا (ENABLE_EXPAND && (!ENABLE_UNEXPAND || applet_name[0] == 'e'))
			IF_EXPAND(expand(file, tab_اندازه, opt));
		وإلا
			IF_UNEXPAND(unexpand(file, tab_اندازه, opt));

		/* Check and close the file */
		إذا (fclose_if_not_stdin(file)) {
			bb_simple_perror_پیغام(*argv);
			exit_حالت = EXIT_FAILURE;
		}
		/* If stdin also clear EOF */
		إذا (file == stdin)
			clearerr(file);
	} طالما (*++argv);

	/* Now close stdin also */
	/* (if we didn't read from it, it's a no-op) */
	إذا (fclose(stdin))
		bb_simple_perror_پیغام_and_die(bb_پیغام_standard_ننوت);

	fflush_stdout_and_exit(exit_حالت);
}
