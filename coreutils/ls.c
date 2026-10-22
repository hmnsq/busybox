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

#شامل "libbb.h"
#شامل "common_bufsiz.h"
#شامل "unicode.h"


/* This is a NOEXEC applet. Be very careful! */


#اگر ENABLE_FTPD
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
#اگر_ختم


شمار {
TERMINAL_چوڑائی  = 80, /* use 79 if terminal has linefold bug */

بانٹو_FILE      = 0,
بانٹو_DIR       = 1,
بانٹو_SUBDIR    = 2,
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
#تعریف_کرو ls_options \
	"Cadi1lgnsxAk"       /* 12 opts, total 12 */ \
	IF_FEATURE_LS_FILETYPES("Fp")    /* 2, 14 */ \
	IF_FEATURE_LS_RECURSIVE("R")     /* 1, 15 */ \
	IF_SELINUX("Z")                  /* 1, 16 */ \
	"Q"                              /* 1, 17 */ \
	IF_FEATURE_LS_TIMESTAMPS("ctu")  /* 3, 20 */ \
	IF_FEATURE_LS_SORTFILES("SXrv")  /* 4, 24 */ \
	IF_FEATURE_LS_FOLLOWLINKS("LH")  /* 2, 26 */ \
	IF_FEATURE_HUMAN_READABLE("h")   /* 1, 27 */ \
	IF_FEATURE_LS_چوڑائی("T:w:")      /* 2, 29 */

شمار {
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
	OPTBIT_بھرا_وقت = OPTBIT_T + 2 * ENABLE_FEATURE_LS_چوڑائی,
	OPTBIT_dirs_پہلا,
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
	OPT_T = (1 << OPTBIT_T) * ENABLE_FEATURE_LS_چوڑائی,
	OPT_w = (1 << OPTBIT_w) * ENABLE_FEATURE_LS_چوڑائی,
	OPT_بھرا_وقت  = (1 << OPTBIT_بھرا_وقت ) * ENABLE_LONG_OPTS,
	OPT_dirs_پہلا = (1 << OPTBIT_dirs_پہلا) * ENABLE_LONG_OPTS,
	OPT_color      = (1 << OPTBIT_color     ) * ENABLE_FEATURE_LS_COLOR,
};

/*
 * a directory entry and its stat info
 */
ساخت dnode {
	ثابت حرف *name;       /* usually basename, but think "ls -l dir/file" */
	ثابت حرف *fullname;   /* full name (usable for stat etc) */
	ساخت dnode *dn_اگلا;  /* for linked list */
	IF_SELINUX(security_context_t sid;)
	smallint fname_allocated;

	/* Used to avoid re-doing [l]stat at printout stage
	 * if we already collected needed data in scan stage:
	 */
	منوال_t    dn_منوال_lstat;   /* obtained with lstat, or 0 */
	منوال_t    dn_منوال_stat;    /* obtained with stat, or 0 */

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
	منوال_t    dn_منوال; /* obtained with lstat OR stat, depending on -L etc */
	off_t     dn_سائز;
#اگر ENABLE_FEATURE_LS_TIMESTAMPS || ENABLE_FEATURE_LS_SORTFILES
	وقت_t    dn_وقت;
#اگر_ختم
	ino_t     dn_ino;
	blkcnt_t  dn_بلاکیں;
	nlink_t   dn_nlink;
	uid_t     dn_uid;
	gid_t     dn_gid;
	صحیح       dn_rdev_maj;
	صحیح       dn_rdev_min;
//	dev_t     dn_dev;
//	blksize_t dn_blksize;
};

ساخت globals {
#اگر ENABLE_FEATURE_LS_COLOR
	smallint show_color;
# define G_show_color (G.show_color)
#ورنہ
# define G_show_color 0
#اگر_ختم
	smallint exit_رمز;
	smallint show_dirname;
#اگر ENABLE_FEATURE_LS_چوڑائی
	غیرعلامتی terminal_چوڑائی;
# define G_terminal_چوڑائی (G.terminal_چوڑائی)
#ورنہ
# define G_terminal_چوڑائی TERMINAL_چوڑائی
#اگر_ختم
#اگر ENABLE_FEATURE_LS_TIMESTAMPS
	/* Do time() just once. Saves one syscall per file for "ls -l" */
	وقت_t current_وقت_t;
#اگر_ختم
} FIX_ALIASING;
#تعریف_کرو G (*(ساخت globals*)bb_common_bufsiz1)
#تعریف_کرو INIT_G() کرو { \
	تیاری_common_bufsiz(); \
	/* we have to zero it out because of NOEXEC */ \
	memset(&G, 0, حجم(G)); \
	IF_FEATURE_LS_چوڑائی(G_terminal_چوڑائی = TERMINAL_چوڑائی;) \
	IF_FEATURE_LS_TIMESTAMPS(وقت(&G.current_وقت_t);) \
} جب (0)

