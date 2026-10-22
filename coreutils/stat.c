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

#అన్తర్భావ "libbb.h"
#అన్తర్భావ "common_bufsiz.h"

గణన {
	OPT_TERSE       = (1 << 0),
	OPT_DEREFERENCE = (1 << 1),
	OPT_FILESYS     = (1 << 2) * ENABLE_FEATURE_STAT_FILESYSTEM,
	OPT_SELINUX     = (1 << (2+ENABLE_FEATURE_STAT_FILESYSTEM)) * ENABLE_SELINUX,
};

#ఒకవేళ ENABLE_FEATURE_STAT_FORMAT
రకనిర్వచనం bool (*statfunc_ptr)(స్థిరరాశి అక్షరం *, స్థిరరాశి అక్షరం *);
#లేకపోతే
రకనిర్వచనం bool (*statfunc_ptr)(స్థిరరాశి అక్షరం *);
#అయితే_ముగింపు

స్థిర స్థిరరాశి అక్షరం *file_type(స్థిరరాశి నిర్మాణం stat *st)
{
	/* See POSIX 1003.1-2001 XCU Table 4-8 lines 17093-17107
	 * for some of these formats.
	 * To keep diagnostics grammatical in English, the
	 * returned string must start with a consonant.
	 */
	ఒకవేళ (S_ISREG(st->st_బహుళకం))  తిరిగి st->st_కొలత == 0 ? "regular empty file" : "regular file";
	ఒకవేళ (S_ISDIR(st->st_బహుళకం))  తిరిగి "directory";
	ఒకవేళ (S_ISBLK(st->st_బహుళకం))  తిరిగి "block special file";
	ఒకవేళ (S_ISCHR(st->st_బహుళకం))  తిరిగి "character special file";
	ఒకవేళ (S_ISFIFO(st->st_బహుళకం)) తిరిగి "fifo";
	ఒకవేళ (S_ISLNK(st->st_బహుళకం))  తిరిగి "symbolic link";
	ఒకవేళ (S_ISSOCK(st->st_బహుళకం)) తిరిగి "socket";
#నిర్వచితమైతే S_TYPEISMQ
	ఒకవేళ (S_TYPEISMQ(st))        తిరిగి "message queue";
#అయితే_ముగింపు
#నిర్వచితమైతే S_TYPEISSEM
	ఒకవేళ (S_TYPEISSEM(st))       తిరిగి "semaphore";
#అయితే_ముగింపు
#నిర్వచితమైతే S_TYPEISSHM
	ఒకవేళ (S_TYPEISSHM(st))       తిరిగి "shared memory object";
#అయితే_ముగింపు
#నిర్వచితమైతే S_TYPEISTMO
	ఒకవేళ (S_TYPEISTMO(st))       తిరిగి "typed memory object";
#అయితే_ముగింపు
	తిరిగి "weird file";
}

స్థిర స్థిరరాశి అక్షరం *human_సమయం(నిర్మాణం timespec *ts)
{
	అక్షరం fmt[పరిమాణం("%Y-%m-%d %H:%M:%S.123456789 %z") + /*paranoia*/ 8];

	/* coreutils 6.3 compat */
#నిర్వచించు buf bb_common_bufsiz1
	అమరిక_common_bufsiz();

	sprintf(stpcpy(fmt, "%Y-%m-%d %H:%M:%S"), ".%09u %%z", (గుర్తులేని)ts->tv_nsec);
	strftime(buf, COMMON_BUFSIZE, fmt, localtime(&ts->tv_sec));
	తిరిగి buf;
#నిర్వచనం_రద్దు buf
}

