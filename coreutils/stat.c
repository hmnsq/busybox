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

#अन्तर्भाव "libbb.h"
#अन्तर्भाव "common_bufsiz.h"

गणना {
	OPT_TERSE       = (1 << 0),
	OPT_DEREFERENCE = (1 << 1),
	OPT_FILESYS     = (1 << 2) * ENABLE_FEATURE_STAT_FILESYSTEM,
	OPT_SELINUX     = (1 << (2+ENABLE_FEATURE_STAT_FILESYSTEM)) * ENABLE_SELINUX,
};

#यदि ENABLE_FEATURE_STAT_FORMAT
प्रकारनिर्देश bool (*statfunc_ptr)(ध्रुव वर्ण *, ध्रुव वर्ण *);
#अन्यथा
प्रकारनिर्देश bool (*statfunc_ptr)(ध्रुव वर्ण *);
#यदि_अन्तः

स्थिर ध्रुव वर्ण *file_type(ध्रुव संरचना stat *st)
{
	/* See POSIX 1003.1-2001 XCU Table 4-8 lines 17093-17107
	 * for some of these formats.
	 * To keep diagnostics grammatical in English, the
	 * returned string must start with a consonant.
	 */
	यदि (S_ISREG(st->st_बहुलकम्))  प्रत्यागम st->st_आकार == 0 ? "regular empty file" : "regular file";
	यदि (S_ISDIR(st->st_बहुलकम्))  प्रत्यागम "directory";
	यदि (S_ISBLK(st->st_बहुलकम्))  प्रत्यागम "block special file";
	यदि (S_ISCHR(st->st_बहुलकम्))  प्रत्यागम "character special file";
	यदि (S_ISFIFO(st->st_बहुलकम्)) प्रत्यागम "fifo";
	यदि (S_ISLNK(st->st_बहुलकम्))  प्रत्यागम "symbolic link";
	यदि (S_ISSOCK(st->st_बहुलकम्)) प्रत्यागम "socket";
#यदि_परिभाषितम् S_TYPEISMQ
	यदि (S_TYPEISMQ(st))        प्रत्यागम "message queue";
#यदि_अन्तः
#यदि_परिभाषितम् S_TYPEISSEM
	यदि (S_TYPEISSEM(st))       प्रत्यागम "semaphore";
#यदि_अन्तः
#यदि_परिभाषितम् S_TYPEISSHM
	यदि (S_TYPEISSHM(st))       प्रत्यागम "shared memory object";
#यदि_अन्तः
#यदि_परिभाषितम् S_TYPEISTMO
	यदि (S_TYPEISTMO(st))       प्रत्यागम "typed memory object";
#यदि_अन्तः
	प्रत्यागम "weird file";
}

स्थिर ध्रुव वर्ण *human_time(संरचना timespec *ts)
{
	वर्ण fmt[परिमाण("%Y-%m-%d %H:%M:%S.123456789 %z") + /*paranoia*/ 8];

	/* coreutils 6.3 compat */
#परिभाषय buf bb_common_bufsiz1
	सज्जता_common_bufsiz();

	sprintf(stpcpy(fmt, "%Y-%m-%d %H:%M:%S"), ".%09u %%z", (अचिह्नित)ts->tv_nsec);
	strftime(buf, COMMON_BUFSIZE, fmt, localtime(&ts->tv_sec));
	प्रत्यागम buf;
#अपरिभाषय buf
}

#यदि ENABLE_FEATURE_STAT_FILESYSTEM
#परिभाषय FS_TYPE_LIST \
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
स्थिर ध्रुव वर्ण *human_fstype(uint32_t f_type)
{
# define FS_TYPE(type, name) type,
	स्थिर ध्रुव uint32_t fstype[] ALIGN4 = {
		FS_TYPE_LIST
	};
# undef FS_TYPE
# define FS_TYPE(type, name) name"\0"
	स्थिर ध्रुव वर्ण humanname[] ALIGN1 =
		FS_TYPE_LIST
		"UNKNOWN";
# undef FS_TYPE
	पूर्णाङ्क i;

	प्रति (i = 0; i < ARRAY_आकार(fstype); ++i)
		यदि (fstype[i] == f_type)
			भङ्ग;
	प्रत्यागम nth_string(humanname, i);
}

