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

#ضمن "libbb.h"
#ضمن "common_bufsiz.h"
#ضمن "unicode.h"


/* This is a NOEXEC applet. Be very careful! */


#إذا ENABLE_FTPD
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
#د_که_پای


تعداد {
TERMINAL_پلنوالی  = 80, /* use 79 if terminal has linefold bug */

وېشل_FILE      = 0,
وېشل_DIR       = 1,
وېشل_SUBDIR    = 2,
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
#تعریف_کړه ls_options \
	"Cadi1lgnsxAk"       /* 12 opts, total 12 */ \
	IF_FEATURE_LS_FILETYPES("Fp")    /* 2, 14 */ \
	IF_FEATURE_LS_RECURSIVE("R")     /* 1, 15 */ \
	IF_SELINUX("Z")                  /* 1, 16 */ \
	"Q"                              /* 1, 17 */ \
	IF_FEATURE_LS_TIMESTAMPS("ctu")  /* 3, 20 */ \
	IF_FEATURE_LS_SORTFILES("SXrv")  /* 4, 24 */ \
	IF_FEATURE_LS_FOLLOWLINKS("LH")  /* 2, 26 */ \
	IF_FEATURE_HUMAN_READABLE("h")   /* 1, 27 */ \
	IF_FEATURE_LS_پلنوالی("T:w:")      /* 2, 29 */

تعداد {
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
	OPTBIT_ډک_وخت = OPTBIT_T + 2 * ENABLE_FEATURE_LS_پلنوالی,
	OPTBIT_dirs_لومړی,
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
	OPT_T = (1 << OPTBIT_T) * ENABLE_FEATURE_LS_پلنوالی,
	OPT_w = (1 << OPTBIT_w) * ENABLE_FEATURE_LS_پلنوالی,
	OPT_ډک_وخت  = (1 << OPTBIT_ډک_وخت ) * ENABLE_LONG_OPTS,
	OPT_dirs_لومړی = (1 << OPTBIT_dirs_لومړی) * ENABLE_LONG_OPTS,
	OPT_color      = (1 << OPTBIT_color     ) * ENABLE_FEATURE_LS_COLOR,
};

/*
 * a directory entry and its stat info
 */
بنية dnode {
	ثابت حرف *name;       /* usually basename, but think "ls -l dir/file" */
	ثابت حرف *fullname;   /* full name (usable for stat etc) */
	بنية dnode *dn_بل;  /* for linked list */
	IF_SELINUX(security_context_t sid;)
	smallint fname_allocated;

	/* Used to avoid re-doing [l]stat at printout stage
	 * if we already collected needed data in scan stage:
	 */
	ډېرپېښ_t    dn_ډېرپېښ_lstat;   /* obtained with lstat, or 0 */
	ډېرپېښ_t    dn_ډېرپېښ_stat;    /* obtained with stat, or 0 */

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
	ډېرپېښ_t    dn_ډېرپېښ; /* obtained with lstat OR stat, depending on -L etc */
	off_t     dn_اندازه;
#إذا ENABLE_FEATURE_LS_TIMESTAMPS || ENABLE_FEATURE_LS_SORTFILES
	وخت_t    dn_وخت;
#د_که_پای
	ino_t     dn_ino;
	blkcnt_t  dn_بلاکونه;
	nlink_t   dn_nlink;
	uid_t     dn_uid;
	gid_t     dn_gid;
	صحيح       dn_rdev_maj;
	صحيح       dn_rdev_min;
//	dev_t     dn_dev;
//	blksize_t dn_blksize;
};

بنية globals {
#إذا ENABLE_FEATURE_LS_COLOR
	smallint show_color;
# define G_show_color (G.show_color)
#وإلا
# define G_show_color 0
#د_که_پای
	smallint exit_کوډ;
	smallint show_dirname;
#إذا ENABLE_FEATURE_LS_پلنوالی
	بینښه terminal_پلنوالی;
# define G_terminal_پلنوالی (G.terminal_پلنوالی)
#وإلا
# define G_terminal_پلنوالی TERMINAL_پلنوالی
#د_که_پای
#إذا ENABLE_FEATURE_LS_TIMESTAMPS
	/* Do time() just once. Saves one syscall per file for "ls -l" */
	وخت_t current_وخت_t;
#د_که_پای
} FIX_ALIASING;
#تعریف_کړه G (*(بنية globals*)bb_common_bufsiz1)
#تعریف_کړه INIT_G() افعل { \
	تیاری_common_bufsiz(); \
	/* we have to zero it out because of NOEXEC */ \
	memset(&G, 0, حجم(G)); \
	IF_FEATURE_LS_پلنوالی(G_terminal_پلنوالی = TERMINAL_پلنوالی;) \
	IF_FEATURE_LS_TIMESTAMPS(وخت(&G.current_وخت_t);) \
} طالما (0)

#تعریف_کړه ESC "\033"


/*** Output code ***/


