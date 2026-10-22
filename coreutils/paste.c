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

स्थिर शून्य paste_files(खाता** files, पूर्णांक file_cnt, अक्षर* delims, पूर्णांक del_cnt)
{
	अक्षर *ओळ;
	अक्षर delim;
	पूर्णांक active_files = file_cnt;
	पूर्णांक i;

	पर्यंत (active_files > 0) {
		पूर्णांक del_idx = 0;

		साठी (i = 0; i < file_cnt; ++i) {
			जर (files[i] == रिक्त)
				सुरूठेवा;

			ओळ = xmalloc_fgetline(files[i]);
			जर (!ओळ) {
				fclose_if_not_stdin(files[i]);
				files[i] = रिक्त;
				--active_files;
				सुरूठेवा;
			}
			fputs_stdout(ओळ);
			मुक्त(ओळ);
			delim = '\n';
			जर (i != file_cnt - 1) {
				delim = delims[del_idx++];
				जर (del_idx == del_cnt)
					del_idx = 0;
			}
			जर (delim != '\0')
				ख_अक्षर_दो(delim, मानक_निकास);
		}
	}
}

स्थिर शून्य paste_files_separate(खाता** files, अक्षर* delims, पूर्णांक del_cnt)
{
	अक्षर *ओळ, *next_ओळ;
	अक्षर delim;
	पूर्णांक i;

	साठी (i = 0; files[i]; ++i) {
		पूर्णांक del_idx = 0;

		ओळ = रिक्त;
		पर्यंत ((next_ओळ = xmalloc_fgetline(files[i])) != रिक्त) {
			जर (ओळ) {
				fputs_stdout(ओळ);
				मुक्त(ओळ);
				delim = delims[del_idx++];
				जर (del_idx == del_cnt)
					del_idx = 0;
				जर (delim != '\0')
					ख_अक्षर_दो(delim, मानक_निकास);
			}
			ओळ = next_ओळ;
		}
		जर (ओळ) {
			/* coreutils adds \n even if this is a final line
			 * of the last file and it was not \n-terminated.
			 */
			म_लिखो("%s\n", ओळ);
			मुक्त(ओळ);
		}
		fclose_if_not_stdin(files[i]);
	}
}

#define PASTE_OPT_DELIMITERS (1 << 0)
#define PASTE_OPT_SEPARATE   (1 << 1)

पूर्णांक paste_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक paste_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	अक्षर *delims = (अक्षर*)"\t";
	पूर्णांक del_cnt = 1;
	अचिन्हित opt;
	पूर्णांक i;

	opt = getopt32(argv, "d:s", &delims);
	argv += optind;

	जर (opt & PASTE_OPT_DELIMITERS) {
		जर (!delims[0])
			bb_simple_error_संदेश_and_die("-d '' is not supported");
		/* unknown mappings are not changed: "\z" -> '\\' 'z' */
		/* trailing backslash, if any, is preserved */
		del_cnt = strcpy_and_process_escape_sequences(delims, delims) - delims;
		/* note: handle NUL properly (do not stop at it!): try -d'\t\0\t' */
	}

	जर (!argv[0])
		(--argv)[0] = (अक्षर*) "-";
	साठी (i = 0; argv[i]; ++i) {
		argv[i] = (शून्य*) fopen_or_warn_stdin(argv[i]);
		जर (!argv[i])
			xfunc_die();
	}

	जर (opt & PASTE_OPT_SEPARATE)
		paste_files_separate((खाता**)argv, delims, del_cnt);
	अन्यथा
		paste_files((खाता**)argv, i, delims, del_cnt);

	fflush_stdout_and_exit(0);
}
