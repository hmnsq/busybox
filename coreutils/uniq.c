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

#ಅನ್ತರ್ಭಾವ "libbb.h"

ಪೂರ್ಣಾಙ್ಕ uniq_main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಙ್ಕ uniq_main(ಪೂರ್ಣಾಙ್ಕ argc UNUSED_PARAM, ವರ್ಣ **argv)
{
	ಧ್ರುವ ವರ್ಣ *ಒಳಹರಿವು_filename;
	unsigned skip_fields, skip_chars, max_chars;
	unsigned opt;
	ವರ್ಣ eol;
	ವರ್ಣ *cur_ಸಾಲು;
	ಧ್ರುವ ವರ್ಣ *cur_compare;

	ಗಣನಾ {
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

	ಒಳಹರಿವು_filename = argv[0];
	ಯದಿ (ಒಳಹರಿವು_filename) {
		ಧ್ರುವ ವರ್ಣ *output;

		ಯದಿ (ಒಳಹರಿವು_filename[0] != '-' || ಒಳಹರಿವು_filename[1]) {
			close(STDIN_FILENO); /* == 0 */
			xopen(ಒಳಹರಿವು_filename, O_RDONLY); /* fd will be 0 */
		}
		output = argv[1];
		ಯದಿ (output) {
			ಯದಿ (argv[2])
				bb_show_usage();
			ಯದಿ (output[0] != '-' || output[1]) {
				// Won't work with "uniq - FILE" and closed stdin:
				//close(STDOUT_FILENO);
				//xopen(output, O_WRONLY | O_CREAT | O_TRUNC);
				xmove_fd(xopen(output, O_WRONLY | O_CREAT | O_TRUNC), STDOUT_FILENO);
			}
		}
	}

	cur_compare = cur_ಸಾಲು = ರಿಕ್ತ; /* prime the pump */
	eol = (opt & OPT_z) ? 0 : '\n';

	ಕುರು {
		unsigned i;
		unsigned long dups;
		ವರ್ಣ *old_ಸಾಲು;
		ಧ್ರುವ ವರ್ಣ *old_compare;

		old_ಸಾಲು = cur_ಸಾಲು;
		old_compare = cur_compare;
		dups = 0;

		/* gnu uniq ignores newlines */
		ಯಾವತ್ ((cur_ಸಾಲು = xmalloc_fgetline(stdin)) != ರಿಕ್ತ) {
			cur_compare = cur_ಸಾಲು;
			ಪ್ರತಿ (i = skip_fields; i; i--) {
				cur_compare = skip_whitespace(cur_compare);
				cur_compare = skip_non_whitespace(cur_compare);
			}
			ಪ್ರತಿ (i = skip_chars; *cur_compare && i; i--) {
				++cur_compare;
			}

			ಯದಿ (!old_ಸಾಲು)
				ಭಙ್ಗ;
			ಯದಿ ((opt & OPT_i)
				? strncasecmp(old_compare, cur_compare, max_chars)
				: strncmp(old_compare, cur_compare, max_chars)
			) {
				ಭಙ್ಗ;
			}

			ಮುಕ್ತ(cur_ಸಾಲು);
			++dups;  /* testing for overflow seems excessive */
		}

		ಯದಿ (old_ಸಾಲು) {
			ಯದಿ (!(opt & (OPT_d << !!dups))) { /* (if dups, opt & OPT_u) */
				ಯದಿ (opt & OPT_c) {
					/* %7lu matches GNU coreutils 6.9 */
					printf("%7lu ", dups + 1);
				}
				printf("%s%c", old_ಸಾಲು, eol);
			}
			ಮುಕ್ತ(old_ಸಾಲು);
		}
	} ಯಾವತ್ (cur_ಸಾಲು);

	die_if_ferror(stdin, ಒಳಹರಿವು_filename);

	fflush_stdout_and_exit_SUCCESS();
}