/* FYI type values: 1:fifo 2:char 4:dir 6:blk 8:file 10:link 12:socket
 * (various wacky OSes: 13:Sun door 14:BSD whiteout 5:XENIX named file
 *  3/7:multiplexed char/block device)
 * and we use 0 for unknown and 15 for executables (see below) */
#تعریف_کړه TYPEINDEX(ډېرپېښ) (((ډېرپېښ) >> 12) & 0x0f)
/*                       un  fi chr -   dir -  blk  -  file -  link - sock -   - exe */
#تعریف_کړه APPCHAR(ډېرپېښ)   ("\0""|""\0""\0""/""\0""\0""\0""\0""\0""@""\0""=""\0""\0""\0" [TYPEINDEX(ډېرپېښ)])
/* 036 black foreground              050 black background
   037 red foreground                051 red background
   040 green foreground              052 green background
   041 brown foreground              053 brown background
   042 blue foreground               054 blue background
   043 magenta (purple) foreground   055 magenta background
   044 cyan (light blue) foreground  056 cyan background
   045 gray foreground               057 white background
*/
#تعریف_کړه COLOR(ډېرپېښ) ( \
	/*un  fi  chr  -  dir  -  blk  -  file -  link -  sock -   -  exe */ \
	"\037\043\043\045\042\045\043\043\000\045\044\045\043\045\045\040" \
	[TYPEINDEX(ډېرپېښ)])
/* Select normal (0) [actually "reset all"] or bold (1)
 * (other attributes are 2:dim 4:underline 5:blink 7:reverse,
 *  let's use 7 for "impossible" types, just for fun)
 * Note: coreutils 6.9 uses inverted red for setuid binaries.
 */
#تعریف_کړه ATTR(ډېرپېښ) ( \
	/*un fi chr - dir - blk - file- link- sock- -  exe */ \
	"\01\00\01\07\01\07\01\07\00\07\01\07\01\07\07\01" \
	[TYPEINDEX(ډېرپېښ)])

#إذا ENABLE_FEATURE_LS_COLOR
/* mode of zero is interpreted as "unknown" (stat failed) */
ساكن حرف fgcolor(ډېرپېښ_t ډېرپېښ)
{
	إذا (S_ISREG(ډېرپېښ) && (ډېرپېښ & (S_IXUSR | S_IXGRP | S_IXOTH)))
		أرجع COLOR(0xF000);	/* File is executable ... */
	أرجع COLOR(ډېرپېښ);
}
ساكن حرف bold(ډېرپېښ_t ډېرپېښ)
{
	إذا (S_ISREG(ډېرپېښ) && (ډېرپېښ & (S_IXUSR | S_IXGRP | S_IXOTH)))
		أرجع ATTR(0xF000);	/* File is executable ... */
	أرجع ATTR(ډېرپېښ);
}
#د_که_پای

#إذا ENABLE_FEATURE_LS_FILETYPES
ساكن حرف append_char(ډېرپېښ_t ډېرپېښ)
{
	إذا (!(option_mask32 & (OPT_F|OPT_p)))
		أرجع '\0';

	إذا (S_ISDIR(ډېرپېښ))
		أرجع '/';
	إذا (!(option_mask32 & OPT_F))
		أرجع '\0';
	إذا (S_ISREG(ډېرپېښ) && (ډېرپېښ & (S_IXUSR | S_IXGRP | S_IXOTH)))
		أرجع '*';
	أرجع APPCHAR(ډېرپېښ);
}
#د_که_پای

ساكن بینښه calc_name_len(ثابت حرف *name)
{
	بینښه len;
	uni_stat_t uni_stat;

	// TODO: quote tab as \t, etc, if -Q
	name = printable_string2(&uni_stat, name);

	إذا (!(option_mask32 & OPT_Q)) {
		أرجع uni_stat.unicode_پلنوالی;
	}

	len = 2 + uni_stat.unicode_پلنوالی;
	طالما (*name) {
		إذا (*name == '"' || *name == '\\') {
			len++;
		}
		name++;
	}
	أرجع len;
}

/* Return the number of used columns.
 * Note that only columnar output uses return value.
 * -l and -1 modes don't care.
 * coreutils 7.2 also supports:
 * ls -b (--escape) = octal escapes (although it doesn't look like working)
 * ls -N (--literal) = not escape at all
 */
ساكن بینښه print_name(ثابت حرف *name)
{
	بینښه len;
	uni_stat_t uni_stat;

	// TODO: quote tab as \t, etc, if -Q
	name = printable_string2(&uni_stat, name);

	إذا (!(option_mask32 & OPT_Q)) {
		fputs_stdout(name);
		أرجع uni_stat.unicode_پلنوالی;
	}

	len = 2 + uni_stat.unicode_پلنوالی;
	توری_چاپ_کړه('"');
	طالما (*name) {
		إذا (*name == '"' || *name == '\\') {
			توری_چاپ_کړه('\\');
			len++;
		}
		توری_چاپ_کړه(*name);
		name++;
	}
	توری_چاپ_کړه('"');
	أرجع len;
}

