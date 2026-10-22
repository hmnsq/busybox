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

#समावेश "libbb.h"
#समावेश "common_bufsiz.h"
#समावेश "unicode.h"


/* This is a NOEXEC applet. Be very careful! */


#अगर ENABLE_FTPD
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
#पूर्ण_अगर


क्रमागत {
TERMINAL_WIDTH  = 80, /* use 79 if terminal has linefold bug */

SPLIT_FILE      = 0,
SPLIT_DIR       = 1,
SPLIT_SUBDIR    = 2,
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
#घोषणा ls_options \
	"Cadi1lgnsxAk"       /* 12 opts, total 12 */ \
	IF_FEATURE_LS_FILETYPES("Fp")    /* 2, 14 */ \
	IF_FEATURE_LS_RECURSIVE("R")     /* 1, 15 */ \
	IF_SELINUX("Z")                  /* 1, 16 */ \
	"Q"                              /* 1, 17 */ \
	IF_FEATURE_LS_TIMESTAMPS("ctu")  /* 3, 20 */ \
	IF_FEATURE_LS_SORTFILES("SXrv")  /* 4, 24 */ \
	IF_FEATURE_LS_FOLLOWLINKS("LH")  /* 2, 26 */ \
	IF_FEATURE_HUMAN_READABLE("h")   /* 1, 27 */ \
	IF_FEATURE_LS_WIDTH("T:w:")      /* 2, 29 */

क्रमागत {
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
	OPTBIT_full_time = OPTBIT_T + 2 * ENABLE_FEATURE_LS_WIDTH,
	OPTBIT_dirs_first,
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
	OPT_T = (1 << OPTBIT_T) * ENABLE_FEATURE_LS_WIDTH,
	OPT_w = (1 << OPTBIT_w) * ENABLE_FEATURE_LS_WIDTH,
	OPT_full_time  = (1 << OPTBIT_full_time ) * ENABLE_LONG_OPTS,
	OPT_dirs_first = (1 << OPTBIT_dirs_first) * ENABLE_LONG_OPTS,
	OPT_color      = (1 << OPTBIT_color     ) * ENABLE_FEATURE_LS_COLOR,
};

/*
 * a directory entry and its stat info
 */
काष्ठा dnode {
	स्थिर अक्षर *name;       /* usually basename, but think "ls -l dir/file" */
	स्थिर अक्षर *fullname;   /* full name (usable for stat etc) */
	काष्ठा dnode *dn_अग्र;  /* for linked list */
	IF_SELINUX(security_context_t sid;)
	smallint fname_allocated;

	/* Used to avoid re-doing [l]stat at printout stage
	 * if we already collected needed data in scan stage:
	 */
	mode_t    dn_mode_lstat;   /* obtained with lstat, or 0 */
	mode_t    dn_mode_stat;    /* obtained with stat, or 0 */

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
	mode_t    dn_mode; /* obtained with lstat OR stat, depending on -L etc */
	off_t     dn_आकार;
#अगर ENABLE_FEATURE_LS_TIMESTAMPS || ENABLE_FEATURE_LS_SORTFILES
	समय_प्रकार    dn_time;
#पूर्ण_अगर
	ino_t     dn_ino;
	blkcnt_t  dn_blocks;
	nlink_t   dn_nlink;
	uid_t     dn_uid;
	gid_t     dn_gid;
	पूर्णांक       dn_rdev_maj;
	पूर्णांक       dn_rdev_min;
//	dev_t     dn_dev;
//	blksize_t dn_blksize;
};

काष्ठा globals {
#अगर ENABLE_FEATURE_LS_COLOR
	smallint show_color;
# define G_show_color (G.show_color)
#अन्यथा
# define G_show_color 0
#पूर्ण_अगर
	smallint exit_code;
	smallint show_dirname;
#अगर ENABLE_FEATURE_LS_WIDTH
	अचिन्हित terminal_width;
# define G_terminal_width (G.terminal_width)
#अन्यथा
# define G_terminal_width TERMINAL_WIDTH
#पूर्ण_अगर
#अगर ENABLE_FEATURE_LS_TIMESTAMPS
	/* Do time() just once. Saves one syscall per file for "ls -l" */
	समय_प्रकार current_time_t;
#पूर्ण_अगर
} FIX_ALIASING;
#घोषणा G (*(काष्ठा globals*)bb_common_bufsiz1)
#घोषणा INIT_G() करो { \
	setup_common_bufsiz(); \
	/* we have to zero it out because of NOEXEC */ \
	स_रखो(&G, 0, माप(G)); \
	IF_FEATURE_LS_WIDTH(G_terminal_width = TERMINAL_WIDTH;) \
	IF_FEATURE_LS_TIMESTAMPS(समय(&G.current_time_t);) \
} जबतक (0)

#घोषणा ESC "\033"


/*** Output code ***/


