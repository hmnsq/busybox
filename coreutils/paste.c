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

#അന്തര്ഭാവ "libbb.h"

സ്ഥിതം ശൂന്യം paste_files(FILE** files, പൂർണ്ണസംഖ്യ file_cnt, അക്ഷരം* delims, പൂർണ്ണസംഖ്യ del_cnt)
{
	അക്ഷരം *വരി;
	അക്ഷരം delim;
	പൂർണ്ണസംഖ്യ active_files = file_cnt;
	പൂർണ്ണസംഖ്യ i;

	ആകുമ്പോൾ (active_files > 0) {
		പൂർണ്ണസംഖ്യ del_idx = 0;

		വേണ്ടി (i = 0; i < file_cnt; ++i) {
			എങ്കിൽ (files[i] == ശൂന്യ)
				തുടരുക;

			വരി = xmalloc_fgetline(files[i]);
			എങ്കിൽ (!വരി) {
				fclose_if_not_stdin(files[i]);
				files[i] = ശൂന്യ;
				--active_files;
				തുടരുക;
			}
			fputs_stdout(വരി);
			സ്വതന്ത്രം(വരി);
			delim = '\n';
			എങ്കിൽ (i != file_cnt - 1) {
				delim = delims[del_idx++];
				എങ്കിൽ (del_idx == del_cnt)
					del_idx = 0;
			}
			എങ്കിൽ (delim != '\0')
				fputc(delim, stdout);
		}
	}
}

സ്ഥിതം ശൂന്യം paste_files_separate(FILE** files, അക്ഷരം* delims, പൂർണ്ണസംഖ്യ del_cnt)
{
	അക്ഷരം *വരി, *next_വരി;
	അക്ഷരം delim;
	പൂർണ്ണസംഖ്യ i;

	വേണ്ടി (i = 0; files[i]; ++i) {
		പൂർണ്ണസംഖ്യ del_idx = 0;

		വരി = ശൂന്യ;
		ആകുമ്പോൾ ((next_വരി = xmalloc_fgetline(files[i])) != ശൂന്യ) {
			എങ്കിൽ (വരി) {
				fputs_stdout(വരി);
				സ്വതന്ത്രം(വരി);
				delim = delims[del_idx++];
				എങ്കിൽ (del_idx == del_cnt)
					del_idx = 0;
				എങ്കിൽ (delim != '\0')
					fputc(delim, stdout);
			}
			വരി = next_വരി;
		}
		എങ്കിൽ (വരി) {
			/* coreutils adds \n even if this is a final line
			 * of the last file and it was not \n-terminated.
			 */
			printf("%s\n", വരി);
			സ്വതന്ത്രം(വരി);
		}
		fclose_if_not_stdin(files[i]);
	}
}

#define PASTE_OPT_DELIMITERS (1 << 0)
#define PASTE_OPT_SEPARATE   (1 << 1)

പൂർണ്ണസംഖ്യ paste_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) MAIN_EXTERNALLY_VISIBLE;
പൂർണ്ണസംഖ്യ paste_main(പൂർണ്ണസംഖ്യ argc UNUSED_PARAM, അക്ഷരം **argv)
{
	അക്ഷരം *delims = (അക്ഷരം*)"\t";
	പൂർണ്ണസംഖ്യ del_cnt = 1;
	unsigned opt;
	പൂർണ്ണസംഖ്യ i;

	opt = getopt32(argv, "d:s", &delims);
	argv += optind;

	എങ്കിൽ (opt & PASTE_OPT_DELIMITERS) {
		എങ്കിൽ (!delims[0])
			bb_simple_error_സന്ദേശം_and_die("-d '' is not supported");
		/* unknown mappings are not changed: "\z" -> '\\' 'z' */
		/* trailing backslash, if any, is preserved */
		del_cnt = strcpy_and_process_escape_sequences(delims, delims) - delims;
		/* note: handle NUL properly (do not stop at it!): try -d'\t\0\t' */
	}

	എങ്കിൽ (!argv[0])
		(--argv)[0] = (അക്ഷരം*) "-";
	വേണ്ടി (i = 0; argv[i]; ++i) {
		argv[i] = (ശൂന്യം*) fopen_or_warn_stdin(argv[i]);
		എങ്കിൽ (!argv[i])
			xfunc_die();
	}

	എങ്കിൽ (opt & PASTE_OPT_SEPARATE)
		paste_files_separate((FILE**)argv, delims, del_cnt);
	അല്ലെങ്കിൽ
		paste_files((FILE**)argv, i, delims, del_cnt);

	fflush_stdout_and_exit(0);
}
