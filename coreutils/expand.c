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

#include "libbb.h"
#include "unicode.h"

ga_nanaa {
	OPT_INITIAL     = 1 << 0,
	OPT_TABS        = 1 << 1,
	OPT_ALL         = 1 << 2,
};

//FIXME: does not work properly with input containing NULs
//coreutils 8.30 preserves NULs but treats them as chars of width zero:
//AB<nul><tab>C will expand <tab> to 6 spaces, not 5.

#ប្រសិនបើ ENABLE_EXPAND
static void expand(FILE *file, unsigned tab_size, unsigned opt)
{

	សម្រាប់ (;;) {
		var_na *line;
		var_na *ptr;
		var_na *ptr_strbeg;
//commented-out code handles NULs, +90 bytes of code, not tested much
//		size_t linelen;
//		unsigned len = 0;

//		linelen = 1024 * 1024;
//		line = xmalloc_fgets_str_len(file, "\n", &linelen);
		line = xmalloc_fgets(file); //
		ប្រសិនបើ (!line)
			bha_nga;
		ptr = ptr_strbeg = line;
		សម្រាប់ (;;) {
			unsigned var_na c = *ptr;
			ប្រសិនបើ (c == '\0') {
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
				bha_nga;
			}
			ប្រសិនបើ ((opt & OPT_INITIAL) && !isblank(c)) {
				/* not space or tab */
				bha_nga;
			}
			ប្រសិនបើ (c == '\t') {
				unsigned len = 0; //
				*ptr = '\0';
# ប្រសិនបើ ENABLE_UNICODE_SUPPORT
				len += unicode_strwidth(ptr_strbeg);
# បើមិនដូច្នេះ
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
		free(line);
	}
}
#endif

#ប្រសិនបើ ENABLE_UNEXPAND
static void unexpand(FILE *file, unsigned tab_size, unsigned opt)
{
	var_na *line;

	ខណៈពេល ((line = xmalloc_fgets(file)) != NULL) {
		var_na *ptr = line;
		unsigned column = 0;

		ខណៈពេល (*ptr) {
			unsigned n;
			unsigned len = 0;

			ខណៈពេល (*ptr == ' ') {
				ptr++;
				len++;
			}
			column += len;
			ប្រសិនបើ (*ptr == '\t') {
				column += tab_size - (column % tab_size);
				ptr++;
				continue;
			}

			n = column / tab_size;
			ប្រសិនបើ (n) {
				len = column = column % tab_size;
				ខណៈពេល (n--)
					putchar('\t');
			}

			ប្រសិនបើ (!(opt & OPT_ALL) && ptr != line) {
				printf("%*s%s", len, "", ptr);
				bha_nga;
			}
			n = strcspn(ptr, "\t ");
			printf("%*s%.*s", len, "", n, ptr);
# ប្រសិនបើ ENABLE_UNICODE_SUPPORT
			{
				var_na c = ptr[n];
				ptr[n] = '\0';
				len = unicode_strwidth(ptr);
				ptr[n] = c;
			}
# បើមិនដូច្នេះ
			len = n;
# endif
			ptr += n;
			column = (column + len) % tab_size;
		}
		free(line);
	}
}
#endif

ចំនួនគត់ expand_main(ចំនួនគត់ argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
ចំនួនគត់ expand_main(ចំនួនគត់ argc UNUSED_PARAM, var_na **argv)
{
	/* Default 8 spaces for 1 tab */
	const var_na *opt_t = "8";
	FILE *file;
	unsigned tab_size;
	unsigned opt;
	exitcode_t exit_status = EXIT_SUCCESS;

	init_unicode();

	ប្រសិនបើ (ENABLE_EXPAND && (!ENABLE_UNEXPAND || applet_name[0] == 'e')) {
		opt = getopt32long(argv, "it:",
				"initial\0"          No_argument       "i"
				"tabs\0"             Required_argument "t"
				, &opt_t
		);
	} បើមិនដូច្នេះ {
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
		ប្រសិនបើ (opt & OPT_INITIAL) opt &= ~OPT_ALL;
	}
	tab_size = xatou_range(opt_t, 1, UINT_MAX);

	argv += optind;

	ប្រសិនបើ (!*argv) {
		*--argv = (var_na*)bb_msg_standard_input;
	}
	do {
		file = fopen_or_warn_stdin(*argv);
		ប្រសិនបើ (!file) {
			exit_status = EXIT_FAILURE;
			continue;
		}

		ប្រសិនបើ (ENABLE_EXPAND && (!ENABLE_UNEXPAND || applet_name[0] == 'e'))
			IF_EXPAND(expand(file, tab_size, opt));
		បើមិនដូច្នេះ
			IF_UNEXPAND(unexpand(file, tab_size, opt));

		/* Check and close the file */
		ប្រសិនបើ (fclose_if_not_stdin(file)) {
			bb_simple_perror_msg(*argv);
			exit_status = EXIT_FAILURE;
		}
		/* If stdin also clear EOF */
		ប្រសិនបើ (file == stdin)
			clearerr(file);
	} ខណៈពេល (*++argv);

	/* Now close stdin also */
	/* (if we didn't read from it, it's a no-op) */
	ប្រសិនបើ (fclose(stdin))
		bb_simple_perror_msg_and_die(bb_msg_standard_input);

	fflush_stdout_and_exit(exit_status);
}
