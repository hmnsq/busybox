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
#جيڪڏهن_پورو


تعداد {
TERMINAL_ويڪر  = 80, /* use 79 if terminal has linefold bug */

ورهايو_FILE      = 0,
ورهايو_DIR       = 1,
ورهايو_SUBDIR    = 2,
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
#وصف_ڪريو ls_options \
	"Cadi1lgnsxAk"       /* 12 opts, total 12 */ \
	IF_FEATURE_LS_FILETYPES("Fp")    /* 2, 14 */ \
	IF_FEATURE_LS_RECURSIVE("R")     /* 1, 15 */ \
	IF_SELINUX("Z")                  /* 1, 16 */ \
	"Q"                              /* 1, 17 */ \
	IF_FEATURE_LS_TIMESTAMPS("ctu")  /* 3, 20 */ \
	IF_FEATURE_LS_SORTFILES("SXrv")  /* 4, 24 */ \
	IF_FEATURE_LS_FOLLOWLINKS("LH")  /* 2, 26 */ \
	IF_FEATURE_HUMAN_READABLE("h")   /* 1, 27 */ \
	IF_FEATURE_LS_ويڪر("T:w:")      /* 2, 29 */

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
	OPTBIT_ڀريل_وقت = OPTBIT_T + 2 * ENABLE_FEATURE_LS_ويڪر,
	OPTBIT_dirs_پهريون,
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
	OPT_T = (1 << OPTBIT_T) * ENABLE_FEATURE_LS_ويڪر,
	OPT_w = (1 << OPTBIT_w) * ENABLE_FEATURE_LS_ويڪر,
	OPT_ڀريل_وقت  = (1 << OPTBIT_ڀريل_وقت ) * ENABLE_LONG_OPTS,
	OPT_dirs_پهريون = (1 << OPTBIT_dirs_پهريون) * ENABLE_LONG_OPTS,
	OPT_color      = (1 << OPTBIT_color     ) * ENABLE_FEATURE_LS_COLOR,
};

/*
 * a directory entry and its stat info
 */
بنية dnode {
	ثابت حرف *name;       /* usually basename, but think "ls -l dir/file" */
	ثابت حرف *fullname;   /* full name (usable for stat etc) */
	بنية dnode *dn_ايندڙ;  /* for linked list */
	IF_SELINUX(security_context_t sid;)
	smallint fname_allocated;

	/* Used to avoid re-doing [l]stat at printout stage
	 * if we already collected needed data in scan stage:
	 */
	گهڻائي_t    dn_گهڻائي_lstat;   /* obtained with lstat, or 0 */
	گهڻائي_t    dn_گهڻائي_stat;    /* obtained with stat, or 0 */

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
	گهڻائي_t    dn_گهڻائي; /* obtained with lstat OR stat, depending on -L etc */
	off_t     dn_سائيز;
#إذا ENABLE_FEATURE_LS_TIMESTAMPS || ENABLE_FEATURE_LS_SORTFILES
	وقت_t    dn_وقت;
#جيڪڏهن_پورو
	ino_t     dn_ino;
	blkcnt_t  dn_بلاڪون;
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
#جيڪڏهن_پورو
	smallint exit_ڪوڊ;
	smallint show_dirname;
#إذا ENABLE_FEATURE_LS_ويڪر
	بنانشان terminal_ويڪر;
# define G_terminal_ويڪر (G.terminal_ويڪر)
#وإلا
# define G_terminal_ويڪر TERMINAL_ويڪر
#جيڪڏهن_پورو
#إذا ENABLE_FEATURE_LS_TIMESTAMPS
	/* Do time() just once. Saves one syscall per file for "ls -l" */
	وقت_t current_وقت_t;
#جيڪڏهن_پورو
} FIX_ALIASING;
#وصف_ڪريو G (*(بنية globals*)bb_common_bufsiz1)
#وصف_ڪريو INIT_G() افعل { \
	تياري_common_bufsiz(); \
	/* we have to zero it out because of NOEXEC */ \
	memset(&G, 0, حجم(G)); \
	IF_FEATURE_LS_ويڪر(G_terminal_ويڪر = TERMINAL_ويڪر;) \
	IF_FEATURE_LS_TIMESTAMPS(وقت(&G.current_وقت_t);) \
} طالما (0)