#تعریف_کرو ESC "\033"


/*** Output code ***/


/* FYI type values: 1:fifo 2:char 4:dir 6:blk 8:file 10:link 12:socket
 * (various wacky OSes: 13:Sun door 14:BSD whiteout 5:XENIX named file
 *  3/7:multiplexed char/block device)
 * and we use 0 for unknown and 15 for executables (see below) */
#تعریف_کرو TYPEINDEX(منوال) (((منوال) >> 12) & 0x0f)
/*                       un  fi chr -   dir -  blk  -  file -  link - sock -   - exe */
#تعریف_کرو APPCHAR(منوال)   ("\0""|""\0""\0""/""\0""\0""\0""\0""\0""@""\0""=""\0""\0""\0" [TYPEINDEX(منوال)])
/* 036 black foreground              050 black background
   037 red foreground                051 red background
   040 green foreground              052 green background
   041 brown foreground              053 brown background
   042 blue foreground               054 blue background
   043 magenta (purple) foreground   055 magenta background
   044 cyan (light blue) foreground  056 cyan background
   045 gray foreground               057 white background
*/
#تعریف_کرو COLOR(منوال) ( \
	/*un  fi  chr  -  dir  -  blk  -  file -  link -  sock -   -  exe */ \
	"\037\043\043\045\042\045\043\043\000\045\044\045\043\045\045\040" \
	[TYPEINDEX(منوال)])
/* Select normal (0) [actually "reset all"] or bold (1)
 * (other attributes are 2:dim 4:underline 5:blink 7:reverse,
 *  let's use 7 for "impossible" types, just for fun)
 * Note: coreutils 6.9 uses inverted red for setuid binaries.
 */
#تعریف_کرو ATTR(منوال) ( \
	/*un fi chr - dir - blk - file- link- sock- -  exe */ \
	"\01\00\01\07\01\07\01\07\00\07\01\07\01\07\07\01" \
	[TYPEINDEX(منوال)])

#اگر ENABLE_FEATURE_LS_COLOR
/* mode of zero is interpreted as "unknown" (stat failed) */
ساکن حرف fgcolor(منوال_t منوال)
{
	اگر (S_ISREG(منوال) && (منوال & (S_IXUSR | S_IXGRP | S_IXOTH)))
		واپس COLOR(0xF000);	/* File is executable ... */
	واپس COLOR(منوال);
}
ساکن حرف bold(منوال_t منوال)
{
	اگر (S_ISREG(منوال) && (منوال & (S_IXUSR | S_IXGRP | S_IXOTH)))
		واپس ATTR(0xF000);	/* File is executable ... */
	واپس ATTR(منوال);
}
#اگر_ختم

#اگر ENABLE_FEATURE_LS_FILETYPES
ساکن حرف append_char(منوال_t منوال)
{
	اگر (!(option_mask32 & (OPT_F|OPT_p)))
		واپس '\0';

	اگر (S_ISDIR(منوال))
		واپس '/';
	اگر (!(option_mask32 & OPT_F))
		واپس '\0';
	اگر (S_ISREG(منوال) && (منوال & (S_IXUSR | S_IXGRP | S_IXOTH)))
		واپس '*';
	واپس APPCHAR(منوال);
}
#اگر_ختم

ساکن غیرعلامتی calc_name_len(ثابت حرف *name)
{
	غیرعلامتی len;
	uni_stat_t uni_stat;

	// TODO: quote tab as \t, etc, if -Q
	name = printable_string2(&uni_stat, name);

	اگر (!(option_mask32 & OPT_Q)) {
		واپس uni_stat.unicode_چوڑائی;
	}

	len = 2 + uni_stat.unicode_چوڑائی;
	جب (*name) {
		اگر (*name == '"' || *name == '\\') {
			len++;
		}
		name++;
	}
	واپس len;
}

/* Return the number of used columns.
 * Note that only columnar output uses return value.
 * -l and -1 modes don't care.
 * coreutils 7.2 also supports:
 * ls -b (--escape) = octal escapes (although it doesn't look like working)
 * ls -N (--literal) = not escape at all
 */
ساکن غیرعلامتی print_name(ثابت حرف *name)
{
	غیرعلامتی len;
	uni_stat_t uni_stat;

	// TODO: quote tab as \t, etc, if -Q
	name = printable_string2(&uni_stat, name);

	اگر (!(option_mask32 & OPT_Q)) {
		fputs_stdout(name);
		واپس uni_stat.unicode_چوڑائی;
	}

	len = 2 + uni_stat.unicode_چوڑائی;
	حرف_لکھو('"');
	جب (*name) {
		اگر (*name == '"' || *name == '\\') {
			حرف_لکھو('\\');
			len++;
		}
		حرف_لکھو(*name);
		name++;
	}
	حرف_لکھو('"');
	واپس len;
}

/* Return the number of used columns.
 * Note that only columnar output uses return value,
 * -l and -1 modes don't care.
 */
