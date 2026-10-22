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

#அந்தர்பாவ "libbb.h"
#அந்தர்பாவ "common_bufsiz.h"

எண்ணிக்கை {
	OPT_TERSE       = (1 << 0),
	OPT_DEREFERENCE = (1 << 1),
	OPT_FILESYS     = (1 << 2) * ENABLE_FEATURE_STAT_FILESYSTEM,
	OPT_SELINUX     = (1 << (2+ENABLE_FEATURE_STAT_FILESYSTEM)) * ENABLE_SELINUX,
};

#எனில் ENABLE_FEATURE_STAT_FORMAT
வகைவரை bool (*statfunc_ptr)(மாறிலி எழுத்து *, மாறிலி எழுத்து *);
#இல்லையெனில்
வகைவரை bool (*statfunc_ptr)(மாறிலி எழுத்து *);
#endif

நிலையான மாறிலி எழுத்து *file_type(மாறிலி கட்டமைப்பு stat *st)
{
	/* See POSIX 1003.1-2001 XCU Table 4-8 lines 17093-17107
	 * for some of these formats.
	 * To keep diagnostics grammatical in English, the
	 * returned string must start with a consonant.
	 */
	எனில் (S_ISREG(st->st_mode))  திரும்பு st->st_size == 0 ? "regular empty file" : "regular file";
	எனில் (S_ISDIR(st->st_mode))  திரும்பு "directory";
	எனில் (S_ISBLK(st->st_mode))  திரும்பு "block special file";
	எனில் (S_ISCHR(st->st_mode))  திரும்பு "character special file";
	எனில் (S_ISFIFO(st->st_mode)) திரும்பு "fifo";
	எனில் (S_ISLNK(st->st_mode))  திரும்பு "symbolic link";
	எனில் (S_ISSOCK(st->st_mode)) திரும்பு "socket";
#ifdef S_TYPEISMQ
	எனில் (S_TYPEISMQ(st))        திரும்பு "message queue";
#endif
#ifdef S_TYPEISSEM
	எனில் (S_TYPEISSEM(st))       திரும்பு "semaphore";
#endif
#ifdef S_TYPEISSHM
	எனில் (S_TYPEISSHM(st))       திரும்பு "shared memory object";
#endif
#ifdef S_TYPEISTMO
	எனில் (S_TYPEISTMO(st))       திரும்பு "typed memory object";
#endif
	திரும்பு "weird file";
}

நிலையான மாறிலி எழுத்து *human_time(கட்டமைப்பு timespec *ts)
{
	எழுத்து fmt[அளவு("%Y-%m-%d %H:%M:%S.123456789 %z") + /*paranoia*/ 8];

	/* coreutils 6.3 compat */
#define buf bb_common_bufsiz1
	setup_common_bufsiz();

	sprintf(stpcpy(fmt, "%Y-%m-%d %H:%M:%S"), ".%09u %%z", (unsigned)ts->tv_nsec);
	strftime(buf, COMMON_BUFSIZE, fmt, localtime(&ts->tv_sec));
	திரும்பு buf;
#undef buf
}

#எனில் ENABLE_FEATURE_STAT_FILESYSTEM
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
நிலையான மாறிலி எழுத்து *human_fstype(uint32_t f_type)
{
# define FS_TYPE(type, name) type,
	நிலையான மாறிலி uint32_t fstype[] ALIGN4 = {
		FS_TYPE_LIST
	};
# undef FS_TYPE
# define FS_TYPE(type, name) name"\0"
	நிலையான மாறிலி எழுத்து humanname[] ALIGN1 =
		FS_TYPE_LIST
		"UNKNOWN";
# undef FS_TYPE
	முழு i;

	க்கு (i = 0; i < ARRAY_SIZE(fstype); ++i)
		எனில் (fstype[i] == f_type)
			நிறுத்து;
	திரும்பு nth_string(humanname, i);
}