/* Return the number of used columns.
 * Note that only columnar output uses return value,
 * -l and -1 modes don't care.
 */
ساكن NOINLINE بینښه display_single(ثابت بنية dnode *dn)
{
	بینښه ستنه = 0;
	حرف *lpath;
	صحيح opt;
#إذا ENABLE_FEATURE_LS_FILETYPES || ENABLE_FEATURE_LS_COLOR
	بنية stat statbuf;
#د_که_پای
#إذا ENABLE_FEATURE_LS_FILETYPES
	حرف append = append_char(dn->dn_ډېرپېښ);
#د_که_پای

	opt = option_mask32;

	/* Do readlink early, so that if it fails, error message
	 * does not appear *inside* the "ls -l" line */
	lpath = عدم;
	إذا (opt & OPT_l)
		إذا (S_ISLNK(dn->dn_ډېرپېښ))
			lpath = xmalloc_readlink_or_warn(dn->fullname);

	إذا (opt & OPT_i) /* show inode# */
		ستنه += چاپ_کړه("%7llu ", (اوږد اوږد) dn->dn_ino);
	إذا (opt & OPT_s) { /* show allocated blocks */
		إذا (opt & OPT_h) {
			ستنه += چاپ_کړه("%"HUMAN_READABLE_MAX_پلنوالی_STR"s ",
				/* print size, show one fractional, use suffixes */
				make_human_readable_str((off_t)dn->dn_بلاکونه << 9, 1, 0)
			);
		} وإلا {
			ستنه += چاپ_کړه("%6"OFF_FMT"u ", (off_t)(dn->dn_بلاکونه >> 1));
		}
	}
	إذا (opt & OPT_l) {
		/* long listing: show mode */
		حرف modestr[12];
		ستنه += چاپ_کړه("%-10s ", bb_ډېرپېښ_string(modestr, dn->dn_ډېرپېښ));
		/* long listing: show number of links */
		ستنه += چاپ_کړه("%4lu ", (اوږد) dn->dn_nlink);
		/* long listing: show user/group */
		إذا (opt & OPT_n) {
			إذا (opt & OPT_g)
				ستنه += چاپ_کړه("%-8u ", (صحيح) dn->dn_gid);
			وإلا
				ستنه += چاپ_کړه("%-8u %-8u ",
						(صحيح) dn->dn_uid,
						(صحيح) dn->dn_gid);
		}
#إذا ENABLE_FEATURE_LS_USERNAME
		وإلا {
			إذا (opt & OPT_g) {
				ستنه += چاپ_کړه("%-8s ",
					get_cached_groupname(dn->dn_gid));
			} وإلا {
				ستنه += چاپ_کړه("%-8s %-8s ",
					get_cached_username(dn->dn_uid),
					get_cached_groupname(dn->dn_gid));
			}
		}
#د_که_پای
#إذا ENABLE_SELINUX
	}
	إذا (opt & OPT_Z) {
		ستنه += چاپ_کړه("%-32s ", dn->sid ? dn->sid : "?");
		freecon(dn->sid);
	}
	إذا (opt & OPT_l) {
#د_که_پای
		/* long listing: show size */
		إذا (S_ISBLK(dn->dn_ډېرپېښ) || S_ISCHR(dn->dn_ډېرپېښ)) {
			ستنه += چاپ_کړه("%4u, %3u ",
					dn->dn_rdev_maj,
					dn->dn_rdev_min);
		} وإلا {
			إذا (opt & OPT_h) {
				ستنه += چاپ_کړه("%"HUMAN_READABLE_MAX_پلنوالی_STR"s ",
					/* print size, show one fractional, use suffixes */
					make_human_readable_str(dn->dn_اندازه, 1, 0)
				);
			} وإلا {
				ستنه += چاپ_کړه("%9"OFF_FMT"u ", dn->dn_اندازه);
			}
		}
#إذا ENABLE_FEATURE_LS_TIMESTAMPS
		/* long listing: show {m,c,a}time */
		إذا (opt & OPT_ډک_وخت) { /* --full-time */
			/* coreutils 8.4 ls --full-time prints:
			 * 2009-07-13 17:49:27.000000000 +0200
			 * we don't show fractional seconds.
			 */
			حرف buf[حجم("YYYY-mm-dd HH:MM:SS TIMEZONE")];
			strftime(buf, حجم(buf), "%Y-%m-%d %H:%M:%S %z",
					localtime(&dn->dn_وخت));
			ستنه += چاپ_کړه("%s ", buf);
		} وإلا { /* ordinary time format */
			/* G.current_time_t is ~== time(NULL) */
			حرف *filetime = ctime(&dn->dn_وخت);
			/* filetime's format: "Wed Jun 30 21:49:08 1993\n" */
			وخت_t age = G.current_وخت_t - dn->dn_وخت;
			إذا (age < 3600L * 24 * 365 / 2 && age > -15 * 60) {
				/* less than 6 months old */
				/* "mmm dd hh:mm " */
				چاپ_کړه("%.12s ", filetime + 4);
			} وإلا {
				/* "mmm dd  yyyy " */
				/* "mmm dd yyyyy " after year 9999 :) */
				strchr(filetime + 20, '\n')[0] = ' ';
				چاپ_کړه("%.7s%6s", filetime + 4, filetime + 20);
			}
			ستنه += 13;
		}
#د_که_پای
	}

#إذا ENABLE_FEATURE_LS_COLOR
	إذا (G_show_color) {
		ډېرپېښ_t ډېرپېښ = dn->dn_ډېرپېښ_lstat;
		إذا (!ډېرپېښ)
			إذا (lstat(dn->fullname, &statbuf) == 0)
				ډېرپېښ = statbuf.st_ډېرپېښ;
		چاپ_کړه(ESC"[%u;%um", bold(ډېرپېښ), fgcolor(ډېرپېښ));
	}
#د_که_پای
	ستنه += print_name(dn->name);
	إذا (G_show_color) {
		چاپ_کړه(ESC"[m");
	}

	إذا (lpath) {
		چاپ_کړه(" -> ");
#إذا ENABLE_FEATURE_LS_FILETYPES || ENABLE_FEATURE_LS_COLOR
		إذا ((opt & (OPT_F|OPT_p))
		 || G_show_color
		) {
			ډېرپېښ_t ډېرپېښ = dn->dn_ډېرپېښ_stat;
			إذا (!ډېرپېښ)
				إذا (stat(dn->fullname, &statbuf) == 0)
					ډېرپېښ = statbuf.st_ډېرپېښ;
# إذا ENABLE_FEATURE_LS_FILETYPES
			append = append_char(ډېرپېښ);
# endif
# إذا ENABLE_FEATURE_LS_COLOR
			إذا (G_show_color) {
				چاپ_کړه(ESC"[%u;%um", bold(ډېرپېښ), fgcolor(ډېرپېښ));
			}
# endif
		}
#د_که_پای
		ستنه += print_name(lpath) + 4;
		حرر(lpath);
		إذا (G_show_color) {
			چاپ_کړه(ESC"[m");
		}
	}
#إذا ENABLE_FEATURE_LS_FILETYPES
	إذا (opt & (OPT_F|OPT_p)) {
		إذا (append) {
			توری_چاپ_کړه(append);
			ستنه++;
		}
	}
#د_که_پای

	أرجع ستنه;
}

