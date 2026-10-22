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

#అన్తర్భావ "libbb.h"
#అన్తర్భావ "common_bufsiz.h"
#అన్తర్భావ "unicode.h"


/* This is a NOEXEC applet. Be very careful! */


#ఒకవేళ ENABLE_FTPD
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
#అయితే_ముగింపు


గణన {
TERMINAL_వెడల్పు  = 80, /* use 79 if terminal has linefold bug */

విభజించు_FILE      = 0,
విభజించు_DIR       = 1,
విభజించు_SUBDIR    = 2,
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
#నిర్వచించు ls_options \
	"Cadi1lgnsxAk"       /* 12 opts, total 12 */ \
	IF_FEATURE_LS_FILETYPES("Fp")    /* 2, 14 */ \
	IF_FEATURE_LS_RECURSIVE("R")     /* 1, 15 */ \
	IF_SELINUX("Z")                  /* 1, 16 */ \
	"Q"                              /* 1, 17 */ \
	IF_FEATURE_LS_TIMESTAMPS("ctu")  /* 3, 20 */ \
	IF_FEATURE_LS_SORTFILES("SXrv")  /* 4, 24 */ \
	IF_FEATURE_LS_FOLLOWLINKS("LH")  /* 2, 26 */ \
	IF_FEATURE_HUMAN_READABLE("h")   /* 1, 27 */ \
	IF_FEATURE_LS_వెడల్పు("T:w:")      /* 2, 29 */

గణన {
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
	OPTBIT_నిండిన_సమయం = OPTBIT_T + 2 * ENABLE_FEATURE_LS_వెడల్పు,
	OPTBIT_dirs_మొదటి,
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
	OPT_T = (1 << OPTBIT_T) * ENABLE_FEATURE_LS_వెడల్పు,
	OPT_w = (1 << OPTBIT_w) * ENABLE_FEATURE_LS_వెడల్పు,
	OPT_నిండిన_సమయం  = (1 << OPTBIT_నిండిన_సమయం ) * ENABLE_LONG_OPTS,
	OPT_dirs_మొదటి = (1 << OPTBIT_dirs_మొదటి) * ENABLE_LONG_OPTS,
	OPT_color      = (1 << OPTBIT_color     ) * ENABLE_FEATURE_LS_COLOR,
};

/*
 * a directory entry and its stat info
 */
నిర్మాణం dnode {
	స్థిరరాశి అక్షరం *name;       /* usually basename, but think "ls -l dir/file" */
	స్థిరరాశి అక్షరం *fullname;   /* full name (usable for stat etc) */
	నిర్మాణం dnode *dn_తదుపరి;  /* for linked list */
	IF_SELINUX(security_context_t sid;)
	smallint fname_allocated;

	/* Used to avoid re-doing [l]stat at printout stage
	 * if we already collected needed data in scan stage:
	 */
	బహుళకం_t    dn_బహుళకం_lstat;   /* obtained with lstat, or 0 */
	బహుళకం_t    dn_బహుళకం_stat;    /* obtained with stat, or 0 */

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
	బహుళకం_t    dn_బహుళకం; /* obtained with lstat OR stat, depending on -L etc */
	off_t     dn_కొలత;
#ఒకవేళ ENABLE_FEATURE_LS_TIMESTAMPS || ENABLE_FEATURE_LS_SORTFILES
	సమయం_t    dn_సమయం;
#అయితే_ముగింపు
	ino_t     dn_ino;
	blkcnt_t  dn_ఖండాలు;
	nlink_t   dn_nlink;
	uid_t     dn_uid;
	gid_t     dn_gid;
	పూర్ణాంకం       dn_rdev_maj;
	పూర్ణాంకం       dn_rdev_min;
//	dev_t     dn_dev;
//	blksize_t dn_blksize;
};

నిర్మాణం globals {
#ఒకవేళ ENABLE_FEATURE_LS_COLOR
	smallint show_color;
# define G_show_color (G.show_color)
#లేకపోతే
# define G_show_color 0
#అయితే_ముగింపు
	smallint exit_సంకేతం;
	smallint show_dirname;
#ఒకవేళ ENABLE_FEATURE_LS_వెడల్పు
	గుర్తులేని terminal_వెడల్పు;
# define G_terminal_వెడల్పు (G.terminal_వెడల్పు)
#లేకపోతే
# define G_terminal_వెడల్పు TERMINAL_వెడల్పు
#అయితే_ముగింపు
#ఒకవేళ ENABLE_FEATURE_LS_TIMESTAMPS
	/* Do time() just once. Saves one syscall per file for "ls -l" */
	సమయం_t current_సమయం_t;
#అయితే_ముగింపు
} FIX_ALIASING;
#నిర్వచించు G (*(నిర్మాణం globals*)bb_common_bufsiz1)
#నిర్వచించు INIT_G() చేయి { \
	అమరిక_common_bufsiz(); \
	/* we have to zero it out because of NOEXEC */ \
	memset(&G, 0, పరిమాణం(G)); \
	IF_FEATURE_LS_వెడల్పు(G_terminal_వెడల్పు = TERMINAL_వెడల్పు;) \
	IF_FEATURE_LS_TIMESTAMPS(సమయం(&G.current_సమయం_t);) \
} అయితే (0)

#నిర్వచించు ESC "\033"


/*** Output code ***/


/* FYI type values: 1:fifo 2:char 4:dir 6:blk 8:file 10:link 12:socket
 * (various wacky OSes: 13:Sun door 14:BSD whiteout 5:XENIX named file
 *  3/7:multiplexed char/block device)
 * and we use 0 for unknown and 15 for executables (see below) */
