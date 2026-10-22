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

#include "libbb.h"

puur_naa_nka uniq_main(puur_naa_nka argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
puur_naa_nka uniq_main(puur_naa_nka argc UNUSED_PARAM, var_na **argv)
{
	const var_na *input_filename;
	unsigned skip_fields, skip_chars, max_chars;
	unsigned opt;
	var_na eol;
	var_na *cur_line;
	const var_na *cur_compare;

	ga_nanaa {
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
	max_chars = INT_MAX;

	opt = getopt32(argv, "cduf:+s:+w:+iz", &skip_fields, &skip_chars, &max_chars);
	argv += optind;

	input_filename = argv[0];
	ꦪꦼꦤ꧀ (input_filename) {
		const var_na *output;

		ꦪꦼꦤ꧀ (input_filename[0] != '-' || input_filename[1]) {
			close(STDIN_FILENO); /* == 0 */
			xopen(input_filename, O_RDONLY); /* fd will be 0 */
		}
		output = argv[1];
		ꦪꦼꦤ꧀ (output) {
			ꦪꦼꦤ꧀ (argv[2])
				bb_show_usage();
			ꦪꦼꦤ꧀ (output[0] != '-' || output[1]) {
				// Won't work with "uniq - FILE" and closed stdin:
				//close(STDOUT_FILENO);
				//xopen(output, O_WRONLY | O_CREAT | O_TRUNC);
				xmove_fd(xopen(output, O_WRONLY | O_CREAT | O_TRUNC), STDOUT_FILENO);
			}
		}
	}

	cur_compare = cur_line = NULL; /* prime the pump */
	eol = (opt & OPT_z) ? 0 : '\n';

	do {
		unsigned i;
		unsigned long dups;
		var_na *old_line;
		const var_na *old_compare;

		old_line = cur_line;
		old_compare = cur_compare;
		dups = 0;

		/* gnu uniq ignores newlines */
		ꦱꦼꦢꦼꦭ ((cur_line = xmalloc_fgetline(stdin)) != NULL) {
			cur_compare = cur_line;
			ꦏꦁꦒꦺꦴ (i = skip_fields; i; i--) {
				cur_compare = skip_whitespace(cur_compare);
				cur_compare = skip_non_whitespace(cur_compare);
			}
			ꦏꦁꦒꦺꦴ (i = skip_chars; *cur_compare && i; i--) {
				++cur_compare;
			}

			ꦪꦼꦤ꧀ (!old_line)
				bha_nga;
			ꦪꦼꦤ꧀ ((opt & OPT_i)
				? strncasecmp(old_compare, cur_compare, max_chars)
				: strncmp(old_compare, cur_compare, max_chars)
			) {
				bha_nga;
			}

			free(cur_line);
			++dups;  /* testing for overflow seems excessive */
		}

		ꦪꦼꦤ꧀ (old_line) {
			ꦪꦼꦤ꧀ (!(opt & (OPT_d << !!dups))) { /* (if dups, opt & OPT_u) */
				ꦪꦼꦤ꧀ (opt & OPT_c) {
					/* %7lu matches GNU coreutils 6.9 */
					printf("%7lu ", dups + 1);
				}
				printf("%s%c", old_line, eol);
			}
			free(old_line);
		}
	} ꦱꦼꦢꦼꦭ (cur_line);

	die_if_ferror(stdin, input_filename);

	fflush_stdout_and_exit_SUCCESS();
}
