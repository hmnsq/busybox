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

ಎಣಿಕೆ {
	OPT_TERSE       = (1 << 0),
	OPT_DEREFERENCE = (1 << 1),
	OPT_FILESYS     = (1 << 2) * ENABLE_FEATURE_STAT_FILESYSTEM,
	OPT_SELINUX     = (1 << (2+ENABLE_FEATURE_STAT_FILESYSTEM)) * ENABLE_SELINUX,
};

#ಆದರೆ ENABLE_FEATURE_STAT_FORMAT
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ bool (*statfunc_ptr)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *);
#ಇಲ್ಲದಿದ್ದರೆ
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ bool (*statfunc_ptr)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *);
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *file_type(ಸ್ಥಿರಾಂಕ ರಚನೆ stat *st)
{
	/* See POSIX 1003.1-2001 XCU Table 4-8 lines 17093-17107
	 * for some of these formats.
	 * To keep diagnostics grammatical in English, the
	 * returned string must start with a consonant.
	 */
	ಆದರೆ (S_ISREG(st->st_ಬಹುಲಕ))  ಹಿಂತಿರುಗಿ st->st_ಗಾತ್ರ == 0 ? "regular empty file" : "regular file";
	ಆದರೆ (S_ISDIR(st->st_ಬಹುಲಕ))  ಹಿಂತಿರುಗಿ "directory";
	ಆದರೆ (S_ISBLK(st->st_ಬಹುಲಕ))  ಹಿಂತಿರುಗಿ "block special file";
	ಆದರೆ (S_ISCHR(st->st_ಬಹುಲಕ))  ಹಿಂತಿರುಗಿ "character special file";
	ಆದರೆ (S_ISFIFO(st->st_ಬಹುಲಕ)) ಹಿಂತಿರುಗಿ "fifo";
	ಆದರೆ (S_ISLNK(st->st_ಬಹುಲಕ))  ಹಿಂತಿರುಗಿ "symbolic link";
	ಆದರೆ (S_ISSOCK(st->st_ಬಹುಲಕ)) ಹಿಂತಿರುಗಿ "socket";
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ S_TYPEISMQ
	ಆದರೆ (S_TYPEISMQ(st))        ಹಿಂತಿರುಗಿ "message queue";
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ S_TYPEISSEM
	ಆದರೆ (S_TYPEISSEM(st))       ಹಿಂತಿರುಗಿ "semaphore";
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ S_TYPEISSHM
	ಆದರೆ (S_TYPEISSHM(st))       ಹಿಂತಿರುಗಿ "shared memory object";
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ S_TYPEISTMO
	ಆದರೆ (S_TYPEISTMO(st))       ಹಿಂತಿರುಗಿ "typed memory object";
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ "weird file";
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *human_ಸಮಯ(ರಚನೆ timespec *ts)
{
	ಅಕ್ಷರ fmt[ಗಾತ್ರ("%Y-%m-%d %H:%M:%S.123456789 %z") + /*paranoia*/ 8];

	/* coreutils 6.3 compat */
#ವ್ಯಾಖ್ಯಾನಿಸು buf bb_common_bufsiz1
	ಸಿದ್ಧತೆ_common_bufsiz();

	sprintf(stpcpy(fmt, "%Y-%m-%d %H:%M:%S"), ".%09u %%z", (ಚಿಹ್ನೆರಹಿತ)ts->tv_nsec);
	strftime(buf, COMMON_BUFSIZE, fmt, localtime(&ts->tv_sec));
	ಹಿಂತಿರುಗಿ buf;
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು buf
}

#ಆದರೆ ENABLE_FEATURE_STAT_FILESYSTEM
#ವ್ಯಾಖ್ಯಾನಿಸು FS_TYPE_LIST \
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
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *human_fstype(uint32_t f_type)
{
# define FS_TYPE(type, name) type,
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ uint32_t fstype[] ALIGN4 = {
		FS_TYPE_LIST
	};
# undef FS_TYPE
# define FS_TYPE(type, name) name"\0"
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ humanname[] ALIGN1 =
		FS_TYPE_LIST
		"UNKNOWN";
# undef FS_TYPE
	ಪೂರ್ಣಾಂಕ i;

	ಗಾಗಿ (i = 0; i < ARRAY_ಗಾತ್ರ(fstype); ++i)
		ಆದರೆ (fstype[i] == f_type)
			ನಿಲ್ಲಿಸು;
	ಹಿಂತಿರುಗಿ nth_string(humanname, i);
}

/* "man statfs" says that statfsbuf->f_fsid is a mess */
/* coreutils treats it as an array of ints, most significant first */
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ get_f_fsid(ಸ್ಥಿರಾಂಕ ರಚನೆ statfs *statfsbuf)
{
	ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ *p = (ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*) &statfsbuf->f_fsid;
	ಚಿಹ್ನೆರಹಿತ sz = ಗಾತ್ರ(statfsbuf->f_fsid) / ಗಾತ್ರ(ಚಿಹ್ನೆರಹಿತ);
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ r = 0;

	ಮಾಡು
		r = (r << (ಗಾತ್ರ(ಚಿಹ್ನೆರಹಿತ)*8)) | *p++;
	ಆಗ (--sz > 0);
	ಹಿಂತಿರುಗಿ r;
}
#ಆದರೆ_ಅಂತ್ಯ  /* FEATURE_STAT_FILESYSTEM */

#ಆದರೆ ENABLE_FEATURE_STAT_FORMAT
ಸ್ಥಿರ ಶೂನ್ಯ strcatc(ಅಕ್ಷರ *str, ಅಕ್ಷರ c)
{
	ಪೂರ್ಣಾಂಕ len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(str);
	str[len++] = c;
	str[len] = '\0';
}

ಸ್ಥಿರ ಶೂನ್ಯ printfs(ಅಕ್ಷರ *pformat, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ)
{
	strcatc(pformat, 's');
	ಬರೆ(pformat, ಸಂದೇಶ);
}

#ಆದರೆ ENABLE_FEATURE_STAT_FILESYSTEM
/* print statfs info */
ಸ್ಥಿರ ಶೂನ್ಯ FAST_FUNC print_statfs(ಅಕ್ಷರ *pformat, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ m,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ filename, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ದತ್ತಾಂಶ
		IF_SELINUX(, security_context_t scontext))
{
	ಸ್ಥಿರಾಂಕ ರಚನೆ statfs *statfsbuf = ದತ್ತಾಂಶ;
	ಆದರೆ (m == 'n') {
		printfs(pformat, filename);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 'i') {
		strcat(pformat, "llx");
		ಬರೆ(pformat, get_f_fsid(statfsbuf));
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 'l') {
		strcat(pformat, "lu");
		ಬರೆ(pformat, (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) statfsbuf->f_namelen);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 't') {
		strcat(pformat, "lx");
		ಬರೆ(pformat, (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) statfsbuf->f_type); /* no equiv */
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 'T') {
		printfs(pformat, human_fstype(statfsbuf->f_type));
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 'b') {
		strcat(pformat, "llu");
		ಬರೆ(pformat, (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ) statfsbuf->f_blocks);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 'f') {
		strcat(pformat, "llu");
		ಬರೆ(pformat, (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ) statfsbuf->f_bfree);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 'a') {
		strcat(pformat, "llu");
		ಬರೆ(pformat, (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ) statfsbuf->f_bavail);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 's' || m == 'S') {
		strcat(pformat, "lu");
		ಬರೆ(pformat, (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) statfsbuf->f_bsize);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 'c') {
		strcat(pformat, "llu");
		ಬರೆ(pformat, (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ) statfsbuf->f_files);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 'd') {
		strcat(pformat, "llu");
		ಬರೆ(pformat, (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ) statfsbuf->f_ffree);
# ಆದರೆ ENABLE_SELINUX
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 'C' && (option_mask32 & OPT_SELINUX)) {
		printfs(pformat, scontext);
# endif
	} ಇಲ್ಲದಿದ್ದರೆ {
		strcatc(pformat, 'c');
		ಬರೆ(pformat, m);
	}
}
#ಆದರೆ_ಅಂತ್ಯ

/* print stat info */
ಸ್ಥಿರ ಶೂನ್ಯ FAST_FUNC print_stat(ಅಕ್ಷರ *pformat, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ m,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ filename, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ದತ್ತಾಂಶ
		IF_SELINUX(, security_context_t scontext))
{
#ವ್ಯಾಖ್ಯಾನಿಸು TYPE_SIGNED(t) (! ((t) 0 < (t) -1))
	ರಚನೆ stat *statbuf = (ರಚನೆ stat *) ದತ್ತಾಂಶ;
	ರಚನೆ passwd *pw_ent;
	ರಚನೆ ಗುಂಪು *gw_ent;

	ಆದರೆ (m == 'n') {
		printfs(pformat, filename);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 'N') {
		strcatc(pformat, 's');
		ಆದರೆ (S_ISLNK(statbuf->st_ಬಹುಲಕ)) {
			ಅಕ್ಷರ *linkname = xmalloc_readlink_or_warn(filename);
			ಆದರೆ (linkname == ಖಾಲಿ)
				ಹಿಂತಿರುಗಿ;
			ಬರೆ("'%s' -> '%s'", filename, linkname);
			ಬಿಡುಗಡೆ(linkname);
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಬರೆ(pformat, filename);
		}
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 'd') {
		strcat(pformat, "llu");
		ಬರೆ(pformat, (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ) statbuf->st_dev);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 'D') {
		strcat(pformat, "llx");
		ಬರೆ(pformat, (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ) statbuf->st_dev);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 'i') {
		strcat(pformat, "llu");
		ಬರೆ(pformat, (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ) statbuf->st_ino);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 'a') {
		strcat(pformat, "lo");
		ಬರೆ(pformat, (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) (statbuf->st_ಬಹುಲಕ & (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)));
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 'A') {
		ಅಕ್ಷರ modestr[12];
		printfs(pformat, bb_ಬಹುಲಕ_string(modestr, statbuf->st_ಬಹುಲಕ));
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 'f') {
		strcat(pformat, "lx");
		ಬರೆ(pformat, (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) statbuf->st_ಬಹುಲಕ);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 'F') {
		printfs(pformat, file_type(statbuf));
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 'h') {
		strcat(pformat, "lu");
		ಬರೆ(pformat, (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) statbuf->st_nlink);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 'u') {
		strcat(pformat, "lu");
		ಬರೆ(pformat, (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) statbuf->st_uid);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 'U') {
		pw_ent = getpwuid(statbuf->st_uid);
		printfs(pformat, (pw_ent != ಖಾಲಿ) ? pw_ent->pw_name : "UNKNOWN");
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 'g') {
		strcat(pformat, "lu");
		ಬರೆ(pformat, (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) statbuf->st_gid);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 'G') {
		gw_ent = getgrgid(statbuf->st_gid);
		printfs(pformat, (gw_ent != ಖಾಲಿ) ? gw_ent->gr_name : "UNKNOWN");
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 't') {
		strcat(pformat, "lx");
		ಬರೆ(pformat, (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) major(statbuf->st_rdev));
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 'T') {
		strcat(pformat, "lx");
		ಬರೆ(pformat, (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) minor(statbuf->st_rdev));
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 's') {
		strcat(pformat, "llu");
		ಬರೆ(pformat, (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ) statbuf->st_ಗಾತ್ರ);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 'B') {
		strcat(pformat, "lu");
		ಬರೆ(pformat, (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) 512); //ST_NBLOCKSIZE
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 'b') {
		strcat(pformat, "llu");
		ಬರೆ(pformat, (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ) statbuf->st_ಖಂಡಗಳು);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 'o') {
		strcat(pformat, "lu");
		ಬರೆ(pformat, (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) statbuf->st_blksize);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 'x') {
		printfs(pformat, human_ಸಮಯ(&statbuf->st_atim));
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 'X') {
		strcat(pformat, TYPE_SIGNED(ಸಮಯ_t) ? "ld" : "lu");
		/* note: (unsigned long) would be wrong:
		 * imagine (unsigned long64)int32 */
		ಬರೆ(pformat, (ದೀರ್ಘ) statbuf->st_atime);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 'y') {
		printfs(pformat, human_ಸಮಯ(&statbuf->st_mtim));
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 'Y') {
		strcat(pformat, TYPE_SIGNED(ಸಮಯ_t) ? "ld" : "lu");
		ಬರೆ(pformat, (ದೀರ್ಘ) statbuf->st_mtime);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 'z') {
		printfs(pformat, human_ಸಮಯ(&statbuf->st_ctim));
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 'Z') {
		strcat(pformat, TYPE_SIGNED(ಸಮಯ_t) ? "ld" : "lu");
		ಬರೆ(pformat, (ದೀರ್ಘ) statbuf->st_ctime);
# ಆದರೆ ENABLE_SELINUX
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (m == 'C' && (option_mask32 & OPT_SELINUX)) {
		printfs(pformat, scontext);
# endif
	} ಇಲ್ಲದಿದ್ದರೆ {
		strcatc(pformat, 'c');
		ಬರೆ(pformat, m);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ print_it(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *masterformat,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename,
		ಶೂನ್ಯ FAST_FUNC (*print_func)(ಅಕ್ಷರ*, ಅಕ್ಷರ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* IF_SELINUX(, security_context_t scontext)),
		ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ದತ್ತಾಂಶ
		IF_SELINUX(, security_context_t scontext))
{
	/* Create a working copy of the format string */
	ಅಕ್ಷರ *format = xstrdup(masterformat);
	/* Add 2 to accommodate our conversion of the stat '%s' format string
	 * to the printf '%llu' one.  */
	ಅಕ್ಷರ *dest = xmalloc(ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(format) + 2 + 1);
	ಅಕ್ಷರ *b;

	b = format;
	ಆಗ (b) {
		/* Each iteration finds next %spec,
		 * prints preceding string and handles found %spec
		 */
		ಗಾತ್ರ_t len;
		ಅಕ್ಷರ *p = strchr(b, '%');
		ಆದರೆ (!p) {
			/* coreutils 6.3 always prints newline at the end */
			/*fputs(b, stdout);*/
			puts(b);
			ನಿಲ್ಲಿಸು;
		}

		/* dest = "%<modifiers>" */
		len = 1 + strspn(p + 1, "#-+.I 0123456789");
		ಸ್ಮರಣೆ_ನಕಲು(dest, p, len);
		dest[len] = '\0';

		/* print preceding string */
		*p = '\0';
		fputs_stdout(b);

		p += len;
		b = p + 1;
		ಆಯ್ಕೆ (*p) {
		ಸ್ಥಿತಿ '\0':
			b = ಖಾಲಿ;
			/* fall through */
		ಸ್ಥಿತಿ '%':
			bb_putchar('%');
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			/* Completes "%<modifiers>" with specifier and printfs */
			print_func(dest, *p, filename, ದತ್ತಾಂಶ IF_SELINUX(,scontext));
			ನಿಲ್ಲಿಸು;
		}
	}

	ಬಿಡುಗಡೆ(format);
	ಬಿಡುಗಡೆ(dest);
}
#ಆದರೆ_ಅಂತ್ಯ  /* FEATURE_STAT_FORMAT */

#ಆದರೆ ENABLE_FEATURE_STAT_FILESYSTEM
/* Stat the file system and print what we find.  */
#ಆದರೆ !ENABLE_FEATURE_STAT_FORMAT
#ವ್ಯಾಖ್ಯಾನಿಸು do_statfs(filename, format) do_statfs(filename)
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ bool do_statfs(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *format)
{
	ರಚನೆ statfs statfsbuf;
#ಆದರೆ !ENABLE_FEATURE_STAT_FORMAT
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *format;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_SELINUX
	security_context_t scontext = ಖಾಲಿ;

	ಆದರೆ (option_mask32 & OPT_SELINUX) {
		ಆದರೆ ((option_mask32 & OPT_DEREFERENCE
		     ? lgetfilecon(filename, &scontext)
		     : getfilecon(filename, &scontext)
		    ) < 0
		) {
			bb_simple_perror_ಸಂದೇಶ(filename);
			ಹಿಂತಿರುಗಿ 0;
		}
	}
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (statfs(filename, &statfsbuf) != 0) {
		bb_perror_ಸಂದೇಶ("can't read file system information for '%s'", filename);
		ಹಿಂತಿರುಗಿ 0;
	}

#ಆದರೆ ENABLE_FEATURE_STAT_FORMAT
	ಆದರೆ (format == ಖಾಲಿ) {
# ಆದರೆ !ENABLE_SELINUX
		format = (option_mask32 & OPT_TERSE
			? "%n %i %l %t %s %b %f %a %c %d"
			: "  File: \"%n\"\n"
			  "    ID: %-8i Namelen: %-7l Type: %T\n"
			  "Block size: %-10s\n"
			  "Blocks: Total: %-10b Free: %-10f Available: %a\n"
			  "Inodes: Total: %-10c Free: %d");
# ಇಲ್ಲದಿದ್ದರೆ
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
#ಇಲ್ಲದಿದ್ದರೆ /* !FEATURE_STAT_FORMAT */
	format = (option_mask32 & OPT_TERSE
		? "%s %llx %lu "
		: "  File: \"%s\"\n"
		  "    ID: %-8llx Namelen: %-7lu ");
	ಬರೆ(format,
	       filename,
	       get_f_fsid(&statfsbuf),
	       statfsbuf.f_namelen);

	ಆದರೆ (option_mask32 & OPT_TERSE)
		ಬರೆ("%lx ", (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) statfsbuf.f_type);
	ಇಲ್ಲದಿದ್ದರೆ
		ಬರೆ("Type: %s\n", human_fstype(statfsbuf.f_type));

# ಆದರೆ !ENABLE_SELINUX
	format = (option_mask32 & OPT_TERSE
		? "%lu %llu %llu %llu %llu %llu\n"
		: "Block size: %-10lu\n"
		  "Blocks: Total: %-10llu Free: %-10llu Available: %llu\n"
		  "Inodes: Total: %-10llu Free: %llu\n");
	ಬರೆ(format,
	       (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) statfsbuf.f_bsize,
	       (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ) statfsbuf.f_blocks,
	       (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ) statfsbuf.f_bfree,
	       (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ) statfsbuf.f_bavail,
	       (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ) statfsbuf.f_files,
	       (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ) statfsbuf.f_ffree);
# ಇಲ್ಲದಿದ್ದರೆ
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
	ಬರೆ(format,
		(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) statfsbuf.f_bsize,
		(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ) statfsbuf.f_blocks,
		(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ) statfsbuf.f_bfree,
		(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ) statfsbuf.f_bavail,
		(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ) statfsbuf.f_files,
		(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ) statfsbuf.f_ffree,
		scontext);

	ಆದರೆ (scontext)
		freecon(scontext);
# endif
#ಆದರೆ_ಅಂತ್ಯ  /* FEATURE_STAT_FORMAT */
	ಹಿಂತಿರುಗಿ 1;
}
#ಆದರೆ_ಅಂತ್ಯ  /* FEATURE_STAT_FILESYSTEM */

/* stat the file and print what we find */
#ಆದರೆ !ENABLE_FEATURE_STAT_FORMAT
#ವ್ಯಾಖ್ಯಾನಿಸು do_stat(filename, format) do_stat(filename)
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ bool do_stat(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *format)
{
	ರಚನೆ stat statbuf;
#ಆದರೆ ENABLE_SELINUX
	security_context_t scontext = ಖಾಲಿ;

	ಆದರೆ (option_mask32 & OPT_SELINUX) {
		ಆದರೆ ((option_mask32 & OPT_DEREFERENCE
		     ? lgetfilecon(filename, &scontext)
		     : getfilecon(filename, &scontext)
		    ) < 0
		) {
			bb_simple_perror_ಸಂದೇಶ(filename);
			ಹಿಂತಿರುಗಿ 0;
		}
	}
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ ((option_mask32 & OPT_DEREFERENCE ? stat : lstat) (filename, &statbuf) != 0) {
		bb_perror_ಸಂದೇಶ("can't stat '%s'", filename);
		ಹಿಂತಿರುಗಿ 0;
	}

#ಆದರೆ ENABLE_FEATURE_STAT_FORMAT
	ಆದರೆ (format == ಖಾಲಿ) {
# ಆದರೆ !ENABLE_SELINUX
		ಆದರೆ (option_mask32 & OPT_TERSE) {
			format = "%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %o";
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (S_ISBLK(statbuf.st_ಬಹುಲಕ) || S_ISCHR(statbuf.st_ಬಹುಲಕ)) {
				format =
					"  File: %N\n"
					"  Size: %-10s\tBlocks: %-10b IO Block: %-6o %F\n"
					"Device: %Dh/%dd\tInode: %-10i  Links: %-5h"
					" Device type: %t,%T\n"
					"Access: (%04a/%10.10A)  Uid: (%5u/%8U)   Gid: (%5g/%8G)\n"
					"Access: %x\n" "Modify: %y\n" "Change: %z";
			} ಇಲ್ಲದಿದ್ದರೆ {
				format =
					"  File: %N\n"
					"  Size: %-10s\tBlocks: %-10b IO Block: %-6o %F\n"
					"Device: %Dh/%dd\tInode: %-10i  Links: %h\n"
					"Access: (%04a/%10.10A)  Uid: (%5u/%8U)   Gid: (%5g/%8G)\n"
					"Access: %x\n" "Modify: %y\n" "Change: %z";
			}
		}
# ಇಲ್ಲದಿದ್ದರೆ
		ಆದರೆ (option_mask32 & OPT_TERSE) {
			format = (option_mask32 & OPT_SELINUX ?
				"%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %o %C\n"
				:
				"%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %o\n"
				);
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (S_ISBLK(statbuf.st_ಬಹುಲಕ) || S_ISCHR(statbuf.st_ಬಹುಲಕ)) {
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
			} ಇಲ್ಲದಿದ್ದರೆ {
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
#ಇಲ್ಲದಿದ್ದರೆ	/* FEATURE_STAT_FORMAT */
	ಆದರೆ (option_mask32 & OPT_TERSE) {
		ಬರೆ("%s %llu %llu %lx %lu %lu %llx %llu %lu %lx %lx %lu %lu %lu %lu"
		       IF_NOT_SELINUX("\n"),
		       filename,
		       (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ) statbuf.st_ಗಾತ್ರ,
		       (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ) statbuf.st_ಖಂಡಗಳು,
		       (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) statbuf.st_ಬಹುಲಕ,
		       (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) statbuf.st_uid,
		       (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) statbuf.st_gid,
		       (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ) statbuf.st_dev,
		       (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ) statbuf.st_ino,
		       (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) statbuf.st_nlink,
		       (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) major(statbuf.st_rdev),
		       (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) minor(statbuf.st_rdev),
		       (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) statbuf.st_atime,
		       (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) statbuf.st_mtime,
		       (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) statbuf.st_ctime,
		       (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) statbuf.st_blksize
		);
# ಆದರೆ ENABLE_SELINUX
		ಆದರೆ (option_mask32 & OPT_SELINUX)
			ಬರೆ(" %s\n", scontext);
		ಇಲ್ಲದಿದ್ದರೆ
			bb_putchar('\n');
# endif
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಅಕ್ಷರ modestr[12];
		ಅಕ್ಷರ *linkname = ಖಾಲಿ;
		ರಚನೆ passwd *pw_ent;
		ರಚನೆ ಗುಂಪು *gw_ent;

		gw_ent = getgrgid(statbuf.st_gid);
		pw_ent = getpwuid(statbuf.st_uid);

		ಆದರೆ (S_ISLNK(statbuf.st_ಬಹುಲಕ))
			linkname = xmalloc_readlink_or_warn(filename);
		ಆದರೆ (linkname) {
			ಬರೆ("  File: '%s' -> '%s'\n", filename, linkname);
			ಬಿಡುಗಡೆ(linkname);
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಬರೆ("  File: '%s'\n", filename);
		}

		ಬರೆ("  Size: %-10llu\tBlocks: %-10llu IO Block: %-6lu %s\n"
		       "Device: %llxh/%llud\tInode: %-10llu  Links: %-5lu",
		       (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ) statbuf.st_ಗಾತ್ರ,
		       (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ) statbuf.st_ಖಂಡಗಳು,
		       (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) statbuf.st_blksize,
		       file_type(&statbuf),
		       (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ) statbuf.st_dev,
		       (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ) statbuf.st_dev,
		       (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ) statbuf.st_ino,
		       (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) statbuf.st_nlink);
		ಆದರೆ (S_ISBLK(statbuf.st_ಬಹುಲಕ) || S_ISCHR(statbuf.st_ಬಹುಲಕ))
			ಬರೆ(" Device type: %lx,%lx\n",
			       (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) major(statbuf.st_rdev),
			       (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) minor(statbuf.st_rdev));
		ಇಲ್ಲದಿದ್ದರೆ
			bb_putchar('\n');
		ಬರೆ("Access: (%04lo/%10.10s)  Uid: (%5lu/%8s)   Gid: (%5lu/%8s)\n",
		       (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) (statbuf.st_ಬಹುಲಕ & (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)),
		       bb_ಬಹುಲಕ_string(modestr, statbuf.st_ಬಹುಲಕ),
		       (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) statbuf.st_uid,
		       (pw_ent != ಖಾಲಿ) ? pw_ent->pw_name : "UNKNOWN",
		       (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) statbuf.st_gid,
		       (gw_ent != ಖಾಲಿ) ? gw_ent->gr_name : "UNKNOWN");
# ಆದರೆ ENABLE_SELINUX
		ಆದರೆ (option_mask32 & OPT_SELINUX)
			ಬರೆ("   S_Context: %s\n", scontext);
# endif
		ಬರೆ("Access: %s\n", human_ಸಮಯ(&statbuf.st_atim));
		ಬರೆ("Modify: %s\n", human_ಸಮಯ(&statbuf.st_mtim));
		ಬರೆ("Change: %s\n", human_ಸಮಯ(&statbuf.st_ctim));
	}
#ಆದರೆ_ಅಂತ್ಯ  /* FEATURE_STAT_FORMAT */
	ಹಿಂತಿರುಗಿ 1;
}

ಪೂರ್ಣಾಂಕ stat_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ stat_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	IF_FEATURE_STAT_FORMAT(ಅಕ್ಷರ *format = ಖಾಲಿ;)
	ಪೂರ್ಣಾಂಕ i;
	ಪೂರ್ಣಾಂಕ ok;
	statfunc_ptr statfunc = do_stat;
#ಆದರೆ ENABLE_FEATURE_STAT_FILESYSTEM || ENABLE_SELINUX
	ಚಿಹ್ನೆರಹಿತ opts;

	opts =
#ಆದರೆ_ಅಂತ್ಯ
	getopt32(argv, "^"
		"tL"
		IF_FEATURE_STAT_FILESYSTEM("f")
		IF_SELINUX("Z")
		IF_FEATURE_STAT_FORMAT("c:")
		"\0" "-1" /* min one arg */
		IF_FEATURE_STAT_FORMAT(,&format)
	);
#ಆದರೆ ENABLE_FEATURE_STAT_FILESYSTEM
	ಆದರೆ (opts & OPT_FILESYS) /* -f */
		statfunc = do_statfs;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_SELINUX
	ಆದರೆ (opts & OPT_SELINUX) {
		selinux_or_die();
	}
#ಆದರೆ_ಅಂತ್ಯ
	ok = 1;
	argv += optind;
	ಗಾಗಿ (i = 0; argv[i]; ++i)
		ok &= statfunc(argv[i] IF_FEATURE_STAT_FORMAT(, format));

	ಹಿಂತಿರುಗಿ (ok ? EXIT_SUCCESS : EXIT_FAILURE);
}