#నిర్వచించు TYPEINDEX(బహుళకం) (((బహుళకం) >> 12) & 0x0f)
/*                       un  fi chr -   dir -  blk  -  file -  link - sock -   - exe */
#నిర్వచించు APPCHAR(బహుళకం)   ("\0""|""\0""\0""/""\0""\0""\0""\0""\0""@""\0""=""\0""\0""\0" [TYPEINDEX(బహుళకం)])
/* 036 black foreground              050 black background
   037 red foreground                051 red background
   040 green foreground              052 green background
   041 brown foreground              053 brown background
   042 blue foreground               054 blue background
   043 magenta (purple) foreground   055 magenta background
   044 cyan (light blue) foreground  056 cyan background
   045 gray foreground               057 white background
*/
#నిర్వచించు COLOR(బహుళకం) ( \
	/*un  fi  chr  -  dir  -  blk  -  file -  link -  sock -   -  exe */ \
	"\037\043\043\045\042\045\043\043\000\045\044\045\043\045\045\040" \
	[TYPEINDEX(బహుళకం)])
/* Select normal (0) [actually "reset all"] or bold (1)
 * (other attributes are 2:dim 4:underline 5:blink 7:reverse,
 *  let's use 7 for "impossible" types, just for fun)
 * Note: coreutils 6.9 uses inverted red for setuid binaries.
 */
#నిర్వచించు ATTR(బహుళకం) ( \
	/*un fi chr - dir - blk - file- link- sock- -  exe */ \
	"\01\00\01\07\01\07\01\07\00\07\01\07\01\07\07\01" \
	[TYPEINDEX(బహుళకం)])

#ఒకవేళ ENABLE_FEATURE_LS_COLOR
/* mode of zero is interpreted as "unknown" (stat failed) */
స్థిర అక్షరం fgcolor(బహుళకం_t బహుళకం)
{
	ఒకవేళ (S_ISREG(బహుళకం) && (బహుళకం & (S_IXUSR | S_IXGRP | S_IXOTH)))
		తిరిగి COLOR(0xF000);	/* File is executable ... */
	తిరిగి COLOR(బహుళకం);
}
స్థిర అక్షరం bold(బహుళకం_t బహుళకం)
{
	ఒకవేళ (S_ISREG(బహుళకం) && (బహుళకం & (S_IXUSR | S_IXGRP | S_IXOTH)))
		తిరిగి ATTR(0xF000);	/* File is executable ... */
	తిరిగి ATTR(బహుళకం);
}
#అయితే_ముగింపు

#ఒకవేళ ENABLE_FEATURE_LS_FILETYPES
స్థిర అక్షరం append_char(బహుళకం_t బహుళకం)
{
	ఒకవేళ (!(option_mask32 & (OPT_F|OPT_p)))
		తిరిగి '\0';

	ఒకవేళ (S_ISDIR(బహుళకం))
		తిరిగి '/';
	ఒకవేళ (!(option_mask32 & OPT_F))
		తిరిగి '\0';
	ఒకవేళ (S_ISREG(బహుళకం) && (బహుళకం & (S_IXUSR | S_IXGRP | S_IXOTH)))
		తిరిగి '*';
	తిరిగి APPCHAR(బహుళకం);
}
#అయితే_ముగింపు

స్థిర గుర్తులేని calc_name_len(స్థిరరాశి అక్షరం *name)
{
	గుర్తులేని len;
	uni_stat_t uni_stat;

	// TODO: quote tab as \t, etc, if -Q
	name = printable_string2(&uni_stat, name);

	ఒకవేళ (!(option_mask32 & OPT_Q)) {
		తిరిగి uni_stat.unicode_వెడల్పు;
	}

	len = 2 + uni_stat.unicode_వెడల్పు;
	అయితే (*name) {
		ఒకవేళ (*name == '"' || *name == '\\') {
			len++;
		}
		name++;
	}
	తిరిగి len;
}

/* Return the number of used columns.
 * Note that only columnar output uses return value.
 * -l and -1 modes don't care.
 * coreutils 7.2 also supports:
 * ls -b (--escape) = octal escapes (although it doesn't look like working)
 * ls -N (--literal) = not escape at all
 */
స్థిర గుర్తులేని print_name(స్థిరరాశి అక్షరం *name)
{
	గుర్తులేని len;
	uni_stat_t uni_stat;

	// TODO: quote tab as \t, etc, if -Q
	name = printable_string2(&uni_stat, name);

	ఒకవేళ (!(option_mask32 & OPT_Q)) {
		fputs_stdout(name);
		తిరిగి uni_stat.unicode_వెడల్పు;
	}

	len = 2 + uni_stat.unicode_వెడల్పు;
	అక్షరం_రాయి('"');
	అయితే (*name) {
		ఒకవేళ (*name == '"' || *name == '\\') {
			అక్షరం_రాయి('\\');
			len++;
		}
		అక్షరం_రాయి(*name);
		name++;
	}
	అక్షరం_రాయి('"');
	తిరిగి len;
}

/* Return the number of used columns.
 * Note that only columnar output uses return value,
 * -l and -1 modes don't care.
 */
