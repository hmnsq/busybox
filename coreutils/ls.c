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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "common_bufsiz.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "unicode.h"


/* This is a NOEXEC applet. Be very careful! */


#ਜੇ ENABLE_FTPD
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
#ਜੇ_ਅੰਤ


ਗਿਣਤੀ {
TERMINAL_ਚੌੜਾਈ  = 80, /* use 79 if terminal has linefold bug */

ਵੰਡੋ_FILE      = 0,
ਵੰਡੋ_DIR       = 1,
ਵੰਡੋ_SUBDIR    = 2,
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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ls_options \
	"Cadi1lgnsxAk"       /* 12 opts, total 12 */ \
	IF_FEATURE_LS_FILETYPES("Fp")    /* 2, 14 */ \
	IF_FEATURE_LS_RECURSIVE("R")     /* 1, 15 */ \
	IF_SELINUX("Z")                  /* 1, 16 */ \
	"Q"                              /* 1, 17 */ \
	IF_FEATURE_LS_TIMESTAMPS("ctu")  /* 3, 20 */ \
	IF_FEATURE_LS_SORTFILES("SXrv")  /* 4, 24 */ \
	IF_FEATURE_LS_FOLLOWLINKS("LH")  /* 2, 26 */ \
	IF_FEATURE_HUMAN_READABLE("h")   /* 1, 27 */ \
	IF_FEATURE_LS_ਚੌੜਾਈ("T:w:")      /* 2, 29 */

ਗਿਣਤੀ {
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
	OPTBIT_ਭਰਿਆ_ਸਮਾਂ = OPTBIT_T + 2 * ENABLE_FEATURE_LS_ਚੌੜਾਈ,
	OPTBIT_dirs_ਪਹਿਲਾ,
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
	OPT_T = (1 << OPTBIT_T) * ENABLE_FEATURE_LS_ਚੌੜਾਈ,
	OPT_w = (1 << OPTBIT_w) * ENABLE_FEATURE_LS_ਚੌੜਾਈ,
	OPT_ਭਰਿਆ_ਸਮਾਂ  = (1 << OPTBIT_ਭਰਿਆ_ਸਮਾਂ ) * ENABLE_LONG_OPTS,
	OPT_dirs_ਪਹਿਲਾ = (1 << OPTBIT_dirs_ਪਹਿਲਾ) * ENABLE_LONG_OPTS,
	OPT_color      = (1 << OPTBIT_color     ) * ENABLE_FEATURE_LS_COLOR,
};

/*
 * a directory entry and its stat info
 */
ਬਣਤਰ dnode {
	ਅਚਲ ਅੱਖਰ *name;       /* usually basename, but think "ls -l dir/file" */
	ਅਚਲ ਅੱਖਰ *fullname;   /* full name (usable for stat etc) */
	ਬਣਤਰ dnode *dn_ਅਗਲਾ;  /* for linked list */
	IF_SELINUX(security_context_t sid;)
	smallint fname_allocated;

	/* Used to avoid re-doing [l]stat at printout stage
	 * if we already collected needed data in scan stage:
	 */
	ਬਹੁਲਕ_t    dn_ਬਹੁਲਕ_lstat;   /* obtained with lstat, or 0 */
	ਬਹੁਲਕ_t    dn_ਬਹੁਲਕ_stat;    /* obtained with stat, or 0 */

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
	ਬਹੁਲਕ_t    dn_ਬਹੁਲਕ; /* obtained with lstat OR stat, depending on -L etc */
	off_t     dn_ਮਾਪ;
#ਜੇ ENABLE_FEATURE_LS_TIMESTAMPS || ENABLE_FEATURE_LS_SORTFILES
	ਸਮਾਂ_t    dn_ਸਮਾਂ;
#ਜੇ_ਅੰਤ
	ino_t     dn_ino;
	blkcnt_t  dn_ਖੰਡਾਂ;
	nlink_t   dn_nlink;
	uid_t     dn_uid;
	gid_t     dn_gid;
	ਪੂਰਨ       dn_rdev_maj;
	ਪੂਰਨ       dn_rdev_min;
//	dev_t     dn_dev;
//	blksize_t dn_blksize;
};

ਬਣਤਰ globals {
#ਜੇ ENABLE_FEATURE_LS_COLOR
	smallint show_color;
# define G_show_color (G.show_color)
#ਨਹੀਂਤਾਂ
# define G_show_color 0
#ਜੇ_ਅੰਤ
	smallint exit_ਕੋਡ;
	smallint show_dirname;
#ਜੇ ENABLE_FEATURE_LS_ਚੌੜਾਈ
	ਬਿਨਾਨਿਸ਼ਾਨੀ terminal_ਚੌੜਾਈ;
# define G_terminal_ਚੌੜਾਈ (G.terminal_ਚੌੜਾਈ)
#ਨਹੀਂਤਾਂ
# define G_terminal_ਚੌੜਾਈ TERMINAL_ਚੌੜਾਈ
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_FEATURE_LS_TIMESTAMPS
	/* Do time() just once. Saves one syscall per file for "ls -l" */
	ਸਮਾਂ_t current_ਸਮਾਂ_t;
#ਜੇ_ਅੰਤ
} FIX_ALIASING;
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ G (*(ਬਣਤਰ globals*)bb_common_bufsiz1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INIT_G() ਕਰੋ { \
	ਤਿਆਰੀ_common_bufsiz(); \
	/* we have to zero it out because of NOEXEC */ \
	memset(&G, 0, ਆਕਾਰ(G)); \
	IF_FEATURE_LS_ਚੌੜਾਈ(G_terminal_ਚੌੜਾਈ = TERMINAL_ਚੌੜਾਈ;) \
	IF_FEATURE_LS_TIMESTAMPS(ਸਮਾਂ(&G.current_ਸਮਾਂ_t);) \
} ਜਦੋਂ (0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ESC "\033"


/*** Output code ***/


/* FYI type values: 1:fifo 2:char 4:dir 6:blk 8:file 10:link 12:socket
 * (various wacky OSes: 13:Sun door 14:BSD whiteout 5:XENIX named file
 *  3/7:multiplexed char/block device)
 * and we use 0 for unknown and 15 for executables (see below) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TYPEINDEX(ਬਹੁਲਕ) (((ਬਹੁਲਕ) >> 12) & 0x0f)
/*                       un  fi chr -   dir -  blk  -  file -  link - sock -   - exe */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ APPCHAR(ਬਹੁਲਕ)   ("\0""|""\0""\0""/""\0""\0""\0""\0""\0""@""\0""=""\0""\0""\0" [TYPEINDEX(ਬਹੁਲਕ)])
/* 036 black foreground              050 black background
   037 red foreground                051 red background
   040 green foreground              052 green background
   041 brown foreground              053 brown background
   042 blue foreground               054 blue background
   043 magenta (purple) foreground   055 magenta background
   044 cyan (light blue) foreground  056 cyan background
   045 gray foreground               057 white background
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COLOR(ਬਹੁਲਕ) ( \
	/*un  fi  chr  -  dir  -  blk  -  file -  link -  sock -   -  exe */ \
	"\037\043\043\045\042\045\043\043\000\045\044\045\043\045\045\040" \
	[TYPEINDEX(ਬਹੁਲਕ)])
/* Select normal (0) [actually "reset all"] or bold (1)
 * (other attributes are 2:dim 4:underline 5:blink 7:reverse,
 *  let's use 7 for "impossible" types, just for fun)
 * Note: coreutils 6.9 uses inverted red for setuid binaries.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATTR(ਬਹੁਲਕ) ( \
	/*un fi chr - dir - blk - file- link- sock- -  exe */ \
	"\01\00\01\07\01\07\01\07\00\07\01\07\01\07\07\01" \
	[TYPEINDEX(ਬਹੁਲਕ)])

#ਜੇ ENABLE_FEATURE_LS_COLOR
/* mode of zero is interpreted as "unknown" (stat failed) */
ਸਥਿਰ ਅੱਖਰ fgcolor(ਬਹੁਲਕ_t ਬਹੁਲਕ)
{
	ਜੇ (S_ISREG(ਬਹੁਲਕ) && (ਬਹੁਲਕ & (S_IXUSR | S_IXGRP | S_IXOTH)))
		ਵਾਪਸ COLOR(0xF000);	/* File is executable ... */
	ਵਾਪਸ COLOR(ਬਹੁਲਕ);
}
ਸਥਿਰ ਅੱਖਰ bold(ਬਹੁਲਕ_t ਬਹੁਲਕ)
{
	ਜੇ (S_ISREG(ਬਹੁਲਕ) && (ਬਹੁਲਕ & (S_IXUSR | S_IXGRP | S_IXOTH)))
		ਵਾਪਸ ATTR(0xF000);	/* File is executable ... */
	ਵਾਪਸ ATTR(ਬਹੁਲਕ);
}
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_FEATURE_LS_FILETYPES
ਸਥਿਰ ਅੱਖਰ append_char(ਬਹੁਲਕ_t ਬਹੁਲਕ)
{
	ਜੇ (!(option_mask32 & (OPT_F|OPT_p)))
		ਵਾਪਸ '\0';

	ਜੇ (S_ISDIR(ਬਹੁਲਕ))
		ਵਾਪਸ '/';
	ਜੇ (!(option_mask32 & OPT_F))
		ਵਾਪਸ '\0';
	ਜੇ (S_ISREG(ਬਹੁਲਕ) && (ਬਹੁਲਕ & (S_IXUSR | S_IXGRP | S_IXOTH)))
		ਵਾਪਸ '*';
	ਵਾਪਸ APPCHAR(ਬਹੁਲਕ);
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ calc_name_len(ਅਚਲ ਅੱਖਰ *name)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ len;
	uni_stat_t uni_stat;

	// TODO: quote tab as \t, etc, if -Q
	name = printable_string2(&uni_stat, name);

	ਜੇ (!(option_mask32 & OPT_Q)) {
		ਵਾਪਸ uni_stat.unicode_ਚੌੜਾਈ;
	}

	len = 2 + uni_stat.unicode_ਚੌੜਾਈ;
	ਜਦੋਂ (*name) {
		ਜੇ (*name == '"' || *name == '\\') {
			len++;
		}
		name++;
	}
	ਵਾਪਸ len;
}

/* Return the number of used columns.
 * Note that only columnar output uses return value.
 * -l and -1 modes don't care.
 * coreutils 7.2 also supports:
 * ls -b (--escape) = octal escapes (although it doesn't look like working)
 * ls -N (--literal) = not escape at all
 */
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ print_name(ਅਚਲ ਅੱਖਰ *name)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ len;
	uni_stat_t uni_stat;

	// TODO: quote tab as \t, etc, if -Q
	name = printable_string2(&uni_stat, name);

	ਜੇ (!(option_mask32 & OPT_Q)) {
		fputs_stdout(name);
		ਵਾਪਸ uni_stat.unicode_ਚੌੜਾਈ;
	}

	len = 2 + uni_stat.unicode_ਚੌੜਾਈ;
	ਅੱਖਰ_ਲਿਖੋ('"');
	ਜਦੋਂ (*name) {
		ਜੇ (*name == '"' || *name == '\\') {
			ਅੱਖਰ_ਲਿਖੋ('\\');
			len++;
		}
		ਅੱਖਰ_ਲਿਖੋ(*name);
		name++;
	}
	ਅੱਖਰ_ਲਿਖੋ('"');
	ਵਾਪਸ len;
}