/* FYI type values: 1:fifo 2:char 4:dir 6:blk 8:file 10:link 12:socket
 * (various wacky OSes: 13:Sun door 14:BSD whiteout 5:XENIX named file
 *  3/7:multiplexed char/block device)
 * and we use 0 for unknown and 15 for executables (see below) */
#घोषणा TYPEINDEX(mode) (((mode) >> 12) & 0x0f)
/*                       un  fi chr -   dir -  blk  -  file -  link - sock -   - exe */
#घोषणा APPCHAR(mode)   ("\0""|""\0""\0""/""\0""\0""\0""\0""\0""@""\0""=""\0""\0""\0" [TYPEINDEX(mode)])
/* 036 black foreground              050 black background
   037 red foreground                051 red background
   040 green foreground              052 green background
   041 brown foreground              053 brown background
   042 blue foreground               054 blue background
   043 magenta (purple) foreground   055 magenta background
   044 cyan (light blue) foreground  056 cyan background
   045 gray foreground               057 white background
*/
#घोषणा COLOR(mode) ( \
	/*un  fi  chr  -  dir  -  blk  -  file -  link -  sock -   -  exe */ \
	"\037\043\043\045\042\045\043\043\000\045\044\045\043\045\045\040" \
	[TYPEINDEX(mode)])
/* Select normal (0) [actually "reset all"] or bold (1)
 * (other attributes are 2:dim 4:underline 5:blink 7:reverse,
 *  let's use 7 for "impossible" types, just for fun)
 * Note: coreutils 6.9 uses inverted red for setuid binaries.
 */
#घोषणा ATTR(mode) ( \
	/*un fi chr - dir - blk - file- link- sock- -  exe */ \
	"\01\00\01\07\01\07\01\07\00\07\01\07\01\07\07\01" \
	[TYPEINDEX(mode)])

#अगर ENABLE_FEATURE_LS_COLOR
/* mode of zero is interpreted as "unknown" (stat failed) */
अटल अक्षर fgcolor(mode_t mode)
{
	अगर (S_ISREG(mode) && (mode & (S_IXUSR | S_IXGRP | S_IXOTH)))
		वापस COLOR(0xF000);	/* File is executable ... */
	वापस COLOR(mode);
}
अटल अक्षर bold(mode_t mode)
{
	अगर (S_ISREG(mode) && (mode & (S_IXUSR | S_IXGRP | S_IXOTH)))
		वापस ATTR(0xF000);	/* File is executable ... */
	वापस ATTR(mode);
}
#पूर्ण_अगर

#अगर ENABLE_FEATURE_LS_FILETYPES
अटल अक्षर append_char(mode_t mode)
{
	अगर (!(option_mask32 & (OPT_F|OPT_p)))
		वापस '\0';

	अगर (S_ISDIR(mode))
		वापस '/';
	अगर (!(option_mask32 & OPT_F))
		वापस '\0';
	अगर (S_ISREG(mode) && (mode & (S_IXUSR | S_IXGRP | S_IXOTH)))
		वापस '*';
	वापस APPCHAR(mode);
}
#पूर्ण_अगर

अटल अचिन्हित calc_name_len(स्थिर अक्षर *name)
{
	अचिन्हित len;
	uni_stat_t uni_stat;

	// TODO: quote tab as \t, etc, if -Q
	name = printable_string2(&uni_stat, name);

	अगर (!(option_mask32 & OPT_Q)) {
		वापस uni_stat.unicode_width;
	}

	len = 2 + uni_stat.unicode_width;
	जबतक (*name) {
		अगर (*name == '"' || *name == '\\') {
			len++;
		}
		name++;
	}
	वापस len;
}

/* Return the number of used columns.
 * Note that only columnar output uses return value.
 * -l and -1 modes don't care.
 * coreutils 7.2 also supports:
 * ls -b (--escape) = octal escapes (although it doesn't look like working)
 * ls -N (--literal) = not escape at all
 */
अटल अचिन्हित print_name(स्थिर अक्षर *name)
{
	अचिन्हित len;
	uni_stat_t uni_stat;

	// TODO: quote tab as \t, etc, if -Q
	name = printable_string2(&uni_stat, name);

	अगर (!(option_mask32 & OPT_Q)) {
		fputs_stdout(name);
		वापस uni_stat.unicode_width;
	}

	len = 2 + uni_stat.unicode_width;
	अक्षर_दो('"');
	जबतक (*name) {
		अगर (*name == '"' || *name == '\\') {
			अक्षर_दो('\\');
			len++;
		}
		अक्षर_दो(*name);
		name++;
	}
	अक्षर_दो('"');
	वापस len;
}

/* Return the number of used columns.
 * Note that only columnar output uses return value,
 * -l and -1 modes don't care.
 */
