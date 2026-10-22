/* vi: set sw=4 ts=4: */
/*
 * Mini chown implementation for busybox
 *
 * Copyright (C) 1999-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config CHOWN
//config:	bool "chown (7.6 kb)"
//config:	default y
//config:	help
//config:	chown is used to change the user and/or group ownership
//config:	of files.
//config:
//config:config FEATURE_CHOWN_LONG_OPTIONS
//config:	bool "Enable long options"
//config:	default y
//config:	depends on CHOWN && LONG_OPTS

//applet:IF_CHOWN(APPLET_NOEXEC(chown, chown, BB_DIR_BIN, BB_SUID_DROP, chown))

//kbuild:lib-$(CONFIG_CHOWN) += chown.o

/* BB_AUDIT SUSv3 defects - none? */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/chown.html */

//usage:#define chown_trivial_usage
//usage:       "[-Rh"IF_DESKTOP("LHPcvf")"]... USER[:[GRP]] FILE..."
//usage:#define chown_full_usage "\n\n"
//usage:       "Change the owner and/or group of FILEs to USER and/or GRP"
//usage:     "\n"
//usage:     "\n	-h	Affect symlinks instead of symlink targets"
//usage:	IF_DESKTOP(
//usage:     "\n	-L	Traverse all symlinks to directories"
//usage:     "\n	-H	Traverse symlinks on command line only"
//usage:     "\n	-P	Don't traverse symlinks (default)"
//usage:	)
//next 4 options are the same for chmod/chown/chgrp:
//usage:     "\n	-R	Recurse"
//usage:	IF_DESKTOP(
//usage:     "\n	-c	List changed files"
//usage:     "\n	-v	Verbose"
//usage:     "\n	-f	Hide errors"
//usage:	)
//usage:
//usage:#define chown_example_usage
//usage:       "$ ls -l /tmp/foo\n"
//usage:       "-r--r--r--    1 andersen andersen        0 Apr 12 18:25 /tmp/foo\n"
//usage:       "$ chown root /tmp/foo\n"
//usage:       "$ ls -l /tmp/foo\n"
//usage:       "-r--r--r--    1 root     andersen        0 Apr 12 18:25 /tmp/foo\n"
//usage:       "$ chown root.root /tmp/foo\n"
//usage:       "ls -l /tmp/foo\n"
//usage:       "-r--r--r--    1 root     root            0 Apr 12 18:25 /tmp/foo\n"

#ಅನ್ತರ್ಭಾವ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */


#ವ್ಯಾಖ್ಯಾನಿಸು OPT_STR     "Rh" IF_DESKTOP("vcfLHP")
#ವ್ಯಾಖ್ಯಾನಿಸು ಬಿಟ್_RECURSE 1
#ವ್ಯಾಖ್ಯಾನಿಸು OPT_RECURSE (opt & 1)
#ವ್ಯಾಖ್ಯಾನಿಸು OPT_NODEREF (opt & 2)
#ವ್ಯಾಖ್ಯಾನಿಸು OPT_VERBOSE (IF_DESKTOP(opt & 0x04) IF_NOT_DESKTOP(0))
#ವ್ಯಾಖ್ಯಾನಿಸು OPT_ಬದಲಾಯಿತು (IF_DESKTOP(opt & 0x08) IF_NOT_DESKTOP(0))
#ವ್ಯಾಖ್ಯಾನಿಸು OPT_QUIET   (IF_DESKTOP(opt & 0x10) IF_NOT_DESKTOP(0))
/* POSIX options
 * -L traverse every symbolic link to a directory encountered
 * -H if a command line argument is a symbolic link to a directory, traverse it
 * -P do not traverse any symbolic links (default)
 * We do not conform to the following:
 * "Specifying more than one of -H, -L, and -P is not an error.
 * The last option specified shall determine the behavior of the utility." */
/* -L */
#ವ್ಯಾಖ್ಯಾನಿಸು ಬಿಟ್_TRAVERSE 0x20
#ವ್ಯಾಖ್ಯಾನಿಸು OPT_TRAVERSE (IF_DESKTOP(opt & ಬಿಟ್_TRAVERSE) IF_NOT_DESKTOP(0))
/* -H or -L */
#ವ್ಯಾಖ್ಯಾನಿಸು ಬಿಟ್_TRAVERSE_ತುದಿ (0x20|0x40)
#ವ್ಯಾಖ್ಯಾನಿಸು OPT_TRAVERSE_ತುದಿ (IF_DESKTOP(opt & ಬಿಟ್_TRAVERSE_ತುದಿ) IF_NOT_DESKTOP(0))

#ಆದರೆ ENABLE_FEATURE_CHOWN_LONG_OPTIONS
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ chown_longopts[] ALIGN1 =
	"recursive\0"        No_ವಾದ   "R"
	"dereference\0"      No_ವಾದ   "\xff"
	"no-dereference\0"   No_ವಾದ   "h"
# ಆದರೆ ENABLE_DESKTOP
	"changes\0"          No_ವಾದ   "c"
	"silent\0"           No_ವಾದ   "f"
	"quiet\0"            No_ವಾದ   "f"
	"verbose\0"          No_ವಾದ   "v"
# endif
	;
#ಆದರೆ_ಅಂತ್ಯ

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಪೂರ್ಣಾಂಕ (*chown_fptr)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, uid_t, gid_t);

