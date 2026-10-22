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

#شامل "libbb.h"
#شامل "common_bufsiz.h"

شمار {
	OPT_TERSE       = (1 << 0),
	OPT_DEREFERENCE = (1 << 1),
	OPT_FILESYS     = (1 << 2) * ENABLE_FEATURE_STAT_FILESYSTEM,
	OPT_SELINUX     = (1 << (2+ENABLE_FEATURE_STAT_FILESYSTEM)) * ENABLE_SELINUX,
};

#اگر ENABLE_FEATURE_STAT_FORMAT
قسمتعریف bool (*statfunc_ptr)(ثابت حرف *, ثابت حرف *);
#ورنہ
قسمتعریف bool (*statfunc_ptr)(ثابت حرف *);
#اگر_ختم

ساکن ثابت حرف *file_type(ثابت ساخت stat *st)
{
	/* See POSIX 1003.1-2001 XCU Table 4-8 lines 17093-17107
	 * for some of these formats.
	 * To keep diagnostics grammatical in English, the
	 * returned string must start with a consonant.
	 */
	اگر (S_ISREG(st->st_منوال))  واپس st->st_سائز == 0 ? "regular empty file" : "regular file";
	اگر (S_ISDIR(st->st_منوال))  واپس "directory";
	اگر (S_ISBLK(st->st_منوال))  واپس "block special file";
	اگر (S_ISCHR(st->st_منوال))  واپس "character special file";
	اگر (S_ISFIFO(st->st_منوال)) واپس "fifo";
	اگر (S_ISLNK(st->st_منوال))  واپس "symbolic link";
	اگر (S_ISSOCK(st->st_منوال)) واپس "socket";
#اگر_تعریف_شدہ S_TYPEISMQ
	اگر (S_TYPEISMQ(st))        واپس "message queue";
#اگر_ختم
#اگر_تعریف_شدہ S_TYPEISSEM
	اگر (S_TYPEISSEM(st))       واپس "semaphore";
#اگر_ختم
#اگر_تعریف_شدہ S_TYPEISSHM
	اگر (S_TYPEISSHM(st))       واپس "shared memory object";
#اگر_ختم
#اگر_تعریف_شدہ S_TYPEISTMO
	اگر (S_TYPEISTMO(st))       واپس "typed memory object";
#اگر_ختم
	واپس "weird file";
}

ساکن ثابت حرف *human_وقت(ساخت timespec *ts)
{
	حرف fmt[حجم("%Y-%m-%d %H:%M:%S.123456789 %z") + /*paranoia*/ 8];

	/* coreutils 6.3 compat */
#تعریف_کرو buf bb_common_bufsiz1
	تیاری_common_bufsiz();

	sprintf(stpcpy(fmt, "%Y-%m-%d %H:%M:%S"), ".%09u %%z", (غیرعلامتی)ts->tv_nsec);
	strftime(buf, COMMON_BUFSIZE, fmt, localtime(&ts->tv_sec));
	واپس buf;
#تعریف_منسوخ buf
}

#اگر ENABLE_FEATURE_STAT_FILESYSTEM
#تعریف_کرو FS_TYPE_LIST \
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
ساکن ثابت حرف *human_fstype(uint32_t f_type)
{
# define FS_TYPE(type, name) type,
	ساکن ثابت uint32_t fstype[] ALIGN4 = {
		FS_TYPE_LIST
	};
# undef FS_TYPE
# define FS_TYPE(type, name) name"\0"
	ساکن ثابت حرف humanname[] ALIGN1 =
		FS_TYPE_LIST
		"UNKNOWN";
# undef FS_TYPE
	صحیح i;

	برائے (i = 0; i < ARRAY_سائز(fstype); ++i)
		اگر (fstype[i] == f_type)
			توڑو;
	واپس nth_string(humanname, i);
}

