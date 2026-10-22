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

#अन्तर्भाव "libbb.h"

स्थिर शून्य paste_files(FILE** files, पूर्णाङ्क file_cnt, वर्ण* delims, पूर्णाङ्क del_cnt)
{
	वर्ण *पंक्ति;
	वर्ण delim;
	पूर्णाङ्क active_files = file_cnt;
	पूर्णाङ्क i;

	यावत् (active_files > 0) {
		पूर्णाङ्क del_idx = 0;

		प्रति (i = 0; i < file_cnt; ++i) {
			यदि (files[i] == रिक्त)
				अनुवर्त;

			पंक्ति = xmalloc_fgetline(files[i]);
			यदि (!पंक्ति) {
				fclose_if_not_stdin(files[i]);
				files[i] = रिक्त;
				--active_files;
				अनुवर्त;
			}
			fputs_stdout(पंक्ति);
			मुक्त(पंक्ति);
			delim = '\n';
			यदि (i != file_cnt - 1) {
				delim = delims[del_idx++];
				यदि (del_idx == del_cnt)
					del_idx = 0;
			}
			यदि (delim != '\0')
				fputc(delim, stdout);
		}
	}
}

स्थिर शून्य paste_files_separate(FILE** files, वर्ण* delims, पूर्णाङ्क del_cnt)
{
	वर्ण *पंक्ति, *next_पंक्ति;
	वर्ण delim;
	पूर्णाङ्क i;

	प्रति (i = 0; files[i]; ++i) {
		पूर्णाङ्क del_idx = 0;

		पंक्ति = रिक्त;
		यावत् ((next_पंक्ति = xmalloc_fgetline(files[i])) != रिक्त) {
			यदि (पंक्ति) {
				fputs_stdout(पंक्ति);
				मुक्त(पंक्ति);
				delim = delims[del_idx++];
				यदि (del_idx == del_cnt)
					del_idx = 0;
				यदि (delim != '\0')
					fputc(delim, stdout);
			}
			पंक्ति = next_पंक्ति;
		}
		यदि (पंक्ति) {
			/* coreutils adds \n even if this is a final line
			 * of the last file and it was not \n-terminated.
			 */
			printf("%s\n", पंक्ति);
			मुक्त(पंक्ति);
		}
		fclose_if_not_stdin(files[i]);
	}
}

#define PASTE_OPT_DELIMITERS (1 << 0)
#define PASTE_OPT_SEPARATE   (1 << 1)

पूर्णाङ्क paste_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क paste_main(पूर्णाङ्क argc UNUSED_PARAM, वर्ण **argv)
{
	वर्ण *delims = (वर्ण*)"\t";
	पूर्णाङ्क del_cnt = 1;
	unsigned opt;
	पूर्णाङ्क i;

	opt = getopt32(argv, "d:s", &delims);
	argv += optind;

	यदि (opt & PASTE_OPT_DELIMITERS) {
		यदि (!delims[0])
			bb_simple_error_संदेश_and_die("-d '' is not supported");
		/* unknown mappings are not changed: "\z" -> '\\' 'z' */
		/* trailing backslash, if any, is preserved */
		del_cnt = strcpy_and_process_escape_sequences(delims, delims) - delims;
		/* note: handle NUL properly (do not stop at it!): try -d'\t\0\t' */
	}

	यदि (!argv[0])
		(--argv)[0] = (वर्ण*) "-";
	प्रति (i = 0; argv[i]; ++i) {
		argv[i] = (शून्य*) fopen_or_warn_stdin(argv[i]);
		यदि (!argv[i])
			xfunc_die();
	}

	यदि (opt & PASTE_OPT_SEPARATE)
		paste_files_separate((FILE**)argv, delims, del_cnt);
	अन्यथा
		paste_files((FILE**)argv, i, delims, del_cnt);

	fflush_stdout_and_exit(0);
}