अटल NOINLINE अचिन्हित display_single(स्थिर काष्ठा dnode *dn)
{
	अचिन्हित column = 0;
	अक्षर *lpath;
	पूर्णांक opt;
#अगर ENABLE_FEATURE_LS_FILETYPES || ENABLE_FEATURE_LS_COLOR
	काष्ठा stat statbuf;
#पूर्ण_अगर
#अगर ENABLE_FEATURE_LS_FILETYPES
	अक्षर append = append_char(dn->dn_mode);
#पूर्ण_अगर

	opt = option_mask32;

	/* Do readlink early, so that if it fails, error message
	 * does not appear *inside* the "ls -l" line */
	lpath = शून्य;
	अगर (opt & OPT_l)
		अगर (S_ISLNK(dn->dn_mode))
			lpath = xmalloc_readlink_or_warn(dn->fullname);

	अगर (opt & OPT_i) /* show inode# */
		column += म_लिखो("%7llu ", (दीर्घ दीर्घ) dn->dn_ino);
	अगर (opt & OPT_s) { /* show allocated blocks */
		अगर (opt & OPT_h) {
			column += म_लिखो("%"HUMAN_READABLE_MAX_WIDTH_STR"s ",
				/* print size, show one fractional, use suffixes */
				make_human_readable_str((off_t)dn->dn_blocks << 9, 1, 0)
			);
		} अन्यथा {
			column += म_लिखो("%6"OFF_FMT"u ", (off_t)(dn->dn_blocks >> 1));
		}
	}
	अगर (opt & OPT_l) {
		/* long listing: show mode */
		अक्षर modestr[12];
		column += म_लिखो("%-10s ", bb_mode_string(modestr, dn->dn_mode));
		/* long listing: show number of links */
		column += म_लिखो("%4lu ", (दीर्घ) dn->dn_nlink);
		/* long listing: show user/group */
		अगर (opt & OPT_n) {
			अगर (opt & OPT_g)
				column += म_लिखो("%-8u ", (पूर्णांक) dn->dn_gid);
			अन्यथा
				column += म_लिखो("%-8u %-8u ",
						(पूर्णांक) dn->dn_uid,
						(पूर्णांक) dn->dn_gid);
		}
#अगर ENABLE_FEATURE_LS_USERNAME
		अन्यथा {
			अगर (opt & OPT_g) {
				column += म_लिखो("%-8s ",
					get_cached_groupname(dn->dn_gid));
			} अन्यथा {
				column += म_लिखो("%-8s %-8s ",
					get_cached_username(dn->dn_uid),
					get_cached_groupname(dn->dn_gid));
			}
		}
#पूर्ण_अगर
#अगर ENABLE_SELINUX
	}
	अगर (opt & OPT_Z) {
		column += म_लिखो("%-32s ", dn->sid ? dn->sid : "?");
		freecon(dn->sid);
	}
	अगर (opt & OPT_l) {
#पूर्ण_अगर
		/* long listing: show size */
		अगर (S_ISBLK(dn->dn_mode) || S_ISCHR(dn->dn_mode)) {
			column += म_लिखो("%4u, %3u ",
					dn->dn_rdev_maj,
					dn->dn_rdev_min);
		} अन्यथा {
			अगर (opt & OPT_h) {
				column += म_लिखो("%"HUMAN_READABLE_MAX_WIDTH_STR"s ",
					/* print size, show one fractional, use suffixes */
					make_human_readable_str(dn->dn_आकार, 1, 0)
				);
			} अन्यथा {
				column += म_लिखो("%9"OFF_FMT"u ", dn->dn_आकार);
			}
		}
#अगर ENABLE_FEATURE_LS_TIMESTAMPS
		/* long listing: show {m,c,a}time */
		अगर (opt & OPT_full_time) { /* --full-time */
			/* coreutils 8.4 ls --full-time prints:
			 * 2009-07-13 17:49:27.000000000 +0200
			 * we don't show fractional seconds.
			 */
			अक्षर buf[माप("YYYY-mm-dd HH:MM:SS TIMEZONE")];
			स_माला(buf, माप(buf), "%Y-%m-%d %H:%M:%S %z",
					स_स्थानीय(&dn->dn_time));
			column += म_लिखो("%s ", buf);
		} अन्यथा { /* ordinary time format */
			/* G.current_time_t is ~== time(NULL) */
			अक्षर *filetime = स_समय(&dn->dn_time);
			/* filetime's format: "Wed Jun 30 21:49:08 1993\n" */
			समय_प्रकार age = G.current_time_t - dn->dn_time;
			अगर (age < 3600L * 24 * 365 / 2 && age > -15 * 60) {
				/* less than 6 months old */
				/* "mmm dd hh:mm " */
				म_लिखो("%.12s ", filetime + 4);
			} अन्यथा {
				/* "mmm dd  yyyy " */
				/* "mmm dd yyyyy " after year 9999 :) */
				म_अक्षर(filetime + 20, '\n')[0] = ' ';
				म_लिखो("%.7s%6s", filetime + 4, filetime + 20);
			}
			column += 13;
		}
#पूर्ण_अगर
	}

#अगर ENABLE_FEATURE_LS_COLOR
	अगर (G_show_color) {
		mode_t mode = dn->dn_mode_lstat;
		अगर (!mode)
			अगर (lstat(dn->fullname, &statbuf) == 0)
				mode = statbuf.st_mode;
		म_लिखो(ESC"[%u;%um", bold(mode), fgcolor(mode));
	}
#पूर्ण_अगर
	column += print_name(dn->name);
	अगर (G_show_color) {
		म_लिखो(ESC"[m");
	}

	अगर (lpath) {
		म_लिखो(" -> ");
#अगर ENABLE_FEATURE_LS_FILETYPES || ENABLE_FEATURE_LS_COLOR
		अगर ((opt & (OPT_F|OPT_p))
		 || G_show_color
		) {
			mode_t mode = dn->dn_mode_stat;
			अगर (!mode)
				अगर (stat(dn->fullname, &statbuf) == 0)
					mode = statbuf.st_mode;
# अगर ENABLE_FEATURE_LS_FILETYPES
			append = append_char(mode);
# endif
# अगर ENABLE_FEATURE_LS_COLOR
			अगर (G_show_color) {
				म_लिखो(ESC"[%u;%um", bold(mode), fgcolor(mode));
			}
# endif
		}
#पूर्ण_अगर
		column += print_name(lpath) + 4;
		मुक्त(lpath);
		अगर (G_show_color) {
			म_लिखो(ESC"[m");
		}
	}
#अगर ENABLE_FEATURE_LS_FILETYPES
	अगर (opt & (OPT_F|OPT_p)) {
		अगर (append) {
			अक्षर_दो(append);
			column++;
		}
	}
#पूर्ण_अगर

	वापस column;
}