ساكن فراغ display_files(بنية dnode **dn, بینښه nfiles)
{
	بینښه i, ncols, nrows, لیکه, nc;
	بینښه ستنه;
	بینښه nexttab;
	بینښه ستنه_پلنوالی = 0; /* used only by coulmnal output */

	إذا (option_mask32 & (OPT_l|OPT_1)) {
		ncols = 1;
	} وإلا {
		/* find the longest file name, use that as the column width */
		لكل (i = 0; dn[i]; i++) {
			صحيح len = calc_name_len(dn[i]->name);
			إذا (ستنه_پلنوالی < len)
				ستنه_پلنوالی = len;
		}
		ستنه_پلنوالی += 2
			+ ((option_mask32 & OPT_Z) ? 33 : 0) /* context width */
			+ ((option_mask32 & OPT_i) ? 8 : 0) /* inode# width */
			+ ((option_mask32 & OPT_s) ? 5 : 0) /* "alloc block" width */
		;
		ncols = (بینښه)G_terminal_پلنوالی / ستنه_پلنوالی;
	}

	إذا (ncols > 1) {
		nrows = nfiles / ncols;
		إذا (nrows * ncols < nfiles)
			nrows++;                /* round up fractionals */
	} وإلا {
		nrows = nfiles;
		ncols = 1;
	}

	ستنه = 0;
	nexttab = 0;
	لكل (لیکه = 0; لیکه < nrows; لیکه++) {
		لكل (nc = 0; nc < ncols; nc++) {
			/* reach into the array based on the column and row */
			إذا (option_mask32 & OPT_x)
				i = (لیکه * ncols) + nc;	/* display across row */
			وإلا
				i = (nc * nrows) + لیکه;	/* display by column */
			إذا (i < nfiles) {
				إذا (ستنه > 0) {
					nexttab -= ستنه;
					چاپ_کړه("%*s", nexttab, "");
					ستنه += nexttab;
				}
				nexttab = ستنه + ستنه_پلنوالی;
				ستنه += display_single(dn[i]);
			}
		}
		توری_چاپ_کړه('\n');
		ستنه = 0;
	}
}


/*** Dir scanning code ***/

