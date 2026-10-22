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

#ضمن "libbb.h"
#ضمن "common_bufsiz.h"

تعداد {
	OPT_TERSE       = (1 << 0),
	OPT_DEREFERENCE = (1 << 1),
	OPT_FILESYS     = (1 << 2) * ENABLE_FEATURE_STAT_FILESYSTEM,
	OPT_SELINUX     = (1 << (2+ENABLE_FEATURE_STAT_FILESYSTEM)) * ENABLE_SELINUX,
};

#إذا ENABLE_FEATURE_STAT_FORMAT
تعريفنوع bool (*statfunc_ptr)(ثابت حرف *, ثابت حرف *);
#وإلا
تعريفنوع bool (*statfunc_ptr)(ثابت حرف *);
#پایان_اگر

ساكن ثابت حرف *file_type(ثابت بنية stat *st)
{
	/* See POSIX 1003.1-2001 XCU Table 4-8 lines 17093-17107
	 * for some of these formats.
	 * To keep diagnostics grammatical in English, the
	 * returned string must start with a consonant.
	 */
	إذا (S_ISREG(st->st_نما))  أرجع st->st_اندازه == 0 ? "regular empty file" : "regular file";
	إذا (S_ISDIR(st->st_نما))  أرجع "directory";
	إذا (S_ISBLK(st->st_نما))  أرجع "block special file";
	إذا (S_ISCHR(st->st_نما))  أرجع "character special file";
	إذا (S_ISFIFO(st->st_نما)) أرجع "fifo";
	إذا (S_ISLNK(st->st_نما))  أرجع "symbolic link";
	إذا (S_ISSOCK(st->st_نما)) أرجع "socket";
#اگر_تعریف_شده S_TYPEISMQ
	إذا (S_TYPEISMQ(st))        أرجع "message queue";
#پایان_اگر
#اگر_تعریف_شده S_TYPEISSEM
	إذا (S_TYPEISSEM(st))       أرجع "semaphore";
#پایان_اگر
#اگر_تعریف_شده S_TYPEISSHM
	إذا (S_TYPEISSHM(st))       أرجع "shared memory object";
#پایان_اگر
#اگر_تعریف_شده S_TYPEISTMO
	إذا (S_TYPEISTMO(st))       أرجع "typed memory object";
#پایان_اگر
	أرجع "weird file";
}

ساكن ثابت حرف *human_زمان(بنية timespec *ts)
{
	حرف fmt[حجم("%Y-%m-%d %H:%M:%S.123456789 %z") + /*paranoia*/ 8];

	/* coreutils 6.3 compat */
#تعریف_کن buf bb_common_bufsiz1
	راهاندازی_common_bufsiz();

	sprintf(stpcpy(fmt, "%Y-%m-%d %H:%M:%S"), ".%09u %%z", (بیعلامت)ts->tv_nsec);
	strftime(buf, COMMON_BUFSIZE, fmt, localtime(&ts->tv_sec));
	أرجع buf;
#لغو_تعریف buf
}

#إذا ENABLE_FEATURE_STAT_FILESYSTEM
#تعریف_کن FS_TYPE_LIST \
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
ساكن ثابت حرف *human_fstype(uint32_t f_type)
{
# define FS_TYPE(type, name) type,
	ساكن ثابت uint32_t fstype[] ALIGN4 = {
		FS_TYPE_LIST
	};
# undef FS_TYPE
# define FS_TYPE(type, name) name"\0"
	ساكن ثابت حرف humanname[] ALIGN1 =
		FS_TYPE_LIST
		"UNKNOWN";
# undef FS_TYPE
	صحيح i;

	لكل (i = 0; i < ARRAY_اندازه(fstype); ++i)
		إذا (fstype[i] == f_type)
			اكسر;
	أرجع nth_string(humanname, i);
}

/* "man statfs" says that statfsbuf->f_fsid is a mess */
/* coreutils treats it as an array of ints, most significant first */
ساكن بیعلامت دراز دراز get_f_fsid(ثابت بنية statfs *statfsbuf)
{
	ثابت بیعلامت *p = (ثابت فراغ*) &statfsbuf->f_fsid;
	بیعلامت sz = حجم(statfsbuf->f_fsid) / حجم(بیعلامت);
	بیعلامت دراز دراز r = 0;

	افعل
		r = (r << (حجم(بیعلامت)*8)) | *p++;
	طالما (--sz > 0);
	أرجع r;
}
#پایان_اگر  /* FEATURE_STAT_FILESYSTEM */

