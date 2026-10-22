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

#समावेश "libbb.h"
#समावेश "unicode.h"

क्रमागत {
	OPT_INITIAL     = 1 << 0,
	OPT_TABS        = 1 << 1,
	OPT_ALL         = 1 << 2,
};

//FIXME: does not work properly with input containing NULs
//coreutils 8.30 preserves NULs but treats them as chars of width zero:
//AB<nul><tab>C will expand <tab> to 6 spaces, not 5.

#अगर ENABLE_EXPAND
अटल व्योम expand(खाता *file, अचिन्हित tab_आकार, अचिन्हित opt)
{

	क्रम (;;) {
		अक्षर *पंक्ति;
		अक्षर *ptr;
		अक्षर *ptr_strbeg;
//commented-out code handles NULs, +90 bytes of code, not tested much
//		size_t linelen;
//		unsigned len = 0;

//		linelen = 1024 * 1024;
//		line = xmalloc_fgets_str_len(file, "\n", &linelen);
		पंक्ति = xmalloc_fgets(file); //
		अगर (!पंक्ति)
			अवरोध;
		ptr = ptr_strbeg = पंक्ति;
		क्रम (;;) {
			अचिन्हित अक्षर c = *ptr;
			अगर (c == '\0') {
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
				अवरोध;
			}
			अगर ((opt & OPT_INITIAL) && !isblank(c)) {
				/* not space or tab */
				अवरोध;
			}
			अगर (c == '\t') {
				अचिन्हित len = 0; //
				*ptr = '\0';
# अगर ENABLE_UNICODE_SUPPORT
				len += unicode_strwidth(ptr_strbeg);
# अन्यथा
				len += ptr - ptr_strbeg;
# endif
				len = tab_आकार - (len % tab_आकार);
				/*while (ptr[1] == '\t') { ptr++; len += tab_size; } - can handle many tabs at once */
				म_लिखो("%s%*s", ptr_strbeg, len, "");
//				len = 0;
				ptr_strbeg = ptr + 1;
			}
			ptr++;
		}
		fputs_stdout(ptr_strbeg);
		मुक्त(पंक्ति);
	}
}
#पूर्ण_अगर

#अगर ENABLE_UNEXPAND
अटल व्योम unexpand(खाता *file, अचिन्हित tab_आकार, अचिन्हित opt)
{
	अक्षर *पंक्ति;

	जबतक ((पंक्ति = xmalloc_fgets(file)) != शून्य) {
		अक्षर *ptr = पंक्ति;
		अचिन्हित column = 0;

		जबतक (*ptr) {
			अचिन्हित n;
			अचिन्हित len = 0;

			जबतक (*ptr == ' ') {
				ptr++;
				len++;
			}
			column += len;
			अगर (*ptr == '\t') {
				column += tab_आकार - (column % tab_आकार);
				ptr++;
				जारी;
			}

			n = column / tab_आकार;
			अगर (n) {
				len = column = column % tab_आकार;
				जबतक (n--)
					अक्षर_दो('\t');
			}

			अगर (!(opt & OPT_ALL) && ptr != पंक्ति) {
				म_लिखो("%*s%s", len, "", ptr);
				अवरोध;
			}
			n = म_खोज(ptr, "\t ");
			म_लिखो("%*s%.*s", len, "", n, ptr);
# अगर ENABLE_UNICODE_SUPPORT
			{
				अक्षर c = ptr[n];
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
		मुक्त(पंक्ति);
	}
}
#पूर्ण_अगर

पूर्णांक expand_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक expand_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	/* Default 8 spaces for 1 tab */
	स्थिर अक्षर *opt_t = "8";
	खाता *file;
	अचिन्हित tab_आकार;
	अचिन्हित opt;
	exitcode_t exit_status = निकास_सफल;

	init_unicode();

	अगर (ENABLE_EXPAND && (!ENABLE_UNEXPAND || applet_name[0] == 'e')) {
		opt = getopt32long(argv, "it:",
				"initial\0"          No_argument       "i"
				"tabs\0"             Required_argument "t"
				, &opt_t
		);
	} अन्यथा {
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
		अगर (opt & OPT_INITIAL) opt &= ~OPT_ALL;
	}
	tab_आकार = xatou_range(opt_t, 1, अच_पूर्णांक_उच्च);

	argv += optind;

	अगर (!*argv) {
		*--argv = (अक्षर*)bb_संदेश_standard_निवेश;
	}
	करो {
		file = fopen_or_warn_stdin(*argv);
		अगर (!file) {
			exit_status = निकास_त्रुटि;
			जारी;
		}

		अगर (ENABLE_EXPAND && (!ENABLE_UNEXPAND || applet_name[0] == 'e'))
			IF_EXPAND(expand(file, tab_आकार, opt));
		अन्यथा
			IF_UNEXPAND(unexpand(file, tab_आकार, opt));

		/* Check and close the file */
		अगर (fclose_if_not_stdin(file)) {
			bb_simple_perror_संदेश(*argv);
			exit_status = निकास_त्रुटि;
		}
		/* If stdin also clear EOF */
		अगर (file == मानक_निवेश)
			साफ_त्रुटि(file);
	} जबतक (*++argv);

	/* Now close stdin also */
	/* (if we didn't read from it, it's a no-op) */
	अगर (ख_बंद(मानक_निवेश))
		bb_simple_perror_संदेश_and_die(bb_संदेश_standard_निवेश);

	fflush_stdout_and_exit(exit_status);
}