/* Return the number of used columns.
 * Note that only columnar output uses return value,
 * -l and -1 modes don't care.
 */
ਸਥਿਰ NOINLINE ਬਿਨਾਨਿਸ਼ਾਨੀ display_single(ਅਚਲ ਬਣਤਰ dnode *dn)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਕਾਲਮ = 0;
	ਅੱਖਰ *lpath;
	ਪੂਰਨ opt;
#ਜੇ ENABLE_FEATURE_LS_FILETYPES || ENABLE_FEATURE_LS_COLOR
	ਬਣਤਰ stat statbuf;
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_FEATURE_LS_FILETYPES
	ਅੱਖਰ append = append_char(dn->dn_ਬਹੁਲਕ);
#ਜੇ_ਅੰਤ

	opt = option_mask32;

	/* Do readlink early, so that if it fails, error message
	 * does not appear *inside* the "ls -l" line */
	lpath = ਨਿਰਾ;
	ਜੇ (opt & OPT_l)
		ਜੇ (S_ISLNK(dn->dn_ਬਹੁਲਕ))
			lpath = xmalloc_readlink_or_warn(dn->fullname);

	ਜੇ (opt & OPT_i) /* show inode# */
		ਕਾਲਮ += ਲਿਖੋ("%7llu ", (ਲੰਮਾ ਲੰਮਾ) dn->dn_ino);
	ਜੇ (opt & OPT_s) { /* show allocated blocks */
		ਜੇ (opt & OPT_h) {
			ਕਾਲਮ += ਲਿਖੋ("%"HUMAN_READABLE_MAX_ਚੌੜਾਈ_STR"s ",
				/* print size, show one fractional, use suffixes */
				make_human_readable_str((off_t)dn->dn_ਖੰਡਾਂ << 9, 1, 0)
			);
		} ਨਹੀਂਤਾਂ {
			ਕਾਲਮ += ਲਿਖੋ("%6"OFF_FMT"u ", (off_t)(dn->dn_ਖੰਡਾਂ >> 1));
		}
	}
	ਜੇ (opt & OPT_l) {
		/* long listing: show mode */
		ਅੱਖਰ modestr[12];
		ਕਾਲਮ += ਲਿਖੋ("%-10s ", bb_ਬਹੁਲਕ_string(modestr, dn->dn_ਬਹੁਲਕ));
		/* long listing: show number of links */
		ਕਾਲਮ += ਲਿਖੋ("%4lu ", (ਲੰਮਾ) dn->dn_nlink);
		/* long listing: show user/group */
		ਜੇ (opt & OPT_n) {
			ਜੇ (opt & OPT_g)
				ਕਾਲਮ += ਲਿਖੋ("%-8u ", (ਪੂਰਨ) dn->dn_gid);
			ਨਹੀਂਤਾਂ
				ਕਾਲਮ += ਲਿਖੋ("%-8u %-8u ",
						(ਪੂਰਨ) dn->dn_uid,
						(ਪੂਰਨ) dn->dn_gid);
		}
#ਜੇ ENABLE_FEATURE_LS_USERNAME
		ਨਹੀਂਤਾਂ {
			ਜੇ (opt & OPT_g) {
				ਕਾਲਮ += ਲਿਖੋ("%-8s ",
					get_cached_groupname(dn->dn_gid));
			} ਨਹੀਂਤਾਂ {
				ਕਾਲਮ += ਲਿਖੋ("%-8s %-8s ",
					get_cached_username(dn->dn_uid),
					get_cached_groupname(dn->dn_gid));
			}
		}
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_SELINUX
	}
	ਜੇ (opt & OPT_Z) {
		ਕਾਲਮ += ਲਿਖੋ("%-32s ", dn->sid ? dn->sid : "?");
		freecon(dn->sid);
	}
	ਜੇ (opt & OPT_l) {
#ਜੇ_ਅੰਤ
		/* long listing: show size */
		ਜੇ (S_ISBLK(dn->dn_ਬਹੁਲਕ) || S_ISCHR(dn->dn_ਬਹੁਲਕ)) {
			ਕਾਲਮ += ਲਿਖੋ("%4u, %3u ",
					dn->dn_rdev_maj,
					dn->dn_rdev_min);
		} ਨਹੀਂਤਾਂ {
			ਜੇ (opt & OPT_h) {
				ਕਾਲਮ += ਲਿਖੋ("%"HUMAN_READABLE_MAX_ਚੌੜਾਈ_STR"s ",
					/* print size, show one fractional, use suffixes */
					make_human_readable_str(dn->dn_ਮਾਪ, 1, 0)
				);
			} ਨਹੀਂਤਾਂ {
				ਕਾਲਮ += ਲਿਖੋ("%9"OFF_FMT"u ", dn->dn_ਮਾਪ);
			}
		}
#ਜੇ ENABLE_FEATURE_LS_TIMESTAMPS
		/* long listing: show {m,c,a}time */
		ਜੇ (opt & OPT_ਭਰਿਆ_ਸਮਾਂ) { /* --full-time */
			/* coreutils 8.4 ls --full-time prints:
			 * 2009-07-13 17:49:27.000000000 +0200
			 * we don't show fractional seconds.
			 */
			ਅੱਖਰ buf[ਆਕਾਰ("YYYY-mm-dd HH:MM:SS TIMEZONE")];
			strftime(buf, ਆਕਾਰ(buf), "%Y-%m-%d %H:%M:%S %z",
					localtime(&dn->dn_ਸਮਾਂ));
			ਕਾਲਮ += ਲਿਖੋ("%s ", buf);
		} ਨਹੀਂਤਾਂ { /* ordinary time format */
			/* G.current_time_t is ~== time(NULL) */
			ਅੱਖਰ *filetime = ctime(&dn->dn_ਸਮਾਂ);
			/* filetime's format: "Wed Jun 30 21:49:08 1993\n" */
			ਸਮਾਂ_t age = G.current_ਸਮਾਂ_t - dn->dn_ਸਮਾਂ;
			ਜੇ (age < 3600L * 24 * 365 / 2 && age > -15 * 60) {
				/* less than 6 months old */
				/* "mmm dd hh:mm " */
				ਲਿਖੋ("%.12s ", filetime + 4);
			} ਨਹੀਂਤਾਂ {
				/* "mmm dd  yyyy " */
				/* "mmm dd yyyyy " after year 9999 :) */
				strchr(filetime + 20, '\n')[0] = ' ';
				ਲਿਖੋ("%.7s%6s", filetime + 4, filetime + 20);
			}
			ਕਾਲਮ += 13;
		}
#ਜੇ_ਅੰਤ
	}