ساکن NOINLINE غیرعلامتی display_single(ثابت ساخت dnode *dn)
{
	غیرعلامتی عمود = 0;
	حرف *lpath;
	صحیح opt;
#اگر ENABLE_FEATURE_LS_FILETYPES || ENABLE_FEATURE_LS_COLOR
	ساخت stat statbuf;
#اگر_ختم
#اگر ENABLE_FEATURE_LS_FILETYPES
	حرف append = append_char(dn->dn_منوال);
#اگر_ختم

	opt = option_mask32;

	/* Do readlink early, so that if it fails, error message
	 * does not appear *inside* the "ls -l" line */
	lpath = تہی;
	اگر (opt & OPT_l)
		اگر (S_ISLNK(dn->dn_منوال))
			lpath = xmalloc_readlink_or_warn(dn->fullname);

	اگر (opt & OPT_i) /* show inode# */
		عمود += لکھو("%7llu ", (طویل طویل) dn->dn_ino);
	اگر (opt & OPT_s) { /* show allocated blocks */
		اگر (opt & OPT_h) {
			عمود += لکھو("%"HUMAN_READABLE_MAX_چوڑائی_STR"s ",
				/* print size, show one fractional, use suffixes */
				make_human_readable_str((off_t)dn->dn_بلاکیں << 9, 1, 0)
			);
		} ورنہ {
			عمود += لکھو("%6"OFF_FMT"u ", (off_t)(dn->dn_بلاکیں >> 1));
		}
	}
	اگر (opt & OPT_l) {
		/* long listing: show mode */
		حرف modestr[12];
		عمود += لکھو("%-10s ", bb_منوال_string(modestr, dn->dn_منوال));
		/* long listing: show number of links */
		عمود += لکھو("%4lu ", (طویل) dn->dn_nlink);
		/* long listing: show user/group */
		اگر (opt & OPT_n) {
			اگر (opt & OPT_g)
				عمود += لکھو("%-8u ", (صحیح) dn->dn_gid);
			ورنہ
				عمود += لکھو("%-8u %-8u ",
						(صحیح) dn->dn_uid,
						(صحیح) dn->dn_gid);
		}
#اگر ENABLE_FEATURE_LS_USERNAME
		ورنہ {
			اگر (opt & OPT_g) {
				عمود += لکھو("%-8s ",
					get_cached_groupname(dn->dn_gid));
			} ورنہ {
				عمود += لکھو("%-8s %-8s ",
					get_cached_username(dn->dn_uid),
					get_cached_groupname(dn->dn_gid));
			}
		}
#اگر_ختم
#اگر ENABLE_SELINUX
	}
	اگر (opt & OPT_Z) {
		عمود += لکھو("%-32s ", dn->sid ? dn->sid : "?");
		freecon(dn->sid);
	}
	اگر (opt & OPT_l) {
#اگر_ختم
		/* long listing: show size */
		اگر (S_ISBLK(dn->dn_منوال) || S_ISCHR(dn->dn_منوال)) {
			عمود += لکھو("%4u, %3u ",
					dn->dn_rdev_maj,
					dn->dn_rdev_min);
		} ورنہ {
			اگر (opt & OPT_h) {
				عمود += لکھو("%"HUMAN_READABLE_MAX_چوڑائی_STR"s ",
					/* print size, show one fractional, use suffixes */
					make_human_readable_str(dn->dn_سائز, 1, 0)
				);
			} ورنہ {
				عمود += لکھو("%9"OFF_FMT"u ", dn->dn_سائز);
			}
		}
#اگر ENABLE_FEATURE_LS_TIMESTAMPS
		/* long listing: show {m,c,a}time */
		اگر (opt & OPT_بھرا_وقت) { /* --full-time */
			/* coreutils 8.4 ls --full-time prints:
			 * 2009-07-13 17:49:27.000000000 +0200
			 * we don't show fractional seconds.
			 */
			حرف buf[حجم("YYYY-mm-dd HH:MM:SS TIMEZONE")];
			strftime(buf, حجم(buf), "%Y-%m-%d %H:%M:%S %z",
					localtime(&dn->dn_وقت));
			عمود += لکھو("%s ", buf);
		} ورنہ { /* ordinary time format */
			/* G.current_time_t is ~== time(NULL) */
			حرف *filetime = ctime(&dn->dn_وقت);
			/* filetime's format: "Wed Jun 30 21:49:08 1993\n" */
			وقت_t age = G.current_وقت_t - dn->dn_وقت;
			اگر (age < 3600L * 24 * 365 / 2 && age > -15 * 60) {
				/* less than 6 months old */
				/* "mmm dd hh:mm " */
				لکھو("%.12s ", filetime + 4);
			} ورنہ {
				/* "mmm dd  yyyy " */
				/* "mmm dd yyyyy " after year 9999 :) */
				strchr(filetime + 20, '\n')[0] = ' ';
				لکھو("%.7s%6s", filetime + 4, filetime + 20);
			}
			عمود += 13;
		}
#اگر_ختم
	}

#اگر ENABLE_FEATURE_LS_COLOR
	اگر (G_show_color) {
		منوال_t منوال = dn->dn_منوال_lstat;
		اگر (!منوال)
			اگر (lstat(dn->fullname, &statbuf) == 0)
				منوال = statbuf.st_منوال;
		لکھو(ESC"[%u;%um", bold(منوال), fgcolor(منوال));
	}
#اگر_ختم
	عمود += print_name(dn->name);
	اگر (G_show_color) {
		لکھو(ESC"[m");
	}

	اگر (lpath) {
		لکھو(" -> ");
#اگر ENABLE_FEATURE_LS_FILETYPES || ENABLE_FEATURE_LS_COLOR
		اگر ((opt & (OPT_F|OPT_p))
		 || G_show_color
		) {
			منوال_t منوال = dn->dn_منوال_stat;
			اگر (!منوال)
				اگر (stat(dn->fullname, &statbuf) == 0)
					منوال = statbuf.st_منوال;
# اگر ENABLE_FEATURE_LS_FILETYPES
			append = append_char(منوال);
# endif
# اگر ENABLE_FEATURE_LS_COLOR
			اگر (G_show_color) {
				لکھو(ESC"[%u;%um", bold(منوال), fgcolor(منوال));
			}
# endif
		}
#اگر_ختم
		عمود += print_name(lpath) + 4;
		آزاد(lpath);
		اگر (G_show_color) {
			لکھو(ESC"[m");
		}
	}
#اگر ENABLE_FEATURE_LS_FILETYPES
	اگر (opt & (OPT_F|OPT_p)) {
		اگر (append) {
			حرف_لکھو(append);
			عمود++;
		}
	}
#اگر_ختم

	واپس عمود;
}