/* "man statfs" says that statfsbuf->f_fsid is a mess */
/* coreutils treats it as an array of ints, most significant first */
நிலையான unsigned long long get_f_fsid(மாறிலி கட்டமைப்பு statfs *statfsbuf)
{
	மாறிலி unsigned *p = (மாறிலி வெறுமை*) &statfsbuf->f_fsid;
	unsigned sz = அளவு(statfsbuf->f_fsid) / அளவு(unsigned);
	unsigned long long r = 0;

	செய்
		r = (r << (அளவு(unsigned)*8)) | *p++;
	போது (--sz > 0);
	திரும்பு r;
}
#endif  /* FEATURE_STAT_FILESYSTEM */

#எனில் ENABLE_FEATURE_STAT_FORMAT
நிலையான வெறுமை strcatc(எழுத்து *str, எழுத்து c)
{
	முழு len = strlen(str);
	str[len++] = c;
	str[len] = '\0';
}

நிலையான வெறுமை printfs(எழுத்து *pformat, மாறிலி எழுத்து *செய்தி)
{
	strcatc(pformat, 's');
	printf(pformat, செய்தி);
}

#எனில் ENABLE_FEATURE_STAT_FILESYSTEM
/* print statfs info */
நிலையான வெறுமை FAST_FUNC print_statfs(எழுத்து *pformat, மாறிலி எழுத்து m,
		மாறிலி எழுத்து *மாறிலி filename, மாறிலி வெறுமை *data
		IF_SELINUX(, security_context_t scontext))
{
	மாறிலி கட்டமைப்பு statfs *statfsbuf = data;
	எனில் (m == 'n') {
		printfs(pformat, filename);
	} இல்லையெனில் எனில் (m == 'i') {
		strcat(pformat, "llx");
		printf(pformat, get_f_fsid(statfsbuf));
	} இல்லையெனில் எனில் (m == 'l') {
		strcat(pformat, "lu");
		printf(pformat, (unsigned long) statfsbuf->f_namelen);
	} இல்லையெனில் எனில் (m == 't') {
		strcat(pformat, "lx");
		printf(pformat, (unsigned long) statfsbuf->f_type); /* no equiv */
	} இல்லையெனில் எனில் (m == 'T') {
		printfs(pformat, human_fstype(statfsbuf->f_type));
	} இல்லையெனில் எனில் (m == 'b') {
		strcat(pformat, "llu");
		printf(pformat, (unsigned long long) statfsbuf->f_blocks);
	} இல்லையெனில் எனில் (m == 'f') {
		strcat(pformat, "llu");
		printf(pformat, (unsigned long long) statfsbuf->f_bfree);
	} இல்லையெனில் எனில் (m == 'a') {
		strcat(pformat, "llu");
		printf(pformat, (unsigned long long) statfsbuf->f_bavail);
	} இல்லையெனில் எனில் (m == 's' || m == 'S') {
		strcat(pformat, "lu");
		printf(pformat, (unsigned long) statfsbuf->f_bsize);
	} இல்லையெனில் எனில் (m == 'c') {
		strcat(pformat, "llu");
		printf(pformat, (unsigned long long) statfsbuf->f_files);
	} இல்லையெனில் எனில் (m == 'd') {
		strcat(pformat, "llu");
		printf(pformat, (unsigned long long) statfsbuf->f_ffree);
# எனில் ENABLE_SELINUX
	} இல்லையெனில் எனில் (m == 'C' && (option_mask32 & OPT_SELINUX)) {
		printfs(pformat, scontext);
# endif
	} இல்லையெனில் {
		strcatc(pformat, 'c');
		printf(pformat, m);
	}
}
#endif