#ఒకవేళ ENABLE_FEATURE_STAT_FILESYSTEM
#నిర్వచించు FS_TYPE_LIST \
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
స్థిర స్థిరరాశి అక్షరం *human_fstype(uint32_t f_type)
{
# define FS_TYPE(type, name) type,
	స్థిర స్థిరరాశి uint32_t fstype[] ALIGN4 = {
		FS_TYPE_LIST
	};
# undef FS_TYPE
# define FS_TYPE(type, name) name"\0"
	స్థిర స్థిరరాశి అక్షరం humanname[] ALIGN1 =
		FS_TYPE_LIST
		"UNKNOWN";
# undef FS_TYPE
	పూర్ణాంకం i;

	కొరకు (i = 0; i < ARRAY_కొలత(fstype); ++i)
		ఒకవేళ (fstype[i] == f_type)
			ఆపు;
	తిరిగి nth_string(humanname, i);
}

/* "man statfs" says that statfsbuf->f_fsid is a mess */
/* coreutils treats it as an array of ints, most significant first */
స్థిర గుర్తులేని దీర్ఘ దీర్ఘ get_f_fsid(స్థిరరాశి నిర్మాణం statfs *statfsbuf)
{
	స్థిరరాశి గుర్తులేని *p = (స్థిరరాశి శూన్య*) &statfsbuf->f_fsid;
	గుర్తులేని sz = పరిమాణం(statfsbuf->f_fsid) / పరిమాణం(గుర్తులేని);
	గుర్తులేని దీర్ఘ దీర్ఘ r = 0;

	చేయి
		r = (r << (పరిమాణం(గుర్తులేని)*8)) | *p++;
	అయితే (--sz > 0);
	తిరిగి r;
}
#అయితే_ముగింపు  /* FEATURE_STAT_FILESYSTEM */

#ఒకవేళ ENABLE_FEATURE_STAT_FORMAT
స్థిర శూన్య strcatc(అక్షరం *str, అక్షరం c)
{
	పూర్ణాంకం len = అక్షరశ్రేణి_పొడవు(str);
	str[len++] = c;
	str[len] = '\0';
}

స్థిర శూన్య printfs(అక్షరం *pformat, స్థిరరాశి అక్షరం *సందేశం)
{
	strcatc(pformat, 's');
	రాయి(pformat, సందేశం);
}

#ఒకవేళ ENABLE_FEATURE_STAT_FILESYSTEM
/* print statfs info */
స్థిర శూన్య FAST_FUNC print_statfs(అక్షరం *pformat, స్థిరరాశి అక్షరం m,
		స్థిరరాశి అక్షరం *స్థిరరాశి filename, స్థిరరాశి శూన్య *దత్తాంశం
		IF_SELINUX(, security_context_t scontext))
{
	స్థిరరాశి నిర్మాణం statfs *statfsbuf = దత్తాంశం;
	ఒకవేళ (m == 'n') {
		printfs(pformat, filename);
	} లేకపోతే ఒకవేళ (m == 'i') {
		strcat(pformat, "llx");
		రాయి(pformat, get_f_fsid(statfsbuf));
	} లేకపోతే ఒకవేళ (m == 'l') {
		strcat(pformat, "lu");
		రాయి(pformat, (గుర్తులేని దీర్ఘ) statfsbuf->f_namelen);
	} లేకపోతే ఒకవేళ (m == 't') {
		strcat(pformat, "lx");
		రాయి(pformat, (గుర్తులేని దీర్ఘ) statfsbuf->f_type); /* no equiv */
	} లేకపోతే ఒకవేళ (m == 'T') {
		printfs(pformat, human_fstype(statfsbuf->f_type));
	} లేకపోతే ఒకవేళ (m == 'b') {
		strcat(pformat, "llu");
		రాయి(pformat, (గుర్తులేని దీర్ఘ దీర్ఘ) statfsbuf->f_blocks);
	} లేకపోతే ఒకవేళ (m == 'f') {
		strcat(pformat, "llu");
		రాయి(pformat, (గుర్తులేని దీర్ఘ దీర్ఘ) statfsbuf->f_bfree);
	} లేకపోతే ఒకవేళ (m == 'a') {
		strcat(pformat, "llu");
		రాయి(pformat, (గుర్తులేని దీర్ఘ దీర్ఘ) statfsbuf->f_bavail);
	} లేకపోతే ఒకవేళ (m == 's' || m == 'S') {
		strcat(pformat, "lu");
		రాయి(pformat, (గుర్తులేని దీర్ఘ) statfsbuf->f_bsize);
	} లేకపోతే ఒకవేళ (m == 'c') {
		strcat(pformat, "llu");
		రాయి(pformat, (గుర్తులేని దీర్ఘ దీర్ఘ) statfsbuf->f_files);
	} లేకపోతే ఒకవేళ (m == 'd') {
		strcat(pformat, "llu");
		రాయి(pformat, (గుర్తులేని దీర్ఘ దీర్ఘ) statfsbuf->f_ffree);
# ఒకవేళ ENABLE_SELINUX
	} లేకపోతే ఒకవేళ (m == 'C' && (option_mask32 & OPT_SELINUX)) {
		printfs(pformat, scontext);
# endif
	} లేకపోతే {
		strcatc(pformat, 'c');
		రాయి(pformat, m);
	}
}
#అయితే_ముగింపు

