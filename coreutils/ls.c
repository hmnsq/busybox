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

#அந்தர்பாவ "libbb.h"
#அந்தர்பாவ "common_bufsiz.h"
#அந்தர்பாவ "unicode.h"


/* This is a NOEXEC applet. Be very careful! */


#எனில் ENABLE_FTPD
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
#endif


எண்ணிக்கை {
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
#define ls_options \
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

எண்ணிக்கை {
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
கட்டமைப்பு dnode {
	மாறிலி எழுத்து *name;       /* usually basename, but think "ls -l dir/file" */
	மாறிலி எழுத்து *fullname;   /* full name (usable for stat etc) */
	கட்டமைப்பு dnode *dn_next;  /* for linked list */
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
	off_t     dn_size;
#எனில் ENABLE_FEATURE_LS_TIMESTAMPS || ENABLE_FEATURE_LS_SORTFILES
	time_t    dn_time;
#endif
	ino_t     dn_ino;
	blkcnt_t  dn_blocks;
	nlink_t   dn_nlink;
	uid_t     dn_uid;
	gid_t     dn_gid;
	முழு       dn_rdev_maj;
	முழு       dn_rdev_min;
//	dev_t     dn_dev;
//	blksize_t dn_blksize;
};

கட்டமைப்பு globals {
#எனில் ENABLE_FEATURE_LS_COLOR
	smallint show_color;
# define G_show_color (G.show_color)
#இல்லையெனில்
# define G_show_color 0
#endif
	smallint exit_code;
	smallint show_dirname;
#எனில் ENABLE_FEATURE_LS_WIDTH
	unsigned terminal_width;
# define G_terminal_width (G.terminal_width)
#இல்லையெனில்
# define G_terminal_width TERMINAL_WIDTH
#endif
#எனில் ENABLE_FEATURE_LS_TIMESTAMPS
	/* Do time() just once. Saves one syscall per file for "ls -l" */
	time_t current_time_t;
#endif
} FIX_ALIASING;
#define G (*(கட்டமைப்பு globals*)bb_common_bufsiz1)
#define INIT_G() செய் { \
	setup_common_bufsiz(); \
	/* we have to zero it out because of NOEXEC */ \
	memset(&G, 0, அளவு(G)); \
	IF_FEATURE_LS_WIDTH(G_terminal_width = TERMINAL_WIDTH;) \
	IF_FEATURE_LS_TIMESTAMPS(time(&G.current_time_t);) \
} போது (0)

#define ESC "\033"


/*** Output code ***/


/* FYI type values: 1:fifo 2:char 4:dir 6:blk 8:file 10:link 12:socket
 * (various wacky OSes: 13:Sun door 14:BSD whiteout 5:XENIX named file
 *  3/7:multiplexed char/block device)
 * and we use 0 for unknown and 15 for executables (see below) */
#define TYPEINDEX(mode) (((mode) >> 12) & 0x0f)
/*                       un  fi chr -   dir -  blk  -  file -  link - sock -   - exe */
#define APPCHAR(mode)   ("\0""|""\0""\0""/""\0""\0""\0""\0""\0""@""\0""=""\0""\0""\0" [TYPEINDEX(mode)])
/* 036 black foreground              050 black background
   037 red foreground                051 red background
   040 green foreground              052 green background
   041 brown foreground              053 brown background
   042 blue foreground               054 blue background
   043 magenta (purple) foreground   055 magenta background
   044 cyan (light blue) foreground  056 cyan background
   045 gray foreground               057 white background
*/
#define COLOR(mode) ( \
	/*un  fi  chr  -  dir  -  blk  -  file -  link -  sock -   -  exe */ \
	"\037\043\043\045\042\045\043\043\000\045\044\045\043\045\045\040" \
	[TYPEINDEX(mode)])
/* Select normal (0) [actually "reset all"] or bold (1)
 * (other attributes are 2:dim 4:underline 5:blink 7:reverse,
 *  let's use 7 for "impossible" types, just for fun)
 * Note: coreutils 6.9 uses inverted red for setuid binaries.
 */
#define ATTR(mode) ( \
	/*un fi chr - dir - blk - file- link- sock- -  exe */ \
	"\01\00\01\07\01\07\01\07\00\07\01\07\01\07\07\01" \
	[TYPEINDEX(mode)])