अटल व्योम display_files(काष्ठा dnode **dn, अचिन्हित nfiles)
{
	अचिन्हित i, ncols, nrows, row, nc;
	अचिन्हित column;
	अचिन्हित nexttab;
	अचिन्हित column_width = 0; /* used only by coulmnal output */

	अगर (option_mask32 & (OPT_l|OPT_1)) {
		ncols = 1;
	} अन्यथा {
		/* find the longest file name, use that as the column width */
		क्रम (i = 0; dn[i]; i++) {
			पूर्णांक len = calc_name_len(dn[i]->name);
			अगर (column_width < len)
				column_width = len;
		}
		column_width += 2
			+ ((option_mask32 & OPT_Z) ? 33 : 0) /* context width */
			+ ((option_mask32 & OPT_i) ? 8 : 0) /* inode# width */
			+ ((option_mask32 & OPT_s) ? 5 : 0) /* "alloc block" width */
		;
		ncols = (अचिन्हित)G_terminal_width / column_width;
	}

	अगर (ncols > 1) {
		nrows = nfiles / ncols;
		अगर (nrows * ncols < nfiles)
			nrows++;                /* round up fractionals */
	} अन्यथा {
		nrows = nfiles;
		ncols = 1;
	}

	column = 0;
	nexttab = 0;
	क्रम (row = 0; row < nrows; row++) {
		क्रम (nc = 0; nc < ncols; nc++) {
			/* reach into the array based on the column and row */
			अगर (option_mask32 & OPT_x)
				i = (row * ncols) + nc;	/* display across row */
			अन्यथा
				i = (nc * nrows) + row;	/* display by column */
			अगर (i < nfiles) {
				अगर (column > 0) {
					nexttab -= column;
					म_लिखो("%*s", nexttab, "");
					column += nexttab;
				}
				nexttab = column + column_width;
				column += display_single(dn[i]);
			}
		}
		अक्षर_दो('\n');
		column = 0;
	}
}


/*** Dir scanning code ***/