ساكن بنية dnode *my_stat(ثابت حرف *fullname, ثابت حرف *name, صحيح force_follow)
{
	بنية stat statbuf;
	بنية dnode *cur;

	cur = xzalloc(حجم(*cur));
	cur->fullname = fullname;
	cur->name = name;

	إذا ((option_mask32 & OPT_L) || force_follow) {
#إذا ENABLE_SELINUX
		إذا (option_mask32 & OPT_Z) {
			getfilecon(fullname, &cur->sid);
		}
#د_که_پای
		إذا (stat(fullname, &statbuf)) {
			bb_simple_perror_پیغام(fullname);
			G.exit_کوډ = EXIT_FAILURE;
			حرر(cur);
			أرجع عدم;
		}
		cur->dn_ډېرپېښ_stat = statbuf.st_ډېرپېښ;
	} وإلا {
#إذا ENABLE_SELINUX
		إذا (option_mask32 & OPT_Z) {
			lgetfilecon(fullname, &cur->sid);
		}
#د_که_پای
		إذا (lstat(fullname, &statbuf)) {
			bb_simple_perror_پیغام(fullname);
			G.exit_کوډ = EXIT_FAILURE;
			حرر(cur);
			أرجع عدم;
		}
		cur->dn_ډېرپېښ_lstat = statbuf.st_ډېرپېښ;
	}

	/* cur->dstat = statbuf: */
	cur->dn_ډېرپېښ   = statbuf.st_ډېرپېښ  ;
	cur->dn_اندازه   = statbuf.st_اندازه  ;
#إذا ENABLE_FEATURE_LS_TIMESTAMPS || ENABLE_FEATURE_LS_SORTFILES
	cur->dn_وخت   = statbuf.st_mtime ;
	إذا (option_mask32 & OPT_u)
		cur->dn_وخت = statbuf.st_atime;
	إذا (option_mask32 & OPT_c)
		cur->dn_وخت = statbuf.st_ctime;
#د_که_پای
	cur->dn_ino    = statbuf.st_ino   ;
	cur->dn_بلاکونه = statbuf.st_بلاکونه;
	cur->dn_nlink  = statbuf.st_nlink ;
	cur->dn_uid    = statbuf.st_uid   ;
	cur->dn_gid    = statbuf.st_gid   ;
	cur->dn_rdev_maj = major(statbuf.st_rdev);
	cur->dn_rdev_min = minor(statbuf.st_rdev);

	أرجع cur;
}

ساكن بینښه شمېر_dirs(بنية dnode **dn, صحيح which)
{
	بینښه dirs, all;

	إذا (!dn)
		أرجع 0;

	dirs = all = 0;
	لكل (; *dn; dn++) {
		ثابت حرف *name;

		all++;
		إذا (!S_ISDIR((*dn)->dn_ډېرپېښ))
			استمر;

		name = (*dn)->name;
		إذا (which != وېشل_SUBDIR /* if not requested to skip . / .. */
		 /* or if it's not . or .. */
		 || name[0] != '.'
		 || (name[1] && (name[1] != '.' || name[2]))
		) {
			dirs++;
		}
	}
	أرجع which != وېشل_FILE ? dirs : all - dirs;
}

/* get memory to hold an array of pointers */
ساكن بنية dnode **dnalloc(بینښه عدد)
{
	إذا (عدد < 1)
		أرجع عدم;

	عدد++; /* so that we have terminating NULL */
	أرجع xzalloc(عدد * حجم(بنية dnode *));
}

#إذا ENABLE_FEATURE_LS_RECURSIVE
ساكن فراغ dfree(بنية dnode **dnp)
{
	بینښه i;

	إذا (dnp == عدم)
		أرجع;

	لكل (i = 0; dnp[i]; i++) {
		بنية dnode *cur = dnp[i];
		إذا (cur->fname_allocated)
			حرر((حرف*)cur->fullname);
		حرر(cur);
	}
	حرر(dnp);
}
#وإلا
#تعریف_کړه dfree(...) ((فراغ)0)
#د_که_پای

/* Returns NULL-terminated malloced vector of pointers (or NULL) */
ساكن بنية dnode **splitdnarray(بنية dnode **dn, صحيح which)
{
	بینښه dncnt, d;
	بنية dnode **dnp;

	إذا (dn == عدم)
		أرجع عدم;

	/* count how many dirs or files there are */
	dncnt = شمېر_dirs(dn, which);

	/* allocate a file array and a dir array */
	dnp = dnalloc(dncnt);

	/* copy the entrys into the file or dir array */
	لكل (d = 0; *dn; dn++) {
		إذا (S_ISDIR((*dn)->dn_ډېرپېښ)) {
			ثابت حرف *name;

			إذا (which == وېشل_FILE)
				استمر;

			name = (*dn)->name;
			إذا ((which & وېشل_DIR) /* any dir... */
			/* ... or not . or .. */
			 || name[0] != '.'
			 || (name[1] && (name[1] != '.' || name[2]))
			) {
				dnp[d++] = *dn;
			}
		} وإلا
		إذا (which == وېشل_FILE) {
			dnp[d++] = *dn;
		}
	}
	أرجع dnp;
}