#எனில் ENABLE_FEATURE_LS_COLOR
/* mode of zero is interpreted as "unknown" (stat failed) */
நிலையான எழுத்து fgcolor(mode_t mode)
{
	எனில் (S_ISREG(mode) && (mode & (S_IXUSR | S_IXGRP | S_IXOTH)))
		திரும்பு COLOR(0xF000);	/* File is executable ... */
	திரும்பு COLOR(mode);
}
நிலையான எழுத்து bold(mode_t mode)
{
	எனில் (S_ISREG(mode) && (mode & (S_IXUSR | S_IXGRP | S_IXOTH)))
		திரும்பு ATTR(0xF000);	/* File is executable ... */
	திரும்பு ATTR(mode);
}
#endif

#எனில் ENABLE_FEATURE_LS_FILETYPES
நிலையான எழுத்து append_char(mode_t mode)
{
	எனில் (!(option_mask32 & (OPT_F|OPT_p)))
		திரும்பு '\0';

	எனில் (S_ISDIR(mode))
		திரும்பு '/';
	எனில் (!(option_mask32 & OPT_F))
		திரும்பு '\0';
	எனில் (S_ISREG(mode) && (mode & (S_IXUSR | S_IXGRP | S_IXOTH)))
		திரும்பு '*';
	திரும்பு APPCHAR(mode);
}
#endif

நிலையான unsigned calc_name_len(மாறிலி எழுத்து *name)
{
	unsigned len;
	uni_stat_t uni_stat;

	// TODO: quote tab as \t, etc, if -Q
	name = printable_string2(&uni_stat, name);

	எனில் (!(option_mask32 & OPT_Q)) {
		திரும்பு uni_stat.unicode_width;
	}

	len = 2 + uni_stat.unicode_width;
	போது (*name) {
		எனில் (*name == '"' || *name == '\\') {
			len++;
		}
		name++;
	}
	திரும்பு len;
}

/* Return the number of used columns.
 * Note that only columnar output uses return value.
 * -l and -1 modes don't care.
 * coreutils 7.2 also supports:
 * ls -b (--escape) = octal escapes (although it doesn't look like working)
 * ls -N (--literal) = not escape at all
 */
நிலையான unsigned print_name(மாறிலி எழுத்து *name)
{
	unsigned len;
	uni_stat_t uni_stat;

	// TODO: quote tab as \t, etc, if -Q
	name = printable_string2(&uni_stat, name);

	எனில் (!(option_mask32 & OPT_Q)) {
		fputs_stdout(name);
		திரும்பு uni_stat.unicode_width;
	}

	len = 2 + uni_stat.unicode_width;
	putchar('"');
	போது (*name) {
		எனில் (*name == '"' || *name == '\\') {
			putchar('\\');
			len++;
		}
		putchar(*name);
		name++;
	}
	putchar('"');
	திரும்பு len;
}

/* Return the number of used columns.
 * Note that only columnar output uses return value,
 * -l and -1 modes don't care.
 */
