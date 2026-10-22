/* vi: set sw=4 ts=4: */
/*
 * paste.c - implementation of the posix paste command
 *
 * Written by Maxime Coste <mawww@kakoune.org>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config PASTE
//config:	bool "paste (5.1 kb)"
//config:	default y
//config:	help
//config:	paste is used to paste lines of different files together
//config:	and write the result to stdout

//applet:IF_PASTE(APPLET_NOEXEC(paste, paste, BB_DIR_USR_BIN, BB_SUID_DROP, paste))

//kbuild:lib-$(CONFIG_PASTE) += paste.o

//usage:#define paste_trivial_usage
//usage:       "[-d LIST] [-s] [FILE]..."
//usage:#define paste_full_usage "\n\n"
//usage:       "Paste lines from each input file, separated with tab\n"
//usage:     "\n	-d LIST	Use delimiters from LIST, not tab"
//usage:     "\n	-s      Serial: one file at a time"
//usage:
//usage:#define paste_example_usage
//usage:       "# write out directory in four columns\n"
//usage:       "$ ls | paste - - - -\n"
//usage:       "# combine pairs of lines from a file into single lines\n"
//usage:       "$ paste -s -d '\\t\\n' file\n"

#అన్తర్భావ "libbb.h"

స్థిర శూన్య paste_files(FILE** files, పూర్ణాంకం file_cnt, అక్షరం* delims, పూర్ణాంకం del_cnt)
{
	అక్షరం *పంక్తి;
	అక్షరం delim;
	పూర్ణాంకం చురుకైన_files = file_cnt;
	పూర్ణాంకం i;

	అయితే (చురుకైన_files > 0) {
		పూర్ణాంకం del_idx = 0;

		కొరకు (i = 0; i < file_cnt; ++i) {
			ఒకవేళ (files[i] == ఖాళీ)
				కొనసాగు;

			పంక్తి = xmalloc_fgetline(files[i]);
			ఒకవేళ (!పంక్తి) {
				fclose_if_not_stdin(files[i]);
				files[i] = ఖాళీ;
				--చురుకైన_files;
				కొనసాగు;
			}
			fputs_stdout(పంక్తి);
			విడుదల(పంక్తి);
			delim = '\n';
			ఒకవేళ (i != file_cnt - 1) {
				delim = delims[del_idx++];
				ఒకవేళ (del_idx == del_cnt)
					del_idx = 0;
			}
			ఒకవేళ (delim != '\0')
				fputc(delim, stdout);
		}
	}
}

స్థిర శూన్య paste_files_separate(FILE** files, అక్షరం* delims, పూర్ణాంకం del_cnt)
{
	అక్షరం *పంక్తి, *తదుపరి_పంక్తి;
	అక్షరం delim;
	పూర్ణాంకం i;

	కొరకు (i = 0; files[i]; ++i) {
		పూర్ణాంకం del_idx = 0;

		పంక్తి = ఖాళీ;
		అయితే ((తదుపరి_పంక్తి = xmalloc_fgetline(files[i])) != ఖాళీ) {
			ఒకవేళ (పంక్తి) {
				fputs_stdout(పంక్తి);
				విడుదల(పంక్తి);
				delim = delims[del_idx++];
				ఒకవేళ (del_idx == del_cnt)
					del_idx = 0;
				ఒకవేళ (delim != '\0')
					fputc(delim, stdout);
			}
			పంక్తి = తదుపరి_పంక్తి;
		}
		ఒకవేళ (పంక్తి) {
			/* coreutils adds \n even if this is a final line
			 * of the last file and it was not \n-terminated.
			 */
			రాయి("%s\n", పంక్తి);
			విడుదల(పంక్తి);
		}
		fclose_if_not_stdin(files[i]);
	}
}

#నిర్వచించు PASTE_OPT_DELIMITERS (1 << 0)
#నిర్వచించు PASTE_OPT_SEPARATE   (1 << 1)

పూర్ణాంకం paste_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం paste_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	అక్షరం *delims = (అక్షరం*)"\t";
	పూర్ణాంకం del_cnt = 1;
	గుర్తులేని opt;
	పూర్ణాంకం i;

	opt = getopt32(argv, "d:s", &delims);
	argv += optind;

	ఒకవేళ (opt & PASTE_OPT_DELIMITERS) {
		ఒకవేళ (!delims[0])
			bb_simple_error_సందేశం_and_die("-d '' is not supported");
		/* unknown mappings are not changed: "\z" -> '\\' 'z' */
		/* trailing backslash, if any, is preserved */
		del_cnt = strcpy_and_process_escape_sequences(delims, delims) - delims;
		/* note: handle NUL properly (do not stop at it!): try -d'\t\0\t' */
	}

	ఒకవేళ (!argv[0])
		(--argv)[0] = (అక్షరం*) "-";
	కొరకు (i = 0; argv[i]; ++i) {
		argv[i] = (శూన్య*) fopen_or_warn_stdin(argv[i]);
		ఒకవేళ (!argv[i])
			xfunc_die();
	}

	ఒకవేళ (opt & PASTE_OPT_SEPARATE)
		paste_files_separate((FILE**)argv, delims, del_cnt);
	లేకపోతే
		paste_files((FILE**)argv, i, delims, del_cnt);

	fflush_stdout_and_exit(0);
}