ساکن خالی display_files(ساخت dnode **dn, غیرعلامتی nfiles)
{
	غیرعلامتی i, ncols, nrows, صف, nc;
	غیرعلامتی عمود;
	غیرعلامتی nexttab;
	غیرعلامتی عمود_چوڑائی = 0; /* used only by coulmnal output */

	اگر (option_mask32 & (OPT_l|OPT_1)) {
		ncols = 1;
	} ورنہ {
		/* find the longest file name, use that as the column width */
		برائے (i = 0; dn[i]; i++) {
			صحیح len = calc_name_len(dn[i]->name);
			اگر (عمود_چوڑائی < len)
				عمود_چوڑائی = len;
		}
		عمود_چوڑائی += 2
			+ ((option_mask32 & OPT_Z) ? 33 : 0) /* context width */
			+ ((option_mask32 & OPT_i) ? 8 : 0) /* inode# width */
			+ ((option_mask32 & OPT_s) ? 5 : 0) /* "alloc block" width */
		;
		ncols = (غیرعلامتی)G_terminal_چوڑائی / عمود_چوڑائی;
	}

	اگر (ncols > 1) {
		nrows = nfiles / ncols;
		اگر (nrows * ncols < nfiles)
			nrows++;                /* round up fractionals */
	} ورنہ {
		nrows = nfiles;
		ncols = 1;
	}

	عمود = 0;
	nexttab = 0;
	برائے (صف = 0; صف < nrows; صف++) {
		برائے (nc = 0; nc < ncols; nc++) {
			/* reach into the array based on the column and row */
			اگر (option_mask32 & OPT_x)
				i = (صف * ncols) + nc;	/* display across row */
			ورنہ
				i = (nc * nrows) + صف;	/* display by column */
			اگر (i < nfiles) {
				اگر (عمود > 0) {
					nexttab -= عمود;
					لکھو("%*s", nexttab, "");
					عمود += nexttab;
				}
				nexttab = عمود + عمود_چوڑائی;
				عمود += display_single(dn[i]);
			}
		}
		حرف_لکھو('\n');
		عمود = 0;
	}
}


/*** Dir scanning code ***/

