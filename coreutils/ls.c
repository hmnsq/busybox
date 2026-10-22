/* vi: set sw=4 ts=4: */
/*
 * Copyright (C) 1996 Brian Candler <B.Candler@pobox.com>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
/* [date unknown. Perhaps before year 2000]
 * To achieve a small memory footprint, this version of 'ls' doesn't do any
 * file sorting, and only has the most essential command line switches
 * (i.e., the ones I couldn't live without :-) All features which involve
 * linking in substantial chunks of libc can be disabled.
 *
 * Although I don't really want to add new features to this program to
 * keep it small, I *am* interested to receive bug fixes and ways to make
 * it more portable.
 *
 * KNOWN BUGS:
 * 1. hidden files can make column width too large
 *
 * NON-OPTIMAL BEHAVIOUR:
 * 1. autowidth reads directories twice
 * 2. if you do a short directory listing without filetype characters
 *    appended, there's no need to stat each one
 * PORTABILITY:
 * 1. requires lstat (BSD) - how do you do it without?
 *
 * [2009-03]
 * ls sorts listing now, and supports almost all options.
 */
//config:config LS
//config:	bool "ls (14 kb)"
//config:	default y
//config:	help
//config:	ls is used to list the contents of directories.
//config:
//config:config FEATURE_LS_FILETYPES
//config:	bool "Enable filetyping options (-p and -F)"
//config:	default y
//config:	depends on LS
//config:
//config:config FEATURE_LS_FOLLOWLINKS
//config:	bool "Enable symlinks dereferencing (-L)"
//config:	default y
//config:	depends on LS
//config:
//config:config FEATURE_LS_RECURSIVE
//config:	bool "Enable recursion (-R)"
//config:	default y
//config:	depends on LS
//config:
//config:config FEATURE_LS_WIDTH
//config:	bool "Enable -w WIDTH and window size autodetection"
//config:	default y
//config:	depends on LS
//config:
//config:config FEATURE_LS_SORTFILES
//config:	bool "Sort the file names"
//config:	default y
//config:	depends on LS
//config:	help
//config:	Allow ls to sort file names alphabetically.
//config:
//config:config FEATURE_LS_TIMESTAMPS
//config:	bool "Show file timestamps"
//config:	default y
//config:	depends on LS
//config:	help
//config:	Allow ls to display timestamps for files.
//config:
//config:config FEATURE_LS_USERNAME
//config:	bool "Show username/groupnames"
//config:	default y
//config:	depends on LS
//config:	help
//config:	Allow ls to display username/groupname for files.
//config:
//config:config FEATURE_LS_COLOR
//config:	bool "Allow use of color to identify file types"
//config:	default y
//config:	depends on LS && LONG_OPTS
//config:	help
//config:	This enables the --color option to ls.
//config:
//config:config FEATURE_LS_COLOR_IS_DEFAULT
//config:	bool "Produce colored ls output by default"
//config:	default y
//config:	depends on FEATURE_LS_COLOR
//config:	help
//config:	Saying yes here will turn coloring on by default,
//config:	even if no "--color" option is given to the ls command.
//config:	This is not recommended, since the colors are not
//config:	configurable, and the output may not be legible on
//config:	many output screens.

//applet:IF_LS(APPLET_NOEXEC(ls, ls, BB_DIR_BIN, BB_SUID_DROP, ls))

//kbuild:lib-$(CONFIG_LS) += ls.o

//usage:#define ls_trivial_usage
//usage:	"[-1AaCxd"
//usage:	IF_FEATURE_LS_FOLLOWLINKS("LH")
//usage:	IF_FEATURE_LS_RECURSIVE("R")
//usage:	IF_FEATURE_LS_FILETYPES("Fp") "lins"
//usage:	IF_FEATURE_HUMAN_READABLE("h")
//usage:	IF_FEATURE_LS_SORTFILES("rSXv")
//usage:	IF_FEATURE_LS_TIMESTAMPS("ctu")
//usage:	IF_SELINUX("kZ") "]"
//usage:	IF_FEATURE_LS_WIDTH(" [-w WIDTH]") " [FILE]..."
//usage:#define ls_full_usage "\n\n"
//usage:       "List directory contents\n"
//usage:     "\n	-1	One column output"
//usage:     "\n	-a	Include names starting with ."
//usage:     "\n	-A	Like -a, but exclude . and .."
////usage:     "\n	-C	List by columns" - don't show, this is a default anyway
//usage:     "\n	-x	List by lines"
//usage:     "\n	-d	List directory names, not contents"
//usage:	IF_FEATURE_LS_FOLLOWLINKS(
//usage:     "\n	-L	Follow symlinks"
//usage:     "\n	-H	Follow symlinks on command line"
//usage:	)
//usage:	IF_FEATURE_LS_RECURSIVE(
//usage:     "\n	-R	Recurse"
//usage:	)
//usage:	IF_FEATURE_LS_FILETYPES(
//usage:     "\n	-p	Append / to directory names"
//usage:     "\n	-F	Append indicator (one of */=@|) to names"
//usage:	)
//usage:     "\n	-l	Long format"
//usage:     "\n	-i	List inode numbers"
//usage:     "\n	-n	List numeric UIDs and GIDs instead of names"
//usage:     "\n	-s	List allocated blocks"
//usage:	IF_FEATURE_LS_TIMESTAMPS(
//usage:     "\n	-lc	List ctime"
//usage:     "\n	-lu	List atime"
//usage:	)
//usage:	IF_FEATURE_LS_TIMESTAMPS(IF_LONG_OPTS(
//usage:     "\n	--full-time	List full date/time"
//usage:	))
//usage:	IF_FEATURE_HUMAN_READABLE(
//usage:     "\n	-h	Human readable sizes (1K 243M 2G)"
//usage:	)
//usage:	IF_FEATURE_LS_SORTFILES(
//usage:	IF_LONG_OPTS(
//usage:     "\n	--group-directories-first"
//usage:	)
//usage:     "\n	-S	Sort by size"
//usage:     "\n	-X	Sort by extension"
//usage:     "\n	-v	Sort by version"
//usage:	)
//usage:	IF_FEATURE_LS_TIMESTAMPS(
//usage:     "\n	-t	Sort by mtime"
//usage:     "\n	-tc	Sort by ctime"
//usage:     "\n	-tu	Sort by atime"
//usage:	)
//usage:     "\n	-r	Reverse sort order"
//usage:	IF_SELINUX(
//usage:     "\n	-Z	List security context and permission"
//usage:	)
//usage:	IF_FEATURE_LS_WIDTH(
//usage:     "\n	-w N	Format N columns wide"
//usage:	)
//usage:	IF_FEATURE_LS_COLOR(
//usage:     "\n	--color[={always,never,auto}]"
//usage:	)

#ಅನ್ತರ್ಭಾವ "libbb.h"
#ಅನ್ತರ್ಭಾವ "common_bufsiz.h"
#ಅನ್ತರ್ಭಾವ "unicode.h"


/* This is a NOEXEC applet. Be very careful! */


#ಆದರೆ ENABLE_FTPD
/* ftpd uses ls, and without timestamps Mozilla won't understand
 * ftpd's LIST output.
 */
# undef CONFIG_FEATURE_LS_TIMESTAMPS
# undef ENABLE_FEATURE_LS_TIMESTAMPS
# undef IF_FEATURE_LS_TIMESTAMPS
# undef IF_NOT_FEATURE_LS_TIMESTAMPS
# define CONFIG_FEATURE_LS_TIMESTAMPS 1
# define ENABLE_FEATURE_LS_TIMESTAMPS 1
# define IF_FEATURE_LS_TIMESTAMPS(...) __VA_ARGS__
# define IF_NOT_FEATURE_LS_TIMESTAMPS(...)
#ಆದರೆ_ಅಂತ್ಯ


ಎಣಿಕೆ {
TERMINAL_ಅಗಲ  = 80, /* use 79 if terminal has linefold bug */

ವಿಭಜಿಸು_FILE      = 0,
ವಿಭಜಿಸು_DIR       = 1,
ವಿಭಜಿಸು_SUBDIR    = 2,
};