நிலையான NOINLINE unsigned display_single(மாறிலி கட்டமைப்பு dnode *dn)
{
	unsigned column = 0;
	எழுத்து *lpath;
	முழு opt;
#எனில் ENABLE_FEATURE_LS_FILETYPES || ENABLE_FEATURE_LS_COLOR
	கட்டமைப்பு stat statbuf;
#endif
#எனில் ENABLE_FEATURE_LS_FILETYPES
	எழுத்து append = append_char(dn->dn_mode);
#endif

	opt = option_mask32;

	/* Do readlink early, so that if it fails, error message
	 * does not appear *inside* the "ls -l" line */
	lpath = வெற்று;
	எனில் (opt & OPT_l)
		எனில் (S_ISLNK(dn->dn_mode))
			lpath = xmalloc_readlink_or_warn(dn->fullname);

	எனில் (opt & OPT_i) /* show inode# */
		column += printf("%7llu ", (long long) dn->dn_ino);
	எனில் (opt & OPT_s) { /* show allocated blocks */
		எனில் (opt & OPT_h) {
			column += printf("%"HUMAN_READABLE_MAX_WIDTH_STR"s ",
				/* print size, show one fractional, use suffixes */
				make_human_readable_str((off_t)dn->dn_blocks << 9, 1, 0)
			);
		} இல்லையெனில் {
			column += printf("%6"OFF_FMT"u ", (off_t)(dn->dn_blocks >> 1));
		}
	}
	எனில் (opt & OPT_l) {
		/* long listing: show mode */
		எழுத்து modestr[12];
		column += printf("%-10s ", bb_mode_string(modestr, dn->dn_mode));
		/* long listing: show number of links */
		column += printf("%4lu ", (long) dn->dn_nlink);
		/* long listing: show user/group */
		எனில் (opt & OPT_n) {
			எனில் (opt & OPT_g)
				column += printf("%-8u ", (முழு) dn->dn_gid);
			இல்லையெனில்
				column += printf("%-8u %-8u ",
						(முழு) dn->dn_uid,
						(முழு) dn->dn_gid);
		}
#எனில் ENABLE_FEATURE_LS_USERNAME
		இல்லையெனில் {
			எனில் (opt & OPT_g) {
				column += printf("%-8s ",
					get_cached_groupname(dn->dn_gid));
			} இல்லையெனில் {
				column += printf("%-8s %-8s ",
					get_cached_username(dn->dn_uid),
					get_cached_groupname(dn->dn_gid));
			}
		}
#endif
#எனில் ENABLE_SELINUX
	}
	எனில் (opt & OPT_Z) {
		column += printf("%-32s ", dn->sid ? dn->sid : "?");
		freecon(dn->sid);
	}
	எனில் (opt & OPT_l) {
#endif
		/* long listing: show size */
		எனில் (S_ISBLK(dn->dn_mode) || S_ISCHR(dn->dn_mode)) {
			column += printf("%4u, %3u ",
					dn->dn_rdev_maj,
					dn->dn_rdev_min);
		} இல்லையெனில் {
			எனில் (opt & OPT_h) {
				column += printf("%"HUMAN_READABLE_MAX_WIDTH_STR"s ",
					/* print size, show one fractional, use suffixes */
					make_human_readable_str(dn->dn_size, 1, 0)
				);
			} இல்லையெனில் {
				column += printf("%9"OFF_FMT"u ", dn->dn_size);
			}
		}
#எனில் ENABLE_FEATURE_LS_TIMESTAMPS
		/* long listing: show {m,c,a}time */
		எனில் (opt & OPT_full_time) { /* --full-time */
			/* coreutils 8.4 ls --full-time prints:
			 * 2009-07-13 17:49:27.000000000 +0200
			 * we don't show fractional seconds.
			 */
			எழுத்து buf[அளவு("YYYY-mm-dd HH:MM:SS TIMEZONE")];
			strftime(buf, அளவு(buf), "%Y-%m-%d %H:%M:%S %z",
					localtime(&dn->dn_time));
			column += printf("%s ", buf);
		} இல்லையெனில் { /* ordinary time format */
			/* G.current_time_t is ~== time(NULL) */
			எழுத்து *filetime = ctime(&dn->dn_time);
			/* filetime's format: "Wed Jun 30 21:49:08 1993\n" */
			time_t age = G.current_time_t - dn->dn_time;
			எனில் (age < 3600L * 24 * 365 / 2 && age > -15 * 60) {
				/* less than 6 months old */
				/* "mmm dd hh:mm " */
				printf("%.12s ", filetime + 4);
			} இல்லையெனில் {
				/* "mmm dd  yyyy " */
				/* "mmm dd yyyyy " after year 9999 :) */
				strchr(filetime + 20, '\n')[0] = ' ';
				printf("%.7s%6s", filetime + 4, filetime + 20);
			}
			column += 13;
		}
#endif
	}

#எனில் ENABLE_FEATURE_LS_COLOR
	எனில் (G_show_color) {
		mode_t mode = dn->dn_mode_lstat;
		எனில் (!mode)
			எனில் (lstat(dn->fullname, &statbuf) == 0)
				mode = statbuf.st_mode;
		printf(ESC"[%u;%um", bold(mode), fgcolor(mode));
	}
#endif
	column += print_name(dn->name);
	எனில் (G_show_color) {
		printf(ESC"[m");
	}

	எனில் (lpath) {
		printf(" -> ");
#எனில் ENABLE_FEATURE_LS_FILETYPES || ENABLE_FEATURE_LS_COLOR
		எனில் ((opt & (OPT_F|OPT_p))
		 || G_show_color
		) {
			mode_t mode = dn->dn_mode_stat;
			எனில் (!mode)
				எனில் (stat(dn->fullname, &statbuf) == 0)
					mode = statbuf.st_mode;
# எனில் ENABLE_FEATURE_LS_FILETYPES
			append = append_char(mode);
# endif
# எனில் ENABLE_FEATURE_LS_COLOR
			எனில் (G_show_color) {
				printf(ESC"[%u;%um", bold(mode), fgcolor(mode));
			}
# endif
		}
#endif
		column += print_name(lpath) + 4;
		விடுவி(lpath);
		எனில் (G_show_color) {
			printf(ESC"[m");
		}
	}
#எனில் ENABLE_FEATURE_LS_FILETYPES
	எனில் (opt & (OPT_F|OPT_p)) {
		எனில் (append) {
			putchar(append);
			column++;
		}
	}
#endif

	திரும்பு column;
}

