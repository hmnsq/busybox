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

#అన్తర్భావ "libbb.h"
#అన్తర్భావ "unicode.h"

గణన {
	OPT_INITIAL     = 1 << 0,
	OPT_TABS        = 1 << 1,
	OPT_ALL         = 1 << 2,
};

//FIXME: does not work properly with input containing NULs
//coreutils 8.30 preserves NULs but treats them as chars of width zero:
//AB<nul><tab>C will expand <tab> to 6 spaces, not 5.

#ఒకవేళ ENABLE_EXPAND
స్థిర శూన్య expand(FILE *file, గుర్తులేని tab_కొలత, గుర్తులేని opt)
{

	కొరకు (;;) {
		అక్షరం *పంక్తి;
		అక్షరం *ptr;
		అక్షరం *ptr_strbeg;
//commented-out code handles NULs, +90 bytes of code, not tested much
//		size_t linelen;
//		unsigned len = 0;

//		linelen = 1024 * 1024;
//		line = xmalloc_fgets_str_len(file, "\n", &linelen);
		పంక్తి = xmalloc_fgets(file); //
		ఒకవేళ (!పంక్తి)
			ఆపు;
		ptr = ptr_strbeg = పంక్తి;
		కొరకు (;;) {
			గుర్తులేని అక్షరం c = *ptr;
			ఒకవేళ (c == '\0') {
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
				ఆపు;
			}
			ఒకవేళ ((opt & OPT_INITIAL) && !isblank(c)) {
				/* not space or tab */
				ఆపు;
			}
			ఒకవేళ (c == '\t') {
				గుర్తులేని len = 0; //
				*ptr = '\0';
# ఒకవేళ ENABLE_UNICODE_SUPPORT
				len += unicode_strwidth(ptr_strbeg);
# లేకపోతే
				len += ptr - ptr_strbeg;
# endif
				len = tab_కొలత - (len % tab_కొలత);
				/*while (ptr[1] == '\t') { ptr++; len += tab_size; } - can handle many tabs at once */
				రాయి("%s%*s", ptr_strbeg, len, "");
//				len = 0;
				ptr_strbeg = ptr + 1;
			}
			ptr++;
		}
		fputs_stdout(ptr_strbeg);
		విడుదల(పంక్తి);
	}
}
#అయితే_ముగింపు

#ఒకవేళ ENABLE_UNEXPAND
స్థిర శూన్య unexpand(FILE *file, గుర్తులేని tab_కొలత, గుర్తులేని opt)
{
	అక్షరం *పంక్తి;

	అయితే ((పంక్తి = xmalloc_fgets(file)) != ఖాళీ) {
		అక్షరం *ptr = పంక్తి;
		గుర్తులేని స్తంభం = 0;

		అయితే (*ptr) {
			గుర్తులేని n;
			గుర్తులేని len = 0;

			అయితే (*ptr == ' ') {
				ptr++;
				len++;
			}
			స్తంభం += len;
			ఒకవేళ (*ptr == '\t') {
				స్తంభం += tab_కొలత - (స్తంభం % tab_కొలత);
				ptr++;
				కొనసాగు;
			}

			n = స్తంభం / tab_కొలత;
			ఒకవేళ (n) {
				len = స్తంభం = స్తంభం % tab_కొలత;
				అయితే (n--)
					అక్షరం_రాయి('\t');
			}

			ఒకవేళ (!(opt & OPT_ALL) && ptr != పంక్తి) {
				రాయి("%*s%s", len, "", ptr);
				ఆపు;
			}
			n = strcspn(ptr, "\t ");
			రాయి("%*s%.*s", len, "", n, ptr);
# ఒకవేళ ENABLE_UNICODE_SUPPORT
			{
				అక్షరం c = ptr[n];
				ptr[n] = '\0';
				len = unicode_strwidth(ptr);
				ptr[n] = c;
			}
# లేకపోతే
			len = n;
# endif
			ptr += n;
			స్తంభం = (స్తంభం + len) % tab_కొలత;
		}
		విడుదల(పంక్తి);
	}
}
#అయితే_ముగింపు

పూర్ణాంకం expand_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం expand_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	/* Default 8 spaces for 1 tab */
	స్థిరరాశి అక్షరం *opt_t = "8";
	FILE *file;
	గుర్తులేని tab_కొలత;
	గుర్తులేని opt;
	exitcode_t exit_స్థితి = EXIT_SUCCESS;

	init_unicode();

	ఒకవేళ (ENABLE_EXPAND && (!ENABLE_UNEXPAND || applet_name[0] == 'e')) {
		opt = getopt32long(argv, "it:",
				"initial\0"          No_వాదం       "i"
				"tabs\0"             Required_వాదం "t"
				, &opt_t
		);
	} లేకపోతే {
		opt = getopt32long(argv, "^"
				"ft:a"
				"\0"
				"ta" /* -t NUM sets -a */,
				"first-only\0"       No_వాదం       "f"
				"tabs\0"             Required_వాదం "t"
				"all\0"              No_వాదం       "a"
				, &opt_t
		);
		/* -t implies -a, but an explicit -f overrides */
		ఒకవేళ (opt & OPT_INITIAL) opt &= ~OPT_ALL;
	}
	tab_కొలత = xatou_range(opt_t, 1, UINT_MAX);

	argv += optind;

	ఒకవేళ (!*argv) {
		*--argv = (అక్షరం*)bb_సందేశం_standard_ఆదానం;
	}
	చేయి {
		file = fopen_or_warn_stdin(*argv);
		ఒకవేళ (!file) {
			exit_స్థితి = EXIT_FAILURE;
			కొనసాగు;
		}

		ఒకవేళ (ENABLE_EXPAND && (!ENABLE_UNEXPAND || applet_name[0] == 'e'))
			IF_EXPAND(expand(file, tab_కొలత, opt));
		లేకపోతే
			IF_UNEXPAND(unexpand(file, tab_కొలత, opt));

		/* Check and close the file */
		ఒకవేళ (fclose_if_not_stdin(file)) {
			bb_simple_perror_సందేశం(*argv);
			exit_స్థితి = EXIT_FAILURE;
		}
		/* If stdin also clear EOF */
		ఒకవేళ (file == stdin)
			clearerr(file);
	} అయితే (*++argv);

	/* Now close stdin also */
	/* (if we didn't read from it, it's a no-op) */
	ఒకవేళ (fclose(stdin))
		bb_simple_perror_సందేశం_and_die(bb_సందేశం_standard_ఆదానం);

	fflush_stdout_and_exit(exit_స్థితి);
}