#ਜੇ ENABLE_FEATURE_LS_COLOR
	ਜੇ (G_show_color) {
		ਬਹੁਲਕ_t ਬਹੁਲਕ = dn->dn_ਬਹੁਲਕ_lstat;
		ਜੇ (!ਬਹੁਲਕ)
			ਜੇ (lstat(dn->fullname, &statbuf) == 0)
				ਬਹੁਲਕ = statbuf.st_ਬਹੁਲਕ;
		ਲਿਖੋ(ESC"[%u;%um", bold(ਬਹੁਲਕ), fgcolor(ਬਹੁਲਕ));
	}
#ਜੇ_ਅੰਤ
	ਕਾਲਮ += print_name(dn->name);
	ਜੇ (G_show_color) {
		ਲਿਖੋ(ESC"[m");
	}

	ਜੇ (lpath) {
		ਲਿਖੋ(" -> ");
#ਜੇ ENABLE_FEATURE_LS_FILETYPES || ENABLE_FEATURE_LS_COLOR
		ਜੇ ((opt & (OPT_F|OPT_p))
		 || G_show_color
		) {
			ਬਹੁਲਕ_t ਬਹੁਲਕ = dn->dn_ਬਹੁਲਕ_stat;
			ਜੇ (!ਬਹੁਲਕ)
				ਜੇ (stat(dn->fullname, &statbuf) == 0)
					ਬਹੁਲਕ = statbuf.st_ਬਹੁਲਕ;
# ਜੇ ENABLE_FEATURE_LS_FILETYPES
			append = append_char(ਬਹੁਲਕ);
# endif
# ਜੇ ENABLE_FEATURE_LS_COLOR
			ਜੇ (G_show_color) {
				ਲਿਖੋ(ESC"[%u;%um", bold(ਬਹੁਲਕ), fgcolor(ਬਹੁਲਕ));
			}
# endif
		}
#ਜੇ_ਅੰਤ
		ਕਾਲਮ += print_name(lpath) + 4;
		ਮੁਕਤ(lpath);
		ਜੇ (G_show_color) {
			ਲਿਖੋ(ESC"[m");
		}
	}
