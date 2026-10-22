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

#അന്തര്ഭാവ "libbb.h"
#അന്തര്ഭാവ "unicode.h"

എണ്ണൽ {
	OPT_INITIAL     = 1 << 0,
	OPT_TABS        = 1 << 1,
	OPT_ALL         = 1 << 2,
};

//FIXME: does not work properly with input containing NULs
//coreutils 8.30 preserves NULs but treats them as chars of width zero:
//AB<nul><tab>C will expand <tab> to 6 spaces, not 5.

#എങ്കിൽ ENABLE_EXPAND
സ്ഥിതം ശൂന്യം expand(FILE *file, unsigned tab_size, unsigned opt)
{

	വേണ്ടി (;;) {
		അക്ഷരം *വരി;
		അക്ഷരം *ptr;
		അക്ഷരം *ptr_strbeg;
//commented-out code handles NULs, +90 bytes of code, not tested much
//		size_t linelen;
//		unsigned len = 0;

//		linelen = 1024 * 1024;
//		line = xmalloc_fgets_str_len(file, "\n", &linelen);
		വരി = xmalloc_fgets(file); //
		എങ്കിൽ (!വരി)
			നിർത്തുക;
		ptr = ptr_strbeg = വരി;
		വേണ്ടി (;;) {
			unsigned അക്ഷരം c = *ptr;
			എങ്കിൽ (c == '\0') {
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
				നിർത്തുക;
			}
			എങ്കിൽ ((opt & OPT_INITIAL) && !isblank(c)) {
				/* not space or tab */
				നിർത്തുക;
			}
			എങ്കിൽ (c == '\t') {
				unsigned len = 0; //
				*ptr = '\0';
# എങ്കിൽ ENABLE_UNICODE_SUPPORT
				len += unicode_strwidth(ptr_strbeg);
# അല്ലെങ്കിൽ
				len += ptr - ptr_strbeg;
# endif
				len = tab_size - (len % tab_size);
				/*while (ptr[1] == '\t') { ptr++; len += tab_size; } - can handle many tabs at once */
				printf("%s%*s", ptr_strbeg, len, "");
//				len = 0;
				ptr_strbeg = ptr + 1;
			}
			ptr++;
		}
		fputs_stdout(ptr_strbeg);
		സ്വതന്ത്രം(വരി);
	}
}
#endif

#എങ്കിൽ ENABLE_UNEXPAND
സ്ഥിതം ശൂന്യം unexpand(FILE *file, unsigned tab_size, unsigned opt)
{
	അക്ഷരം *വരി;

	ആകുമ്പോൾ ((വരി = xmalloc_fgets(file)) != ശൂന്യ) {
		അക്ഷരം *ptr = വരി;
		unsigned column = 0;

		ആകുമ്പോൾ (*ptr) {
			unsigned n;
			unsigned len = 0;

			ആകുമ്പോൾ (*ptr == ' ') {
				ptr++;
				len++;
			}
			column += len;
			എങ്കിൽ (*ptr == '\t') {
				column += tab_size - (column % tab_size);
				ptr++;
				തുടരുക;
			}

			n = column / tab_size;
			എങ്കിൽ (n) {
				len = column = column % tab_size;
				ആകുമ്പോൾ (n--)
					putchar('\t');
			}

			എങ്കിൽ (!(opt & OPT_ALL) && ptr != വരി) {
				printf("%*s%s", len, "", ptr);
				നിർത്തുക;
			}
			n = strcspn(ptr, "\t ");
			printf("%*s%.*s", len, "", n, ptr);
# എങ്കിൽ ENABLE_UNICODE_SUPPORT
			{
				അക്ഷരം c = ptr[n];
				ptr[n] = '\0';
				len = unicode_strwidth(ptr);
				ptr[n] = c;
			}
# അല്ലെങ്കിൽ
			len = n;
# endif
			ptr += n;
			column = (column + len) % tab_size;
		}
		സ്വതന്ത്രം(വരി);
	}
}
#endif

പൂർണ്ണസംഖ്യ expand_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) MAIN_EXTERNALLY_VISIBLE;
പൂർണ്ണസംഖ്യ expand_main(പൂർണ്ണസംഖ്യ argc UNUSED_PARAM, അക്ഷരം **argv)
{
	/* Default 8 spaces for 1 tab */
	സ്ഥിരം അക്ഷരം *opt_t = "8";
	FILE *file;
	unsigned tab_size;
	unsigned opt;
	exitcode_t exit_status = EXIT_SUCCESS;

	init_unicode();

	എങ്കിൽ (ENABLE_EXPAND && (!ENABLE_UNEXPAND || applet_name[0] == 'e')) {
		opt = getopt32long(argv, "it:",
				"initial\0"          No_argument       "i"
				"tabs\0"             Required_argument "t"
				, &opt_t
		);
	} അല്ലെങ്കിൽ {
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
		എങ്കിൽ (opt & OPT_INITIAL) opt &= ~OPT_ALL;
	}
	tab_size = xatou_range(opt_t, 1, UINT_MAX);

	argv += optind;

	എങ്കിൽ (!*argv) {
		*--argv = (അക്ഷരം*)bb_സന്ദേശം_standard_നിവേശം;
	}
	ചെയ്യുക {
		file = fopen_or_warn_stdin(*argv);
		എങ്കിൽ (!file) {
			exit_status = EXIT_FAILURE;
			തുടരുക;
		}

		എങ്കിൽ (ENABLE_EXPAND && (!ENABLE_UNEXPAND || applet_name[0] == 'e'))
			IF_EXPAND(expand(file, tab_size, opt));
		അല്ലെങ്കിൽ
			IF_UNEXPAND(unexpand(file, tab_size, opt));

		/* Check and close the file */
		എങ്കിൽ (fclose_if_not_stdin(file)) {
			bb_simple_perror_സന്ദേശം(*argv);
			exit_status = EXIT_FAILURE;
		}
		/* If stdin also clear EOF */
		എങ്കിൽ (file == stdin)
			clearerr(file);
	} ആകുമ്പോൾ (*++argv);

	/* Now close stdin also */
	/* (if we didn't read from it, it's a no-op) */
	എങ്കിൽ (fclose(stdin))
		bb_simple_perror_സന്ദേശം_and_die(bb_സന്ദേശം_standard_നിവേശം);

	fflush_stdout_and_exit(exit_status);
}