ساکن ساخت dnode *my_stat(ثابت حرف *fullname, ثابت حرف *name, صحیح force_follow)
{
	ساخت stat statbuf;
	ساخت dnode *cur;

	cur = xzalloc(حجم(*cur));
	cur->fullname = fullname;
	cur->name = name;

	اگر ((option_mask32 & OPT_L) || force_follow) {
#اگر ENABLE_SELINUX
		اگر (option_mask32 & OPT_Z) {
			getfilecon(fullname, &cur->sid);
		}
#اگر_ختم
		اگر (stat(fullname, &statbuf)) {
			bb_simple_perror_پیغام(fullname);
			G.exit_رمز = EXIT_FAILURE;
			آزاد(cur);
			واپس تہی;
		}
		cur->dn_منوال_stat = statbuf.st_منوال;
	} ورنہ {
#اگر ENABLE_SELINUX
		اگر (option_mask32 & OPT_Z) {
			lgetfilecon(fullname, &cur->sid);
		}
#اگر_ختم
		اگر (lstat(fullname, &statbuf)) {
			bb_simple_perror_پیغام(fullname);
			G.exit_رمز = EXIT_FAILURE;
			آزاد(cur);
			واپس تہی;
		}
		cur->dn_منوال_lstat = statbuf.st_منوال;
	}

	/* cur->dstat = statbuf: */
	cur->dn_منوال   = statbuf.st_منوال  ;
	cur->dn_سائز   = statbuf.st_سائز  ;
#اگر ENABLE_FEATURE_LS_TIMESTAMPS || ENABLE_FEATURE_LS_SORTFILES
	cur->dn_وقت   = statbuf.st_mtime ;
	اگر (option_mask32 & OPT_u)
		cur->dn_وقت = statbuf.st_atime;
	اگر (option_mask32 & OPT_c)
		cur->dn_وقت = statbuf.st_ctime;
#اگر_ختم
	cur->dn_ino    = statbuf.st_ino   ;
	cur->dn_بلاکیں = statbuf.st_بلاکیں;
	cur->dn_nlink  = statbuf.st_nlink ;
	cur->dn_uid    = statbuf.st_uid   ;
	cur->dn_gid    = statbuf.st_gid   ;
	cur->dn_rdev_maj = major(statbuf.st_rdev);
	cur->dn_rdev_min = minor(statbuf.st_rdev);

	واپس cur;
}

ساکن غیرعلامتی گنتی_dirs(ساخت dnode **dn, صحیح which)
{
	غیرعلامتی dirs, all;

	اگر (!dn)
		واپس 0;

	dirs = all = 0;
	برائے (; *dn; dn++) {
		ثابت حرف *name;

		all++;
		اگر (!S_ISDIR((*dn)->dn_منوال))
			جاری;

		name = (*dn)->name;
		اگر (which != بانٹو_SUBDIR /* if not requested to skip . / .. */
		 /* or if it's not . or .. */
		 || name[0] != '.'
		 || (name[1] && (name[1] != '.' || name[2]))
		) {
			dirs++;
		}
	}
	واپس which != بانٹو_FILE ? dirs : all - dirs;
}

/* get memory to hold an array of pointers */
ساکن ساخت dnode **dnalloc(غیرعلامتی عدد)
{
	اگر (عدد < 1)
		واپس تہی;

	عدد++; /* so that we have terminating NULL */
	واپس xzalloc(عدد * حجم(ساخت dnode *));
}

#اگر ENABLE_FEATURE_LS_RECURSIVE
ساکن خالی dfree(ساخت dnode **dnp)
{
	غیرعلامتی i;

	اگر (dnp == تہی)
		واپس;

	برائے (i = 0; dnp[i]; i++) {
		ساخت dnode *cur = dnp[i];
		اگر (cur->fname_allocated)
			آزاد((حرف*)cur->fullname);
		آزاد(cur);
	}
	آزاد(dnp);
}
#ورنہ
#تعریف_کرو dfree(...) ((خالی)0)
#اگر_ختم

/* Returns NULL-terminated malloced vector of pointers (or NULL) */
ساکن ساخت dnode **splitdnarray(ساخت dnode **dn, صحیح which)
{
	غیرعلامتی dncnt, d;
	ساخت dnode **dnp;

	اگر (dn == تہی)
		واپس تہی;

	/* count how many dirs or files there are */
	dncnt = گنتی_dirs(dn, which);

	/* allocate a file array and a dir array */
	dnp = dnalloc(dncnt);

	/* copy the entrys into the file or dir array */
	برائے (d = 0; *dn; dn++) {
		اگر (S_ISDIR((*dn)->dn_منوال)) {
			ثابت حرف *name;

			اگر (which == بانٹو_FILE)
				جاری;

			name = (*dn)->name;
			اگر ((which & بانٹو_DIR) /* any dir... */
			/* ... or not . or .. */
			 || name[0] != '.'
			 || (name[1] && (name[1] != '.' || name[2]))
			) {
				dnp[d++] = *dn;
			}
		} ورنہ
		اگر (which == بانٹو_FILE) {
			dnp[d++] = *dn;
		}
	}
	واپس dnp;
}