#ਜੇ ENABLE_FEATURE_LS_FILETYPES
	ਜੇ (opt & (OPT_F|OPT_p)) {
		ਜੇ (append) {
			ਅੱਖਰ_ਲਿਖੋ(append);
			ਕਾਲਮ++;
		}
	}
#ਜੇ_ਅੰਤ

	ਵਾਪਸ ਕਾਲਮ;
}

ਸਥਿਰ ਖਾਲੀ display_files(ਬਣਤਰ dnode **dn, ਬਿਨਾਨਿਸ਼ਾਨੀ nfiles)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ i, ncols, nrows, ਪੰਕਤੀ, nc;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਕਾਲਮ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ nexttab;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਕਾਲਮ_ਚੌੜਾਈ = 0; /* used only by coulmnal output */

	ਜੇ (option_mask32 & (OPT_l|OPT_1)) {
		ncols = 1;
	} ਨਹੀਂਤਾਂ {
		/* find the longest file name, use that as the column width */
		ਲਈ (i = 0; dn[i]; i++) {
			ਪੂਰਨ len = calc_name_len(dn[i]->name);
			ਜੇ (ਕਾਲਮ_ਚੌੜਾਈ < len)
				ਕਾਲਮ_ਚੌੜਾਈ = len;
		}
		ਕਾਲਮ_ਚੌੜਾਈ += 2
			+ ((option_mask32 & OPT_Z) ? 33 : 0) /* context width */
			+ ((option_mask32 & OPT_i) ? 8 : 0) /* inode# width */
			+ ((option_mask32 & OPT_s) ? 5 : 0) /* "alloc block" width */
		;
		ncols = (ਬਿਨਾਨਿਸ਼ਾਨੀ)G_terminal_ਚੌੜਾਈ / ਕਾਲਮ_ਚੌੜਾਈ;
	}

	ਜੇ (ncols > 1) {
		nrows = nfiles / ncols;
		ਜੇ (nrows * ncols < nfiles)
			nrows++;                /* round up fractionals */
	} ਨਹੀਂਤਾਂ {
		nrows = nfiles;
		ncols = 1;
	}

	ਕਾਲਮ = 0;
	nexttab = 0;
	ਲਈ (ਪੰਕਤੀ = 0; ਪੰਕਤੀ < nrows; ਪੰਕਤੀ++) {
		ਲਈ (nc = 0; nc < ncols; nc++) {
			/* reach into the array based on the column and row */
			ਜੇ (option_mask32 & OPT_x)
				i = (ਪੰਕਤੀ * ncols) + nc;	/* display across row */
			ਨਹੀਂਤਾਂ
				i = (nc * nrows) + ਪੰਕਤੀ;	/* display by column */
			ਜੇ (i < nfiles) {
				ਜੇ (ਕਾਲਮ > 0) {
					nexttab -= ਕਾਲਮ;
					ਲਿਖੋ("%*s", nexttab, "");
					ਕਾਲਮ += nexttab;
				}
				nexttab = ਕਾਲਮ + ਕਾਲਮ_ਚੌੜਾਈ;
				ਕਾਲਮ += display_single(dn[i]);
			}
		}
		ਅੱਖਰ_ਲਿਖੋ('\n');
		ਕਾਲਮ = 0;
	}
}


/*** Dir scanning code ***/

