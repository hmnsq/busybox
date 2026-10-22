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

#അന്തര്ഭാവ "libbb.h"
#അന്തര്ഭാവ "common_bufsiz.h"

എണ്ണൽ {
	OPT_TERSE       = (1 << 0),
	OPT_DEREFERENCE = (1 << 1),
	OPT_FILESYS     = (1 << 2) * ENABLE_FEATURE_STAT_FILESYSTEM,
	OPT_SELINUX     = (1 << (2+ENABLE_FEATURE_STAT_FILESYSTEM)) * ENABLE_SELINUX,
};

#എങ്കിൽ ENABLE_FEATURE_STAT_FORMAT
തരനിർവചനം bool (*statfunc_ptr)(സ്ഥിരം അക്ഷരം *, സ്ഥിരം അക്ഷരം *);
#അല്ലെങ്കിൽ
തരനിർവചനം bool (*statfunc_ptr)(സ്ഥിരം അക്ഷരം *);
#endif

സ്ഥിതം സ്ഥിരം അക്ഷരം *file_type(സ്ഥിരം ഘടന stat *st)
{
	/* See POSIX 1003.1-2001 XCU Table 4-8 lines 17093-17107
	 * for some of these formats.
	 * To keep diagnostics grammatical in English, the
	 * returned string must start with a consonant.
	 */
	എങ്കിൽ (S_ISREG(st->st_mode))  തിരികെ st->st_size == 0 ? "regular empty file" : "regular file";
	എങ്കിൽ (S_ISDIR(st->st_mode))  തിരികെ "directory";
	എങ്കിൽ (S_ISBLK(st->st_mode))  തിരികെ "block special file";
	എങ്കിൽ (S_ISCHR(st->st_mode))  തിരികെ "character special file";
	എങ്കിൽ (S_ISFIFO(st->st_mode)) തിരികെ "fifo";
	എങ്കിൽ (S_ISLNK(st->st_mode))  തിരികെ "symbolic link";
	എങ്കിൽ (S_ISSOCK(st->st_mode)) തിരികെ "socket";
#ifdef S_TYPEISMQ
	എങ്കിൽ (S_TYPEISMQ(st))        തിരികെ "message queue";
#endif
#ifdef S_TYPEISSEM
	എങ്കിൽ (S_TYPEISSEM(st))       തിരികെ "semaphore";
#endif
#ifdef S_TYPEISSHM
	എങ്കിൽ (S_TYPEISSHM(st))       തിരികെ "shared memory object";
#endif
#ifdef S_TYPEISTMO
	എങ്കിൽ (S_TYPEISTMO(st))       തിരികെ "typed memory object";
#endif
	തിരികെ "weird file";
}

സ്ഥിതം സ്ഥിരം അക്ഷരം *human_time(ഘടന timespec *ts)
{
	അക്ഷരം fmt[വലുപ്പം("%Y-%m-%d %H:%M:%S.123456789 %z") + /*paranoia*/ 8];

	/* coreutils 6.3 compat */
#define buf bb_common_bufsiz1
	setup_common_bufsiz();

	sprintf(stpcpy(fmt, "%Y-%m-%d %H:%M:%S"), ".%09u %%z", (unsigned)ts->tv_nsec);
	strftime(buf, COMMON_BUFSIZE, fmt, localtime(&ts->tv_sec));
	തിരികെ buf;
#undef buf
}

#എങ്കിൽ ENABLE_FEATURE_STAT_FILESYSTEM
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
സ്ഥിതം സ്ഥിരം അക്ഷരം *human_fstype(uint32_t f_type)
{
# define FS_TYPE(type, name) type,
	സ്ഥിതം സ്ഥിരം uint32_t fstype[] ALIGN4 = {
		FS_TYPE_LIST
	};
# undef FS_TYPE
# define FS_TYPE(type, name) name"\0"
	സ്ഥിതം സ്ഥിരം അക്ഷരം humanname[] ALIGN1 =
		FS_TYPE_LIST
		"UNKNOWN";
# undef FS_TYPE
	പൂർണ്ണസംഖ്യ i;

	വേണ്ടി (i = 0; i < ARRAY_SIZE(fstype); ++i)
		എങ്കിൽ (fstype[i] == f_type)
			നിർത്തുക;
	തിരികെ nth_string(humanname, i);
}