#اگر ENABLE_FEATURE_LS_SORTFILES
ساکن صحیح sortcmp(ثابت خالی *a, ثابت خالی *b)
{
	ساخت dnode *d1 = *(ساخت dnode **)a;
	ساخت dnode *d2 = *(ساخت dnode **)b;
	غیرعلامتی opt = option_mask32;
	off_t dif;

	dif = 0; /* assume sort by name */
	// TODO: use pre-initialized function pointer
	// instead of branch forest
	اگر (opt & OPT_dirs_پہلا) {
		dif = S_ISDIR(d2->dn_منوال) - S_ISDIR(d1->dn_منوال);
		اگر (dif != 0)
			جاؤ maybe_invert_and_ret;
	}

	اگر (opt & OPT_S) { /* sort by size */
		dif = (d2->dn_سائز - d1->dn_سائز);
	} ورنہ
	اگر (opt & OPT_t) { /* sort by time */
		dif = (d2->dn_وقت - d1->dn_وقت);
	} ورنہ
#اگر defined(HAVE_STRVERSCMP) && HAVE_STRVERSCMP == 1
	اگر (opt & OPT_v) { /* sort by version */
		dif = strverscmp(d1->name, d2->name);
	} ورنہ
#اگر_ختم
	اگر (opt & OPT_X) { /* sort by extension */
		dif = strcmp(strchrnul(d1->name, '.'), strchrnul(d2->name, '.'));
	}
	اگر (dif == 0) {
		/* sort by name, use as tie breaker for other sorts */
		اگر (ENABLE_LOCALE_SUPPORT)
			dif = strcoll(d1->name, d2->name);
		ورنہ
			dif = strcmp(d1->name, d2->name);
	} ورنہ {
		/* Make dif fit into an int */
		اگر (حجم(dif) > حجم(صحیح)) {
			شمار { بٹ_مقصد_سرکاؤ = 8 * (حجم(dif) - حجم(صحیح)) };
			/* shift leaving only "int" worth of bits */
			/* (this requires dif != 0, and here it is nonzero) */
			dif = 1 | (صحیح)((uoff_t)dif >> بٹ_مقصد_سرکاؤ);
		}
	}
 maybe_invert_and_ret:
	واپس (opt & OPT_r) ? -(صحیح)dif : (صحیح)dif;
}

ساکن خالی dnsort(ساخت dnode **dn, صحیح سائز)
{
	qsort(dn, سائز, حجم(*dn), sortcmp);
}

ساکن خالی ترتیب_دو_and_display_files(ساخت dnode **dn, غیرعلامتی nfiles)
{
	dnsort(dn, nfiles);
	display_files(dn, nfiles);
}
#ورنہ
# define dnsort(dn, سائز) ((خالی)0)
# define ترتیب_دو_and_display_files(dn, nfiles) display_files(dn, nfiles)
#اگر_ختم

/* Returns NULL-terminated malloced vector of pointers (or NULL) */
ساکن ساخت dnode **جائزہ_one_dir(ثابت حرف *راستہ, غیرعلامتی *nfiles_p)
{
	ساخت dnode *dn, *cur, **dnp;
	ساخت dirent *اندراج_2;
	DIR *dir;
	غیرعلامتی i, nfiles;

	*nfiles_p = 0;
	dir = warn_opendir(راستہ);
	اگر (dir == تہی) {
		G.exit_رمز = EXIT_FAILURE;
		واپس تہی;	/* could not open the dir */
	}
	dn = تہی;
	nfiles = 0;
	جب ((اندراج_2 = readdir(dir)) != تہی) {
		حرف *fullname;

		/* are we going to list the file- it may be . or .. or a hidden file */
		اگر (اندراج_2->d_name[0] == '.') {
			اگر (!(option_mask32 & (OPT_a|OPT_A)))
				جاری; /* skip all dotfiles if no -a/-A */
			اگر (!(option_mask32 & OPT_a)
			 && (!اندراج_2->d_name[1] || (اندراج_2->d_name[1] == '.' && !اندراج_2->d_name[2]))
			) {
				جاری; /* if only -A, skip . and .. but show other dotfiles */
			}
		}
		fullname = concat_راستہ_file(راستہ, اندراج_2->d_name);
		cur = my_stat(fullname, bb_basename(fullname), 0);
		اگر (!cur) {
			آزاد(fullname);
			جاری;
		}
		cur->fname_allocated = 1;
		cur->dn_اگلا = dn;
		dn = cur;
		nfiles++;
	}
	closedir(dir);

	اگر (dn == تہی)
		واپس تہی;

	/* now that we know how many files there are
	 * allocate memory for an array to hold dnode pointers
	 */
	*nfiles_p = nfiles;
	dnp = dnalloc(nfiles);
	برائے (i = 0; /* i < nfiles - detected via !dn below */; i++) {
		dnp[i] = dn;	/* save pointer to node in array */
		dn = dn->dn_اگلا;
		اگر (!dn)
			توڑو;
	}

	واپس dnp;
}

#اگر ENABLE_DESKTOP
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
ساکن off_t calculate_بلاکیں(ساخت dnode **dn)
{
	uoff_t بلاکیں = 1;
	اگر (dn) {
		جب (*dn) {
			/* st_blocks is in 512 byte blocks */
			بلاکیں += (*dn)->dn_بلاکیں;
			dn++;
		}
	}

	/* Even though standard says use 512 byte blocks, coreutils use 1k */
	/* Actually, we round up by calculating (blocks + 1) / 2,
	 * "+ 1" was done when we initialized blocks to 1 */
	واپس بلاکیں >> 1;
}
#اگر_ختم

