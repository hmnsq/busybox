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

#अन्तर्भाव "libbb.h"
#अन्तर्भाव "common_bufsiz.h"
#अन्तर्भाव "unicode.h"


/* This is a NOEXEC applet. Be very careful! */


#सचे ENABLE_FTPD
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


गणना {
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

गणना {
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
संरचना dnode {
	ध्रुव वर्ण *name;       /* usually basename, but think "ls -l dir/file" */
	ध्रुव वर्ण *fullname;   /* full name (usable for stat etc) */
	संरचना dnode *dn_next;  /* for linked list */
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
#सचे ENABLE_FEATURE_LS_TIMESTAMPS || ENABLE_FEATURE_LS_SORTFILES
	time_t    dn_time;
#endif
	ino_t     dn_ino;
	blkcnt_t  dn_blocks;
	nlink_t   dn_nlink;
	uid_t     dn_uid;
	gid_t     dn_gid;
	पूर्णाङ्क       dn_rdev_maj;
	पूर्णाङ्क       dn_rdev_min;
//	dev_t     dn_dev;
//	blksize_t dn_blksize;
};

संरचना globals {
#सचे ENABLE_FEATURE_LS_COLOR
	smallint show_color;
# define G_show_color (G.show_color)
#अञ्ञथा
# define G_show_color 0
#endif
	smallint exit_code;
	smallint show_dirname;
#सचे ENABLE_FEATURE_LS_WIDTH
	unsigned terminal_width;
# define G_terminal_width (G.terminal_width)
#अञ्ञथा
# define G_terminal_width TERMINAL_WIDTH
#endif
#सचे ENABLE_FEATURE_LS_TIMESTAMPS
	/* Do time() just once. Saves one syscall per file for "ls -l" */
	time_t current_time_t;
#endif
} FIX_ALIASING;
#define G (*(संरचना globals*)bb_common_bufsiz1)
#define INIT_G() करोहि { \
	setup_common_bufsiz(); \
	/* we have to zero it out because of NOEXEC */ \
	memset(&G, 0, परिमाण(G)); \
	IF_FEATURE_LS_WIDTH(G_terminal_width = TERMINAL_WIDTH;) \
	IF_FEATURE_LS_TIMESTAMPS(time(&G.current_time_t);) \
} याव (0)

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

#सचे ENABLE_FEATURE_LS_COLOR
/* mode of zero is interpreted as "unknown" (stat failed) */
स्थिर वर्ण fgcolor(mode_t mode)
{
	सचे (S_ISREG(mode) && (mode & (S_IXUSR | S_IXGRP | S_IXOTH)))
		पटिनिवत्त COLOR(0xF000);	/* File is executable ... */
	पटिनिवत्त COLOR(mode);
}
स्थिर वर्ण bold(mode_t mode)
{
	सचे (S_ISREG(mode) && (mode & (S_IXUSR | S_IXGRP | S_IXOTH)))
		पटिनिवत्त ATTR(0xF000);	/* File is executable ... */
	पटिनिवत्त ATTR(mode);
}
#endif

#सचे ENABLE_FEATURE_LS_FILETYPES
स्थिर वर्ण append_char(mode_t mode)
{
	सचे (!(option_mask32 & (OPT_F|OPT_p)))
		पटिनिवत्त '\0';

	सचे (S_ISDIR(mode))
		पटिनिवत्त '/';
	सचे (!(option_mask32 & OPT_F))
		पटिनिवत्त '\0';
	सचे (S_ISREG(mode) && (mode & (S_IXUSR | S_IXGRP | S_IXOTH)))
		पटिनिवत्त '*';
	पटिनिवत्त APPCHAR(mode);
}
#endif

स्थिर unsigned calc_name_len(ध्रुव वर्ण *name)
{
	unsigned len;
	uni_stat_t uni_stat;

	// TODO: quote tab as \t, etc, if -Q
	name = printable_string2(&uni_stat, name);

	सचे (!(option_mask32 & OPT_Q)) {
		पटिनिवत्त uni_stat.unicode_width;
	}

	len = 2 + uni_stat.unicode_width;
	याव (*name) {
		सचे (*name == '"' || *name == '\\') {
			len++;
		}
		name++;
	}
	पटिनिवत्त len;
}

/* Return the number of used columns.
 * Note that only columnar output uses return value.
 * -l and -1 modes don't care.
 * coreutils 7.2 also supports:
 * ls -b (--escape) = octal escapes (although it doesn't look like working)
 * ls -N (--literal) = not escape at all
 */
स्थिर unsigned print_name(ध्रुव वर्ण *name)
{
	unsigned len;
	uni_stat_t uni_stat;

	// TODO: quote tab as \t, etc, if -Q
	name = printable_string2(&uni_stat, name);

	सचे (!(option_mask32 & OPT_Q)) {
		fputs_stdout(name);
		पटिनिवत्त uni_stat.unicode_width;
	}

	len = 2 + uni_stat.unicode_width;
	putchar('"');
	याव (*name) {
		सचे (*name == '"' || *name == '\\') {
			putchar('\\');
			len++;
		}
		putchar(*name);
		name++;
	}
	putchar('"');
	पटिनिवत्त len;
}

/* Return the number of used columns.
 * Note that only columnar output uses return value,
 * -l and -1 modes don't care.
 */
स्थिर NOINLINE unsigned display_single(ध्रुव संरचना dnode *dn)
{
	unsigned column = 0;
	वर्ण *lpath;
	पूर्णाङ्क opt;
#सचे ENABLE_FEATURE_LS_FILETYPES || ENABLE_FEATURE_LS_COLOR
	संरचना stat statbuf;
#endif
#सचे ENABLE_FEATURE_LS_FILETYPES
	वर्ण append = append_char(dn->dn_mode);
#endif

	opt = option_mask32;

	/* Do readlink early, so that if it fails, error message
	 * does not appear *inside* the "ls -l" line */
	lpath = रिक्त;
	सचे (opt & OPT_l)
		सचे (S_ISLNK(dn->dn_mode))
			lpath = xmalloc_readlink_or_warn(dn->fullname);

	सचे (opt & OPT_i) /* show inode# */
		column += printf("%7llu ", (long long) dn->dn_ino);
	सचे (opt & OPT_s) { /* show allocated blocks */
		सचे (opt & OPT_h) {
			column += printf("%"HUMAN_READABLE_MAX_WIDTH_STR"s ",
				/* print size, show one fractional, use suffixes */
				make_human_readable_str((off_t)dn->dn_blocks << 9, 1, 0)
			);
		} अञ्ञथा {
			column += printf("%6"OFF_FMT"u ", (off_t)(dn->dn_blocks >> 1));
		}
	}
	सचे (opt & OPT_l) {
		/* long listing: show mode */
		वर्ण modestr[12];
		column += printf("%-10s ", bb_mode_string(modestr, dn->dn_mode));
		/* long listing: show number of links */
		column += printf("%4lu ", (long) dn->dn_nlink);
		/* long listing: show user/group */
		सचे (opt & OPT_n) {
			सचे (opt & OPT_g)
				column += printf("%-8u ", (पूर्णाङ्क) dn->dn_gid);
			अञ्ञथा
				column += printf("%-8u %-8u ",
						(पूर्णाङ्क) dn->dn_uid,
						(पूर्णाङ्क) dn->dn_gid);
		}
#सचे ENABLE_FEATURE_LS_USERNAME
		अञ्ञथा {
			सचे (opt & OPT_g) {
				column += printf("%-8s ",
					get_cached_groupname(dn->dn_gid));
			} अञ्ञथा {
				column += printf("%-8s %-8s ",
					get_cached_username(dn->dn_uid),
					get_cached_groupname(dn->dn_gid));
			}
		}
#endif
#सचे ENABLE_SELINUX
	}
	सचे (opt & OPT_Z) {
		column += printf("%-32s ", dn->sid ? dn->sid : "?");
		freecon(dn->sid);
	}
	सचे (opt & OPT_l) {
#endif
		/* long listing: show size */
		सचे (S_ISBLK(dn->dn_mode) || S_ISCHR(dn->dn_mode)) {
			column += printf("%4u, %3u ",
					dn->dn_rdev_maj,
					dn->dn_rdev_min);
		} अञ्ञथा {
			सचे (opt & OPT_h) {
				column += printf("%"HUMAN_READABLE_MAX_WIDTH_STR"s ",
					/* print size, show one fractional, use suffixes */
					make_human_readable_str(dn->dn_size, 1, 0)
				);
			} अञ्ञथा {
				column += printf("%9"OFF_FMT"u ", dn->dn_size);
			}
		}
#सचे ENABLE_FEATURE_LS_TIMESTAMPS
		/* long listing: show {m,c,a}time */
		सचे (opt & OPT_full_time) { /* --full-time */
			/* coreutils 8.4 ls --full-time prints:
			 * 2009-07-13 17:49:27.000000000 +0200
			 * we don't show fractional seconds.
			 */
			वर्ण buf[परिमाण("YYYY-mm-dd HH:MM:SS TIMEZONE")];
			strftime(buf, परिमाण(buf), "%Y-%m-%d %H:%M:%S %z",
					localtime(&dn->dn_time));
			column += printf("%s ", buf);
		} अञ्ञथा { /* ordinary time format */
			/* G.current_time_t is ~== time(NULL) */
			वर्ण *filetime = ctime(&dn->dn_time);
			/* filetime's format: "Wed Jun 30 21:49:08 1993\n" */
			time_t age = G.current_time_t - dn->dn_time;
			सचे (age < 3600L * 24 * 365 / 2 && age > -15 * 60) {
				/* less than 6 months old */
				/* "mmm dd hh:mm " */
				printf("%.12s ", filetime + 4);
			} अञ्ञथा {
				/* "mmm dd  yyyy " */
				/* "mmm dd yyyyy " after year 9999 :) */
				strchr(filetime + 20, '\n')[0] = ' ';
				printf("%.7s%6s", filetime + 4, filetime + 20);
			}
			column += 13;
		}
#endif
	}

#सचे ENABLE_FEATURE_LS_COLOR
	सचे (G_show_color) {
		mode_t mode = dn->dn_mode_lstat;
		सचे (!mode)
			सचे (lstat(dn->fullname, &statbuf) == 0)
				mode = statbuf.st_mode;
		printf(ESC"[%u;%um", bold(mode), fgcolor(mode));
	}
#endif
	column += print_name(dn->name);
	सचे (G_show_color) {
		printf(ESC"[m");
	}

	सचे (lpath) {
		printf(" -> ");
#सचे ENABLE_FEATURE_LS_FILETYPES || ENABLE_FEATURE_LS_COLOR
		सचे ((opt & (OPT_F|OPT_p))
		 || G_show_color
		) {
			mode_t mode = dn->dn_mode_stat;
			सचे (!mode)
				सचे (stat(dn->fullname, &statbuf) == 0)
					mode = statbuf.st_mode;
# सचे ENABLE_FEATURE_LS_FILETYPES
			append = append_char(mode);
# endif
# सचे ENABLE_FEATURE_LS_COLOR
			सचे (G_show_color) {
				printf(ESC"[%u;%um", bold(mode), fgcolor(mode));
			}
# endif
		}
#endif
		column += print_name(lpath) + 4;
		मुक्त(lpath);
		सचे (G_show_color) {
			printf(ESC"[m");
		}
	}
#सचे ENABLE_FEATURE_LS_FILETYPES
	सचे (opt & (OPT_F|OPT_p)) {
		सचे (append) {
			putchar(append);
			column++;
		}
	}
#endif

	पटिनिवत्त column;
}

स्थिर शून्य display_files(संरचना dnode **dn, unsigned nfiles)
{
	unsigned i, ncols, nrows, row, nc;
	unsigned column;
	unsigned nexttab;
	unsigned column_width = 0; /* used only by coulmnal output */

	सचे (option_mask32 & (OPT_l|OPT_1)) {
		ncols = 1;
	} अञ्ञथा {
		/* find the longest file name, use that as the column width */
		अत्थाय (i = 0; dn[i]; i++) {
			पूर्णाङ्क len = calc_name_len(dn[i]->name);
			सचे (column_width < len)
				column_width = len;
		}
		column_width += 2
			+ ((option_mask32 & OPT_Z) ? 33 : 0) /* context width */
			+ ((option_mask32 & OPT_i) ? 8 : 0) /* inode# width */
			+ ((option_mask32 & OPT_s) ? 5 : 0) /* "alloc block" width */
		;
		ncols = (unsigned)G_terminal_width / column_width;
	}

	सचे (ncols > 1) {
		nrows = nfiles / ncols;
		सचे (nrows * ncols < nfiles)
			nrows++;                /* round up fractionals */
	} अञ्ञथा {
		nrows = nfiles;
		ncols = 1;
	}

	column = 0;
	nexttab = 0;
	अत्थाय (row = 0; row < nrows; row++) {
		अत्थाय (nc = 0; nc < ncols; nc++) {
			/* reach into the array based on the column and row */
			सचे (option_mask32 & OPT_x)
				i = (row * ncols) + nc;	/* display across row */
			अञ्ञथा
				i = (nc * nrows) + row;	/* display by column */
			सचे (i < nfiles) {
				सचे (column > 0) {
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

स्थिर संरचना dnode *my_stat(ध्रुव वर्ण *fullname, ध्रुव वर्ण *name, पूर्णाङ्क force_follow)
{
	संरचना stat statbuf;
	संरचना dnode *cur;

	cur = xzalloc(परिमाण(*cur));
	cur->fullname = fullname;
	cur->name = name;

	सचे ((option_mask32 & OPT_L) || force_follow) {
#सचे ENABLE_SELINUX
		सचे (option_mask32 & OPT_Z) {
			getfilecon(fullname, &cur->sid);
		}
#endif
		सचे (stat(fullname, &statbuf)) {
			bb_simple_perror_सन्देशः(fullname);
			G.exit_code = EXIT_FAILURE;
			मुक्त(cur);
			पटिनिवत्त रिक्त;
		}
		cur->dn_mode_stat = statbuf.st_mode;
	} अञ्ञथा {
#सचे ENABLE_SELINUX
		सचे (option_mask32 & OPT_Z) {
			lgetfilecon(fullname, &cur->sid);
		}
#endif
		सचे (lstat(fullname, &statbuf)) {
			bb_simple_perror_सन्देशः(fullname);
			G.exit_code = EXIT_FAILURE;
			मुक्त(cur);
			पटिनिवत्त रिक्त;
		}
		cur->dn_mode_lstat = statbuf.st_mode;
	}

	/* cur->dstat = statbuf: */
	cur->dn_mode   = statbuf.st_mode  ;
	cur->dn_size   = statbuf.st_size  ;
#सचे ENABLE_FEATURE_LS_TIMESTAMPS || ENABLE_FEATURE_LS_SORTFILES
	cur->dn_time   = statbuf.st_mtime ;
	सचे (option_mask32 & OPT_u)
		cur->dn_time = statbuf.st_atime;
	सचे (option_mask32 & OPT_c)
		cur->dn_time = statbuf.st_ctime;
#endif
	cur->dn_ino    = statbuf.st_ino   ;
	cur->dn_blocks = statbuf.st_blocks;
	cur->dn_nlink  = statbuf.st_nlink ;
	cur->dn_uid    = statbuf.st_uid   ;
	cur->dn_gid    = statbuf.st_gid   ;
	cur->dn_rdev_maj = major(statbuf.st_rdev);
	cur->dn_rdev_min = minor(statbuf.st_rdev);

	पटिनिवत्त cur;
}

स्थिर unsigned गणनम्_dirs(संरचना dnode **dn, पूर्णाङ्क which)
{
	unsigned dirs, all;

	सचे (!dn)
		पटिनिवत्त 0;

	dirs = all = 0;
	अत्थाय (; *dn; dn++) {
		ध्रुव वर्ण *name;

		all++;
		सचे (!S_ISDIR((*dn)->dn_mode))
			अनुवर्त;

		name = (*dn)->name;
		सचे (which != SPLIT_SUBDIR /* if not requested to skip . / .. */
		 /* or if it's not . or .. */
		 || name[0] != '.'
		 || (name[1] && (name[1] != '.' || name[2]))
		) {
			dirs++;
		}
	}
	पटिनिवत्त which != SPLIT_FILE ? dirs : all - dirs;
}

/* get memory to hold an array of pointers */
स्थिर संरचना dnode **dnalloc(unsigned अङ्कः)
{
	सचे (अङ्कः < 1)
		पटिनिवत्त रिक्त;

	अङ्कः++; /* so that we have terminating NULL */
	पटिनिवत्त xzalloc(अङ्कः * परिमाण(संरचना dnode *));
}

#सचे ENABLE_FEATURE_LS_RECURSIVE
स्थिर शून्य dfree(संरचना dnode **dnp)
{
	unsigned i;

	सचे (dnp == रिक्त)
		पटिनिवत्त;

	अत्थाय (i = 0; dnp[i]; i++) {
		संरचना dnode *cur = dnp[i];
		सचे (cur->fname_allocated)
			मुक्त((वर्ण*)cur->fullname);
		मुक्त(cur);
	}
	मुक्त(dnp);
}
#अञ्ञथा
#define dfree(...) ((शून्य)0)
#endif

/* Returns NULL-terminated malloced vector of pointers (or NULL) */
स्थिर संरचना dnode **splitdnarray(संरचना dnode **dn, पूर्णाङ्क which)
{
	unsigned dncnt, d;
	संरचना dnode **dnp;

	सचे (dn == रिक्त)
		पटिनिवत्त रिक्त;

	/* count how many dirs or files there are */
	dncnt = गणनम्_dirs(dn, which);

	/* allocate a file array and a dir array */
	dnp = dnalloc(dncnt);

	/* copy the entrys into the file or dir array */
	अत्थाय (d = 0; *dn; dn++) {
		सचे (S_ISDIR((*dn)->dn_mode)) {
			ध्रुव वर्ण *name;

			सचे (which == SPLIT_FILE)
				अनुवर्त;

			name = (*dn)->name;
			सचे ((which & SPLIT_DIR) /* any dir... */
			/* ... or not . or .. */
			 || name[0] != '.'
			 || (name[1] && (name[1] != '.' || name[2]))
			) {
				dnp[d++] = *dn;
			}
		} अञ्ञथा
		सचे (which == SPLIT_FILE) {
			dnp[d++] = *dn;
		}
	}
	पटिनिवत्त dnp;
}

#सचे ENABLE_FEATURE_LS_SORTFILES
स्थिर पूर्णाङ्क sortcmp(ध्रुव शून्य *a, ध्रुव शून्य *b)
{
	संरचना dnode *d1 = *(संरचना dnode **)a;
	संरचना dnode *d2 = *(संरचना dnode **)b;
	unsigned opt = option_mask32;
	off_t dif;

	dif = 0; /* assume sort by name */
	// TODO: use pre-initialized function pointer
	// instead of branch forest
	सचे (opt & OPT_dirs_first) {
		dif = S_ISDIR(d2->dn_mode) - S_ISDIR(d1->dn_mode);
		सचे (dif != 0)
			गच्छ maybe_invert_and_ret;
	}

	सचे (opt & OPT_S) { /* sort by size */
		dif = (d2->dn_size - d1->dn_size);
	} अञ्ञथा
	सचे (opt & OPT_t) { /* sort by time */
		dif = (d2->dn_time - d1->dn_time);
	} अञ्ञथा
#सचे defined(HAVE_STRVERSCMP) && HAVE_STRVERSCMP == 1
	सचे (opt & OPT_v) { /* sort by version */
		dif = strverscmp(d1->name, d2->name);
	} अञ्ञथा
#endif
	सचे (opt & OPT_X) { /* sort by extension */
		dif = strcmp(strchrnul(d1->name, '.'), strchrnul(d2->name, '.'));
	}
	सचे (dif == 0) {
		/* sort by name, use as tie breaker for other sorts */
		सचे (ENABLE_LOCALE_SUPPORT)
			dif = strcoll(d1->name, d2->name);
		अञ्ञथा
			dif = strcmp(d1->name, d2->name);
	} अञ्ञथा {
		/* Make dif fit into an int */
		सचे (परिमाण(dif) > परिमाण(पूर्णाङ्क)) {
			गणना { BITS_TO_SHIFT = 8 * (परिमाण(dif) - परिमाण(पूर्णाङ्क)) };
			/* shift leaving only "int" worth of bits */
			/* (this requires dif != 0, and here it is nonzero) */
			dif = 1 | (पूर्णाङ्क)((uoff_t)dif >> BITS_TO_SHIFT);
		}
	}
 maybe_invert_and_ret:
	पटिनिवत्त (opt & OPT_r) ? -(पूर्णाङ्क)dif : (पूर्णाङ्क)dif;
}

स्थिर शून्य dnsort(संरचना dnode **dn, पूर्णाङ्क size)
{
	qsort(dn, size, परिमाण(*dn), sortcmp);
}

स्थिर शून्य sort_and_display_files(संरचना dnode **dn, unsigned nfiles)
{
	dnsort(dn, nfiles);
	display_files(dn, nfiles);
}
#अञ्ञथा
# define dnsort(dn, size) ((शून्य)0)
# define sort_and_display_files(dn, nfiles) display_files(dn, nfiles)
#endif

/* Returns NULL-terminated malloced vector of pointers (or NULL) */
स्थिर संरचना dnode **scan_one_dir(ध्रुव वर्ण *path, unsigned *nfiles_p)
{
	संरचना dnode *dn, *cur, **dnp;
	संरचना dirent *entry;
	DIR *dir;
	unsigned i, nfiles;

	*nfiles_p = 0;
	dir = warn_opendir(path);
	सचे (dir == रिक्त) {
		G.exit_code = EXIT_FAILURE;
		पटिनिवत्त रिक्त;	/* could not open the dir */
	}
	dn = रिक्त;
	nfiles = 0;
	याव ((entry = readdir(dir)) != रिक्त) {
		वर्ण *fullname;

		/* are we going to list the file- it may be . or .. or a hidden file */
		सचे (entry->d_name[0] == '.') {
			सचे (!(option_mask32 & (OPT_a|OPT_A)))
				अनुवर्त; /* skip all dotfiles if no -a/-A */
			सचे (!(option_mask32 & OPT_a)
			 && (!entry->d_name[1] || (entry->d_name[1] == '.' && !entry->d_name[2]))
			) {
				अनुवर्त; /* if only -A, skip . and .. but show other dotfiles */
			}
		}
		fullname = concat_path_file(path, entry->d_name);
		cur = my_stat(fullname, bb_basename(fullname), 0);
		सचे (!cur) {
			मुक्त(fullname);
			अनुवर्त;
		}
		cur->fname_allocated = 1;
		cur->dn_next = dn;
		dn = cur;
		nfiles++;
	}
	closedir(dir);

	सचे (dn == रिक्त)
		पटिनिवत्त रिक्त;

	/* now that we know how many files there are
	 * allocate memory for an array to hold dnode pointers
	 */
	*nfiles_p = nfiles;
	dnp = dnalloc(nfiles);
	अत्थाय (i = 0; /* i < nfiles - detected via !dn below */; i++) {
		dnp[i] = dn;	/* save pointer to node in array */
		dn = dn->dn_next;
		सचे (!dn)
			भङ्ग;
	}

	पटिनिवत्त dnp;
}

#सचे ENABLE_DESKTOP
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
स्थिर off_t calculate_blocks(संरचना dnode **dn)
{
	uoff_t blocks = 1;
	सचे (dn) {
		याव (*dn) {
			/* st_blocks is in 512 byte blocks */
			blocks += (*dn)->dn_blocks;
			dn++;
		}
	}

	/* Even though standard says use 512 byte blocks, coreutils use 1k */
	/* Actually, we round up by calculating (blocks + 1) / 2,
	 * "+ 1" was done when we initialized blocks to 1 */
	पटिनिवत्त blocks >> 1;
}
#endif

स्थिर शून्य scan_and_display_dirs_recur(संरचना dnode **dn, पूर्णाङ्क first)
{
	unsigned nfiles;
	संरचना dnode **subdnp;

	अत्थाय (; *dn; dn++) {
		सचे (G.show_dirname || (option_mask32 & OPT_R)) {
			सचे (!first)
				bb_putchar('\n');
			first = 0;
			printf("%s:\n", (*dn)->fullname);
		}
		subdnp = scan_one_dir((*dn)->fullname, &nfiles);
#सचे ENABLE_DESKTOP
		सचे (option_mask32 & (OPT_s|OPT_l)) {
			सचे (option_mask32 & OPT_h) {
				printf("total %-"HUMAN_READABLE_MAX_WIDTH_STR"s\n",
					/* print size, no fractions, use suffixes */
					make_human_readable_str(calculate_blocks(subdnp) * 1024,
								0, 0)
				);
			} अञ्ञथा {
				printf("total %"OFF_FMT"u\n", calculate_blocks(subdnp));
			}
		}
#endif
		सचे (nfiles > 0) {
			/* list all files at this level */
			sort_and_display_files(subdnp, nfiles);

			सचे (ENABLE_FEATURE_LS_RECURSIVE
			 && (option_mask32 & OPT_R)
			) {
				संरचना dnode **dnd;
				unsigned dndirs;
				/* recursive - list the sub-dirs */
				dnd = splitdnarray(subdnp, SPLIT_SUBDIR);
				dndirs = गणनम्_dirs(subdnp, SPLIT_SUBDIR);
				सचे (dndirs > 0) {
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


पूर्णाङ्क ls_main(पूर्णाङ्क argc UNUSED_PARAM, वर्ण **argv)
{	/*      ^^^^^^^^^^^^^^^^^ note: if FTPD, argc can be wrong, see ftpd.c */
	संरचना dnode **dnd;
	संरचना dnode **dnf;
	संरचना dnode **dnp;
	संरचना dnode *dn;
	संरचना dnode *cur;
	unsigned opt;
	unsigned nfiles;
	unsigned dnfiles;
	unsigned dndirs;
	unsigned i;
#सचे ENABLE_FEATURE_LS_COLOR
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
	स्थिर ध्रुव वर्ण color_str[] ALIGN1 =
		"always\0""yes\0""force\0"
		"auto\0""tty\0""if-tty\0";
	/* need to initialize since --color has _an optional_ argument */
	ध्रुव वर्ण *color_opt = color_str; /* "always" */
#endif
#सचे ENABLE_LONG_OPTS
	स्थिर ध्रुव वर्ण ls_longopts[] ALIGN1 =
		"full-time\0" No_argument "\xff"
		"group-directories-first\0" No_argument "\xfe"
		IF_FEATURE_LS_COLOR("color\0" Optional_argument "\xfd")
	;
#endif

	INIT_G();

	init_unicode();

#सचे ENABLE_FEATURE_LS_WIDTH
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
		IF_FEATURE_LS_WIDTH(, /*-T*/ रिक्त, /*-w*/ &G_terminal_width)
		IF_FEATURE_LS_COLOR(, &color_opt)
	);
#सचे 0 /* option bits debug */
	bb_error_सन्देशः("opt:0x%08x l:%x H:%x color:%x dirs:%x", opt, OPT_l, OPT_H, OPT_color, OPT_dirs_first);
	सचे (opt & OPT_c         ) bb_error_सन्देशः("-c");
	सचे (opt & OPT_l         ) bb_error_सन्देशः("-l");
	सचे (opt & OPT_H         ) bb_error_सन्देशः("-H");
	सचे (opt & OPT_color     ) bb_error_सन्देशः("--color");
	सचे (opt & OPT_dirs_first) bb_error_सन्देशः("--group-directories-first");
	सचे (opt & OPT_full_time ) bb_error_सन्देशः("--full-time");
	exit(0);
#endif

#सचे ENABLE_SELINUX
	सचे (opt & OPT_Z) {
		सचे (!is_selinux_enabled())
			option_mask32 &= ~OPT_Z;
	}
#endif

#सचे ENABLE_FEATURE_LS_COLOR
	/* set G_show_color = 1/0 */
	सचे (ENABLE_FEATURE_LS_COLOR_IS_DEFAULT && !is_TERM_dumb()) {
		वर्ण *p = getenv("LS_COLORS");
		/* LS_COLORS is unset, or (not empty && not "none") ? */
		सचे (!p || (p[0] && strcmp(p, "none") != 0)) {
			सचे (isatty(STDOUT_FILENO)) {
				/* check isatty() last because it's expensive (syscall) */
				G_show_color = 1;
			}
		}
	}
	सचे (opt & OPT_color) {
		सचे (color_opt[0] == 'n')
			G_show_color = 0;
		अञ्ञथा विकल्प (index_in_substrings(color_str, color_opt)) {
		पक्ष 3:
		पक्ष 4:
		पक्ष 5:
			सचे (!is_TERM_dumb() && isatty(STDOUT_FILENO)) {
		पक्ष 0:
		पक्ष 1:
		पक्ष 2:
				G_show_color = 1;
			}
		}
	}
#endif

	/* sort out which command line options take precedence */
	सचे (ENABLE_FEATURE_LS_RECURSIVE && (opt & OPT_d))
		option_mask32 &= ~OPT_R;	/* no recurse if listing only dir */
	सचे (!(opt & OPT_l)) { /* not -l? */
		सचे (ENABLE_FEATURE_LS_TIMESTAMPS && ENABLE_FEATURE_LS_SORTFILES) {
			/* when to sort by time? -t[cu] sorts by time even with -l */
			/* (this is achieved by opt_flags[] element for -t) */
			/* without -l, bare -c or -u enable sort too */
			/* (with -l, bare -c or -u just select which time to show) */
			सचे (opt & (OPT_c|OPT_u)) {
				option_mask32 |= OPT_t;
			}
		}
	}

	/* choose a display format if one was not already specified by an option */
	सचे (!(option_mask32 & (OPT_l|OPT_1|OPT_x|OPT_C)))
		option_mask32 |= (isatty(STDOUT_FILENO) ? OPT_C : OPT_1);

	सचे (ENABLE_FTPD && applet_name[0] == 'f') {
		/* ftpd secret backdoor. dirs first are much nicer */
		option_mask32 |= OPT_dirs_first;
	}

	argv += optind;
	सचे (!argv[0])
		*--argv = (वर्ण*)".";

	सचे (argv[1])
		G.show_dirname = 1; /* 2 or more items? label directories */

	/* stuff the command line file names into a dnode array */
	dn = रिक्त;
	nfiles = 0;
	करोहि {
		cur = my_stat(*argv, *argv,
			/* follow links on command line unless -l, -i, -s or -F: */
			!(option_mask32 & (OPT_l|OPT_i|OPT_s|OPT_F))
			/* ... or if -H: */
			|| (option_mask32 & OPT_H)
			/* ... or if -L, but my_stat always follows links if -L */
		);
		argv++;
		सचे (!cur)
			अनुवर्त;
		/*cur->fname_allocated = 0; - already is */
		cur->dn_next = dn;
		dn = cur;
		nfiles++;
	} याव (*argv);

	/* nfiles _may_ be 0 here - try "ls doesnt_exist" */
	सचे (nfiles == 0)
		पटिनिवत्त G.exit_code;

	/* now that we know how many files there are
	 * allocate memory for an array to hold dnode pointers
	 */
	dnp = dnalloc(nfiles);
	अत्थाय (i = 0; /* i < nfiles - detected via !dn below */; i++) {
		dnp[i] = dn;	/* save pointer to node in array */
		dn = dn->dn_next;
		सचे (!dn)
			भङ्ग;
	}

	सचे (option_mask32 & OPT_d) {
		sort_and_display_files(dnp, nfiles);
	} अञ्ञथा {
		dnd = splitdnarray(dnp, SPLIT_DIR);
		dnf = splitdnarray(dnp, SPLIT_FILE);
		dndirs = गणनम्_dirs(dnp, SPLIT_DIR);
		dnfiles = nfiles - dndirs;
		सचे (dnfiles > 0) {
			sort_and_display_files(dnf, dnfiles);
			सचे (ENABLE_FEATURE_CLEAN_UP)
				मुक्त(dnf);
		}
		सचे (dndirs > 0) {
			dnsort(dnd, dndirs);
			scan_and_display_dirs_recur(dnd, dnfiles == 0);
			सचे (ENABLE_FEATURE_CLEAN_UP)
				मुक्त(dnd);
		}
	}

	सचे (ENABLE_FEATURE_CLEAN_UP)
		dfree(dnp);
	पटिनिवत्त G.exit_code;
}