/* -Cadi1l  Std options, busybox always supports */
/* -gnsxA   Std options, busybox always supports */
/* -Q       GNU option, busybox always supports */
/* -k       Std option, busybox always supports (by ignoring) */
/*          It means "for -s, show sizes in kbytes" */
/*          Seems to only affect "POSIXLY_CORRECT=1 ls -sk" */
/*          since otherwise -s shows kbytes anyway */
/* -LHRctur Std options, busybox optionally supports */
/* -Fp      Std options, busybox optionally supports */
/* -SXvhTw  GNU options, busybox optionally supports */
/* -T WIDTH Ignored (we don't use tabs on output) */
/* -Z       SELinux mandated option, busybox optionally supports */
#ವ್ಯಾಖ್ಯಾನಿಸು ls_options \
	"Cadi1lgnsxAk"       /* 12 opts, total 12 */ \
	IF_FEATURE_LS_FILETYPES("Fp")    /* 2, 14 */ \
	IF_FEATURE_LS_RECURSIVE("R")     /* 1, 15 */ \
	IF_SELINUX("Z")                  /* 1, 16 */ \
	"Q"                              /* 1, 17 */ \
	IF_FEATURE_LS_TIMESTAMPS("ctu")  /* 3, 20 */ \
	IF_FEATURE_LS_SORTFILES("SXrv")  /* 4, 24 */ \
	IF_FEATURE_LS_FOLLOWLINKS("LH")  /* 2, 26 */ \
	IF_FEATURE_HUMAN_READABLE("h")   /* 1, 27 */ \
	IF_FEATURE_LS_ಅಗಲ("T:w:")      /* 2, 29 */

ಎಣಿಕೆ {
	OPT_C = (1 << 0),
	OPT_a = (1 << 1),
	OPT_d = (1 << 2),
	OPT_i = (1 << 3),
	OPT_1 = (1 << 4),
	OPT_l = (1 << 5),
	OPT_g = (1 << 6),
	OPT_n = (1 << 7),
	OPT_s = (1 << 8),
	OPT_x = (1 << 9),
	OPT_A = (1 << 10),
	//OPT_k = (1 << 11),

	OPTBIT_F = 12,
	OPTBIT_p, /* 13 */
	OPTBIT_R = OPTBIT_F + 2 * ENABLE_FEATURE_LS_FILETYPES,
	OPTBIT_Z = OPTBIT_R + 1 * ENABLE_FEATURE_LS_RECURSIVE,
	OPTBIT_Q = OPTBIT_Z + 1 * ENABLE_SELINUX,
	OPTBIT_c, /* 17 */
	OPTBIT_t, /* 18 */
	OPTBIT_u, /* 19 */
	OPTBIT_S = OPTBIT_c + 3 * ENABLE_FEATURE_LS_TIMESTAMPS,
	OPTBIT_X, /* 21 */
	OPTBIT_r, /* 22 */
	OPTBIT_v, /* 23 */
	OPTBIT_L = OPTBIT_S + 4 * ENABLE_FEATURE_LS_SORTFILES,
	OPTBIT_H, /* 25 */
	OPTBIT_h = OPTBIT_L + 2 * ENABLE_FEATURE_LS_FOLLOWLINKS,
	OPTBIT_T = OPTBIT_h + 1 * ENABLE_FEATURE_HUMAN_READABLE,
	OPTBIT_w, /* 28 */
	OPTBIT_ತುಂಬಿದ_ಸಮಯ = OPTBIT_T + 2 * ENABLE_FEATURE_LS_ಅಗಲ,
	OPTBIT_dirs_ಮೊದಲ,
	OPTBIT_color, /* 31 */
	/* with long opts, we use all 32 bits */

	OPT_F = (1 << OPTBIT_F) * ENABLE_FEATURE_LS_FILETYPES,
	OPT_p = (1 << OPTBIT_p) * ENABLE_FEATURE_LS_FILETYPES,
	OPT_R = (1 << OPTBIT_R) * ENABLE_FEATURE_LS_RECURSIVE,
	OPT_Z = (1 << OPTBIT_Z) * ENABLE_SELINUX,
	OPT_Q = (1 << OPTBIT_Q),
	OPT_c = (1 << OPTBIT_c) * ENABLE_FEATURE_LS_TIMESTAMPS,
	OPT_t = (1 << OPTBIT_t) * ENABLE_FEATURE_LS_TIMESTAMPS,
	OPT_u = (1 << OPTBIT_u) * ENABLE_FEATURE_LS_TIMESTAMPS,
	OPT_S = (1 << OPTBIT_S) * ENABLE_FEATURE_LS_SORTFILES,
	OPT_X = (1 << OPTBIT_X) * ENABLE_FEATURE_LS_SORTFILES,
	OPT_r = (1 << OPTBIT_r) * ENABLE_FEATURE_LS_SORTFILES,
	OPT_v = (1 << OPTBIT_v) * ENABLE_FEATURE_LS_SORTFILES,
	OPT_L = (1 << OPTBIT_L) * ENABLE_FEATURE_LS_FOLLOWLINKS,
	OPT_H = (1 << OPTBIT_H) * ENABLE_FEATURE_LS_FOLLOWLINKS,
	OPT_h = (1 << OPTBIT_h) * ENABLE_FEATURE_HUMAN_READABLE,
	OPT_T = (1 << OPTBIT_T) * ENABLE_FEATURE_LS_ಅಗಲ,
	OPT_w = (1 << OPTBIT_w) * ENABLE_FEATURE_LS_ಅಗಲ,
	OPT_ತುಂಬಿದ_ಸಮಯ  = (1 << OPTBIT_ತುಂಬಿದ_ಸಮಯ ) * ENABLE_LONG_OPTS,
	OPT_dirs_ಮೊದಲ = (1 << OPTBIT_dirs_ಮೊದಲ) * ENABLE_LONG_OPTS,
	OPT_color      = (1 << OPTBIT_color     ) * ENABLE_FEATURE_LS_COLOR,
};

/*
 * a directory entry and its stat info
 */
ರಚನೆ dnode {
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;       /* usually basename, but think "ls -l dir/file" */
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fullname;   /* full name (usable for stat etc) */
	ರಚನೆ dnode *dn_ಮುಂದಿನ;  /* for linked list */
	IF_SELINUX(security_context_t sid;)
	smallint fname_allocated;

	/* Used to avoid re-doing [l]stat at printout stage
	 * if we already collected needed data in scan stage:
	 */
	ಬಹುಲಕ_t    dn_ಬಹುಲಕ_lstat;   /* obtained with lstat, or 0 */
	ಬಹುಲಕ_t    dn_ಬಹುಲಕ_stat;    /* obtained with stat, or 0 */

//	struct stat dstat;
// struct stat is huge. We don't need it in full.
// At least we don't need st_dev and st_blksize,
// but there are invisible fields as well
// (such as nanosecond-resolution timespamps)
// and padding, which we also don't want to store.
// We also pre-parse dev_t dn_rdev (in glibc, it's huge).
// On 32-bit uclibc: dnode size went from 112 to 84 bytes.
//
	/* Same names as in struct stat, but with dn_ instead of st_ pfx: */
	ಬಹುಲಕ_t    dn_ಬಹುಲಕ; /* obtained with lstat OR stat, depending on -L etc */
	off_t     dn_ಗಾತ್ರ;
#ಆದರೆ ENABLE_FEATURE_LS_TIMESTAMPS || ENABLE_FEATURE_LS_SORTFILES
	ಸಮಯ_t    dn_ಸಮಯ;
#ಆದರೆ_ಅಂತ್ಯ
	ino_t     dn_ino;
	blkcnt_t  dn_ಖಂಡಗಳು;
	nlink_t   dn_nlink;
	uid_t     dn_uid;
	gid_t     dn_gid;
	ಪೂರ್ಣಾಂಕ       dn_rdev_maj;
	ಪೂರ್ಣಾಂಕ       dn_rdev_min;
//	dev_t     dn_dev;
//	blksize_t dn_blksize;
};