ساکن خالی جائزہ_and_display_dirs_recur(ساخت dnode **dn, صحیح پہلا)
{
	غیرعلامتی nfiles;
	ساخت dnode **subdnp;

	برائے (; *dn; dn++) {
		اگر (G.show_dirname || (option_mask32 & OPT_R)) {
			اگر (!پہلا)
				bb_putchar('\n');
			پہلا = 0;
			لکھو("%s:\n", (*dn)->fullname);
		}
		subdnp = جائزہ_one_dir((*dn)->fullname, &nfiles);
#اگر ENABLE_DESKTOP
		اگر (option_mask32 & (OPT_s|OPT_l)) {
			اگر (option_mask32 & OPT_h) {
				لکھو("total %-"HUMAN_READABLE_MAX_چوڑائی_STR"s\n",
					/* print size, no fractions, use suffixes */
					make_human_readable_str(calculate_بلاکیں(subdnp) * 1024,
								0, 0)
				);
			} ورنہ {
				لکھو("total %"OFF_FMT"u\n", calculate_بلاکیں(subdnp));
			}
		}
#اگر_ختم
		اگر (nfiles > 0) {
			/* list all files at this level */
			ترتیب_دو_and_display_files(subdnp, nfiles);

			اگر (ENABLE_FEATURE_LS_RECURSIVE
			 && (option_mask32 & OPT_R)
			) {
				ساخت dnode **dnd;
				غیرعلامتی dndirs;
				/* recursive - list the sub-dirs */
				dnd = splitdnarray(subdnp, بانٹو_SUBDIR);
				dndirs = گنتی_dirs(subdnp, بانٹو_SUBDIR);
				اگر (dndirs > 0) {
					dnsort(dnd, dndirs);
					جائزہ_and_display_dirs_recur(dnd, 0);
					/* free the array of dnode pointers to the dirs */
					آزاد(dnd);
				}
			}
			/* free the dnodes and the fullname mem */
			dfree(subdnp);
		}
	}
}


صحیح ls_main(صحیح argc غیر_مستعمل_PARAM, حرف **argv)
{	/*      ^^^^^^^^^^^^^^^^^ note: if FTPD, argc can be wrong, see ftpd.c */
	ساخت dnode **dnd;
	ساخت dnode **dnf;
	ساخت dnode **dnp;
	ساخت dnode *dn;
	ساخت dnode *cur;
	غیرعلامتی opt;
	غیرعلامتی nfiles;
	غیرعلامتی dnfiles;
	غیرعلامتی dndirs;
	غیرعلامتی i;
#اگر ENABLE_FEATURE_LS_COLOR
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
	ساکن ثابت حرف color_str[] ALIGN1 =
		"always\0""yes\0""force\0"
		"auto\0""tty\0""if-tty\0";
	/* need to initialize since --color has _an optional_ argument */
	ثابت حرف *color_opt = color_str; /* "always" */
#اگر_ختم
#اگر ENABLE_LONG_OPTS
	ساکن ثابت حرف ls_longopts[] ALIGN1 =
		"full-time\0" No_دلیل "\xff"
		"group-directories-first\0" No_دلیل "\xfe"
		IF_FEATURE_LS_COLOR("color\0" Optional_دلیل "\xfd")
	;
#اگر_ختم

	INIT_G();

	init_unicode();

#اگر ENABLE_FEATURE_LS_چوڑائی
	/* obtain the terminal width */
	G_terminal_چوڑائی = get_terminal_چوڑائی(STDIN_FILENO);
	/* go one less... */
	G_terminal_چوڑائی--;
#اگر_ختم

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
			IF_FEATURE_LS_چوڑائی(":w+")
		, ls_longopts
		IF_FEATURE_LS_چوڑائی(, /*-T*/ تہی, /*-w*/ &G_terminal_چوڑائی)
		IF_FEATURE_LS_COLOR(, &color_opt)
	);
#اگر 0 /* option bits debug */
	bb_error_پیغام("opt:0x%08x l:%x H:%x color:%x dirs:%x", opt, OPT_l, OPT_H, OPT_color, OPT_dirs_پہلا);
	اگر (opt & OPT_c         ) bb_error_پیغام("-c");
	اگر (opt & OPT_l         ) bb_error_پیغام("-l");
	اگر (opt & OPT_H         ) bb_error_پیغام("-H");
	اگر (opt & OPT_color     ) bb_error_پیغام("--color");
	اگر (opt & OPT_dirs_پہلا) bb_error_پیغام("--group-directories-first");
	اگر (opt & OPT_بھرا_وقت ) bb_error_پیغام("--full-time");
	باہر_نکلو(0);
#اگر_ختم

