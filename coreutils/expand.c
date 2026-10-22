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

#অন্তর্ভাব "libbb.h"
#অন্তর্ভাব "unicode.h"

গণনা {
	OPT_INITIAL     = 1 << 0,
	OPT_TABS        = 1 << 1,
	OPT_ALL         = 1 << 2,
};

//FIXME: does not work properly with input containing NULs
//coreutils 8.30 preserves NULs but treats them as chars of width zero:
//AB<nul><tab>C will expand <tab> to 6 spaces, not 5.

#যদি ENABLE_EXPAND
স্থির শূন্য expand(FILE *file, unsigned tab_আকার, unsigned opt)
{

	জন্য (;;) {
		অক্ষর *সারি;
		অক্ষর *ptr;
		অক্ষর *ptr_strbeg;
//commented-out code handles NULs, +90 bytes of code, not tested much
//		size_t linelen;
//		unsigned len = 0;

//		linelen = 1024 * 1024;
//		line = xmalloc_fgets_str_len(file, "\n", &linelen);
		সারি = xmalloc_fgets(file); //
		যদি (!সারি)
			বিরতি;
		ptr = ptr_strbeg = সারি;
		জন্য (;;) {
			unsigned অক্ষর c = *ptr;
			যদি (c == '\0') {
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
				বিরতি;
			}
			যদি ((opt & OPT_INITIAL) && !isblank(c)) {
				/* not space or tab */
				বিরতি;
			}
			যদি (c == '\t') {
				unsigned len = 0; //
				*ptr = '\0';
# যদি ENABLE_UNICODE_SUPPORT
				len += unicode_strwidth(ptr_strbeg);
# অন্যথা
				len += ptr - ptr_strbeg;
# endif
				len = tab_আকার - (len % tab_আকার);
				/*while (ptr[1] == '\t') { ptr++; len += tab_size; } - can handle many tabs at once */
				printf("%s%*s", ptr_strbeg, len, "");
//				len = 0;
				ptr_strbeg = ptr + 1;
			}
			ptr++;
		}
		fputs_stdout(ptr_strbeg);
		মুক্ত(সারি);
	}
}
#endif

#যদি ENABLE_UNEXPAND
স্থির শূন্য unexpand(FILE *file, unsigned tab_আকার, unsigned opt)
{
	অক্ষর *সারি;

	যখন ((সারি = xmalloc_fgets(file)) != ফাঁকা) {
		অক্ষর *ptr = সারি;
		unsigned column = 0;

		যখন (*ptr) {
			unsigned n;
			unsigned len = 0;

			যখন (*ptr == ' ') {
				ptr++;
				len++;
			}
			column += len;
			যদি (*ptr == '\t') {
				column += tab_আকার - (column % tab_আকার);
				ptr++;
				চালু;
			}

			n = column / tab_আকার;
			যদি (n) {
				len = column = column % tab_আকার;
				যখন (n--)
					putchar('\t');
			}

			যদি (!(opt & OPT_ALL) && ptr != সারি) {
				printf("%*s%s", len, "", ptr);
				বিরতি;
			}
			n = strcspn(ptr, "\t ");
			printf("%*s%.*s", len, "", n, ptr);
# যদি ENABLE_UNICODE_SUPPORT
			{
				অক্ষর c = ptr[n];
				ptr[n] = '\0';
				len = unicode_strwidth(ptr);
				ptr[n] = c;
			}
# অন্যথা
			len = n;
# endif
			ptr += n;
			column = (column + len) % tab_আকার;
		}
		মুক্ত(সারি);
	}
}
#endif

পূর্ণসংখ্যা expand_main(পূর্ণসংখ্যা argc, অক্ষর **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণসংখ্যা expand_main(পূর্ণসংখ্যা argc UNUSED_PARAM, অক্ষর **argv)
{
	/* Default 8 spaces for 1 tab */
	ধ্রুবক অক্ষর *opt_t = "8";
	FILE *file;
	unsigned tab_আকার;
	unsigned opt;
	exitcode_t exit_status = EXIT_SUCCESS;

	init_unicode();

	যদি (ENABLE_EXPAND && (!ENABLE_UNEXPAND || applet_name[0] == 'e')) {
		opt = getopt32long(argv, "it:",
				"initial\0"          No_argument       "i"
				"tabs\0"             Required_argument "t"
				, &opt_t
		);
	} অন্যথা {
		opt = getopt32long(argv, "^"
				"ft:a"
				"\0"
				"ta" /* -t NUM sets -a */,
				"first-only\0"       No_argument       "f"
				"tabs\0"             Required_argument "t"
				"all\0"              No_argument       "a"
				, &opt_t
		);
		/* -t implies -a, but an explicit -f overrides */
		যদি (opt & OPT_INITIAL) opt &= ~OPT_ALL;
	}
	tab_আকার = xatou_range(opt_t, 1, UINT_MAX);

	argv += optind;

	যদি (!*argv) {
		*--argv = (অক্ষর*)bb_বার্তা_standard_প্রবেশ;
	}
	করো {
		file = fopen_or_warn_stdin(*argv);
		যদি (!file) {
			exit_status = EXIT_FAILURE;
			চালু;
		}

		যদি (ENABLE_EXPAND && (!ENABLE_UNEXPAND || applet_name[0] == 'e'))
			IF_EXPAND(expand(file, tab_আকার, opt));
		অন্যথা
			IF_UNEXPAND(unexpand(file, tab_আকার, opt));

		/* Check and close the file */
		যদি (fclose_if_not_stdin(file)) {
			bb_simple_perror_বার্তা(*argv);
			exit_status = EXIT_FAILURE;
		}
		/* If stdin also clear EOF */
		যদি (file == stdin)
			clearerr(file);
	} যখন (*++argv);

	/* Now close stdin also */
	/* (if we didn't read from it, it's a no-op) */
	যদি (fclose(stdin))
		bb_simple_perror_বার্তা_and_die(bb_বার্তা_standard_প্রবেশ);

	fflush_stdout_and_exit(exit_status);
}