స్థిర NOINLINE గుర్తులేని display_single(స్థిరరాశి నిర్మాణం dnode *dn)
{
	గుర్తులేని స్తంభం = 0;
	అక్షరం *lpath;
	పూర్ణాంకం opt;
#ఒకవేళ ENABLE_FEATURE_LS_FILETYPES || ENABLE_FEATURE_LS_COLOR
	నిర్మాణం stat statbuf;
#అయితే_ముగింపు
#ఒకవేళ ENABLE_FEATURE_LS_FILETYPES
	అక్షరం append = append_char(dn->dn_బహుళకం);
#అయితే_ముగింపు

	opt = option_mask32;

	/* Do readlink early, so that if it fails, error message
	 * does not appear *inside* the "ls -l" line */
	lpath = ఖాళీ;
	ఒకవేళ (opt & OPT_l)
		ఒకవేళ (S_ISLNK(dn->dn_బహుళకం))
			lpath = xmalloc_readlink_or_warn(dn->fullname);

	ఒకవేళ (opt & OPT_i) /* show inode# */
		స్తంభం += రాయి("%7llu ", (దీర్ఘ దీర్ఘ) dn->dn_ino);
	ఒకవేళ (opt & OPT_s) { /* show allocated blocks */
		ఒకవేళ (opt & OPT_h) {
			స్తంభం += రాయి("%"HUMAN_READABLE_MAX_వెడల్పు_STR"s ",
				/* print size, show one fractional, use suffixes */
				make_human_readable_str((off_t)dn->dn_ఖండాలు << 9, 1, 0)
			);
		} లేకపోతే {
			స్తంభం += రాయి("%6"OFF_FMT"u ", (off_t)(dn->dn_ఖండాలు >> 1));
		}
	}
	ఒకవేళ (opt & OPT_l) {
		/* long listing: show mode */
		అక్షరం modestr[12];
		స్తంభం += రాయి("%-10s ", bb_బహుళకం_string(modestr, dn->dn_బహుళకం));
		/* long listing: show number of links */
		స్తంభం += రాయి("%4lu ", (దీర్ఘ) dn->dn_nlink);
		/* long listing: show user/group */
		ఒకవేళ (opt & OPT_n) {
			ఒకవేళ (opt & OPT_g)
				స్తంభం += రాయి("%-8u ", (పూర్ణాంకం) dn->dn_gid);
			లేకపోతే
				స్తంభం += రాయి("%-8u %-8u ",
						(పూర్ణాంకం) dn->dn_uid,
						(పూర్ణాంకం) dn->dn_gid);
		}
#ఒకవేళ ENABLE_FEATURE_LS_USERNAME
		లేకపోతే {
			ఒకవేళ (opt & OPT_g) {
				స్తంభం += రాయి("%-8s ",
					get_cached_groupname(dn->dn_gid));
			} లేకపోతే {
				స్తంభం += రాయి("%-8s %-8s ",
					get_cached_username(dn->dn_uid),
					get_cached_groupname(dn->dn_gid));
			}
		}
#అయితే_ముగింపు
#ఒకవేళ ENABLE_SELINUX
	}
	ఒకవేళ (opt & OPT_Z) {
		స్తంభం += రాయి("%-32s ", dn->sid ? dn->sid : "?");
		freecon(dn->sid);
	}
	ఒకవేళ (opt & OPT_l) {
#అయితే_ముగింపు
		/* long listing: show size */
		ఒకవేళ (S_ISBLK(dn->dn_బహుళకం) || S_ISCHR(dn->dn_బహుళకం)) {
			స్తంభం += రాయి("%4u, %3u ",
					dn->dn_rdev_maj,
					dn->dn_rdev_min);
		} లేకపోతే {
			ఒకవేళ (opt & OPT_h) {
				స్తంభం += రాయి("%"HUMAN_READABLE_MAX_వెడల్పు_STR"s ",
					/* print size, show one fractional, use suffixes */
					make_human_readable_str(dn->dn_కొలత, 1, 0)
				);
			} లేకపోతే {
				స్తంభం += రాయి("%9"OFF_FMT"u ", dn->dn_కొలత);
			}
		}
#ఒకవేళ ENABLE_FEATURE_LS_TIMESTAMPS
		/* long listing: show {m,c,a}time */
		ఒకవేళ (opt & OPT_నిండిన_సమయం) { /* --full-time */
			/* coreutils 8.4 ls --full-time prints:
			 * 2009-07-13 17:49:27.000000000 +0200
			 * we don't show fractional seconds.
			 */
			అక్షరం buf[పరిమాణం("YYYY-mm-dd HH:MM:SS TIMEZONE")];
			strftime(buf, పరిమాణం(buf), "%Y-%m-%d %H:%M:%S %z",
					localtime(&dn->dn_సమయం));
			స్తంభం += రాయి("%s ", buf);
		} లేకపోతే { /* ordinary time format */
			/* G.current_time_t is ~== time(NULL) */
			అక్షరం *filetime = ctime(&dn->dn_సమయం);
			/* filetime's format: "Wed Jun 30 21:49:08 1993\n" */
			సమయం_t age = G.current_సమయం_t - dn->dn_సమయం;
			ఒకవేళ (age < 3600L * 24 * 365 / 2 && age > -15 * 60) {
				/* less than 6 months old */
				/* "mmm dd hh:mm " */
				రాయి("%.12s ", filetime + 4);
			} లేకపోతే {
				/* "mmm dd  yyyy " */
				/* "mmm dd yyyyy " after year 9999 :) */
				strchr(filetime + 20, '\n')[0] = ' ';
				రాయి("%.7s%6s", filetime + 4, filetime + 20);
			}
			స్తంభం += 13;
		}
#అయితే_ముగింపు
	}

#ఒకవేళ ENABLE_FEATURE_LS_COLOR
	ఒకవేళ (G_show_color) {
		బహుళకం_t బహుళకం = dn->dn_బహుళకం_lstat;
		ఒకవేళ (!బహుళకం)
			ఒకవేళ (lstat(dn->fullname, &statbuf) == 0)
				బహుళకం = statbuf.st_బహుళకం;
		రాయి(ESC"[%u;%um", bold(బహుళకం), fgcolor(బహుళకం));
	}
#అయితే_ముగింపు
	స్తంభం += print_name(dn->name);
	ఒకవేళ (G_show_color) {
		రాయి(ESC"[m");
	}

	ఒకవేళ (lpath) {
		రాయి(" -> ");
#ఒకవేళ ENABLE_FEATURE_LS_FILETYPES || ENABLE_FEATURE_LS_COLOR
		ఒకవేళ ((opt & (OPT_F|OPT_p))
		 || G_show_color
		) {
			బహుళకం_t బహుళకం = dn->dn_బహుళకం_stat;
			ఒకవేళ (!బహుళకం)
				ఒకవేళ (stat(dn->fullname, &statbuf) == 0)
					బహుళకం = statbuf.st_బహుళకం;
# ఒకవేళ ENABLE_FEATURE_LS_FILETYPES
			append = append_char(బహుళకం);
# endif
# ఒకవేళ ENABLE_FEATURE_LS_COLOR
			ఒకవేళ (G_show_color) {
				రాయి(ESC"[%u;%um", bold(బహుళకం), fgcolor(బహుళకం));
			}
# endif
		}
#అయితే_ముగింపు
		స్తంభం += print_name(lpath) + 4;
		విడుదల(lpath);
		ఒకవేళ (G_show_color) {
			రాయి(ESC"[m");
		}
	}
#ఒకవేళ ENABLE_FEATURE_LS_FILETYPES
	ఒకవేళ (opt & (OPT_F|OPT_p)) {
		ఒకవేళ (append) {
			అక్షరం_రాయి(append);
			స్తంభం++;
		}
	}
#అయితే_ముగింపు

	తిరిగి స్తంభం;
}

