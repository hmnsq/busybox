/* vi: set sw=4 ts=4: */
/*
 * uniq implementation for busybox
 *
 * Copyright (C) 2005  Manuel Novoa III  <mjn3@codepoet.org>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config UNIQ
//config:	bool "uniq (5.1 kb)"
//config:	default y
//config:	help
//config:	uniq is used to remove duplicate lines from a sorted file.

//applet:IF_UNIQ(APPLET(uniq, BB_DIR_USR_BIN, BB_SUID_DROP))

//kbuild:lib-$(CONFIG_UNIQ) += uniq.o

/* BB_AUDIT SUSv3 compliant */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/uniq.html */

//usage:#define uniq_trivial_usage
//usage:       "[-cduiz] [-f,s,w N] [FILE [OUTFILE]]"
//usage:#define uniq_full_usage "\n\n"
//usage:       "Discard duplicate lines\n"
//usage:     "\n	-c	Prefix lines by the number of occurrences"
//usage:     "\n	-d	Only print duplicate lines"
//usage:     "\n	-u	Only print unique lines"
//usage:     "\n	-i	Ignore case"
//usage:     "\n	-z	NUL terminated output"
//usage:     "\n	-f N	Skip first N fields"
//usage:     "\n	-s N	Skip first N chars (after any skipped fields)"
//usage:     "\n	-w N	Compare N characters in line"
//usage:
//usage:#define uniq_example_usage
//usage:       "$ echo -e \"a\\na\\nb\\nc\\nc\\na\" | sort | uniq\n"
//usage:       "a\n"
//usage:       "b\n"
//usage:       "c\n"

#अन्तर्भाव "libbb.h"

पूर्णांक uniq_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक uniq_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	स्थिरांक अक्षर *आदान_filename;
	अचिन्हित skip_fields, skip_chars, max_chars;
	अचिन्हित opt;
	अक्षर eol;
	अक्षर *cur_ओळ;
	स्थिरांक अक्षर *cur_compare;

	गणना {
		OPT_c = 1 << 0,
		OPT_d = 1 << 1, /* print only dups */
		OPT_u = 1 << 2, /* print only uniq */
		OPT_f = 1 << 3,
		OPT_s = 1 << 4,
		OPT_w = 1 << 5,
		OPT_i = 1 << 6,
		OPT_z = 1 << 7,
	};

	skip_fields = skip_chars = 0;
	max_chars = पूर्णांक_उच्च;

	opt = getopt32(argv, "cduf:+s:+w:+iz", &skip_fields, &skip_chars, &max_chars);
	argv += optind;

	आदान_filename = argv[0];
	जर (आदान_filename) {
		स्थिरांक अक्षर *output;

		जर (आदान_filename[0] != '-' || आदान_filename[1]) {
			बंद(STDIN_FILENO); /* == 0 */
			xopen(आदान_filename, O_RDONLY); /* fd will be 0 */
		}
		output = argv[1];
		जर (output) {
			जर (argv[2])
				bb_show_usage();
			जर (output[0] != '-' || output[1]) {
				// Won't work with "uniq - FILE" and closed stdin:
				//close(STDOUT_FILENO);
				//xopen(output, O_WRONLY | O_CREAT | O_TRUNC);
				xmove_fd(xopen(output, O_WRONLY | O_CREAT | O_TRUNC), STDOUT_FILENO);
			}
		}
	}

	cur_compare = cur_ओळ = रिक्त; /* prime the pump */
	eol = (opt & OPT_z) ? 0 : '\n';

	करा {
		अचिन्हित i;
		अचिन्हित दीर्घ dups;
		अक्षर *old_ओळ;
		स्थिरांक अक्षर *old_compare;

		old_ओळ = cur_ओळ;
		old_compare = cur_compare;
		dups = 0;

		/* gnu uniq ignores newlines */
		पर्यंत ((cur_ओळ = xmalloc_fgetline(मानक_निवेश)) != रिक्त) {
			cur_compare = cur_ओळ;
			साठी (i = skip_fields; i; i--) {
				cur_compare = skip_whitespace(cur_compare);
				cur_compare = skip_non_whitespace(cur_compare);
			}
			साठी (i = skip_chars; *cur_compare && i; i--) {
				++cur_compare;
			}

			जर (!old_ओळ)
				थांबा;
			जर ((opt & OPT_i)
				? strncasecmp(old_compare, cur_compare, max_chars)
				: म_भेदन(old_compare, cur_compare, max_chars)
			) {
				थांबा;
			}

			मुक्त(cur_ओळ);
			++dups;  /* testing for overflow seems excessive */
		}

		जर (old_ओळ) {
			जर (!(opt & (OPT_d << !!dups))) { /* (if dups, opt & OPT_u) */
				जर (opt & OPT_c) {
					/* %7lu matches GNU coreutils 6.9 */
					म_लिखो("%7lu ", dups + 1);
				}
				म_लिखो("%s%c", old_ओळ, eol);
			}
			मुक्त(old_ओळ);
		}
	} पर्यंत (cur_ओळ);

	die_if_ferror(मानक_निवेश, आदान_filename);

	fflush_stdout_and_exit_SUCCESS();
}