#إذا ENABLE_FEATURE_STAT_FORMAT
ساكن فراغ strcatc(حرف *str, حرف c)
{
	صحيح len = طول_رشته(str);
	str[len++] = c;
	str[len] = '\0';
}

ساكن فراغ printfs(حرف *pformat, ثابت حرف *پیام)
{
	strcatc(pformat, 's');
	چاپ_کن(pformat, پیام);
}

#إذا ENABLE_FEATURE_STAT_FILESYSTEM
/* print statfs info */
ساكن فراغ FAST_FUNC print_statfs(حرف *pformat, ثابت حرف m,
		ثابت حرف *ثابت filename, ثابت فراغ *داده
		IF_SELINUX(, security_context_t scontext))
{
	ثابت بنية statfs *statfsbuf = داده;
	إذا (m == 'n') {
		printfs(pformat, filename);
	} وإلا إذا (m == 'i') {
		strcat(pformat, "llx");
		چاپ_کن(pformat, get_f_fsid(statfsbuf));
	} وإلا إذا (m == 'l') {
		strcat(pformat, "lu");
		چاپ_کن(pformat, (بیعلامت دراز) statfsbuf->f_namelen);
	} وإلا إذا (m == 't') {
		strcat(pformat, "lx");
		چاپ_کن(pformat, (بیعلامت دراز) statfsbuf->f_type); /* no equiv */
	} وإلا إذا (m == 'T') {
		printfs(pformat, human_fstype(statfsbuf->f_type));
	} وإلا إذا (m == 'b') {
		strcat(pformat, "llu");
		چاپ_کن(pformat, (بیعلامت دراز دراز) statfsbuf->f_blocks);
	} وإلا إذا (m == 'f') {
		strcat(pformat, "llu");
		چاپ_کن(pformat, (بیعلامت دراز دراز) statfsbuf->f_bfree);
	} وإلا إذا (m == 'a') {
		strcat(pformat, "llu");
		چاپ_کن(pformat, (بیعلامت دراز دراز) statfsbuf->f_bavail);
	} وإلا إذا (m == 's' || m == 'S') {
		strcat(pformat, "lu");
		چاپ_کن(pformat, (بیعلامت دراز) statfsbuf->f_bsize);
	} وإلا إذا (m == 'c') {
		strcat(pformat, "llu");
		چاپ_کن(pformat, (بیعلامت دراز دراز) statfsbuf->f_files);
	} وإلا إذا (m == 'd') {
		strcat(pformat, "llu");
		چاپ_کن(pformat, (بیعلامت دراز دراز) statfsbuf->f_ffree);
# إذا ENABLE_SELINUX
	} وإلا إذا (m == 'C' && (option_mask32 & OPT_SELINUX)) {
		printfs(pformat, scontext);
# endif
	} وإلا {
		strcatc(pformat, 'c');
		چاپ_کن(pformat, m);
	}
}
#پایان_اگر

