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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "unicode.h"

ਗਿਣਤੀ {
	OPT_INITIAL     = 1 << 0,
	OPT_TABS        = 1 << 1,
	OPT_ALL         = 1 << 2,
};

//FIXME: does not work properly with input containing NULs
//coreutils 8.30 preserves NULs but treats them as chars of width zero:
//AB<nul><tab>C will expand <tab> to 6 spaces, not 5.

#ਜੇ ENABLE_EXPAND
ਸਥਿਰ ਖਾਲੀ expand(FILE *file, ਬਿਨਾਨਿਸ਼ਾਨੀ tab_ਮਾਪ, ਬਿਨਾਨਿਸ਼ਾਨੀ opt)
{

	ਲਈ (;;) {
		ਅੱਖਰ *ਸਤਰ;
		ਅੱਖਰ *ptr;
		ਅੱਖਰ *ptr_strbeg;
//commented-out code handles NULs, +90 bytes of code, not tested much
//		size_t linelen;
//		unsigned len = 0;

//		linelen = 1024 * 1024;
//		line = xmalloc_fgets_str_len(file, "\n", &linelen);
		ਸਤਰ = xmalloc_fgets(file); //
		ਜੇ (!ਸਤਰ)
			ਰੋਕੋ;
		ptr = ptr_strbeg = ਸਤਰ;
		ਲਈ (;;) {
			ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ c = *ptr;
			ਜੇ (c == '\0') {
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
				ਰੋਕੋ;
			}
			ਜੇ ((opt & OPT_INITIAL) && !isblank(c)) {
				/* not space or tab */
				ਰੋਕੋ;
			}
			ਜੇ (c == '\t') {
				ਬਿਨਾਨਿਸ਼ਾਨੀ len = 0; //
				*ptr = '\0';
# ਜੇ ENABLE_UNICODE_SUPPORT
				len += unicode_strwidth(ptr_strbeg);
# ਨਹੀਂਤਾਂ
				len += ptr - ptr_strbeg;
# endif
				len = tab_ਮਾਪ - (len % tab_ਮਾਪ);
				/*while (ptr[1] == '\t') { ptr++; len += tab_size; } - can handle many tabs at once */
				ਲਿਖੋ("%s%*s", ptr_strbeg, len, "");
//				len = 0;
				ptr_strbeg = ptr + 1;
			}
			ptr++;
		}
		fputs_stdout(ptr_strbeg);
		ਮੁਕਤ(ਸਤਰ);
	}
}
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_UNEXPAND
ਸਥਿਰ ਖਾਲੀ unexpand(FILE *file, ਬਿਨਾਨਿਸ਼ਾਨੀ tab_ਮਾਪ, ਬਿਨਾਨਿਸ਼ਾਨੀ opt)
{
	ਅੱਖਰ *ਸਤਰ;

	ਜਦੋਂ ((ਸਤਰ = xmalloc_fgets(file)) != ਨਿਰਾ) {
		ਅੱਖਰ *ptr = ਸਤਰ;
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਕਾਲਮ = 0;

		ਜਦੋਂ (*ptr) {
			ਬਿਨਾਨਿਸ਼ਾਨੀ n;
			ਬਿਨਾਨਿਸ਼ਾਨੀ len = 0;

			ਜਦੋਂ (*ptr == ' ') {
				ptr++;
				len++;
			}
			ਕਾਲਮ += len;
			ਜੇ (*ptr == '\t') {
				ਕਾਲਮ += tab_ਮਾਪ - (ਕਾਲਮ % tab_ਮਾਪ);
				ptr++;
				ਜਾਰੀ;
			}

			n = ਕਾਲਮ / tab_ਮਾਪ;
			ਜੇ (n) {
				len = ਕਾਲਮ = ਕਾਲਮ % tab_ਮਾਪ;
				ਜਦੋਂ (n--)
					ਅੱਖਰ_ਲਿਖੋ('\t');
			}

			ਜੇ (!(opt & OPT_ALL) && ptr != ਸਤਰ) {
				ਲਿਖੋ("%*s%s", len, "", ptr);
				ਰੋਕੋ;
			}
			n = strcspn(ptr, "\t ");
			ਲਿਖੋ("%*s%.*s", len, "", n, ptr);
# ਜੇ ENABLE_UNICODE_SUPPORT
			{
				ਅੱਖਰ c = ptr[n];
				ptr[n] = '\0';
				len = unicode_strwidth(ptr);
				ptr[n] = c;
			}
# ਨਹੀਂਤਾਂ
			len = n;
# endif
			ptr += n;
			ਕਾਲਮ = (ਕਾਲਮ + len) % tab_ਮਾਪ;
		}
		ਮੁਕਤ(ਸਤਰ);
	}
}
#ਜੇ_ਅੰਤ

ਪੂਰਨ expand_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ expand_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
	/* Default 8 spaces for 1 tab */
	ਅਚਲ ਅੱਖਰ *opt_t = "8";
	FILE *file;
	ਬਿਨਾਨਿਸ਼ਾਨੀ tab_ਮਾਪ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ opt;
	exitcode_t exit_ਸਥਿਤੀ = EXIT_SUCCESS;

	init_unicode();

	ਜੇ (ENABLE_EXPAND && (!ENABLE_UNEXPAND || applet_name[0] == 'e')) {
		opt = getopt32long(argv, "it:",
				"initial\0"          No_ਦਲੀਲ       "i"
				"tabs\0"             Required_ਦਲੀਲ "t"
				, &opt_t
		);
	} ਨਹੀਂਤਾਂ {
		opt = getopt32long(argv, "^"
				"ft:a"
				"\0"
				"ta" /* -t NUM sets -a */,
				"first-only\0"       No_ਦਲੀਲ       "f"
				"tabs\0"             Required_ਦਲੀਲ "t"
				"all\0"              No_ਦਲੀਲ       "a"
				, &opt_t
		);
		/* -t implies -a, but an explicit -f overrides */
		ਜੇ (opt & OPT_INITIAL) opt &= ~OPT_ALL;
	}
	tab_ਮਾਪ = xatou_range(opt_t, 1, UINT_MAX);

	argv += optind;

	ਜੇ (!*argv) {
		*--argv = (ਅੱਖਰ*)bb_ਸੁਨੇਹਾ_standard_ਨਿਵੇਸ਼;
	}
	ਕਰੋ {
		file = fopen_or_warn_stdin(*argv);
		ਜੇ (!file) {
			exit_ਸਥਿਤੀ = EXIT_FAILURE;
			ਜਾਰੀ;
		}

		ਜੇ (ENABLE_EXPAND && (!ENABLE_UNEXPAND || applet_name[0] == 'e'))
			IF_EXPAND(expand(file, tab_ਮਾਪ, opt));
		ਨਹੀਂਤਾਂ
			IF_UNEXPAND(unexpand(file, tab_ਮਾਪ, opt));

		/* Check and close the file */
		ਜੇ (fclose_if_not_stdin(file)) {
			bb_simple_perror_ਸੁਨੇਹਾ(*argv);
			exit_ਸਥਿਤੀ = EXIT_FAILURE;
		}
		/* If stdin also clear EOF */
		ਜੇ (file == stdin)
			clearerr(file);
	} ਜਦੋਂ (*++argv);

	/* Now close stdin also */
	/* (if we didn't read from it, it's a no-op) */
	ਜੇ (fclose(stdin))
		bb_simple_perror_ਸੁਨੇਹਾ_and_die(bb_ਸੁਨੇਹਾ_standard_ਨਿਵੇਸ਼);

	fflush_stdout_and_exit(exit_ਸਥਿਤੀ);
}
