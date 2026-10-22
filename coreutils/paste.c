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

static void paste_files(FILE** files, 整数 file_cnt, char* delims, 整数 del_cnt)
{
	char *line;
	char delim;
	整数 active_files = file_cnt;
	整数 i;

	当 (active_files > 0) {
		整数 del_idx = 0;

		为 (i = 0; i < file_cnt; ++i) {
			如果 (files[i] == NULL)
				continue;

			line = xmalloc_fgetline(files[i]);
			如果 (!line) {
				fclose_if_not_stdin(files[i]);
				files[i] = NULL;
				--active_files;
				continue;
			}
			fputs_stdout(line);
			free(line);
			delim = '\n';
			如果 (i != file_cnt - 1) {
				delim = delims[del_idx++];
				如果 (del_idx == del_cnt)
					del_idx = 0;
			}
			如果 (delim != '\0')
				fputc(delim, stdout);
		}
	}
}

static void paste_files_separate(FILE** files, char* delims, 整数 del_cnt)
{
	char *line, *next_line;
	char delim;
	整数 i;

	为 (i = 0; files[i]; ++i) {
		整数 del_idx = 0;

		line = NULL;
		当 ((next_line = xmalloc_fgetline(files[i])) != NULL) {
			如果 (line) {
				fputs_stdout(line);
				free(line);
				delim = delims[del_idx++];
				如果 (del_idx == del_cnt)
					del_idx = 0;
				如果 (delim != '\0')
					fputc(delim, stdout);
			}
			line = next_line;
		}
		如果 (line) {
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

整数 paste_main(整数 argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
整数 paste_main(整数 argc UNUSED_PARAM, char **argv)
{
	char *delims = (char*)"\t";
	整数 del_cnt = 1;
	unsigned opt;
	整数 i;

	opt = getopt32(argv, "d:s", &delims);
	argv += optind;

	如果 (opt & PASTE_OPT_DELIMITERS) {
		如果 (!delims[0])
			bb_simple_error_msg_and_die("-d '' is not supported");
		/* unknown mappings are not changed: "\z" -> '\\' 'z' */
		/* trailing backslash, if any, is preserved */
		del_cnt = strcpy_and_process_escape_sequences(delims, delims) - delims;
		/* note: handle NUL properly (do not stop at it!): try -d'\t\0\t' */
	}

	如果 (!argv[0])
		(--argv)[0] = (char*) "-";
	为 (i = 0; argv[i]; ++i) {
		argv[i] = (void*) fopen_or_warn_stdin(argv[i]);
		如果 (!argv[i])
			xfunc_die();
	}

	如果 (opt & PASTE_OPT_SEPARATE)
		paste_files_separate((FILE**)argv, delims, del_cnt);
	否则
		paste_files((FILE**)argv, i, delims, del_cnt);

	fflush_stdout_and_exit(0);
}