/* print stat info */
ساكن فراغ FAST_FUNC print_stat(حرف *pformat, ثابت حرف m,
		ثابت حرف *ثابت filename, ثابت فراغ *داده
		IF_SELINUX(, security_context_t scontext))
{
#تعریف_کن TYPE_SIGNED(t) (! ((t) 0 < (t) -1))
	بنية stat *statbuf = (بنية stat *) داده;
	بنية passwd *pw_ent;
	بنية گروه *gw_ent;

	إذا (m == 'n') {
		printfs(pformat, filename);
	} وإلا إذا (m == 'N') {
		strcatc(pformat, 's');
		إذا (S_ISLNK(statbuf->st_نما)) {
			حرف *linkname = xmalloc_readlink_or_warn(filename);
			إذا (linkname == عدم)
				أرجع;
			چاپ_کن("'%s' -> '%s'", filename, linkname);
			حرر(linkname);
		} وإلا {
			چاپ_کن(pformat, filename);
		}
	} وإلا إذا (m == 'd') {
		strcat(pformat, "llu");
		چاپ_کن(pformat, (بیعلامت دراز دراز) statbuf->st_dev);
	} وإلا إذا (m == 'D') {
		strcat(pformat, "llx");
		چاپ_کن(pformat, (بیعلامت دراز دراز) statbuf->st_dev);
	} وإلا إذا (m == 'i') {
		strcat(pformat, "llu");
		چاپ_کن(pformat, (بیعلامت دراز دراز) statbuf->st_ino);
	} وإلا إذا (m == 'a') {
		strcat(pformat, "lo");
		چاپ_کن(pformat, (بیعلامت دراز) (statbuf->st_نما & (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)));
	} وإلا إذا (m == 'A') {
		حرف modestr[12];
		printfs(pformat, bb_نما_string(modestr, statbuf->st_نما));
	} وإلا إذا (m == 'f') {
		strcat(pformat, "lx");
		چاپ_کن(pformat, (بیعلامت دراز) statbuf->st_نما);
	} وإلا إذا (m == 'F') {
		printfs(pformat, file_type(statbuf));
	} وإلا إذا (m == 'h') {
		strcat(pformat, "lu");
		چاپ_کن(pformat, (بیعلامت دراز) statbuf->st_nlink);
	} وإلا إذا (m == 'u') {
		strcat(pformat, "lu");
		چاپ_کن(pformat, (بیعلامت دراز) statbuf->st_uid);
	} وإلا إذا (m == 'U') {
		pw_ent = getpwuid(statbuf->st_uid);
		printfs(pformat, (pw_ent != عدم) ? pw_ent->pw_name : "UNKNOWN");
	} وإلا إذا (m == 'g') {
		strcat(pformat, "lu");
		چاپ_کن(pformat, (بیعلامت دراز) statbuf->st_gid);
	} وإلا إذا (m == 'G') {
		gw_ent = getgrgid(statbuf->st_gid);
		printfs(pformat, (gw_ent != عدم) ? gw_ent->gr_name : "UNKNOWN");
	} وإلا إذا (m == 't') {
		strcat(pformat, "lx");
		چاپ_کن(pformat, (بیعلامت دراز) major(statbuf->st_rdev));
	} وإلا إذا (m == 'T') {
		strcat(pformat, "lx");
		چاپ_کن(pformat, (بیعلامت دراز) minor(statbuf->st_rdev));
	} وإلا إذا (m == 's') {
		strcat(pformat, "llu");
		چاپ_کن(pformat, (بیعلامت دراز دراز) statbuf->st_اندازه);
	} وإلا إذا (m == 'B') {
		strcat(pformat, "lu");
		چاپ_کن(pformat, (بیعلامت دراز) 512); //ST_NBLOCKSIZE
	} وإلا إذا (m == 'b') {
		strcat(pformat, "llu");
		چاپ_کن(pformat, (بیعلامت دراز دراز) statbuf->st_بلوکها);
	} وإلا إذا (m == 'o') {
		strcat(pformat, "lu");
		چاپ_کن(pformat, (بیعلامت دراز) statbuf->st_blksize);
	} وإلا إذا (m == 'x') {
		printfs(pformat, human_زمان(&statbuf->st_atim));
	} وإلا إذا (m == 'X') {
		strcat(pformat, TYPE_SIGNED(زمان_t) ? "ld" : "lu");
		/* note: (unsigned long) would be wrong:
		 * imagine (unsigned long64)int32 */
		چاپ_کن(pformat, (دراز) statbuf->st_atime);
	} وإلا إذا (m == 'y') {
		printfs(pformat, human_زمان(&statbuf->st_mtim));
	} وإلا إذا (m == 'Y') {
		strcat(pformat, TYPE_SIGNED(زمان_t) ? "ld" : "lu");
		چاپ_کن(pformat, (دراز) statbuf->st_mtime);
	} وإلا إذا (m == 'z') {
		printfs(pformat, human_زمان(&statbuf->st_ctim));
	} وإلا إذا (m == 'Z') {
		strcat(pformat, TYPE_SIGNED(زمان_t) ? "ld" : "lu");
		چاپ_کن(pformat, (دراز) statbuf->st_ctime);
# إذا ENABLE_SELINUX
	} وإلا إذا (m == 'C' && (option_mask32 & OPT_SELINUX)) {
		printfs(pformat, scontext);
# endif
	} وإلا {
		strcatc(pformat, 'c');
		چاپ_کن(pformat, m);
	}
}