#وصف_ڪريو ESC "\033"


/*** Output code ***/


/* FYI type values: 1:fifo 2:char 4:dir 6:blk 8:file 10:link 12:socket
 * (various wacky OSes: 13:Sun door 14:BSD whiteout 5:XENIX named file
 *  3/7:multiplexed char/block device)
 * and we use 0 for unknown and 15 for executables (see below) */
#وصف_ڪريو TYPEINDEX(گهڻائي) (((گهڻائي) >> 12) & 0x0f)
/*                       un  fi chr -   dir -  blk  -  file -  link - sock -   - exe */
#وصف_ڪريو APPCHAR(گهڻائي)   ("\0""|""\0""\0""/""\0""\0""\0""\0""\0""@""\0""=""\0""\0""\0" [TYPEINDEX(گهڻائي)])
/* 036 black foreground              050 black background
   037 red foreground                051 red background
   040 green foreground              052 green background
   041 brown foreground              053 brown background
   042 blue foreground               054 blue background
   043 magenta (purple) foreground   055 magenta background
   044 cyan (light blue) foreground  056 cyan background
   045 gray foreground               057 white background
*/
#وصف_ڪريو COLOR(گهڻائي) ( \
	/*un  fi  chr  -  dir  -  blk  -  file -  link -  sock -   -  exe */ \
	"\037\043\043\045\042\045\043\043\000\045\044\045\043\045\045\040" \
	[TYPEINDEX(گهڻائي)])
/* Select normal (0) [actually "reset all"] or bold (1)
 * (other attributes are 2:dim 4:underline 5:blink 7:reverse,
 *  let's use 7 for "impossible" types, just for fun)
 * Note: coreutils 6.9 uses inverted red for setuid binaries.
 */
#وصف_ڪريو ATTR(گهڻائي) ( \
	/*un fi chr - dir - blk - file- link- sock- -  exe */ \
	"\01\00\01\07\01\07\01\07\00\07\01\07\01\07\07\01" \
	[TYPEINDEX(گهڻائي)])

#إذا ENABLE_FEATURE_LS_COLOR
/* mode of zero is interpreted as "unknown" (stat failed) */
ساكن حرف fgcolor(گهڻائي_t گهڻائي)
{
	إذا (S_ISREG(گهڻائي) && (گهڻائي & (S_IXUSR | S_IXGRP | S_IXOTH)))
		أرجع COLOR(0xF000);	/* File is executable ... */
	أرجع COLOR(گهڻائي);
}
ساكن حرف bold(گهڻائي_t گهڻائي)
{
	إذا (S_ISREG(گهڻائي) && (گهڻائي & (S_IXUSR | S_IXGRP | S_IXOTH)))
		أرجع ATTR(0xF000);	/* File is executable ... */
	أرجع ATTR(گهڻائي);
}
#جيڪڏهن_پورو

#إذا ENABLE_FEATURE_LS_FILETYPES
ساكن حرف append_char(گهڻائي_t گهڻائي)
{
	إذا (!(option_mask32 & (OPT_F|OPT_p)))
		أرجع '\0';

	إذا (S_ISDIR(گهڻائي))
		أرجع '/';
	إذا (!(option_mask32 & OPT_F))
		أرجع '\0';
	إذا (S_ISREG(گهڻائي) && (گهڻائي & (S_IXUSR | S_IXGRP | S_IXOTH)))
		أرجع '*';
	أرجع APPCHAR(گهڻائي);
}
#جيڪڏهن_پورو