/* "man statfs" says that statfsbuf->f_fsid is a mess */
/* coreutils treats it as an array of ints, most significant first */
स्थिर अचिह्नित दीर्घ दीर्घ get_f_fsid(ध्रुव संरचना statfs *statfsbuf)
{
	ध्रुव अचिह्नित *p = (ध्रुव शून्य*) &statfsbuf->f_fsid;
	अचिह्नित sz = परिमाण(statfsbuf->f_fsid) / परिमाण(अचिह्नित);
	अचिह्नित दीर्घ दीर्घ r = 0;

	कुरु
		r = (r << (परिमाण(अचिह्नित)*8)) | *p++;
	यावत् (--sz > 0);
	प्रत्यागम r;
}
#यदि_अन्तः  /* FEATURE_STAT_FILESYSTEM */

#यदि ENABLE_FEATURE_STAT_FORMAT
स्थिर शून्य strcatc(वर्ण *str, वर्ण c)
{
	पूर्णाङ्क len = वाक्यदैर्घ्यम्(str);
	str[len++] = c;
	str[len] = '\0';
}

स्थिर शून्य printfs(वर्ण *pformat, ध्रुव वर्ण *सन्देशः)
{
	strcatc(pformat, 's');
	प्रारूप_लिख(pformat, सन्देशः);
}

#यदि ENABLE_FEATURE_STAT_FILESYSTEM
/* print statfs info */
स्थिर शून्य FAST_FUNC print_statfs(वर्ण *pformat, ध्रुव वर्ण m,
		ध्रुव वर्ण *ध्रुव filename, ध्रुव शून्य *दत्त
		IF_SELINUX(, security_context_t scontext))
{
	ध्रुव संरचना statfs *statfsbuf = दत्त;
	यदि (m == 'n') {
		printfs(pformat, filename);
	} अन्यथा यदि (m == 'i') {
		वाक्यं_योजय(pformat, "llx");
		प्रारूप_लिख(pformat, get_f_fsid(statfsbuf));
	} अन्यथा यदि (m == 'l') {
		वाक्यं_योजय(pformat, "lu");
		प्रारूप_लिख(pformat, (अचिह्नित दीर्घ) statfsbuf->f_namelen);
	} अन्यथा यदि (m == 't') {
		वाक्यं_योजय(pformat, "lx");
		प्रारूप_लिख(pformat, (अचिह्नित दीर्घ) statfsbuf->f_type); /* no equiv */
	} अन्यथा यदि (m == 'T') {
		printfs(pformat, human_fstype(statfsbuf->f_type));
	} अन्यथा यदि (m == 'b') {
		वाक्यं_योजय(pformat, "llu");
		प्रारूप_लिख(pformat, (अचिह्नित दीर्घ दीर्घ) statfsbuf->f_blocks);
	} अन्यथा यदि (m == 'f') {
		वाक्यं_योजय(pformat, "llu");
		प्रारूप_लिख(pformat, (अचिह्नित दीर्घ दीर्घ) statfsbuf->f_bfree);
	} अन्यथा यदि (m == 'a') {
		वाक्यं_योजय(pformat, "llu");
		प्रारूप_लिख(pformat, (अचिह्नित दीर्घ दीर्घ) statfsbuf->f_bavail);
	} अन्यथा यदि (m == 's' || m == 'S') {
		वाक्यं_योजय(pformat, "lu");
		प्रारूप_लिख(pformat, (अचिह्नित दीर्घ) statfsbuf->f_bsize);
	} अन्यथा यदि (m == 'c') {
		वाक्यं_योजय(pformat, "llu");
		प्रारूप_लिख(pformat, (अचिह्नित दीर्घ दीर्घ) statfsbuf->f_files);
	} अन्यथा यदि (m == 'd') {
		वाक्यं_योजय(pformat, "llu");
		प्रारूप_लिख(pformat, (अचिह्नित दीर्घ दीर्घ) statfsbuf->f_ffree);
# यदि ENABLE_SELINUX
	} अन्यथा यदि (m == 'C' && (option_mask32 & OPT_SELINUX)) {
		printfs(pformat, scontext);
# endif
	} अन्यथा {
		strcatc(pformat, 'c');
		प्रारूप_लिख(pformat, m);
	}
}
#यदि_अन्तः