#إذا ENABLE_FEATURE_LS_SORTFILES
ساكن صحيح sortcmp(ثابت فراغ *a, ثابت فراغ *b)
{
	بنية dnode *d1 = *(بنية dnode **)a;
	بنية dnode *d2 = *(بنية dnode **)b;
	بینښه opt = option_mask32;
	off_t dif;

	dif = 0; /* assume sort by name */
	// TODO: use pre-initialized function pointer
	// instead of branch forest
	إذا (opt & OPT_dirs_لومړی) {
		dif = S_ISDIR(d2->dn_ډېرپېښ) - S_ISDIR(d1->dn_ډېرپېښ);
		إذا (dif != 0)
			اذهب maybe_invert_and_ret;
	}

	إذا (opt & OPT_S) { /* sort by size */
		dif = (d2->dn_اندازه - d1->dn_اندازه);
	} وإلا
	إذا (opt & OPT_t) { /* sort by time */
		dif = (d2->dn_وخت - d1->dn_وخت);
	} وإلا
#إذا defined(HAVE_STRVERSCMP) && HAVE_STRVERSCMP == 1
	إذا (opt & OPT_v) { /* sort by version */
		dif = strverscmp(d1->name, d2->name);
	} وإلا
#د_که_پای
	إذا (opt & OPT_X) { /* sort by extension */
		dif = strcmp(strchrnul(d1->name, '.'), strchrnul(d2->name, '.'));
	}
	إذا (dif == 0) {
		/* sort by name, use as tie breaker for other sorts */
		إذا (ENABLE_LOCALE_SUPPORT)
			dif = strcoll(d1->name, d2->name);
		وإلا
			dif = strcmp(d1->name, d2->name);
	} وإلا {
		/* Make dif fit into an int */
		إذا (حجم(dif) > حجم(صحيح)) {
			تعداد { بټونه_منزل_ښويدنه = 8 * (حجم(dif) - حجم(صحيح)) };
			/* shift leaving only "int" worth of bits */
			/* (this requires dif != 0, and here it is nonzero) */
			dif = 1 | (صحيح)((uoff_t)dif >> بټونه_منزل_ښويدنه);
		}
	}
 maybe_invert_and_ret:
	أرجع (opt & OPT_r) ? -(صحيح)dif : (صحيح)dif;
}

ساكن فراغ dnsort(بنية dnode **dn, صحيح اندازه)
{
	qsort(dn, اندازه, حجم(*dn), sortcmp);
}

ساكن فراغ ترتیبول_and_display_files(بنية dnode **dn, بینښه nfiles)
{
	dnsort(dn, nfiles);
	display_files(dn, nfiles);
}
#وإلا
# define dnsort(dn, اندازه) ((فراغ)0)
# define ترتیبول_and_display_files(dn, nfiles) display_files(dn, nfiles)
#د_که_پای

/* Returns NULL-terminated malloced vector of pointers (or NULL) */
ساكن بنية dnode **لټون_one_dir(ثابت حرف *لار, بینښه *nfiles_p)
{
	بنية dnode *dn, *cur, **dnp;
	بنية dirent *ثبت;
	DIR *dir;
	بینښه i, nfiles;

	*nfiles_p = 0;
	dir = warn_opendir(لار);
	إذا (dir == عدم) {
		G.exit_کوډ = EXIT_FAILURE;
		أرجع عدم;	/* could not open the dir */
	}
	dn = عدم;
	nfiles = 0;
	طالما ((ثبت = readdir(dir)) != عدم) {
		حرف *fullname;

		/* are we going to list the file- it may be . or .. or a hidden file */
		إذا (ثبت->d_name[0] == '.') {
			إذا (!(option_mask32 & (OPT_a|OPT_A)))
				استمر; /* skip all dotfiles if no -a/-A */
			إذا (!(option_mask32 & OPT_a)
			 && (!ثبت->d_name[1] || (ثبت->d_name[1] == '.' && !ثبت->d_name[2]))
			) {
				استمر; /* if only -A, skip . and .. but show other dotfiles */
			}
		}
		fullname = concat_لار_file(لار, ثبت->d_name);
		cur = my_stat(fullname, bb_basename(fullname), 0);
		إذا (!cur) {
			حرر(fullname);
			استمر;
		}
		cur->fname_allocated = 1;
		cur->dn_بل = dn;
		dn = cur;
		nfiles++;
	}
	closedir(dir);

	إذا (dn == عدم)
		أرجع عدم;

	/* now that we know how many files there are
	 * allocate memory for an array to hold dnode pointers
	 */
	*nfiles_p = nfiles;
	dnp = dnalloc(nfiles);
	لكل (i = 0; /* i < nfiles - detected via !dn below */; i++) {
		dnp[i] = dn;	/* save pointer to node in array */
		dn = dn->dn_بل;
		إذا (!dn)
			اكسر;
	}

	أرجع dnp;
}

