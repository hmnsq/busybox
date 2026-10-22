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

#ಅನ್ತರ್ಭಾವ "libbb.h"

ಸ್ಥಿರ ಶೂನ್ಯ paste_files(FILE** files, ಪೂರ್ಣಾಙ್ಕ file_cnt, ವರ್ಣ* delims, ಪೂರ್ಣಾಙ್ಕ del_cnt)
{
	ವರ್ಣ *ಸಾಲು;
	ವರ್ಣ delim;
	ಪೂರ್ಣಾಙ್ಕ active_files = file_cnt;
	ಪೂರ್ಣಾಙ್ಕ i;

	ಯಾವತ್ (active_files > 0) {
		ಪೂರ್ಣಾಙ್ಕ del_idx = 0;

		ಪ್ರತಿ (i = 0; i < file_cnt; ++i) {
			ಯದಿ (files[i] == ರಿಕ್ತ)
				ಅನುವರ್ತ;

			ಸಾಲು = xmalloc_fgetline(files[i]);
			ಯದಿ (!ಸಾಲು) {
				fclose_if_not_stdin(files[i]);
				files[i] = ರಿಕ್ತ;
				--active_files;
				ಅನುವರ್ತ;
			}
			fputs_stdout(ಸಾಲು);
			ಮುಕ್ತ(ಸಾಲು);
			delim = '\n';
			ಯದಿ (i != file_cnt - 1) {
				delim = delims[del_idx++];
				ಯದಿ (del_idx == del_cnt)
					del_idx = 0;
			}
			ಯದಿ (delim != '\0')
				fputc(delim, stdout);
		}
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ paste_files_separate(FILE** files, ವರ್ಣ* delims, ಪೂರ್ಣಾಙ್ಕ del_cnt)
{
	ವರ್ಣ *ಸಾಲು, *next_ಸಾಲು;
	ವರ್ಣ delim;
	ಪೂರ್ಣಾಙ್ಕ i;

	ಪ್ರತಿ (i = 0; files[i]; ++i) {
		ಪೂರ್ಣಾಙ್ಕ del_idx = 0;

		ಸಾಲು = ರಿಕ್ತ;
		ಯಾವತ್ ((next_ಸಾಲು = xmalloc_fgetline(files[i])) != ರಿಕ್ತ) {
			ಯದಿ (ಸಾಲು) {
				fputs_stdout(ಸಾಲು);
				ಮುಕ್ತ(ಸಾಲು);
				delim = delims[del_idx++];
				ಯದಿ (del_idx == del_cnt)
					del_idx = 0;
				ಯದಿ (delim != '\0')
					fputc(delim, stdout);
			}
			ಸಾಲು = next_ಸಾಲು;
		}
		ಯದಿ (ಸಾಲು) {
			/* coreutils adds \n even if this is a final line
			 * of the last file and it was not \n-terminated.
			 */
			printf("%s\n", ಸಾಲು);
			ಮುಕ್ತ(ಸಾಲು);
		}
		fclose_if_not_stdin(files[i]);
	}
}

#define PASTE_OPT_DELIMITERS (1 << 0)
#define PASTE_OPT_SEPARATE   (1 << 1)

ಪೂರ್ಣಾಙ್ಕ paste_main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಙ್ಕ paste_main(ಪೂರ್ಣಾಙ್ಕ argc UNUSED_PARAM, ವರ್ಣ **argv)
{
	ವರ್ಣ *delims = (ವರ್ಣ*)"\t";
	ಪೂರ್ಣಾಙ್ಕ del_cnt = 1;
	unsigned opt;
	ಪೂರ್ಣಾಙ್ಕ i;

	opt = getopt32(argv, "d:s", &delims);
	argv += optind;

	ಯದಿ (opt & PASTE_OPT_DELIMITERS) {
		ಯದಿ (!delims[0])
			bb_simple_error_ಸಂದೇಶ_and_die("-d '' is not supported");
		/* unknown mappings are not changed: "\z" -> '\\' 'z' */
		/* trailing backslash, if any, is preserved */
		del_cnt = strcpy_and_process_escape_sequences(delims, delims) - delims;
		/* note: handle NUL properly (do not stop at it!): try -d'\t\0\t' */
	}

	ಯದಿ (!argv[0])
		(--argv)[0] = (ವರ್ಣ*) "-";
	ಪ್ರತಿ (i = 0; argv[i]; ++i) {
		argv[i] = (ಶೂನ್ಯ*) fopen_or_warn_stdin(argv[i]);
		ಯದಿ (!argv[i])
			xfunc_die();
	}

	ಯದಿ (opt & PASTE_OPT_SEPARATE)
		paste_files_separate((FILE**)argv, delims, del_cnt);
	ಅನ್ಯಥಾ
		paste_files((FILE**)argv, i, delims, del_cnt);

	fflush_stdout_and_exit(0);
}