अटल काष्ठा dnode *my_stat(स्थिर अक्षर *fullname, स्थिर अक्षर *name, पूर्णांक force_follow)
{
	काष्ठा stat statbuf;
	काष्ठा dnode *cur;

	cur = xzalloc(माप(*cur));
	cur->fullname = fullname;
	cur->name = name;

	अगर ((option_mask32 & OPT_L) || force_follow) {
#अगर ENABLE_SELINUX
		अगर (option_mask32 & OPT_Z) {
			getfilecon(fullname, &cur->sid);
		}
#पूर्ण_अगर
		अगर (stat(fullname, &statbuf)) {
			bb_simple_perror_संदेश(fullname);
			G.exit_code = निकास_त्रुटि;
			मुक्त(cur);
			वापस शून्य;
		}
		cur->dn_mode_stat = statbuf.st_mode;
	} अन्यथा {
#अगर ENABLE_SELINUX
		अगर (option_mask32 & OPT_Z) {
			lgetfilecon(fullname, &cur->sid);
		}
#पूर्ण_अगर
		अगर (lstat(fullname, &statbuf)) {
			bb_simple_perror_संदेश(fullname);
			G.exit_code = निकास_त्रुटि;
			मुक्त(cur);
			वापस शून्य;
		}
		cur->dn_mode_lstat = statbuf.st_mode;
	}

	/* cur->dstat = statbuf: */
	cur->dn_mode   = statbuf.st_mode  ;
	cur->dn_आकार   = statbuf.st_आकार  ;
#अगर ENABLE_FEATURE_LS_TIMESTAMPS || ENABLE_FEATURE_LS_SORTFILES
	cur->dn_time   = statbuf.st_mtime ;
	अगर (option_mask32 & OPT_u)
		cur->dn_time = statbuf.st_atime;
	अगर (option_mask32 & OPT_c)
		cur->dn_time = statbuf.st_ctime;
#पूर्ण_अगर
	cur->dn_ino    = statbuf.st_ino   ;
	cur->dn_blocks = statbuf.st_blocks;
	cur->dn_nlink  = statbuf.st_nlink ;
	cur->dn_uid    = statbuf.st_uid   ;
	cur->dn_gid    = statbuf.st_gid   ;
	cur->dn_rdev_maj = major(statbuf.st_rdev);
	cur->dn_rdev_min = minor(statbuf.st_rdev);

	वापस cur;
}

अटल अचिन्हित गिनती_dirs(काष्ठा dnode **dn, पूर्णांक which)
{
	अचिन्हित dirs, all;

	अगर (!dn)
		वापस 0;

	dirs = all = 0;
	क्रम (; *dn; dn++) {
		स्थिर अक्षर *name;

		all++;
		अगर (!S_ISDIR((*dn)->dn_mode))
			जारी;

		name = (*dn)->name;
		अगर (which != SPLIT_SUBDIR /* if not requested to skip . / .. */
		 /* or if it's not . or .. */
		 || name[0] != '.'
		 || (name[1] && (name[1] != '.' || name[2]))
		) {
			dirs++;
		}
	}
	वापस which != SPLIT_FILE ? dirs : all - dirs;
}

/* get memory to hold an array of pointers */
अटल काष्ठा dnode **dnalloc(अचिन्हित अंक)
{
	अगर (अंक < 1)
		वापस शून्य;

	अंक++; /* so that we have terminating NULL */
	वापस xzalloc(अंक * माप(काष्ठा dnode *));
}

#अगर ENABLE_FEATURE_LS_RECURSIVE
अटल व्योम dfree(काष्ठा dnode **dnp)
{
	अचिन्हित i;

	अगर (dnp == शून्य)
		वापस;

	क्रम (i = 0; dnp[i]; i++) {
		काष्ठा dnode *cur = dnp[i];
		अगर (cur->fname_allocated)
			मुक्त((अक्षर*)cur->fullname);
		मुक्त(cur);
	}
	मुक्त(dnp);
}
#अन्यथा
#घोषणा dfree(...) ((व्योम)0)
#पूर्ण_अगर

/* Returns NULL-terminated malloced vector of pointers (or NULL) */
अटल काष्ठा dnode **splitdnarray(काष्ठा dnode **dn, पूर्णांक which)
{
	अचिन्हित dncnt, d;
	काष्ठा dnode **dnp;

	अगर (dn == शून्य)
		वापस शून्य;

	/* count how many dirs or files there are */
	dncnt = गिनती_dirs(dn, which);

	/* allocate a file array and a dir array */
	dnp = dnalloc(dncnt);

	/* copy the entrys into the file or dir array */
	क्रम (d = 0; *dn; dn++) {
		अगर (S_ISDIR((*dn)->dn_mode)) {
			स्थिर अक्षर *name;

			अगर (which == SPLIT_FILE)
				जारी;

			name = (*dn)->name;
			अगर ((which & SPLIT_DIR) /* any dir... */
			/* ... or not . or .. */
			 || name[0] != '.'
			 || (name[1] && (name[1] != '.' || name[2]))
			) {
				dnp[d++] = *dn;
			}
		} अन्यथा
		अगर (which == SPLIT_FILE) {
			dnp[d++] = *dn;
		}
	}
	वापस dnp;
}