/* print stat info */
స్థిర శూన్య FAST_FUNC print_stat(అక్షరం *pformat, స్థిరరాశి అక్షరం m,
		స్థిరరాశి అక్షరం *స్థిరరాశి filename, స్థిరరాశి శూన్య *దత్తాంశం
		IF_SELINUX(, security_context_t scontext))
{
#నిర్వచించు TYPE_SIGNED(t) (! ((t) 0 < (t) -1))
	నిర్మాణం stat *statbuf = (నిర్మాణం stat *) దత్తాంశం;
	నిర్మాణం passwd *pw_ent;
	నిర్మాణం సమూహం *gw_ent;

	ఒకవేళ (m == 'n') {
		printfs(pformat, filename);
	} లేకపోతే ఒకవేళ (m == 'N') {
		strcatc(pformat, 's');
		ఒకవేళ (S_ISLNK(statbuf->st_బహుళకం)) {
			అక్షరం *linkname = xmalloc_readlink_or_warn(filename);
			ఒకవేళ (linkname == ఖాళీ)
				తిరిగి;
			రాయి("'%s' -> '%s'", filename, linkname);
			విడుదల(linkname);
		} లేకపోతే {
			రాయి(pformat, filename);
		}
	} లేకపోతే ఒకవేళ (m == 'd') {
		strcat(pformat, "llu");
		రాయి(pformat, (గుర్తులేని దీర్ఘ దీర్ఘ) statbuf->st_dev);
	} లేకపోతే ఒకవేళ (m == 'D') {
		strcat(pformat, "llx");
		రాయి(pformat, (గుర్తులేని దీర్ఘ దీర్ఘ) statbuf->st_dev);
	} లేకపోతే ఒకవేళ (m == 'i') {
		strcat(pformat, "llu");
		రాయి(pformat, (గుర్తులేని దీర్ఘ దీర్ఘ) statbuf->st_ino);
	} లేకపోతే ఒకవేళ (m == 'a') {
		strcat(pformat, "lo");
		రాయి(pformat, (గుర్తులేని దీర్ఘ) (statbuf->st_బహుళకం & (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)));
	} లేకపోతే ఒకవేళ (m == 'A') {
		అక్షరం modestr[12];
		printfs(pformat, bb_బహుళకం_string(modestr, statbuf->st_బహుళకం));
	} లేకపోతే ఒకవేళ (m == 'f') {
		strcat(pformat, "lx");
		రాయి(pformat, (గుర్తులేని దీర్ఘ) statbuf->st_బహుళకం);
	} లేకపోతే ఒకవేళ (m == 'F') {
		printfs(pformat, file_type(statbuf));
	} లేకపోతే ఒకవేళ (m == 'h') {
		strcat(pformat, "lu");
		రాయి(pformat, (గుర్తులేని దీర్ఘ) statbuf->st_nlink);
	} లేకపోతే ఒకవేళ (m == 'u') {
		strcat(pformat, "lu");
		రాయి(pformat, (గుర్తులేని దీర్ఘ) statbuf->st_uid);
	} లేకపోతే ఒకవేళ (m == 'U') {
		pw_ent = getpwuid(statbuf->st_uid);
		printfs(pformat, (pw_ent != ఖాళీ) ? pw_ent->pw_name : "UNKNOWN");
	} లేకపోతే ఒకవేళ (m == 'g') {
		strcat(pformat, "lu");
		రాయి(pformat, (గుర్తులేని దీర్ఘ) statbuf->st_gid);
	} లేకపోతే ఒకవేళ (m == 'G') {
		gw_ent = getgrgid(statbuf->st_gid);
		printfs(pformat, (gw_ent != ఖాళీ) ? gw_ent->gr_name : "UNKNOWN");
	} లేకపోతే ఒకవేళ (m == 't') {
		strcat(pformat, "lx");
		రాయి(pformat, (గుర్తులేని దీర్ఘ) major(statbuf->st_rdev));
	} లేకపోతే ఒకవేళ (m == 'T') {
		strcat(pformat, "lx");
		రాయి(pformat, (గుర్తులేని దీర్ఘ) minor(statbuf->st_rdev));
	} లేకపోతే ఒకవేళ (m == 's') {
		strcat(pformat, "llu");
		రాయి(pformat, (గుర్తులేని దీర్ఘ దీర్ఘ) statbuf->st_కొలత);
	} లేకపోతే ఒకవేళ (m == 'B') {
		strcat(pformat, "lu");
		రాయి(pformat, (గుర్తులేని దీర్ఘ) 512); //ST_NBLOCKSIZE
	} లేకపోతే ఒకవేళ (m == 'b') {
		strcat(pformat, "llu");
		రాయి(pformat, (గుర్తులేని దీర్ఘ దీర్ఘ) statbuf->st_ఖండాలు);
	} లేకపోతే ఒకవేళ (m == 'o') {
		strcat(pformat, "lu");
		రాయి(pformat, (గుర్తులేని దీర్ఘ) statbuf->st_blksize);
	} లేకపోతే ఒకవేళ (m == 'x') {
		printfs(pformat, human_సమయం(&statbuf->st_atim));
	} లేకపోతే ఒకవేళ (m == 'X') {
		strcat(pformat, TYPE_SIGNED(సమయం_t) ? "ld" : "lu");
		/* note: (unsigned long) would be wrong:
		 * imagine (unsigned long64)int32 */
		రాయి(pformat, (దీర్ఘ) statbuf->st_atime);
	} లేకపోతే ఒకవేళ (m == 'y') {
		printfs(pformat, human_సమయం(&statbuf->st_mtim));
	} లేకపోతే ఒకవేళ (m == 'Y') {
		strcat(pformat, TYPE_SIGNED(సమయం_t) ? "ld" : "lu");
		రాయి(pformat, (దీర్ఘ) statbuf->st_mtime);
	} లేకపోతే ఒకవేళ (m == 'z') {
		printfs(pformat, human_సమయం(&statbuf->st_ctim));
	} లేకపోతే ఒకవేళ (m == 'Z') {
		strcat(pformat, TYPE_SIGNED(సమయం_t) ? "ld" : "lu");
		రాయి(pformat, (దీర్ఘ) statbuf->st_ctime);
# ఒకవేళ ENABLE_SELINUX
	} లేకపోతే ఒకవేళ (m == 'C' && (option_mask32 & OPT_SELINUX)) {
		printfs(pformat, scontext);
# endif
	} లేకపోతే {
		strcatc(pformat, 'c');
		రాయి(pformat, m);
	}
}

