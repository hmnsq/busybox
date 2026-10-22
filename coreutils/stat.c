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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "common_bufsiz.h"

ਗਿਣਤੀ {
	OPT_TERSE       = (1 << 0),
	OPT_DEREFERENCE = (1 << 1),
	OPT_FILESYS     = (1 << 2) * ENABLE_FEATURE_STAT_FILESYSTEM,
	OPT_SELINUX     = (1 << (2+ENABLE_FEATURE_STAT_FILESYSTEM)) * ENABLE_SELINUX,
};

#ਜੇ ENABLE_FEATURE_STAT_FORMAT
ਕਿਸਮਪਰਿਭਾਸ਼ਾ bool (*statfunc_ptr)(ਅਚਲ ਅੱਖਰ *, ਅਚਲ ਅੱਖਰ *);
#ਨਹੀਂਤਾਂ
ਕਿਸਮਪਰਿਭਾਸ਼ਾ bool (*statfunc_ptr)(ਅਚਲ ਅੱਖਰ *);
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *file_type(ਅਚਲ ਬਣਤਰ stat *st)
{
	/* See POSIX 1003.1-2001 XCU Table 4-8 lines 17093-17107
	 * for some of these formats.
	 * To keep diagnostics grammatical in English, the
	 * returned string must start with a consonant.
	 */
	ਜੇ (S_ISREG(st->st_ਬਹੁਲਕ))  ਵਾਪਸ st->st_ਮਾਪ == 0 ? "regular empty file" : "regular file";
	ਜੇ (S_ISDIR(st->st_ਬਹੁਲਕ))  ਵਾਪਸ "directory";
	ਜੇ (S_ISBLK(st->st_ਬਹੁਲਕ))  ਵਾਪਸ "block special file";
	ਜੇ (S_ISCHR(st->st_ਬਹੁਲਕ))  ਵਾਪਸ "character special file";
	ਜੇ (S_ISFIFO(st->st_ਬਹੁਲਕ)) ਵਾਪਸ "fifo";
	ਜੇ (S_ISLNK(st->st_ਬਹੁਲਕ))  ਵਾਪਸ "symbolic link";
	ਜੇ (S_ISSOCK(st->st_ਬਹੁਲਕ)) ਵਾਪਸ "socket";
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ S_TYPEISMQ
	ਜੇ (S_TYPEISMQ(st))        ਵਾਪਸ "message queue";
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ S_TYPEISSEM
	ਜੇ (S_TYPEISSEM(st))       ਵਾਪਸ "semaphore";
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ S_TYPEISSHM
	ਜੇ (S_TYPEISSHM(st))       ਵਾਪਸ "shared memory object";
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ S_TYPEISTMO
	ਜੇ (S_TYPEISTMO(st))       ਵਾਪਸ "typed memory object";
#ਜੇ_ਅੰਤ
	ਵਾਪਸ "weird file";
}

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *human_ਸਮਾਂ(ਬਣਤਰ timespec *ts)
{
	ਅੱਖਰ fmt[ਆਕਾਰ("%Y-%m-%d %H:%M:%S.123456789 %z") + /*paranoia*/ 8];

	/* coreutils 6.3 compat */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ buf bb_common_bufsiz1
	ਤਿਆਰੀ_common_bufsiz();

	sprintf(stpcpy(fmt, "%Y-%m-%d %H:%M:%S"), ".%09u %%z", (ਬਿਨਾਨਿਸ਼ਾਨੀ)ts->tv_nsec);
	strftime(buf, COMMON_BUFSIZE, fmt, localtime(&ts->tv_sec));
	ਵਾਪਸ buf;
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ buf
}