/* "man statfs" says that statfsbuf->f_fsid is a mess */
/* coreutils treats it as an array of ints, most significant first */
സ്ഥിതം unsigned long long get_f_fsid(സ്ഥിരം ഘടന statfs *statfsbuf)
{
	സ്ഥിരം unsigned *p = (സ്ഥിരം ശൂന്യം*) &statfsbuf->f_fsid;
	unsigned sz = വലുപ്പം(statfsbuf->f_fsid) / വലുപ്പം(unsigned);
	unsigned long long r = 0;

	ചെയ്യുക
		r = (r << (വലുപ്പം(unsigned)*8)) | *p++;
	ആകുമ്പോൾ (--sz > 0);
	തിരികെ r;
}
#endif  /* FEATURE_STAT_FILESYSTEM */

#എങ്കിൽ ENABLE_FEATURE_STAT_FORMAT
സ്ഥിതം ശൂന്യം strcatc(അക്ഷരം *str, അക്ഷരം c)
{
	പൂർണ്ണസംഖ്യ len = strlen(str);
	str[len++] = c;
	str[len] = '\0';
}

സ്ഥിതം ശൂന്യം printfs(അക്ഷരം *pformat, സ്ഥിരം അക്ഷരം *സന്ദേശം)
{
	strcatc(pformat, 's');
	printf(pformat, സന്ദേശം);
}

#എങ്കിൽ ENABLE_FEATURE_STAT_FILESYSTEM
/* print statfs info */
സ്ഥിതം ശൂന്യം FAST_FUNC print_statfs(അക്ഷരം *pformat, സ്ഥിരം അക്ഷരം m,
		സ്ഥിരം അക്ഷരം *സ്ഥിരം filename, സ്ഥിരം ശൂന്യം *data
		IF_SELINUX(, security_context_t scontext))
{
	സ്ഥിരം ഘടന statfs *statfsbuf = data;
	എങ്കിൽ (m == 'n') {
		printfs(pformat, filename);
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 'i') {
		strcat(pformat, "llx");
		printf(pformat, get_f_fsid(statfsbuf));
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 'l') {
		strcat(pformat, "lu");
		printf(pformat, (unsigned long) statfsbuf->f_namelen);
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 't') {
		strcat(pformat, "lx");
		printf(pformat, (unsigned long) statfsbuf->f_type); /* no equiv */
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 'T') {
		printfs(pformat, human_fstype(statfsbuf->f_type));
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 'b') {
		strcat(pformat, "llu");
		printf(pformat, (unsigned long long) statfsbuf->f_blocks);
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 'f') {
		strcat(pformat, "llu");
		printf(pformat, (unsigned long long) statfsbuf->f_bfree);
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 'a') {
		strcat(pformat, "llu");
		printf(pformat, (unsigned long long) statfsbuf->f_bavail);
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 's' || m == 'S') {
		strcat(pformat, "lu");
		printf(pformat, (unsigned long) statfsbuf->f_bsize);
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 'c') {
		strcat(pformat, "llu");
		printf(pformat, (unsigned long long) statfsbuf->f_files);
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 'd') {
		strcat(pformat, "llu");
		printf(pformat, (unsigned long long) statfsbuf->f_ffree);
# എങ്കിൽ ENABLE_SELINUX
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 'C' && (option_mask32 & OPT_SELINUX)) {
		printfs(pformat, scontext);
# endif
	} അല്ലെങ്കിൽ {
		strcatc(pformat, 'c');
		printf(pformat, m);
	}
}
#endif