ساكن بنانشان calc_name_len(ثابت حرف *name)
{
	بنانشان len;
	uni_stat_t uni_stat;

	// TODO: quote tab as \t, etc, if -Q
	name = printable_string2(&uni_stat, name);

	إذا (!(option_mask32 & OPT_Q)) {
		أرجع uni_stat.unicode_ويڪر;
	}

	len = 2 + uni_stat.unicode_ويڪر;
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
ساكن بنانشان print_name(ثابت حرف *name)
{
	بنانشان len;
	uni_stat_t uni_stat;

	// TODO: quote tab as \t, etc, if -Q
	name = printable_string2(&uni_stat, name);

	إذا (!(option_mask32 & OPT_Q)) {
		fputs_stdout(name);
		أرجع uni_stat.unicode_ويڪر;
	}

	len = 2 + uni_stat.unicode_ويڪر;
	اکر_لکو('"');
	طالما (*name) {
		إذا (*name == '"' || *name == '\\') {
			اکر_لکو('\\');
			len++;
		}
		اکر_لکو(*name);
		name++;
	}
	اکر_لکو('"');
	أرجع len;
}

/* Return the number of used columns.
 * Note that only columnar output uses return value,
 * -l and -1 modes don't care.
 */
ساكن NOINLINE بنانشان display_single(ثابت بنية dnode *dn)
{
	بنانشان ڪالم = 0;
	حرف *lpath;
	صحيح opt;
#إذا ENABLE_FEATURE_LS_FILETYPES || ENABLE_FEATURE_LS_COLOR
	بنية stat statbuf;
#جيڪڏهن_پورو
#إذا ENABLE_FEATURE_LS_FILETYPES
	حرف append = append_char(dn->dn_گهڻائي);
#جيڪڏهن_پورو

	opt = option_mask32;

	/* Do readlink early, so that if it fails, error message
	 * does not appear *inside* the "ls -l" line */
	lpath = عدم;
	إذا (opt & OPT_l)
		إذا (S_ISLNK(dn->dn_گهڻائي))
			lpath = xmalloc_readlink_or_warn(dn->fullname);

	إذا (opt & OPT_i) /* show inode# */
		ڪالم += لکو("%7llu ", (ڊگهو ڊگهو) dn->dn_ino);
	إذا (opt & OPT_s) { /* show allocated blocks */
		إذا (opt & OPT_h) {
			ڪالم += لکو("%"HUMAN_READABLE_MAX_ويڪر_STR"s ",
				/* print size, show one fractional, use suffixes */
				make_human_readable_str((off_t)dn->dn_بلاڪون << 9, 1, 0)
			);
		} وإلا {
			ڪالم += لکو("%6"OFF_FMT"u ", (off_t)(dn->dn_بلاڪون >> 1));
		}
	}
	إذا (opt & OPT_l) {
		/* long listing: show mode */
		حرف modestr[12];
		ڪالم += لکو("%-10s ", bb_گهڻائي_string(modestr, dn->dn_گهڻائي));
		/* long listing: show number of links */
		ڪالم += لکو("%4lu ", (ڊگهو) dn->dn_nlink);
		/* long listing: show user/group */
		إذا (opt & OPT_n) {
			إذا (opt & OPT_g)
				ڪالم += لکو("%-8u ", (صحيح) dn->dn_gid);
			وإلا
				ڪالم += لکو("%-8u %-8u ",
						(صحيح) dn->dn_uid,
						(صحيح) dn->dn_gid);
		}
#إذا ENABLE_FEATURE_LS_USERNAME
		وإلا {
			إذا (opt & OPT_g) {
				ڪالم += لکو("%-8s ",
					get_cached_groupname(dn->dn_gid));
			} وإلا {
				ڪالم += لکو("%-8s %-8s ",
					get_cached_username(dn->dn_uid),
					get_cached_groupname(dn->dn_gid));
			}
		}
#جيڪڏهن_پورو
#إذا ENABLE_SELINUX
	}
	إذا (opt & OPT_Z) {
		ڪالم += لکو("%-32s ", dn->sid ? dn->sid : "?");
		freecon(dn->sid);
	}
	إذا (opt & OPT_l) {
#جيڪڏهن_پورو
		/* long listing: show size */
		إذا (S_ISBLK(dn->dn_گهڻائي) || S_ISCHR(dn->dn_گهڻائي)) {
			ڪالم += لکو("%4u, %3u ",
					dn->dn_rdev_maj,
					dn->dn_rdev_min);
		} وإلا {
			إذا (opt & OPT_h) {
				ڪالم += لکو("%"HUMAN_READABLE_MAX_ويڪر_STR"s ",
					/* print size, show one fractional, use suffixes */
					make_human_readable_str(dn->dn_سائيز, 1, 0)
				);
			} وإلا {
				ڪالم += لکو("%9"OFF_FMT"u ", dn->dn_سائيز);
			}
		}
#إذا ENABLE_FEATURE_LS_TIMESTAMPS
		/* long listing: show {m,c,a}time */
		إذا (opt & OPT_ڀريل_وقت) { /* --full-time */
			/* coreutils 8.4 ls --full-time prints:
			 * 2009-07-13 17:49:27.000000000 +0200
			 * we don't show fractional seconds.
			 */
			حرف buf[حجم("YYYY-mm-dd HH:MM:SS TIMEZONE")];
			strftime(buf, حجم(buf), "%Y-%m-%d %H:%M:%S %z",
					localtime(&dn->dn_وقت));
			ڪالم += لکو("%s ", buf);
		} وإلا { /* ordinary time format */
			/* G.current_time_t is ~== time(NULL) */
			حرف *filetime = ctime(&dn->dn_وقت);
			/* filetime's format: "Wed Jun 30 21:49:08 1993\n" */
			وقت_t age = G.current_وقت_t - dn->dn_وقت;
			إذا (age < 3600L * 24 * 365 / 2 && age > -15 * 60) {
				/* less than 6 months old */
				/* "mmm dd hh:mm " */
				لکو("%.12s ", filetime + 4);
			} وإلا {
				/* "mmm dd  yyyy " */
				/* "mmm dd yyyyy " after year 9999 :) */
				strchr(filetime + 20, '\n')[0] = ' ';
				لکو("%.7s%6s", filetime + 4, filetime + 20);
			}
			ڪالم += 13;
		}
#جيڪڏهن_پورو
	}

#إذا ENABLE_FEATURE_LS_COLOR
	إذا (G_show_color) {
		گهڻائي_t گهڻائي = dn->dn_گهڻائي_lstat;
		إذا (!گهڻائي)
			إذا (lstat(dn->fullname, &statbuf) == 0)
				گهڻائي = statbuf.st_گهڻائي;
		لکو(ESC"[%u;%um", bold(گهڻائي), fgcolor(گهڻائي));
	}
#جيڪڏهن_پورو
	ڪالم += print_name(dn->name);
	إذا (G_show_color) {
		لکو(ESC"[m");
	}

	إذا (lpath) {
		لکو(" -> ");
#إذا ENABLE_FEATURE_LS_FILETYPES || ENABLE_FEATURE_LS_COLOR
		إذا ((opt & (OPT_F|OPT_p))
		 || G_show_color
		) {
			گهڻائي_t گهڻائي = dn->dn_گهڻائي_stat;
			إذا (!گهڻائي)
				إذا (stat(dn->fullname, &statbuf) == 0)
					گهڻائي = statbuf.st_گهڻائي;
# إذا ENABLE_FEATURE_LS_FILETYPES
			append = append_char(گهڻائي);
# endif
# إذا ENABLE_FEATURE_LS_COLOR
			إذا (G_show_color) {
				لکو(ESC"[%u;%um", bold(گهڻائي), fgcolor(گهڻائي));
			}
# endif
		}
#جيڪڏهن_پورو
		ڪالم += print_name(lpath) + 4;
		حرر(lpath);
		إذا (G_show_color) {
			لکو(ESC"[m");
		}
	}
#إذا ENABLE_FEATURE_LS_FILETYPES
	إذا (opt & (OPT_F|OPT_p)) {
		إذا (append) {
			اکر_لکو(append);
			ڪالم++;
		}
	}
#جيڪڏهن_پورو

	أرجع ڪالم;
}

ساكن فراغ display_files(بنية dnode **dn, بنانشان nfiles)
{
	بنانشان i, ncols, nrows, صف, nc;
	بنانشان ڪالم;
	بنانشان nexttab;
	بنانشان ڪالم_ويڪر = 0; /* used only by coulmnal output */

	إذا (option_mask32 & (OPT_l|OPT_1)) {
		ncols = 1;
	} وإلا {
		/* find the longest file name, use that as the column width */
		لكل (i = 0; dn[i]; i++) {
			صحيح len = calc_name_len(dn[i]->name);
			إذا (ڪالم_ويڪر < len)
				ڪالم_ويڪر = len;
		}
		ڪالم_ويڪر += 2
			+ ((option_mask32 & OPT_Z) ? 33 : 0) /* context width */
			+ ((option_mask32 & OPT_i) ? 8 : 0) /* inode# width */
			+ ((option_mask32 & OPT_s) ? 5 : 0) /* "alloc block" width */
		;
		ncols = (بنانشان)G_terminal_ويڪر / ڪالم_ويڪر;
	}

	إذا (ncols > 1) {
		nrows = nfiles / ncols;
		إذا (nrows * ncols < nfiles)
			nrows++;                /* round up fractionals */
	} وإلا {
		nrows = nfiles;
		ncols = 1;
	}

	ڪالم = 0;
	nexttab = 0;
	لكل (صف = 0; صف < nrows; صف++) {
		لكل (nc = 0; nc < ncols; nc++) {
			/* reach into the array based on the column and row */
			إذا (option_mask32 & OPT_x)
				i = (صف * ncols) + nc;	/* display across row */
			وإلا
				i = (nc * nrows) + صف;	/* display by column */
			إذا (i < nfiles) {
				إذا (ڪالم > 0) {
					nexttab -= ڪالم;
					لکو("%*s", nexttab, "");
					ڪالم += nexttab;
				}
				nexttab = ڪالم + ڪالم_ويڪر;
				ڪالم += display_single(dn[i]);
			}
		}
		اکر_لکو('\n');
		ڪالم = 0;
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
#جيڪڏهن_پورو
		إذا (stat(fullname, &statbuf)) {
			bb_simple_perror_نياپو(fullname);
			G.exit_ڪوڊ = EXIT_FAILURE;
			حرر(cur);
			أرجع عدم;
		}
		cur->dn_گهڻائي_stat = statbuf.st_گهڻائي;
	} وإلا {
#إذا ENABLE_SELINUX
		إذا (option_mask32 & OPT_Z) {
			lgetfilecon(fullname, &cur->sid);
		}
#جيڪڏهن_پورو
		إذا (lstat(fullname, &statbuf)) {
			bb_simple_perror_نياپو(fullname);
			G.exit_ڪوڊ = EXIT_FAILURE;
			حرر(cur);
			أرجع عدم;
		}
		cur->dn_گهڻائي_lstat = statbuf.st_گهڻائي;
	}

	/* cur->dstat = statbuf: */
	cur->dn_گهڻائي   = statbuf.st_گهڻائي  ;
	cur->dn_سائيز   = statbuf.st_سائيز  ;
#إذا ENABLE_FEATURE_LS_TIMESTAMPS || ENABLE_FEATURE_LS_SORTFILES
	cur->dn_وقت   = statbuf.st_mtime ;
	إذا (option_mask32 & OPT_u)
		cur->dn_وقت = statbuf.st_atime;
	إذا (option_mask32 & OPT_c)
		cur->dn_وقت = statbuf.st_ctime;
#جيڪڏهن_پورو
	cur->dn_ino    = statbuf.st_ino   ;
	cur->dn_بلاڪون = statbuf.st_بلاڪون;
	cur->dn_nlink  = statbuf.st_nlink ;
	cur->dn_uid    = statbuf.st_uid   ;
	cur->dn_gid    = statbuf.st_gid   ;
	cur->dn_rdev_maj = major(statbuf.st_rdev);
	cur->dn_rdev_min = minor(statbuf.st_rdev);

	أرجع cur;
}

ساكن بنانشان ڳڻپ_dirs(بنية dnode **dn, صحيح which)
{
	بنانشان dirs, all;

	إذا (!dn)
		أرجع 0;

	dirs = all = 0;
	لكل (; *dn; dn++) {
		ثابت حرف *name;

		all++;
		إذا (!S_ISDIR((*dn)->dn_گهڻائي))
			استمر;

		name = (*dn)->name;
		إذا (which != ورهايو_SUBDIR /* if not requested to skip . / .. */
		 /* or if it's not . or .. */
		 || name[0] != '.'
		 || (name[1] && (name[1] != '.' || name[2]))
		) {
			dirs++;
		}
	}
	أرجع which != ورهايو_FILE ? dirs : all - dirs;
}

/* get memory to hold an array of pointers */
ساكن بنية dnode **dnalloc(بنانشان انگ)
{
	إذا (انگ < 1)
		أرجع عدم;

	انگ++; /* so that we have terminating NULL */
	أرجع xzalloc(انگ * حجم(بنية dnode *));
}

#إذا ENABLE_FEATURE_LS_RECURSIVE
ساكن فراغ dfree(بنية dnode **dnp)
{
	بنانشان i;

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
#وصف_ڪريو dfree(...) ((فراغ)0)
#جيڪڏهن_پورو

/* Returns NULL-terminated malloced vector of pointers (or NULL) */
ساكن بنية dnode **splitdnarray(بنية dnode **dn, صحيح which)
{
	بنانشان dncnt, d;
	بنية dnode **dnp;

	إذا (dn == عدم)
		أرجع عدم;

	/* count how many dirs or files there are */
	dncnt = ڳڻپ_dirs(dn, which);

	/* allocate a file array and a dir array */
	dnp = dnalloc(dncnt);

	/* copy the entrys into the file or dir array */
	لكل (d = 0; *dn; dn++) {
		إذا (S_ISDIR((*dn)->dn_گهڻائي)) {
			ثابت حرف *name;

			إذا (which == ورهايو_FILE)
				استمر;

			name = (*dn)->name;
			إذا ((which & ورهايو_DIR) /* any dir... */
			/* ... or not . or .. */
			 || name[0] != '.'
			 || (name[1] && (name[1] != '.' || name[2]))
			) {
				dnp[d++] = *dn;
			}
		} وإلا
		إذا (which == ورهايو_FILE) {
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
	بنانشان opt = option_mask32;
	off_t dif;

	dif = 0; /* assume sort by name */
	// TODO: use pre-initialized function pointer
	// instead of branch forest
	إذا (opt & OPT_dirs_پهريون) {
		dif = S_ISDIR(d2->dn_گهڻائي) - S_ISDIR(d1->dn_گهڻائي);
		إذا (dif != 0)
			اذهب maybe_invert_and_ret;
	}

	إذا (opt & OPT_S) { /* sort by size */
		dif = (d2->dn_سائيز - d1->dn_سائيز);
	} وإلا
	إذا (opt & OPT_t) { /* sort by time */
		dif = (d2->dn_وقت - d1->dn_وقت);
	} وإلا
#إذا defined(HAVE_STRVERSCMP) && HAVE_STRVERSCMP == 1
	إذا (opt & OPT_v) { /* sort by version */
		dif = strverscmp(d1->name, d2->name);
	} وإلا
#جيڪڏهن_پورو
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
			تعداد { بٽ_منزل_سرڪايو = 8 * (حجم(dif) - حجم(صحيح)) };
			/* shift leaving only "int" worth of bits */
			/* (this requires dif != 0, and here it is nonzero) */
			dif = 1 | (صحيح)((uoff_t)dif >> بٽ_منزل_سرڪايو);
		}
	}
 maybe_invert_and_ret:
	أرجع (opt & OPT_r) ? -(صحيح)dif : (صحيح)dif;
}

ساكن فراغ dnsort(بنية dnode **dn, صحيح سائيز)
{
	qsort(dn, سائيز, حجم(*dn), sortcmp);
}

ساكن فراغ ترتيب_ڏيو_and_display_files(بنية dnode **dn, بنانشان nfiles)
{
	dnsort(dn, nfiles);
	display_files(dn, nfiles);
}
#وإلا
# define dnsort(dn, سائيز) ((فراغ)0)
# define ترتيب_ڏيو_and_display_files(dn, nfiles) display_files(dn, nfiles)
#جيڪڏهن_پورو

/* Returns NULL-terminated malloced vector of pointers (or NULL) */
ساكن بنية dnode **جاچ_one_dir(ثابت حرف *رستو, بنانشان *nfiles_p)
{
	بنية dnode *dn, *cur, **dnp;
	بنية dirent *داخلا;
	DIR *dir;
	بنانشان i, nfiles;

	*nfiles_p = 0;
	dir = warn_opendir(رستو);
	إذا (dir == عدم) {
		G.exit_ڪوڊ = EXIT_FAILURE;
		أرجع عدم;	/* could not open the dir */
	}
	dn = عدم;
	nfiles = 0;
	طالما ((داخلا = readdir(dir)) != عدم) {
		حرف *fullname;

		/* are we going to list the file- it may be . or .. or a hidden file */
		إذا (داخلا->d_name[0] == '.') {
			إذا (!(option_mask32 & (OPT_a|OPT_A)))
				استمر; /* skip all dotfiles if no -a/-A */
			إذا (!(option_mask32 & OPT_a)
			 && (!داخلا->d_name[1] || (داخلا->d_name[1] == '.' && !داخلا->d_name[2]))
			) {
				استمر; /* if only -A, skip . and .. but show other dotfiles */
			}
		}
		fullname = concat_رستو_file(رستو, داخلا->d_name);
		cur = my_stat(fullname, bb_basename(fullname), 0);
		إذا (!cur) {
			حرر(fullname);
			استمر;
		}
		cur->fname_allocated = 1;
		cur->dn_ايندڙ = dn;
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
		dn = dn->dn_ايندڙ;
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
ساكن off_t calculate_بلاڪون(بنية dnode **dn)
{
	uoff_t بلاڪون = 1;
	إذا (dn) {
		طالما (*dn) {
			/* st_blocks is in 512 byte blocks */
			بلاڪون += (*dn)->dn_بلاڪون;
			dn++;
		}
	}

	/* Even though standard says use 512 byte blocks, coreutils use 1k */
	/* Actually, we round up by calculating (blocks + 1) / 2,
	 * "+ 1" was done when we initialized blocks to 1 */
	أرجع بلاڪون >> 1;
}
#جيڪڏهن_پورو

ساكن فراغ جاچ_and_display_dirs_recur(بنية dnode **dn, صحيح پهريون)
{
	بنانشان nfiles;
	بنية dnode **subdnp;

	لكل (; *dn; dn++) {
		إذا (G.show_dirname || (option_mask32 & OPT_R)) {
			إذا (!پهريون)
				bb_putchar('\n');
			پهريون = 0;
			لکو("%s:\n", (*dn)->fullname);
		}
		subdnp = جاچ_one_dir((*dn)->fullname, &nfiles);
#إذا ENABLE_DESKTOP
		إذا (option_mask32 & (OPT_s|OPT_l)) {
			إذا (option_mask32 & OPT_h) {
				لکو("total %-"HUMAN_READABLE_MAX_ويڪر_STR"s\n",
					/* print size, no fractions, use suffixes */
					make_human_readable_str(calculate_بلاڪون(subdnp) * 1024,
								0, 0)
				);
			} وإلا {
				لکو("total %"OFF_FMT"u\n", calculate_بلاڪون(subdnp));
			}
		}
#جيڪڏهن_پورو
		إذا (nfiles > 0) {
			/* list all files at this level */
			ترتيب_ڏيو_and_display_files(subdnp, nfiles);

			إذا (ENABLE_FEATURE_LS_RECURSIVE
			 && (option_mask32 & OPT_R)
			) {
				بنية dnode **dnd;
				بنانشان dndirs;
				/* recursive - list the sub-dirs */
				dnd = splitdnarray(subdnp, ورهايو_SUBDIR);
				dndirs = ڳڻپ_dirs(subdnp, ورهايو_SUBDIR);
				إذا (dndirs > 0) {
					dnsort(dnd, dndirs);
					جاچ_and_display_dirs_recur(dnd, 0);
					/* free the array of dnode pointers to the dirs */
					حرر(dnd);
				}
			}
			/* free the dnodes and the fullname mem */
			dfree(subdnp);
		}
	}
}


صحيح ls_main(صحيح argc اڻ_استعمال_ٿيل_PARAM, حرف **argv)
{	/*      ^^^^^^^^^^^^^^^^^ note: if FTPD, argc can be wrong, see ftpd.c */
	بنية dnode **dnd;
	بنية dnode **dnf;
	بنية dnode **dnp;
	بنية dnode *dn;
	بنية dnode *cur;
	بنانشان opt;
	بنانشان nfiles;
	بنانشان dnfiles;
	بنانشان dndirs;
	بنانشان i;
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
#جيڪڏهن_پورو
#إذا ENABLE_LONG_OPTS
	ساكن ثابت حرف ls_longopts[] ALIGN1 =
		"full-time\0" No_دليل "\xff"
		"group-directories-first\0" No_دليل "\xfe"
		IF_FEATURE_LS_COLOR("color\0" Optional_دليل "\xfd")
	;
#جيڪڏهن_پورو

	INIT_G();

	init_unicode();

#إذا ENABLE_FEATURE_LS_ويڪر
	/* obtain the terminal width */
	G_terminal_ويڪر = get_terminal_ويڪر(STDIN_FILENO);
	/* go one less... */
	G_terminal_ويڪر--;
#جيڪڏهن_پورو

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
			IF_FEATURE_LS_ويڪر(":w+")
		, ls_longopts
		IF_FEATURE_LS_ويڪر(, /*-T*/ عدم, /*-w*/ &G_terminal_ويڪر)
		IF_FEATURE_LS_COLOR(, &color_opt)
	);
#إذا 0 /* option bits debug */
	bb_error_نياپو("opt:0x%08x l:%x H:%x color:%x dirs:%x", opt, OPT_l, OPT_H, OPT_color, OPT_dirs_پهريون);
	إذا (opt & OPT_c         ) bb_error_نياپو("-c");
	إذا (opt & OPT_l         ) bb_error_نياپو("-l");
	إذا (opt & OPT_H         ) bb_error_نياپو("-H");
	إذا (opt & OPT_color     ) bb_error_نياپو("--color");
	إذا (opt & OPT_dirs_پهريون) bb_error_نياپو("--group-directories-first");
	إذا (opt & OPT_ڀريل_وقت ) bb_error_نياپو("--full-time");
	ٻاهر_نڪرو(0);
#جيڪڏهن_پورو

#إذا ENABLE_SELINUX
	إذا (opt & OPT_Z) {
		إذا (!is_selinux_enabled())
			option_mask32 &= ~OPT_Z;
	}
#جيڪڏهن_پورو

#إذا ENABLE_FEATURE_LS_COLOR
	/* set G_show_color = 1/0 */
	إذا (ENABLE_FEATURE_LS_COLOR_IS_DEFAULT && !is_رڪن_dumb()) {
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
		وإلا بدل (اشاريو_اندريون_substrings(color_str, color_opt)) {
		حالة 3:
		حالة 4:
		حالة 5:
			إذا (!is_رڪن_dumb() && isatty(STDOUT_FILENO)) {
		حالة 0:
		حالة 1:
		حالة 2:
				G_show_color = 1;
			}
		}
	}
#جيڪڏهن_پورو

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
		option_mask32 |= OPT_dirs_پهريون;
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
		cur->dn_ايندڙ = dn;
		dn = cur;
		nfiles++;
	} طالما (*argv);

	/* nfiles _may_ be 0 here - try "ls doesnt_exist" */
	إذا (nfiles == 0)
		أرجع G.exit_ڪوڊ;

	/* now that we know how many files there are
	 * allocate memory for an array to hold dnode pointers
	 */
	dnp = dnalloc(nfiles);
	لكل (i = 0; /* i < nfiles - detected via !dn below */; i++) {
		dnp[i] = dn;	/* save pointer to node in array */
		dn = dn->dn_ايندڙ;
		إذا (!dn)
			اكسر;
	}

	إذا (option_mask32 & OPT_d) {
		ترتيب_ڏيو_and_display_files(dnp, nfiles);
	} وإلا {
		dnd = splitdnarray(dnp, ورهايو_DIR);
		dnf = splitdnarray(dnp, ورهايو_FILE);
		dndirs = ڳڻپ_dirs(dnp, ورهايو_DIR);
		dnfiles = nfiles - dndirs;
		إذا (dnfiles > 0) {
			ترتيب_ڏيو_and_display_files(dnf, dnfiles);
			إذا (ENABLE_FEATURE_CLEAN_مٿي)
				حرر(dnf);
		}
		إذا (dndirs > 0) {
			dnsort(dnd, dndirs);
			جاچ_and_display_dirs_recur(dnd, dnfiles == 0);
			إذا (ENABLE_FEATURE_CLEAN_مٿي)
				حرر(dnd);
		}
	}

	إذا (ENABLE_FEATURE_CLEAN_مٿي)
		dfree(dnp);
	أرجع G.exit_ڪوڊ;
}
