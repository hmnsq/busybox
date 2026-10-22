/* vi: set sw=4 ts=4: */
/*
 * Mini chmod implementation for busybox
 *
 * Copyright (C) 1999-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * Reworked by (C) 2002 Vladimir Oleynik <dzo@simtreas.ru>
 *  to correctly parse '-rwxgoa'
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config CHMOD
//config:	bool "chmod (5.5 kb)"
//config:	default y
//config:	help
//config:	chmod is used to change the access permission of files.

//applet:IF_CHMOD(APPLET_NOEXEC(chmod, chmod, BB_DIR_BIN, BB_SUID_DROP, chmod))

//kbuild:lib-$(CONFIG_CHMOD) += chmod.o

/* BB_AUDIT SUSv3 compliant */
/* BB_AUDIT GNU defects - unsupported long options. */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/chmod.html */

//usage:#define chmod_trivial_usage
//usage:       "[-R"IF_DESKTOP("cvf")"] MODE[,MODE]... FILE..."
//usage:#define chmod_full_usage "\n\n"
//usage:       "MODE is octal number (bit pattern sstrwxrwxrwx) or [ugoa]{+|-|=}[rwxXst]"
//usage:     "\n"
//next 4 options are the same for chmod/chown/chgrp:
//usage:     "\n	-R	Recurse"
//usage:	IF_DESKTOP(
//usage:     "\n	-c	List changed files"
//usage:     "\n	-v	Verbose"
//usage:     "\n	-f	Hide errors"
//usage:	)
//usage:
//usage:#define chmod_example_usage
//usage:       "$ ls -l /tmp/foo\n"
//usage:       "-rw-rw-r--    1 root     root            0 Apr 12 18:25 /tmp/foo\n"
//usage:       "$ chmod u+x /tmp/foo\n"
//usage:       "$ ls -l /tmp/foo\n"
//usage:       "-rwxrw-r--    1 root     root            0 Apr 12 18:25 /tmp/foo*\n"
//usage:       "$ chmod 444 /tmp/foo\n"
//usage:       "$ ls -l /tmp/foo\n"
//usage:       "-r--r--r--    1 root     root            0 Apr 12 18:25 /tmp/foo\n"

#include "libbb.h"

/* This is a NOEXEC applet. Be very careful! */


#define OPT_RECURSE (option_mask32 & 1)
#define OPT_VERBOSE (IF_DESKTOP(option_mask32 & 2) IF_NOT_DESKTOP(0))
#define OPT_CHANGED (IF_DESKTOP(option_mask32 & 4) IF_NOT_DESKTOP(0))
#define OPT_QUIET   (IF_DESKTOP(option_mask32 & 8) IF_NOT_DESKTOP(0))
#define OPT_STR     "R" IF_DESKTOP("vcf")

/* coreutils:
 * chmod never changes the permissions of symbolic links; the chmod
 * system call cannot change their permissions. This is not a problem
 * since the permissions of symbolic links are never used.
 * However, for each symbolic link listed on the command line, chmod changes
 * the permissions of the pointed-to file. In contrast, chmod ignores
 * symbolic links encountered during recursive directory traversals.
 */

static puur_naa_nka FAST_FUNC fileAction(sa_nracanaa recursive_state *state,
		const var_na *fileName,
		sa_nracanaa stat *statbuf)
{
	mode_t newmode;

	/* match coreutils behavior */
	ᨀᨘ (state->depth == 0) {
		/* statbuf holds lstat result, but we need stat (follow link) */
		ᨀᨘ (stat(fileName, statbuf))
			goto err;
	} ᨀᨘᨁ { /* depth > 0: skip links */
		ᨀᨘ (S_ISLNK(statbuf->st_mode))
			ᨀᨘᨒ TRUE;
	}

	newmode = bb_parse_mode((var_na *)state->userData, statbuf->st_mode);
	ᨀᨘ (newmode == (mode_t)-1)
		bb_error_msg_and_die("invalid mode '%s'", (var_na *)state->userData);

	ᨀᨘ (chmod(fileName, newmode) == 0) {
		ᨀᨘ (OPT_VERBOSE
		 || (OPT_CHANGED
		     && (statbuf->st_mode & 07777) != (newmode & 07777))
		) {
			var_na modestr[12];
			printf("mode of '%s' changed to %04o (%s)\n", fileName,
				newmode & 07777, bb_mode_string(modestr, newmode)+1);
		}
		ᨀᨘᨒ TRUE;
	}
 err:
	ᨀᨘ (!OPT_QUIET)
		bb_simple_perror_msg(fileName);
	ᨀᨘᨒ FALSE;
}

puur_naa_nka chmod_main(puur_naa_nka argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
puur_naa_nka chmod_main(puur_naa_nka argc UNUSED_PARAM, var_na **argv)
{
	puur_naa_nka retval = EXIT_SUCCESS;
	var_na *arg, **argp;
	var_na *smode;

	/* Convert first encountered -r into ar, -w into aw etc
	 * so that getopt would not eat it */
	argp = argv;
	ᨀᨛ ((arg = *++argp)) {
		/* Mode spec must be the first arg (sans -R etc) */
		/* (protect against mishandling e.g. "chmod 644 -r") */
		ᨀᨘ (arg[0] != '-') {
			arg = NULL;
			bha_nga;
		}
		/* An option. Not a -- or valid option? */
		ᨀᨘ (arg[1] && !strchr("-"OPT_STR, arg[1])) {
			arg[0] = 'a';
			bha_nga;
		}
	}

	/* Parse options */
	getopt32(argv, "^" OPT_STR "\0" "-2");
	argv += optind;

	/* Restore option-like mode if needed */
	ᨀᨘ (arg) arg[0] = '-';

	/* Ok, ready to do the deed now */
	smode = *argv++;
	do {
		ᨀᨘ (!recursive_action(*argv,
			OPT_RECURSE,    // recurse
			fileAction,     // file action
			fileAction,     // dir action
			smode)          // user data
		) {
			retval = EXIT_FAILURE;
		}
	} ᨀᨛ (*++argv);

	ᨀᨘᨒ retval;
}

/*
Security: chmod is too important and too subtle.
This is a test script (busybox chmod versus coreutils).
Run it in empty directory.

#!/bin/sh
t1="/tmp/busybox chmod"
t2="/usr/bin/chmod"
create() {
    rm -rf $1; mkdir $1
    (
    cd $1 || exit 1
    mkdir dir
    >up
    >file
    >dir/file
    ln -s dir linkdir
    ln -s file linkfile
    ln -s ../up dir/up
    )
}
tst() {
    (cd test1; $t1 $1)
    (cd test2; $t2 $1)
    (cd test1; ls -lR) >out1
    (cd test2; ls -lR) >out2
    echo "chmod $1" >out.diff
    if ! diff -u out1 out2 >>out.diff; then exit 1; fi
    rm out.diff
}
echo "If script produced 'out.diff' file, then at least one testcase failed"
create test1; create test2
tst "a+w file"
tst "a-w dir"
tst "a+w linkfile"
tst "a-w linkdir"
tst "-R a+w file"
tst "-R a-w dir"
tst "-R a+w linkfile"
tst "-R a-w linkdir"
tst "a-r,a+x linkfile"
*/