/* "man statfs" says that statfsbuf->f_fsid is a mess */
/* coreutils treats it as an array of ints, most significant first */
ساکن غیرعلامتی طویل طویل get_f_fsid(ثابت ساخت statfs *statfsbuf)
{
	ثابت غیرعلامتی *p = (ثابت خالی*) &statfsbuf->f_fsid;
	غیرعلامتی sz = حجم(statfsbuf->f_fsid) / حجم(غیرعلامتی);
	غیرعلامتی طویل طویل r = 0;

	کرو
		r = (r << (حجم(غیرعلامتی)*8)) | *p++;
	جب (--sz > 0);
	واپس r;
}
#اگر_ختم  /* FEATURE_STAT_FILESYSTEM */

#اگر ENABLE_FEATURE_STAT_FORMAT
ساکن خالی strcatc(حرف *str, حرف c)
{
	صحیح len = لڑی_لمبائی(str);
	str[len++] = c;
	str[len] = '\0';
}

ساکن خالی printfs(حرف *pformat, ثابت حرف *پیغام)
{
	strcatc(pformat, 's');
	لکھو(pformat, پیغام);
}

#اگر ENABLE_FEATURE_STAT_FILESYSTEM
/* print statfs info */
ساکن خالی FAST_FUNC print_statfs(حرف *pformat, ثابت حرف m,
		ثابت حرف *ثابت filename, ثابت خالی *ڈیٹا
		IF_SELINUX(, security_context_t scontext))
{
	ثابت ساخت statfs *statfsbuf = ڈیٹا;
	اگر (m == 'n') {
		printfs(pformat, filename);
	} ورنہ اگر (m == 'i') {
		strcat(pformat, "llx");
		لکھو(pformat, get_f_fsid(statfsbuf));
	} ورنہ اگر (m == 'l') {
		strcat(pformat, "lu");
		لکھو(pformat, (غیرعلامتی طویل) statfsbuf->f_namelen);
	} ورنہ اگر (m == 't') {
		strcat(pformat, "lx");
		لکھو(pformat, (غیرعلامتی طویل) statfsbuf->f_type); /* no equiv */
	} ورنہ اگر (m == 'T') {
		printfs(pformat, human_fstype(statfsbuf->f_type));
	} ورنہ اگر (m == 'b') {
		strcat(pformat, "llu");
		لکھو(pformat, (غیرعلامتی طویل طویل) statfsbuf->f_blocks);
	} ورنہ اگر (m == 'f') {
		strcat(pformat, "llu");
		لکھو(pformat, (غیرعلامتی طویل طویل) statfsbuf->f_bfree);
	} ورنہ اگر (m == 'a') {
		strcat(pformat, "llu");
		لکھو(pformat, (غیرعلامتی طویل طویل) statfsbuf->f_bavail);
	} ورنہ اگر (m == 's' || m == 'S') {
		strcat(pformat, "lu");
		لکھو(pformat, (غیرعلامتی طویل) statfsbuf->f_bsize);
	} ورنہ اگر (m == 'c') {
		strcat(pformat, "llu");
		لکھو(pformat, (غیرعلامتی طویل طویل) statfsbuf->f_files);
	} ورنہ اگر (m == 'd') {
		strcat(pformat, "llu");
		لکھو(pformat, (غیرعلامتی طویل طویل) statfsbuf->f_ffree);
# اگر ENABLE_SELINUX
	} ورنہ اگر (m == 'C' && (option_mask32 & OPT_SELINUX)) {
		printfs(pformat, scontext);
# endif
	} ورنہ {
		strcatc(pformat, 'c');
		لکھو(pformat, m);
	}
}
#اگر_ختم