స్థిర శూన్య print_it(స్థిరరాశి అక్షరం *masterformat,
		స్థిరరాశి అక్షరం *filename,
		శూన్య FAST_FUNC (*print_func)(అక్షరం*, అక్షరం, స్థిరరాశి అక్షరం*, స్థిరరాశి శూన్య* IF_SELINUX(, security_context_t scontext)),
		స్థిరరాశి శూన్య *దత్తాంశం
		IF_SELINUX(, security_context_t scontext))
{
	/* Create a working copy of the format string */
	అక్షరం *format = xstrdup(masterformat);
	/* Add 2 to accommodate our conversion of the stat '%s' format string
	 * to the printf '%llu' one.  */
	అక్షరం *dest = xmalloc(అక్షరశ్రేణి_పొడవు(format) + 2 + 1);
	అక్షరం *b;

	b = format;
	అయితే (b) {
		/* Each iteration finds next %spec,
		 * prints preceding string and handles found %spec
		 */
		కొలత_t len;
		అక్షరం *p = strchr(b, '%');
		ఒకవేళ (!p) {
			/* coreutils 6.3 always prints newline at the end */
			/*fputs(b, stdout);*/
			puts(b);
			ఆపు;
		}

		/* dest = "%<modifiers>" */
		len = 1 + strspn(p + 1, "#-+.I 0123456789");
		జ్ఞాపకం_నకలు(dest, p, len);
		dest[len] = '\0';

		/* print preceding string */
		*p = '\0';
		fputs_stdout(b);

		p += len;
		b = p + 1;
		ఎంపిక (*p) {
		స్థితి '\0':
			b = ఖాళీ;
			/* fall through */
		స్థితి '%':
			bb_putchar('%');
			ఆపు;
		అప్రమేయ:
			/* Completes "%<modifiers>" with specifier and printfs */
			print_func(dest, *p, filename, దత్తాంశం IF_SELINUX(,scontext));
			ఆపు;
		}
	}

	విడుదల(format);
	విడుదల(dest);
}
#అయితే_ముగింపు  /* FEATURE_STAT_FORMAT */