ਸਥਿਰ ਬਣਤਰ dnode *my_stat(ਅਚਲ ਅੱਖਰ *fullname, ਅਚਲ ਅੱਖਰ *name, ਪੂਰਨ force_follow)
{
	ਬਣਤਰ stat statbuf;
	ਬਣਤਰ dnode *cur;

	cur = xzalloc(ਆਕਾਰ(*cur));
	cur->fullname = fullname;
	cur->name = name;

	ਜੇ ((option_mask32 & OPT_L) || force_follow) {
#ਜੇ ENABLE_SELINUX
		ਜੇ (option_mask32 & OPT_Z) {
			getfilecon(fullname, &cur->sid);
		}
#ਜੇ_ਅੰਤ
		ਜੇ (stat(fullname, &statbuf)) {
			bb_simple_perror_ਸੁਨੇਹਾ(fullname);
			G.exit_ਕੋਡ = EXIT_FAILURE;
			ਮੁਕਤ(cur);
			ਵਾਪਸ ਨਿਰਾ;
		}
		cur->dn_ਬਹੁਲਕ_stat = statbuf.st_ਬਹੁਲਕ;
	} ਨਹੀਂਤਾਂ {
#ਜੇ ENABLE_SELINUX
		ਜੇ (option_mask32 & OPT_Z) {
			lgetfilecon(fullname, &cur->sid);
		}
#ਜੇ_ਅੰਤ
		ਜੇ (lstat(fullname, &statbuf)) {
			bb_simple_perror_ਸੁਨੇਹਾ(fullname);
			G.exit_ਕੋਡ = EXIT_FAILURE;
			ਮੁਕਤ(cur);
			ਵਾਪਸ ਨਿਰਾ;
		}
		cur->dn_ਬਹੁਲਕ_lstat = statbuf.st_ਬਹੁਲਕ;
	}

	/* cur->dstat = statbuf: */
	cur->dn_ਬਹੁਲਕ   = statbuf.st_ਬਹੁਲਕ  ;
	cur->dn_ਮਾਪ   = statbuf.st_ਮਾਪ  ;
#ਜੇ ENABLE_FEATURE_LS_TIMESTAMPS || ENABLE_FEATURE_LS_SORTFILES
	cur->dn_ਸਮਾਂ   = statbuf.st_mtime ;
	ਜੇ (option_mask32 & OPT_u)
		cur->dn_ਸਮਾਂ = statbuf.st_atime;
	ਜੇ (option_mask32 & OPT_c)
		cur->dn_ਸਮਾਂ = statbuf.st_ctime;
#ਜੇ_ਅੰਤ
	cur->dn_ino    = statbuf.st_ino   ;
	cur->dn_ਖੰਡਾਂ = statbuf.st_ਖੰਡਾਂ;
	cur->dn_nlink  = statbuf.st_nlink ;
	cur->dn_uid    = statbuf.st_uid   ;
	cur->dn_gid    = statbuf.st_gid   ;
	cur->dn_rdev_maj = major(statbuf.st_rdev);
	cur->dn_rdev_min = minor(statbuf.st_rdev);

	ਵਾਪਸ cur;
}

ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਗਿਣਤੀ_dirs(ਬਣਤਰ dnode **dn, ਪੂਰਨ which)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ dirs, all;

	ਜੇ (!dn)
		ਵਾਪਸ 0;

	dirs = all = 0;
	ਲਈ (; *dn; dn++) {
		ਅਚਲ ਅੱਖਰ *name;

		all++;
		ਜੇ (!S_ISDIR((*dn)->dn_ਬਹੁਲਕ))
			ਜਾਰੀ;

		name = (*dn)->name;
		ਜੇ (which != ਵੰਡੋ_SUBDIR /* if not requested to skip . / .. */
		 /* or if it's not . or .. */
		 || name[0] != '.'
		 || (name[1] && (name[1] != '.' || name[2]))
		) {
			dirs++;
		}
	}
	ਵਾਪਸ which != ਵੰਡੋ_FILE ? dirs : all - dirs;
}

/* get memory to hold an array of pointers */
ਸਥਿਰ ਬਣਤਰ dnode **dnalloc(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ)
{
	ਜੇ (ਅੰਕ < 1)
		ਵਾਪਸ ਨਿਰਾ;

	ਅੰਕ++; /* so that we have terminating NULL */
	ਵਾਪਸ xzalloc(ਅੰਕ * ਆਕਾਰ(ਬਣਤਰ dnode *));
}

#ਜੇ ENABLE_FEATURE_LS_RECURSIVE
ਸਥਿਰ ਖਾਲੀ dfree(ਬਣਤਰ dnode **dnp)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ i;

	ਜੇ (dnp == ਨਿਰਾ)
		ਵਾਪਸ;

	ਲਈ (i = 0; dnp[i]; i++) {
		ਬਣਤਰ dnode *cur = dnp[i];
		ਜੇ (cur->fname_allocated)
			ਮੁਕਤ((ਅੱਖਰ*)cur->fullname);
		ਮੁਕਤ(cur);
	}
	ਮੁਕਤ(dnp);
}
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ dfree(...) ((ਖਾਲੀ)0)
#ਜੇ_ਅੰਤ

/* Returns NULL-terminated malloced vector of pointers (or NULL) */
ਸਥਿਰ ਬਣਤਰ dnode **splitdnarray(ਬਣਤਰ dnode **dn, ਪੂਰਨ which)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ dncnt, d;
	ਬਣਤਰ dnode **dnp;

	ਜੇ (dn == ਨਿਰਾ)
		ਵਾਪਸ ਨਿਰਾ;

	/* count how many dirs or files there are */
	dncnt = ਗਿਣਤੀ_dirs(dn, which);

	/* allocate a file array and a dir array */
	dnp = dnalloc(dncnt);

	/* copy the entrys into the file or dir array */
	ਲਈ (d = 0; *dn; dn++) {
		ਜੇ (S_ISDIR((*dn)->dn_ਬਹੁਲਕ)) {
			ਅਚਲ ਅੱਖਰ *name;

			ਜੇ (which == ਵੰਡੋ_FILE)
				ਜਾਰੀ;

			name = (*dn)->name;
			ਜੇ ((which & ਵੰਡੋ_DIR) /* any dir... */
			/* ... or not . or .. */
			 || name[0] != '.'
			 || (name[1] && (name[1] != '.' || name[2]))
			) {
				dnp[d++] = *dn;
			}
		} ਨਹੀਂਤਾਂ
		ਜੇ (which == ਵੰਡੋ_FILE) {
			dnp[d++] = *dn;
		}
	}
	ਵਾਪਸ dnp;
}

