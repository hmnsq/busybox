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

पूर्णाङ्क uniq_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क uniq_main(पूर्णाङ्क argc UNUSED_PARAM, वर्ण **argv)
{
	ध्रुव वर्ण *निवेश_filename;
	अचिन्हित skip_fields, skip_chars, max_chars;
	अचिन्हित opt;
	वर्ण eol;
	वर्ण *cur_पंक्ति;
	ध्रुव वर्ण *cur_compare;

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

	निवेश_filename = argv[0];
	यदि (निवेश_filename) {
		ध्रुव वर्ण *output;

		यदि (निवेश_filename[0] != '-' || निवेश_filename[1]) {
			बंद(STDIN_FILENO); /* == 0 */
			xopen(निवेश_filename, O_RDONLY); /* fd will be 0 */
		}
		output = argv[1];
		यदि (output) {
			यदि (argv[2])
				bb_show_usage();
			यदि (output[0] != '-' || output[1]) {
				// Won't work with "uniq - FILE" and closed stdin:
				//close(STDOUT_FILENO);
				//xopen(output, O_WRONLY | O_CREAT | O_TRUNC);
				xmove_fd(xopen(output, O_WRONLY | O_CREAT | O_TRUNC), STDOUT_FILENO);
			}
		}
	}

	cur_compare = cur_पंक्ति = रिक्त; /* prime the pump */
	eol = (opt & OPT_z) ? 0 : '\n';

	कुरु {
		अचिन्हित i;
		अचिन्हित दीर्घ dups;
		वर्ण *old_पंक्ति;
		ध्रुव वर्ण *old_compare;

		old_पंक्ति = cur_पंक्ति;
		old_compare = cur_compare;
		dups = 0;

		/* gnu uniq ignores newlines */
		यावत् ((cur_पंक्ति = xmalloc_fgetline(मानक_निवेश)) != रिक्त) {
			cur_compare = cur_पंक्ति;
			प्रति (i = skip_fields; i; i--) {
				cur_compare = skip_whitespace(cur_compare);
				cur_compare = skip_non_whitespace(cur_compare);
			}
			प्रति (i = skip_chars; *cur_compare && i; i--) {
				++cur_compare;
			}

			यदि (!old_पंक्ति)
				भङ्ग;
			यदि ((opt & OPT_i)
				? strncasecmp(old_compare, cur_compare, max_chars)
				: म_भेदन(old_compare, cur_compare, max_chars)
			) {
				भङ्ग;
			}

			मुक्त(cur_पंक्ति);
			++dups;  /* testing for overflow seems excessive */
		}

		यदि (old_पंक्ति) {
			यदि (!(opt & (OPT_d << !!dups))) { /* (if dups, opt & OPT_u) */
				यदि (opt & OPT_c) {
					/* %7lu matches GNU coreutils 6.9 */
					म_लिखो("%7lu ", dups + 1);
				}
				म_लिखो("%s%c", old_पंक्ति, eol);
			}
			मुक्त(old_पंक्ति);
		}
	} यावत् (cur_पंक्ति);

	die_if_ferror(मानक_निवेश, निवेश_filename);

	fflush_stdout_and_exit_SUCCESS();
}