స్థిర శూన్య display_files(నిర్మాణం dnode **dn, గుర్తులేని nfiles)
{
	గుర్తులేని i, ncols, nrows, అడ్డువరుస, nc;
	గుర్తులేని స్తంభం;
	గుర్తులేని nexttab;
	గుర్తులేని స్తంభం_వెడల్పు = 0; /* used only by coulmnal output */

	ఒకవేళ (option_mask32 & (OPT_l|OPT_1)) {
		ncols = 1;
	} లేకపోతే {
		/* find the longest file name, use that as the column width */
		కొరకు (i = 0; dn[i]; i++) {
			పూర్ణాంకం len = calc_name_len(dn[i]->name);
			ఒకవేళ (స్తంభం_వెడల్పు < len)
				స్తంభం_వెడల్పు = len;
		}
		స్తంభం_వెడల్పు += 2
			+ ((option_mask32 & OPT_Z) ? 33 : 0) /* context width */
			+ ((option_mask32 & OPT_i) ? 8 : 0) /* inode# width */
			+ ((option_mask32 & OPT_s) ? 5 : 0) /* "alloc block" width */
		;
		ncols = (గుర్తులేని)G_terminal_వెడల్పు / స్తంభం_వెడల్పు;
	}

	ఒకవేళ (ncols > 1) {
		nrows = nfiles / ncols;
		ఒకవేళ (nrows * ncols < nfiles)
			nrows++;                /* round up fractionals */
	} లేకపోతే {
		nrows = nfiles;
		ncols = 1;
	}

	స్తంభం = 0;
	nexttab = 0;
	కొరకు (అడ్డువరుస = 0; అడ్డువరుస < nrows; అడ్డువరుస++) {
		కొరకు (nc = 0; nc < ncols; nc++) {
			/* reach into the array based on the column and row */
			ఒకవేళ (option_mask32 & OPT_x)
				i = (అడ్డువరుస * ncols) + nc;	/* display across row */
			లేకపోతే
				i = (nc * nrows) + అడ్డువరుస;	/* display by column */
			ఒకవేళ (i < nfiles) {
				ఒకవేళ (స్తంభం > 0) {
					nexttab -= స్తంభం;
					రాయి("%*s", nexttab, "");
					స్తంభం += nexttab;
				}
				nexttab = స్తంభం + స్తంభం_వెడల్పు;
				స్తంభం += display_single(dn[i]);
			}
		}
		అక్షరం_రాయి('\n');
		స్తంభం = 0;
	}
}


/*** Dir scanning code ***/