#إذا ENABLE_DESKTOP
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
ساكن off_t calculate_بلاکونه(بنية dnode **dn)
{
	uoff_t بلاکونه = 1;
	إذا (dn) {
		طالما (*dn) {
			/* st_blocks is in 512 byte blocks */
			بلاکونه += (*dn)->dn_بلاکونه;
			dn++;
		}
	}

	/* Even though standard says use 512 byte blocks, coreutils use 1k */
	/* Actually, we round up by calculating (blocks + 1) / 2,
	 * "+ 1" was done when we initialized blocks to 1 */
	أرجع بلاکونه >> 1;
}
#د_که_پای

ساكن فراغ لټون_and_display_dirs_recur(بنية dnode **dn, صحيح لومړی)
{
	بینښه nfiles;
	بنية dnode **subdnp;

	لكل (; *dn; dn++) {
		إذا (G.show_dirname || (option_mask32 & OPT_R)) {
			إذا (!لومړی)
				bb_putchar('\n');
			لومړی = 0;
			چاپ_کړه("%s:\n", (*dn)->fullname);
		}
		subdnp = لټون_one_dir((*dn)->fullname, &nfiles);
#إذا ENABLE_DESKTOP
		إذا (option_mask32 & (OPT_s|OPT_l)) {
			إذا (option_mask32 & OPT_h) {
				چاپ_کړه("total %-"HUMAN_READABLE_MAX_پلنوالی_STR"s\n",
					/* print size, no fractions, use suffixes */
					make_human_readable_str(calculate_بلاکونه(subdnp) * 1024,
								0, 0)
				);
			} وإلا {
				چاپ_کړه("total %"OFF_FMT"u\n", calculate_بلاکونه(subdnp));
			}
		}
#د_که_پای
		إذا (nfiles > 0) {
			/* list all files at this level */
			ترتیبول_and_display_files(subdnp, nfiles);

			إذا (ENABLE_FEATURE_LS_RECURSIVE
			 && (option_mask32 & OPT_R)
			) {
				بنية dnode **dnd;
				بینښه dndirs;
				/* recursive - list the sub-dirs */
				dnd = splitdnarray(subdnp, وېشل_SUBDIR);
				dndirs = شمېر_dirs(subdnp, وېشل_SUBDIR);
				إذا (dndirs > 0) {
					dnsort(dnd, dndirs);
					لټون_and_display_dirs_recur(dnd, 0);
					/* free the array of dnode pointers to the dirs */
					حرر(dnd);
				}
			}
			/* free the dnodes and the fullname mem */
			dfree(subdnp);
		}
	}
}


صحيح ls_main(صحيح argc ناکارول_شوی_PARAM, حرف **argv)
{	/*      ^^^^^^^^^^^^^^^^^ note: if FTPD, argc can be wrong, see ftpd.c */
	بنية dnode **dnd;
	بنية dnode **dnf;
	بنية dnode **dnp;
	بنية dnode *dn;
	بنية dnode *cur;
	بینښه opt;
	بینښه nfiles;
	بینښه dnfiles;
	بینښه dndirs;
	بینښه i;
#إذا ENABLE_FEATURE_LS_COLOR
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
	ساكن ثابت حرف color_str[] ALIGN1 =
		"always\0""yes\0""force\0"
		"auto\0""tty\0""if-tty\0";
	/* need to initialize since --color has _an optional_ argument */
	ثابت حرف *color_opt = color_str; /* "always" */
#د_که_پای
#إذا ENABLE_LONG_OPTS
	ساكن ثابت حرف ls_longopts[] ALIGN1 =
		"full-time\0" No_دلیل "\xff"
		"group-directories-first\0" No_دلیل "\xfe"
		IF_FEATURE_LS_COLOR("color\0" Optional_دلیل "\xfd")
	;
#د_که_پای

	INIT_G();

	init_unicode();

#إذا ENABLE_FEATURE_LS_پلنوالی
	/* obtain the terminal width */
	G_terminal_پلنوالی = get_terminal_پلنوالی(STDIN_FILENO);
	/* go one less... */
	G_terminal_پلنوالی--;
#د_که_پای

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
			IF_FEATURE_LS_پلنوالی(":w+")
		, ls_longopts
		IF_FEATURE_LS_پلنوالی(, /*-T*/ عدم, /*-w*/ &G_terminal_پلنوالی)
		IF_FEATURE_LS_COLOR(, &color_opt)
	);
#إذا 0 /* option bits debug */
	bb_error_پیغام("opt:0x%08x l:%x H:%x color:%x dirs:%x", opt, OPT_l, OPT_H, OPT_color, OPT_dirs_لومړی);
	إذا (opt & OPT_c         ) bb_error_پیغام("-c");
	إذا (opt & OPT_l         ) bb_error_پیغام("-l");
	إذا (opt & OPT_H         ) bb_error_پیغام("-H");
	إذا (opt & OPT_color     ) bb_error_پیغام("--color");
	إذا (opt & OPT_dirs_لومړی) bb_error_پیغام("--group-directories-first");
	إذا (opt & OPT_ډک_وخت ) bb_error_پیغام("--full-time");
	وتل(0);
