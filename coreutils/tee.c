/* vi: set sw=4 ts=4: */
/*
 * tee implementation for busybox
 *
 * Copyright (C) 2003  Manuel Novoa III  <mjn3@codepoet.org>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config TEE
//config:	bool "tee (4.4 kb)"
//config:	default y
//config:	help
//config:	tee is used to read from standard input and write
//config:	to standard output and files.
//config:
//config:config FEATURE_TEE_USE_BLOCK_IO
//config:	bool "Enable block I/O (larger/faster) instead of byte I/O"
//config:	default y
//config:	depends on TEE
//config:	help
//config:	Enable this option for a faster tee, at expense of size.

//applet:IF_TEE(APPLET(tee, BB_DIR_USR_BIN, BB_SUID_DROP))

//kbuild:lib-$(CONFIG_TEE) += tee.o

/* BB_AUDIT SUSv3 compliant */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/tee.html */

//usage:#define tee_trivial_usage
//usage:       "[-ai] [FILE]..."
//usage:#define tee_full_usage "\n\n"
//usage:       "Copy stdin to each FILE, and also to stdout\n"
//usage:     "\n	-a	Append to the given FILEs, don't overwrite"
//usage:     "\n	-i	Ignore interrupt signals (SIGINT)"
//usage:
//usage:#define tee_example_usage
//usage:       "$ echo \"Hello\" | tee /tmp/foo\n"
//usage:       "$ cat /tmp/foo\n"
//usage:       "Hello\n"

// Bare "tee" with no below options does not install SIGPIPE handler - just dies on it.
// TODO:
//	--output-error[=MODE]
//		'warn'		diagnose errors writing to any output
//		'warn-nopipe'	diagnose errors writing to any output not a pipe
//		'exit'		exit on error writing to any output
//		'exit-nopipe'	exit on error writing to any output not a pipe
// ^^^ all of these should set SIGPIPE to SIG_IGN.
// Because "exit" mode should print error message and exit1(1) - not die on SIGPIPE.
// "exit-nopipe" does not exit on EPIPE and does not set exitcode to 1 too.
//	-p	diagnose errors writing to non pipes
// ^^^^ this should set SIGPIPE to SIG_IGN. EPIPE is ignored (same as "warn-nopipe")

#അന്തര്ഭാവ "libbb.h"
#അന്തര്ഭാവ "common_bufsiz.h"

പൂർണ്ണസംഖ്യ tee_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) MAIN_EXTERNALLY_VISIBLE;
പൂർണ്ണസംഖ്യ tee_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv)
{
	സ്ഥിരം അക്ഷരം *mode = "w\0a";
	FILE **files;
	FILE **fp;
	അക്ഷരം **names;
	അക്ഷരം **np;
	അക്ഷരം retval;
//TODO: make unconditional
#എങ്കിൽ ENABLE_FEATURE_TEE_USE_BLOCK_IO
	ssize_t c;
# define buf bb_common_bufsiz1
	setup_common_bufsiz();
#അല്ലെങ്കിൽ
	പൂർണ്ണസംഖ്യ c;
#endif
	retval = getopt32(argv, "ia");	/* 'a' must be 2nd */
	argc -= optind;
	argv += optind;

	mode += (retval & 2);	/* Since 'a' is the 2nd option... */

	എങ്കിൽ (retval & 1) {
		സൂചന(SIGINT, SIG_IGN);
	}
	retval = EXIT_SUCCESS;
	/* if (opt_p || opt_output_error)
		signal(SIGPIPE, SIG_IGN);
	 */

	/* Allocate an array of FILE *'s, with one extra for a sentinel. */
	fp = files = xzalloc(വലുപ്പം(FILE *) * (argc + 2));
	np = names = argv - 1;

	files[0] = stdout;
	പോകുക GOT_NEW_FILE;

	ചെയ്യുക {
		*fp = stdout;
		എങ്കിൽ (NOT_LONE_DASH(*argv)) {
			*fp = fopen_or_warn(*argv, mode);
			എങ്കിൽ (*fp == ശൂന്യ) {
				retval = EXIT_FAILURE;
				argv++;
				തുടരുക;
			}
		}
		*np = *argv++;
 GOT_NEW_FILE:
		setbuf(*fp, ശൂന്യ);	/* tee must not buffer output. */
		fp++;
		np++;
	} ആകുമ്പോൾ (*argv);
	/* names[0] will be filled later */

#എങ്കിൽ ENABLE_FEATURE_TEE_USE_BLOCK_IO
	ആകുമ്പോൾ ((c = safe_read(STDIN_FILENO, buf, COMMON_BUFSIZE)) > 0) {
		fp = files;
		ചെയ്യുക
			fwrite(buf, 1, c, *fp);
			/* if (opt_p && fwrite() != c && !EPIPE) bb_error_msg("..."); */
		ആകുമ്പോൾ (*++fp);
	}
	എങ്കിൽ (c < 0) {		/* Make sure read errors are signaled. */
		retval = EXIT_FAILURE;
	}
#അല്ലെങ്കിൽ
	setvbuf(stdout, ശൂന്യ, _IONBF, 0);
	ആകുമ്പോൾ ((c = getchar()) != EOF) {
		fp = files;
		ചെയ്യുക
			putc(c, *fp);
			/* if (opt_p && putc() == EOF && !EPIPE) bb_error_msg("..."); */
		ആകുമ്പോൾ (*++fp);
	}
#endif

	/* Now we need to check for i/o errors on stdin and the various
	 * output files.  Since we know that the first entry in the output
	 * file table is stdout, we can save one "if ferror" test by
	 * setting the first entry to stdin and checking stdout error
	 * status with fflush_stdout_and_exit()... although fflush()ing
	 * is unnecessary here. */
	np = names;
	fp = files;
	names[0] = (അക്ഷരം *) bb_സന്ദേശം_standard_നിവേശം;
	files[0] = stdin;
	ചെയ്യുക {	/* Now check for input and output errors. */
		/* Checking ferror should be sufficient, but we may want to fclose.
		 * If we do, remember not to close stdin! */
		die_if_ferror(*fp++, *np++);
	} ആകുമ്പോൾ (*fp);

	fflush_stdout_and_exit(retval);
}
