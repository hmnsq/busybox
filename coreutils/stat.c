/* vi: set sw=4 ts=4: */
/*
 * stat -- display file or file system status
 *
 * Copyright (C) 2001, 2002, 2003, 2004, 2005 Free Software Foundation.
 * Copyright (C) 2005 by Erik Andersen <andersen@codepoet.org>
 * Copyright (C) 2005 by Mike Frysinger <vapier@gentoo.org>
 * Copyright (C) 2006 by Yoshinori Sato <ysato@users.sourceforge.jp>
 *
 * Written by Michael Meskes
 * Taken from coreutils and turned into a busybox applet by Mike Frysinger
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config STAT
//config:	bool "stat (11 kb)"
//config:	default y
//config:	help
//config:	display file or filesystem status.
//config:
//config:config FEATURE_STAT_FORMAT
//config:	bool "Enable custom formats (-c)"
//config:	default y
//config:	depends on STAT
//config:	help
//config:	Without this, stat will not support the '-c format' option where
//config:	users can pass a custom format string for output. This adds about
//config:	7k to a nonstatic build on amd64.
//config:
//config:config FEATURE_STAT_FILESYSTEM
//config:	bool "Enable display of filesystem status (-f)"
//config:	default y
//config:	depends on STAT
//config:	help
//config:	Without this, stat will not support the '-f' option to display
//config:	information about filesystem status.

//applet:IF_STAT(APPLET_NOEXEC(stat, stat, BB_DIR_BIN, BB_SUID_DROP, stat))

//kbuild:lib-$(CONFIG_STAT) += stat.o

//usage:#define stat_trivial_usage
//usage:       "[-lt"IF_FEATURE_STAT_FILESYSTEM("f")"] "IF_FEATURE_STAT_FORMAT("[-c FMT] ")"FILE..."
//usage:#define stat_full_usage "\n\n"
//usage:       "Display file"
//usage:            IF_FEATURE_STAT_FILESYSTEM(" (default) or filesystem")
//usage:            " status\n"
//usage:	IF_FEATURE_STAT_FORMAT(
//usage:     "\n	-c FMT	Use the specified format"
//usage:	)
//usage:	IF_FEATURE_STAT_FILESYSTEM(
//usage:     "\n	-f	Display filesystem status"
//usage:	)
//usage:     "\n	-L	Follow links"
//usage:     "\n	-t	Terse display"
//usage:	IF_SELINUX(
//usage:     "\n	-Z	Print security context"
//usage:	)
//usage:	IF_FEATURE_STAT_FORMAT(
//usage:       "\n\nFMT sequences"IF_FEATURE_STAT_FILESYSTEM(" for files")":\n"
//usage:       " %a	Access rights in octal\n"
//usage:       " %A	Access rights in human readable form\n"
//usage:       " %b	Number of blocks allocated (see %B)\n"
//usage:       " %B	Size in bytes of each block reported by %b\n"
//usage:       " %d	Device number in decimal\n"
//usage:       " %D	Device number in hex\n"
//usage:       " %f	Raw mode in hex\n"
//usage:       " %F	File type\n"
//usage:       " %g	Group ID\n"
//usage:       " %G	Group name\n"
//usage:       " %h	Number of hard links\n"
//usage:       " %i	Inode number\n"
//usage:       " %n	File name\n"
//usage:       " %N	File name, with -> TARGET if symlink\n"
//usage:       " %o	I/O block size\n"
//usage:       " %s	Total size in bytes\n"
//usage:       " %t	Major device type in hex\n"
//usage:       " %T	Minor device type in hex\n"
//usage:       " %u	User ID\n"
//usage:       " %U	User name\n"
//usage:       " %x	Time of last access\n"
//usage:       " %X	Time of last access as seconds since Epoch\n"
//usage:       " %y	Time of last modification\n"
//usage:       " %Y	Time of last modification as seconds since Epoch\n"
//usage:       " %z	Time of last change\n"
//usage:       " %Z	Time of last change as seconds since Epoch\n"
//usage:	IF_FEATURE_STAT_FILESYSTEM(
//usage:       "\nFMT sequences for file systems:\n"
//usage:       " %a	Free blocks available to non-superuser\n"
//usage:       " %b	Total data blocks\n"
//usage:       " %c	Total file nodes\n"
//usage:       " %d	Free file nodes\n"
//usage:       " %f	Free blocks\n"
//usage:	IF_SELINUX(
//usage:       " %C	Security context in selinux\n"
//usage:	)
//usage:       " %i	File System ID in hex\n"
//usage:       " %l	Maximum length of filenames\n"
//usage:       " %n	File name\n"
//usage:       " %s	Block size (for faster transfer)\n"
//usage:       " %S	Fundamental block size (for block counts)\n"
//usage:       " %t	Type in hex\n"
//usage:       " %T	Type in human readable form"
//usage:	)
//usage:	)

#ಅನ್ತರ್ಭಾವ "libbb.h"
#ಅನ್ತರ್ಭಾವ "common_bufsiz.h"

ಗಣನಾ {
	OPT_TERSE       = (1 << 0),
	OPT_DEREFERENCE = (1 << 1),
	OPT_FILESYS     = (1 << 2) * ENABLE_FEATURE_STAT_FILESYSTEM,
	OPT_SELINUX     = (1 << (2+ENABLE_FEATURE_STAT_FILESYSTEM)) * ENABLE_SELINUX,
};

#ಯದಿ ENABLE_FEATURE_STAT_FORMAT
ಪ್ರಕಾರನಿರ್ದೇಶ bool (*statfunc_ptr)(ಧ್ರುವ ವರ್ಣ *, ಧ್ರುವ ವರ್ಣ *);
#ಅನ್ಯಥಾ
ಪ್ರಕಾರನಿರ್ದೇಶ bool (*statfunc_ptr)(ಧ್ರುವ ವರ್ಣ *);
#endif

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *file_type(ಧ್ರುವ ಸಂರಚನಾ stat *st)
{
	/* See POSIX 1003.1-2001 XCU Table 4-8 lines 17093-17107
	 * for some of these formats.
	 * To keep diagnostics grammatical in English, the
	 * returned string must start with a consonant.
	 */
	ಯದಿ (S_ISREG(st->st_mode))  ಪ್ರತ್ಯಾಗಮ st->st_size == 0 ? "regular empty file" : "regular file";
	ಯದಿ (S_ISDIR(st->st_mode))  ಪ್ರತ್ಯಾಗಮ "directory";
	ಯದಿ (S_ISBLK(st->st_mode))  ಪ್ರತ್ಯಾಗಮ "block special file";
	ಯದಿ (S_ISCHR(st->st_mode))  ಪ್ರತ್ಯಾಗಮ "character special file";
	ಯದಿ (S_ISFIFO(st->st_mode)) ಪ್ರತ್ಯಾಗಮ "fifo";
	ಯದಿ (S_ISLNK(st->st_mode))  ಪ್ರತ್ಯಾಗಮ "symbolic link";
	ಯದಿ (S_ISSOCK(st->st_mode)) ಪ್ರತ್ಯಾಗಮ "socket";