/* print stat info */
ساکن خالی FAST_FUNC print_stat(حرف *pformat, ثابت حرف m,
		ثابت حرف *ثابت filename, ثابت خالی *ڈیٹا
		IF_SELINUX(, security_context_t scontext))
{
#تعریف_کرو TYPE_SIGNED(t) (! ((t) 0 < (t) -1))
	ساخت stat *statbuf = (ساخت stat *) ڈیٹا;
	ساخت passwd *pw_ent;
	ساخت گروہ *gw_ent;

	اگر (m == 'n') {
		printfs(pformat, filename);
	} ورنہ اگر (m == 'N') {
		strcatc(pformat, 's');
		اگر (S_ISLNK(statbuf->st_منوال)) {
			حرف *linkname = xmalloc_readlink_or_warn(filename);
			اگر (linkname == تہی)
				واپس;
			لکھو("'%s' -> '%s'", filename, linkname);
			آزاد(linkname);
		} ورنہ {
			لکھو(pformat, filename);
		}
	} ورنہ اگر (m == 'd') {
		strcat(pformat, "llu");
		لکھو(pformat, (غیرعلامتی طویل طویل) statbuf->st_dev);
	} ورنہ اگر (m == 'D') {
		strcat(pformat, "llx");
		لکھو(pformat, (غیرعلامتی طویل طویل) statbuf->st_dev);
	} ورنہ اگر (m == 'i') {
		strcat(pformat, "llu");
		لکھو(pformat, (غیرعلامتی طویل طویل) statbuf->st_ino);
	} ورنہ اگر (m == 'a') {
		strcat(pformat, "lo");
		لکھو(pformat, (غیرعلامتی طویل) (statbuf->st_منوال & (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)));
	} ورنہ اگر (m == 'A') {
		حرف modestr[12];
		printfs(pformat, bb_منوال_string(modestr, statbuf->st_منوال));
	} ورنہ اگر (m == 'f') {
		strcat(pformat, "lx");
		لکھو(pformat, (غیرعلامتی طویل) statbuf->st_منوال);
	} ورنہ اگر (m == 'F') {
		printfs(pformat, file_type(statbuf));
	} ورنہ اگر (m == 'h') {
		strcat(pformat, "lu");
		لکھو(pformat, (غیرعلامتی طویل) statbuf->st_nlink);
	} ورنہ اگر (m == 'u') {
		strcat(pformat, "lu");
		لکھو(pformat, (غیرعلامتی طویل) statbuf->st_uid);
	} ورنہ اگر (m == 'U') {
		pw_ent = getpwuid(statbuf->st_uid);
		printfs(pformat, (pw_ent != تہی) ? pw_ent->pw_name : "UNKNOWN");
	} ورنہ اگر (m == 'g') {
		strcat(pformat, "lu");
		لکھو(pformat, (غیرعلامتی طویل) statbuf->st_gid);
	} ورنہ اگر (m == 'G') {
		gw_ent = getgrgid(statbuf->st_gid);
		printfs(pformat, (gw_ent != تہی) ? gw_ent->gr_name : "UNKNOWN");
	} ورنہ اگر (m == 't') {
		strcat(pformat, "lx");
		لکھو(pformat, (غیرعلامتی طویل) major(statbuf->st_rdev));
	} ورنہ اگر (m == 'T') {
		strcat(pformat, "lx");
		لکھو(pformat, (غیرعلامتی طویل) minor(statbuf->st_rdev));
	} ورنہ اگر (m == 's') {
		strcat(pformat, "llu");
		لکھو(pformat, (غیرعلامتی طویل طویل) statbuf->st_سائز);
	} ورنہ اگر (m == 'B') {
		strcat(pformat, "lu");
		لکھو(pformat, (غیرعلامتی طویل) 512); //ST_NBLOCKSIZE
	} ورنہ اگر (m == 'b') {
		strcat(pformat, "llu");
		لکھو(pformat, (غیرعلامتی طویل طویل) statbuf->st_بلاکیں);
	} ورنہ اگر (m == 'o') {
		strcat(pformat, "lu");
		لکھو(pformat, (غیرعلامتی طویل) statbuf->st_blksize);
	} ورنہ اگر (m == 'x') {
		printfs(pformat, human_وقت(&statbuf->st_atim));
	} ورنہ اگر (m == 'X') {
		strcat(pformat, TYPE_SIGNED(وقت_t) ? "ld" : "lu");
		/* note: (unsigned long) would be wrong:
		 * imagine (unsigned long64)int32 */
		لکھو(pformat, (طویل) statbuf->st_atime);
	} ورنہ اگر (m == 'y') {
		printfs(pformat, human_وقت(&statbuf->st_mtim));
	} ورنہ اگر (m == 'Y') {
		strcat(pformat, TYPE_SIGNED(وقت_t) ? "ld" : "lu");
		لکھو(pformat, (طویل) statbuf->st_mtime);
	} ورنہ اگر (m == 'z') {
		printfs(pformat, human_وقت(&statbuf->st_ctim));
	} ورنہ اگر (m == 'Z') {
		strcat(pformat, TYPE_SIGNED(وقت_t) ? "ld" : "lu");
		لکھو(pformat, (طویل) statbuf->st_ctime);
# اگر ENABLE_SELINUX
	} ورنہ اگر (m == 'C' && (option_mask32 & OPT_SELINUX)) {
		printfs(pformat, scontext);
# endif
	} ورنہ {
		strcatc(pformat, 'c');
		لکھو(pformat, m);
	}
}

