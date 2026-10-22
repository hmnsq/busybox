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

ಸ್ಥಿರ ಶೂನ್ಯ paste_files(FILE** files, ಪೂರ್ಣಾಂಕ file_cnt, ಅಕ್ಷರ* delims, ಪೂರ್ಣಾಂಕ del_cnt)
{
	ಅಕ್ಷರ *ಸಾಲು;
	ಅಕ್ಷರ delim;
	ಪೂರ್ಣಾಂಕ ಸಕ್ರಿಯ_files = file_cnt;
	ಪೂರ್ಣಾಂಕ i;

	ಆಗ (ಸಕ್ರಿಯ_files > 0) {
		ಪೂರ್ಣಾಂಕ del_idx = 0;

		ಗಾಗಿ (i = 0; i < file_cnt; ++i) {
			ಆದರೆ (files[i] == ಖಾಲಿ)
				ಮುಂದುವರಿ;

			ಸಾಲು = xmalloc_fgetline(files[i]);
			ಆದರೆ (!ಸಾಲು) {
				fclose_if_not_stdin(files[i]);
				files[i] = ಖಾಲಿ;
				--ಸಕ್ರಿಯ_files;
				ಮುಂದುವರಿ;
			}
			fputs_stdout(ಸಾಲು);
			ಬಿಡುಗಡೆ(ಸಾಲು);
			delim = '\n';
			ಆದರೆ (i != file_cnt - 1) {
				delim = delims[del_idx++];
				ಆದರೆ (del_idx == del_cnt)
					del_idx = 0;
			}
			ಆದರೆ (delim != '\0')
				fputc(delim, stdout);
		}
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ paste_files_separate(FILE** files, ಅಕ್ಷರ* delims, ಪೂರ್ಣಾಂಕ del_cnt)
{
	ಅಕ್ಷರ *ಸಾಲು, *ಮುಂದಿನ_ಸಾಲು;
	ಅಕ್ಷರ delim;
	ಪೂರ್ಣಾಂಕ i;

	ಗಾಗಿ (i = 0; files[i]; ++i) {
		ಪೂರ್ಣಾಂಕ del_idx = 0;

		ಸಾಲು = ಖಾಲಿ;
		ಆಗ ((ಮುಂದಿನ_ಸಾಲು = xmalloc_fgetline(files[i])) != ಖಾಲಿ) {
			ಆದರೆ (ಸಾಲು) {
				fputs_stdout(ಸಾಲು);
				ಬಿಡುಗಡೆ(ಸಾಲು);
				delim = delims[del_idx++];
				ಆದರೆ (del_idx == del_cnt)
					del_idx = 0;
				ಆದರೆ (delim != '\0')
					fputc(delim, stdout);
			}
			ಸಾಲು = ಮುಂದಿನ_ಸಾಲು;
		}
		ಆದರೆ (ಸಾಲು) {
			/* coreutils adds \n even if this is a final line
			 * of the last file and it was not \n-terminated.
			 */
			ಬರೆ("%s\n", ಸಾಲು);
			ಬಿಡುಗಡೆ(ಸಾಲು);
		}
		fclose_if_not_stdin(files[i]);
	}
}

#ವ್ಯಾಖ್ಯಾನಿಸು PASTE_OPT_DELIMITERS (1 << 0)
#ವ್ಯಾಖ್ಯಾನಿಸು PASTE_OPT_SEPARATE   (1 << 1)

ಪೂರ್ಣಾಂಕ paste_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ paste_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	ಅಕ್ಷರ *delims = (ಅಕ್ಷರ*)"\t";
	ಪೂರ್ಣಾಂಕ del_cnt = 1;
	ಚಿಹ್ನೆರಹಿತ opt;
	ಪೂರ್ಣಾಂಕ i;

	opt = getopt32(argv, "d:s", &delims);
	argv += optind;

	ಆದರೆ (opt & PASTE_OPT_DELIMITERS) {
		ಆದರೆ (!delims[0])
			bb_simple_error_ಸಂದೇಶ_and_die("-d '' is not supported");
		/* unknown mappings are not changed: "\z" -> '\\' 'z' */
		/* trailing backslash, if any, is preserved */
		del_cnt = strcpy_and_process_escape_sequences(delims, delims) - delims;
		/* note: handle NUL properly (do not stop at it!): try -d'\t\0\t' */
	}

	ಆದರೆ (!argv[0])
		(--argv)[0] = (ಅಕ್ಷರ*) "-";
	ಗಾಗಿ (i = 0; argv[i]; ++i) {
		argv[i] = (ಶೂನ್ಯ*) fopen_or_warn_stdin(argv[i]);
		ಆದರೆ (!argv[i])
			xfunc_die();
	}

	ಆದರೆ (opt & PASTE_OPT_SEPARATE)
		paste_files_separate((FILE**)argv, delims, del_cnt);
	ಇಲ್ಲದಿದ್ದರೆ
		paste_files((FILE**)argv, i, delims, del_cnt);

	fflush_stdout_and_exit(0);
}