#अगर ENABLE_FEATURE_LS_SORTFILES
अटल पूर्णांक sortcmp(स्थिर व्योम *a, स्थिर व्योम *b)
{
	काष्ठा dnode *d1 = *(काष्ठा dnode **)a;
	काष्ठा dnode *d2 = *(काष्ठा dnode **)b;
	अचिन्हित opt = option_mask32;
	off_t dif;

	dif = 0; /* assume sort by name */
	// TODO: use pre-initialized function pointer
	// instead of branch forest
	अगर (opt & OPT_dirs_first) {
		dif = S_ISDIR(d2->dn_mode) - S_ISDIR(d1->dn_mode);
		अगर (dif != 0)
			जाओ maybe_invert_and_ret;
	}

	अगर (opt & OPT_S) { /* sort by size */
		dif = (d2->dn_आकार - d1->dn_आकार);
	} अन्यथा
	अगर (opt & OPT_t) { /* sort by time */
		dif = (d2->dn_time - d1->dn_time);
	} अन्यथा
#अगर defined(HAVE_STRVERSCMP) && HAVE_STRVERSCMP == 1
	अगर (opt & OPT_v) { /* sort by version */
		dif = strverscmp(d1->name, d2->name);
	} अन्यथा
#पूर्ण_अगर
	अगर (opt & OPT_X) { /* sort by extension */
		dif = म_भेद(strchrnul(d1->name, '.'), strchrnul(d2->name, '.'));
	}
	अगर (dif == 0) {
		/* sort by name, use as tie breaker for other sorts */
		अगर (ENABLE_LOCALE_SUPPORT)
			dif = strcoll(d1->name, d2->name);
		अन्यथा
			dif = म_भेद(d1->name, d2->name);
	} अन्यथा {
		/* Make dif fit into an int */
		अगर (माप(dif) > माप(पूर्णांक)) {
			क्रमागत { BITS_TO_SHIFT = 8 * (माप(dif) - माप(पूर्णांक)) };
			/* shift leaving only "int" worth of bits */
			/* (this requires dif != 0, and here it is nonzero) */
			dif = 1 | (पूर्णांक)((uoff_t)dif >> BITS_TO_SHIFT);
		}
	}
 maybe_invert_and_ret:
	वापस (opt & OPT_r) ? -(पूर्णांक)dif : (पूर्णांक)dif;
}

अटल व्योम dnsort(काष्ठा dnode **dn, पूर्णांक आकार)
{
	क्विक(dn, आकार, माप(*dn), sortcmp);
}

अटल व्योम sort_and_display_files(काष्ठा dnode **dn, अचिन्हित nfiles)
{
	dnsort(dn, nfiles);
	display_files(dn, nfiles);
}
#अन्यथा
# define dnsort(dn, आकार) ((व्योम)0)
# define sort_and_display_files(dn, nfiles) display_files(dn, nfiles)
#पूर्ण_अगर

/* Returns NULL-terminated malloced vector of pointers (or NULL) */
अटल काष्ठा dnode **scan_one_dir(स्थिर अक्षर *path, अचिन्हित *nfiles_p)
{
	काष्ठा dnode *dn, *cur, **dnp;
	काष्ठा dirent *entry;
	सूची *dir;
	अचिन्हित i, nfiles;

	*nfiles_p = 0;
	dir = warn_opendir(path);
	अगर (dir == शून्य) {
		G.exit_code = निकास_त्रुटि;
		वापस शून्य;	/* could not open the dir */
	}
	dn = शून्य;
	nfiles = 0;
	जबतक ((entry = सूची_पढ़ो(dir)) != शून्य) {
		अक्षर *fullname;

		/* are we going to list the file- it may be . or .. or a hidden file */
		अगर (entry->d_name[0] == '.') {
			अगर (!(option_mask32 & (OPT_a|OPT_A)))
				जारी; /* skip all dotfiles if no -a/-A */
			अगर (!(option_mask32 & OPT_a)
			 && (!entry->d_name[1] || (entry->d_name[1] == '.' && !entry->d_name[2]))
			) {
				जारी; /* if only -A, skip . and .. but show other dotfiles */
			}
		}
		fullname = concat_path_file(path, entry->d_name);
		cur = my_stat(fullname, bb_basename(fullname), 0);
		अगर (!cur) {
			मुक्त(fullname);
			जारी;
		}
		cur->fname_allocated = 1;
		cur->dn_अग्र = dn;
		dn = cur;
		nfiles++;
	}
	बंद_सूची(dir);

	अगर (dn == शून्य)
		वापस शून्य;

	/* now that we know how many files there are
	 * allocate memory for an array to hold dnode pointers
	 */
	*nfiles_p = nfiles;
	dnp = dnalloc(nfiles);
	क्रम (i = 0; /* i < nfiles - detected via !dn below */; i++) {
		dnp[i] = dn;	/* save pointer to node in array */
		dn = dn->dn_अग्र;
		अगर (!dn)
			अवरोध;
	}

	वापस dnp;
}

#अगर ENABLE_DESKTOP
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
अटल off_t calculate_blocks(काष्ठा dnode **dn)
{
	uoff_t blocks = 1;
	अगर (dn) {
		जबतक (*dn) {
			/* st_blocks is in 512 byte blocks */
			blocks += (*dn)->dn_blocks;
			dn++;
		}
	}

	/* Even though standard says use 512 byte blocks, coreutils use 1k */
	/* Actually, we round up by calculating (blocks + 1) / 2,
	 * "+ 1" was done when we initialized blocks to 1 */
	वापस blocks >> 1;
}
#पूर्ण_अगर