/* print stat info */
நிலையான வெறுமை FAST_FUNC print_stat(எழுத்து *pformat, மாறிலி எழுத்து m,
		மாறிலி எழுத்து *மாறிலி filename, மாறிலி வெறுமை *data
		IF_SELINUX(, security_context_t scontext))
{
#define TYPE_SIGNED(t) (! ((t) 0 < (t) -1))
	கட்டமைப்பு stat *statbuf = (கட்டமைப்பு stat *) data;
	கட்டமைப்பு passwd *pw_ent;
	கட்டமைப்பு group *gw_ent;

	எனில் (m == 'n') {
		printfs(pformat, filename);
	} இல்லையெனில் எனில் (m == 'N') {
		strcatc(pformat, 's');
		எனில் (S_ISLNK(statbuf->st_mode)) {
			எழுத்து *linkname = xmalloc_readlink_or_warn(filename);
			எனில் (linkname == வெற்று)
				திரும்பு;
			printf("'%s' -> '%s'", filename, linkname);
			விடுவி(linkname);
		} இல்லையெனில் {
			printf(pformat, filename);
		}
	} இல்லையெனில் எனில் (m == 'd') {
		strcat(pformat, "llu");
		printf(pformat, (unsigned long long) statbuf->st_dev);
	} இல்லையெனில் எனில் (m == 'D') {
		strcat(pformat, "llx");
		printf(pformat, (unsigned long long) statbuf->st_dev);
	} இல்லையெனில் எனில் (m == 'i') {
		strcat(pformat, "llu");
		printf(pformat, (unsigned long long) statbuf->st_ino);
	} இல்லையெனில் எனில் (m == 'a') {
		strcat(pformat, "lo");
		printf(pformat, (unsigned long) (statbuf->st_mode & (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)));
	} இல்லையெனில் எனில் (m == 'A') {
		எழுத்து modestr[12];
		printfs(pformat, bb_mode_string(modestr, statbuf->st_mode));
	} இல்லையெனில் எனில் (m == 'f') {
		strcat(pformat, "lx");
		printf(pformat, (unsigned long) statbuf->st_mode);
	} இல்லையெனில் எனில் (m == 'F') {
		printfs(pformat, file_type(statbuf));
	} இல்லையெனில் எனில் (m == 'h') {
		strcat(pformat, "lu");
		printf(pformat, (unsigned long) statbuf->st_nlink);
	} இல்லையெனில் எனில் (m == 'u') {
		strcat(pformat, "lu");
		printf(pformat, (unsigned long) statbuf->st_uid);
	} இல்லையெனில் எனில் (m == 'U') {
		pw_ent = getpwuid(statbuf->st_uid);
		printfs(pformat, (pw_ent != வெற்று) ? pw_ent->pw_name : "UNKNOWN");
	} இல்லையெனில் எனில் (m == 'g') {
		strcat(pformat, "lu");
		printf(pformat, (unsigned long) statbuf->st_gid);
	} இல்லையெனில் எனில் (m == 'G') {
		gw_ent = getgrgid(statbuf->st_gid);
		printfs(pformat, (gw_ent != வெற்று) ? gw_ent->gr_name : "UNKNOWN");
	} இல்லையெனில் எனில் (m == 't') {
		strcat(pformat, "lx");
		printf(pformat, (unsigned long) major(statbuf->st_rdev));
	} இல்லையெனில் எனில் (m == 'T') {
		strcat(pformat, "lx");
		printf(pformat, (unsigned long) minor(statbuf->st_rdev));
	} இல்லையெனில் எனில் (m == 's') {
		strcat(pformat, "llu");
		printf(pformat, (unsigned long long) statbuf->st_size);
	} இல்லையெனில் எனில் (m == 'B') {
		strcat(pformat, "lu");
		printf(pformat, (unsigned long) 512); //ST_NBLOCKSIZE
	} இல்லையெனில் எனில் (m == 'b') {
		strcat(pformat, "llu");
		printf(pformat, (unsigned long long) statbuf->st_blocks);
	} இல்லையெனில் எனில் (m == 'o') {
		strcat(pformat, "lu");
		printf(pformat, (unsigned long) statbuf->st_blksize);
	} இல்லையெனில் எனில் (m == 'x') {
		printfs(pformat, human_time(&statbuf->st_atim));
	} இல்லையெனில் எனில் (m == 'X') {
		strcat(pformat, TYPE_SIGNED(time_t) ? "ld" : "lu");
		/* note: (unsigned long) would be wrong:
		 * imagine (unsigned long64)int32 */
		printf(pformat, (long) statbuf->st_atime);
	} இல்லையெனில் எனில் (m == 'y') {
		printfs(pformat, human_time(&statbuf->st_mtim));
	} இல்லையெனில் எனில் (m == 'Y') {
		strcat(pformat, TYPE_SIGNED(time_t) ? "ld" : "lu");
		printf(pformat, (long) statbuf->st_mtime);
	} இல்லையெனில் எனில் (m == 'z') {
		printfs(pformat, human_time(&statbuf->st_ctim));
	} இல்லையெனில் எனில் (m == 'Z') {
		strcat(pformat, TYPE_SIGNED(time_t) ? "ld" : "lu");
		printf(pformat, (long) statbuf->st_ctime);
# எனில் ENABLE_SELINUX
	} இல்லையெனில் எனில் (m == 'C' && (option_mask32 & OPT_SELINUX)) {
		printfs(pformat, scontext);
# endif
	} இல்லையெனில் {
		strcatc(pformat, 'c');
		printf(pformat, m);
	}
}