#ਜੇ ENABLE_FEATURE_STAT_FILESYSTEM
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FS_TYPE_LIST \
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
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *human_fstype(uint32_t f_type)
{
# define FS_TYPE(type, name) type,
	ਸਥਿਰ ਅਚਲ uint32_t fstype[] ALIGN4 = {
		FS_TYPE_LIST
	};
# undef FS_TYPE
# define FS_TYPE(type, name) name"\0"
	ਸਥਿਰ ਅਚਲ ਅੱਖਰ humanname[] ALIGN1 =
		FS_TYPE_LIST
		"UNKNOWN";
# undef FS_TYPE
	ਪੂਰਨ i;

	ਲਈ (i = 0; i < ARRAY_ਮਾਪ(fstype); ++i)
		ਜੇ (fstype[i] == f_type)
			ਰੋਕੋ;
	ਵਾਪਸ nth_string(humanname, i);
}

/* "man statfs" says that statfsbuf->f_fsid is a mess */
/* coreutils treats it as an array of ints, most significant first */
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ get_f_fsid(ਅਚਲ ਬਣਤਰ statfs *statfsbuf)
{
	ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ *p = (ਅਚਲ ਖਾਲੀ*) &statfsbuf->f_fsid;
	ਬਿਨਾਨਿਸ਼ਾਨੀ sz = ਆਕਾਰ(statfsbuf->f_fsid) / ਆਕਾਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ);
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ r = 0;

	ਕਰੋ
		r = (r << (ਆਕਾਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ)*8)) | *p++;
	ਜਦੋਂ (--sz > 0);
	ਵਾਪਸ r;
}
#ਜੇ_ਅੰਤ  /* FEATURE_STAT_FILESYSTEM */

#ਜੇ ENABLE_FEATURE_STAT_FORMAT
ਸਥਿਰ ਖਾਲੀ strcatc(ਅੱਖਰ *str, ਅੱਖਰ c)
{
	ਪੂਰਨ len = ਲੜੀ_ਲੰਬਾਈ(str);
	str[len++] = c;
	str[len] = '\0';
}

ਸਥਿਰ ਖਾਲੀ printfs(ਅੱਖਰ *pformat, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ)
{
	strcatc(pformat, 's');
	ਲਿਖੋ(pformat, ਸੁਨੇਹਾ);
}

#ਜੇ ENABLE_FEATURE_STAT_FILESYSTEM
/* print statfs info */
ਸਥਿਰ ਖਾਲੀ FAST_FUNC print_statfs(ਅੱਖਰ *pformat, ਅਚਲ ਅੱਖਰ m,
		ਅਚਲ ਅੱਖਰ *ਅਚਲ filename, ਅਚਲ ਖਾਲੀ *ਅੰਕੜੇ
		IF_SELINUX(, security_context_t scontext))
{
	ਅਚਲ ਬਣਤਰ statfs *statfsbuf = ਅੰਕੜੇ;
	ਜੇ (m == 'n') {
		printfs(pformat, filename);
	} ਨਹੀਂਤਾਂ ਜੇ (m == 'i') {
		strcat(pformat, "llx");
		ਲਿਖੋ(pformat, get_f_fsid(statfsbuf));
	} ਨਹੀਂਤਾਂ ਜੇ (m == 'l') {
		strcat(pformat, "lu");
		ਲਿਖੋ(pformat, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) statfsbuf->f_namelen);
	} ਨਹੀਂਤਾਂ ਜੇ (m == 't') {
		strcat(pformat, "lx");
		ਲਿਖੋ(pformat, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) statfsbuf->f_type); /* no equiv */
	} ਨਹੀਂਤਾਂ ਜੇ (m == 'T') {
		printfs(pformat, human_fstype(statfsbuf->f_type));
	} ਨਹੀਂਤਾਂ ਜੇ (m == 'b') {
		strcat(pformat, "llu");
		ਲਿਖੋ(pformat, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ) statfsbuf->f_blocks);
	} ਨਹੀਂਤਾਂ ਜੇ (m == 'f') {
		strcat(pformat, "llu");
		ਲਿਖੋ(pformat, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ) statfsbuf->f_bfree);
	} ਨਹੀਂਤਾਂ ਜੇ (m == 'a') {
		strcat(pformat, "llu");
		ਲਿਖੋ(pformat, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ) statfsbuf->f_bavail);
	} ਨਹੀਂਤਾਂ ਜੇ (m == 's' || m == 'S') {
		strcat(pformat, "lu");
		ਲਿਖੋ(pformat, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) statfsbuf->f_bsize);
	} ਨਹੀਂਤਾਂ ਜੇ (m == 'c') {
		strcat(pformat, "llu");
		ਲਿਖੋ(pformat, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ) statfsbuf->f_files);
	} ਨਹੀਂਤਾਂ ਜੇ (m == 'd') {
		strcat(pformat, "llu");
		ਲਿਖੋ(pformat, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ) statfsbuf->f_ffree);