#ifdef S_TYPEISMQ
	ಯದಿ (S_TYPEISMQ(st))        ಪ್ರತ್ಯಾಗಮ "message queue";
#endif
#ifdef S_TYPEISSEM
	ಯದಿ (S_TYPEISSEM(st))       ಪ್ರತ್ಯಾಗಮ "semaphore";
#endif
#ifdef S_TYPEISSHM
	ಯದಿ (S_TYPEISSHM(st))       ಪ್ರತ್ಯಾಗಮ "shared memory object";
#endif
#ifdef S_TYPEISTMO
	ಯದಿ (S_TYPEISTMO(st))       ಪ್ರತ್ಯಾಗಮ "typed memory object";
#endif
	ಪ್ರತ್ಯಾಗಮ "weird file";
}

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *human_time(ಸಂರಚನಾ timespec *ts)
{
	ವರ್ಣ fmt[ಪರಿಮಾಣ("%Y-%m-%d %H:%M:%S.123456789 %z") + /*paranoia*/ 8];

	/* coreutils 6.3 compat */
#define buf bb_common_bufsiz1
	setup_common_bufsiz();

	sprintf(stpcpy(fmt, "%Y-%m-%d %H:%M:%S"), ".%09u %%z", (unsigned)ts->tv_nsec);
	strftime(buf, COMMON_BUFSIZE, fmt, localtime(&ts->tv_sec));
	ಪ್ರತ್ಯಾಗಮ buf;
#undef buf
}

#ಯದಿ ENABLE_FEATURE_STAT_FILESYSTEM
#define FS_TYPE_LIST \
FS_TYPE(0xADFF,     "affs") \
FS_TYPE(0x1CD1,     "devpts") \
FS_TYPE(0x137D,     "ext") \
FS_TYPE(0xEF51,     "ext2") \
FS_TYPE(0xEF53,     "ext2/ext3") \
FS_TYPE(0x3153464a, "jfs") \
FS_TYPE(0x58465342, "xfs") \
FS_TYPE(0xF995E849, "hpfs") \
FS_TYPE(0x9660,     "isofs") \
FS_TYPE(0x4000,     "isofs") \
FS_TYPE(0x4004,     "isofs") \
FS_TYPE(0x137F,     "minix") \
FS_TYPE(0x138F,     "minix (30 char.)") \
FS_TYPE(0x2468,     "minix v2") \
FS_TYPE(0x2478,     "minix v2 (30 char.)") \
FS_TYPE(0x4d44,     "msdos") \
FS_TYPE(0x4006,     "fat") \
FS_TYPE(0x564c,     "novell") \
FS_TYPE(0x6969,     "nfs") \
FS_TYPE(0x9fa0,     "proc") \
FS_TYPE(0x517B,     "smb") \
FS_TYPE(0x012FF7B4, "xenix") \
FS_TYPE(0x012FF7B5, "sysv4") \
FS_TYPE(0x012FF7B6, "sysv2") \
FS_TYPE(0x012FF7B7, "coh") \
FS_TYPE(0x00011954, "ufs") \
FS_TYPE(0x012FD16D, "xia") \
FS_TYPE(0x5346544e, "ntfs") \
FS_TYPE(0x1021994,  "tmpfs") \
FS_TYPE(0x52654973, "reiserfs") \
FS_TYPE(0x28cd3d45, "cramfs") \
FS_TYPE(0x7275,     "romfs") \
FS_TYPE(0x858458f6, "ramfs") \
FS_TYPE(0x73717368, "squashfs") \
FS_TYPE(0x62656572, "sysfs")
/* Return the type of the specified file system.
 * Some systems have statfvs.f_basetype[FSTYPSZ]. (AIX, HP-UX, and Solaris)
 * Others have statfs.f_fstypename[MFSNAMELEN]. (NetBSD 1.5.2)
 * Still others have neither and have to get by with f_type (Linux).
 */
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *human_fstype(uint32_t f_type)
{
# define FS_TYPE(type, name) type,
	ಸ್ಥಿರ ಧ್ರುವ uint32_t fstype[] ALIGN4 = {
		FS_TYPE_LIST
	};
# undef FS_TYPE
# define FS_TYPE(type, name) name"\0"
	ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ humanname[] ALIGN1 =
		FS_TYPE_LIST
		"UNKNOWN";
# undef FS_TYPE
	ಪೂರ್ಣಾಙ್ಕ i;

	ಪ್ರತಿ (i = 0; i < ARRAY_SIZE(fstype); ++i)
		ಯದಿ (fstype[i] == f_type)
			ಭಙ್ಗ;
	ಪ್ರತ್ಯಾಗಮ nth_string(humanname, i);
}