/* print stat info */
स्थिर शून्य FAST_FUNC print_stat(वर्ण *pformat, ध्रुव वर्ण m,
		ध्रुव वर्ण *ध्रुव filename, ध्रुव शून्य *दत्त
		IF_SELINUX(, security_context_t scontext))
{
#परिभाषय TYPE_SIGNED(t) (! ((t) 0 < (t) -1))
	संरचना stat *statbuf = (संरचना stat *) दत्त;
	संरचना passwd *pw_ent;
	संरचना गणः *gw_ent;

	यदि (m == 'n') {
		printfs(pformat, filename);
	} अन्यथा यदि (m == 'N') {
		strcatc(pformat, 's');
		यदि (S_ISLNK(statbuf->st_बहुलकम्)) {
			वर्ण *linkname = xmalloc_readlink_or_warn(filename);
			यदि (linkname == रिक्त)
				प्रत्यागम;
			प्रारूप_लिख("'%s' -> '%s'", filename, linkname);
			मुक्त(linkname);
		} अन्यथा {
			प्रारूप_लिख(pformat, filename);
		}
	} अन्यथा यदि (m == 'd') {
		वाक्यं_योजय(pformat, "llu");
		प्रारूप_लिख(pformat, (अचिह्नित दीर्घ दीर्घ) statbuf->st_dev);
	} अन्यथा यदि (m == 'D') {
		वाक्यं_योजय(pformat, "llx");
		प्रारूप_लिख(pformat, (अचिह्नित दीर्घ दीर्घ) statbuf->st_dev);
	} अन्यथा यदि (m == 'i') {
		वाक्यं_योजय(pformat, "llu");
		प्रारूप_लिख(pformat, (अचिह्नित दीर्घ दीर्घ) statbuf->st_ino);
	} अन्यथा यदि (m == 'a') {
		वाक्यं_योजय(pformat, "lo");
		प्रारूप_लिख(pformat, (अचिह्नित दीर्घ) (statbuf->st_बहुलकम् & (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)));
	} अन्यथा यदि (m == 'A') {
		वर्ण modestr[12];
		printfs(pformat, bb_बहुलकम्_string(modestr, statbuf->st_बहुलकम्));
	} अन्यथा यदि (m == 'f') {
		वाक्यं_योजय(pformat, "lx");
		प्रारूप_लिख(pformat, (अचिह्नित दीर्घ) statbuf->st_बहुलकम्);
	} अन्यथा यदि (m == 'F') {
		printfs(pformat, file_type(statbuf));
	} अन्यथा यदि (m == 'h') {
		वाक्यं_योजय(pformat, "lu");
		प्रारूप_लिख(pformat, (अचिह्नित दीर्घ) statbuf->st_nlink);
	} अन्यथा यदि (m == 'u') {
		वाक्यं_योजय(pformat, "lu");
		प्रारूप_लिख(pformat, (अचिह्नित दीर्घ) statbuf->st_uid);
	} अन्यथा यदि (m == 'U') {
		pw_ent = getpwuid(statbuf->st_uid);
		printfs(pformat, (pw_ent != रिक्त) ? pw_ent->pw_name : "UNKNOWN");
	} अन्यथा यदि (m == 'g') {
		वाक्यं_योजय(pformat, "lu");
		प्रारूप_लिख(pformat, (अचिह्नित दीर्घ) statbuf->st_gid);
	} अन्यथा यदि (m == 'G') {
		gw_ent = getgrgid(statbuf->st_gid);
		printfs(pformat, (gw_ent != रिक्त) ? gw_ent->gr_name : "UNKNOWN");
	} अन्यथा यदि (m == 't') {
		वाक्यं_योजय(pformat, "lx");
		प्रारूप_लिख(pformat, (अचिह्नित दीर्घ) major(statbuf->st_rdev));
	} अन्यथा यदि (m == 'T') {
		वाक्यं_योजय(pformat, "lx");
		प्रारूप_लिख(pformat, (अचिह्नित दीर्घ) minor(statbuf->st_rdev));
	} अन्यथा यदि (m == 's') {
		वाक्यं_योजय(pformat, "llu");
		प्रारूप_लिख(pformat, (अचिह्नित दीर्घ दीर्घ) statbuf->st_आकार);
	} अन्यथा यदि (m == 'B') {
		वाक्यं_योजय(pformat, "lu");
		प्रारूप_लिख(pformat, (अचिह्नित दीर्घ) 512); //ST_NBLOCKSIZE
	} अन्यथा यदि (m == 'b') {
		वाक्यं_योजय(pformat, "llu");
		प्रारूप_लिख(pformat, (अचिह्नित दीर्घ दीर्घ) statbuf->st_खण्डानि);
	} अन्यथा यदि (m == 'o') {
		वाक्यं_योजय(pformat, "lu");
		प्रारूप_लिख(pformat, (अचिह्नित दीर्घ) statbuf->st_blksize);
	} अन्यथा यदि (m == 'x') {
		printfs(pformat, human_time(&statbuf->st_atim));
	} अन्यथा यदि (m == 'X') {
		वाक्यं_योजय(pformat, TYPE_SIGNED(time_t) ? "ld" : "lu");
		/* note: (unsigned long) would be wrong:
		 * imagine (unsigned long64)int32 */
		प्रारूप_लिख(pformat, (दीर्घ) statbuf->st_atime);
	} अन्यथा यदि (m == 'y') {
		printfs(pformat, human_time(&statbuf->st_mtim));
	} अन्यथा यदि (m == 'Y') {
		वाक्यं_योजय(pformat, TYPE_SIGNED(time_t) ? "ld" : "lu");
		प्रारूप_लिख(pformat, (दीर्घ) statbuf->st_mtime);
	} अन्यथा यदि (m == 'z') {
		printfs(pformat, human_time(&statbuf->st_ctim));
	} अन्यथा यदि (m == 'Z') {
		वाक्यं_योजय(pformat, TYPE_SIGNED(time_t) ? "ld" : "lu");
		प्रारूप_लिख(pformat, (दीर्घ) statbuf->st_ctime);
# यदि ENABLE_SELINUX
	} अन्यथा यदि (m == 'C' && (option_mask32 & OPT_SELINUX)) {
		printfs(pformat, scontext);
# endif
	} अन्यथा {
		strcatc(pformat, 'c');
		प्रारूप_लिख(pformat, m);
	}
}