# ਜੇ ENABLE_SELINUX
	} ਨਹੀਂਤਾਂ ਜੇ (m == 'C' && (option_mask32 & OPT_SELINUX)) {
		printfs(pformat, scontext);
# endif
	} ਨਹੀਂਤਾਂ {
		strcatc(pformat, 'c');
		ਲਿਖੋ(pformat, m);
	}
}
#ਜੇ_ਅੰਤ

/* print stat info */
ਸਥਿਰ ਖਾਲੀ FAST_FUNC print_stat(ਅੱਖਰ *pformat, ਅਚਲ ਅੱਖਰ m,
		ਅਚਲ ਅੱਖਰ *ਅਚਲ filename, ਅਚਲ ਖਾਲੀ *ਅੰਕੜੇ
		IF_SELINUX(, security_context_t scontext))
{
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TYPE_SIGNED(t) (! ((t) 0 < (t) -1))
	ਬਣਤਰ stat *statbuf = (ਬਣਤਰ stat *) ਅੰਕੜੇ;
	ਬਣਤਰ passwd *pw_ent;
	ਬਣਤਰ ਸਮੂਹ *gw_ent;

	ਜੇ (m == 'n') {
		printfs(pformat, filename);
	} ਨਹੀਂਤਾਂ ਜੇ (m == 'N') {
		strcatc(pformat, 's');
		ਜੇ (S_ISLNK(statbuf->st_ਬਹੁਲਕ)) {
			ਅੱਖਰ *linkname = xmalloc_readlink_or_warn(filename);
			ਜੇ (linkname == ਨਿਰਾ)
				ਵਾਪਸ;
			ਲਿਖੋ("'%s' -> '%s'", filename, linkname);
			ਮੁਕਤ(linkname);
		} ਨਹੀਂਤਾਂ {
			ਲਿਖੋ(pformat, filename);
		}
	} ਨਹੀਂਤਾਂ ਜੇ (m == 'd') {
		strcat(pformat, "llu");
		ਲਿਖੋ(pformat, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ) statbuf->st_dev);
	} ਨਹੀਂਤਾਂ ਜੇ (m == 'D') {
		strcat(pformat, "llx");
		ਲਿਖੋ(pformat, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ) statbuf->st_dev);
	} ਨਹੀਂਤਾਂ ਜੇ (m == 'i') {
		strcat(pformat, "llu");
		ਲਿਖੋ(pformat, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ) statbuf->st_ino);
	} ਨਹੀਂਤਾਂ ਜੇ (m == 'a') {
		strcat(pformat, "lo");
		ਲਿਖੋ(pformat, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) (statbuf->st_ਬਹੁਲਕ & (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)));
	} ਨਹੀਂਤਾਂ ਜੇ (m == 'A') {
		ਅੱਖਰ modestr[12];
		printfs(pformat, bb_ਬਹੁਲਕ_string(modestr, statbuf->st_ਬਹੁਲਕ));
	} ਨਹੀਂਤਾਂ ਜੇ (m == 'f') {
		strcat(pformat, "lx");
		ਲਿਖੋ(pformat, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) statbuf->st_ਬਹੁਲਕ);
	} ਨਹੀਂਤਾਂ ਜੇ (m == 'F') {
		printfs(pformat, file_type(statbuf));
	} ਨਹੀਂਤਾਂ ਜੇ (m == 'h') {
		strcat(pformat, "lu");
		ਲਿਖੋ(pformat, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) statbuf->st_nlink);
	} ਨਹੀਂਤਾਂ ਜੇ (m == 'u') {
		strcat(pformat, "lu");
		ਲਿਖੋ(pformat, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) statbuf->st_uid);
	} ਨਹੀਂਤਾਂ ਜੇ (m == 'U') {
		pw_ent = getpwuid(statbuf->st_uid);
		printfs(pformat, (pw_ent != ਨਿਰਾ) ? pw_ent->pw_name : "UNKNOWN");
	} ਨਹੀਂਤਾਂ ਜੇ (m == 'g') {
		strcat(pformat, "lu");
		ਲਿਖੋ(pformat, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) statbuf->st_gid);
	} ਨਹੀਂਤਾਂ ਜੇ (m == 'G') {
		gw_ent = getgrgid(statbuf->st_gid);
		printfs(pformat, (gw_ent != ਨਿਰਾ) ? gw_ent->gr_name : "UNKNOWN");
	} ਨਹੀਂਤਾਂ ਜੇ (m == 't') {
		strcat(pformat, "lx");
		ਲਿਖੋ(pformat, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) major(statbuf->st_rdev));
	} ਨਹੀਂਤਾਂ ਜੇ (m == 'T') {
		strcat(pformat, "lx");
		ਲਿਖੋ(pformat, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) minor(statbuf->st_rdev));
	} ਨਹੀਂਤਾਂ ਜੇ (m == 's') {
		strcat(pformat, "llu");
		ਲਿਖੋ(pformat, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ) statbuf->st_ਮਾਪ);
	} ਨਹੀਂਤਾਂ ਜੇ (m == 'B') {
		strcat(pformat, "lu");
		ਲਿਖੋ(pformat, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) 512); //ST_NBLOCKSIZE
	} ਨਹੀਂਤਾਂ ਜੇ (m == 'b') {
		strcat(pformat, "llu");
		ਲਿਖੋ(pformat, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ) statbuf->st_ਖੰਡਾਂ);
	} ਨਹੀਂਤਾਂ ਜੇ (m == 'o') {
		strcat(pformat, "lu");
		ਲਿਖੋ(pformat, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) statbuf->st_blksize);
	} ਨਹੀਂਤਾਂ ਜੇ (m == 'x') {
		printfs(pformat, human_ਸਮਾਂ(&statbuf->st_atim));
	} ਨਹੀਂਤਾਂ ਜੇ (m == 'X') {
		strcat(pformat, TYPE_SIGNED(ਸਮਾਂ_t) ? "ld" : "lu");
		/* note: (unsigned long) would be wrong:
		 * imagine (unsigned long64)int32 */
		ਲਿਖੋ(pformat, (ਲੰਮਾ) statbuf->st_atime);
	} ਨਹੀਂਤਾਂ ਜੇ (m == 'y') {
		printfs(pformat, human_ਸਮਾਂ(&statbuf->st_mtim));
	} ਨਹੀਂਤਾਂ ਜੇ (m == 'Y') {
		strcat(pformat, TYPE_SIGNED(ਸਮਾਂ_t) ? "ld" : "lu");
		ਲਿਖੋ(pformat, (ਲੰਮਾ) statbuf->st_mtime);
	} ਨਹੀਂਤਾਂ ਜੇ (m == 'z') {
		printfs(pformat, human_ਸਮਾਂ(&statbuf->st_ctim));
	} ਨਹੀਂਤਾਂ ਜੇ (m == 'Z') {
		strcat(pformat, TYPE_SIGNED(ਸਮਾਂ_t) ? "ld" : "lu");
		ਲਿਖੋ(pformat, (ਲੰਮਾ) statbuf->st_ctime);
# ਜੇ ENABLE_SELINUX
	} ਨਹੀਂਤਾਂ ਜੇ (m == 'C' && (option_mask32 & OPT_SELINUX)) {
		printfs(pformat, scontext);
# endif
	} ਨਹੀਂਤਾਂ {
		strcatc(pformat, 'c');
		ਲਿਖੋ(pformat, m);
	}
}