/* "man statfs" says that statfsbuf->f_fsid is a mess */
/* coreutils treats it as an array of ints, most significant first */
ಸ್ಥಿರ unsigned long long get_f_fsid(ಧ್ರುವ ಸಂರಚನಾ statfs *statfsbuf)
{
	ಧ್ರುವ unsigned *p = (ಧ್ರುವ ಶೂನ್ಯ*) &statfsbuf->f_fsid;
	unsigned sz = ಪರಿಮಾಣ(statfsbuf->f_fsid) / ಪರಿಮಾಣ(unsigned);
	unsigned long long r = 0;

	ಕುರು
		r = (r << (ಪರಿಮಾಣ(unsigned)*8)) | *p++;
	ಯಾವತ್ (--sz > 0);
	ಪ್ರತ್ಯಾಗಮ r;
}
#endif  /* FEATURE_STAT_FILESYSTEM */

#ಯದಿ ENABLE_FEATURE_STAT_FORMAT
ಸ್ಥಿರ ಶೂನ್ಯ strcatc(ವರ್ಣ *str, ವರ್ಣ c)
{
	ಪೂರ್ಣಾಙ್ಕ len = strlen(str);
	str[len++] = c;
	str[len] = '\0';
}

ಸ್ಥಿರ ಶೂನ್ಯ printfs(ವರ್ಣ *pformat, ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ)
{
	strcatc(pformat, 's');
	printf(pformat, ಸಂದೇಶ);
}

#ಯದಿ ENABLE_FEATURE_STAT_FILESYSTEM
/* print statfs info */
ಸ್ಥಿರ ಶೂನ್ಯ FAST_FUNC print_statfs(ವರ್ಣ *pformat, ಧ್ರುವ ವರ್ಣ m,
		ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ filename, ಧ್ರುವ ಶೂನ್ಯ *data
		IF_SELINUX(, security_context_t scontext))
{
	ಧ್ರುವ ಸಂರಚನಾ statfs *statfsbuf = data;
	ಯದಿ (m == 'n') {
		printfs(pformat, filename);
	} ಅನ್ಯಥಾ ಯದಿ (m == 'i') {
		strcat(pformat, "llx");
		printf(pformat, get_f_fsid(statfsbuf));
	} ಅನ್ಯಥಾ ಯದಿ (m == 'l') {
		strcat(pformat, "lu");
		printf(pformat, (unsigned long) statfsbuf->f_namelen);
	} ಅನ್ಯಥಾ ಯದಿ (m == 't') {
		strcat(pformat, "lx");
		printf(pformat, (unsigned long) statfsbuf->f_type); /* no equiv */
	} ಅನ್ಯಥಾ ಯದಿ (m == 'T') {
		printfs(pformat, human_fstype(statfsbuf->f_type));
	} ಅನ್ಯಥಾ ಯದಿ (m == 'b') {
		strcat(pformat, "llu");
		printf(pformat, (unsigned long long) statfsbuf->f_blocks);
	} ಅನ್ಯಥಾ ಯದಿ (m == 'f') {
		strcat(pformat, "llu");
		printf(pformat, (unsigned long long) statfsbuf->f_bfree);
	} ಅನ್ಯಥಾ ಯದಿ (m == 'a') {
		strcat(pformat, "llu");
		printf(pformat, (unsigned long long) statfsbuf->f_bavail);
	} ಅನ್ಯಥಾ ಯದಿ (m == 's' || m == 'S') {
		strcat(pformat, "lu");
		printf(pformat, (unsigned long) statfsbuf->f_bsize);
	} ಅನ್ಯಥಾ ಯದಿ (m == 'c') {
		strcat(pformat, "llu");
		printf(pformat, (unsigned long long) statfsbuf->f_files);
	} ಅನ್ಯಥಾ ಯದಿ (m == 'd') {
		strcat(pformat, "llu");
		printf(pformat, (unsigned long long) statfsbuf->f_ffree);
# ಯದಿ ENABLE_SELINUX
	} ಅನ್ಯಥಾ ಯದಿ (m == 'C' && (option_mask32 & OPT_SELINUX)) {
		printfs(pformat, scontext);
# endif
	} ಅನ್ಯಥಾ {
		strcatc(pformat, 'c');
		printf(pformat, m);
	}
}
#endif