நிலையான வெறுமை display_files(கட்டமைப்பு dnode **dn, unsigned nfiles)
{
	unsigned i, ncols, nrows, row, nc;
	unsigned column;
	unsigned nexttab;
	unsigned column_width = 0; /* used only by coulmnal output */

	எனில் (option_mask32 & (OPT_l|OPT_1)) {
		ncols = 1;
	} இல்லையெனில் {
		/* find the longest file name, use that as the column width */
		க்கு (i = 0; dn[i]; i++) {
			முழு len = calc_name_len(dn[i]->name);
			எனில் (column_width < len)
				column_width = len;
		}
		column_width += 2
			+ ((option_mask32 & OPT_Z) ? 33 : 0) /* context width */
			+ ((option_mask32 & OPT_i) ? 8 : 0) /* inode# width */
			+ ((option_mask32 & OPT_s) ? 5 : 0) /* "alloc block" width */
		;
		ncols = (unsigned)G_terminal_width / column_width;
	}

	எனில் (ncols > 1) {
		nrows = nfiles / ncols;
		எனில் (nrows * ncols < nfiles)
			nrows++;                /* round up fractionals */
	} இல்லையெனில் {
		nrows = nfiles;
		ncols = 1;
	}

	column = 0;
	nexttab = 0;
	க்கு (row = 0; row < nrows; row++) {
		க்கு (nc = 0; nc < ncols; nc++) {
			/* reach into the array based on the column and row */
			எனில் (option_mask32 & OPT_x)
				i = (row * ncols) + nc;	/* display across row */
			இல்லையெனில்
				i = (nc * nrows) + row;	/* display by column */
			எனில் (i < nfiles) {
				எனில் (column > 0) {
					nexttab -= column;
					printf("%*s", nexttab, "");
					column += nexttab;
				}
				nexttab = column + column_width;
				column += display_single(dn[i]);
			}
		}
		putchar('\n');
		column = 0;
	}
}


/*** Dir scanning code ***/

நிலையான கட்டமைப்பு dnode *my_stat(மாறிலி எழுத்து *fullname, மாறிலி எழுத்து *name, முழு force_follow)
{
	கட்டமைப்பு stat statbuf;
	கட்டமைப்பு dnode *cur;

	cur = xzalloc(அளவு(*cur));
	cur->fullname = fullname;
	cur->name = name;

	எனில் ((option_mask32 & OPT_L) || force_follow) {
#எனில் ENABLE_SELINUX
		எனில் (option_mask32 & OPT_Z) {
			getfilecon(fullname, &cur->sid);
		}
#endif
		எனில் (stat(fullname, &statbuf)) {
			bb_simple_perror_செய்தி(fullname);
			G.exit_code = EXIT_FAILURE;
			விடுவி(cur);
			திரும்பு வெற்று;
		}
		cur->dn_mode_stat = statbuf.st_mode;
	} இல்லையெனில் {
#எனில் ENABLE_SELINUX
		எனில் (option_mask32 & OPT_Z) {
			lgetfilecon(fullname, &cur->sid);
		}
#endif
		எனில் (lstat(fullname, &statbuf)) {
			bb_simple_perror_செய்தி(fullname);
			G.exit_code = EXIT_FAILURE;
			விடுவி(cur);
			திரும்பு வெற்று;
		}
		cur->dn_mode_lstat = statbuf.st_mode;
	}

	/* cur->dstat = statbuf: */
	cur->dn_mode   = statbuf.st_mode  ;
	cur->dn_size   = statbuf.st_size  ;
#எனில் ENABLE_FEATURE_LS_TIMESTAMPS || ENABLE_FEATURE_LS_SORTFILES
	cur->dn_time   = statbuf.st_mtime ;
	எனில் (option_mask32 & OPT_u)
		cur->dn_time = statbuf.st_atime;
	எனில் (option_mask32 & OPT_c)
		cur->dn_time = statbuf.st_ctime;
#endif
	cur->dn_ino    = statbuf.st_ino   ;
	cur->dn_blocks = statbuf.st_blocks;
	cur->dn_nlink  = statbuf.st_nlink ;
	cur->dn_uid    = statbuf.st_uid   ;
	cur->dn_gid    = statbuf.st_gid   ;
	cur->dn_rdev_maj = major(statbuf.st_rdev);
	cur->dn_rdev_min = minor(statbuf.st_rdev);

	திரும்பு cur;
}