#اگر ENABLE_SELINUX
	اگر (opt & OPT_Z) {
		اگر (!is_selinux_enabled())
			option_mask32 &= ~OPT_Z;
	}
#اگر_ختم

#اگر ENABLE_FEATURE_LS_COLOR
	/* set G_show_color = 1/0 */
	اگر (ENABLE_FEATURE_LS_COLOR_IS_DEFAULT && !is_رکن_dumb()) {
		حرف *p = getenv("LS_COLORS");
		/* LS_COLORS is unset, or (not empty && not "none") ? */
		اگر (!p || (p[0] && strcmp(p, "none") != 0)) {
			اگر (isatty(STDOUT_FILENO)) {
				/* check isatty() last because it's expensive (syscall) */
				G_show_color = 1;
			}
		}
	}
	اگر (opt & OPT_color) {
		اگر (color_opt[0] == 'n')
			G_show_color = 0;
		ورنہ انتخاب (اشاریہ_اندر_substrings(color_str, color_opt)) {
		حالت 3:
		حالت 4:
		حالت 5:
			اگر (!is_رکن_dumb() && isatty(STDOUT_FILENO)) {
		حالت 0:
		حالت 1:
		حالت 2:
				G_show_color = 1;
			}
		}
	}
#اگر_ختم

	/* sort out which command line options take precedence */
	اگر (ENABLE_FEATURE_LS_RECURSIVE && (opt & OPT_d))
		option_mask32 &= ~OPT_R;	/* no recurse if listing only dir */
	اگر (!(opt & OPT_l)) { /* not -l? */
		اگر (ENABLE_FEATURE_LS_TIMESTAMPS && ENABLE_FEATURE_LS_SORTFILES) {
			/* when to sort by time? -t[cu] sorts by time even with -l */
			/* (this is achieved by opt_flags[] element for -t) */
			/* without -l, bare -c or -u enable sort too */
			/* (with -l, bare -c or -u just select which time to show) */
			اگر (opt & (OPT_c|OPT_u)) {
				option_mask32 |= OPT_t;
			}
		}
	}

	/* choose a display format if one was not already specified by an option */
	اگر (!(option_mask32 & (OPT_l|OPT_1|OPT_x|OPT_C)))
		option_mask32 |= (isatty(STDOUT_FILENO) ? OPT_C : OPT_1);

	اگر (ENABLE_FTPD && applet_name[0] == 'f') {
		/* ftpd secret backdoor. dirs first are much nicer */
		option_mask32 |= OPT_dirs_پہلا;
	}

	argv += optind;
	اگر (!argv[0])
		*--argv = (حرف*)".";

	اگر (argv[1])
		G.show_dirname = 1; /* 2 or more items? label directories */

	/* stuff the command line file names into a dnode array */
	dn = تہی;
	nfiles = 0;
	کرو {
		cur = my_stat(*argv, *argv,
			/* follow links on command line unless -l, -i, -s or -F: */
			!(option_mask32 & (OPT_l|OPT_i|OPT_s|OPT_F))
			/* ... or if -H: */
			|| (option_mask32 & OPT_H)
			/* ... or if -L, but my_stat always follows links if -L */
		);
		argv++;
		اگر (!cur)
			جاری;
		/*cur->fname_allocated = 0; - already is */
		cur->dn_اگلا = dn;
		dn = cur;
		nfiles++;
	} جب (*argv);

	/* nfiles _may_ be 0 here - try "ls doesnt_exist" */
	اگر (nfiles == 0)
		واپس G.exit_رمز;

	/* now that we know how many files there are
	 * allocate memory for an array to hold dnode pointers
	 */
	dnp = dnalloc(nfiles);
	برائے (i = 0; /* i < nfiles - detected via !dn below */; i++) {
		dnp[i] = dn;	/* save pointer to node in array */
		dn = dn->dn_اگلا;
		اگر (!dn)
			توڑو;
	}

	اگر (option_mask32 & OPT_d) {
		ترتیب_دو_and_display_files(dnp, nfiles);
	} ورنہ {
		dnd = splitdnarray(dnp, بانٹو_DIR);
		dnf = splitdnarray(dnp, بانٹو_FILE);
		dndirs = گنتی_dirs(dnp, بانٹو_DIR);
		dnfiles = nfiles - dndirs;
		اگر (dnfiles > 0) {
			ترتیب_دو_and_display_files(dnf, dnfiles);
			اگر (ENABLE_FEATURE_CLEAN_اوپر)
				آزاد(dnf);
		}
		اگر (dndirs > 0) {
			dnsort(dnd, dndirs);
			جائزہ_and_display_dirs_recur(dnd, dnfiles == 0);
			اگر (ENABLE_FEATURE_CLEAN_اوپر)
				آزاد(dnd);
		}
	}

	اگر (ENABLE_FEATURE_CLEAN_اوپر)
		dfree(dnp);
	واپس G.exit_رمز;
}
