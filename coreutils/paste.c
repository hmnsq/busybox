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

#অন্তর্ভাব "libbb.h"

স্থির শূন্য paste_files(FILE** files, পূর্ণসংখ্যা file_cnt, অক্ষর* delims, পূর্ণসংখ্যা del_cnt)
{
	অক্ষর *সারি;
	অক্ষর delim;
	পূর্ণসংখ্যা active_files = file_cnt;
	পূর্ণসংখ্যা i;

	যখন (active_files > 0) {
		পূর্ণসংখ্যা del_idx = 0;

		জন্য (i = 0; i < file_cnt; ++i) {
			যদি (files[i] == ফাঁকা)
				চালু;

			সারি = xmalloc_fgetline(files[i]);
			যদি (!সারি) {
				fclose_if_not_stdin(files[i]);
				files[i] = ফাঁকা;
				--active_files;
				চালু;
			}
			fputs_stdout(সারি);
			মুক্ত(সারি);
			delim = '\n';
			যদি (i != file_cnt - 1) {
				delim = delims[del_idx++];
				যদি (del_idx == del_cnt)
					del_idx = 0;
			}
			যদি (delim != '\0')
				fputc(delim, stdout);
		}
	}
}

স্থির শূন্য paste_files_separate(FILE** files, অক্ষর* delims, পূর্ণসংখ্যা del_cnt)
{
	অক্ষর *সারি, *পরবর্তী_সারি;
	অক্ষর delim;
	পূর্ণসংখ্যা i;

	জন্য (i = 0; files[i]; ++i) {
		পূর্ণসংখ্যা del_idx = 0;

		সারি = ফাঁকা;
		যখন ((পরবর্তী_সারি = xmalloc_fgetline(files[i])) != ফাঁকা) {
			যদি (সারি) {
				fputs_stdout(সারি);
				মুক্ত(সারি);
				delim = delims[del_idx++];
				যদি (del_idx == del_cnt)
					del_idx = 0;
				যদি (delim != '\0')
					fputc(delim, stdout);
			}
			সারি = পরবর্তী_সারি;
		}
		যদি (সারি) {
			/* coreutils adds \n even if this is a final line
			 * of the last file and it was not \n-terminated.
			 */
			printf("%s\n", সারি);
			মুক্ত(সারি);
		}
		fclose_if_not_stdin(files[i]);
	}
}

#define PASTE_OPT_DELIMITERS (1 << 0)
#define PASTE_OPT_SEPARATE   (1 << 1)

পূর্ণসংখ্যা paste_main(পূর্ণসংখ্যা argc, অক্ষর **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণসংখ্যা paste_main(পূর্ণসংখ্যা argc UNUSED_PARAM, অক্ষর **argv)
{
	অক্ষর *delims = (অক্ষর*)"\t";
	পূর্ণসংখ্যা del_cnt = 1;
	unsigned opt;
	পূর্ণসংখ্যা i;

	opt = getopt32(argv, "d:s", &delims);
	argv += optind;

	যদি (opt & PASTE_OPT_DELIMITERS) {
		যদি (!delims[0])
			bb_simple_error_বার্তা_and_die("-d '' is not supported");
		/* unknown mappings are not changed: "\z" -> '\\' 'z' */
		/* trailing backslash, if any, is preserved */
		del_cnt = strcpy_and_process_escape_sequences(delims, delims) - delims;
		/* note: handle NUL properly (do not stop at it!): try -d'\t\0\t' */
	}

	যদি (!argv[0])
		(--argv)[0] = (অক্ষর*) "-";
	জন্য (i = 0; argv[i]; ++i) {
		argv[i] = (শূন্য*) fopen_or_warn_stdin(argv[i]);
		যদি (!argv[i])
			xfunc_die();
	}

	যদি (opt & PASTE_OPT_SEPARATE)
		paste_files_separate((FILE**)argv, delims, del_cnt);
	অন্যথা
		paste_files((FILE**)argv, i, delims, del_cnt);

	fflush_stdout_and_exit(0);
}