நிலையான unsigned மொத்தம்_dirs(கட்டமைப்பு dnode **dn, முழு which)
{
	unsigned dirs, all;

	எனில் (!dn)
		திரும்பு 0;

	dirs = all = 0;
	க்கு (; *dn; dn++) {
		மாறிலி எழுத்து *name;

		all++;
		எனில் (!S_ISDIR((*dn)->dn_mode))
			தொடர்;

		name = (*dn)->name;
		எனில் (which != SPLIT_SUBDIR /* if not requested to skip . / .. */
		 /* or if it's not . or .. */
		 || name[0] != '.'
		 || (name[1] && (name[1] != '.' || name[2]))
		) {
			dirs++;
		}
	}
	திரும்பு which != SPLIT_FILE ? dirs : all - dirs;
}

/* get memory to hold an array of pointers */
நிலையான கட்டமைப்பு dnode **dnalloc(unsigned இலக்கம்)
{
	எனில் (இலக்கம் < 1)
		திரும்பு வெற்று;

	இலக்கம்++; /* so that we have terminating NULL */
	திரும்பு xzalloc(இலக்கம் * அளவு(கட்டமைப்பு dnode *));
}

#எனில் ENABLE_FEATURE_LS_RECURSIVE
நிலையான வெறுமை dfree(கட்டமைப்பு dnode **dnp)
{
	unsigned i;

	எனில் (dnp == வெற்று)
		திரும்பு;

	க்கு (i = 0; dnp[i]; i++) {
		கட்டமைப்பு dnode *cur = dnp[i];
		எனில் (cur->fname_allocated)
			விடுவி((எழுத்து*)cur->fullname);
		விடுவி(cur);
	}
	விடுவி(dnp);
}
#இல்லையெனில்
#define dfree(...) ((வெறுமை)0)
#endif

/* Returns NULL-terminated malloced vector of pointers (or NULL) */
நிலையான கட்டமைப்பு dnode **splitdnarray(கட்டமைப்பு dnode **dn, முழு which)
{
	unsigned dncnt, d;
	கட்டமைப்பு dnode **dnp;

	எனில் (dn == வெற்று)
		திரும்பு வெற்று;

	/* count how many dirs or files there are */
	dncnt = மொத்தம்_dirs(dn, which);

	/* allocate a file array and a dir array */
	dnp = dnalloc(dncnt);

	/* copy the entrys into the file or dir array */
	க்கு (d = 0; *dn; dn++) {
		எனில் (S_ISDIR((*dn)->dn_mode)) {
			மாறிலி எழுத்து *name;

			எனில் (which == SPLIT_FILE)
				தொடர்;

			name = (*dn)->name;
			எனில் ((which & SPLIT_DIR) /* any dir... */
			/* ... or not . or .. */
			 || name[0] != '.'
			 || (name[1] && (name[1] != '.' || name[2]))
			) {
				dnp[d++] = *dn;
			}
		} இல்லையெனில்
		எனில் (which == SPLIT_FILE) {
			dnp[d++] = *dn;
		}
	}
	திரும்பு dnp;
}

#எனில் ENABLE_FEATURE_LS_SORTFILES
நிலையான முழு sortcmp(மாறிலி வெறுமை *a, மாறிலி வெறுமை *b)
{
	கட்டமைப்பு dnode *d1 = *(கட்டமைப்பு dnode **)a;
	கட்டமைப்பு dnode *d2 = *(கட்டமைப்பு dnode **)b;
	unsigned opt = option_mask32;
	off_t dif;

	dif = 0; /* assume sort by name */
	// TODO: use pre-initialized function pointer
	// instead of branch forest
	எனில் (opt & OPT_dirs_first) {
		dif = S_ISDIR(d2->dn_mode) - S_ISDIR(d1->dn_mode);
		எனில் (dif != 0)
			செல் maybe_invert_and_ret;
	}

	எனில் (opt & OPT_S) { /* sort by size */
		dif = (d2->dn_size - d1->dn_size);
	} இல்லையெனில்
	எனில் (opt & OPT_t) { /* sort by time */
		dif = (d2->dn_time - d1->dn_time);
	} இல்லையெனில்
#எனில் defined(HAVE_STRVERSCMP) && HAVE_STRVERSCMP == 1
	எனில் (opt & OPT_v) { /* sort by version */
		dif = strverscmp(d1->name, d2->name);
	} இல்லையெனில்
#endif
	எனில் (opt & OPT_X) { /* sort by extension */
		dif = strcmp(strchrnul(d1->name, '.'), strchrnul(d2->name, '.'));
	}
	எனில் (dif == 0) {
		/* sort by name, use as tie breaker for other sorts */
		எனில் (ENABLE_LOCALE_SUPPORT)
			dif = strcoll(d1->name, d2->name);
		இல்லையெனில்
			dif = strcmp(d1->name, d2->name);
	} இல்லையெனில் {
		/* Make dif fit into an int */
		எனில் (அளவு(dif) > அளவு(முழு)) {
			எண்ணிக்கை { BITS_TO_SHIFT = 8 * (அளவு(dif) - அளவு(முழு)) };
			/* shift leaving only "int" worth of bits */
			/* (this requires dif != 0, and here it is nonzero) */
			dif = 1 | (முழு)((uoff_t)dif >> BITS_TO_SHIFT);
		}
	}
 maybe_invert_and_ret:
	திரும்பு (opt & OPT_r) ? -(முழு)dif : (முழு)dif;
}