స్థిర నిర్మాణం dnode *my_stat(స్థిరరాశి అక్షరం *fullname, స్థిరరాశి అక్షరం *name, పూర్ణాంకం force_follow)
{
	నిర్మాణం stat statbuf;
	నిర్మాణం dnode *cur;

	cur = xzalloc(పరిమాణం(*cur));
	cur->fullname = fullname;
	cur->name = name;

	ఒకవేళ ((option_mask32 & OPT_L) || force_follow) {
#ఒకవేళ ENABLE_SELINUX
		ఒకవేళ (option_mask32 & OPT_Z) {
			getfilecon(fullname, &cur->sid);
		}
#అయితే_ముగింపు
		ఒకవేళ (stat(fullname, &statbuf)) {
			bb_simple_perror_సందేశం(fullname);
			G.exit_సంకేతం = EXIT_FAILURE;
			విడుదల(cur);
			తిరిగి ఖాళీ;
		}
		cur->dn_బహుళకం_stat = statbuf.st_బహుళకం;
	} లేకపోతే {
#ఒకవేళ ENABLE_SELINUX
		ఒకవేళ (option_mask32 & OPT_Z) {
			lgetfilecon(fullname, &cur->sid);
		}
#అయితే_ముగింపు
		ఒకవేళ (lstat(fullname, &statbuf)) {
			bb_simple_perror_సందేశం(fullname);
			G.exit_సంకేతం = EXIT_FAILURE;
			విడుదల(cur);
			తిరిగి ఖాళీ;
		}
		cur->dn_బహుళకం_lstat = statbuf.st_బహుళకం;
	}

	/* cur->dstat = statbuf: */
	cur->dn_బహుళకం   = statbuf.st_బహుళకం  ;
	cur->dn_కొలత   = statbuf.st_కొలత  ;
#ఒకవేళ ENABLE_FEATURE_LS_TIMESTAMPS || ENABLE_FEATURE_LS_SORTFILES
	cur->dn_సమయం   = statbuf.st_mtime ;
	ఒకవేళ (option_mask32 & OPT_u)
		cur->dn_సమయం = statbuf.st_atime;
	ఒకవేళ (option_mask32 & OPT_c)
		cur->dn_సమయం = statbuf.st_ctime;
#అయితే_ముగింపు
	cur->dn_ino    = statbuf.st_ino   ;
	cur->dn_ఖండాలు = statbuf.st_ఖండాలు;
	cur->dn_nlink  = statbuf.st_nlink ;
	cur->dn_uid    = statbuf.st_uid   ;
	cur->dn_gid    = statbuf.st_gid   ;
	cur->dn_rdev_maj = major(statbuf.st_rdev);
	cur->dn_rdev_min = minor(statbuf.st_rdev);

	తిరిగి cur;
}

స్థిర గుర్తులేని లెక్క_dirs(నిర్మాణం dnode **dn, పూర్ణాంకం which)
{
	గుర్తులేని dirs, all;

	ఒకవేళ (!dn)
		తిరిగి 0;

	dirs = all = 0;
	కొరకు (; *dn; dn++) {
		స్థిరరాశి అక్షరం *name;

		all++;
		ఒకవేళ (!S_ISDIR((*dn)->dn_బహుళకం))
			కొనసాగు;

		name = (*dn)->name;
		ఒకవేళ (which != విభజించు_SUBDIR /* if not requested to skip . / .. */
		 /* or if it's not . or .. */
		 || name[0] != '.'
		 || (name[1] && (name[1] != '.' || name[2]))
		) {
			dirs++;
		}
	}
	తిరిగి which != విభజించు_FILE ? dirs : all - dirs;
}

/* get memory to hold an array of pointers */
స్థిర నిర్మాణం dnode **dnalloc(గుర్తులేని అంకె)
{
	ఒకవేళ (అంకె < 1)
		తిరిగి ఖాళీ;

	అంకె++; /* so that we have terminating NULL */
	తిరిగి xzalloc(అంకె * పరిమాణం(నిర్మాణం dnode *));
}

#ఒకవేళ ENABLE_FEATURE_LS_RECURSIVE
స్థిర శూన్య dfree(నిర్మాణం dnode **dnp)
{
	గుర్తులేని i;

	ఒకవేళ (dnp == ఖాళీ)
		తిరిగి;

	కొరకు (i = 0; dnp[i]; i++) {
		నిర్మాణం dnode *cur = dnp[i];
		ఒకవేళ (cur->fname_allocated)
			విడుదల((అక్షరం*)cur->fullname);
		విడుదల(cur);
	}
	విడుదల(dnp);
}
#లేకపోతే
#నిర్వచించు dfree(...) ((శూన్య)0)
#అయితే_ముగింపు

/* Returns NULL-terminated malloced vector of pointers (or NULL) */
స్థిర నిర్మాణం dnode **splitdnarray(నిర్మాణం dnode **dn, పూర్ణాంకం which)
{
	గుర్తులేని dncnt, d;
	నిర్మాణం dnode **dnp;

	ఒకవేళ (dn == ఖాళీ)
		తిరిగి ఖాళీ;

	/* count how many dirs or files there are */
	dncnt = లెక్క_dirs(dn, which);

	/* allocate a file array and a dir array */
	dnp = dnalloc(dncnt);

	/* copy the entrys into the file or dir array */
	కొరకు (d = 0; *dn; dn++) {
		ఒకవేళ (S_ISDIR((*dn)->dn_బహుళకం)) {
			స్థిరరాశి అక్షరం *name;

			ఒకవేళ (which == విభజించు_FILE)
				కొనసాగు;

			name = (*dn)->name;
			ఒకవేళ ((which & విభజించు_DIR) /* any dir... */
			/* ... or not . or .. */
			 || name[0] != '.'
			 || (name[1] && (name[1] != '.' || name[2]))
			) {
				dnp[d++] = *dn;
			}
		} లేకపోతే
		ఒకవేళ (which == విభజించు_FILE) {
			dnp[d++] = *dn;
		}
	}
	తిరిగి dnp;
}

