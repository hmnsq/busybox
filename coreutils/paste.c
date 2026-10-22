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
	वर्ण *पङ्क्तिः;
	वर्ण delim;
	पूर्णाङ्क active_files = file_cnt;
	पूर्णाङ्क i;

	जाव (active_files > 0) {
		पूर्णाङ्क del_idx = 0;

		प्रति (i = 0; i < file_cnt; ++i) {
			जइ (files[i] == रिक्त)
				अनुवर्त;

			पङ्क्तिः = xmalloc_fgetline(files[i]);
			जइ (!पङ्क्तिः) {
				fclose_if_not_stdin(files[i]);
				files[i] = रिक्त;
				--active_files;
				अनुवर्त;
			}
			fputs_stdout(पङ्क्तिः);
			मुक्त(पङ्क्तिः);
			delim = '\n';
			जइ (i != file_cnt - 1) {
				delim = delims[del_idx++];
				जइ (del_idx == del_cnt)
					del_idx = 0;
			}
			जइ (delim != '\0')
				fputc(delim, stdout);
		}
	}
}

स्थिर शून्य paste_files_separate(FILE** files, वर्ण* delims, पूर्णाङ्क del_cnt)
{
	वर्ण *पङ्क्तिः, *next_पङ्क्तिः;
	वर्ण delim;
	पूर्णाङ्क i;

	प्रति (i = 0; files[i]; ++i) {
		पूर्णाङ्क del_idx = 0;

		पङ्क्तिः = रिक्त;
		जाव ((next_पङ्क्तिः = xmalloc_fgetline(files[i])) != रिक्त) {
			जइ (पङ्क्तिः) {
				fputs_stdout(पङ्क्तिः);
				मुक्त(पङ्क्तिः);
				delim = delims[del_idx++];
				जइ (del_idx == del_cnt)
					del_idx = 0;
				जइ (delim != '\0')
					fputc(delim, stdout);
			}
			पङ्क्तिः = next_पङ्क्तिः;
		}
		जइ (पङ्क्तिः) {
			/* coreutils adds \n even if this is a final line
			 * of the last file and it was not \n-terminated.
			 */
			printf("%s\n", पङ्क्तिः);
			मुक्त(पङ्क्तिः);
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

	जइ (opt & PASTE_OPT_DELIMITERS) {
		जइ (!delims[0])
			bb_simple_error_सन्देशः_and_die("-d '' is not supported");
		/* unknown mappings are not changed: "\z" -> '\\' 'z' */
		/* trailing backslash, if any, is preserved */
		del_cnt = strcpy_and_process_escape_sequences(delims, delims) - delims;
		/* note: handle NUL properly (do not stop at it!): try -d'\t\0\t' */
	}

	जइ (!argv[0])
		(--argv)[0] = (वर्ण*) "-";
	प्रति (i = 0; argv[i]; ++i) {
		argv[i] = (शून्य*) fopen_or_warn_stdin(argv[i]);
		जइ (!argv[i])
			xfunc_die();
	}

	जइ (opt & PASTE_OPT_SEPARATE)
		paste_files_separate((FILE**)argv, delims, del_cnt);
	अण्णहा
		paste_files((FILE**)argv, i, delims, del_cnt);

	fflush_stdout_and_exit(0);
}
