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

#ضمن "libbb.h"

ساكن فراغ paste_files(FILE** files, صحيح file_cnt, حرف* delims, صحيح del_cnt)
{
	حرف *سطر;
	حرف delim;
	صحيح فعال_files = file_cnt;
	صحيح i;

	طالما (فعال_files > 0) {
		صحيح del_idx = 0;

		لكل (i = 0; i < file_cnt; ++i) {
			إذا (files[i] == عدم)
				استمر;

			سطر = xmalloc_fgetline(files[i]);
			إذا (!سطر) {
				fclose_if_not_stdin(files[i]);
				files[i] = عدم;
				--فعال_files;
				استمر;
			}
			fputs_stdout(سطر);
			حرر(سطر);
			delim = '\n';
			إذا (i != file_cnt - 1) {
				delim = delims[del_idx++];
				إذا (del_idx == del_cnt)
					del_idx = 0;
			}
			إذا (delim != '\0')
				fputc(delim, stdout);
		}
	}
}

ساكن فراغ paste_files_separate(FILE** files, حرف* delims, صحيح del_cnt)
{
	حرف *سطر, *بعدی_سطر;
	حرف delim;
	صحيح i;

	لكل (i = 0; files[i]; ++i) {
		صحيح del_idx = 0;

		سطر = عدم;
		طالما ((بعدی_سطر = xmalloc_fgetline(files[i])) != عدم) {
			إذا (سطر) {
				fputs_stdout(سطر);
				حرر(سطر);
				delim = delims[del_idx++];
				إذا (del_idx == del_cnt)
					del_idx = 0;
				إذا (delim != '\0')
					fputc(delim, stdout);
			}
			سطر = بعدی_سطر;
		}
		إذا (سطر) {
			/* coreutils adds \n even if this is a final line
			 * of the last file and it was not \n-terminated.
			 */
			چاپ_کن("%s\n", سطر);
			حرر(سطر);
		}
		fclose_if_not_stdin(files[i]);
	}
}

#تعریف_کن PASTE_OPT_DELIMITERS (1 << 0)
#تعریف_کن PASTE_OPT_SEPARATE   (1 << 1)

صحيح paste_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح paste_main(صحيح argc استفادهنشده_PARAM, حرف **argv)
{
	حرف *delims = (حرف*)"\t";
	صحيح del_cnt = 1;
	بیعلامت opt;
	صحيح i;

	opt = getopt32(argv, "d:s", &delims);
	argv += optind;

	إذا (opt & PASTE_OPT_DELIMITERS) {
		إذا (!delims[0])
			bb_simple_error_پیام_and_die("-d '' is not supported");
		/* unknown mappings are not changed: "\z" -> '\\' 'z' */
		/* trailing backslash, if any, is preserved */
		del_cnt = strcpy_and_process_escape_sequences(delims, delims) - delims;
		/* note: handle NUL properly (do not stop at it!): try -d'\t\0\t' */
	}

	إذا (!argv[0])
		(--argv)[0] = (حرف*) "-";
	لكل (i = 0; argv[i]; ++i) {
		argv[i] = (فراغ*) fopen_or_warn_stdin(argv[i]);
		إذا (!argv[i])
			xfunc_die();
	}

	إذا (opt & PASTE_OPT_SEPARATE)
		paste_files_separate((FILE**)argv, delims, del_cnt);
	وإلا
		paste_files((FILE**)argv, i, delims, del_cnt);

	fflush_stdout_and_exit(0);
}