#ఒకవేళ ENABLE_FEATURE_LS_SORTFILES
స్థిర పూర్ణాంకం sortcmp(స్థిరరాశి శూన్య *a, స్థిరరాశి శూన్య *b)
{
	నిర్మాణం dnode *d1 = *(నిర్మాణం dnode **)a;
	నిర్మాణం dnode *d2 = *(నిర్మాణం dnode **)b;
	గుర్తులేని opt = option_mask32;
	off_t dif;

	dif = 0; /* assume sort by name */
	// TODO: use pre-initialized function pointer
	// instead of branch forest
	ఒకవేళ (opt & OPT_dirs_మొదటి) {
		dif = S_ISDIR(d2->dn_బహుళకం) - S_ISDIR(d1->dn_బహుళకం);
		ఒకవేళ (dif != 0)
			వెళ్ళు maybe_invert_and_ret;
	}

	ఒకవేళ (opt & OPT_S) { /* sort by size */
		dif = (d2->dn_కొలత - d1->dn_కొలత);
	} లేకపోతే
	ఒకవేళ (opt & OPT_t) { /* sort by time */
		dif = (d2->dn_సమయం - d1->dn_సమయం);
	} లేకపోతే
#ఒకవేళ defined(HAVE_STRVERSCMP) && HAVE_STRVERSCMP == 1
	ఒకవేళ (opt & OPT_v) { /* sort by version */
		dif = strverscmp(d1->name, d2->name);
	} లేకపోతే
#అయితే_ముగింపు
	ఒకవేళ (opt & OPT_X) { /* sort by extension */
		dif = strcmp(strchrnul(d1->name, '.'), strchrnul(d2->name, '.'));
	}
	ఒకవేళ (dif == 0) {
		/* sort by name, use as tie breaker for other sorts */
		ఒకవేళ (ENABLE_LOCALE_SUPPORT)
			dif = strcoll(d1->name, d2->name);
		లేకపోతే
			dif = strcmp(d1->name, d2->name);
	} లేకపోతే {
		/* Make dif fit into an int */
		ఒకవేళ (పరిమాణం(dif) > పరిమాణం(పూర్ణాంకం)) {
			గణన { బిట్లు_గమ్యం_జరుపు = 8 * (పరిమాణం(dif) - పరిమాణం(పూర్ణాంకం)) };
			/* shift leaving only "int" worth of bits */
			/* (this requires dif != 0, and here it is nonzero) */
			dif = 1 | (పూర్ణాంకం)((uoff_t)dif >> బిట్లు_గమ్యం_జరుపు);
		}
	}
 maybe_invert_and_ret:
	తిరిగి (opt & OPT_r) ? -(పూర్ణాంకం)dif : (పూర్ణాంకం)dif;
}

స్థిర శూన్య dnsort(నిర్మాణం dnode **dn, పూర్ణాంకం కొలత)
{
	qsort(dn, కొలత, పరిమాణం(*dn), sortcmp);
}

స్థిర శూన్య క్రమీకరించు_and_display_files(నిర్మాణం dnode **dn, గుర్తులేని nfiles)
{
	dnsort(dn, nfiles);
	display_files(dn, nfiles);
}
#లేకపోతే
# define dnsort(dn, కొలత) ((శూన్య)0)
# define క్రమీకరించు_and_display_files(dn, nfiles) display_files(dn, nfiles)
#అయితే_ముగింపు

/* Returns NULL-terminated malloced vector of pointers (or NULL) */
స్థిర నిర్మాణం dnode **పరిశీలన_one_dir(స్థిరరాశి అక్షరం *మార్గం, గుర్తులేని *nfiles_p)
{
	నిర్మాణం dnode *dn, *cur, **dnp;
	నిర్మాణం dirent *నమోదు_2;
	DIR *dir;
	గుర్తులేని i, nfiles;

	*nfiles_p = 0;
	dir = warn_opendir(మార్గం);
	ఒకవేళ (dir == ఖాళీ) {
		G.exit_సంకేతం = EXIT_FAILURE;
		తిరిగి ఖాళీ;	/* could not open the dir */
	}
	dn = ఖాళీ;
	nfiles = 0;
	అయితే ((నమోదు_2 = readdir(dir)) != ఖాళీ) {
		అక్షరం *fullname;

		/* are we going to list the file- it may be . or .. or a hidden file */
		ఒకవేళ (నమోదు_2->d_name[0] == '.') {
			ఒకవేళ (!(option_mask32 & (OPT_a|OPT_A)))
				కొనసాగు; /* skip all dotfiles if no -a/-A */
			ఒకవేళ (!(option_mask32 & OPT_a)
			 && (!నమోదు_2->d_name[1] || (నమోదు_2->d_name[1] == '.' && !నమోదు_2->d_name[2]))
			) {
				కొనసాగు; /* if only -A, skip . and .. but show other dotfiles */
			}
		}
		fullname = concat_మార్గం_file(మార్గం, నమోదు_2->d_name);
		cur = my_stat(fullname, bb_basename(fullname), 0);
		ఒకవేళ (!cur) {
			విడుదల(fullname);
			కొనసాగు;
		}
		cur->fname_allocated = 1;
		cur->dn_తదుపరి = dn;
		dn = cur;
		nfiles++;
	}
	closedir(dir);

	ఒకవేళ (dn == ఖాళీ)
		తిరిగి ఖాళీ;

	/* now that we know how many files there are
	 * allocate memory for an array to hold dnode pointers
	 */
	*nfiles_p = nfiles;
	dnp = dnalloc(nfiles);
	కొరకు (i = 0; /* i < nfiles - detected via !dn below */; i++) {
		dnp[i] = dn;	/* save pointer to node in array */
		dn = dn->dn_తదుపరి;
		ఒకవేళ (!dn)
			ఆపు;
	}

	తిరిగి dnp;
}