#د_که_پای

#إذا ENABLE_SELINUX
	إذا (opt & OPT_Z) {
		إذا (!is_selinux_enabled())
			option_mask32 &= ~OPT_Z;
	}
#د_که_پای

#إذا ENABLE_FEATURE_LS_COLOR
	/* set G_show_color = 1/0 */
	إذا (ENABLE_FEATURE_LS_COLOR_IS_DEFAULT && !is_غړی_dumb()) {
		حرف *p = getenv("LS_COLORS");
		/* LS_COLORS is unset, or (not empty && not "none") ? */
		إذا (!p || (p[0] && strcmp(p, "none") != 0)) {
			إذا (isatty(STDOUT_FILENO)) {
				/* check isatty() last because it's expensive (syscall) */
				G_show_color = 1;
			}
		}
	}
	إذا (opt & OPT_color) {
		إذا (color_opt[0] == 'n')
			G_show_color = 0;
		وإلا بدل (شاخص_دننه_substrings(color_str, color_opt)) {
		حالة 3:
		حالة 4:
		حالة 5:
			إذا (!is_غړی_dumb() && isatty(STDOUT_FILENO)) {
		حالة 0:
		حالة 1:
		حالة 2:
				G_show_color = 1;
			}
		}
	}
#د_که_پای

	/* sort out which command line options take precedence */
	إذا (ENABLE_FEATURE_LS_RECURSIVE && (opt & OPT_d))
		option_mask32 &= ~OPT_R;	/* no recurse if listing only dir */
	إذا (!(opt & OPT_l)) { /* not -l? */
		إذا (ENABLE_FEATURE_LS_TIMESTAMPS && ENABLE_FEATURE_LS_SORTFILES) {
			/* when to sort by time? -t[cu] sorts by time even with -l */
			/* (this is achieved by opt_flags[] element for -t) */
			/* without -l, bare -c or -u enable sort too */
			/* (with -l, bare -c or -u just select which time to show) */
			إذا (opt & (OPT_c|OPT_u)) {
				option_mask32 |= OPT_t;
			}
		}
	}

	/* choose a display format if one was not already specified by an option */
	إذا (!(option_mask32 & (OPT_l|OPT_1|OPT_x|OPT_C)))
		option_mask32 |= (isatty(STDOUT_FILENO) ? OPT_C : OPT_1);

	إذا (ENABLE_FTPD && applet_name[0] == 'f') {
		/* ftpd secret backdoor. dirs first are much nicer */
		option_mask32 |= OPT_dirs_لومړی;
	}

	argv += optind;
	إذا (!argv[0])
		*--argv = (حرف*)".";

	إذا (argv[1])
		G.show_dirname = 1; /* 2 or more items? label directories */

	/* stuff the command line file names into a dnode array */
	dn = عدم;
	nfiles = 0;
	افعل {
		cur = my_stat(*argv, *argv,
			/* follow links on command line unless -l, -i, -s or -F: */
			!(option_mask32 & (OPT_l|OPT_i|OPT_s|OPT_F))
			/* ... or if -H: */
			|| (option_mask32 & OPT_H)
			/* ... or if -L, but my_stat always follows links if -L */
		);
		argv++;
		إذا (!cur)
			استمر;
		/*cur->fname_allocated = 0; - already is */
		cur->dn_بل = dn;
		dn = cur;
		nfiles++;
	} طالما (*argv);

	/* nfiles _may_ be 0 here - try "ls doesnt_exist" */
	إذا (nfiles == 0)
		أرجع G.exit_کوډ;

	/* now that we know how many files there are
	 * allocate memory for an array to hold dnode pointers
	 */
	dnp = dnalloc(nfiles);
	لكل (i = 0; /* i < nfiles - detected via !dn below */; i++) {
		dnp[i] = dn;	/* save pointer to node in array */
		dn = dn->dn_بل;
		إذا (!dn)
			اكسر;
	}

	إذا (option_mask32 & OPT_d) {
		ترتیبول_and_display_files(dnp, nfiles);
	} وإلا {
		dnd = splitdnarray(dnp, وېشل_DIR);
		dnf = splitdnarray(dnp, وېشل_FILE);
		dndirs = شمېر_dirs(dnp, وېشل_DIR);
		dnfiles = nfiles - dndirs;
		إذا (dnfiles > 0) {
			ترتیبول_and_display_files(dnf, dnfiles);
			إذا (ENABLE_FEATURE_CLEAN_پورته)
				حرر(dnf);
		}
		إذا (dndirs > 0) {
			dnsort(dnd, dndirs);
			لټون_and_display_dirs_recur(dnd, dnfiles == 0);
			إذا (ENABLE_FEATURE_CLEAN_پورته)
				حرر(dnd);
		}
	}

	إذا (ENABLE_FEATURE_CLEAN_پورته)
		dfree(dnp);
	أرجع G.exit_کوډ;
}
