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

#અન્તર્ભાવ "libbb.h"

સ્થિર શૂન્ય paste_files(FILE** files, પૂર્ણાંક file_cnt, અક્ષર* delims, પૂર્ણાંક del_cnt)
{
	અક્ષર *લીટી;
	અક્ષર delim;
	પૂર્ણાંક active_files = file_cnt;
	પૂર્ણાંક i;

	જ્યારે (active_files > 0) {
		પૂર્ણાંક del_idx = 0;

		માટે (i = 0; i < file_cnt; ++i) {
			જો (files[i] == ખાલી)
				ચાલુ;

			લીટી = xmalloc_fgetline(files[i]);
			જો (!લીટી) {
				fclose_if_not_stdin(files[i]);
				files[i] = ખાલી;
				--active_files;
				ચાલુ;
			}
			fputs_stdout(લીટી);
			મુક્ત(લીટી);
			delim = '\n';
			જો (i != file_cnt - 1) {
				delim = delims[del_idx++];
				જો (del_idx == del_cnt)
					del_idx = 0;
			}
			જો (delim != '\0')
				fputc(delim, stdout);
		}
	}
}

સ્થિર શૂન્ય paste_files_separate(FILE** files, અક્ષર* delims, પૂર્ણાંક del_cnt)
{
	અક્ષર *લીટી, *next_લીટી;
	અક્ષર delim;
	પૂર્ણાંક i;

	માટે (i = 0; files[i]; ++i) {
		પૂર્ણાંક del_idx = 0;

		લીટી = ખાલી;
		જ્યારે ((next_લીટી = xmalloc_fgetline(files[i])) != ખાલી) {
			જો (લીટી) {
				fputs_stdout(લીટી);
				મુક્ત(લીટી);
				delim = delims[del_idx++];
				જો (del_idx == del_cnt)
					del_idx = 0;
				જો (delim != '\0')
					fputc(delim, stdout);
			}
			લીટી = next_લીટી;
		}
		જો (લીટી) {
			/* coreutils adds \n even if this is a final line
			 * of the last file and it was not \n-terminated.
			 */
			printf("%s\n", લીટી);
			મુક્ત(લીટી);
		}
		fclose_if_not_stdin(files[i]);
	}
}

#define PASTE_OPT_DELIMITERS (1 << 0)
#define PASTE_OPT_SEPARATE   (1 << 1)

પૂર્ણાંક paste_main(પૂર્ણાંક argc, અક્ષર **argv) MAIN_EXTERNALLY_VISIBLE;
પૂર્ણાંક paste_main(પૂર્ણાંક argc UNUSED_PARAM, અક્ષર **argv)
{
	અક્ષર *delims = (અક્ષર*)"\t";
	પૂર્ણાંક del_cnt = 1;
	unsigned opt;
	પૂર્ણાંક i;

	opt = getopt32(argv, "d:s", &delims);
	argv += optind;

	જો (opt & PASTE_OPT_DELIMITERS) {
		જો (!delims[0])
			bb_simple_error_સંદેશ_and_die("-d '' is not supported");
		/* unknown mappings are not changed: "\z" -> '\\' 'z' */
		/* trailing backslash, if any, is preserved */
		del_cnt = strcpy_and_process_escape_sequences(delims, delims) - delims;
		/* note: handle NUL properly (do not stop at it!): try -d'\t\0\t' */
	}

	જો (!argv[0])
		(--argv)[0] = (અક્ષર*) "-";
	માટે (i = 0; argv[i]; ++i) {
		argv[i] = (શૂન્ય*) fopen_or_warn_stdin(argv[i]);
		જો (!argv[i])
			xfunc_die();
	}

	જો (opt & PASTE_OPT_SEPARATE)
		paste_files_separate((FILE**)argv, delims, del_cnt);
	નહીંતર
		paste_files((FILE**)argv, i, delims, del_cnt);

	fflush_stdout_and_exit(0);
}
