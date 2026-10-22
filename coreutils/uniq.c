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

#అన్తర్భావ "libbb.h"

పూర్ణాంకం uniq_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం uniq_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	స్థిరరాశి అక్షరం *ఆదానం_filename;
	గుర్తులేని skip_fields, skip_chars, max_chars;
	గుర్తులేని opt;
	అక్షరం eol;
	అక్షరం *cur_పంక్తి;
	స్థిరరాశి అక్షరం *cur_compare;

	గణన {
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
	max_chars = గరిష్ఠ_పూర్ణాంకం;

	opt = getopt32(argv, "cduf:+s:+w:+iz", &skip_fields, &skip_chars, &max_chars);
	argv += optind;

	ఆదానం_filename = argv[0];
	ఒకవేళ (ఆదానం_filename) {
		స్థిరరాశి అక్షరం *output;

		ఒకవేళ (ఆదానం_filename[0] != '-' || ఆదానం_filename[1]) {
			close(STDIN_FILENO); /* == 0 */
			xopen(ఆదానం_filename, O_RDONLY); /* fd will be 0 */
		}
		output = argv[1];
		ఒకవేళ (output) {
			ఒకవేళ (argv[2])
				bb_show_usage();
			ఒకవేళ (output[0] != '-' || output[1]) {
				// Won't work with "uniq - FILE" and closed stdin:
				//close(STDOUT_FILENO);
				//xopen(output, O_WRONLY | O_CREAT | O_TRUNC);
				xmove_fd(xopen(output, O_WRONLY | O_CREAT | O_TRUNC), STDOUT_FILENO);
			}
		}
	}

	cur_compare = cur_పంక్తి = ఖాళీ; /* prime the pump */
	eol = (opt & OPT_z) ? 0 : '\n';

	చేయి {
		గుర్తులేని i;
		గుర్తులేని దీర్ఘ dups;
		అక్షరం *old_పంక్తి;
		స్థిరరాశి అక్షరం *old_compare;

		old_పంక్తి = cur_పంక్తి;
		old_compare = cur_compare;
		dups = 0;

		/* gnu uniq ignores newlines */
		అయితే ((cur_పంక్తి = xmalloc_fgetline(stdin)) != ఖాళీ) {
			cur_compare = cur_పంక్తి;
			కొరకు (i = skip_fields; i; i--) {
				cur_compare = skip_whitespace(cur_compare);
				cur_compare = skip_non_whitespace(cur_compare);
			}
			కొరకు (i = skip_chars; *cur_compare && i; i--) {
				++cur_compare;
			}

			ఒకవేళ (!old_పంక్తి)
				ఆపు;
			ఒకవేళ ((opt & OPT_i)
				? strncasecmp(old_compare, cur_compare, max_chars)
				: అక్షరశ్రేణి_పోలిక(old_compare, cur_compare, max_chars)
			) {
				ఆపు;
			}

			విడుదల(cur_పంక్తి);
			++dups;  /* testing for overflow seems excessive */
		}

		ఒకవేళ (old_పంక్తి) {
			ఒకవేళ (!(opt & (OPT_d << !!dups))) { /* (if dups, opt & OPT_u) */
				ఒకవేళ (opt & OPT_c) {
					/* %7lu matches GNU coreutils 6.9 */
					రాయి("%7lu ", dups + 1);
				}
				రాయి("%s%c", old_పంక్తి, eol);
			}
			విడుదల(old_పంక్తి);
		}
	} అయితే (cur_పంక్తి);

	die_if_ferror(stdin, ఆదానం_filename);

	fflush_stdout_and_exit_SUCCESS();
}