#ఒకవేళ ENABLE_DESKTOP
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
స్థిర off_t calculate_ఖండాలు(నిర్మాణం dnode **dn)
{
	uoff_t ఖండాలు = 1;
	ఒకవేళ (dn) {
		అయితే (*dn) {
			/* st_blocks is in 512 byte blocks */
			ఖండాలు += (*dn)->dn_ఖండాలు;
			dn++;
		}
	}

	/* Even though standard says use 512 byte blocks, coreutils use 1k */
	/* Actually, we round up by calculating (blocks + 1) / 2,
	 * "+ 1" was done when we initialized blocks to 1 */
	తిరిగి ఖండాలు >> 1;
}
#అయితే_ముగింపు

స్థిర శూన్య పరిశీలన_and_display_dirs_recur(నిర్మాణం dnode **dn, పూర్ణాంకం మొదటి)
{
	గుర్తులేని nfiles;
	నిర్మాణం dnode **subdnp;

	కొరకు (; *dn; dn++) {
		ఒకవేళ (G.show_dirname || (option_mask32 & OPT_R)) {
			ఒకవేళ (!మొదటి)
				bb_putchar('\n');
			మొదటి = 0;
			రాయి("%s:\n", (*dn)->fullname);
		}
		subdnp = పరిశీలన_one_dir((*dn)->fullname, &nfiles);
#ఒకవేళ ENABLE_DESKTOP
		ఒకవేళ (option_mask32 & (OPT_s|OPT_l)) {
			ఒకవేళ (option_mask32 & OPT_h) {
				రాయి("total %-"HUMAN_READABLE_MAX_వెడల్పు_STR"s\n",
					/* print size, no fractions, use suffixes */
					make_human_readable_str(calculate_ఖండాలు(subdnp) * 1024,
								0, 0)
				);
			} లేకపోతే {
				రాయి("total %"OFF_FMT"u\n", calculate_ఖండాలు(subdnp));
			}
		}
#అయితే_ముగింపు
		ఒకవేళ (nfiles > 0) {
			/* list all files at this level */
			క్రమీకరించు_and_display_files(subdnp, nfiles);

			ఒకవేళ (ENABLE_FEATURE_LS_RECURSIVE
			 && (option_mask32 & OPT_R)
			) {
				నిర్మాణం dnode **dnd;
				గుర్తులేని dndirs;
				/* recursive - list the sub-dirs */
				dnd = splitdnarray(subdnp, విభజించు_SUBDIR);
				dndirs = లెక్క_dirs(subdnp, విభజించు_SUBDIR);
				ఒకవేళ (dndirs > 0) {
					dnsort(dnd, dndirs);
					పరిశీలన_and_display_dirs_recur(dnd, 0);
					/* free the array of dnode pointers to the dirs */
					విడుదల(dnd);
				}
			}
			/* free the dnodes and the fullname mem */
			dfree(subdnp);
		}
	}
}


పూర్ణాంకం ls_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{	/*      ^^^^^^^^^^^^^^^^^ note: if FTPD, argc can be wrong, see ftpd.c */
	నిర్మాణం dnode **dnd;
	నిర్మాణం dnode **dnf;
	నిర్మాణం dnode **dnp;
	నిర్మాణం dnode *dn;
	నిర్మాణం dnode *cur;
	గుర్తులేని opt;
	గుర్తులేని nfiles;
	గుర్తులేని dnfiles;
	గుర్తులేని dndirs;
	గుర్తులేని i;
#ఒకవేళ ENABLE_FEATURE_LS_COLOR
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
	స్థిర స్థిరరాశి అక్షరం color_str[] ALIGN1 =
		"always\0""yes\0""force\0"
		"auto\0""tty\0""if-tty\0";
	/* need to initialize since --color has _an optional_ argument */
	స్థిరరాశి అక్షరం *color_opt = color_str; /* "always" */
#అయితే_ముగింపు
#ఒకవేళ ENABLE_LONG_OPTS
	స్థిర స్థిరరాశి అక్షరం ls_longopts[] ALIGN1 =
		"full-time\0" No_వాదం "\xff"
		"group-directories-first\0" No_వాదం "\xfe"
		IF_FEATURE_LS_COLOR("color\0" Optional_వాదం "\xfd")
	;
#అయితే_ముగింపు

	INIT_G();

	init_unicode();

#ఒకవేళ ENABLE_FEATURE_LS_వెడల్పు
	/* obtain the terminal width */
	G_terminal_వెడల్పు = get_terminal_వెడల్పు(STDIN_FILENO);
	/* go one less... */
	G_terminal_వెడల్పు--;
#అయితే_ముగింపు

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
			IF_FEATURE_LS_వెడల్పు(":w+")
		, ls_longopts
		IF_FEATURE_LS_వెడల్పు(, /*-T*/ ఖాళీ, /*-w*/ &G_terminal_వెడల్పు)
		IF_FEATURE_LS_COLOR(, &color_opt)
	);
#ఒకవేళ 0 /* option bits debug */
	bb_error_సందేశం("opt:0x%08x l:%x H:%x color:%x dirs:%x", opt, OPT_l, OPT_H, OPT_color, OPT_dirs_మొదటి);
	ఒకవేళ (opt & OPT_c         ) bb_error_సందేశం("-c");
	ఒకవేళ (opt & OPT_l         ) bb_error_సందేశం("-l");
	ఒకవేళ (opt & OPT_H         ) bb_error_సందేశం("-H");
	ఒకవేళ (opt & OPT_color     ) bb_error_సందేశం("--color");
	ఒకవేళ (opt & OPT_dirs_మొదటి) bb_error_సందేశం("--group-directories-first");
	ఒకవేళ (opt & OPT_నిండిన_సమయం ) bb_error_సందేశం("--full-time");
	నిష్క్రమించు(0);