ساکن خالی print_it(ثابت حرف *masterformat,
		ثابت حرف *filename,
		خالی FAST_FUNC (*print_func)(حرف*, حرف, ثابت حرف*, ثابت خالی* IF_SELINUX(, security_context_t scontext)),
		ثابت خالی *ڈیٹا
		IF_SELINUX(, security_context_t scontext))
{
	/* Create a working copy of the format string */
	حرف *format = xstrdup(masterformat);
	/* Add 2 to accommodate our conversion of the stat '%s' format string
	 * to the printf '%llu' one.  */
	حرف *dest = xmalloc(لڑی_لمبائی(format) + 2 + 1);
	حرف *b;

	b = format;
	جب (b) {
		/* Each iteration finds next %spec,
		 * prints preceding string and handles found %spec
		 */
		سائز_t len;
		حرف *p = strchr(b, '%');
		اگر (!p) {
			/* coreutils 6.3 always prints newline at the end */
			/*fputs(b, stdout);*/
			puts(b);
			توڑو;
		}

		/* dest = "%<modifiers>" */
		len = 1 + strspn(p + 1, "#-+.I 0123456789");
		حافظہ_نقل(dest, p, len);
		dest[len] = '\0';

		/* print preceding string */
		*p = '\0';
		fputs_stdout(b);

		p += len;
		b = p + 1;
		انتخاب (*p) {
		حالت '\0':
			b = تہی;
			/* fall through */
		حالت '%':
			bb_putchar('%');
			توڑو;
		طےشدہ:
			/* Completes "%<modifiers>" with specifier and printfs */
			print_func(dest, *p, filename, ڈیٹا IF_SELINUX(,scontext));
			توڑو;
		}
	}

	آزاد(format);
	آزاد(dest);
}
#اگر_ختم  /* FEATURE_STAT_FORMAT */