#ਜੇ ENABLE_FEATURE_LS_SORTFILES
ਸਥਿਰ ਪੂਰਨ sortcmp(ਅਚਲ ਖਾਲੀ *a, ਅਚਲ ਖਾਲੀ *b)
{
	ਬਣਤਰ dnode *d1 = *(ਬਣਤਰ dnode **)a;
	ਬਣਤਰ dnode *d2 = *(ਬਣਤਰ dnode **)b;
	ਬਿਨਾਨਿਸ਼ਾਨੀ opt = option_mask32;
	off_t dif;

	dif = 0; /* assume sort by name */
	// TODO: use pre-initialized function pointer
	// instead of branch forest
	ਜੇ (opt & OPT_dirs_ਪਹਿਲਾ) {
		dif = S_ISDIR(d2->dn_ਬਹੁਲਕ) - S_ISDIR(d1->dn_ਬਹੁਲਕ);
		ਜੇ (dif != 0)
			ਜਾਓ maybe_invert_and_ret;
	}

	ਜੇ (opt & OPT_S) { /* sort by size */
		dif = (d2->dn_ਮਾਪ - d1->dn_ਮਾਪ);
	} ਨਹੀਂਤਾਂ
	ਜੇ (opt & OPT_t) { /* sort by time */
		dif = (d2->dn_ਸਮਾਂ - d1->dn_ਸਮਾਂ);
	} ਨਹੀਂਤਾਂ
#ਜੇ defined(HAVE_STRVERSCMP) && HAVE_STRVERSCMP == 1
	ਜੇ (opt & OPT_v) { /* sort by version */
		dif = strverscmp(d1->name, d2->name);
	} ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
	ਜੇ (opt & OPT_X) { /* sort by extension */
		dif = strcmp(strchrnul(d1->name, '.'), strchrnul(d2->name, '.'));
	}
	ਜੇ (dif == 0) {
		/* sort by name, use as tie breaker for other sorts */
		ਜੇ (ENABLE_LOCALE_SUPPORT)
			dif = strcoll(d1->name, d2->name);
		ਨਹੀਂਤਾਂ
			dif = strcmp(d1->name, d2->name);
	} ਨਹੀਂਤਾਂ {
		/* Make dif fit into an int */
		ਜੇ (ਆਕਾਰ(dif) > ਆਕਾਰ(ਪੂਰਨ)) {
			ਗਿਣਤੀ { ਬਿੱਟ_ਮੰਜ਼ਿਲ_ਖਿਸਕਾਓ = 8 * (ਆਕਾਰ(dif) - ਆਕਾਰ(ਪੂਰਨ)) };
			/* shift leaving only "int" worth of bits */
			/* (this requires dif != 0, and here it is nonzero) */
			dif = 1 | (ਪੂਰਨ)((uoff_t)dif >> ਬਿੱਟ_ਮੰਜ਼ਿਲ_ਖਿਸਕਾਓ);
		}
	}
 maybe_invert_and_ret:
	ਵਾਪਸ (opt & OPT_r) ? -(ਪੂਰਨ)dif : (ਪੂਰਨ)dif;
}

ਸਥਿਰ ਖਾਲੀ dnsort(ਬਣਤਰ dnode **dn, ਪੂਰਨ ਮਾਪ)
{
	qsort(dn, ਮਾਪ, ਆਕਾਰ(*dn), sortcmp);
}

ਸਥਿਰ ਖਾਲੀ ਛਾਂਟੋ_and_display_files(ਬਣਤਰ dnode **dn, ਬਿਨਾਨਿਸ਼ਾਨੀ nfiles)
{
	dnsort(dn, nfiles);
	display_files(dn, nfiles);
}
#ਨਹੀਂਤਾਂ
# define dnsort(dn, ਮਾਪ) ((ਖਾਲੀ)0)
# define ਛਾਂਟੋ_and_display_files(dn, nfiles) display_files(dn, nfiles)
#ਜੇ_ਅੰਤ

/* Returns NULL-terminated malloced vector of pointers (or NULL) */
ਸਥਿਰ ਬਣਤਰ dnode **ਜਾਂਚ_one_dir(ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ਬਿਨਾਨਿਸ਼ਾਨੀ *nfiles_p)
{
	ਬਣਤਰ dnode *dn, *cur, **dnp;
	ਬਣਤਰ dirent *ਇੰਦਰਾਜ;
	DIR *dir;
	ਬਿਨਾਨਿਸ਼ਾਨੀ i, nfiles;

	*nfiles_p = 0;
	dir = warn_opendir(ਰਸਤਾ);
	ਜੇ (dir == ਨਿਰਾ) {
		G.exit_ਕੋਡ = EXIT_FAILURE;
		ਵਾਪਸ ਨਿਰਾ;	/* could not open the dir */
	}
	dn = ਨਿਰਾ;
	nfiles = 0;
	ਜਦੋਂ ((ਇੰਦਰਾਜ = readdir(dir)) != ਨਿਰਾ) {
		ਅੱਖਰ *fullname;

		/* are we going to list the file- it may be . or .. or a hidden file */
		ਜੇ (ਇੰਦਰਾਜ->d_name[0] == '.') {
			ਜੇ (!(option_mask32 & (OPT_a|OPT_A)))
				ਜਾਰੀ; /* skip all dotfiles if no -a/-A */
			ਜੇ (!(option_mask32 & OPT_a)
			 && (!ਇੰਦਰਾਜ->d_name[1] || (ਇੰਦਰਾਜ->d_name[1] == '.' && !ਇੰਦਰਾਜ->d_name[2]))
			) {
				ਜਾਰੀ; /* if only -A, skip . and .. but show other dotfiles */
			}
		}
		fullname = concat_ਰਸਤਾ_file(ਰਸਤਾ, ਇੰਦਰਾਜ->d_name);
		cur = my_stat(fullname, bb_basename(fullname), 0);
		ਜੇ (!cur) {
			ਮੁਕਤ(fullname);
			ਜਾਰੀ;
		}
		cur->fname_allocated = 1;
		cur->dn_ਅਗਲਾ = dn;
		dn = cur;
		nfiles++;
	}
	closedir(dir);

	ਜੇ (dn == ਨਿਰਾ)
		ਵਾਪਸ ਨਿਰਾ;

	/* now that we know how many files there are
	 * allocate memory for an array to hold dnode pointers
	 */
	*nfiles_p = nfiles;
	dnp = dnalloc(nfiles);
	ਲਈ (i = 0; /* i < nfiles - detected via !dn below */; i++) {
		dnp[i] = dn;	/* save pointer to node in array */
		dn = dn->dn_ਅਗਲਾ;
		ਜੇ (!dn)
			ਰੋਕੋ;
	}

	ਵਾਪਸ dnp;
}

