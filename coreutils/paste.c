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

#شامل "libbb.h"

ساکن خالی paste_files(FILE** files, صحیح file_cnt, حرف* delims, صحیح del_cnt)
{
	حرف *سطر;
	حرف delim;
	صحیح فعال_files = file_cnt;
	صحیح i;

	جب (فعال_files > 0) {
		صحیح del_idx = 0;

		برائے (i = 0; i < file_cnt; ++i) {
			اگر (files[i] == تہی)
				جاری;

			سطر = xmalloc_fgetline(files[i]);
			اگر (!سطر) {
				fclose_if_not_stdin(files[i]);
				files[i] = تہی;
				--فعال_files;
				جاری;
			}
			fputs_stdout(سطر);
			آزاد(سطر);
			delim = '\n';
			اگر (i != file_cnt - 1) {
				delim = delims[del_idx++];
				اگر (del_idx == del_cnt)
					del_idx = 0;
			}
			اگر (delim != '\0')
				fputc(delim, stdout);
		}
	}
}

ساکن خالی paste_files_separate(FILE** files, حرف* delims, صحیح del_cnt)
{
	حرف *سطر, *اگلا_سطر;
	حرف delim;
	صحیح i;

	برائے (i = 0; files[i]; ++i) {
		صحیح del_idx = 0;

		سطر = تہی;
		جب ((اگلا_سطر = xmalloc_fgetline(files[i])) != تہی) {
			اگر (سطر) {
				fputs_stdout(سطر);
				آزاد(سطر);
				delim = delims[del_idx++];
				اگر (del_idx == del_cnt)
					del_idx = 0;
				اگر (delim != '\0')
					fputc(delim, stdout);
			}
			سطر = اگلا_سطر;
		}
		اگر (سطر) {
			/* coreutils adds \n even if this is a final line
			 * of the last file and it was not \n-terminated.
			 */
			لکھو("%s\n", سطر);
			آزاد(سطر);
		}
		fclose_if_not_stdin(files[i]);
	}
}

#تعریف_کرو PASTE_OPT_DELIMITERS (1 << 0)
#تعریف_کرو PASTE_OPT_SEPARATE   (1 << 1)

صحیح paste_main(صحیح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح paste_main(صحیح argc غیر_مستعمل_PARAM, حرف **argv)
{
	حرف *delims = (حرف*)"\t";
	صحیح del_cnt = 1;
	غیرعلامتی opt;
	صحیح i;

	opt = getopt32(argv, "d:s", &delims);
	argv += optind;

	اگر (opt & PASTE_OPT_DELIMITERS) {
		اگر (!delims[0])
			bb_simple_error_پیغام_and_die("-d '' is not supported");
		/* unknown mappings are not changed: "\z" -> '\\' 'z' */
		/* trailing backslash, if any, is preserved */
		del_cnt = strcpy_and_process_escape_sequences(delims, delims) - delims;
		/* note: handle NUL properly (do not stop at it!): try -d'\t\0\t' */
	}

	اگر (!argv[0])
		(--argv)[0] = (حرف*) "-";
	برائے (i = 0; argv[i]; ++i) {
		argv[i] = (خالی*) fopen_or_warn_stdin(argv[i]);
		اگر (!argv[i])
			xfunc_die();
	}

	اگر (opt & PASTE_OPT_SEPARATE)
		paste_files_separate((FILE**)argv, delims, del_cnt);
	ورنہ
		paste_files((FILE**)argv, i, delims, del_cnt);

	fflush_stdout_and_exit(0);
}