நிலையான வெறுமை dnsort(கட்டமைப்பு dnode **dn, முழு size)
{
	qsort(dn, size, அளவு(*dn), sortcmp);
}

நிலையான வெறுமை sort_and_display_files(கட்டமைப்பு dnode **dn, unsigned nfiles)
{
	dnsort(dn, nfiles);
	display_files(dn, nfiles);
}
#இல்லையெனில்
# define dnsort(dn, size) ((வெறுமை)0)
# define sort_and_display_files(dn, nfiles) display_files(dn, nfiles)
#endif

/* Returns NULL-terminated malloced vector of pointers (or NULL) */
நிலையான கட்டமைப்பு dnode **scan_one_dir(மாறிலி எழுத்து *path, unsigned *nfiles_p)
{
	கட்டமைப்பு dnode *dn, *cur, **dnp;
	கட்டமைப்பு dirent *entry;
	DIR *dir;
	unsigned i, nfiles;

	*nfiles_p = 0;
	dir = warn_opendir(path);
	எனில் (dir == வெற்று) {
		G.exit_code = EXIT_FAILURE;
		திரும்பு வெற்று;	/* could not open the dir */
	}
	dn = வெற்று;
	nfiles = 0;
	போது ((entry = readdir(dir)) != வெற்று) {
		எழுத்து *fullname;

		/* are we going to list the file- it may be . or .. or a hidden file */
		எனில் (entry->d_name[0] == '.') {
			எனில் (!(option_mask32 & (OPT_a|OPT_A)))
				தொடர்; /* skip all dotfiles if no -a/-A */
			எனில் (!(option_mask32 & OPT_a)
			 && (!entry->d_name[1] || (entry->d_name[1] == '.' && !entry->d_name[2]))
			) {
				தொடர்; /* if only -A, skip . and .. but show other dotfiles */
			}
		}
		fullname = concat_path_file(path, entry->d_name);
		cur = my_stat(fullname, bb_basename(fullname), 0);
		எனில் (!cur) {
			விடுவி(fullname);
			தொடர்;
		}
		cur->fname_allocated = 1;
		cur->dn_next = dn;
		dn = cur;
		nfiles++;
	}
	closedir(dir);

	எனில் (dn == வெற்று)
		திரும்பு வெற்று;

	/* now that we know how many files there are
	 * allocate memory for an array to hold dnode pointers
	 */
	*nfiles_p = nfiles;
	dnp = dnalloc(nfiles);
	க்கு (i = 0; /* i < nfiles - detected via !dn below */; i++) {
		dnp[i] = dn;	/* save pointer to node in array */
		dn = dn->dn_next;
		எனில் (!dn)
			நிறுத்து;
	}

	திரும்பு dnp;
}

#எனில் ENABLE_DESKTOP
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
நிலையான off_t calculate_blocks(கட்டமைப்பு dnode **dn)
{
	uoff_t blocks = 1;
	எனில் (dn) {
		போது (*dn) {
			/* st_blocks is in 512 byte blocks */
			blocks += (*dn)->dn_blocks;
			dn++;
		}
	}

	/* Even though standard says use 512 byte blocks, coreutils use 1k */
	/* Actually, we round up by calculating (blocks + 1) / 2,
	 * "+ 1" was done when we initialized blocks to 1 */
	திரும்பு blocks >> 1;
}
#endif