ਸਥਿਰ ਖਾਲੀ print_it(ਅਚਲ ਅੱਖਰ *masterformat,
		ਅਚਲ ਅੱਖਰ *filename,
		ਖਾਲੀ FAST_FUNC (*print_func)(ਅੱਖਰ*, ਅੱਖਰ, ਅਚਲ ਅੱਖਰ*, ਅਚਲ ਖਾਲੀ* IF_SELINUX(, security_context_t scontext)),
		ਅਚਲ ਖਾਲੀ *ਅੰਕੜੇ
		IF_SELINUX(, security_context_t scontext))
{
	/* Create a working copy of the format string */
	ਅੱਖਰ *format = xstrdup(masterformat);
	/* Add 2 to accommodate our conversion of the stat '%s' format string
	 * to the printf '%llu' one.  */
	ਅੱਖਰ *dest = xmalloc(ਲੜੀ_ਲੰਬਾਈ(format) + 2 + 1);
	ਅੱਖਰ *b;

	b = format;
	ਜਦੋਂ (b) {
		/* Each iteration finds next %spec,
		 * prints preceding string and handles found %spec
		 */
		ਮਾਪ_t len;
		ਅੱਖਰ *p = strchr(b, '%');
		ਜੇ (!p) {
			/* coreutils 6.3 always prints newline at the end */
			/*fputs(b, stdout);*/
			puts(b);
			ਰੋਕੋ;
		}

		/* dest = "%<modifiers>" */
		len = 1 + strspn(p + 1, "#-+.I 0123456789");
		ਸਮ੍ਰਿਤੀ_ਨਕਲ(dest, p, len);
		dest[len] = '\0';

		/* print preceding string */
		*p = '\0';
		fputs_stdout(b);

		p += len;
		b = p + 1;
		ਚੋਣ (*p) {
		ਹਾਲਤ '\0':
			b = ਨਿਰਾ;
			/* fall through */
		ਹਾਲਤ '%':
			bb_putchar('%');
			ਰੋਕੋ;
		ਮੂਲ:
			/* Completes "%<modifiers>" with specifier and printfs */
			print_func(dest, *p, filename, ਅੰਕੜੇ IF_SELINUX(,scontext));
			ਰੋਕੋ;
		}
	}

	ਮੁਕਤ(format);
	ਮੁਕਤ(dest);
}
#ਜੇ_ਅੰਤ  /* FEATURE_STAT_FORMAT */