#ఒకవేళ ENABLE_FEATURE_STAT_FILESYSTEM
/* Stat the file system and print what we find.  */
#ఒకవేళ !ENABLE_FEATURE_STAT_FORMAT
#నిర్వచించు do_statfs(filename, format) do_statfs(filename)
#అయితే_ముగింపు
స్థిర bool do_statfs(స్థిరరాశి అక్షరం *filename, స్థిరరాశి అక్షరం *format)
{
	నిర్మాణం statfs statfsbuf;
#ఒకవేళ !ENABLE_FEATURE_STAT_FORMAT
	స్థిరరాశి అక్షరం *format;
#అయితే_ముగింపు
#ఒకవేళ ENABLE_SELINUX
	security_context_t scontext = ఖాళీ;

	ఒకవేళ (option_mask32 & OPT_SELINUX) {
		ఒకవేళ ((option_mask32 & OPT_DEREFERENCE
		     ? lgetfilecon(filename, &scontext)
		     : getfilecon(filename, &scontext)
		    ) < 0
		) {
			bb_simple_perror_సందేశం(filename);
			తిరిగి 0;
		}
	}
#అయితే_ముగింపు
	ఒకవేళ (statfs(filename, &statfsbuf) != 0) {
		bb_perror_సందేశం("can't read file system information for '%s'", filename);
		తిరిగి 0;
	}

#ఒకవేళ ENABLE_FEATURE_STAT_FORMAT
	ఒకవేళ (format == ఖాళీ) {
# ఒకవేళ !ENABLE_SELINUX
		format = (option_mask32 & OPT_TERSE
			? "%n %i %l %t %s %b %f %a %c %d"
			: "  File: \"%n\"\n"
			  "    ID: %-8i Namelen: %-7l Type: %T\n"
			  "Block size: %-10s\n"
			  "Blocks: Total: %-10b Free: %-10f Available: %a\n"
			  "Inodes: Total: %-10c Free: %d");
# లేకపోతే
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
#లేకపోతే /* !FEATURE_STAT_FORMAT */
	format = (option_mask32 & OPT_TERSE
		? "%s %llx %lu "
		: "  File: \"%s\"\n"
		  "    ID: %-8llx Namelen: %-7lu ");
	రాయి(format,
	       filename,
	       get_f_fsid(&statfsbuf),
	       statfsbuf.f_namelen);

	ఒకవేళ (option_mask32 & OPT_TERSE)
		రాయి("%lx ", (గుర్తులేని దీర్ఘ) statfsbuf.f_type);
	లేకపోతే
		రాయి("Type: %s\n", human_fstype(statfsbuf.f_type));

# ఒకవేళ !ENABLE_SELINUX
	format = (option_mask32 & OPT_TERSE
		? "%lu %llu %llu %llu %llu %llu\n"
		: "Block size: %-10lu\n"
		  "Blocks: Total: %-10llu Free: %-10llu Available: %llu\n"
		  "Inodes: Total: %-10llu Free: %llu\n");
	రాయి(format,
	       (గుర్తులేని దీర్ఘ) statfsbuf.f_bsize,
	       (గుర్తులేని దీర్ఘ దీర్ఘ) statfsbuf.f_blocks,
	       (గుర్తులేని దీర్ఘ దీర్ఘ) statfsbuf.f_bfree,
	       (గుర్తులేని దీర్ఘ దీర్ఘ) statfsbuf.f_bavail,
	       (గుర్తులేని దీర్ఘ దీర్ఘ) statfsbuf.f_files,
	       (గుర్తులేని దీర్ఘ దీర్ఘ) statfsbuf.f_ffree);
# లేకపోతే
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
	రాయి(format,
		(గుర్తులేని దీర్ఘ) statfsbuf.f_bsize,
		(గుర్తులేని దీర్ఘ దీర్ఘ) statfsbuf.f_blocks,
		(గుర్తులేని దీర్ఘ దీర్ఘ) statfsbuf.f_bfree,
		(గుర్తులేని దీర్ఘ దీర్ఘ) statfsbuf.f_bavail,
		(గుర్తులేని దీర్ఘ దీర్ఘ) statfsbuf.f_files,
		(గుర్తులేని దీర్ఘ దీర్ఘ) statfsbuf.f_ffree,
		scontext);

	ఒకవేళ (scontext)
		freecon(scontext);
# endif
#అయితే_ముగింపు  /* FEATURE_STAT_FORMAT */
	తిరిగి 1;
}
#అయితే_ముగింపు  /* FEATURE_STAT_FILESYSTEM */

/* stat the file and print what we find */
#ఒకవేళ !ENABLE_FEATURE_STAT_FORMAT
#నిర్వచించు do_stat(filename, format) do_stat(filename)
#అయితే_ముగింపు
స్థిర bool do_stat(స్థిరరాశి అక్షరం *filename, స్థిరరాశి అక్షరం *format)
{
	నిర్మాణం stat statbuf;
#ఒకవేళ ENABLE_SELINUX
	security_context_t scontext = ఖాళీ;

	ఒకవేళ (option_mask32 & OPT_SELINUX) {
		ఒకవేళ ((option_mask32 & OPT_DEREFERENCE
		     ? lgetfilecon(filename, &scontext)
		     : getfilecon(filename, &scontext)
		    ) < 0
		) {
			bb_simple_perror_సందేశం(filename);
			తిరిగి 0;
		}
	}
#అయితే_ముగింపు
	ఒకవేళ ((option_mask32 & OPT_DEREFERENCE ? stat : lstat) (filename, &statbuf) != 0) {
		bb_perror_సందేశం("can't stat '%s'", filename);
		తిరిగి 0;
	}

#ఒకవేళ ENABLE_FEATURE_STAT_FORMAT
	ఒకవేళ (format == ఖాళీ) {
# ఒకవేళ !ENABLE_SELINUX
		ఒకవేళ (option_mask32 & OPT_TERSE) {
			format = "%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %o";
		} లేకపోతే {
			ఒకవేళ (S_ISBLK(statbuf.st_బహుళకం) || S_ISCHR(statbuf.st_బహుళకం)) {
				format =
					"  File: %N\n"
					"  Size: %-10s\tBlocks: %-10b IO Block: %-6o %F\n"
					"Device: %Dh/%dd\tInode: %-10i  Links: %-5h"
					" Device type: %t,%T\n"
					"Access: (%04a/%10.10A)  Uid: (%5u/%8U)   Gid: (%5g/%8G)\n"
					"Access: %x\n" "Modify: %y\n" "Change: %z";
			} లేకపోతే {
				format =
					"  File: %N\n"
					"  Size: %-10s\tBlocks: %-10b IO Block: %-6o %F\n"
					"Device: %Dh/%dd\tInode: %-10i  Links: %h\n"
					"Access: (%04a/%10.10A)  Uid: (%5u/%8U)   Gid: (%5g/%8G)\n"
					"Access: %x\n" "Modify: %y\n" "Change: %z";
			}
		}
# లేకపోతే
		ఒకవేళ (option_mask32 & OPT_TERSE) {
			format = (option_mask32 & OPT_SELINUX ?
				"%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %o %C\n"
				:
				"%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %o\n"
				);
		} లేకపోతే {
			ఒకవేళ (S_ISBLK(statbuf.st_బహుళకం) || S_ISCHR(statbuf.st_బహుళకం)) {
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
			} లేకపోతే {
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
#లేకపోతే	/* FEATURE_STAT_FORMAT */
	ఒకవేళ (option_mask32 & OPT_TERSE) {
		రాయి("%s %llu %llu %lx %lu %lu %llx %llu %lu %lx %lx %lu %lu %lu %lu"
		       IF_NOT_SELINUX("\n"),
		       filename,
		       (గుర్తులేని దీర్ఘ దీర్ఘ) statbuf.st_కొలత,
		       (గుర్తులేని దీర్ఘ దీర్ఘ) statbuf.st_ఖండాలు,
		       (గుర్తులేని దీర్ఘ) statbuf.st_బహుళకం,
		       (గుర్తులేని దీర్ఘ) statbuf.st_uid,
		       (గుర్తులేని దీర్ఘ) statbuf.st_gid,
		       (గుర్తులేని దీర్ఘ దీర్ఘ) statbuf.st_dev,
		       (గుర్తులేని దీర్ఘ దీర్ఘ) statbuf.st_ino,
		       (గుర్తులేని దీర్ఘ) statbuf.st_nlink,
		       (గుర్తులేని దీర్ఘ) major(statbuf.st_rdev),
		       (గుర్తులేని దీర్ఘ) minor(statbuf.st_rdev),
		       (గుర్తులేని దీర్ఘ) statbuf.st_atime,
		       (గుర్తులేని దీర్ఘ) statbuf.st_mtime,
		       (గుర్తులేని దీర్ఘ) statbuf.st_ctime,
		       (గుర్తులేని దీర్ఘ) statbuf.st_blksize
		);
# ఒకవేళ ENABLE_SELINUX
		ఒకవేళ (option_mask32 & OPT_SELINUX)
			రాయి(" %s\n", scontext);
		లేకపోతే
			bb_putchar('\n');
# endif
	} లేకపోతే {
		అక్షరం modestr[12];
		అక్షరం *linkname = ఖాళీ;
		నిర్మాణం passwd *pw_ent;
		నిర్మాణం సమూహం *gw_ent;

		gw_ent = getgrgid(statbuf.st_gid);
		pw_ent = getpwuid(statbuf.st_uid);

		ఒకవేళ (S_ISLNK(statbuf.st_బహుళకం))
			linkname = xmalloc_readlink_or_warn(filename);
		ఒకవేళ (linkname) {
			రాయి("  File: '%s' -> '%s'\n", filename, linkname);
			విడుదల(linkname);
		} లేకపోతే {
			రాయి("  File: '%s'\n", filename);
		}

		రాయి("  Size: %-10llu\tBlocks: %-10llu IO Block: %-6lu %s\n"
		       "Device: %llxh/%llud\tInode: %-10llu  Links: %-5lu",
		       (గుర్తులేని దీర్ఘ దీర్ఘ) statbuf.st_కొలత,
		       (గుర్తులేని దీర్ఘ దీర్ఘ) statbuf.st_ఖండాలు,
		       (గుర్తులేని దీర్ఘ) statbuf.st_blksize,
		       file_type(&statbuf),
		       (గుర్తులేని దీర్ఘ దీర్ఘ) statbuf.st_dev,
		       (గుర్తులేని దీర్ఘ దీర్ఘ) statbuf.st_dev,
		       (గుర్తులేని దీర్ఘ దీర్ఘ) statbuf.st_ino,
		       (గుర్తులేని దీర్ఘ) statbuf.st_nlink);
		ఒకవేళ (S_ISBLK(statbuf.st_బహుళకం) || S_ISCHR(statbuf.st_బహుళకం))
			రాయి(" Device type: %lx,%lx\n",
			       (గుర్తులేని దీర్ఘ) major(statbuf.st_rdev),
			       (గుర్తులేని దీర్ఘ) minor(statbuf.st_rdev));
		లేకపోతే
			bb_putchar('\n');
		రాయి("Access: (%04lo/%10.10s)  Uid: (%5lu/%8s)   Gid: (%5lu/%8s)\n",
		       (గుర్తులేని దీర్ఘ) (statbuf.st_బహుళకం & (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)),
		       bb_బహుళకం_string(modestr, statbuf.st_బహుళకం),
		       (గుర్తులేని దీర్ఘ) statbuf.st_uid,
		       (pw_ent != ఖాళీ) ? pw_ent->pw_name : "UNKNOWN",
		       (గుర్తులేని దీర్ఘ) statbuf.st_gid,
		       (gw_ent != ఖాళీ) ? gw_ent->gr_name : "UNKNOWN");
# ఒకవేళ ENABLE_SELINUX
		ఒకవేళ (option_mask32 & OPT_SELINUX)
			రాయి("   S_Context: %s\n", scontext);
# endif
		రాయి("Access: %s\n", human_సమయం(&statbuf.st_atim));
		రాయి("Modify: %s\n", human_సమయం(&statbuf.st_mtim));
		రాయి("Change: %s\n", human_సమయం(&statbuf.st_ctim));
	}
#అయితే_ముగింపు  /* FEATURE_STAT_FORMAT */
	తిరిగి 1;
}

పూర్ణాంకం stat_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం stat_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	IF_FEATURE_STAT_FORMAT(అక్షరం *format = ఖాళీ;)
	పూర్ణాంకం i;
	పూర్ణాంకం ok;
	statfunc_ptr statfunc = do_stat;
#ఒకవేళ ENABLE_FEATURE_STAT_FILESYSTEM || ENABLE_SELINUX
	గుర్తులేని opts;

	opts =
#అయితే_ముగింపు
	getopt32(argv, "^"
		"tL"
		IF_FEATURE_STAT_FILESYSTEM("f")
		IF_SELINUX("Z")
		IF_FEATURE_STAT_FORMAT("c:")
		"\0" "-1" /* min one arg */
		IF_FEATURE_STAT_FORMAT(,&format)
	);
#ఒకవేళ ENABLE_FEATURE_STAT_FILESYSTEM
	ఒకవేళ (opts & OPT_FILESYS) /* -f */
		statfunc = do_statfs;
#అయితే_ముగింపు
#ఒకవేళ ENABLE_SELINUX
	ఒకవేళ (opts & OPT_SELINUX) {
		selinux_or_die();
	}
#అయితే_ముగింపు
	ok = 1;
	argv += optind;
	కొరకు (i = 0; argv[i]; ++i)
		ok &= statfunc(argv[i] IF_FEATURE_STAT_FORMAT(, format));

	తిరిగి (ok ? EXIT_SUCCESS : EXIT_FAILURE);
}