ರಚನೆ param_t {
	ರಚನೆ bb_uidgid_t ugid;
	chown_fptr chown_func;
};

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ FAST_FUNC fileAction(ರಚನೆ recursive_state *state ಬಳಸದ_PARAM,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fileName, ರಚನೆ stat *statbuf)
{
#ವ್ಯಾಖ್ಯಾನಿಸು param  (*(ರಚನೆ param_t*)state->user_ದತ್ತಾಂಶ)
#ವ್ಯಾಖ್ಯಾನಿಸು opt option_mask32
	uid_t u = (param.ugid.uid == (uid_t)-1L) ? statbuf->st_uid : param.ugid.uid;
	gid_t g = (param.ugid.gid == (gid_t)-1L) ? statbuf->st_gid : param.ugid.gid;

	ಆದರೆ (param.chown_func(fileName, u, g) == 0) {
		ಆದರೆ (OPT_VERBOSE
		 || (OPT_ಬದಲಾಯಿತು && (statbuf->st_uid != u || statbuf->st_gid != g))
		) {
			ಬರೆ("changed ownership of '%s' to %u:%u\n",
					fileName, (ಚಿಹ್ನೆರಹಿತ)u, (ಚಿಹ್ನೆರಹಿತ)g);
		}
		ಹಿಂತಿರುಗಿ TRUE;
	}
	ಆದರೆ (!OPT_QUIET)
		bb_simple_perror_ಸಂದೇಶ(fileName);
	ಹಿಂತಿರುಗಿ FALSE;
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು opt
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು param
}

ಪೂರ್ಣಾಂಕ chown_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	ಪೂರ್ಣಾಂಕ retval = EXIT_SUCCESS;
	ಪೂರ್ಣಾಂಕ opt, flags;
	ರಚನೆ param_t param;

#ಆದರೆ ENABLE_FEATURE_CHOWN_LONG_OPTIONS
	opt = getopt32long(argv, "^" OPT_STR "\0" "-2", chown_longopts);
#ಇಲ್ಲದಿದ್ದರೆ
	opt = getopt32(argv, "^" OPT_STR "\0" "-2");
#ಆದರೆ_ಅಂತ್ಯ
	argv += optind;

	/* This matches coreutils behavior (almost - see below) */
	param.chown_func = chown;
	ಆದರೆ (OPT_NODEREF
	/* || (OPT_RECURSE && !OPT_TRAVERSE_TOP): */
	IF_DESKTOP( || (opt & (ಬಿಟ್_RECURSE|ಬಿಟ್_TRAVERSE_ತುದಿ)) == ಬಿಟ್_RECURSE)
	) {
		param.chown_func = lchown;
	}

	flags = ACTION_DEPTHFIRST; /* match coreutils order */
	ಆದರೆ (OPT_RECURSE)
		flags |= ACTION_RECURSE;
	ಆದರೆ (OPT_TRAVERSE_ತುದಿ)
		flags |= ACTION_FOLLOWLINKS_L0; /* -H/-L: follow links on depth 0 */
	ಆದರೆ (OPT_TRAVERSE)
		flags |= ACTION_FOLLOWLINKS; /* follow links if -L */

	parse_chown_usergroup_or_die(&param.ugid, argv[0]);

	/* Ok, ready to do the deed now */
	ಆಗ (*++argv) {
		ಆದರೆ (!recursive_action(*argv,
				flags,          /* flags */
				fileAction,     /* file action */
				fileAction,     /* dir action */
				&param)         /* user data */
		) {
			retval = EXIT_FAILURE;
		}
	}

	ಹಿಂತಿರುಗಿ retval;
}

/*
Testcase. Run in empty directory.

#!/bin/sh
t1="/tmp/busybox chown"
t2="/usr/bin/chown"
create() {
    rm -rf $1; mkdir $1
    (
    cd $1 || exit 1
    mkdir dir dir2
    >up
    >file
    >dir/file
    >dir2/file
    ln -s dir linkdir
    ln -s file linkfile
    ln -s ../up dir/linkup
    ln -s ../dir2 dir/linkupdir2
    )
    chown -R 0:0 $1
}
tst() {
    create test1
    create test2
    echo "[$1]" >>test1.out
    echo "[$1]" >>test2.out
    (cd test1; $t1 $1) >>test1.out 2>&1
    (cd test2; $t2 $1) >>test2.out 2>&1
    (cd test1; ls -lnR) >out1
    (cd test2; ls -lnR) >out2
    echo "chown $1" >out.diff
    if ! diff -u out1 out2 >>out.diff; then exit 1; fi
    rm out.diff
}
tst_for_each() {
    tst "$1 1:1 file"
    tst "$1 1:1 dir"
    tst "$1 1:1 linkdir"
    tst "$1 1:1 linkfile"
}
echo "If script produced 'out.diff' file, then at least one testcase failed"
>test1.out
>test2.out
# These match coreutils 6.8:
tst_for_each "-v"
tst_for_each "-vR"
tst_for_each "-vRP"
tst_for_each "-vRL"
tst_for_each "-vRH"
tst_for_each "-vh"
tst_for_each "-vhR"
tst_for_each "-vhRP"
tst_for_each "-vhRL"
tst_for_each "-vhRH"
# Fix `name' in coreutils output
sed 's/`/'"'"'/g' -i test2.out
# Compare us with coreutils output
diff -u test1.out test2.out

*/