ರಚನೆ globals {
#ಆದರೆ ENABLE_FEATURE_LS_COLOR
	smallint show_color;
# define G_show_color (G.show_color)
#ಇಲ್ಲದಿದ್ದರೆ
# define G_show_color 0
#ಆದರೆ_ಅಂತ್ಯ
	smallint exit_ಸಂಕೇತ;
	smallint show_dirname;
#ಆದರೆ ENABLE_FEATURE_LS_ಅಗಲ
	ಚಿಹ್ನೆರಹಿತ terminal_ಅಗಲ;
# define G_terminal_ಅಗಲ (G.terminal_ಅಗಲ)
#ಇಲ್ಲದಿದ್ದರೆ
# define G_terminal_ಅಗಲ TERMINAL_ಅಗಲ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_FEATURE_LS_TIMESTAMPS
	/* Do time() just once. Saves one syscall per file for "ls -l" */
	ಸಮಯ_t current_ಸಮಯ_t;
#ಆದರೆ_ಅಂತ್ಯ
} FIX_ALIASING;
#ವ್ಯಾಖ್ಯಾನಿಸು G (*(ರಚನೆ globals*)bb_common_bufsiz1)
#ವ್ಯಾಖ್ಯಾನಿಸು INIT_G() ಮಾಡು { \
	ಸಿದ್ಧತೆ_common_bufsiz(); \
	/* we have to zero it out because of NOEXEC */ \
	memset(&G, 0, ಗಾತ್ರ(G)); \
	IF_FEATURE_LS_ಅಗಲ(G_terminal_ಅಗಲ = TERMINAL_ಅಗಲ;) \
	IF_FEATURE_LS_TIMESTAMPS(ಸಮಯ(&G.current_ಸಮಯ_t);) \
} ಆಗ (0)

#ವ್ಯಾಖ್ಯಾನಿಸು ESC "\033"


/*** Output code ***/


/* FYI type values: 1:fifo 2:char 4:dir 6:blk 8:file 10:link 12:socket
 * (various wacky OSes: 13:Sun door 14:BSD whiteout 5:XENIX named file
 *  3/7:multiplexed char/block device)
 * and we use 0 for unknown and 15 for executables (see below) */
#ವ್ಯಾಖ್ಯಾನಿಸು TYPEINDEX(ಬಹುಲಕ) (((ಬಹುಲಕ) >> 12) & 0x0f)
/*                       un  fi chr -   dir -  blk  -  file -  link - sock -   - exe */
#ವ್ಯಾಖ್ಯಾನಿಸು APPCHAR(ಬಹುಲಕ)   ("\0""|""\0""\0""/""\0""\0""\0""\0""\0""@""\0""=""\0""\0""\0" [TYPEINDEX(ಬಹುಲಕ)])
/* 036 black foreground              050 black background
   037 red foreground                051 red background
   040 green foreground              052 green background
   041 brown foreground              053 brown background
   042 blue foreground               054 blue background
   043 magenta (purple) foreground   055 magenta background
   044 cyan (light blue) foreground  056 cyan background
   045 gray foreground               057 white background
*/
#ವ್ಯಾಖ್ಯಾನಿಸು COLOR(ಬಹುಲಕ) ( \
	/*un  fi  chr  -  dir  -  blk  -  file -  link -  sock -   -  exe */ \
	"\037\043\043\045\042\045\043\043\000\045\044\045\043\045\045\040" \
	[TYPEINDEX(ಬಹುಲಕ)])
/* Select normal (0) [actually "reset all"] or bold (1)
 * (other attributes are 2:dim 4:underline 5:blink 7:reverse,
 *  let's use 7 for "impossible" types, just for fun)
 * Note: coreutils 6.9 uses inverted red for setuid binaries.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು ATTR(ಬಹುಲಕ) ( \
	/*un fi chr - dir - blk - file- link- sock- -  exe */ \
	"\01\00\01\07\01\07\01\07\00\07\01\07\01\07\07\01" \
	[TYPEINDEX(ಬಹುಲಕ)])

