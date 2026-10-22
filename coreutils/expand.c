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

#अन्तर्भाव "libbb.h"
#अन्तर्भाव "unicode.h"

गणना {
	OPT_INITIAL     = 1 << 0,
	OPT_TABS        = 1 << 1,
	OPT_ALL         = 1 << 2,
};

//FIXME: does not work properly with input containing NULs
//coreutils 8.30 preserves NULs but treats them as chars of width zero:
//AB<nul><tab>C will expand <tab> to 6 spaces, not 5.

#यदि ENABLE_EXPAND
स्थिर शून्य expand(FILE *file, अचिह्नित tab_आकार, अचिह्नित opt)
{

	प्रति (;;) {
		वर्ण *पङ्क्तिः;
		वर्ण *ptr;
		वर्ण *ptr_strbeg;
//commented-out code handles NULs, +90 bytes of code, not tested much
//		size_t linelen;
//		unsigned len = 0;

//		linelen = 1024 * 1024;
//		line = xmalloc_fgets_str_len(file, "\n", &linelen);
		पङ्क्तिः = xmalloc_fgets(file); //
		यदि (!पङ्क्तिः)
			भङ्ग;
		ptr = ptr_strbeg = पङ्क्तिः;
		प्रति (;;) {
			अचिह्नित वर्ण c = *ptr;
			यदि (c == '\0') {
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
				भङ्ग;
			}
			यदि ((opt & OPT_INITIAL) && !isblank(c)) {
				/* not space or tab */
				भङ्ग;
			}
			यदि (c == '\t') {
				अचिह्नित len = 0; //
				*ptr = '\0';
# यदि ENABLE_UNICODE_SUPPORT
				len += unicode_strwidth(ptr_strbeg);
# अन्यथा
				len += ptr - ptr_strbeg;
# endif
				len = tab_आकार - (len % tab_आकार);
				/*while (ptr[1] == '\t') { ptr++; len += tab_size; } - can handle many tabs at once */
				प्रारूप_लिख("%s%*s", ptr_strbeg, len, "");
//				len = 0;
				ptr_strbeg = ptr + 1;
			}
			ptr++;
		}
		fputs_stdout(ptr_strbeg);
		मुक्त(पङ्क्तिः);
	}
}
#यदि_अन्तः

#यदि ENABLE_UNEXPAND
स्थिर शून्य unexpand(FILE *file, अचिह्नित tab_आकार, अचिह्नित opt)
{
	वर्ण *पङ्क्तिः;

	यावत् ((पङ्क्तिः = xmalloc_fgets(file)) != रिक्त) {
		वर्ण *ptr = पङ्क्तिः;
		अचिह्नित column = 0;

		यावत् (*ptr) {
			अचिह्नित n;
			अचिह्नित len = 0;

			यावत् (*ptr == ' ') {
				ptr++;
				len++;
			}
			column += len;
			यदि (*ptr == '\t') {
				column += tab_आकार - (column % tab_आकार);
				ptr++;
				अनुवर्त;
			}

			n = column / tab_आकार;
			यदि (n) {
				len = column = column % tab_आकार;
				यावत् (n--)
					अक्षरं_लिख('\t');
			}

			यदि (!(opt & OPT_ALL) && ptr != पङ्क्तिः) {
				प्रारूप_लिख("%*s%s", len, "", ptr);
				भङ्ग;
			}
			n = strcspn(ptr, "\t ");
			प्रारूप_लिख("%*s%.*s", len, "", n, ptr);
# यदि ENABLE_UNICODE_SUPPORT
			{
				वर्ण c = ptr[n];
				ptr[n] = '\0';
				len = unicode_strwidth(ptr);
				ptr[n] = c;
			}
# अन्यथा
			len = n;
# endif
			ptr += n;
			column = (column + len) % tab_आकार;
		}
		मुक्त(पङ्क्तिः);
	}
}
#यदि_अन्तः

पूर्णाङ्क expand_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क expand_main(पूर्णाङ्क argc अप्रयुक्तम्_PARAM, वर्ण **argv)
{
	/* Default 8 spaces for 1 tab */
	ध्रुव वर्ण *opt_t = "8";
	FILE *file;
	अचिह्नित tab_आकार;
	अचिह्नित opt;
	exitcode_t exit_स्थितिः = EXIT_SUCCESS;

	init_unicode();

	यदि (ENABLE_EXPAND && (!ENABLE_UNEXPAND || applet_name[0] == 'e')) {
		opt = getopt32long(argv, "it:",
				"initial\0"          No_तर्कः       "i"
				"tabs\0"             Required_तर्कः "t"
				, &opt_t
		);
	} अन्यथा {
		opt = getopt32long(argv, "^"
				"ft:a"
				"\0"
				"ta" /* -t NUM sets -a */,
				"first-only\0"       No_तर्कः       "f"
				"tabs\0"             Required_तर्कः "t"
				"all\0"              No_तर्कः       "a"
				, &opt_t
		);
		/* -t implies -a, but an explicit -f overrides */
		यदि (opt & OPT_INITIAL) opt &= ~OPT_ALL;
	}
	tab_आकार = xatou_range(opt_t, 1, UINT_MAX);

	argv += optind;

	यदि (!*argv) {
		*--argv = (वर्ण*)bb_सन्देशः_standard_निवेशः;
	}
	कुरु {
		file = fopen_or_warn_stdin(*argv);
		यदि (!file) {
			exit_स्थितिः = EXIT_FAILURE;
			अनुवर्त;
		}

		यदि (ENABLE_EXPAND && (!ENABLE_UNEXPAND || applet_name[0] == 'e'))
			IF_EXPAND(expand(file, tab_आकार, opt));
		अन्यथा
			IF_UNEXPAND(unexpand(file, tab_आकार, opt));

		/* Check and close the file */
		यदि (fclose_if_not_stdin(file)) {
			bb_simple_perror_सन्देशः(*argv);
			exit_स्थितिः = EXIT_FAILURE;
		}
		/* If stdin also clear EOF */
		यदि (file == stdin)
			clearerr(file);
	} यावत् (*++argv);

	/* Now close stdin also */
	/* (if we didn't read from it, it's a no-op) */
	यदि (fclose(stdin))
		bb_simple_perror_सन्देशः_and_die(bb_सन्देशः_standard_निवेशः);

	fflush_stdout_and_exit(exit_स्थितिः);
}