ساكن فراغ print_it(ثابت حرف *masterformat,
		ثابت حرف *filename,
		فراغ FAST_FUNC (*print_func)(حرف*, حرف, ثابت حرف*, ثابت فراغ* IF_SELINUX(, security_context_t scontext)),
		ثابت فراغ *داده
		IF_SELINUX(, security_context_t scontext))
{
	/* Create a working copy of the format string */
	حرف *format = xstrdup(masterformat);
	/* Add 2 to accommodate our conversion of the stat '%s' format string
	 * to the printf '%llu' one.  */
	حرف *dest = xmalloc(طول_رشته(format) + 2 + 1);
	حرف *b;

	b = format;
	طالما (b) {
		/* Each iteration finds next %spec,
		 * prints preceding string and handles found %spec
		 */
		اندازه_t len;
		حرف *p = strchr(b, '%');
		إذا (!p) {
			/* coreutils 6.3 always prints newline at the end */
			/*fputs(b, stdout);*/
			puts(b);
			اكسر;
		}

		/* dest = "%<modifiers>" */
		len = 1 + strspn(p + 1, "#-+.I 0123456789");
		کپی_حافظه(dest, p, len);
		dest[len] = '\0';

		/* print preceding string */
		*p = '\0';
		fputs_stdout(b);

		p += len;
		b = p + 1;
		بدل (*p) {
		حالة '\0':
			b = عدم;
			/* fall through */
		حالة '%':
			bb_putchar('%');
			اكسر;
		افتراضي:
			/* Completes "%<modifiers>" with specifier and printfs */
			print_func(dest, *p, filename, داده IF_SELINUX(,scontext));
			اكسر;
		}
	}

	حرر(format);
	حرر(dest);
}
#پایان_اگر  /* FEATURE_STAT_FORMAT */