/* print stat info */
സ്ഥിതം ശൂന്യം FAST_FUNC print_stat(അക്ഷരം *pformat, സ്ഥിരം അക്ഷരം m,
		സ്ഥിരം അക്ഷരം *സ്ഥിരം filename, സ്ഥിരം ശൂന്യം *data
		IF_SELINUX(, security_context_t scontext))
{
#define TYPE_SIGNED(t) (! ((t) 0 < (t) -1))
	ഘടന stat *statbuf = (ഘടന stat *) data;
	ഘടന passwd *pw_ent;
	ഘടന group *gw_ent;

	എങ്കിൽ (m == 'n') {
		printfs(pformat, filename);
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 'N') {
		strcatc(pformat, 's');
		എങ്കിൽ (S_ISLNK(statbuf->st_mode)) {
			അക്ഷരം *linkname = xmalloc_readlink_or_warn(filename);
			എങ്കിൽ (linkname == ശൂന്യ)
				തിരികെ;
			printf("'%s' -> '%s'", filename, linkname);
			സ്വതന്ത്രം(linkname);
		} അല്ലെങ്കിൽ {
			printf(pformat, filename);
		}
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 'd') {
		strcat(pformat, "llu");
		printf(pformat, (unsigned long long) statbuf->st_dev);
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 'D') {
		strcat(pformat, "llx");
		printf(pformat, (unsigned long long) statbuf->st_dev);
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 'i') {
		strcat(pformat, "llu");
		printf(pformat, (unsigned long long) statbuf->st_ino);
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 'a') {
		strcat(pformat, "lo");
		printf(pformat, (unsigned long) (statbuf->st_mode & (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)));
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 'A') {
		അക്ഷരം modestr[12];
		printfs(pformat, bb_mode_string(modestr, statbuf->st_mode));
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 'f') {
		strcat(pformat, "lx");
		printf(pformat, (unsigned long) statbuf->st_mode);
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 'F') {
		printfs(pformat, file_type(statbuf));
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 'h') {
		strcat(pformat, "lu");
		printf(pformat, (unsigned long) statbuf->st_nlink);
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 'u') {
		strcat(pformat, "lu");
		printf(pformat, (unsigned long) statbuf->st_uid);
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 'U') {
		pw_ent = getpwuid(statbuf->st_uid);
		printfs(pformat, (pw_ent != ശൂന്യ) ? pw_ent->pw_name : "UNKNOWN");
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 'g') {
		strcat(pformat, "lu");
		printf(pformat, (unsigned long) statbuf->st_gid);
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 'G') {
		gw_ent = getgrgid(statbuf->st_gid);
		printfs(pformat, (gw_ent != ശൂന്യ) ? gw_ent->gr_name : "UNKNOWN");
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 't') {
		strcat(pformat, "lx");
		printf(pformat, (unsigned long) major(statbuf->st_rdev));
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 'T') {
		strcat(pformat, "lx");
		printf(pformat, (unsigned long) minor(statbuf->st_rdev));
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 's') {
		strcat(pformat, "llu");
		printf(pformat, (unsigned long long) statbuf->st_size);
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 'B') {
		strcat(pformat, "lu");
		printf(pformat, (unsigned long) 512); //ST_NBLOCKSIZE
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 'b') {
		strcat(pformat, "llu");
		printf(pformat, (unsigned long long) statbuf->st_blocks);
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 'o') {
		strcat(pformat, "lu");
		printf(pformat, (unsigned long) statbuf->st_blksize);
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 'x') {
		printfs(pformat, human_time(&statbuf->st_atim));
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 'X') {
		strcat(pformat, TYPE_SIGNED(time_t) ? "ld" : "lu");
		/* note: (unsigned long) would be wrong:
		 * imagine (unsigned long64)int32 */
		printf(pformat, (long) statbuf->st_atime);
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 'y') {
		printfs(pformat, human_time(&statbuf->st_mtim));
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 'Y') {
		strcat(pformat, TYPE_SIGNED(time_t) ? "ld" : "lu");
		printf(pformat, (long) statbuf->st_mtime);
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 'z') {
		printfs(pformat, human_time(&statbuf->st_ctim));
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 'Z') {
		strcat(pformat, TYPE_SIGNED(time_t) ? "ld" : "lu");
		printf(pformat, (long) statbuf->st_ctime);
# എങ്കിൽ ENABLE_SELINUX
	} അല്ലെങ്കിൽ എങ്കിൽ (m == 'C' && (option_mask32 & OPT_SELINUX)) {
		printfs(pformat, scontext);
# endif
	} അല്ലെങ്കിൽ {
		strcatc(pformat, 'c');
		printf(pformat, m);
	}
}