#ਜੇ ENABLE_FEATURE_STAT_FILESYSTEM
/* Stat the file system and print what we find.  */
#ਜੇ !ENABLE_FEATURE_STAT_FORMAT
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ do_statfs(filename, format) do_statfs(filename)
#ਜੇ_ਅੰਤ
ਸਥਿਰ bool do_statfs(ਅਚਲ ਅੱਖਰ *filename, ਅਚਲ ਅੱਖਰ *format)
{
	ਬਣਤਰ statfs statfsbuf;
#ਜੇ !ENABLE_FEATURE_STAT_FORMAT
	ਅਚਲ ਅੱਖਰ *format;
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_SELINUX
	security_context_t scontext = ਨਿਰਾ;

	ਜੇ (option_mask32 & OPT_SELINUX) {
		ਜੇ ((option_mask32 & OPT_DEREFERENCE
		     ? lgetfilecon(filename, &scontext)
		     : getfilecon(filename, &scontext)
		    ) < 0
		) {
			bb_simple_perror_ਸੁਨੇਹਾ(filename);
			ਵਾਪਸ 0;
		}
	}
#ਜੇ_ਅੰਤ
	ਜੇ (statfs(filename, &statfsbuf) != 0) {
		bb_perror_ਸੁਨੇਹਾ("can't read file system information for '%s'", filename);
		ਵਾਪਸ 0;
	}

#ਜੇ ENABLE_FEATURE_STAT_FORMAT
	ਜੇ (format == ਨਿਰਾ) {
# ਜੇ !ENABLE_SELINUX
		format = (option_mask32 & OPT_TERSE
			? "%n %i %l %t %s %b %f %a %c %d"
			: "  File: \"%n\"\n"
			  "    ID: %-8i Namelen: %-7l Type: %T\n"
			  "Block size: %-10s\n"
			  "Blocks: Total: %-10b Free: %-10f Available: %a\n"
			  "Inodes: Total: %-10c Free: %d");
# ਨਹੀਂਤਾਂ
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
#ਨਹੀਂਤਾਂ /* !FEATURE_STAT_FORMAT */
	format = (option_mask32 & OPT_TERSE
		? "%s %llx %lu "
		: "  File: \"%s\"\n"
		  "    ID: %-8llx Namelen: %-7lu ");
	ਲਿਖੋ(format,
	       filename,
	       get_f_fsid(&statfsbuf),
	       statfsbuf.f_namelen);

	ਜੇ (option_mask32 & OPT_TERSE)
		ਲਿਖੋ("%lx ", (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) statfsbuf.f_type);
	ਨਹੀਂਤਾਂ
		ਲਿਖੋ("Type: %s\n", human_fstype(statfsbuf.f_type));

# ਜੇ !ENABLE_SELINUX
	format = (option_mask32 & OPT_TERSE
		? "%lu %llu %llu %llu %llu %llu\n"
		: "Block size: %-10lu\n"
		  "Blocks: Total: %-10llu Free: %-10llu Available: %llu\n"
		  "Inodes: Total: %-10llu Free: %llu\n");
	ਲਿਖੋ(format,
	       (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) statfsbuf.f_bsize,
	       (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ) statfsbuf.f_blocks,
	       (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ) statfsbuf.f_bfree,
	       (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ) statfsbuf.f_bavail,
	       (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ) statfsbuf.f_files,
	       (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ) statfsbuf.f_ffree);
# ਨਹੀਂਤਾਂ
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
	ਲਿਖੋ(format,
		(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) statfsbuf.f_bsize,
		(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ) statfsbuf.f_blocks,
		(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ) statfsbuf.f_bfree,
		(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ) statfsbuf.f_bavail,
		(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ) statfsbuf.f_files,
		(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ) statfsbuf.f_ffree,
		scontext);

	ਜੇ (scontext)
		freecon(scontext);
# endif
#ਜੇ_ਅੰਤ  /* FEATURE_STAT_FORMAT */
	ਵਾਪਸ 1;
}
#ਜੇ_ਅੰਤ  /* FEATURE_STAT_FILESYSTEM */

/* stat the file and print what we find */
#ਜੇ !ENABLE_FEATURE_STAT_FORMAT
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ do_stat(filename, format) do_stat(filename)
#ਜੇ_ਅੰਤ
ਸਥਿਰ bool do_stat(ਅਚਲ ਅੱਖਰ *filename, ਅਚਲ ਅੱਖਰ *format)
{
	ਬਣਤਰ stat statbuf;
#ਜੇ ENABLE_SELINUX
	security_context_t scontext = ਨਿਰਾ;

	ਜੇ (option_mask32 & OPT_SELINUX) {
		ਜੇ ((option_mask32 & OPT_DEREFERENCE
		     ? lgetfilecon(filename, &scontext)
		     : getfilecon(filename, &scontext)
		    ) < 0
		) {
			bb_simple_perror_ਸੁਨੇਹਾ(filename);
			ਵਾਪਸ 0;
		}
	}
#ਜੇ_ਅੰਤ
	ਜੇ ((option_mask32 & OPT_DEREFERENCE ? stat : lstat) (filename, &statbuf) != 0) {
		bb_perror_ਸੁਨੇਹਾ("can't stat '%s'", filename);
		ਵਾਪਸ 0;
	}

#ਜੇ ENABLE_FEATURE_STAT_FORMAT
	ਜੇ (format == ਨਿਰਾ) {
# ਜੇ !ENABLE_SELINUX
		ਜੇ (option_mask32 & OPT_TERSE) {
			format = "%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %o";
		} ਨਹੀਂਤਾਂ {
			ਜੇ (S_ISBLK(statbuf.st_ਬਹੁਲਕ) || S_ISCHR(statbuf.st_ਬਹੁਲਕ)) {
				format =
					"  File: %N\n"
					"  Size: %-10s\tBlocks: %-10b IO Block: %-6o %F\n"
					"Device: %Dh/%dd\tInode: %-10i  Links: %-5h"
					" Device type: %t,%T\n"
					"Access: (%04a/%10.10A)  Uid: (%5u/%8U)   Gid: (%5g/%8G)\n"
					"Access: %x\n" "Modify: %y\n" "Change: %z";
			} ਨਹੀਂਤਾਂ {
				format =
					"  File: %N\n"
					"  Size: %-10s\tBlocks: %-10b IO Block: %-6o %F\n"
					"Device: %Dh/%dd\tInode: %-10i  Links: %h\n"
					"Access: (%04a/%10.10A)  Uid: (%5u/%8U)   Gid: (%5g/%8G)\n"
					"Access: %x\n" "Modify: %y\n" "Change: %z";
			}
		}
# ਨਹੀਂਤਾਂ
		ਜੇ (option_mask32 & OPT_TERSE) {
			format = (option_mask32 & OPT_SELINUX ?
				"%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %o %C\n"
				:
				"%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %o\n"
				);
		} ਨਹੀਂਤਾਂ {
			ਜੇ (S_ISBLK(statbuf.st_ਬਹੁਲਕ) || S_ISCHR(statbuf.st_ਬਹੁਲਕ)) {
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
			} ਨਹੀਂਤਾਂ {
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
#ਨਹੀਂਤਾਂ	/* FEATURE_STAT_FORMAT */
	ਜੇ (option_mask32 & OPT_TERSE) {
		ਲਿਖੋ("%s %llu %llu %lx %lu %lu %llx %llu %lu %lx %lx %lu %lu %lu %lu"
		       IF_NOT_SELINUX("\n"),
		       filename,
		       (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ) statbuf.st_ਮਾਪ,
		       (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ) statbuf.st_ਖੰਡਾਂ,
		       (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) statbuf.st_ਬਹੁਲਕ,
		       (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) statbuf.st_uid,
		       (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) statbuf.st_gid,
		       (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ) statbuf.st_dev,
		       (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ) statbuf.st_ino,
		       (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) statbuf.st_nlink,
		       (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) major(statbuf.st_rdev),
		       (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) minor(statbuf.st_rdev),
		       (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) statbuf.st_atime,
		       (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) statbuf.st_mtime,
		       (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) statbuf.st_ctime,
		       (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) statbuf.st_blksize
		);
# ਜੇ ENABLE_SELINUX
		ਜੇ (option_mask32 & OPT_SELINUX)
			ਲਿਖੋ(" %s\n", scontext);
		ਨਹੀਂਤਾਂ
			bb_putchar('\n');
# endif
	} ਨਹੀਂਤਾਂ {
		ਅੱਖਰ modestr[12];
		ਅੱਖਰ *linkname = ਨਿਰਾ;
		ਬਣਤਰ passwd *pw_ent;
		ਬਣਤਰ ਸਮੂਹ *gw_ent;

		gw_ent = getgrgid(statbuf.st_gid);
		pw_ent = getpwuid(statbuf.st_uid);

		ਜੇ (S_ISLNK(statbuf.st_ਬਹੁਲਕ))
			linkname = xmalloc_readlink_or_warn(filename);
		ਜੇ (linkname) {
			ਲਿਖੋ("  File: '%s' -> '%s'\n", filename, linkname);
			ਮੁਕਤ(linkname);
		} ਨਹੀਂਤਾਂ {
			ਲਿਖੋ("  File: '%s'\n", filename);
		}

		ਲਿਖੋ("  Size: %-10llu\tBlocks: %-10llu IO Block: %-6lu %s\n"
		       "Device: %llxh/%llud\tInode: %-10llu  Links: %-5lu",
		       (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ) statbuf.st_ਮਾਪ,
		       (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ) statbuf.st_ਖੰਡਾਂ,
		       (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) statbuf.st_blksize,
		       file_type(&statbuf),
		       (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ) statbuf.st_dev,
		       (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ) statbuf.st_dev,
		       (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ) statbuf.st_ino,
		       (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) statbuf.st_nlink);
		ਜੇ (S_ISBLK(statbuf.st_ਬਹੁਲਕ) || S_ISCHR(statbuf.st_ਬਹੁਲਕ))
			ਲਿਖੋ(" Device type: %lx,%lx\n",
			       (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) major(statbuf.st_rdev),
			       (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) minor(statbuf.st_rdev));
		ਨਹੀਂਤਾਂ
			bb_putchar('\n');
		ਲਿਖੋ("Access: (%04lo/%10.10s)  Uid: (%5lu/%8s)   Gid: (%5lu/%8s)\n",
		       (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) (statbuf.st_ਬਹੁਲਕ & (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)),
		       bb_ਬਹੁਲਕ_string(modestr, statbuf.st_ਬਹੁਲਕ),
		       (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) statbuf.st_uid,
		       (pw_ent != ਨਿਰਾ) ? pw_ent->pw_name : "UNKNOWN",
		       (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) statbuf.st_gid,
		       (gw_ent != ਨਿਰਾ) ? gw_ent->gr_name : "UNKNOWN");
# ਜੇ ENABLE_SELINUX
		ਜੇ (option_mask32 & OPT_SELINUX)
			ਲਿਖੋ("   S_Context: %s\n", scontext);
# endif
		ਲਿਖੋ("Access: %s\n", human_ਸਮਾਂ(&statbuf.st_atim));
		ਲਿਖੋ("Modify: %s\n", human_ਸਮਾਂ(&statbuf.st_mtim));
		ਲਿਖੋ("Change: %s\n", human_ਸਮਾਂ(&statbuf.st_ctim));
	}
#ਜੇ_ਅੰਤ  /* FEATURE_STAT_FORMAT */
	ਵਾਪਸ 1;
}

ਪੂਰਨ stat_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ stat_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
	IF_FEATURE_STAT_FORMAT(ਅੱਖਰ *format = ਨਿਰਾ;)
	ਪੂਰਨ i;
	ਪੂਰਨ ok;
	statfunc_ptr statfunc = do_stat;
#ਜੇ ENABLE_FEATURE_STAT_FILESYSTEM || ENABLE_SELINUX
	ਬਿਨਾਨਿਸ਼ਾਨੀ opts;

	opts =
#ਜੇ_ਅੰਤ
	getopt32(argv, "^"
		"tL"
		IF_FEATURE_STAT_FILESYSTEM("f")
		IF_SELINUX("Z")
		IF_FEATURE_STAT_FORMAT("c:")
		"\0" "-1" /* min one arg */
		IF_FEATURE_STAT_FORMAT(,&format)
	);
#ਜੇ ENABLE_FEATURE_STAT_FILESYSTEM
	ਜੇ (opts & OPT_FILESYS) /* -f */
		statfunc = do_statfs;
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_SELINUX
	ਜੇ (opts & OPT_SELINUX) {
		selinux_or_die();
	}
#ਜੇ_ਅੰਤ
	ok = 1;
	argv += optind;
	ਲਈ (i = 0; argv[i]; ++i)
		ok &= statfunc(argv[i] IF_FEATURE_STAT_FORMAT(, format));

	ਵਾਪਸ (ok ? EXIT_SUCCESS : EXIT_FAILURE);
}