நிலையான வெறுமை scan_and_display_dirs_recur(கட்டமைப்பு dnode **dn, முழு first)
{
	unsigned nfiles;
	கட்டமைப்பு dnode **subdnp;

	க்கு (; *dn; dn++) {
		எனில் (G.show_dirname || (option_mask32 & OPT_R)) {
			எனில் (!first)
				bb_putchar('\n');
			first = 0;
			printf("%s:\n", (*dn)->fullname);
		}
		subdnp = scan_one_dir((*dn)->fullname, &nfiles);
#எனில் ENABLE_DESKTOP
		எனில் (option_mask32 & (OPT_s|OPT_l)) {
			எனில் (option_mask32 & OPT_h) {
				printf("total %-"HUMAN_READABLE_MAX_WIDTH_STR"s\n",
					/* print size, no fractions, use suffixes */
					make_human_readable_str(calculate_blocks(subdnp) * 1024,
								0, 0)
				);
			} இல்லையெனில் {
				printf("total %"OFF_FMT"u\n", calculate_blocks(subdnp));
			}
		}
#endif
		எனில் (nfiles > 0) {
			/* list all files at this level */
			sort_and_display_files(subdnp, nfiles);

			எனில் (ENABLE_FEATURE_LS_RECURSIVE
			 && (option_mask32 & OPT_R)
			) {
				கட்டமைப்பு dnode **dnd;
				unsigned dndirs;
				/* recursive - list the sub-dirs */
				dnd = splitdnarray(subdnp, SPLIT_SUBDIR);
				dndirs = மொத்தம்_dirs(subdnp, SPLIT_SUBDIR);
				எனில் (dndirs > 0) {
					dnsort(dnd, dndirs);
					scan_and_display_dirs_recur(dnd, 0);
					/* free the array of dnode pointers to the dirs */
					விடுவி(dnd);
				}
			}
			/* free the dnodes and the fullname mem */
			dfree(subdnp);
		}
	}
}


முழு ls_main(முழு argc UNUSED_PARAM, எழுத்து **argv)
{	/*      ^^^^^^^^^^^^^^^^^ note: if FTPD, argc can be wrong, see ftpd.c */
	கட்டமைப்பு dnode **dnd;
	கட்டமைப்பு dnode **dnf;
	கட்டமைப்பு dnode **dnp;
	கட்டமைப்பு dnode *dn;
	கட்டமைப்பு dnode *cur;
	unsigned opt;
	unsigned nfiles;
	unsigned dnfiles;
	unsigned dndirs;
	unsigned i;
#எனில் ENABLE_FEATURE_LS_COLOR
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
	நிலையான மாறிலி எழுத்து color_str[] ALIGN1 =
		"always\0""yes\0""force\0"
		"auto\0""tty\0""if-tty\0";
	/* need to initialize since --color has _an optional_ argument */
	மாறிலி எழுத்து *color_opt = color_str; /* "always" */
#endif
#எனில் ENABLE_LONG_OPTS
	நிலையான மாறிலி எழுத்து ls_longopts[] ALIGN1 =
		"full-time\0" No_argument "\xff"
		"group-directories-first\0" No_argument "\xfe"
		IF_FEATURE_LS_COLOR("color\0" Optional_argument "\xfd")
	;
#endif

	INIT_G();

	init_unicode();

#எனில் ENABLE_FEATURE_LS_WIDTH
	/* obtain the terminal width */
	G_terminal_width = get_terminal_width(STDIN_FILENO);
	/* go one less... */
	G_terminal_width--;
#endif

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
		IF_FEATURE_LS_WIDTH(, /*-T*/ வெற்று, /*-w*/ &G_terminal_width)
		IF_FEATURE_LS_COLOR(, &color_opt)
	);
#எனில் 0 /* option bits debug */
	bb_error_செய்தி("opt:0x%08x l:%x H:%x color:%x dirs:%x", opt, OPT_l, OPT_H, OPT_color, OPT_dirs_first);
	எனில் (opt & OPT_c         ) bb_error_செய்தி("-c");
	எனில் (opt & OPT_l         ) bb_error_செய்தி("-l");
	எனில் (opt & OPT_H         ) bb_error_செய்தி("-H");
	எனில் (opt & OPT_color     ) bb_error_செய்தி("--color");
	எனில் (opt & OPT_dirs_first) bb_error_செய்தி("--group-directories-first");
	எனில் (opt & OPT_full_time ) bb_error_செய்தி("--full-time");
	exit(0);
#endif

#எனில் ENABLE_SELINUX
	எனில் (opt & OPT_Z) {
		எனில் (!is_selinux_enabled())
			option_mask32 &= ~OPT_Z;
	}
#endif