സ്ഥിതം ശൂന്യം print_it(സ്ഥിരം അക്ഷരം *masterformat,
		സ്ഥിരം അക്ഷരം *filename,
		ശൂന്യം FAST_FUNC (*print_func)(അക്ഷരം*, അക്ഷരം, സ്ഥിരം അക്ഷരം*, സ്ഥിരം ശൂന്യം* IF_SELINUX(, security_context_t scontext)),
		സ്ഥിരം ശൂന്യം *data
		IF_SELINUX(, security_context_t scontext))
{
	/* Create a working copy of the format string */
	അക്ഷരം *format = xstrdup(masterformat);
	/* Add 2 to accommodate our conversion of the stat '%s' format string
	 * to the printf '%llu' one.  */
	അക്ഷരം *dest = xmalloc(strlen(format) + 2 + 1);
	അക്ഷരം *b;

	b = format;
	ആകുമ്പോൾ (b) {
		/* Each iteration finds next %spec,
		 * prints preceding string and handles found %spec
		 */
		size_t len;
		അക്ഷരം *p = strchr(b, '%');
		എങ്കിൽ (!p) {
			/* coreutils 6.3 always prints newline at the end */
			/*fputs(b, stdout);*/
			puts(b);
			നിർത്തുക;
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
		തിരഞ്ഞെടുപ്പ് (*p) {
		അവസ്ഥ '\0':
			b = ശൂന്യ;
			/* fall through */
		അവസ്ഥ '%':
			bb_putchar('%');
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
			/* Completes "%<modifiers>" with specifier and printfs */
			print_func(dest, *p, filename, data IF_SELINUX(,scontext));
			നിർത്തുക;
		}
	}

	സ്വതന്ത്രം(format);
	സ്വതന്ത്രം(dest);
}
#endif  /* FEATURE_STAT_FORMAT */

#എങ്കിൽ ENABLE_FEATURE_STAT_FILESYSTEM
/* Stat the file system and print what we find.  */
#എങ്കിൽ !ENABLE_FEATURE_STAT_FORMAT
#define do_statfs(filename, format) do_statfs(filename)
#endif
സ്ഥിതം bool do_statfs(സ്ഥിരം അക്ഷരം *filename, സ്ഥിരം അക്ഷരം *format)
{
	ഘടന statfs statfsbuf;
#എങ്കിൽ !ENABLE_FEATURE_STAT_FORMAT
	സ്ഥിരം അക്ഷരം *format;
#endif
#എങ്കിൽ ENABLE_SELINUX
	security_context_t scontext = ശൂന്യ;

	എങ്കിൽ (option_mask32 & OPT_SELINUX) {
		എങ്കിൽ ((option_mask32 & OPT_DEREFERENCE
		     ? lgetfilecon(filename, &scontext)
		     : getfilecon(filename, &scontext)
		    ) < 0
		) {
			bb_simple_perror_സന്ദേശം(filename);
			തിരികെ 0;
		}
	}
#endif
	എങ്കിൽ (statfs(filename, &statfsbuf) != 0) {
		bb_perror_സന്ദേശം("can't read file system information for '%s'", filename);
		തിരികെ 0;
	}

#എങ്കിൽ ENABLE_FEATURE_STAT_FORMAT
	എങ്കിൽ (format == ശൂന്യ) {
# എങ്കിൽ !ENABLE_SELINUX
		format = (option_mask32 & OPT_TERSE
			? "%n %i %l %t %s %b %f %a %c %d"
			: "  File: \"%n\"\n"
			  "    ID: %-8i Namelen: %-7l Type: %T\n"
			  "Block size: %-10s\n"
			  "Blocks: Total: %-10b Free: %-10f Available: %a\n"
			  "Inodes: Total: %-10c Free: %d");
# അല്ലെങ്കിൽ
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
#അല്ലെങ്കിൽ /* !FEATURE_STAT_FORMAT */
	format = (option_mask32 & OPT_TERSE
		? "%s %llx %lu "
		: "  File: \"%s\"\n"
		  "    ID: %-8llx Namelen: %-7lu ");
	printf(format,
	       filename,
	       get_f_fsid(&statfsbuf),
	       statfsbuf.f_namelen);

	എങ്കിൽ (option_mask32 & OPT_TERSE)
		printf("%lx ", (unsigned long) statfsbuf.f_type);
	അല്ലെങ്കിൽ
		printf("Type: %s\n", human_fstype(statfsbuf.f_type));

# എങ്കിൽ !ENABLE_SELINUX
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
# അല്ലെങ്കിൽ
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

	എങ്കിൽ (scontext)
		freecon(scontext);
# endif
#endif  /* FEATURE_STAT_FORMAT */
	തിരികെ 1;
}
#endif  /* FEATURE_STAT_FILESYSTEM */

/* stat the file and print what we find */
#എങ്കിൽ !ENABLE_FEATURE_STAT_FORMAT
#define do_stat(filename, format) do_stat(filename)
#endif
സ്ഥിതം bool do_stat(സ്ഥിരം അക്ഷരം *filename, സ്ഥിരം അക്ഷരം *format)
{
	ഘടന stat statbuf;
#എങ്കിൽ ENABLE_SELINUX
	security_context_t scontext = ശൂന്യ;

	എങ്കിൽ (option_mask32 & OPT_SELINUX) {
		എങ്കിൽ ((option_mask32 & OPT_DEREFERENCE
		     ? lgetfilecon(filename, &scontext)
		     : getfilecon(filename, &scontext)
		    ) < 0
		) {
			bb_simple_perror_സന്ദേശം(filename);
			തിരികെ 0;
		}
	}
#endif
	എങ്കിൽ ((option_mask32 & OPT_DEREFERENCE ? stat : lstat) (filename, &statbuf) != 0) {
		bb_perror_സന്ദേശം("can't stat '%s'", filename);
		തിരികെ 0;
	}

#എങ്കിൽ ENABLE_FEATURE_STAT_FORMAT
	എങ്കിൽ (format == ശൂന്യ) {
# എങ്കിൽ !ENABLE_SELINUX
		എങ്കിൽ (option_mask32 & OPT_TERSE) {
			format = "%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %o";
		} അല്ലെങ്കിൽ {
			എങ്കിൽ (S_ISBLK(statbuf.st_mode) || S_ISCHR(statbuf.st_mode)) {
				format =
					"  File: %N\n"
					"  Size: %-10s\tBlocks: %-10b IO Block: %-6o %F\n"
					"Device: %Dh/%dd\tInode: %-10i  Links: %-5h"
					" Device type: %t,%T\n"
					"Access: (%04a/%10.10A)  Uid: (%5u/%8U)   Gid: (%5g/%8G)\n"
					"Access: %x\n" "Modify: %y\n" "Change: %z";
			} അല്ലെങ്കിൽ {
				format =
					"  File: %N\n"
					"  Size: %-10s\tBlocks: %-10b IO Block: %-6o %F\n"
					"Device: %Dh/%dd\tInode: %-10i  Links: %h\n"
					"Access: (%04a/%10.10A)  Uid: (%5u/%8U)   Gid: (%5g/%8G)\n"
					"Access: %x\n" "Modify: %y\n" "Change: %z";
			}
		}
# അല്ലെങ്കിൽ
		എങ്കിൽ (option_mask32 & OPT_TERSE) {
			format = (option_mask32 & OPT_SELINUX ?
				"%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %o %C\n"
				:
				"%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %o\n"
				);
		} അല്ലെങ്കിൽ {
			എങ്കിൽ (S_ISBLK(statbuf.st_mode) || S_ISCHR(statbuf.st_mode)) {
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
			} അല്ലെങ്കിൽ {
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
#അല്ലെങ്കിൽ	/* FEATURE_STAT_FORMAT */
	എങ്കിൽ (option_mask32 & OPT_TERSE) {
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
# എങ്കിൽ ENABLE_SELINUX
		എങ്കിൽ (option_mask32 & OPT_SELINUX)
			printf(" %s\n", scontext);
		അല്ലെങ്കിൽ
			bb_putchar('\n');
# endif
	} അല്ലെങ്കിൽ {
		അക്ഷരം modestr[12];
		അക്ഷരം *linkname = ശൂന്യ;
		ഘടന passwd *pw_ent;
		ഘടന group *gw_ent;

		gw_ent = getgrgid(statbuf.st_gid);
		pw_ent = getpwuid(statbuf.st_uid);

		എങ്കിൽ (S_ISLNK(statbuf.st_mode))
			linkname = xmalloc_readlink_or_warn(filename);
		എങ്കിൽ (linkname) {
			printf("  File: '%s' -> '%s'\n", filename, linkname);
			സ്വതന്ത്രം(linkname);
		} അല്ലെങ്കിൽ {
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
		എങ്കിൽ (S_ISBLK(statbuf.st_mode) || S_ISCHR(statbuf.st_mode))
			printf(" Device type: %lx,%lx\n",
			       (unsigned long) major(statbuf.st_rdev),
			       (unsigned long) minor(statbuf.st_rdev));
		അല്ലെങ്കിൽ
			bb_putchar('\n');
		printf("Access: (%04lo/%10.10s)  Uid: (%5lu/%8s)   Gid: (%5lu/%8s)\n",
		       (unsigned long) (statbuf.st_mode & (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)),
		       bb_mode_string(modestr, statbuf.st_mode),
		       (unsigned long) statbuf.st_uid,
		       (pw_ent != ശൂന്യ) ? pw_ent->pw_name : "UNKNOWN",
		       (unsigned long) statbuf.st_gid,
		       (gw_ent != ശൂന്യ) ? gw_ent->gr_name : "UNKNOWN");
# എങ്കിൽ ENABLE_SELINUX
		എങ്കിൽ (option_mask32 & OPT_SELINUX)
			printf("   S_Context: %s\n", scontext);
# endif
		printf("Access: %s\n", human_time(&statbuf.st_atim));
		printf("Modify: %s\n", human_time(&statbuf.st_mtim));
		printf("Change: %s\n", human_time(&statbuf.st_ctim));
	}
#endif  /* FEATURE_STAT_FORMAT */
	തിരികെ 1;
}

പൂർണ്ണസംഖ്യ stat_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) MAIN_EXTERNALLY_VISIBLE;
പൂർണ്ണസംഖ്യ stat_main(പൂർണ്ണസംഖ്യ argc UNUSED_PARAM, അക്ഷരം **argv)
{
	IF_FEATURE_STAT_FORMAT(അക്ഷരം *format = ശൂന്യ;)
	പൂർണ്ണസംഖ്യ i;
	പൂർണ്ണസംഖ്യ ok;
	statfunc_ptr statfunc = do_stat;
#എങ്കിൽ ENABLE_FEATURE_STAT_FILESYSTEM || ENABLE_SELINUX
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
#എങ്കിൽ ENABLE_FEATURE_STAT_FILESYSTEM
	എങ്കിൽ (opts & OPT_FILESYS) /* -f */
		statfunc = do_statfs;
#endif
#എങ്കിൽ ENABLE_SELINUX
	എങ്കിൽ (opts & OPT_SELINUX) {
		selinux_or_die();
	}
#endif
	ok = 1;
	argv += optind;
	വേണ്ടി (i = 0; argv[i]; ++i)
		ok &= statfunc(argv[i] IF_FEATURE_STAT_FORMAT(, format));

	തിരികെ (ok ? EXIT_SUCCESS : EXIT_FAILURE);
}
