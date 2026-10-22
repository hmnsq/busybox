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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"

ਸਥਿਰ ਖਾਲੀ paste_files(FILE** files, ਪੂਰਨ file_cnt, ਅੱਖਰ* delims, ਪੂਰਨ del_cnt)
{
	ਅੱਖਰ *ਸਤਰ;
	ਅੱਖਰ delim;
	ਪੂਰਨ ਸਰਗਰਮ_files = file_cnt;
	ਪੂਰਨ i;

	ਜਦੋਂ (ਸਰਗਰਮ_files > 0) {
		ਪੂਰਨ del_idx = 0;

		ਲਈ (i = 0; i < file_cnt; ++i) {
			ਜੇ (files[i] == ਨਿਰਾ)
				ਜਾਰੀ;

			ਸਤਰ = xmalloc_fgetline(files[i]);
			ਜੇ (!ਸਤਰ) {
				fclose_if_not_stdin(files[i]);
				files[i] = ਨਿਰਾ;
				--ਸਰਗਰਮ_files;
				ਜਾਰੀ;
			}
			fputs_stdout(ਸਤਰ);
			ਮੁਕਤ(ਸਤਰ);
			delim = '\n';
			ਜੇ (i != file_cnt - 1) {
				delim = delims[del_idx++];
				ਜੇ (del_idx == del_cnt)
					del_idx = 0;
			}
			ਜੇ (delim != '\0')
				fputc(delim, stdout);
		}
	}
}

ਸਥਿਰ ਖਾਲੀ paste_files_separate(FILE** files, ਅੱਖਰ* delims, ਪੂਰਨ del_cnt)
{
	ਅੱਖਰ *ਸਤਰ, *ਅਗਲਾ_ਸਤਰ;
	ਅੱਖਰ delim;
	ਪੂਰਨ i;

	ਲਈ (i = 0; files[i]; ++i) {
		ਪੂਰਨ del_idx = 0;

		ਸਤਰ = ਨਿਰਾ;
		ਜਦੋਂ ((ਅਗਲਾ_ਸਤਰ = xmalloc_fgetline(files[i])) != ਨਿਰਾ) {
			ਜੇ (ਸਤਰ) {
				fputs_stdout(ਸਤਰ);
				ਮੁਕਤ(ਸਤਰ);
				delim = delims[del_idx++];
				ਜੇ (del_idx == del_cnt)
					del_idx = 0;
				ਜੇ (delim != '\0')
					fputc(delim, stdout);
			}
			ਸਤਰ = ਅਗਲਾ_ਸਤਰ;
		}
		ਜੇ (ਸਤਰ) {
			/* coreutils adds \n even if this is a final line
			 * of the last file and it was not \n-terminated.
			 */
			ਲਿਖੋ("%s\n", ਸਤਰ);
			ਮੁਕਤ(ਸਤਰ);
		}
		fclose_if_not_stdin(files[i]);
	}
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PASTE_OPT_DELIMITERS (1 << 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PASTE_OPT_SEPARATE   (1 << 1)

ਪੂਰਨ paste_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ paste_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
	ਅੱਖਰ *delims = (ਅੱਖਰ*)"\t";
	ਪੂਰਨ del_cnt = 1;
	ਬਿਨਾਨਿਸ਼ਾਨੀ opt;
	ਪੂਰਨ i;

	opt = getopt32(argv, "d:s", &delims);
	argv += optind;

	ਜੇ (opt & PASTE_OPT_DELIMITERS) {
		ਜੇ (!delims[0])
			bb_simple_error_ਸੁਨੇਹਾ_and_die("-d '' is not supported");
		/* unknown mappings are not changed: "\z" -> '\\' 'z' */
		/* trailing backslash, if any, is preserved */
		del_cnt = strcpy_and_process_escape_sequences(delims, delims) - delims;
		/* note: handle NUL properly (do not stop at it!): try -d'\t\0\t' */
	}

	ਜੇ (!argv[0])
		(--argv)[0] = (ਅੱਖਰ*) "-";
	ਲਈ (i = 0; argv[i]; ++i) {
		argv[i] = (ਖਾਲੀ*) fopen_or_warn_stdin(argv[i]);
		ਜੇ (!argv[i])
			xfunc_die();
	}

	ਜੇ (opt & PASTE_OPT_SEPARATE)
		paste_files_separate((FILE**)argv, delims, del_cnt);
	ਨਹੀਂਤਾਂ
		paste_files((FILE**)argv, i, delims, del_cnt);

	fflush_stdout_and_exit(0);
}