स्थिर शून्य print_it(ध्रुव वर्ण *masterformat,
		ध्रुव वर्ण *filename,
		शून्य FAST_FUNC (*print_func)(वर्ण*, वर्ण, ध्रुव वर्ण*, ध्रुव शून्य* IF_SELINUX(, security_context_t scontext)),
		ध्रुव शून्य *दत्त
		IF_SELINUX(, security_context_t scontext))
{
	/* Create a working copy of the format string */
	वर्ण *format = xstrdup(masterformat);
	/* Add 2 to accommodate our conversion of the stat '%s' format string
	 * to the printf '%llu' one.  */
	वर्ण *dest = xmalloc(वाक्यदैर्घ्यम्(format) + 2 + 1);
	वर्ण *b;

	b = format;
	यावत् (b) {
		/* Each iteration finds next %spec,
		 * prints preceding string and handles found %spec
		 */
		आकार_t len;
		वर्ण *p = strchr(b, '%');
		यदि (!p) {
			/* coreutils 6.3 always prints newline at the end */
			/*fputs(b, stdout);*/
			पङ्क्तिं_लिख(b);
			भङ्ग;
		}

		/* dest = "%<modifiers>" */
		len = 1 + strspn(p + 1, "#-+.I 0123456789");
		स्मृतिं_प्रतिलिख(dest, p, len);
		dest[len] = '\0';

		/* print preceding string */
		*p = '\0';
		fputs_stdout(b);

		p += len;
		b = p + 1;
		विकल्प (*p) {
		पक्ष '\0':
			b = रिक्त;
			/* fall through */
		पक्ष '%':
			bb_putchar('%');
			भङ्ग;
		स्वभाव:
			/* Completes "%<modifiers>" with specifier and printfs */
			print_func(dest, *p, filename, दत्त IF_SELINUX(,scontext));
			भङ्ग;
		}
	}

	मुक्त(format);
	मुक्त(dest);
}
#यदि_अन्तः  /* FEATURE_STAT_FORMAT */