#إذا ENABLE_FEATURE_STAT_FILESYSTEM
/* Stat the file system and print what we find.  */
#إذا !ENABLE_FEATURE_STAT_FORMAT
#تعریف_کن do_statfs(filename, format) do_statfs(filename)
#پایان_اگر
ساكن bool do_statfs(ثابت حرف *filename, ثابت حرف *format)
{
	بنية statfs statfsbuf;
#إذا !ENABLE_FEATURE_STAT_FORMAT
	ثابت حرف *format;
#پایان_اگر
#إذا ENABLE_SELINUX
	security_context_t scontext = عدم;

	إذا (option_mask32 & OPT_SELINUX) {
		إذا ((option_mask32 & OPT_DEREFERENCE
		     ? lgetfilecon(filename, &scontext)
		     : getfilecon(filename, &scontext)
		    ) < 0
		) {
			bb_simple_perror_پیام(filename);
			أرجع 0;
		}
	}
#پایان_اگر
	إذا (statfs(filename, &statfsbuf) != 0) {
		bb_perror_پیام("can't read file system information for '%s'", filename);
		أرجع 0;
	}

#إذا ENABLE_FEATURE_STAT_FORMAT
	إذا (format == عدم) {
# إذا !ENABLE_SELINUX
		format = (option_mask32 & OPT_TERSE
			? "%n %i %l %t %s %b %f %a %c %d"
			: "  File: \"%n\"\n"
			  "    ID: %-8i Namelen: %-7l Type: %T\n"
			  "Block size: %-10s\n"
			  "Blocks: Total: %-10b Free: %-10f Available: %a\n"
			  "Inodes: Total: %-10c Free: %d");
# وإلا
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
#وإلا /* !FEATURE_STAT_FORMAT */
	format = (option_mask32 & OPT_TERSE
		? "%s %llx %lu "
		: "  File: \"%s\"\n"
		  "    ID: %-8llx Namelen: %-7lu ");
	چاپ_کن(format,
	       filename,
	       get_f_fsid(&statfsbuf),
	       statfsbuf.f_namelen);

	إذا (option_mask32 & OPT_TERSE)
		چاپ_کن("%lx ", (بیعلامت دراز) statfsbuf.f_type);
	وإلا
		چاپ_کن("Type: %s\n", human_fstype(statfsbuf.f_type));

# إذا !ENABLE_SELINUX
	format = (option_mask32 & OPT_TERSE
		? "%lu %llu %llu %llu %llu %llu\n"
		: "Block size: %-10lu\n"
		  "Blocks: Total: %-10llu Free: %-10llu Available: %llu\n"
		  "Inodes: Total: %-10llu Free: %llu\n");
	چاپ_کن(format,
	       (بیعلامت دراز) statfsbuf.f_bsize,
	       (بیعلامت دراز دراز) statfsbuf.f_blocks,
	       (بیعلامت دراز دراز) statfsbuf.f_bfree,
	       (بیعلامت دراز دراز) statfsbuf.f_bavail,
	       (بیعلامت دراز دراز) statfsbuf.f_files,
	       (بیعلامت دراز دراز) statfsbuf.f_ffree);
# وإلا
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
	چاپ_کن(format,
		(بیعلامت دراز) statfsbuf.f_bsize,
		(بیعلامت دراز دراز) statfsbuf.f_blocks,
		(بیعلامت دراز دراز) statfsbuf.f_bfree,
		(بیعلامت دراز دراز) statfsbuf.f_bavail,
		(بیعلامت دراز دراز) statfsbuf.f_files,
		(بیعلامت دراز دراز) statfsbuf.f_ffree,
		scontext);

	إذا (scontext)
		freecon(scontext);
# endif
#پایان_اگر  /* FEATURE_STAT_FORMAT */
	أرجع 1;
}
#پایان_اگر  /* FEATURE_STAT_FILESYSTEM */

/* stat the file and print what we find */
#إذا !ENABLE_FEATURE_STAT_FORMAT
#تعریف_کن do_stat(filename, format) do_stat(filename)
#پایان_اگر
ساكن bool do_stat(ثابت حرف *filename, ثابت حرف *format)
{
	بنية stat statbuf;
#إذا ENABLE_SELINUX
	security_context_t scontext = عدم;

	إذا (option_mask32 & OPT_SELINUX) {
		إذا ((option_mask32 & OPT_DEREFERENCE
		     ? lgetfilecon(filename, &scontext)
		     : getfilecon(filename, &scontext)
		    ) < 0
		) {
			bb_simple_perror_پیام(filename);
			أرجع 0;
		}
	}
#پایان_اگر
	إذا ((option_mask32 & OPT_DEREFERENCE ? stat : lstat) (filename, &statbuf) != 0) {
		bb_perror_پیام("can't stat '%s'", filename);
		أرجع 0;
	}

#إذا ENABLE_FEATURE_STAT_FORMAT
	إذا (format == عدم) {
# إذا !ENABLE_SELINUX
		إذا (option_mask32 & OPT_TERSE) {
			format = "%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %o";
		} وإلا {
			إذا (S_ISBLK(statbuf.st_نما) || S_ISCHR(statbuf.st_نما)) {
				format =
					"  File: %N\n"
					"  Size: %-10s\tBlocks: %-10b IO Block: %-6o %F\n"
					"Device: %Dh/%dd\tInode: %-10i  Links: %-5h"
					" Device type: %t,%T\n"
					"Access: (%04a/%10.10A)  Uid: (%5u/%8U)   Gid: (%5g/%8G)\n"
					"Access: %x\n" "Modify: %y\n" "Change: %z";
			} وإلا {
				format =
					"  File: %N\n"
					"  Size: %-10s\tBlocks: %-10b IO Block: %-6o %F\n"
					"Device: %Dh/%dd\tInode: %-10i  Links: %h\n"
					"Access: (%04a/%10.10A)  Uid: (%5u/%8U)   Gid: (%5g/%8G)\n"
					"Access: %x\n" "Modify: %y\n" "Change: %z";
			}
		}
# وإلا
		إذا (option_mask32 & OPT_TERSE) {
			format = (option_mask32 & OPT_SELINUX ?
				"%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %o %C\n"
				:
				"%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %o\n"
				);
		} وإلا {
			إذا (S_ISBLK(statbuf.st_نما) || S_ISCHR(statbuf.st_نما)) {
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
			} وإلا {
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
#وإلا	/* FEATURE_STAT_FORMAT */
	إذا (option_mask32 & OPT_TERSE) {
		چاپ_کن("%s %llu %llu %lx %lu %lu %llx %llu %lu %lx %lx %lu %lu %lu %lu"
		       IF_NOT_SELINUX("\n"),
		       filename,
		       (بیعلامت دراز دراز) statbuf.st_اندازه,
		       (بیعلامت دراز دراز) statbuf.st_بلوکها,
		       (بیعلامت دراز) statbuf.st_نما,
		       (بیعلامت دراز) statbuf.st_uid,
		       (بیعلامت دراز) statbuf.st_gid,
		       (بیعلامت دراز دراز) statbuf.st_dev,
		       (بیعلامت دراز دراز) statbuf.st_ino,
		       (بیعلامت دراز) statbuf.st_nlink,
		       (بیعلامت دراز) major(statbuf.st_rdev),
		       (بیعلامت دراز) minor(statbuf.st_rdev),
		       (بیعلامت دراز) statbuf.st_atime,
		       (بیعلامت دراز) statbuf.st_mtime,
		       (بیعلامت دراز) statbuf.st_ctime,
		       (بیعلامت دراز) statbuf.st_blksize
		);
# إذا ENABLE_SELINUX
		إذا (option_mask32 & OPT_SELINUX)
			چاپ_کن(" %s\n", scontext);
		وإلا
			bb_putchar('\n');
# endif
	} وإلا {
		حرف modestr[12];
		حرف *linkname = عدم;
		بنية passwd *pw_ent;
		بنية گروه *gw_ent;

		gw_ent = getgrgid(statbuf.st_gid);
		pw_ent = getpwuid(statbuf.st_uid);

		إذا (S_ISLNK(statbuf.st_نما))
			linkname = xmalloc_readlink_or_warn(filename);
		إذا (linkname) {
			چاپ_کن("  File: '%s' -> '%s'\n", filename, linkname);
			حرر(linkname);
		} وإلا {
			چاپ_کن("  File: '%s'\n", filename);
		}

		چاپ_کن("  Size: %-10llu\tBlocks: %-10llu IO Block: %-6lu %s\n"
		       "Device: %llxh/%llud\tInode: %-10llu  Links: %-5lu",
		       (بیعلامت دراز دراز) statbuf.st_اندازه,
		       (بیعلامت دراز دراز) statbuf.st_بلوکها,
		       (بیعلامت دراز) statbuf.st_blksize,
		       file_type(&statbuf),
		       (بیعلامت دراز دراز) statbuf.st_dev,
		       (بیعلامت دراز دراز) statbuf.st_dev,
		       (بیعلامت دراز دراز) statbuf.st_ino,
		       (بیعلامت دراز) statbuf.st_nlink);
		إذا (S_ISBLK(statbuf.st_نما) || S_ISCHR(statbuf.st_نما))
			چاپ_کن(" Device type: %lx,%lx\n",
			       (بیعلامت دراز) major(statbuf.st_rdev),
			       (بیعلامت دراز) minor(statbuf.st_rdev));
		وإلا
			bb_putchar('\n');
		چاپ_کن("Access: (%04lo/%10.10s)  Uid: (%5lu/%8s)   Gid: (%5lu/%8s)\n",
		       (بیعلامت دراز) (statbuf.st_نما & (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)),
		       bb_نما_string(modestr, statbuf.st_نما),
		       (بیعلامت دراز) statbuf.st_uid,
		       (pw_ent != عدم) ? pw_ent->pw_name : "UNKNOWN",
		       (بیعلامت دراز) statbuf.st_gid,
		       (gw_ent != عدم) ? gw_ent->gr_name : "UNKNOWN");
# إذا ENABLE_SELINUX
		إذا (option_mask32 & OPT_SELINUX)
			چاپ_کن("   S_Context: %s\n", scontext);
# endif
		چاپ_کن("Access: %s\n", human_زمان(&statbuf.st_atim));
		چاپ_کن("Modify: %s\n", human_زمان(&statbuf.st_mtim));
		چاپ_کن("Change: %s\n", human_زمان(&statbuf.st_ctim));
	}
#پایان_اگر  /* FEATURE_STAT_FORMAT */
	أرجع 1;
}

صحيح stat_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح stat_main(صحيح argc استفادهنشده_PARAM, حرف **argv)
{
	IF_FEATURE_STAT_FORMAT(حرف *format = عدم;)
	صحيح i;
	صحيح ok;
	statfunc_ptr statfunc = do_stat;
#إذا ENABLE_FEATURE_STAT_FILESYSTEM || ENABLE_SELINUX
	بیعلامت opts;

	opts =
#پایان_اگر
	getopt32(argv, "^"
		"tL"
		IF_FEATURE_STAT_FILESYSTEM("f")
		IF_SELINUX("Z")
		IF_FEATURE_STAT_FORMAT("c:")
		"\0" "-1" /* min one arg */
		IF_FEATURE_STAT_FORMAT(,&format)
	);
#إذا ENABLE_FEATURE_STAT_FILESYSTEM
	إذا (opts & OPT_FILESYS) /* -f */
		statfunc = do_statfs;
#پایان_اگر
#إذا ENABLE_SELINUX
	إذا (opts & OPT_SELINUX) {
		selinux_or_die();
	}
#پایان_اگر
	ok = 1;
	argv += optind;
	لكل (i = 0; argv[i]; ++i)
		ok &= statfunc(argv[i] IF_FEATURE_STAT_FORMAT(, format));

	أرجع (ok ? EXIT_SUCCESS : EXIT_FAILURE);
}