अटल व्योम scan_and_display_dirs_recur(काष्ठा dnode **dn, पूर्णांक first)
{
	अचिन्हित nfiles;
	काष्ठा dnode **subdnp;

	क्रम (; *dn; dn++) {
		अगर (G.show_dirname || (option_mask32 & OPT_R)) {
			अगर (!first)
				bb_putchar('\n');
			first = 0;
			म_लिखो("%s:\n", (*dn)->fullname);
		}
		subdnp = scan_one_dir((*dn)->fullname, &nfiles);
#अगर ENABLE_DESKTOP
		अगर (option_mask32 & (OPT_s|OPT_l)) {
			अगर (option_mask32 & OPT_h) {
				म_लिखो("total %-"HUMAN_READABLE_MAX_WIDTH_STR"s\n",
					/* print size, no fractions, use suffixes */
					make_human_readable_str(calculate_blocks(subdnp) * 1024,
								0, 0)
				);
			} अन्यथा {
				म_लिखो("total %"OFF_FMT"u\n", calculate_blocks(subdnp));
			}
		}
#पूर्ण_अगर
		अगर (nfiles > 0) {
			/* list all files at this level */
			sort_and_display_files(subdnp, nfiles);

			अगर (ENABLE_FEATURE_LS_RECURSIVE
			 && (option_mask32 & OPT_R)
			) {
				काष्ठा dnode **dnd;
				अचिन्हित dndirs;
				/* recursive - list the sub-dirs */
				dnd = splitdnarray(subdnp, SPLIT_SUBDIR);
				dndirs = गिनती_dirs(subdnp, SPLIT_SUBDIR);
				अगर (dndirs > 0) {
					dnsort(dnd, dndirs);
					scan_and_display_dirs_recur(dnd, 0);
					/* free the array of dnode pointers to the dirs */
					मुक्त(dnd);
				}
			}
			/* free the dnodes and the fullname mem */
			dfree(subdnp);
		}
	}
}


पूर्णांक ls_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{	/*      ^^^^^^^^^^^^^^^^^ note: if FTPD, argc can be wrong, see ftpd.c */
	काष्ठा dnode **dnd;
	काष्ठा dnode **dnf;
	काष्ठा dnode **dnp;
	काष्ठा dnode *dn;
	काष्ठा dnode *cur;
	अचिन्हित opt;
	अचिन्हित nfiles;
	अचिन्हित dnfiles;
	अचिन्हित dndirs;
	अचिन्हित i;
#अगर ENABLE_FEATURE_LS_COLOR
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
	अटल स्थिर अक्षर color_str[] ALIGN1 =
		"always\0""yes\0""force\0"
		"auto\0""tty\0""if-tty\0";
	/* need to initialize since --color has _an optional_ argument */
	स्थिर अक्षर *color_opt = color_str; /* "always" */
#पूर्ण_अगर
#अगर ENABLE_LONG_OPTS
	अटल स्थिर अक्षर ls_longopts[] ALIGN1 =
		"full-time\0" No_argument "\xff"
		"group-directories-first\0" No_argument "\xfe"
		IF_FEATURE_LS_COLOR("color\0" Optional_argument "\xfd")
	;
#पूर्ण_अगर

	INIT_G();

	init_unicode();

#अगर ENABLE_FEATURE_LS_WIDTH
	/* obtain the terminal width */
	G_terminal_width = get_terminal_width(STDIN_FILENO);
	/* go one less... */
	G_terminal_width--;
#पूर्ण_अगर

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
			IF_FEATURE_LS_WIDTH(":w+")
		, ls_longopts
		IF_FEATURE_LS_WIDTH(, /*-T*/ शून्य, /*-w*/ &G_terminal_width)
		IF_FEATURE_LS_COLOR(, &color_opt)
	);
#अगर 0 /* option bits debug */
	bb_error_संदेश("opt:0x%08x l:%x H:%x color:%x dirs:%x", opt, OPT_l, OPT_H, OPT_color, OPT_dirs_first);
	अगर (opt & OPT_c         ) bb_error_संदेश("-c");
	अगर (opt & OPT_l         ) bb_error_संदेश("-l");
	अगर (opt & OPT_H         ) bb_error_संदेश("-H");
	अगर (opt & OPT_color     ) bb_error_संदेश("--color");
	अगर (opt & OPT_dirs_first) bb_error_संदेश("--group-directories-first");
	अगर (opt & OPT_full_time ) bb_error_संदेश("--full-time");
	निकास(0);
#पूर्ण_अगर