#यदि ENABLE_FEATURE_STAT_FILESYSTEM
/* Stat the file system and print what we find.  */
#यदि !ENABLE_FEATURE_STAT_FORMAT
#परिभाषय do_statfs(filename, format) do_statfs(filename)
#यदि_अन्तः
स्थिर bool do_statfs(ध्रुव वर्ण *filename, ध्रुव वर्ण *format)
{
	संरचना statfs statfsbuf;
#यदि !ENABLE_FEATURE_STAT_FORMAT
	ध्रुव वर्ण *format;
#यदि_अन्तः
#यदि ENABLE_SELINUX
	security_context_t scontext = रिक्त;

	यदि (option_mask32 & OPT_SELINUX) {
		यदि ((option_mask32 & OPT_DEREFERENCE
		     ? lgetfilecon(filename, &scontext)
		     : getfilecon(filename, &scontext)
		    ) < 0
		) {
			bb_simple_perror_सन्देशः(filename);
			प्रत्यागम 0;
		}
	}
#यदि_अन्तः
	यदि (statfs(filename, &statfsbuf) != 0) {
		bb_perror_सन्देशः("can't read file system information for '%s'", filename);
		प्रत्यागम 0;
	}

#यदि ENABLE_FEATURE_STAT_FORMAT
	यदि (format == रिक्त) {
# यदि !ENABLE_SELINUX
		format = (option_mask32 & OPT_TERSE
			? "%n %i %l %t %s %b %f %a %c %d"
			: "  File: \"%n\"\n"
			  "    ID: %-8i Namelen: %-7l Type: %T\n"
			  "Block size: %-10s\n"
			  "Blocks: Total: %-10b Free: %-10f Available: %a\n"
			  "Inodes: Total: %-10c Free: %d");
# अन्यथा
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
#अन्यथा /* !FEATURE_STAT_FORMAT */
	format = (option_mask32 & OPT_TERSE
		? "%s %llx %lu "
		: "  File: \"%s\"\n"
		  "    ID: %-8llx Namelen: %-7lu ");
	प्रारूप_लिख(format,
	       filename,
	       get_f_fsid(&statfsbuf),
	       statfsbuf.f_namelen);

	यदि (option_mask32 & OPT_TERSE)
		प्रारूप_लिख("%lx ", (अचिह्नित दीर्घ) statfsbuf.f_type);
	अन्यथा
		प्रारूप_लिख("Type: %s\n", human_fstype(statfsbuf.f_type));

# यदि !ENABLE_SELINUX
	format = (option_mask32 & OPT_TERSE
		? "%lu %llu %llu %llu %llu %llu\n"
		: "Block size: %-10lu\n"
		  "Blocks: Total: %-10llu Free: %-10llu Available: %llu\n"
		  "Inodes: Total: %-10llu Free: %llu\n");
	प्रारूप_लिख(format,
	       (अचिह्नित दीर्घ) statfsbuf.f_bsize,
	       (अचिह्नित दीर्घ दीर्घ) statfsbuf.f_blocks,
	       (अचिह्नित दीर्घ दीर्घ) statfsbuf.f_bfree,
	       (अचिह्नित दीर्घ दीर्घ) statfsbuf.f_bavail,
	       (अचिह्नित दीर्घ दीर्घ) statfsbuf.f_files,
	       (अचिह्नित दीर्घ दीर्घ) statfsbuf.f_ffree);
# अन्यथा
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
	प्रारूप_लिख(format,
		(अचिह्नित दीर्घ) statfsbuf.f_bsize,
		(अचिह्नित दीर्घ दीर्घ) statfsbuf.f_blocks,
		(अचिह्नित दीर्घ दीर्घ) statfsbuf.f_bfree,
		(अचिह्नित दीर्घ दीर्घ) statfsbuf.f_bavail,
		(अचिह्नित दीर्घ दीर्घ) statfsbuf.f_files,
		(अचिह्नित दीर्घ दीर्घ) statfsbuf.f_ffree,
		scontext);

	यदि (scontext)
		freecon(scontext);
# endif
#यदि_अन्तः  /* FEATURE_STAT_FORMAT */
	प्रत्यागम 1;
}
#यदि_अन्तः  /* FEATURE_STAT_FILESYSTEM */

/* stat the file and print what we find */
#यदि !ENABLE_FEATURE_STAT_FORMAT
#परिभाषय do_stat(filename, format) do_stat(filename)
#यदि_अन्तः
स्थिर bool do_stat(ध्रुव वर्ण *filename, ध्रुव वर्ण *format)
{
	संरचना stat statbuf;
#यदि ENABLE_SELINUX
	security_context_t scontext = रिक्त;

	यदि (option_mask32 & OPT_SELINUX) {
		यदि ((option_mask32 & OPT_DEREFERENCE
		     ? lgetfilecon(filename, &scontext)
		     : getfilecon(filename, &scontext)
		    ) < 0
		) {
			bb_simple_perror_सन्देशः(filename);
			प्रत्यागम 0;
		}
	}
#यदि_अन्तः
	यदि ((option_mask32 & OPT_DEREFERENCE ? stat : lstat) (filename, &statbuf) != 0) {
		bb_perror_सन्देशः("can't stat '%s'", filename);
		प्रत्यागम 0;
	}

#यदि ENABLE_FEATURE_STAT_FORMAT
	यदि (format == रिक्त) {
# यदि !ENABLE_SELINUX
		यदि (option_mask32 & OPT_TERSE) {
			format = "%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %o";
		} अन्यथा {
			यदि (S_ISBLK(statbuf.st_बहुलकम्) || S_ISCHR(statbuf.st_बहुलकम्)) {
				format =
					"  File: %N\n"
					"  Size: %-10s\tBlocks: %-10b IO Block: %-6o %F\n"
					"Device: %Dh/%dd\tInode: %-10i  Links: %-5h"
					" Device type: %t,%T\n"
					"Access: (%04a/%10.10A)  Uid: (%5u/%8U)   Gid: (%5g/%8G)\n"
					"Access: %x\n" "Modify: %y\n" "Change: %z";
			} अन्यथा {
				format =
					"  File: %N\n"
					"  Size: %-10s\tBlocks: %-10b IO Block: %-6o %F\n"
					"Device: %Dh/%dd\tInode: %-10i  Links: %h\n"
					"Access: (%04a/%10.10A)  Uid: (%5u/%8U)   Gid: (%5g/%8G)\n"
					"Access: %x\n" "Modify: %y\n" "Change: %z";
			}
		}
# अन्यथा
		यदि (option_mask32 & OPT_TERSE) {
			format = (option_mask32 & OPT_SELINUX ?
				"%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %o %C\n"
				:
				"%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %o\n"
				);
		} अन्यथा {
			यदि (S_ISBLK(statbuf.st_बहुलकम्) || S_ISCHR(statbuf.st_बहुलकम्)) {
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
			} अन्यथा {
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
#अन्यथा	/* FEATURE_STAT_FORMAT */
	यदि (option_mask32 & OPT_TERSE) {
		प्रारूप_लिख("%s %llu %llu %lx %lu %lu %llx %llu %lu %lx %lx %lu %lu %lu %lu"
		       IF_NOT_SELINUX("\n"),
		       filename,
		       (अचिह्नित दीर्घ दीर्घ) statbuf.st_आकार,
		       (अचिह्नित दीर्घ दीर्घ) statbuf.st_खण्डानि,
		       (अचिह्नित दीर्घ) statbuf.st_बहुलकम्,
		       (अचिह्नित दीर्घ) statbuf.st_uid,
		       (अचिह्नित दीर्घ) statbuf.st_gid,
		       (अचिह्नित दीर्घ दीर्घ) statbuf.st_dev,
		       (अचिह्नित दीर्घ दीर्घ) statbuf.st_ino,
		       (अचिह्नित दीर्घ) statbuf.st_nlink,
		       (अचिह्नित दीर्घ) major(statbuf.st_rdev),
		       (अचिह्नित दीर्घ) minor(statbuf.st_rdev),
		       (अचिह्नित दीर्घ) statbuf.st_atime,
		       (अचिह्नित दीर्घ) statbuf.st_mtime,
		       (अचिह्नित दीर्घ) statbuf.st_ctime,
		       (अचिह्नित दीर्घ) statbuf.st_blksize
		);
# यदि ENABLE_SELINUX
		यदि (option_mask32 & OPT_SELINUX)
			प्रारूप_लिख(" %s\n", scontext);
		अन्यथा
			bb_putchar('\n');
# endif
	} अन्यथा {
		वर्ण modestr[12];
		वर्ण *linkname = रिक्त;
		संरचना passwd *pw_ent;
		संरचना गणः *gw_ent;

		gw_ent = getgrgid(statbuf.st_gid);
		pw_ent = getpwuid(statbuf.st_uid);

		यदि (S_ISLNK(statbuf.st_बहुलकम्))
			linkname = xmalloc_readlink_or_warn(filename);
		यदि (linkname) {
			प्रारूप_लिख("  File: '%s' -> '%s'\n", filename, linkname);
			मुक्त(linkname);
		} अन्यथा {
			प्रारूप_लिख("  File: '%s'\n", filename);
		}

		प्रारूप_लिख("  Size: %-10llu\tBlocks: %-10llu IO Block: %-6lu %s\n"
		       "Device: %llxh/%llud\tInode: %-10llu  Links: %-5lu",
		       (अचिह्नित दीर्घ दीर्घ) statbuf.st_आकार,
		       (अचिह्नित दीर्घ दीर्घ) statbuf.st_खण्डानि,
		       (अचिह्नित दीर्घ) statbuf.st_blksize,
		       file_type(&statbuf),
		       (अचिह्नित दीर्घ दीर्घ) statbuf.st_dev,
		       (अचिह्नित दीर्घ दीर्घ) statbuf.st_dev,
		       (अचिह्नित दीर्घ दीर्घ) statbuf.st_ino,
		       (अचिह्नित दीर्घ) statbuf.st_nlink);
		यदि (S_ISBLK(statbuf.st_बहुलकम्) || S_ISCHR(statbuf.st_बहुलकम्))
			प्रारूप_लिख(" Device type: %lx,%lx\n",
			       (अचिह्नित दीर्घ) major(statbuf.st_rdev),
			       (अचिह्नित दीर्घ) minor(statbuf.st_rdev));
		अन्यथा
			bb_putchar('\n');
		प्रारूप_लिख("Access: (%04lo/%10.10s)  Uid: (%5lu/%8s)   Gid: (%5lu/%8s)\n",
		       (अचिह्नित दीर्घ) (statbuf.st_बहुलकम् & (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)),
		       bb_बहुलकम्_string(modestr, statbuf.st_बहुलकम्),
		       (अचिह्नित दीर्घ) statbuf.st_uid,
		       (pw_ent != रिक्त) ? pw_ent->pw_name : "UNKNOWN",
		       (अचिह्नित दीर्घ) statbuf.st_gid,
		       (gw_ent != रिक्त) ? gw_ent->gr_name : "UNKNOWN");
# यदि ENABLE_SELINUX
		यदि (option_mask32 & OPT_SELINUX)
			प्रारूप_लिख("   S_Context: %s\n", scontext);
# endif
		प्रारूप_लिख("Access: %s\n", human_time(&statbuf.st_atim));
		प्रारूप_लिख("Modify: %s\n", human_time(&statbuf.st_mtim));
		प्रारूप_लिख("Change: %s\n", human_time(&statbuf.st_ctim));
	}
#यदि_अन्तः  /* FEATURE_STAT_FORMAT */
	प्रत्यागम 1;
}

पूर्णाङ्क stat_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क stat_main(पूर्णाङ्क argc अप्रयुक्तम्_PARAM, वर्ण **argv)
{
	IF_FEATURE_STAT_FORMAT(वर्ण *format = रिक्त;)
	पूर्णाङ्क i;
	पूर्णाङ्क ok;
	statfunc_ptr statfunc = do_stat;
#यदि ENABLE_FEATURE_STAT_FILESYSTEM || ENABLE_SELINUX
	अचिह्नित opts;

	opts =
#यदि_अन्तः
	getopt32(argv, "^"
		"tL"
		IF_FEATURE_STAT_FILESYSTEM("f")
		IF_SELINUX("Z")
		IF_FEATURE_STAT_FORMAT("c:")
		"\0" "-1" /* min one arg */
		IF_FEATURE_STAT_FORMAT(,&format)
	);
#यदि ENABLE_FEATURE_STAT_FILESYSTEM
	यदि (opts & OPT_FILESYS) /* -f */
		statfunc = do_statfs;
#यदि_अन्तः
#यदि ENABLE_SELINUX
	यदि (opts & OPT_SELINUX) {
		selinux_or_die();
	}
#यदि_अन्तः
	ok = 1;
	argv += optind;
	प्रति (i = 0; argv[i]; ++i)
		ok &= statfunc(argv[i] IF_FEATURE_STAT_FORMAT(, format));

	प्रत्यागम (ok ? EXIT_SUCCESS : EXIT_FAILURE);
}