#ಆದರೆ ENABLE_FEATURE_LS_COLOR
/* mode of zero is interpreted as "unknown" (stat failed) */
ಸ್ಥಿರ ಅಕ್ಷರ fgcolor(ಬಹುಲಕ_t ಬಹುಲಕ)
{
	ಆದರೆ (S_ISREG(ಬಹುಲಕ) && (ಬಹುಲಕ & (S_IXUSR | S_IXGRP | S_IXOTH)))
		ಹಿಂತಿರುಗಿ COLOR(0xF000);	/* File is executable ... */
	ಹಿಂತಿರುಗಿ COLOR(ಬಹುಲಕ);
}
ಸ್ಥಿರ ಅಕ್ಷರ bold(ಬಹುಲಕ_t ಬಹುಲಕ)
{
	ಆದರೆ (S_ISREG(ಬಹುಲಕ) && (ಬಹುಲಕ & (S_IXUSR | S_IXGRP | S_IXOTH)))
		ಹಿಂತಿರುಗಿ ATTR(0xF000);	/* File is executable ... */
	ಹಿಂತಿರುಗಿ ATTR(ಬಹುಲಕ);
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_FEATURE_LS_FILETYPES
ಸ್ಥಿರ ಅಕ್ಷರ append_char(ಬಹುಲಕ_t ಬಹುಲಕ)
{
	ಆದರೆ (!(option_mask32 & (OPT_F|OPT_p)))
		ಹಿಂತಿರುಗಿ '\0';

	ಆದರೆ (S_ISDIR(ಬಹುಲಕ))
		ಹಿಂತಿರುಗಿ '/';
	ಆದರೆ (!(option_mask32 & OPT_F))
		ಹಿಂತಿರುಗಿ '\0';
	ಆದರೆ (S_ISREG(ಬಹುಲಕ) && (ಬಹುಲಕ & (S_IXUSR | S_IXGRP | S_IXOTH)))
		ಹಿಂತಿರುಗಿ '*';
	ಹಿಂತಿರುಗಿ APPCHAR(ಬಹುಲಕ);
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ calc_name_len(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ಚಿಹ್ನೆರಹಿತ len;
	uni_stat_t uni_stat;

	// TODO: quote tab as \t, etc, if -Q
	name = printable_string2(&uni_stat, name);

	ಆದರೆ (!(option_mask32 & OPT_Q)) {
		ಹಿಂತಿರುಗಿ uni_stat.unicode_ಅಗಲ;
	}

	len = 2 + uni_stat.unicode_ಅಗಲ;
	ಆಗ (*name) {
		ಆದರೆ (*name == '"' || *name == '\\') {
			len++;
		}
		name++;
	}
	ಹಿಂತಿರುಗಿ len;
}

/* Return the number of used columns.
 * Note that only columnar output uses return value.
 * -l and -1 modes don't care.
 * coreutils 7.2 also supports:
 * ls -b (--escape) = octal escapes (although it doesn't look like working)
 * ls -N (--literal) = not escape at all
 */
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ print_name(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ಚಿಹ್ನೆರಹಿತ len;
	uni_stat_t uni_stat;

	// TODO: quote tab as \t, etc, if -Q
	name = printable_string2(&uni_stat, name);

	ಆದರೆ (!(option_mask32 & OPT_Q)) {
		fputs_stdout(name);
		ಹಿಂತಿರುಗಿ uni_stat.unicode_ಅಗಲ;
	}

	len = 2 + uni_stat.unicode_ಅಗಲ;
	ಅಕ್ಷರ_ಬರೆ('"');
	ಆಗ (*name) {
		ಆದರೆ (*name == '"' || *name == '\\') {
			ಅಕ್ಷರ_ಬರೆ('\\');
			len++;
		}
		ಅಕ್ಷರ_ಬರೆ(*name);
		name++;
	}
	ಅಕ್ಷರ_ಬರೆ('"');
	ಹಿಂತಿರುಗಿ len;
}

/* Return the number of used columns.
 * Note that only columnar output uses return value,
 * -l and -1 modes don't care.
 */
ಸ್ಥಿರ NOINLINE ಚಿಹ್ನೆರಹಿತ display_single(ಸ್ಥಿರಾಂಕ ರಚನೆ dnode *dn)
{
	ಚಿಹ್ನೆರಹಿತ ಸ್ತಂಭ = 0;
	ಅಕ್ಷರ *lpath;
	ಪೂರ್ಣಾಂಕ opt;
#ಆದರೆ ENABLE_FEATURE_LS_FILETYPES || ENABLE_FEATURE_LS_COLOR
	ರಚನೆ stat statbuf;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_FEATURE_LS_FILETYPES
	ಅಕ್ಷರ append = append_char(dn->dn_ಬಹುಲಕ);
#ಆದರೆ_ಅಂತ್ಯ

	opt = option_mask32;

	/* Do readlink early, so that if it fails, error message
	 * does not appear *inside* the "ls -l" line */
	lpath = ಖಾಲಿ;
	ಆದರೆ (opt & OPT_l)
		ಆದರೆ (S_ISLNK(dn->dn_ಬಹುಲಕ))
			lpath = xmalloc_readlink_or_warn(dn->fullname);

	ಆದರೆ (opt & OPT_i) /* show inode# */
		ಸ್ತಂಭ += ಬರೆ("%7llu ", (ದೀರ್ಘ ದೀರ್ಘ) dn->dn_ino);
	ಆದರೆ (opt & OPT_s) { /* show allocated blocks */
		ಆದರೆ (opt & OPT_h) {
			ಸ್ತಂಭ += ಬರೆ("%"HUMAN_READABLE_MAX_ಅಗಲ_STR"s ",
				/* print size, show one fractional, use suffixes */
				make_human_readable_str((off_t)dn->dn_ಖಂಡಗಳು << 9, 1, 0)
			);
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಸ್ತಂಭ += ಬರೆ("%6"OFF_FMT"u ", (off_t)(dn->dn_ಖಂಡಗಳು >> 1));
		}
	}
	ಆದರೆ (opt & OPT_l) {
		/* long listing: show mode */
		ಅಕ್ಷರ modestr[12];
		ಸ್ತಂಭ += ಬರೆ("%-10s ", bb_ಬಹುಲಕ_string(modestr, dn->dn_ಬಹುಲಕ));
		/* long listing: show number of links */
		ಸ್ತಂಭ += ಬರೆ("%4lu ", (ದೀರ್ಘ) dn->dn_nlink);
		/* long listing: show user/group */
		ಆದರೆ (opt & OPT_n) {
			ಆದರೆ (opt & OPT_g)
				ಸ್ತಂಭ += ಬರೆ("%-8u ", (ಪೂರ್ಣಾಂಕ) dn->dn_gid);
			ಇಲ್ಲದಿದ್ದರೆ
				ಸ್ತಂಭ += ಬರೆ("%-8u %-8u ",
						(ಪೂರ್ಣಾಂಕ) dn->dn_uid,
						(ಪೂರ್ಣಾಂಕ) dn->dn_gid);
		}
#ಆದರೆ ENABLE_FEATURE_LS_USERNAME
		ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (opt & OPT_g) {
				ಸ್ತಂಭ += ಬರೆ("%-8s ",
					get_cached_groupname(dn->dn_gid));
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಸ್ತಂಭ += ಬರೆ("%-8s %-8s ",
					get_cached_username(dn->dn_uid),
					get_cached_groupname(dn->dn_gid));
			}
		}
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_SELINUX
	}
	ಆದರೆ (opt & OPT_Z) {
		ಸ್ತಂಭ += ಬರೆ("%-32s ", dn->sid ? dn->sid : "?");
		freecon(dn->sid);
	}
	ಆದರೆ (opt & OPT_l) {
#ಆದರೆ_ಅಂತ್ಯ
		/* long listing: show size */
		ಆದರೆ (S_ISBLK(dn->dn_ಬಹುಲಕ) || S_ISCHR(dn->dn_ಬಹುಲಕ)) {
			ಸ್ತಂಭ += ಬರೆ("%4u, %3u ",
					dn->dn_rdev_maj,
					dn->dn_rdev_min);
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (opt & OPT_h) {
				ಸ್ತಂಭ += ಬರೆ("%"HUMAN_READABLE_MAX_ಅಗಲ_STR"s ",
					/* print size, show one fractional, use suffixes */
					make_human_readable_str(dn->dn_ಗಾತ್ರ, 1, 0)
				);
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಸ್ತಂಭ += ಬರೆ("%9"OFF_FMT"u ", dn->dn_ಗಾತ್ರ);
			}
		}
#ಆದರೆ ENABLE_FEATURE_LS_TIMESTAMPS
		/* long listing: show {m,c,a}time */
		ಆದರೆ (opt & OPT_ತುಂಬಿದ_ಸಮಯ) { /* --full-time */
			/* coreutils 8.4 ls --full-time prints:
			 * 2009-07-13 17:49:27.000000000 +0200
			 * we don't show fractional seconds.
			 */
			ಅಕ್ಷರ buf[ಗಾತ್ರ("YYYY-mm-dd HH:MM:SS TIMEZONE")];
			strftime(buf, ಗಾತ್ರ(buf), "%Y-%m-%d %H:%M:%S %z",
					localtime(&dn->dn_ಸಮಯ));
			ಸ್ತಂಭ += ಬರೆ("%s ", buf);
		} ಇಲ್ಲದಿದ್ದರೆ { /* ordinary time format */
			/* G.current_time_t is ~== time(NULL) */
			ಅಕ್ಷರ *filetime = ctime(&dn->dn_ಸಮಯ);
			/* filetime's format: "Wed Jun 30 21:49:08 1993\n" */
			ಸಮಯ_t age = G.current_ಸಮಯ_t - dn->dn_ಸಮಯ;
			ಆದರೆ (age < 3600L * 24 * 365 / 2 && age > -15 * 60) {
				/* less than 6 months old */
				/* "mmm dd hh:mm " */
				ಬರೆ("%.12s ", filetime + 4);
			} ಇಲ್ಲದಿದ್ದರೆ {
				/* "mmm dd  yyyy " */
				/* "mmm dd yyyyy " after year 9999 :) */
				strchr(filetime + 20, '\n')[0] = ' ';
				ಬರೆ("%.7s%6s", filetime + 4, filetime + 20);
			}
			ಸ್ತಂಭ += 13;
		}
#ಆದರೆ_ಅಂತ್ಯ
	}

#ಆದರೆ ENABLE_FEATURE_LS_COLOR
	ಆದರೆ (G_show_color) {
		ಬಹುಲಕ_t ಬಹುಲಕ = dn->dn_ಬಹುಲಕ_lstat;
		ಆದರೆ (!ಬಹುಲಕ)
			ಆದರೆ (lstat(dn->fullname, &statbuf) == 0)
				ಬಹುಲಕ = statbuf.st_ಬಹುಲಕ;
		ಬರೆ(ESC"[%u;%um", bold(ಬಹುಲಕ), fgcolor(ಬಹುಲಕ));
	}
#ಆದರೆ_ಅಂತ್ಯ
	ಸ್ತಂಭ += print_name(dn->name);
	ಆದರೆ (G_show_color) {
		ಬರೆ(ESC"[m");
	}

	ಆದರೆ (lpath) {
		ಬರೆ(" -> ");
#ಆದರೆ ENABLE_FEATURE_LS_FILETYPES || ENABLE_FEATURE_LS_COLOR
		ಆದರೆ ((opt & (OPT_F|OPT_p))
		 || G_show_color
		) {
			ಬಹುಲಕ_t ಬಹುಲಕ = dn->dn_ಬಹುಲಕ_stat;
			ಆದರೆ (!ಬಹುಲಕ)
				ಆದರೆ (stat(dn->fullname, &statbuf) == 0)
					ಬಹುಲಕ = statbuf.st_ಬಹುಲಕ;
# ಆದರೆ ENABLE_FEATURE_LS_FILETYPES
			append = append_char(ಬಹುಲಕ);
# endif
# ಆದರೆ ENABLE_FEATURE_LS_COLOR
			ಆದರೆ (G_show_color) {
				ಬರೆ(ESC"[%u;%um", bold(ಬಹುಲಕ), fgcolor(ಬಹುಲಕ));
			}
# endif
		}
#ಆದರೆ_ಅಂತ್ಯ
		ಸ್ತಂಭ += print_name(lpath) + 4;
		ಬಿಡುಗಡೆ(lpath);
		ಆದರೆ (G_show_color) {
			ಬರೆ(ESC"[m");
		}
	}
#ಆದರೆ ENABLE_FEATURE_LS_FILETYPES
	ಆದರೆ (opt & (OPT_F|OPT_p)) {
		ಆದರೆ (append) {
			ಅಕ್ಷರ_ಬರೆ(append);
			ಸ್ತಂಭ++;
		}
	}
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ ಸ್ತಂಭ;
}

ಸ್ಥಿರ ಶೂನ್ಯ display_files(ರಚನೆ dnode **dn, ಚಿಹ್ನೆರಹಿತ nfiles)
{
	ಚಿಹ್ನೆರಹಿತ i, ncols, nrows, ಅಡ್ಡಸಾಲು, nc;
	ಚಿಹ್ನೆರಹಿತ ಸ್ತಂಭ;
	ಚಿಹ್ನೆರಹಿತ nexttab;
	ಚಿಹ್ನೆರಹಿತ ಸ್ತಂಭ_ಅಗಲ = 0; /* used only by coulmnal output */

	ಆದರೆ (option_mask32 & (OPT_l|OPT_1)) {
		ncols = 1;
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* find the longest file name, use that as the column width */
		ಗಾಗಿ (i = 0; dn[i]; i++) {
			ಪೂರ್ಣಾಂಕ len = calc_name_len(dn[i]->name);
			ಆದರೆ (ಸ್ತಂಭ_ಅಗಲ < len)
				ಸ್ತಂಭ_ಅಗಲ = len;
		}
		ಸ್ತಂಭ_ಅಗಲ += 2
			+ ((option_mask32 & OPT_Z) ? 33 : 0) /* context width */
			+ ((option_mask32 & OPT_i) ? 8 : 0) /* inode# width */
			+ ((option_mask32 & OPT_s) ? 5 : 0) /* "alloc block" width */
		;
		ncols = (ಚಿಹ್ನೆರಹಿತ)G_terminal_ಅಗಲ / ಸ್ತಂಭ_ಅಗಲ;
	}

	ಆದರೆ (ncols > 1) {
		nrows = nfiles / ncols;
		ಆದರೆ (nrows * ncols < nfiles)
			nrows++;                /* round up fractionals */
	} ಇಲ್ಲದಿದ್ದರೆ {
		nrows = nfiles;
		ncols = 1;
	}

	ಸ್ತಂಭ = 0;
	nexttab = 0;
	ಗಾಗಿ (ಅಡ್ಡಸಾಲು = 0; ಅಡ್ಡಸಾಲು < nrows; ಅಡ್ಡಸಾಲು++) {
		ಗಾಗಿ (nc = 0; nc < ncols; nc++) {
			/* reach into the array based on the column and row */
			ಆದರೆ (option_mask32 & OPT_x)
				i = (ಅಡ್ಡಸಾಲು * ncols) + nc;	/* display across row */
			ಇಲ್ಲದಿದ್ದರೆ
				i = (nc * nrows) + ಅಡ್ಡಸಾಲು;	/* display by column */
			ಆದರೆ (i < nfiles) {
				ಆದರೆ (ಸ್ತಂಭ > 0) {
					nexttab -= ಸ್ತಂಭ;
					ಬರೆ("%*s", nexttab, "");
					ಸ್ತಂಭ += nexttab;
				}
				nexttab = ಸ್ತಂಭ + ಸ್ತಂಭ_ಅಗಲ;
				ಸ್ತಂಭ += display_single(dn[i]);
			}
		}
		ಅಕ್ಷರ_ಬರೆ('\n');
		ಸ್ತಂಭ = 0;
	}
}


/*** Dir scanning code ***/

ಸ್ಥಿರ ರಚನೆ dnode *my_stat(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fullname, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಪೂರ್ಣಾಂಕ force_follow)
{
	ರಚನೆ stat statbuf;
	ರಚನೆ dnode *cur;

	cur = xzalloc(ಗಾತ್ರ(*cur));
	cur->fullname = fullname;
	cur->name = name;

	ಆದರೆ ((option_mask32 & OPT_L) || force_follow) {
#ಆದರೆ ENABLE_SELINUX
		ಆದರೆ (option_mask32 & OPT_Z) {
			getfilecon(fullname, &cur->sid);
		}
#ಆದರೆ_ಅಂತ್ಯ
		ಆದರೆ (stat(fullname, &statbuf)) {
			bb_simple_perror_ಸಂದೇಶ(fullname);
			G.exit_ಸಂಕೇತ = EXIT_FAILURE;
			ಬಿಡುಗಡೆ(cur);
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}
		cur->dn_ಬಹುಲಕ_stat = statbuf.st_ಬಹುಲಕ;
	} ಇಲ್ಲದಿದ್ದರೆ {
#ಆದರೆ ENABLE_SELINUX
		ಆದರೆ (option_mask32 & OPT_Z) {
			lgetfilecon(fullname, &cur->sid);
		}
#ಆದರೆ_ಅಂತ್ಯ
		ಆದರೆ (lstat(fullname, &statbuf)) {
			bb_simple_perror_ಸಂದೇಶ(fullname);
			G.exit_ಸಂಕೇತ = EXIT_FAILURE;
			ಬಿಡುಗಡೆ(cur);
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}
		cur->dn_ಬಹುಲಕ_lstat = statbuf.st_ಬಹುಲಕ;
	}

	/* cur->dstat = statbuf: */
	cur->dn_ಬಹುಲಕ   = statbuf.st_ಬಹುಲಕ  ;
	cur->dn_ಗಾತ್ರ   = statbuf.st_ಗಾತ್ರ  ;
#ಆದರೆ ENABLE_FEATURE_LS_TIMESTAMPS || ENABLE_FEATURE_LS_SORTFILES
	cur->dn_ಸಮಯ   = statbuf.st_mtime ;
	ಆದರೆ (option_mask32 & OPT_u)
		cur->dn_ಸಮಯ = statbuf.st_atime;
	ಆದರೆ (option_mask32 & OPT_c)
		cur->dn_ಸಮಯ = statbuf.st_ctime;
#ಆದರೆ_ಅಂತ್ಯ
	cur->dn_ino    = statbuf.st_ino   ;
	cur->dn_ಖಂಡಗಳು = statbuf.st_ಖಂಡಗಳು;
	cur->dn_nlink  = statbuf.st_nlink ;
	cur->dn_uid    = statbuf.st_uid   ;
	cur->dn_gid    = statbuf.st_gid   ;
	cur->dn_rdev_maj = major(statbuf.st_rdev);
	cur->dn_rdev_min = minor(statbuf.st_rdev);

	ಹಿಂತಿರುಗಿ cur;
}

ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಎಣಿಕೆ_dirs(ರಚನೆ dnode **dn, ಪೂರ್ಣಾಂಕ which)
{
	ಚಿಹ್ನೆರಹಿತ dirs, all;

	ಆದರೆ (!dn)
		ಹಿಂತಿರುಗಿ 0;

	dirs = all = 0;
	ಗಾಗಿ (; *dn; dn++) {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;

		all++;
		ಆದರೆ (!S_ISDIR((*dn)->dn_ಬಹುಲಕ))
			ಮುಂದುವರಿ;

		name = (*dn)->name;
		ಆದರೆ (which != ವಿಭಜಿಸು_SUBDIR /* if not requested to skip . / .. */
		 /* or if it's not . or .. */
		 || name[0] != '.'
		 || (name[1] && (name[1] != '.' || name[2]))
		) {
			dirs++;
		}
	}
	ಹಿಂತಿರುಗಿ which != ವಿಭಜಿಸು_FILE ? dirs : all - dirs;
}

/* get memory to hold an array of pointers */
ಸ್ಥಿರ ರಚನೆ dnode **dnalloc(ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ)
{
	ಆದರೆ (ಅಂಕೆ < 1)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಅಂಕೆ++; /* so that we have terminating NULL */
	ಹಿಂತಿರುಗಿ xzalloc(ಅಂಕೆ * ಗಾತ್ರ(ರಚನೆ dnode *));
}

#ಆದರೆ ENABLE_FEATURE_LS_RECURSIVE
ಸ್ಥಿರ ಶೂನ್ಯ dfree(ರಚನೆ dnode **dnp)
{
	ಚಿಹ್ನೆರಹಿತ i;

	ಆದರೆ (dnp == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ;

	ಗಾಗಿ (i = 0; dnp[i]; i++) {
		ರಚನೆ dnode *cur = dnp[i];
		ಆದರೆ (cur->fname_allocated)
			ಬಿಡುಗಡೆ((ಅಕ್ಷರ*)cur->fullname);
		ಬಿಡುಗಡೆ(cur);
	}
	ಬಿಡುಗಡೆ(dnp);
}
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು dfree(...) ((ಶೂನ್ಯ)0)
#ಆದರೆ_ಅಂತ್ಯ

/* Returns NULL-terminated malloced vector of pointers (or NULL) */
ಸ್ಥಿರ ರಚನೆ dnode **splitdnarray(ರಚನೆ dnode **dn, ಪೂರ್ಣಾಂಕ which)
{
	ಚಿಹ್ನೆರಹಿತ dncnt, d;
	ರಚನೆ dnode **dnp;

	ಆದರೆ (dn == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	/* count how many dirs or files there are */
	dncnt = ಎಣಿಕೆ_dirs(dn, which);

	/* allocate a file array and a dir array */
	dnp = dnalloc(dncnt);

	/* copy the entrys into the file or dir array */
	ಗಾಗಿ (d = 0; *dn; dn++) {
		ಆದರೆ (S_ISDIR((*dn)->dn_ಬಹುಲಕ)) {
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;

			ಆದರೆ (which == ವಿಭಜಿಸು_FILE)
				ಮುಂದುವರಿ;

			name = (*dn)->name;
			ಆದರೆ ((which & ವಿಭಜಿಸು_DIR) /* any dir... */
			/* ... or not . or .. */
			 || name[0] != '.'
			 || (name[1] && (name[1] != '.' || name[2]))
			) {
				dnp[d++] = *dn;
			}
		} ಇಲ್ಲದಿದ್ದರೆ
		ಆದರೆ (which == ವಿಭಜಿಸು_FILE) {
			dnp[d++] = *dn;
		}
	}
	ಹಿಂತಿರುಗಿ dnp;
}

#ಆದರೆ ENABLE_FEATURE_LS_SORTFILES
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sortcmp(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *a, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *b)
{
	ರಚನೆ dnode *d1 = *(ರಚನೆ dnode **)a;
	ರಚನೆ dnode *d2 = *(ರಚನೆ dnode **)b;
	ಚಿಹ್ನೆರಹಿತ opt = option_mask32;
	off_t dif;

	dif = 0; /* assume sort by name */
	// TODO: use pre-initialized function pointer
	// instead of branch forest
	ಆದರೆ (opt & OPT_dirs_ಮೊದಲ) {
		dif = S_ISDIR(d2->dn_ಬಹುಲಕ) - S_ISDIR(d1->dn_ಬಹುಲಕ);
		ಆದರೆ (dif != 0)
			ಹೋಗು maybe_invert_and_ret;
	}

	ಆದರೆ (opt & OPT_S) { /* sort by size */
		dif = (d2->dn_ಗಾತ್ರ - d1->dn_ಗಾತ್ರ);
	} ಇಲ್ಲದಿದ್ದರೆ
	ಆದರೆ (opt & OPT_t) { /* sort by time */
		dif = (d2->dn_ಸಮಯ - d1->dn_ಸಮಯ);
	} ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ defined(HAVE_STRVERSCMP) && HAVE_STRVERSCMP == 1
	ಆದರೆ (opt & OPT_v) { /* sort by version */
		dif = strverscmp(d1->name, d2->name);
	} ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (opt & OPT_X) { /* sort by extension */
		dif = strcmp(strchrnul(d1->name, '.'), strchrnul(d2->name, '.'));
	}
	ಆದರೆ (dif == 0) {
		/* sort by name, use as tie breaker for other sorts */
		ಆದರೆ (ENABLE_LOCALE_SUPPORT)
			dif = strcoll(d1->name, d2->name);
		ಇಲ್ಲದಿದ್ದರೆ
			dif = strcmp(d1->name, d2->name);
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Make dif fit into an int */
		ಆದರೆ (ಗಾತ್ರ(dif) > ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ)) {
			ಎಣಿಕೆ { ಬಿಟ್ಟುಗಳು_ಗಮ್ಯ_ಸರಿಸು = 8 * (ಗಾತ್ರ(dif) - ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ)) };
			/* shift leaving only "int" worth of bits */
			/* (this requires dif != 0, and here it is nonzero) */
			dif = 1 | (ಪೂರ್ಣಾಂಕ)((uoff_t)dif >> ಬಿಟ್ಟುಗಳು_ಗಮ್ಯ_ಸರಿಸು);
		}
	}
 maybe_invert_and_ret:
	ಹಿಂತಿರುಗಿ (opt & OPT_r) ? -(ಪೂರ್ಣಾಂಕ)dif : (ಪೂರ್ಣಾಂಕ)dif;
}

ಸ್ಥಿರ ಶೂನ್ಯ dnsort(ರಚನೆ dnode **dn, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2)
{
	qsort(dn, ಗಾತ್ರ_2, ಗಾತ್ರ(*dn), sortcmp);
}

ಸ್ಥಿರ ಶೂನ್ಯ ವಿಂಗಡಿಸು_and_display_files(ರಚನೆ dnode **dn, ಚಿಹ್ನೆರಹಿತ nfiles)
{
	dnsort(dn, nfiles);
	display_files(dn, nfiles);
}
#ಇಲ್ಲದಿದ್ದರೆ
# define dnsort(dn, ಗಾತ್ರ_2) ((ಶೂನ್ಯ)0)
# define ವಿಂಗಡಿಸು_and_display_files(dn, nfiles) display_files(dn, nfiles)
#ಆದರೆ_ಅಂತ್ಯ

/* Returns NULL-terminated malloced vector of pointers (or NULL) */
ಸ್ಥಿರ ರಚನೆ dnode **ಪರಿಶೀಲನೆ_one_dir(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ಚಿಹ್ನೆರಹಿತ *nfiles_p)
{
	ರಚನೆ dnode *dn, *cur, **dnp;
	ರಚನೆ dirent *ನಮೂದು;
	DIR *dir;
	ಚಿಹ್ನೆರಹಿತ i, nfiles;

	*nfiles_p = 0;
	dir = warn_opendir(ಮಾರ್ಗ);
	ಆದರೆ (dir == ಖಾಲಿ) {
		G.exit_ಸಂಕೇತ = EXIT_FAILURE;
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;	/* could not open the dir */
	}
	dn = ಖಾಲಿ;
	nfiles = 0;
	ಆಗ ((ನಮೂದು = readdir(dir)) != ಖಾಲಿ) {
		ಅಕ್ಷರ *fullname;

		/* are we going to list the file- it may be . or .. or a hidden file */
		ಆದರೆ (ನಮೂದು->d_name[0] == '.') {
			ಆದರೆ (!(option_mask32 & (OPT_a|OPT_A)))
				ಮುಂದುವರಿ; /* skip all dotfiles if no -a/-A */
			ಆದರೆ (!(option_mask32 & OPT_a)
			 && (!ನಮೂದು->d_name[1] || (ನಮೂದು->d_name[1] == '.' && !ನಮೂದು->d_name[2]))
			) {
				ಮುಂದುವರಿ; /* if only -A, skip . and .. but show other dotfiles */
			}
		}
		fullname = concat_ಮಾರ್ಗ_file(ಮಾರ್ಗ, ನಮೂದು->d_name);
		cur = my_stat(fullname, bb_basename(fullname), 0);
		ಆದರೆ (!cur) {
			ಬಿಡುಗಡೆ(fullname);
			ಮುಂದುವರಿ;
		}
		cur->fname_allocated = 1;
		cur->dn_ಮುಂದಿನ = dn;
		dn = cur;
		nfiles++;
	}
	closedir(dir);

	ಆದರೆ (dn == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	/* now that we know how many files there are
	 * allocate memory for an array to hold dnode pointers
	 */
	*nfiles_p = nfiles;
	dnp = dnalloc(nfiles);
	ಗಾಗಿ (i = 0; /* i < nfiles - detected via !dn below */; i++) {
		dnp[i] = dn;	/* save pointer to node in array */
		dn = dn->dn_ಮುಂದಿನ;
		ಆದರೆ (!dn)
			ನಿಲ್ಲಿಸು;
	}

	ಹಿಂತಿರುಗಿ dnp;
}

#ಆದರೆ ENABLE_DESKTOP
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/ls.html
 * If any of the -l, -n, -s options is specified, each list
 * of files within the directory shall be preceded by a
 * status line indicating the number of file system blocks
 * occupied by files in the directory in 512-byte units if
 * the -k option is not specified, or 1024-byte units if the
 * -k option is specified, rounded up to the next integral
 * number of units.
 */
/* by Jorgen Overgaard (jorgen AT antistaten.se) */
ಸ್ಥಿರ off_t calculate_ಖಂಡಗಳು(ರಚನೆ dnode **dn)
{
	uoff_t ಖಂಡಗಳು = 1;
	ಆದರೆ (dn) {
		ಆಗ (*dn) {
			/* st_blocks is in 512 byte blocks */
			ಖಂಡಗಳು += (*dn)->dn_ಖಂಡಗಳು;
			dn++;
		}
	}

	/* Even though standard says use 512 byte blocks, coreutils use 1k */
	/* Actually, we round up by calculating (blocks + 1) / 2,
	 * "+ 1" was done when we initialized blocks to 1 */
	ಹಿಂತಿರುಗಿ ಖಂಡಗಳು >> 1;
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಶೂನ್ಯ ಪರಿಶೀಲನೆ_and_display_dirs_recur(ರಚನೆ dnode **dn, ಪೂರ್ಣಾಂಕ ಮೊದಲ)
{
	ಚಿಹ್ನೆರಹಿತ nfiles;
	ರಚನೆ dnode **subdnp;

	ಗಾಗಿ (; *dn; dn++) {
		ಆದರೆ (G.show_dirname || (option_mask32 & OPT_R)) {
			ಆದರೆ (!ಮೊದಲ)
				bb_putchar('\n');
			ಮೊದಲ = 0;
			ಬರೆ("%s:\n", (*dn)->fullname);
		}
		subdnp = ಪರಿಶೀಲನೆ_one_dir((*dn)->fullname, &nfiles);
#ಆದರೆ ENABLE_DESKTOP
		ಆದರೆ (option_mask32 & (OPT_s|OPT_l)) {
			ಆದರೆ (option_mask32 & OPT_h) {
				ಬರೆ("total %-"HUMAN_READABLE_MAX_ಅಗಲ_STR"s\n",
					/* print size, no fractions, use suffixes */
					make_human_readable_str(calculate_ಖಂಡಗಳು(subdnp) * 1024,
								0, 0)
				);
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಬರೆ("total %"OFF_FMT"u\n", calculate_ಖಂಡಗಳು(subdnp));
			}
		}
#ಆದರೆ_ಅಂತ್ಯ
		ಆದರೆ (nfiles > 0) {
			/* list all files at this level */
			ವಿಂಗಡಿಸು_and_display_files(subdnp, nfiles);

			ಆದರೆ (ENABLE_FEATURE_LS_RECURSIVE
			 && (option_mask32 & OPT_R)
			) {
				ರಚನೆ dnode **dnd;
				ಚಿಹ್ನೆರಹಿತ dndirs;
				/* recursive - list the sub-dirs */
				dnd = splitdnarray(subdnp, ವಿಭಜಿಸು_SUBDIR);
				dndirs = ಎಣಿಕೆ_dirs(subdnp, ವಿಭಜಿಸು_SUBDIR);
				ಆದರೆ (dndirs > 0) {
					dnsort(dnd, dndirs);
					ಪರಿಶೀಲನೆ_and_display_dirs_recur(dnd, 0);
					/* free the array of dnode pointers to the dirs */
					ಬಿಡುಗಡೆ(dnd);
				}
			}
			/* free the dnodes and the fullname mem */
			dfree(subdnp);
		}
	}
}


ಪೂರ್ಣಾಂಕ ls_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{	/*      ^^^^^^^^^^^^^^^^^ note: if FTPD, argc can be wrong, see ftpd.c */
	ರಚನೆ dnode **dnd;
	ರಚನೆ dnode **dnf;
	ರಚನೆ dnode **dnp;
	ರಚನೆ dnode *dn;
	ರಚನೆ dnode *cur;
	ಚಿಹ್ನೆರಹಿತ opt;
	ಚಿಹ್ನೆರಹಿತ nfiles;
	ಚಿಹ್ನೆರಹಿತ dnfiles;
	ಚಿಹ್ನೆರಹಿತ dndirs;
	ಚಿಹ್ನೆರಹಿತ i;
#ಆದರೆ ENABLE_FEATURE_LS_COLOR
	/* colored LS support by JaWi, janwillem.janssen@lxtreme.nl */
	/* coreutils 6.10:
	 * # ls --color=BOGUS
	 * ls: invalid argument 'BOGUS' for '--color'
	 * Valid arguments are:
	 * 'always', 'yes', 'force'
	 * 'never', 'no', 'none'
	 * 'auto', 'tty', 'if-tty'
	 * (and substrings: "--color=alwa" work too)
	 */
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ color_str[] ALIGN1 =
		"always\0""yes\0""force\0"
		"auto\0""tty\0""if-tty\0";
	/* need to initialize since --color has _an optional_ argument */
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *color_opt = color_str; /* "always" */
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_LONG_OPTS
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ ls_longopts[] ALIGN1 =
		"full-time\0" No_ವಾದ "\xff"
		"group-directories-first\0" No_ವಾದ "\xfe"
		IF_FEATURE_LS_COLOR("color\0" Optional_ವಾದ "\xfd")
	;
#ಆದರೆ_ಅಂತ್ಯ

	INIT_G();

	init_unicode();

#ಆದರೆ ENABLE_FEATURE_LS_ಅಗಲ
	/* obtain the terminal width */
	G_terminal_ಅಗಲ = get_terminal_ಅಗಲ(STDIN_FILENO);
	/* go one less... */
	G_terminal_ಅಗಲ--;
#ಆದರೆ_ಅಂತ್ಯ

	/* process options */
	opt = getopt32long(argv, "^"
		ls_options
			"\0"
			/* -n and -g imply -l */
			"nl:gl"
			/* --full-time implies -l */
			IF_FEATURE_LS_TIMESTAMPS(IF_LONG_OPTS(":\xff""l"))
			/* http://pubs.opengroup.org/onlinepubs/9699919799/utilities/ls.html:
			 * in some pairs of opts, only last one takes effect:
			 */
			IF_FEATURE_LS_TIMESTAMPS(IF_FEATURE_LS_SORTFILES(":t-S:S-t")) /* time/size */
			// ":m-l:l-m" - we don't have -m
			IF_FEATURE_LS_FOLLOWLINKS(":H-L:L-H")
			":C-xl:x-Cl:l-xC" /* bycols/bylines/long */
			":C-1:1-C" /* bycols/oneline */
			":x-1:1-x" /* bylines/oneline (not in SuS, but in GNU coreutils 8.4) */
			IF_FEATURE_LS_TIMESTAMPS(":c-u:u-c") /* mtime/atime */
			/* -w NUM: */
			IF_FEATURE_LS_ಅಗಲ(":w+")
		, ls_longopts
		IF_FEATURE_LS_ಅಗಲ(, /*-T*/ ಖಾಲಿ, /*-w*/ &G_terminal_ಅಗಲ)
		IF_FEATURE_LS_COLOR(, &color_opt)
	);
#ಆದರೆ 0 /* option bits debug */
	bb_error_ಸಂದೇಶ("opt:0x%08x l:%x H:%x color:%x dirs:%x", opt, OPT_l, OPT_H, OPT_color, OPT_dirs_ಮೊದಲ);
	ಆದರೆ (opt & OPT_c         ) bb_error_ಸಂದೇಶ("-c");
	ಆದರೆ (opt & OPT_l         ) bb_error_ಸಂದೇಶ("-l");
	ಆದರೆ (opt & OPT_H         ) bb_error_ಸಂದೇಶ("-H");
	ಆದರೆ (opt & OPT_color     ) bb_error_ಸಂದೇಶ("--color");
	ಆದರೆ (opt & OPT_dirs_ಮೊದಲ) bb_error_ಸಂದೇಶ("--group-directories-first");
	ಆದರೆ (opt & OPT_ತುಂಬಿದ_ಸಮಯ ) bb_error_ಸಂದೇಶ("--full-time");
	ನಿರ್ಗಮಿಸು(0);
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_SELINUX
	ಆದರೆ (opt & OPT_Z) {
		ಆದರೆ (!is_selinux_enabled())
			option_mask32 &= ~OPT_Z;
	}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_FEATURE_LS_COLOR
	/* set G_show_color = 1/0 */
	ಆದರೆ (ENABLE_FEATURE_LS_COLOR_IS_DEFAULT && !is_ಪದ_dumb()) {
		ಅಕ್ಷರ *p = getenv("LS_COLORS");
		/* LS_COLORS is unset, or (not empty && not "none") ? */
		ಆದರೆ (!p || (p[0] && strcmp(p, "none") != 0)) {
			ಆದರೆ (isatty(STDOUT_FILENO)) {
				/* check isatty() last because it's expensive (syscall) */
				G_show_color = 1;
			}
		}
	}
	ಆದರೆ (opt & OPT_color) {
		ಆದರೆ (color_opt[0] == 'n')
			G_show_color = 0;
		ಇಲ್ಲದಿದ್ದರೆ ಆಯ್ಕೆ (ಸೂಚ್ಯಂಕ_ಒಳ_substrings(color_str, color_opt)) {
		ಸ್ಥಿತಿ 3:
		ಸ್ಥಿತಿ 4:
		ಸ್ಥಿತಿ 5:
			ಆದರೆ (!is_ಪದ_dumb() && isatty(STDOUT_FILENO)) {
		ಸ್ಥಿತಿ 0:
		ಸ್ಥಿತಿ 1:
		ಸ್ಥಿತಿ 2:
				G_show_color = 1;
			}
		}
	}
#ಆದರೆ_ಅಂತ್ಯ

	/* sort out which command line options take precedence */
	ಆದರೆ (ENABLE_FEATURE_LS_RECURSIVE && (opt & OPT_d))
		option_mask32 &= ~OPT_R;	/* no recurse if listing only dir */
	ಆದರೆ (!(opt & OPT_l)) { /* not -l? */
		ಆದರೆ (ENABLE_FEATURE_LS_TIMESTAMPS && ENABLE_FEATURE_LS_SORTFILES) {
			/* when to sort by time? -t[cu] sorts by time even with -l */
			/* (this is achieved by opt_flags[] element for -t) */
			/* without -l, bare -c or -u enable sort too */
			/* (with -l, bare -c or -u just select which time to show) */
			ಆದರೆ (opt & (OPT_c|OPT_u)) {
				option_mask32 |= OPT_t;
			}
		}
	}

	/* choose a display format if one was not already specified by an option */
	ಆದರೆ (!(option_mask32 & (OPT_l|OPT_1|OPT_x|OPT_C)))
		option_mask32 |= (isatty(STDOUT_FILENO) ? OPT_C : OPT_1);

	ಆದರೆ (ENABLE_FTPD && applet_name[0] == 'f') {
		/* ftpd secret backdoor. dirs first are much nicer */
		option_mask32 |= OPT_dirs_ಮೊದಲ;
	}

	argv += optind;
	ಆದರೆ (!argv[0])
		*--argv = (ಅಕ್ಷರ*)".";

	ಆದರೆ (argv[1])
		G.show_dirname = 1; /* 2 or more items? label directories */

	/* stuff the command line file names into a dnode array */
	dn = ಖಾಲಿ;
	nfiles = 0;
	ಮಾಡು {
		cur = my_stat(*argv, *argv,
			/* follow links on command line unless -l, -i, -s or -F: */
			!(option_mask32 & (OPT_l|OPT_i|OPT_s|OPT_F))
			/* ... or if -H: */
			|| (option_mask32 & OPT_H)
			/* ... or if -L, but my_stat always follows links if -L */
		);
		argv++;
		ಆದರೆ (!cur)
			ಮುಂದುವರಿ;
		/*cur->fname_allocated = 0; - already is */
		cur->dn_ಮುಂದಿನ = dn;
		dn = cur;
		nfiles++;
	} ಆಗ (*argv);

	/* nfiles _may_ be 0 here - try "ls doesnt_exist" */
	ಆದರೆ (nfiles == 0)
		ಹಿಂತಿರುಗಿ G.exit_ಸಂಕೇತ;

	/* now that we know how many files there are
	 * allocate memory for an array to hold dnode pointers
	 */
	dnp = dnalloc(nfiles);
	ಗಾಗಿ (i = 0; /* i < nfiles - detected via !dn below */; i++) {
		dnp[i] = dn;	/* save pointer to node in array */
		dn = dn->dn_ಮುಂದಿನ;
		ಆದರೆ (!dn)
			ನಿಲ್ಲಿಸು;
	}

	ಆದರೆ (option_mask32 & OPT_d) {
		ವಿಂಗಡಿಸು_and_display_files(dnp, nfiles);
	} ಇಲ್ಲದಿದ್ದರೆ {
		dnd = splitdnarray(dnp, ವಿಭಜಿಸು_DIR);
		dnf = splitdnarray(dnp, ವಿಭಜಿಸು_FILE);
		dndirs = ಎಣಿಕೆ_dirs(dnp, ವಿಭಜಿಸು_DIR);
		dnfiles = nfiles - dndirs;
		ಆದರೆ (dnfiles > 0) {
			ವಿಂಗಡಿಸು_and_display_files(dnf, dnfiles);
			ಆದರೆ (ENABLE_FEATURE_CLEAN_ಮೇಲೆ)
				ಬಿಡುಗಡೆ(dnf);
		}
		ಆದರೆ (dndirs > 0) {
			dnsort(dnd, dndirs);
			ಪರಿಶೀಲನೆ_and_display_dirs_recur(dnd, dnfiles == 0);
			ಆದರೆ (ENABLE_FEATURE_CLEAN_ಮೇಲೆ)
				ಬಿಡುಗಡೆ(dnd);
		}
	}

	ಆದರೆ (ENABLE_FEATURE_CLEAN_ಮೇಲೆ)
		dfree(dnp);
	ಹಿಂತಿರುಗಿ G.exit_ಸಂಕೇತ;
}
