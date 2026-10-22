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

#ಅನ್ತರ್ಭಾವ "libbb.h"
#ಅನ್ತರ್ಭಾವ "unicode.h"

ಎಣಿಕೆ {
	OPT_INITIAL     = 1 << 0,
	OPT_TABS        = 1 << 1,
	OPT_ALL         = 1 << 2,
};

//FIXME: does not work properly with input containing NULs
//coreutils 8.30 preserves NULs but treats them as chars of width zero:
//AB<nul><tab>C will expand <tab> to 6 spaces, not 5.

#ಆದರೆ ENABLE_EXPAND
ಸ್ಥಿರ ಶೂನ್ಯ expand(FILE *file, ಚಿಹ್ನೆರಹಿತ tab_ಗಾತ್ರ, ಚಿಹ್ನೆರಹಿತ opt)
{

	ಗಾಗಿ (;;) {
		ಅಕ್ಷರ *ಸಾಲು;
		ಅಕ್ಷರ *ptr;
		ಅಕ್ಷರ *ptr_strbeg;
//commented-out code handles NULs, +90 bytes of code, not tested much
//		size_t linelen;
//		unsigned len = 0;

//		linelen = 1024 * 1024;
//		line = xmalloc_fgets_str_len(file, "\n", &linelen);
		ಸಾಲು = xmalloc_fgets(file); //
		ಆದರೆ (!ಸಾಲು)
			ನಿಲ್ಲಿಸು;
		ptr = ptr_strbeg = ಸಾಲು;
		ಗಾಗಿ (;;) {
			ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ c = *ptr;
			ಆದರೆ (c == '\0') {
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
				ನಿಲ್ಲಿಸು;
			}
			ಆದರೆ ((opt & OPT_INITIAL) && !isblank(c)) {
				/* not space or tab */
				ನಿಲ್ಲಿಸು;
			}
			ಆದರೆ (c == '\t') {
				ಚಿಹ್ನೆರಹಿತ len = 0; //
				*ptr = '\0';
# ಆದರೆ ENABLE_UNICODE_SUPPORT
				len += unicode_strwidth(ptr_strbeg);
# ಇಲ್ಲದಿದ್ದರೆ
				len += ptr - ptr_strbeg;
# endif
				len = tab_ಗಾತ್ರ - (len % tab_ಗಾತ್ರ);
				/*while (ptr[1] == '\t') { ptr++; len += tab_size; } - can handle many tabs at once */
				ಬರೆ("%s%*s", ptr_strbeg, len, "");
//				len = 0;
				ptr_strbeg = ptr + 1;
			}
			ptr++;
		}
		fputs_stdout(ptr_strbeg);
		ಬಿಡುಗಡೆ(ಸಾಲು);
	}
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_UNEXPAND
ಸ್ಥಿರ ಶೂನ್ಯ unexpand(FILE *file, ಚಿಹ್ನೆರಹಿತ tab_ಗಾತ್ರ, ಚಿಹ್ನೆರಹಿತ opt)
{
	ಅಕ್ಷರ *ಸಾಲು;

	ಆಗ ((ಸಾಲು = xmalloc_fgets(file)) != ಖಾಲಿ) {
		ಅಕ್ಷರ *ptr = ಸಾಲು;
		ಚಿಹ್ನೆರಹಿತ ಸ್ತಂಭ = 0;

		ಆಗ (*ptr) {
			ಚಿಹ್ನೆರಹಿತ n;
			ಚಿಹ್ನೆರಹಿತ len = 0;

			ಆಗ (*ptr == ' ') {
				ptr++;
				len++;
			}
			ಸ್ತಂಭ += len;
			ಆದರೆ (*ptr == '\t') {
				ಸ್ತಂಭ += tab_ಗಾತ್ರ - (ಸ್ತಂಭ % tab_ಗಾತ್ರ);
				ptr++;
				ಮುಂದುವರಿ;
			}

			n = ಸ್ತಂಭ / tab_ಗಾತ್ರ;
			ಆದರೆ (n) {
				len = ಸ್ತಂಭ = ಸ್ತಂಭ % tab_ಗಾತ್ರ;
				ಆಗ (n--)
					ಅಕ್ಷರ_ಬರೆ('\t');
			}

			ಆದರೆ (!(opt & OPT_ALL) && ptr != ಸಾಲು) {
				ಬರೆ("%*s%s", len, "", ptr);
				ನಿಲ್ಲಿಸು;
			}
			n = strcspn(ptr, "\t ");
			ಬರೆ("%*s%.*s", len, "", n, ptr);
# ಆದರೆ ENABLE_UNICODE_SUPPORT
			{
				ಅಕ್ಷರ c = ptr[n];
				ptr[n] = '\0';
				len = unicode_strwidth(ptr);
				ptr[n] = c;
			}
# ಇಲ್ಲದಿದ್ದರೆ
			len = n;
# endif
			ptr += n;
			ಸ್ತಂಭ = (ಸ್ತಂಭ + len) % tab_ಗಾತ್ರ;
		}
		ಬಿಡುಗಡೆ(ಸಾಲು);
	}
}
#ಆದರೆ_ಅಂತ್ಯ

ಪೂರ್ಣಾಂಕ expand_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ expand_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	/* Default 8 spaces for 1 tab */
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *opt_t = "8";
	FILE *file;
	ಚಿಹ್ನೆರಹಿತ tab_ಗಾತ್ರ;
	ಚಿಹ್ನೆರಹಿತ opt;
	exitcode_t exit_ಸ್ಥಿತಿ = EXIT_SUCCESS;

	init_unicode();

	ಆದರೆ (ENABLE_EXPAND && (!ENABLE_UNEXPAND || applet_name[0] == 'e')) {
		opt = getopt32long(argv, "it:",
				"initial\0"          No_ವಾದ       "i"
				"tabs\0"             Required_ವಾದ "t"
				, &opt_t
		);
	} ಇಲ್ಲದಿದ್ದರೆ {
		opt = getopt32long(argv, "^"
				"ft:a"
				"\0"
				"ta" /* -t NUM sets -a */,
				"first-only\0"       No_ವಾದ       "f"
				"tabs\0"             Required_ವಾದ "t"
				"all\0"              No_ವಾದ       "a"
				, &opt_t
		);
		/* -t implies -a, but an explicit -f overrides */
		ಆದರೆ (opt & OPT_INITIAL) opt &= ~OPT_ALL;
	}
	tab_ಗಾತ್ರ = xatou_range(opt_t, 1, UINT_MAX);

	argv += optind;

	ಆದರೆ (!*argv) {
		*--argv = (ಅಕ್ಷರ*)bb_ಸಂದೇಶ_standard_ಒಳಹರಿವು;
	}
	ಮಾಡು {
		file = fopen_or_warn_stdin(*argv);
		ಆದರೆ (!file) {
			exit_ಸ್ಥಿತಿ = EXIT_FAILURE;
			ಮುಂದುವರಿ;
		}

		ಆದರೆ (ENABLE_EXPAND && (!ENABLE_UNEXPAND || applet_name[0] == 'e'))
			IF_EXPAND(expand(file, tab_ಗಾತ್ರ, opt));
		ಇಲ್ಲದಿದ್ದರೆ
			IF_UNEXPAND(unexpand(file, tab_ಗಾತ್ರ, opt));

		/* Check and close the file */
		ಆದರೆ (fclose_if_not_stdin(file)) {
			bb_simple_perror_ಸಂದೇಶ(*argv);
			exit_ಸ್ಥಿತಿ = EXIT_FAILURE;
		}
		/* If stdin also clear EOF */
		ಆದರೆ (file == stdin)
			clearerr(file);
	} ಆಗ (*++argv);

	/* Now close stdin also */
	/* (if we didn't read from it, it's a no-op) */
	ಆದರೆ (fclose(stdin))
		bb_simple_perror_ಸಂದೇಶ_and_die(bb_ಸಂದೇಶ_standard_ಒಳಹರಿವು);

	fflush_stdout_and_exit(exit_ಸ್ಥಿತಿ);
}