#ਜੇ ENABLE_DESKTOP
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
ਸਥਿਰ off_t calculate_ਖੰਡਾਂ(ਬਣਤਰ dnode **dn)
{
	uoff_t ਖੰਡਾਂ = 1;
	ਜੇ (dn) {
		ਜਦੋਂ (*dn) {
			/* st_blocks is in 512 byte blocks */
			ਖੰਡਾਂ += (*dn)->dn_ਖੰਡਾਂ;
			dn++;
		}
	}

	/* Even though standard says use 512 byte blocks, coreutils use 1k */
	/* Actually, we round up by calculating (blocks + 1) / 2,
	 * "+ 1" was done when we initialized blocks to 1 */
	ਵਾਪਸ ਖੰਡਾਂ >> 1;
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਖਾਲੀ ਜਾਂਚ_and_display_dirs_recur(ਬਣਤਰ dnode **dn, ਪੂਰਨ ਪਹਿਲਾ)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ nfiles;
	ਬਣਤਰ dnode **subdnp;

	ਲਈ (; *dn; dn++) {
		ਜੇ (G.show_dirname || (option_mask32 & OPT_R)) {
			ਜੇ (!ਪਹਿਲਾ)
				bb_putchar('\n');
			ਪਹਿਲਾ = 0;
			ਲਿਖੋ("%s:\n", (*dn)->fullname);
		}
		subdnp = ਜਾਂਚ_one_dir((*dn)->fullname, &nfiles);
#ਜੇ ENABLE_DESKTOP
		ਜੇ (option_mask32 & (OPT_s|OPT_l)) {
			ਜੇ (option_mask32 & OPT_h) {
				ਲਿਖੋ("total %-"HUMAN_READABLE_MAX_ਚੌੜਾਈ_STR"s\n",
					/* print size, no fractions, use suffixes */
					make_human_readable_str(calculate_ਖੰਡਾਂ(subdnp) * 1024,
								0, 0)
				);
			} ਨਹੀਂਤਾਂ {
				ਲਿਖੋ("total %"OFF_FMT"u\n", calculate_ਖੰਡਾਂ(subdnp));
			}
		}
#ਜੇ_ਅੰਤ
		ਜੇ (nfiles > 0) {
			/* list all files at this level */
			ਛਾਂਟੋ_and_display_files(subdnp, nfiles);

			ਜੇ (ENABLE_FEATURE_LS_RECURSIVE
			 && (option_mask32 & OPT_R)
			) {
				ਬਣਤਰ dnode **dnd;
				ਬਿਨਾਨਿਸ਼ਾਨੀ dndirs;
				/* recursive - list the sub-dirs */
				dnd = splitdnarray(subdnp, ਵੰਡੋ_SUBDIR);
				dndirs = ਗਿਣਤੀ_dirs(subdnp, ਵੰਡੋ_SUBDIR);
				ਜੇ (dndirs > 0) {
					dnsort(dnd, dndirs);
					ਜਾਂਚ_and_display_dirs_recur(dnd, 0);
					/* free the array of dnode pointers to the dirs */
					ਮੁਕਤ(dnd);
				}
			}
			/* free the dnodes and the fullname mem */
			dfree(subdnp);
		}
	}
}


ਪੂਰਨ ls_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{	/*      ^^^^^^^^^^^^^^^^^ note: if FTPD, argc can be wrong, see ftpd.c */
	ਬਣਤਰ dnode **dnd;
	ਬਣਤਰ dnode **dnf;
	ਬਣਤਰ dnode **dnp;
	ਬਣਤਰ dnode *dn;
	ਬਣਤਰ dnode *cur;
	ਬਿਨਾਨਿਸ਼ਾਨੀ opt;
	ਬਿਨਾਨਿਸ਼ਾਨੀ nfiles;
	ਬਿਨਾਨਿਸ਼ਾਨੀ dnfiles;
	ਬਿਨਾਨਿਸ਼ਾਨੀ dndirs;
	ਬਿਨਾਨਿਸ਼ਾਨੀ i;
#ਜੇ ENABLE_FEATURE_LS_COLOR
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
	ਸਥਿਰ ਅਚਲ ਅੱਖਰ color_str[] ALIGN1 =
		"always\0""yes\0""force\0"
		"auto\0""tty\0""if-tty\0";
	/* need to initialize since --color has _an optional_ argument */
	ਅਚਲ ਅੱਖਰ *color_opt = color_str; /* "always" */
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_LONG_OPTS
	ਸਥਿਰ ਅਚਲ ਅੱਖਰ ls_longopts[] ALIGN1 =
		"full-time\0" No_ਦਲੀਲ "\xff"
		"group-directories-first\0" No_ਦਲੀਲ "\xfe"
		IF_FEATURE_LS_COLOR("color\0" Optional_ਦਲੀਲ "\xfd")
	;
#ਜੇ_ਅੰਤ

	INIT_G();

	init_unicode();

#ਜੇ ENABLE_FEATURE_LS_ਚੌੜਾਈ
	/* obtain the terminal width */
	G_terminal_ਚੌੜਾਈ = get_terminal_ਚੌੜਾਈ(STDIN_FILENO);
	/* go one less... */
	G_terminal_ਚੌੜਾਈ--;
#ਜੇ_ਅੰਤ

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
			IF_FEATURE_LS_ਚੌੜਾਈ(":w+")
		, ls_longopts
		IF_FEATURE_LS_ਚੌੜਾਈ(, /*-T*/ ਨਿਰਾ, /*-w*/ &G_terminal_ਚੌੜਾਈ)
		IF_FEATURE_LS_COLOR(, &color_opt)
	);