#எனில் ENABLE_FEATURE_LS_COLOR
	/* set G_show_color = 1/0 */
	எனில் (ENABLE_FEATURE_LS_COLOR_IS_DEFAULT && !is_TERM_dumb()) {
		எழுத்து *p = getenv("LS_COLORS");
		/* LS_COLORS is unset, or (not empty && not "none") ? */
		எனில் (!p || (p[0] && strcmp(p, "none") != 0)) {
			எனில் (isatty(STDOUT_FILENO)) {
				/* check isatty() last because it's expensive (syscall) */
				G_show_color = 1;
			}
		}
	}
	எனில் (opt & OPT_color) {
		எனில் (color_opt[0] == 'n')
			G_show_color = 0;
		இல்லையெனில் தேர்வு (index_in_substrings(color_str, color_opt)) {
		நிலை 3:
		நிலை 4:
		நிலை 5:
			எனில் (!is_TERM_dumb() && isatty(STDOUT_FILENO)) {
		நிலை 0:
		நிலை 1:
		நிலை 2:
				G_show_color = 1;
			}
		}
	}
#endif

	/* sort out which command line options take precedence */
	எனில் (ENABLE_FEATURE_LS_RECURSIVE && (opt & OPT_d))
		option_mask32 &= ~OPT_R;	/* no recurse if listing only dir */
	எனில் (!(opt & OPT_l)) { /* not -l? */
		எனில் (ENABLE_FEATURE_LS_TIMESTAMPS && ENABLE_FEATURE_LS_SORTFILES) {
			/* when to sort by time? -t[cu] sorts by time even with -l */
			/* (this is achieved by opt_flags[] element for -t) */
			/* without -l, bare -c or -u enable sort too */
			/* (with -l, bare -c or -u just select which time to show) */
			எனில் (opt & (OPT_c|OPT_u)) {
				option_mask32 |= OPT_t;
			}
		}
	}

	/* choose a display format if one was not already specified by an option */
	எனில் (!(option_mask32 & (OPT_l|OPT_1|OPT_x|OPT_C)))
		option_mask32 |= (isatty(STDOUT_FILENO) ? OPT_C : OPT_1);

	எனில் (ENABLE_FTPD && applet_name[0] == 'f') {
		/* ftpd secret backdoor. dirs first are much nicer */
		option_mask32 |= OPT_dirs_first;
	}

	argv += optind;
	எனில் (!argv[0])
		*--argv = (எழுத்து*)".";

	எனில் (argv[1])
		G.show_dirname = 1; /* 2 or more items? label directories */

	/* stuff the command line file names into a dnode array */
	dn = வெற்று;
	nfiles = 0;
	செய் {
		cur = my_stat(*argv, *argv,
			/* follow links on command line unless -l, -i, -s or -F: */
			!(option_mask32 & (OPT_l|OPT_i|OPT_s|OPT_F))
			/* ... or if -H: */
			|| (option_mask32 & OPT_H)
			/* ... or if -L, but my_stat always follows links if -L */
		);
		argv++;
		எனில் (!cur)
			தொடர்;
		/*cur->fname_allocated = 0; - already is */
		cur->dn_next = dn;
		dn = cur;
		nfiles++;
	} போது (*argv);

	/* nfiles _may_ be 0 here - try "ls doesnt_exist" */
	எனில் (nfiles == 0)
		திரும்பு G.exit_code;

	/* now that we know how many files there are
	 * allocate memory for an array to hold dnode pointers
	 */
	dnp = dnalloc(nfiles);
	க்கு (i = 0; /* i < nfiles - detected via !dn below */; i++) {
		dnp[i] = dn;	/* save pointer to node in array */
		dn = dn->dn_next;
		எனில் (!dn)
			நிறுத்து;
	}

	எனில் (option_mask32 & OPT_d) {
		sort_and_display_files(dnp, nfiles);
	} இல்லையெனில் {
		dnd = splitdnarray(dnp, SPLIT_DIR);
		dnf = splitdnarray(dnp, SPLIT_FILE);
		dndirs = மொத்தம்_dirs(dnp, SPLIT_DIR);
		dnfiles = nfiles - dndirs;
		எனில் (dnfiles > 0) {
			sort_and_display_files(dnf, dnfiles);
			எனில் (ENABLE_FEATURE_CLEAN_UP)
				விடுவி(dnf);
		}
		எனில் (dndirs > 0) {
			dnsort(dnd, dndirs);
			scan_and_display_dirs_recur(dnd, dnfiles == 0);
			எனில் (ENABLE_FEATURE_CLEAN_UP)
				விடுவி(dnd);
		}
	}

	எனில் (ENABLE_FEATURE_CLEAN_UP)
		dfree(dnp);
	திரும்பு G.exit_code;
}