/* print stat info */
ಸ್ಥಿರ ಶೂನ್ಯ FAST_FUNC print_stat(ವರ್ಣ *pformat, ಧ್ರುವ ವರ್ಣ m,
		ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ filename, ಧ್ರುವ ಶೂನ್ಯ *data
		IF_SELINUX(, security_context_t scontext))
{
#define TYPE_SIGNED(t) (! ((t) 0 < (t) -1))
	ಸಂರಚನಾ stat *statbuf = (ಸಂರಚನಾ stat *) data;
	ಸಂರಚನಾ passwd *pw_ent;
	ಸಂರಚನಾ group *gw_ent;

	ಯದಿ (m == 'n') {
		printfs(pformat, filename);
	} ಅನ್ಯಥಾ ಯದಿ (m == 'N') {
		strcatc(pformat, 's');
		ಯದಿ (S_ISLNK(statbuf->st_mode)) {
			ವರ್ಣ *linkname = xmalloc_readlink_or_warn(filename);
			ಯದಿ (linkname == ರಿಕ್ತ)
				ಪ್ರತ್ಯಾಗಮ;
			printf("'%s' -> '%s'", filename, linkname);
			ಮುಕ್ತ(linkname);
		} ಅನ್ಯಥಾ {
			printf(pformat, filename);
		}
	} ಅನ್ಯಥಾ ಯದಿ (m == 'd') {
		strcat(pformat, "llu");
		printf(pformat, (unsigned long long) statbuf->st_dev);
	} ಅನ್ಯಥಾ ಯದಿ (m == 'D') {
		strcat(pformat, "llx");
		printf(pformat, (unsigned long long) statbuf->st_dev);
	} ಅನ್ಯಥಾ ಯದಿ (m == 'i') {
		strcat(pformat, "llu");
		printf(pformat, (unsigned long long) statbuf->st_ino);
	} ಅನ್ಯಥಾ ಯದಿ (m == 'a') {
		strcat(pformat, "lo");
		printf(pformat, (unsigned long) (statbuf->st_mode & (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)));
	} ಅನ್ಯಥಾ ಯದಿ (m == 'A') {
		ವರ್ಣ modestr[12];
		printfs(pformat, bb_mode_string(modestr, statbuf->st_mode));
	} ಅನ್ಯಥಾ ಯದಿ (m == 'f') {
		strcat(pformat, "lx");
		printf(pformat, (unsigned long) statbuf->st_mode);
	} ಅನ್ಯಥಾ ಯದಿ (m == 'F') {
		printfs(pformat, file_type(statbuf));
	} ಅನ್ಯಥಾ ಯದಿ (m == 'h') {
		strcat(pformat, "lu");
		printf(pformat, (unsigned long) statbuf->st_nlink);
	} ಅನ್ಯಥಾ ಯದಿ (m == 'u') {
		strcat(pformat, "lu");
		printf(pformat, (unsigned long) statbuf->st_uid);
	} ಅನ್ಯಥಾ ಯದಿ (m == 'U') {
		pw_ent = getpwuid(statbuf->st_uid);
		printfs(pformat, (pw_ent != ರಿಕ್ತ) ? pw_ent->pw_name : "UNKNOWN");
	} ಅನ್ಯಥಾ ಯದಿ (m == 'g') {
		strcat(pformat, "lu");
		printf(pformat, (unsigned long) statbuf->st_gid);
	} ಅನ್ಯಥಾ ಯದಿ (m == 'G') {
		gw_ent = getgrgid(statbuf->st_gid);
		printfs(pformat, (gw_ent != ರಿಕ್ತ) ? gw_ent->gr_name : "UNKNOWN");
	} ಅನ್ಯಥಾ ಯದಿ (m == 't') {
		strcat(pformat, "lx");
		printf(pformat, (unsigned long) major(statbuf->st_rdev));
	} ಅನ್ಯಥಾ ಯದಿ (m == 'T') {
		strcat(pformat, "lx");
		printf(pformat, (unsigned long) minor(statbuf->st_rdev));
	} ಅನ್ಯಥಾ ಯದಿ (m == 's') {
		strcat(pformat, "llu");
		printf(pformat, (unsigned long long) statbuf->st_size);
	} ಅನ್ಯಥಾ ಯದಿ (m == 'B') {
		strcat(pformat, "lu");
		printf(pformat, (unsigned long) 512); //ST_NBLOCKSIZE
	} ಅನ್ಯಥಾ ಯದಿ (m == 'b') {
		strcat(pformat, "llu");
		printf(pformat, (unsigned long long) statbuf->st_blocks);
	} ಅನ್ಯಥಾ ಯದಿ (m == 'o') {
		strcat(pformat, "lu");
		printf(pformat, (unsigned long) statbuf->st_blksize);
	} ಅನ್ಯಥಾ ಯದಿ (m == 'x') {
		printfs(pformat, human_time(&statbuf->st_atim));
	} ಅನ್ಯಥಾ ಯದಿ (m == 'X') {
		strcat(pformat, TYPE_SIGNED(time_t) ? "ld" : "lu");
		/* note: (unsigned long) would be wrong:
		 * imagine (unsigned long64)int32 */
		printf(pformat, (long) statbuf->st_atime);
	} ಅನ್ಯಥಾ ಯದಿ (m == 'y') {
		printfs(pformat, human_time(&statbuf->st_mtim));
	} ಅನ್ಯಥಾ ಯದಿ (m == 'Y') {
		strcat(pformat, TYPE_SIGNED(time_t) ? "ld" : "lu");
		printf(pformat, (long) statbuf->st_mtime);
	} ಅನ್ಯಥಾ ಯದಿ (m == 'z') {
		printfs(pformat, human_time(&statbuf->st_ctim));
	} ಅನ್ಯಥಾ ಯದಿ (m == 'Z') {
		strcat(pformat, TYPE_SIGNED(time_t) ? "ld" : "lu");
		printf(pformat, (long) statbuf->st_ctime);
# ಯದಿ ENABLE_SELINUX
	} ಅನ್ಯಥಾ ಯದಿ (m == 'C' && (option_mask32 & OPT_SELINUX)) {
		printfs(pformat, scontext);
# endif
	} ಅನ್ಯಥಾ {
		strcatc(pformat, 'c');
		printf(pformat, m);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ print_it(ಧ್ರುವ ವರ್ಣ *masterformat,
		ಧ್ರುವ ವರ್ಣ *filename,
		ಶೂನ್ಯ FAST_FUNC (*print_func)(ವರ್ಣ*, ವರ್ಣ, ಧ್ರುವ ವರ್ಣ*, ಧ್ರುವ ಶೂನ್ಯ* IF_SELINUX(, security_context_t scontext)),
		ಧ್ರುವ ಶೂನ್ಯ *data
		IF_SELINUX(, security_context_t scontext))
{
	/* Create a working copy of the format string */
	ವರ್ಣ *format = xstrdup(masterformat);
	/* Add 2 to accommodate our conversion of the stat '%s' format string
	 * to the printf '%llu' one.  */
	ವರ್ಣ *dest = xmalloc(strlen(format) + 2 + 1);
	ವರ್ಣ *b;

	b = format;
	ಯಾವತ್ (b) {
		/* Each iteration finds next %spec,
		 * prints preceding string and handles found %spec
		 */
		size_t len;
		ವರ್ಣ *p = strchr(b, '%');
		ಯದಿ (!p) {
			/* coreutils 6.3 always prints newline at the end */
			/*fputs(b, stdout);*/
			puts(b);
			ಭಙ್ಗ;
		}

		/* dest = "%<modifiers>" */
		len = 1 + strspn(p + 1, "#-+.I 0123456789");
		memcpy(dest, p, len);
		dest[len] = '\0';

		/* print preceding string */
		*p = '\0';
		fputs_stdout(b);

		p += len;
		b = p + 1;
		ವಿಕಲ್ಪ (*p) {
		ಪಕ್ಷ '\0':
			b = ರಿಕ್ತ;
			/* fall through */
		ಪಕ್ಷ '%':
			bb_putchar('%');
			ಭಙ್ಗ;
		ಸ್ವಭಾವ:
			/* Completes "%<modifiers>" with specifier and printfs */
			print_func(dest, *p, filename, data IF_SELINUX(,scontext));
			ಭಙ್ಗ;
		}
	}

	ಮುಕ್ತ(format);
	ಮುಕ್ತ(dest);
}
#endif  /* FEATURE_STAT_FORMAT */

#ಯದಿ ENABLE_FEATURE_STAT_FILESYSTEM
/* Stat the file system and print what we find.  */
#ಯದಿ !ENABLE_FEATURE_STAT_FORMAT
#define do_statfs(filename, format) do_statfs(filename)
#endif
ಸ್ಥಿರ bool do_statfs(ಧ್ರುವ ವರ್ಣ *filename, ಧ್ರುವ ವರ್ಣ *format)
{
	ಸಂರಚನಾ statfs statfsbuf;
#ಯದಿ !ENABLE_FEATURE_STAT_FORMAT
	ಧ್ರುವ ವರ್ಣ *format;
#endif
#ಯದಿ ENABLE_SELINUX
	security_context_t scontext = ರಿಕ್ತ;

	ಯದಿ (option_mask32 & OPT_SELINUX) {
		ಯದಿ ((option_mask32 & OPT_DEREFERENCE
		     ? lgetfilecon(filename, &scontext)
		     : getfilecon(filename, &scontext)
		    ) < 0
		) {
			bb_simple_perror_ಸಂದೇಶ(filename);
			ಪ್ರತ್ಯಾಗಮ 0;
		}
	}
#endif
	ಯದಿ (statfs(filename, &statfsbuf) != 0) {
		bb_perror_ಸಂದೇಶ("can't read file system information for '%s'", filename);
		ಪ್ರತ್ಯಾಗಮ 0;
	}

#ಯದಿ ENABLE_FEATURE_STAT_FORMAT
	ಯದಿ (format == ರಿಕ್ತ) {
# ಯದಿ !ENABLE_SELINUX
		format = (option_mask32 & OPT_TERSE
			? "%n %i %l %t %s %b %f %a %c %d"
			: "  File: \"%n\"\n"
			  "    ID: %-8i Namelen: %-7l Type: %T\n"
			  "Block size: %-10s\n"
			  "Blocks: Total: %-10b Free: %-10f Available: %a\n"
			  "Inodes: Total: %-10c Free: %d");
# ಅನ್ಯಥಾ
		format = (option_mask32 & OPT_TERSE
			? (option_mask32 & OPT_SELINUX
				? "%n %i %l %t %s %b %f %a %c %d %C"
				: "%n %i %l %t %s %b %f %a %c %d")
			: (option_mask32 & OPT_SELINUX
				? "  File: \"%n\"\n"
				"    ID: %-8i Namelen: %-7l Type: %T\n"
				"Block size: %-10s\n"
				"Blocks: Total: %-10b Free: %-10f Available: %a\n"
				"Inodes: Total: %-10c Free: %d"
				"  S_context: %C"
				: "  File: \"%n\"\n"
				"    ID: %-8i Namelen: %-7l Type: %T\n"
				"Block size: %-10s\n"
				"Blocks: Total: %-10b Free: %-10f Available: %a\n"
				"Inodes: Total: %-10c Free: %d")
			);
# endif /* SELINUX */
	}
	print_it(format, filename, print_statfs, &statfsbuf IF_SELINUX(, scontext));
#ಅನ್ಯಥಾ /* !FEATURE_STAT_FORMAT */
	format = (option_mask32 & OPT_TERSE
		? "%s %llx %lu "
		: "  File: \"%s\"\n"
		  "    ID: %-8llx Namelen: %-7lu ");
	printf(format,
	       filename,
	       get_f_fsid(&statfsbuf),
	       statfsbuf.f_namelen);

	ಯದಿ (option_mask32 & OPT_TERSE)
		printf("%lx ", (unsigned long) statfsbuf.f_type);
	ಅನ್ಯಥಾ
		printf("Type: %s\n", human_fstype(statfsbuf.f_type));

# ಯದಿ !ENABLE_SELINUX
	format = (option_mask32 & OPT_TERSE
		? "%lu %llu %llu %llu %llu %llu\n"
		: "Block size: %-10lu\n"
		  "Blocks: Total: %-10llu Free: %-10llu Available: %llu\n"
		  "Inodes: Total: %-10llu Free: %llu\n");
	printf(format,
	       (unsigned long) statfsbuf.f_bsize,
	       (unsigned long long) statfsbuf.f_blocks,
	       (unsigned long long) statfsbuf.f_bfree,
	       (unsigned long long) statfsbuf.f_bavail,
	       (unsigned long long) statfsbuf.f_files,
	       (unsigned long long) statfsbuf.f_ffree);
# ಅನ್ಯಥಾ
	format = (option_mask32 & OPT_TERSE
		? (option_mask32 & OPT_SELINUX ? "%lu %llu %llu %llu %llu %llu %C\n" : "%lu %llu %llu %llu %llu %llu\n")
		: (option_mask32 & OPT_SELINUX
			?	"Block size: %-10lu\n"
				"Blocks: Total: %-10llu Free: %-10llu Available: %llu\n"
				"Inodes: Total: %-10llu Free: %llu"
				"S_context: %C\n"
			:	"Block size: %-10lu\n"
				"Blocks: Total: %-10llu Free: %-10llu Available: %llu\n"
				"Inodes: Total: %-10llu Free: %llu\n"
			)
		);
	printf(format,
		(unsigned long) statfsbuf.f_bsize,
		(unsigned long long) statfsbuf.f_blocks,
		(unsigned long long) statfsbuf.f_bfree,
		(unsigned long long) statfsbuf.f_bavail,
		(unsigned long long) statfsbuf.f_files,
		(unsigned long long) statfsbuf.f_ffree,
		scontext);

	ಯದಿ (scontext)
		freecon(scontext);
# endif
#endif  /* FEATURE_STAT_FORMAT */
	ಪ್ರತ್ಯಾಗಮ 1;
}
#endif  /* FEATURE_STAT_FILESYSTEM */

/* stat the file and print what we find */
#ಯದಿ !ENABLE_FEATURE_STAT_FORMAT
#define do_stat(filename, format) do_stat(filename)
#endif
ಸ್ಥಿರ bool do_stat(ಧ್ರುವ ವರ್ಣ *filename, ಧ್ರುವ ವರ್ಣ *format)
{
	ಸಂರಚನಾ stat statbuf;
#ಯದಿ ENABLE_SELINUX
	security_context_t scontext = ರಿಕ್ತ;

	ಯದಿ (option_mask32 & OPT_SELINUX) {
		ಯದಿ ((option_mask32 & OPT_DEREFERENCE
		     ? lgetfilecon(filename, &scontext)
		     : getfilecon(filename, &scontext)
		    ) < 0
		) {
			bb_simple_perror_ಸಂದೇಶ(filename);
			ಪ್ರತ್ಯಾಗಮ 0;
		}
	}
#endif
	ಯದಿ ((option_mask32 & OPT_DEREFERENCE ? stat : lstat) (filename, &statbuf) != 0) {
		bb_perror_ಸಂದೇಶ("can't stat '%s'", filename);
		ಪ್ರತ್ಯಾಗಮ 0;
	}

#ಯದಿ ENABLE_FEATURE_STAT_FORMAT
	ಯದಿ (format == ರಿಕ್ತ) {
# ಯದಿ !ENABLE_SELINUX
		ಯದಿ (option_mask32 & OPT_TERSE) {
			format = "%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %o";
		} ಅನ್ಯಥಾ {
			ಯದಿ (S_ISBLK(statbuf.st_mode) || S_ISCHR(statbuf.st_mode)) {
				format =
					"  File: %N\n"
					"  Size: %-10s\tBlocks: %-10b IO Block: %-6o %F\n"
					"Device: %Dh/%dd\tInode: %-10i  Links: %-5h"
					" Device type: %t,%T\n"
					"Access: (%04a/%10.10A)  Uid: (%5u/%8U)   Gid: (%5g/%8G)\n"
					"Access: %x\n" "Modify: %y\n" "Change: %z";
			} ಅನ್ಯಥಾ {
				format =
					"  File: %N\n"
					"  Size: %-10s\tBlocks: %-10b IO Block: %-6o %F\n"
					"Device: %Dh/%dd\tInode: %-10i  Links: %h\n"
					"Access: (%04a/%10.10A)  Uid: (%5u/%8U)   Gid: (%5g/%8G)\n"
					"Access: %x\n" "Modify: %y\n" "Change: %z";
			}
		}
# ಅನ್ಯಥಾ
		ಯದಿ (option_mask32 & OPT_TERSE) {
			format = (option_mask32 & OPT_SELINUX ?
				"%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %o %C\n"
				:
				"%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %o\n"
				);
		} ಅನ್ಯಥಾ {
			ಯದಿ (S_ISBLK(statbuf.st_mode) || S_ISCHR(statbuf.st_mode)) {
				format = (option_mask32 & OPT_SELINUX ?
					"  File: %N\n"
					"  Size: %-10s\tBlocks: %-10b IO Block: %-6o %F\n"
					"Device: %Dh/%dd\tInode: %-10i  Links: %-5h"
					" Device type: %t,%T\n"
					"Access: (%04a/%10.10A)  Uid: (%5u/%8U)   Gid: (%5g/%8G)\n"
					"   S_Context: %C\n"
					"Access: %x\n" "Modify: %y\n" "Change: %z"
					:
					"  File: %N\n"
					"  Size: %-10s\tBlocks: %-10b IO Block: %-6o %F\n"
					"Device: %Dh/%dd\tInode: %-10i  Links: %-5h"
					" Device type: %t,%T\n"
					"Access: (%04a/%10.10A)  Uid: (%5u/%8U)   Gid: (%5g/%8G)\n"
					"Access: %x\n" "Modify: %y\n" "Change: %z"
					);
			} ಅನ್ಯಥಾ {
				format = (option_mask32 & OPT_SELINUX ?
					"  File: %N\n"
					"  Size: %-10s\tBlocks: %-10b IO Block: %-6o %F\n"
					"Device: %Dh/%dd\tInode: %-10i  Links: %h\n"
					"Access: (%04a/%10.10A)  Uid: (%5u/%8U)   Gid: (%5g/%8G)\n"
					"S_Context: %C\n"
					"Access: %x\n" "Modify: %y\n" "Change: %z"
					:
					"  File: %N\n"
					"  Size: %-10s\tBlocks: %-10b IO Block: %-6o %F\n"
					"Device: %Dh/%dd\tInode: %-10i  Links: %h\n"
					"Access: (%04a/%10.10A)  Uid: (%5u/%8U)   Gid: (%5g/%8G)\n"
					"Access: %x\n" "Modify: %y\n" "Change: %z"
					);
			}
		}
# endif
	}
	print_it(format, filename, print_stat, &statbuf IF_SELINUX(, scontext));
#ಅನ್ಯಥಾ	/* FEATURE_STAT_FORMAT */
	ಯದಿ (option_mask32 & OPT_TERSE) {
		printf("%s %llu %llu %lx %lu %lu %llx %llu %lu %lx %lx %lu %lu %lu %lu"
		       IF_NOT_SELINUX("\n"),
		       filename,
		       (unsigned long long) statbuf.st_size,
		       (unsigned long long) statbuf.st_blocks,
		       (unsigned long) statbuf.st_mode,
		       (unsigned long) statbuf.st_uid,
		       (unsigned long) statbuf.st_gid,
		       (unsigned long long) statbuf.st_dev,
		       (unsigned long long) statbuf.st_ino,
		       (unsigned long) statbuf.st_nlink,
		       (unsigned long) major(statbuf.st_rdev),
		       (unsigned long) minor(statbuf.st_rdev),
		       (unsigned long) statbuf.st_atime,
		       (unsigned long) statbuf.st_mtime,
		       (unsigned long) statbuf.st_ctime,
		       (unsigned long) statbuf.st_blksize
		);
# ಯದಿ ENABLE_SELINUX
		ಯದಿ (option_mask32 & OPT_SELINUX)
			printf(" %s\n", scontext);
		ಅನ್ಯಥಾ
			bb_putchar('\n');
# endif
	} ಅನ್ಯಥಾ {
		ವರ್ಣ modestr[12];
		ವರ್ಣ *linkname = ರಿಕ್ತ;
		ಸಂರಚನಾ passwd *pw_ent;
		ಸಂರಚನಾ group *gw_ent;

		gw_ent = getgrgid(statbuf.st_gid);
		pw_ent = getpwuid(statbuf.st_uid);

		ಯದಿ (S_ISLNK(statbuf.st_mode))
			linkname = xmalloc_readlink_or_warn(filename);
		ಯದಿ (linkname) {
			printf("  File: '%s' -> '%s'\n", filename, linkname);
			ಮುಕ್ತ(linkname);
		} ಅನ್ಯಥಾ {
			printf("  File: '%s'\n", filename);
		}

		printf("  Size: %-10llu\tBlocks: %-10llu IO Block: %-6lu %s\n"
		       "Device: %llxh/%llud\tInode: %-10llu  Links: %-5lu",
		       (unsigned long long) statbuf.st_size,
		       (unsigned long long) statbuf.st_blocks,
		       (unsigned long) statbuf.st_blksize,
		       file_type(&statbuf),
		       (unsigned long long) statbuf.st_dev,
		       (unsigned long long) statbuf.st_dev,
		       (unsigned long long) statbuf.st_ino,
		       (unsigned long) statbuf.st_nlink);
		ಯದಿ (S_ISBLK(statbuf.st_mode) || S_ISCHR(statbuf.st_mode))
			printf(" Device type: %lx,%lx\n",
			       (unsigned long) major(statbuf.st_rdev),
			       (unsigned long) minor(statbuf.st_rdev));
		ಅನ್ಯಥಾ
			bb_putchar('\n');
		printf("Access: (%04lo/%10.10s)  Uid: (%5lu/%8s)   Gid: (%5lu/%8s)\n",
		       (unsigned long) (statbuf.st_mode & (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)),
		       bb_mode_string(modestr, statbuf.st_mode),
		       (unsigned long) statbuf.st_uid,
		       (pw_ent != ರಿಕ್ತ) ? pw_ent->pw_name : "UNKNOWN",
		       (unsigned long) statbuf.st_gid,
		       (gw_ent != ರಿಕ್ತ) ? gw_ent->gr_name : "UNKNOWN");
# ಯದಿ ENABLE_SELINUX
		ಯದಿ (option_mask32 & OPT_SELINUX)
			printf("   S_Context: %s\n", scontext);
# endif
		printf("Access: %s\n", human_time(&statbuf.st_atim));
		printf("Modify: %s\n", human_time(&statbuf.st_mtim));
		printf("Change: %s\n", human_time(&statbuf.st_ctim));
	}
#endif  /* FEATURE_STAT_FORMAT */
	ಪ್ರತ್ಯಾಗಮ 1;
}

ಪೂರ್ಣಾಙ್ಕ stat_main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಙ್ಕ stat_main(ಪೂರ್ಣಾಙ್ಕ argc UNUSED_PARAM, ವರ್ಣ **argv)
{
	IF_FEATURE_STAT_FORMAT(ವರ್ಣ *format = ರಿಕ್ತ;)
	ಪೂರ್ಣಾಙ್ಕ i;
	ಪೂರ್ಣಾಙ್ಕ ok;
	statfunc_ptr statfunc = do_stat;
#ಯದಿ ENABLE_FEATURE_STAT_FILESYSTEM || ENABLE_SELINUX
	unsigned opts;

	opts =
#endif
	getopt32(argv, "^"
		"tL"
		IF_FEATURE_STAT_FILESYSTEM("f")
		IF_SELINUX("Z")
		IF_FEATURE_STAT_FORMAT("c:")
		"\0" "-1" /* min one arg */
		IF_FEATURE_STAT_FORMAT(,&format)
	);
#ಯದಿ ENABLE_FEATURE_STAT_FILESYSTEM
	ಯದಿ (opts & OPT_FILESYS) /* -f */
		statfunc = do_statfs;
#endif
#ಯದಿ ENABLE_SELINUX
	ಯದಿ (opts & OPT_SELINUX) {
		selinux_or_die();
	}
#endif
	ok = 1;
	argv += optind;
	ಪ್ರತಿ (i = 0; argv[i]; ++i)
		ok &= statfunc(argv[i] IF_FEATURE_STAT_FORMAT(, format));

	ಪ್ರತ್ಯಾಗಮ (ok ? EXIT_SUCCESS : EXIT_FAILURE);
}