#ਜੇ 0 /* option bits debug */
	bb_error_ਸੁਨੇਹਾ("opt:0x%08x l:%x H:%x color:%x dirs:%x", opt, OPT_l, OPT_H, OPT_color, OPT_dirs_ਪਹਿਲਾ);
	ਜੇ (opt & OPT_c         ) bb_error_ਸੁਨੇਹਾ("-c");
	ਜੇ (opt & OPT_l         ) bb_error_ਸੁਨੇਹਾ("-l");
	ਜੇ (opt & OPT_H         ) bb_error_ਸੁਨੇਹਾ("-H");
	ਜੇ (opt & OPT_color     ) bb_error_ਸੁਨੇਹਾ("--color");
	ਜੇ (opt & OPT_dirs_ਪਹਿਲਾ) bb_error_ਸੁਨੇਹਾ("--group-directories-first");
	ਜੇ (opt & OPT_ਭਰਿਆ_ਸਮਾਂ ) bb_error_ਸੁਨੇਹਾ("--full-time");
	ਬਾਹਰ_ਨਿਕਲੋ(0);
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_SELINUX
	ਜੇ (opt & OPT_Z) {
		ਜੇ (!is_selinux_enabled())
			option_mask32 &= ~OPT_Z;
	}
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_FEATURE_LS_COLOR
	/* set G_show_color = 1/0 */
	ਜੇ (ENABLE_FEATURE_LS_COLOR_IS_DEFAULT && !is_ਪਦ_dumb()) {
		ਅੱਖਰ *p = getenv("LS_COLORS");
		/* LS_COLORS is unset, or (not empty && not "none") ? */
		ਜੇ (!p || (p[0] && strcmp(p, "none") != 0)) {
			ਜੇ (isatty(STDOUT_FILENO)) {
				/* check isatty() last because it's expensive (syscall) */
				G_show_color = 1;
			}
		}
	}
	ਜੇ (opt & OPT_color) {
		ਜੇ (color_opt[0] == 'n')
			G_show_color = 0;
		ਨਹੀਂਤਾਂ ਚੋਣ (ਸੂਚਕਾਂਕ_ਵਿੱਚ_substrings(color_str, color_opt)) {
		ਹਾਲਤ 3:
		ਹਾਲਤ 4:
		ਹਾਲਤ 5:
			ਜੇ (!is_ਪਦ_dumb() && isatty(STDOUT_FILENO)) {
		ਹਾਲਤ 0:
		ਹਾਲਤ 1:
		ਹਾਲਤ 2:
				G_show_color = 1;
			}
		}
	}
#ਜੇ_ਅੰਤ

	/* sort out which command line options take precedence */
	ਜੇ (ENABLE_FEATURE_LS_RECURSIVE && (opt & OPT_d))
		option_mask32 &= ~OPT_R;	/* no recurse if listing only dir */
	ਜੇ (!(opt & OPT_l)) { /* not -l? */
		ਜੇ (ENABLE_FEATURE_LS_TIMESTAMPS && ENABLE_FEATURE_LS_SORTFILES) {
			/* when to sort by time? -t[cu] sorts by time even with -l */
			/* (this is achieved by opt_flags[] element for -t) */
			/* without -l, bare -c or -u enable sort too */
			/* (with -l, bare -c or -u just select which time to show) */
			ਜੇ (opt & (OPT_c|OPT_u)) {
				option_mask32 |= OPT_t;
			}
		}
	}

	/* choose a display format if one was not already specified by an option */
	ਜੇ (!(option_mask32 & (OPT_l|OPT_1|OPT_x|OPT_C)))
		option_mask32 |= (isatty(STDOUT_FILENO) ? OPT_C : OPT_1);

	ਜੇ (ENABLE_FTPD && applet_name[0] == 'f') {
		/* ftpd secret backdoor. dirs first are much nicer */
		option_mask32 |= OPT_dirs_ਪਹਿਲਾ;
	}

	argv += optind;
	ਜੇ (!argv[0])
		*--argv = (ਅੱਖਰ*)".";

	ਜੇ (argv[1])
		G.show_dirname = 1; /* 2 or more items? label directories */

	/* stuff the command line file names into a dnode array */
	dn = ਨਿਰਾ;
	nfiles = 0;
	ਕਰੋ {
		cur = my_stat(*argv, *argv,
			/* follow links on command line unless -l, -i, -s or -F: */
			!(option_mask32 & (OPT_l|OPT_i|OPT_s|OPT_F))
			/* ... or if -H: */
			|| (option_mask32 & OPT_H)
			/* ... or if -L, but my_stat always follows links if -L */
		);
		argv++;
		ਜੇ (!cur)
			ਜਾਰੀ;
		/*cur->fname_allocated = 0; - already is */
		cur->dn_ਅਗਲਾ = dn;
		dn = cur;
		nfiles++;
	} ਜਦੋਂ (*argv);

	/* nfiles _may_ be 0 here - try "ls doesnt_exist" */
	ਜੇ (nfiles == 0)
		ਵਾਪਸ G.exit_ਕੋਡ;

	/* now that we know how many files there are
	 * allocate memory for an array to hold dnode pointers
	 */
	dnp = dnalloc(nfiles);
	ਲਈ (i = 0; /* i < nfiles - detected via !dn below */; i++) {
		dnp[i] = dn;	/* save pointer to node in array */
		dn = dn->dn_ਅਗਲਾ;
		ਜੇ (!dn)
			ਰੋਕੋ;
	}

	ਜੇ (option_mask32 & OPT_d) {
		ਛਾਂਟੋ_and_display_files(dnp, nfiles);
	} ਨਹੀਂਤਾਂ {
		dnd = splitdnarray(dnp, ਵੰਡੋ_DIR);
		dnf = splitdnarray(dnp, ਵੰਡੋ_FILE);
		dndirs = ਗਿਣਤੀ_dirs(dnp, ਵੰਡੋ_DIR);
		dnfiles = nfiles - dndirs;
		ਜੇ (dnfiles > 0) {
			ਛਾਂਟੋ_and_display_files(dnf, dnfiles);
			ਜੇ (ENABLE_FEATURE_CLEAN_ਉੱਪਰ)
				ਮੁਕਤ(dnf);
		}
		ਜੇ (dndirs > 0) {
			dnsort(dnd, dndirs);
			ਜਾਂਚ_and_display_dirs_recur(dnd, dnfiles == 0);
			ਜੇ (ENABLE_FEATURE_CLEAN_ਉੱਪਰ)
				ਮੁਕਤ(dnd);
		}
	}

	ਜੇ (ENABLE_FEATURE_CLEAN_ਉੱਪਰ)
		dfree(dnp);
	ਵਾਪਸ G.exit_ਕੋਡ;
}