#अगर ENABLE_SELINUX
	अगर (opt & OPT_Z) {
		अगर (!is_selinux_enabled())
			option_mask32 &= ~OPT_Z;
	}
#पूर्ण_अगर

#अगर ENABLE_FEATURE_LS_COLOR
	/* set G_show_color = 1/0 */
	अगर (ENABLE_FEATURE_LS_COLOR_IS_DEFAULT && !is_TERM_dumb()) {
		अक्षर *p = दो_पर्या("LS_COLORS");
		/* LS_COLORS is unset, or (not empty && not "none") ? */
		अगर (!p || (p[0] && म_भेद(p, "none") != 0)) {
			अगर (isatty(STDOUT_FILENO)) {
				/* check isatty() last because it's expensive (syscall) */
				G_show_color = 1;
			}
		}
	}
	अगर (opt & OPT_color) {
		अगर (color_opt[0] == 'n')
			G_show_color = 0;
		अन्यथा चयन (सूचकांक_in_substrings(color_str, color_opt)) {
		हाल 3:
		हाल 4:
		हाल 5:
			अगर (!is_TERM_dumb() && isatty(STDOUT_FILENO)) {
		हाल 0:
		हाल 1:
		हाल 2:
				G_show_color = 1;
			}
		}
	}
#पूर्ण_अगर

	/* sort out which command line options take precedence */
	अगर (ENABLE_FEATURE_LS_RECURSIVE && (opt & OPT_d))
		option_mask32 &= ~OPT_R;	/* no recurse if listing only dir */
	अगर (!(opt & OPT_l)) { /* not -l? */
		अगर (ENABLE_FEATURE_LS_TIMESTAMPS && ENABLE_FEATURE_LS_SORTFILES) {
			/* when to sort by time? -t[cu] sorts by time even with -l */
			/* (this is achieved by opt_flags[] element for -t) */
			/* without -l, bare -c or -u enable sort too */
			/* (with -l, bare -c or -u just select which time to show) */
			अगर (opt & (OPT_c|OPT_u)) {
				option_mask32 |= OPT_t;
			}
		}
	}

	/* choose a display format if one was not already specified by an option */
	अगर (!(option_mask32 & (OPT_l|OPT_1|OPT_x|OPT_C)))
		option_mask32 |= (isatty(STDOUT_FILENO) ? OPT_C : OPT_1);

	अगर (ENABLE_FTPD && applet_name[0] == 'f') {
		/* ftpd secret backdoor. dirs first are much nicer */
		option_mask32 |= OPT_dirs_first;
	}

	argv += optind;
	अगर (!argv[0])
		*--argv = (अक्षर*)".";

	अगर (argv[1])
		G.show_dirname = 1; /* 2 or more items? label directories */

	/* stuff the command line file names into a dnode array */
	dn = शून्य;
	nfiles = 0;
	करो {
		cur = my_stat(*argv, *argv,
			/* follow links on command line unless -l, -i, -s or -F: */
			!(option_mask32 & (OPT_l|OPT_i|OPT_s|OPT_F))
			/* ... or if -H: */
			|| (option_mask32 & OPT_H)
			/* ... or if -L, but my_stat always follows links if -L */
		);
		argv++;
		अगर (!cur)
			जारी;
		/*cur->fname_allocated = 0; - already is */
		cur->dn_अग्र = dn;
		dn = cur;
		nfiles++;
	} जबतक (*argv);

	/* nfiles _may_ be 0 here - try "ls doesnt_exist" */
	अगर (nfiles == 0)
		वापस G.exit_code;

	/* now that we know how many files there are
	 * allocate memory for an array to hold dnode pointers
	 */
	dnp = dnalloc(nfiles);
	क्रम (i = 0; /* i < nfiles - detected via !dn below */; i++) {
		dnp[i] = dn;	/* save pointer to node in array */
		dn = dn->dn_अग्र;
		अगर (!dn)
			अवरोध;
	}

	अगर (option_mask32 & OPT_d) {
		sort_and_display_files(dnp, nfiles);
	} अन्यथा {
		dnd = splitdnarray(dnp, SPLIT_DIR);
		dnf = splitdnarray(dnp, SPLIT_FILE);
		dndirs = गिनती_dirs(dnp, SPLIT_DIR);
		dnfiles = nfiles - dndirs;
		अगर (dnfiles > 0) {
			sort_and_display_files(dnf, dnfiles);
			अगर (ENABLE_FEATURE_CLEAN_UP)
				मुक्त(dnf);
		}
		अगर (dndirs > 0) {
			dnsort(dnd, dndirs);
			scan_and_display_dirs_recur(dnd, dnfiles == 0);
			अगर (ENABLE_FEATURE_CLEAN_UP)
				मुक्त(dnd);
		}
	}

	अगर (ENABLE_FEATURE_CLEAN_UP)
		dfree(dnp);
	वापस G.exit_code;
}