#اگر ENABLE_FEATURE_STAT_FILESYSTEM
/* Stat the file system and print what we find.  */
#اگر !ENABLE_FEATURE_STAT_FORMAT
#تعریف_کرو do_statfs(filename, format) do_statfs(filename)
#اگر_ختم
ساکن bool do_statfs(ثابت حرف *filename, ثابت حرف *format)
{
	ساخت statfs statfsbuf;
#اگر !ENABLE_FEATURE_STAT_FORMAT
	ثابت حرف *format;
#اگر_ختم
#اگر ENABLE_SELINUX
	security_context_t scontext = تہی;

	اگر (option_mask32 & OPT_SELINUX) {
		اگر ((option_mask32 & OPT_DEREFERENCE
		     ? lgetfilecon(filename, &scontext)
		     : getfilecon(filename, &scontext)
		    ) < 0
		) {
			bb_simple_perror_پیغام(filename);
			واپس 0;
		}
	}
#اگر_ختم
	اگر (statfs(filename, &statfsbuf) != 0) {
		bb_perror_پیغام("can't read file system information for '%s'", filename);
		واپس 0;
	}

#اگر ENABLE_FEATURE_STAT_FORMAT
	اگر (format == تہی) {
# اگر !ENABLE_SELINUX
		format = (option_mask32 & OPT_TERSE
			? "%n %i %l %t %s %b %f %a %c %d"
			: "  File: \"%n\"\n"
			  "    ID: %-8i Namelen: %-7l Type: %T\n"
			  "Block size: %-10s\n"
			  "Blocks: Total: %-10b Free: %-10f Available: %a\n"
			  "Inodes: Total: %-10c Free: %d");
# ورنہ
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
#ورنہ /* !FEATURE_STAT_FORMAT */
	format = (option_mask32 & OPT_TERSE
		? "%s %llx %lu "
		: "  File: \"%s\"\n"
		  "    ID: %-8llx Namelen: %-7lu ");
	لکھو(format,
	       filename,
	       get_f_fsid(&statfsbuf),
	       statfsbuf.f_namelen);

	اگر (option_mask32 & OPT_TERSE)
		لکھو("%lx ", (غیرعلامتی طویل) statfsbuf.f_type);
	ورنہ
		لکھو("Type: %s\n", human_fstype(statfsbuf.f_type));

# اگر !ENABLE_SELINUX
	format = (option_mask32 & OPT_TERSE
		? "%lu %llu %llu %llu %llu %llu\n"
		: "Block size: %-10lu\n"
		  "Blocks: Total: %-10llu Free: %-10llu Available: %llu\n"
		  "Inodes: Total: %-10llu Free: %llu\n");
	لکھو(format,
	       (غیرعلامتی طویل) statfsbuf.f_bsize,
	       (غیرعلامتی طویل طویل) statfsbuf.f_blocks,
	       (غیرعلامتی طویل طویل) statfsbuf.f_bfree,
	       (غیرعلامتی طویل طویل) statfsbuf.f_bavail,
	       (غیرعلامتی طویل طویل) statfsbuf.f_files,
	       (غیرعلامتی طویل طویل) statfsbuf.f_ffree);
# ورنہ
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
	لکھو(format,
		(غیرعلامتی طویل) statfsbuf.f_bsize,
		(غیرعلامتی طویل طویل) statfsbuf.f_blocks,
		(غیرعلامتی طویل طویل) statfsbuf.f_bfree,
		(غیرعلامتی طویل طویل) statfsbuf.f_bavail,
		(غیرعلامتی طویل طویل) statfsbuf.f_files,
		(غیرعلامتی طویل طویل) statfsbuf.f_ffree,
		scontext);

	اگر (scontext)
		freecon(scontext);
# endif
#اگر_ختم  /* FEATURE_STAT_FORMAT */
	واپس 1;
}
#اگر_ختم  /* FEATURE_STAT_FILESYSTEM */

/* stat the file and print what we find */
#اگر !ENABLE_FEATURE_STAT_FORMAT
#تعریف_کرو do_stat(filename, format) do_stat(filename)
#اگر_ختم
ساکن bool do_stat(ثابت حرف *filename, ثابت حرف *format)
{
	ساخت stat statbuf;
#اگر ENABLE_SELINUX
	security_context_t scontext = تہی;

	اگر (option_mask32 & OPT_SELINUX) {
		اگر ((option_mask32 & OPT_DEREFERENCE
		     ? lgetfilecon(filename, &scontext)
		     : getfilecon(filename, &scontext)
		    ) < 0
		) {
			bb_simple_perror_پیغام(filename);
			واپس 0;
		}
	}
#اگر_ختم
	اگر ((option_mask32 & OPT_DEREFERENCE ? stat : lstat) (filename, &statbuf) != 0) {
		bb_perror_پیغام("can't stat '%s'", filename);
		واپس 0;
	}

#اگر ENABLE_FEATURE_STAT_FORMAT
	اگر (format == تہی) {
# اگر !ENABLE_SELINUX
		اگر (option_mask32 & OPT_TERSE) {
			format = "%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %o";
		} ورنہ {
			اگر (S_ISBLK(statbuf.st_منوال) || S_ISCHR(statbuf.st_منوال)) {
				format =
					"  File: %N\n"
					"  Size: %-10s\tBlocks: %-10b IO Block: %-6o %F\n"
					"Device: %Dh/%dd\tInode: %-10i  Links: %-5h"
					" Device type: %t,%T\n"
					"Access: (%04a/%10.10A)  Uid: (%5u/%8U)   Gid: (%5g/%8G)\n"
					"Access: %x\n" "Modify: %y\n" "Change: %z";
			} ورنہ {
				format =
					"  File: %N\n"
					"  Size: %-10s\tBlocks: %-10b IO Block: %-6o %F\n"
					"Device: %Dh/%dd\tInode: %-10i  Links: %h\n"
					"Access: (%04a/%10.10A)  Uid: (%5u/%8U)   Gid: (%5g/%8G)\n"
					"Access: %x\n" "Modify: %y\n" "Change: %z";
			}
		}
# ورنہ
		اگر (option_mask32 & OPT_TERSE) {
			format = (option_mask32 & OPT_SELINUX ?
				"%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %o %C\n"
				:
				"%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %o\n"
				);
		} ورنہ {
			اگر (S_ISBLK(statbuf.st_منوال) || S_ISCHR(statbuf.st_منوال)) {
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
			} ورنہ {
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
#ورنہ	/* FEATURE_STAT_FORMAT */
	اگر (option_mask32 & OPT_TERSE) {
		لکھو("%s %llu %llu %lx %lu %lu %llx %llu %lu %lx %lx %lu %lu %lu %lu"
		       IF_NOT_SELINUX("\n"),
		       filename,
		       (غیرعلامتی طویل طویل) statbuf.st_سائز,
		       (غیرعلامتی طویل طویل) statbuf.st_بلاکیں,
		       (غیرعلامتی طویل) statbuf.st_منوال,
		       (غیرعلامتی طویل) statbuf.st_uid,
		       (غیرعلامتی طویل) statbuf.st_gid,
		       (غیرعلامتی طویل طویل) statbuf.st_dev,
		       (غیرعلامتی طویل طویل) statbuf.st_ino,
		       (غیرعلامتی طویل) statbuf.st_nlink,
		       (غیرعلامتی طویل) major(statbuf.st_rdev),
		       (غیرعلامتی طویل) minor(statbuf.st_rdev),
		       (غیرعلامتی طویل) statbuf.st_atime,
		       (غیرعلامتی طویل) statbuf.st_mtime,
		       (غیرعلامتی طویل) statbuf.st_ctime,
		       (غیرعلامتی طویل) statbuf.st_blksize
		);
# اگر ENABLE_SELINUX
		اگر (option_mask32 & OPT_SELINUX)
			لکھو(" %s\n", scontext);
		ورنہ
			bb_putchar('\n');
# endif
	} ورنہ {
		حرف modestr[12];
		حرف *linkname = تہی;
		ساخت passwd *pw_ent;
		ساخت گروہ *gw_ent;

		gw_ent = getgrgid(statbuf.st_gid);
		pw_ent = getpwuid(statbuf.st_uid);

		اگر (S_ISLNK(statbuf.st_منوال))
			linkname = xmalloc_readlink_or_warn(filename);
		اگر (linkname) {
			لکھو("  File: '%s' -> '%s'\n", filename, linkname);
			آزاد(linkname);
		} ورنہ {
			لکھو("  File: '%s'\n", filename);
		}

		لکھو("  Size: %-10llu\tBlocks: %-10llu IO Block: %-6lu %s\n"
		       "Device: %llxh/%llud\tInode: %-10llu  Links: %-5lu",
		       (غیرعلامتی طویل طویل) statbuf.st_سائز,
		       (غیرعلامتی طویل طویل) statbuf.st_بلاکیں,
		       (غیرعلامتی طویل) statbuf.st_blksize,
		       file_type(&statbuf),
		       (غیرعلامتی طویل طویل) statbuf.st_dev,
		       (غیرعلامتی طویل طویل) statbuf.st_dev,
		       (غیرعلامتی طویل طویل) statbuf.st_ino,
		       (غیرعلامتی طویل) statbuf.st_nlink);
		اگر (S_ISBLK(statbuf.st_منوال) || S_ISCHR(statbuf.st_منوال))
			لکھو(" Device type: %lx,%lx\n",
			       (غیرعلامتی طویل) major(statbuf.st_rdev),
			       (غیرعلامتی طویل) minor(statbuf.st_rdev));
		ورنہ
			bb_putchar('\n');
		لکھو("Access: (%04lo/%10.10s)  Uid: (%5lu/%8s)   Gid: (%5lu/%8s)\n",
		       (غیرعلامتی طویل) (statbuf.st_منوال & (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)),
		       bb_منوال_string(modestr, statbuf.st_منوال),
		       (غیرعلامتی طویل) statbuf.st_uid,
		       (pw_ent != تہی) ? pw_ent->pw_name : "UNKNOWN",
		       (غیرعلامتی طویل) statbuf.st_gid,
		       (gw_ent != تہی) ? gw_ent->gr_name : "UNKNOWN");
# اگر ENABLE_SELINUX
		اگر (option_mask32 & OPT_SELINUX)
			لکھو("   S_Context: %s\n", scontext);
# endif
		لکھو("Access: %s\n", human_وقت(&statbuf.st_atim));
		لکھو("Modify: %s\n", human_وقت(&statbuf.st_mtim));
		لکھو("Change: %s\n", human_وقت(&statbuf.st_ctim));
	}
#اگر_ختم  /* FEATURE_STAT_FORMAT */
	واپس 1;
}

صحیح stat_main(صحیح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح stat_main(صحیح argc غیر_مستعمل_PARAM, حرف **argv)
{
	IF_FEATURE_STAT_FORMAT(حرف *format = تہی;)
	صحیح i;
	صحیح ok;
	statfunc_ptr statfunc = do_stat;
#اگر ENABLE_FEATURE_STAT_FILESYSTEM || ENABLE_SELINUX
	غیرعلامتی opts;

	opts =
#اگر_ختم
	getopt32(argv, "^"
		"tL"
		IF_FEATURE_STAT_FILESYSTEM("f")
		IF_SELINUX("Z")
		IF_FEATURE_STAT_FORMAT("c:")
		"\0" "-1" /* min one arg */
		IF_FEATURE_STAT_FORMAT(,&format)
	);
#اگر ENABLE_FEATURE_STAT_FILESYSTEM
	اگر (opts & OPT_FILESYS) /* -f */
		statfunc = do_statfs;
#اگر_ختم
#اگر ENABLE_SELINUX
	اگر (opts & OPT_SELINUX) {
		selinux_or_die();
	}
#اگر_ختم
	ok = 1;
	argv += optind;
	برائے (i = 0; argv[i]; ++i)
		ok &= statfunc(argv[i] IF_FEATURE_STAT_FORMAT(, format));

	واپس (ok ? EXIT_SUCCESS : EXIT_FAILURE);
}
