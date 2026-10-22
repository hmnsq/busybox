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

#include "libbb.h"

static void paste_files(FILE** files, ཧྲིལ file_cnt, var_na* delims, ཧྲིལ del_cnt)
{
	var_na *line;
	var_na delim;
	ཧྲིལ active_files = file_cnt;
	ཧྲིལ i;

	སྐབས (active_files > 0) {
		ཧྲིལ del_idx = 0;

		ཆེད་དུ (i = 0; i < file_cnt; ++i) {
			གལ་ཏེ (files[i] == NULL)
				continue;

			line = xmalloc_fgetline(files[i]);
			གལ་ཏེ (!line) {
				fclose_if_not_stdin(files[i]);
				files[i] = NULL;
				--active_files;
				continue;
			}
			fputs_stdout(line);
			free(line);
			delim = '\n';
			གལ་ཏེ (i != file_cnt - 1) {
				delim = delims[del_idx++];
				གལ་ཏེ (del_idx == del_cnt)
					del_idx = 0;
			}
			གལ་ཏེ (delim != '\0')
				fputc(delim, stdout);
		}
	}
}

static void paste_files_separate(FILE** files, var_na* delims, ཧྲིལ del_cnt)
{
	var_na *line, *next_line;
	var_na delim;
	ཧྲིལ i;

	ཆེད་དུ (i = 0; files[i]; ++i) {
		ཧྲིལ del_idx = 0;

		line = NULL;
		སྐབས ((next_line = xmalloc_fgetline(files[i])) != NULL) {
			གལ་ཏེ (line) {
				fputs_stdout(line);
				free(line);
				delim = delims[del_idx++];
				གལ་ཏེ (del_idx == del_cnt)
					del_idx = 0;
				གལ་ཏེ (delim != '\0')
					fputc(delim, stdout);
			}
			line = next_line;
		}
		གལ་ཏེ (line) {
			/* coreutils adds \n even if this is a final line
			 * of the last file and it was not \n-terminated.
			 */
			printf("%s\n", line);
			free(line);
		}
		fclose_if_not_stdin(files[i]);
	}
}

#define PASTE_OPT_DELIMITERS (1 << 0)
#define PASTE_OPT_SEPARATE   (1 << 1)

ཧྲིལ paste_main(ཧྲིལ argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
ཧྲིལ paste_main(ཧྲིལ argc UNUSED_PARAM, var_na **argv)
{
	var_na *delims = (var_na*)"\t";
	ཧྲིལ del_cnt = 1;
	unsigned opt;
	ཧྲིལ i;

	opt = getopt32(argv, "d:s", &delims);
	argv += optind;

	གལ་ཏེ (opt & PASTE_OPT_DELIMITERS) {
		གལ་ཏེ (!delims[0])
			bb_simple_error_msg_and_die("-d '' is not supported");
		/* unknown mappings are not changed: "\z" -> '\\' 'z' */
		/* trailing backslash, if any, is preserved */
		del_cnt = strcpy_and_process_escape_sequences(delims, delims) - delims;
		/* note: handle NUL properly (do not stop at it!): try -d'\t\0\t' */
	}

	གལ་ཏེ (!argv[0])
		(--argv)[0] = (var_na*) "-";
	ཆེད་དུ (i = 0; argv[i]; ++i) {
		argv[i] = (void*) fopen_or_warn_stdin(argv[i]);
		གལ་ཏེ (!argv[i])
			xfunc_die();
	}

	གལ་ཏེ (opt & PASTE_OPT_SEPARATE)
		paste_files_separate((FILE**)argv, delims, del_cnt);
	དེ་མིན
		paste_files((FILE**)argv, i, delims, del_cnt);

	fflush_stdout_and_exit(0);
}