#అయితే_ముగింపు

#ఒకవేళ ENABLE_SELINUX
	ఒకవేళ (opt & OPT_Z) {
		ఒకవేళ (!is_selinux_enabled())
			option_mask32 &= ~OPT_Z;
	}
#అయితే_ముగింపు

#ఒకవేళ ENABLE_FEATURE_LS_COLOR
	/* set G_show_color = 1/0 */
	ఒకవేళ (ENABLE_FEATURE_LS_COLOR_IS_DEFAULT && !is_పదం_dumb()) {
		అక్షరం *p = getenv("LS_COLORS");
		/* LS_COLORS is unset, or (not empty && not "none") ? */
		ఒకవేళ (!p || (p[0] && strcmp(p, "none") != 0)) {
			ఒకవేళ (isatty(STDOUT_FILENO)) {
				/* check isatty() last because it's expensive (syscall) */
				G_show_color = 1;
			}
		}
	}
	ఒకవేళ (opt & OPT_color) {
		ఒకవేళ (color_opt[0] == 'n')
			G_show_color = 0;
		లేకపోతే ఎంపిక (సూచీ_లోపల_substrings(color_str, color_opt)) {
		స్థితి 3:
		స్థితి 4:
		స్థితి 5:
			ఒకవేళ (!is_పదం_dumb() && isatty(STDOUT_FILENO)) {
		స్థితి 0:
		స్థితి 1:
		స్థితి 2:
				G_show_color = 1;
			}
		}
	}
#అయితే_ముగింపు

	/* sort out which command line options take precedence */
	ఒకవేళ (ENABLE_FEATURE_LS_RECURSIVE && (opt & OPT_d))
		option_mask32 &= ~OPT_R;	/* no recurse if listing only dir */
	ఒకవేళ (!(opt & OPT_l)) { /* not -l? */
		ఒకవేళ (ENABLE_FEATURE_LS_TIMESTAMPS && ENABLE_FEATURE_LS_SORTFILES) {
			/* when to sort by time? -t[cu] sorts by time even with -l */
			/* (this is achieved by opt_flags[] element for -t) */
			/* without -l, bare -c or -u enable sort too */
			/* (with -l, bare -c or -u just select which time to show) */
			ఒకవేళ (opt & (OPT_c|OPT_u)) {
				option_mask32 |= OPT_t;
			}
		}
	}

	/* choose a display format if one was not already specified by an option */
	ఒకవేళ (!(option_mask32 & (OPT_l|OPT_1|OPT_x|OPT_C)))
		option_mask32 |= (isatty(STDOUT_FILENO) ? OPT_C : OPT_1);

	ఒకవేళ (ENABLE_FTPD && applet_name[0] == 'f') {
		/* ftpd secret backdoor. dirs first are much nicer */
		option_mask32 |= OPT_dirs_మొదటి;
	}

	argv += optind;
	ఒకవేళ (!argv[0])
		*--argv = (అక్షరం*)".";

	ఒకవేళ (argv[1])
		G.show_dirname = 1; /* 2 or more items? label directories */

	/* stuff the command line file names into a dnode array */
	dn = ఖాళీ;
	nfiles = 0;
	చేయి {
		cur = my_stat(*argv, *argv,
			/* follow links on command line unless -l, -i, -s or -F: */
			!(option_mask32 & (OPT_l|OPT_i|OPT_s|OPT_F))
			/* ... or if -H: */
			|| (option_mask32 & OPT_H)
			/* ... or if -L, but my_stat always follows links if -L */
		);
		argv++;
		ఒకవేళ (!cur)
			కొనసాగు;
		/*cur->fname_allocated = 0; - already is */
		cur->dn_తదుపరి = dn;
		dn = cur;
		nfiles++;
	} అయితే (*argv);

	/* nfiles _may_ be 0 here - try "ls doesnt_exist" */
	ఒకవేళ (nfiles == 0)
		తిరిగి G.exit_సంకేతం;

	/* now that we know how many files there are
	 * allocate memory for an array to hold dnode pointers
	 */
	dnp = dnalloc(nfiles);
	కొరకు (i = 0; /* i < nfiles - detected via !dn below */; i++) {
		dnp[i] = dn;	/* save pointer to node in array */
		dn = dn->dn_తదుపరి;
		ఒకవేళ (!dn)
			ఆపు;
	}

	ఒకవేళ (option_mask32 & OPT_d) {
		క్రమీకరించు_and_display_files(dnp, nfiles);
	} లేకపోతే {
		dnd = splitdnarray(dnp, విభజించు_DIR);
		dnf = splitdnarray(dnp, విభజించు_FILE);
		dndirs = లెక్క_dirs(dnp, విభజించు_DIR);
		dnfiles = nfiles - dndirs;
		ఒకవేళ (dnfiles > 0) {
			క్రమీకరించు_and_display_files(dnf, dnfiles);
			ఒకవేళ (ENABLE_FEATURE_CLEAN_పైకి)
				విడుదల(dnf);
		}
		ఒకవేళ (dndirs > 0) {
			dnsort(dnd, dndirs);
			పరిశీలన_and_display_dirs_recur(dnd, dnfiles == 0);
			ఒకవేళ (ENABLE_FEATURE_CLEAN_పైకి)
				విడుదల(dnd);
		}
	}

	ఒకవేళ (ENABLE_FEATURE_CLEAN_పైకి)
		dfree(dnp);
	తిరిగి G.exit_సంకేతం;
}