நிலையான வெறுமை print_it(மாறிலி எழுத்து *masterformat,
		மாறிலி எழுத்து *filename,
		வெறுமை FAST_FUNC (*print_func)(எழுத்து*, எழுத்து, மாறிலி எழுத்து*, மாறிலி வெறுமை* IF_SELINUX(, security_context_t scontext)),
		மாறிலி வெறுமை *data
		IF_SELINUX(, security_context_t scontext))
{
	/* Create a working copy of the format string */
	எழுத்து *format = xstrdup(masterformat);
	/* Add 2 to accommodate our conversion of the stat '%s' format string
	 * to the printf '%llu' one.  */
	எழுத்து *dest = xmalloc(strlen(format) + 2 + 1);
	எழுத்து *b;

	b = format;
	போது (b) {
		/* Each iteration finds next %spec,
		 * prints preceding string and handles found %spec
		 */
		size_t len;
		எழுத்து *p = strchr(b, '%');
		எனில் (!p) {
			/* coreutils 6.3 always prints newline at the end */
			/*fputs(b, stdout);*/
			puts(b);
			நிறுத்து;
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
		தேர்வு (*p) {
		நிலை '\0':
			b = வெற்று;
			/* fall through */
		நிலை '%':
			bb_putchar('%');
			நிறுத்து;
		இயல்பு:
			/* Completes "%<modifiers>" with specifier and printfs */
			print_func(dest, *p, filename, data IF_SELINUX(,scontext));
			நிறுத்து;
		}
	}

	விடுவி(format);
	விடுவி(dest);
}
#endif  /* FEATURE_STAT_FORMAT */

#எனில் ENABLE_FEATURE_STAT_FILESYSTEM
/* Stat the file system and print what we find.  */
#எனில் !ENABLE_FEATURE_STAT_FORMAT
#define do_statfs(filename, format) do_statfs(filename)
#endif
நிலையான bool do_statfs(மாறிலி எழுத்து *filename, மாறிலி எழுத்து *format)
{
	கட்டமைப்பு statfs statfsbuf;
#எனில் !ENABLE_FEATURE_STAT_FORMAT
	மாறிலி எழுத்து *format;
#endif
#எனில் ENABLE_SELINUX
	security_context_t scontext = வெற்று;

	எனில் (option_mask32 & OPT_SELINUX) {
		எனில் ((option_mask32 & OPT_DEREFERENCE
		     ? lgetfilecon(filename, &scontext)
		     : getfilecon(filename, &scontext)
		    ) < 0
		) {
			bb_simple_perror_செய்தி(filename);
			திரும்பு 0;
		}
	}
#endif
	எனில் (statfs(filename, &statfsbuf) != 0) {
		bb_perror_செய்தி("can't read file system information for '%s'", filename);
		திரும்பு 0;
	}

#எனில் ENABLE_FEATURE_STAT_FORMAT
	எனில் (format == வெற்று) {
# எனில் !ENABLE_SELINUX
		format = (option_mask32 & OPT_TERSE
			? "%n %i %l %t %s %b %f %a %c %d"
			: "  File: \"%n\"\n"
			  "    ID: %-8i Namelen: %-7l Type: %T\n"
			  "Block size: %-10s\n"
			  "Blocks: Total: %-10b Free: %-10f Available: %a\n"
			  "Inodes: Total: %-10c Free: %d");
# இல்லையெனில்
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
#இல்லையெனில் /* !FEATURE_STAT_FORMAT */
	format = (option_mask32 & OPT_TERSE
		? "%s %llx %lu "
		: "  File: \"%s\"\n"
		  "    ID: %-8llx Namelen: %-7lu ");
	printf(format,
	       filename,
	       get_f_fsid(&statfsbuf),
	       statfsbuf.f_namelen);

	எனில் (option_mask32 & OPT_TERSE)
		printf("%lx ", (unsigned long) statfsbuf.f_type);
	இல்லையெனில்
		printf("Type: %s\n", human_fstype(statfsbuf.f_type));

# எனில் !ENABLE_SELINUX
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
# இல்லையெனில்
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

	எனில் (scontext)
		freecon(scontext);
# endif
#endif  /* FEATURE_STAT_FORMAT */
	திரும்பு 1;
}
#endif  /* FEATURE_STAT_FILESYSTEM */

/* stat the file and print what we find */
#எனில் !ENABLE_FEATURE_STAT_FORMAT
#define do_stat(filename, format) do_stat(filename)
#endif
நிலையான bool do_stat(மாறிலி எழுத்து *filename, மாறிலி எழுத்து *format)
{
	கட்டமைப்பு stat statbuf;
#எனில் ENABLE_SELINUX
	security_context_t scontext = வெற்று;

	எனில் (option_mask32 & OPT_SELINUX) {
		எனில் ((option_mask32 & OPT_DEREFERENCE
		     ? lgetfilecon(filename, &scontext)
		     : getfilecon(filename, &scontext)
		    ) < 0
		) {
			bb_simple_perror_செய்தி(filename);
			திரும்பு 0;
		}
	}
#endif
	எனில் ((option_mask32 & OPT_DEREFERENCE ? stat : lstat) (filename, &statbuf) != 0) {
		bb_perror_செய்தி("can't stat '%s'", filename);
		திரும்பு 0;
	}

#எனில் ENABLE_FEATURE_STAT_FORMAT
	எனில் (format == வெற்று) {
# எனில் !ENABLE_SELINUX
		எனில் (option_mask32 & OPT_TERSE) {
			format = "%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %o";
		} இல்லையெனில் {
			எனில் (S_ISBLK(statbuf.st_mode) || S_ISCHR(statbuf.st_mode)) {
				format =
					"  File: %N\n"
					"  Size: %-10s\tBlocks: %-10b IO Block: %-6o %F\n"
					"Device: %Dh/%dd\tInode: %-10i  Links: %-5h"
					" Device type: %t,%T\n"
					"Access: (%04a/%10.10A)  Uid: (%5u/%8U)   Gid: (%5g/%8G)\n"
					"Access: %x\n" "Modify: %y\n" "Change: %z";
			} இல்லையெனில் {
				format =
					"  File: %N\n"
					"  Size: %-10s\tBlocks: %-10b IO Block: %-6o %F\n"
					"Device: %Dh/%dd\tInode: %-10i  Links: %h\n"
					"Access: (%04a/%10.10A)  Uid: (%5u/%8U)   Gid: (%5g/%8G)\n"
					"Access: %x\n" "Modify: %y\n" "Change: %z";
			}
		}
# இல்லையெனில்
		எனில் (option_mask32 & OPT_TERSE) {
			format = (option_mask32 & OPT_SELINUX ?
				"%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %o %C\n"
				:
				"%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %o\n"
				);
		} இல்லையெனில் {
			எனில் (S_ISBLK(statbuf.st_mode) || S_ISCHR(statbuf.st_mode)) {
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
			} இல்லையெனில் {
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
#இல்லையெனில்	/* FEATURE_STAT_FORMAT */
	எனில் (option_mask32 & OPT_TERSE) {
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
# எனில் ENABLE_SELINUX
		எனில் (option_mask32 & OPT_SELINUX)
			printf(" %s\n", scontext);
		இல்லையெனில்
			bb_putchar('\n');
# endif
	} இல்லையெனில் {
		எழுத்து modestr[12];
		எழுத்து *linkname = வெற்று;
		கட்டமைப்பு passwd *pw_ent;
		கட்டமைப்பு group *gw_ent;

		gw_ent = getgrgid(statbuf.st_gid);
		pw_ent = getpwuid(statbuf.st_uid);

		எனில் (S_ISLNK(statbuf.st_mode))
			linkname = xmalloc_readlink_or_warn(filename);
		எனில் (linkname) {
			printf("  File: '%s' -> '%s'\n", filename, linkname);
			விடுவி(linkname);
		} இல்லையெனில் {
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
		எனில் (S_ISBLK(statbuf.st_mode) || S_ISCHR(statbuf.st_mode))
			printf(" Device type: %lx,%lx\n",
			       (unsigned long) major(statbuf.st_rdev),
			       (unsigned long) minor(statbuf.st_rdev));
		இல்லையெனில்
			bb_putchar('\n');
		printf("Access: (%04lo/%10.10s)  Uid: (%5lu/%8s)   Gid: (%5lu/%8s)\n",
		       (unsigned long) (statbuf.st_mode & (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)),
		       bb_mode_string(modestr, statbuf.st_mode),
		       (unsigned long) statbuf.st_uid,
		       (pw_ent != வெற்று) ? pw_ent->pw_name : "UNKNOWN",
		       (unsigned long) statbuf.st_gid,
		       (gw_ent != வெற்று) ? gw_ent->gr_name : "UNKNOWN");
# எனில் ENABLE_SELINUX
		எனில் (option_mask32 & OPT_SELINUX)
			printf("   S_Context: %s\n", scontext);
# endif
		printf("Access: %s\n", human_time(&statbuf.st_atim));
		printf("Modify: %s\n", human_time(&statbuf.st_mtim));
		printf("Change: %s\n", human_time(&statbuf.st_ctim));
	}
#endif  /* FEATURE_STAT_FORMAT */
	திரும்பு 1;
}

முழு stat_main(முழு argc, எழுத்து **argv) MAIN_EXTERNALLY_VISIBLE;
முழு stat_main(முழு argc UNUSED_PARAM, எழுத்து **argv)
{
	IF_FEATURE_STAT_FORMAT(எழுத்து *format = வெற்று;)
	முழு i;
	முழு ok;
	statfunc_ptr statfunc = do_stat;
#எனில் ENABLE_FEATURE_STAT_FILESYSTEM || ENABLE_SELINUX
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
#எனில் ENABLE_FEATURE_STAT_FILESYSTEM
	எனில் (opts & OPT_FILESYS) /* -f */
		statfunc = do_statfs;
#endif
#எனில் ENABLE_SELINUX
	எனில் (opts & OPT_SELINUX) {
		selinux_or_die();
	}
#endif
	ok = 1;
	argv += optind;
	க்கு (i = 0; argv[i]; ++i)
		ok &= statfunc(argv[i] IF_FEATURE_STAT_FORMAT(, format));

	திரும்பு (ok ? EXIT_SUCCESS : EXIT_FAILURE);
}
