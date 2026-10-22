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

#जर ENABLE_FEATURE_STAT_FORMAT
प्रकारव्याख्या bool (*statfunc_ptr)(स्थिरांक अक्षर *, स्थिरांक अक्षर *);
#अन्यथा
प्रकारव्याख्या bool (*statfunc_ptr)(स्थिरांक अक्षर *);
#endif

स्थिर स्थिरांक अक्षर *file_type(स्थिरांक संरचना stat *st)
{
	/* See POSIX 1003.1-2001 XCU Table 4-8 lines 17093-17107
	 * for some of these formats.
	 * To keep diagnostics grammatical in English, the
	 * returned string must start with a consonant.
	 */
	जर (S_ISREG(st->st_mode))  परत st->st_size == 0 ? "regular empty file" : "regular file";
	जर (S_ISDIR(st->st_mode))  परत "directory";
	जर (S_ISBLK(st->st_mode))  परत "block special file";
	जर (S_ISCHR(st->st_mode))  परत "character special file";
	जर (S_ISFIFO(st->st_mode)) परत "fifo";
	जर (S_ISLNK(st->st_mode))  परत "symbolic link";
	जर (S_ISSOCK(st->st_mode)) परत "socket";
#ifdef S_TYPEISMQ
	जर (S_TYPEISMQ(st))        परत "message queue";
#endif
#ifdef S_TYPEISSEM
	जर (S_TYPEISSEM(st))       परत "semaphore";
#endif
#ifdef S_TYPEISSHM
	जर (S_TYPEISSHM(st))       परत "shared memory object";
#endif
#ifdef S_TYPEISTMO
	जर (S_TYPEISTMO(st))       परत "typed memory object";
#endif
	परत "weird file";
}

स्थिर स्थिरांक अक्षर *human_time(संरचना timespec *ts)
{
	अक्षर fmt[आकार("%Y-%m-%d %H:%M:%S.123456789 %z") + /*paranoia*/ 8];

	/* coreutils 6.3 compat */
#define buf bb_common_bufsiz1
	setup_common_bufsiz();

	प्र_लिखो(stpcpy(fmt, "%Y-%m-%d %H:%M:%S"), ".%09u %%z", (अचिन्हित)ts->tv_nsec);
	स_माला(buf, COMMON_BUFSIZE, fmt, स_स्थानीय(&ts->tv_sec));
	परत buf;
#undef buf
}

#जर ENABLE_FEATURE_STAT_FILESYSTEM
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
स्थिर स्थिरांक अक्षर *human_fstype(uint32_t f_type)
{
# define FS_TYPE(type, name) type,
	स्थिर स्थिरांक uint32_t fstype[] ALIGN4 = {
		FS_TYPE_LIST
	};
# undef FS_TYPE
# define FS_TYPE(type, name) name"\0"
	स्थिर स्थिरांक अक्षर humanname[] ALIGN1 =
		FS_TYPE_LIST
		"UNKNOWN";
# undef FS_TYPE
	पूर्णांक i;

	साठी (i = 0; i < ARRAY_SIZE(fstype); ++i)
		जर (fstype[i] == f_type)
			थांबा;
	परत nth_string(humanname, i);
}

/* "man statfs" says that statfsbuf->f_fsid is a mess */
/* coreutils treats it as an array of ints, most significant first */
स्थिर अचिन्हित दीर्घ दीर्घ get_f_fsid(स्थिरांक संरचना statfs *statfsbuf)
{
	स्थिरांक अचिन्हित *p = (स्थिरांक शून्य*) &statfsbuf->f_fsid;
	अचिन्हित sz = आकार(statfsbuf->f_fsid) / आकार(अचिन्हित);
	अचिन्हित दीर्घ दीर्घ r = 0;

	करा
		r = (r << (आकार(अचिन्हित)*8)) | *p++;
	पर्यंत (--sz > 0);
	परत r;
}
#endif  /* FEATURE_STAT_FILESYSTEM */

#जर ENABLE_FEATURE_STAT_FORMAT
स्थिर शून्य strcatc(अक्षर *str, अक्षर c)
{
	पूर्णांक len = म_माप(str);
	str[len++] = c;
	str[len] = '\0';
}

स्थिर शून्य printfs(अक्षर *pformat, स्थिरांक अक्षर *संदेश)
{
	strcatc(pformat, 's');
	म_लिखो(pformat, संदेश);
}

#जर ENABLE_FEATURE_STAT_FILESYSTEM
/* print statfs info */
स्थिर शून्य FAST_FUNC print_statfs(अक्षर *pformat, स्थिरांक अक्षर m,
		स्थिरांक अक्षर *स्थिरांक filename, स्थिरांक शून्य *data
		IF_SELINUX(, security_context_t scontext))
{
	स्थिरांक संरचना statfs *statfsbuf = data;
	जर (m == 'n') {
		printfs(pformat, filename);
	} अन्यथा जर (m == 'i') {
		म_जोड़ो(pformat, "llx");
		म_लिखो(pformat, get_f_fsid(statfsbuf));
	} अन्यथा जर (m == 'l') {
		म_जोड़ो(pformat, "lu");
		म_लिखो(pformat, (अचिन्हित दीर्घ) statfsbuf->f_namelen);
	} अन्यथा जर (m == 't') {
		म_जोड़ो(pformat, "lx");
		म_लिखो(pformat, (अचिन्हित दीर्घ) statfsbuf->f_type); /* no equiv */
	} अन्यथा जर (m == 'T') {
		printfs(pformat, human_fstype(statfsbuf->f_type));
	} अन्यथा जर (m == 'b') {
		म_जोड़ो(pformat, "llu");
		म_लिखो(pformat, (अचिन्हित दीर्घ दीर्घ) statfsbuf->f_blocks);
	} अन्यथा जर (m == 'f') {
		म_जोड़ो(pformat, "llu");
		म_लिखो(pformat, (अचिन्हित दीर्घ दीर्घ) statfsbuf->f_bfree);
	} अन्यथा जर (m == 'a') {
		म_जोड़ो(pformat, "llu");
		म_लिखो(pformat, (अचिन्हित दीर्घ दीर्घ) statfsbuf->f_bavail);
	} अन्यथा जर (m == 's' || m == 'S') {
		म_जोड़ो(pformat, "lu");
		म_लिखो(pformat, (अचिन्हित दीर्घ) statfsbuf->f_bsize);
	} अन्यथा जर (m == 'c') {
		म_जोड़ो(pformat, "llu");
		म_लिखो(pformat, (अचिन्हित दीर्घ दीर्घ) statfsbuf->f_files);
	} अन्यथा जर (m == 'd') {
		म_जोड़ो(pformat, "llu");
		म_लिखो(pformat, (अचिन्हित दीर्घ दीर्घ) statfsbuf->f_ffree);
# जर ENABLE_SELINUX
	} अन्यथा जर (m == 'C' && (option_mask32 & OPT_SELINUX)) {
		printfs(pformat, scontext);
# endif
	} अन्यथा {
		strcatc(pformat, 'c');
		म_लिखो(pformat, m);
	}
}
#endif

/* print stat info */
स्थिर शून्य FAST_FUNC print_stat(अक्षर *pformat, स्थिरांक अक्षर m,
		स्थिरांक अक्षर *स्थिरांक filename, स्थिरांक शून्य *data
		IF_SELINUX(, security_context_t scontext))
{
#define TYPE_SIGNED(t) (! ((t) 0 < (t) -1))
	संरचना stat *statbuf = (संरचना stat *) data;
	संरचना passwd *pw_ent;
	संरचना group *gw_ent;

	जर (m == 'n') {
		printfs(pformat, filename);
	} अन्यथा जर (m == 'N') {
		strcatc(pformat, 's');
		जर (S_ISLNK(statbuf->st_mode)) {
			अक्षर *linkname = xmalloc_readlink_or_warn(filename);
			जर (linkname == रिक्त)
				परत;
			म_लिखो("'%s' -> '%s'", filename, linkname);
			मुक्त(linkname);
		} अन्यथा {
			म_लिखो(pformat, filename);
		}
	} अन्यथा जर (m == 'd') {
		म_जोड़ो(pformat, "llu");
		म_लिखो(pformat, (अचिन्हित दीर्घ दीर्घ) statbuf->st_dev);
	} अन्यथा जर (m == 'D') {
		म_जोड़ो(pformat, "llx");
		म_लिखो(pformat, (अचिन्हित दीर्घ दीर्घ) statbuf->st_dev);
	} अन्यथा जर (m == 'i') {
		म_जोड़ो(pformat, "llu");
		म_लिखो(pformat, (अचिन्हित दीर्घ दीर्घ) statbuf->st_ino);
	} अन्यथा जर (m == 'a') {
		म_जोड़ो(pformat, "lo");
		म_लिखो(pformat, (अचिन्हित दीर्घ) (statbuf->st_mode & (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)));
	} अन्यथा जर (m == 'A') {
		अक्षर modestr[12];
		printfs(pformat, bb_mode_string(modestr, statbuf->st_mode));
	} अन्यथा जर (m == 'f') {
		म_जोड़ो(pformat, "lx");
		म_लिखो(pformat, (अचिन्हित दीर्घ) statbuf->st_mode);
	} अन्यथा जर (m == 'F') {
		printfs(pformat, file_type(statbuf));
	} अन्यथा जर (m == 'h') {
		म_जोड़ो(pformat, "lu");
		म_लिखो(pformat, (अचिन्हित दीर्घ) statbuf->st_nlink);
	} अन्यथा जर (m == 'u') {
		म_जोड़ो(pformat, "lu");
		म_लिखो(pformat, (अचिन्हित दीर्घ) statbuf->st_uid);
	} अन्यथा जर (m == 'U') {
		pw_ent = getpwuid(statbuf->st_uid);
		printfs(pformat, (pw_ent != रिक्त) ? pw_ent->pw_name : "UNKNOWN");
	} अन्यथा जर (m == 'g') {
		म_जोड़ो(pformat, "lu");
		म_लिखो(pformat, (अचिन्हित दीर्घ) statbuf->st_gid);
	} अन्यथा जर (m == 'G') {
		gw_ent = getgrgid(statbuf->st_gid);
		printfs(pformat, (gw_ent != रिक्त) ? gw_ent->gr_name : "UNKNOWN");
	} अन्यथा जर (m == 't') {
		म_जोड़ो(pformat, "lx");
		म_लिखो(pformat, (अचिन्हित दीर्घ) major(statbuf->st_rdev));
	} अन्यथा जर (m == 'T') {
		म_जोड़ो(pformat, "lx");
		म_लिखो(pformat, (अचिन्हित दीर्घ) minor(statbuf->st_rdev));
	} अन्यथा जर (m == 's') {
		म_जोड़ो(pformat, "llu");
		म_लिखो(pformat, (अचिन्हित दीर्घ दीर्घ) statbuf->st_size);
	} अन्यथा जर (m == 'B') {
		म_जोड़ो(pformat, "lu");
		म_लिखो(pformat, (अचिन्हित दीर्घ) 512); //ST_NBLOCKSIZE
	} अन्यथा जर (m == 'b') {
		म_जोड़ो(pformat, "llu");
		म_लिखो(pformat, (अचिन्हित दीर्घ दीर्घ) statbuf->st_blocks);
	} अन्यथा जर (m == 'o') {
		म_जोड़ो(pformat, "lu");
		म_लिखो(pformat, (अचिन्हित दीर्घ) statbuf->st_blksize);
	} अन्यथा जर (m == 'x') {
		printfs(pformat, human_time(&statbuf->st_atim));
	} अन्यथा जर (m == 'X') {
		म_जोड़ो(pformat, TYPE_SIGNED(समय_प्रकार) ? "ld" : "lu");
		/* note: (unsigned long) would be wrong:
		 * imagine (unsigned long64)int32 */
		म_लिखो(pformat, (दीर्घ) statbuf->st_atime);
	} अन्यथा जर (m == 'y') {
		printfs(pformat, human_time(&statbuf->st_mtim));
	} अन्यथा जर (m == 'Y') {
		म_जोड़ो(pformat, TYPE_SIGNED(समय_प्रकार) ? "ld" : "lu");
		म_लिखो(pformat, (दीर्घ) statbuf->st_mtime);
	} अन्यथा जर (m == 'z') {
		printfs(pformat, human_time(&statbuf->st_ctim));
	} अन्यथा जर (m == 'Z') {
		म_जोड़ो(pformat, TYPE_SIGNED(समय_प्रकार) ? "ld" : "lu");
		म_लिखो(pformat, (दीर्घ) statbuf->st_ctime);
# जर ENABLE_SELINUX
	} अन्यथा जर (m == 'C' && (option_mask32 & OPT_SELINUX)) {
		printfs(pformat, scontext);
# endif
	} अन्यथा {
		strcatc(pformat, 'c');
		म_लिखो(pformat, m);
	}
}

स्थिर शून्य print_it(स्थिरांक अक्षर *masterformat,
		स्थिरांक अक्षर *filename,
		शून्य FAST_FUNC (*print_func)(अक्षर*, अक्षर, स्थिरांक अक्षर*, स्थिरांक शून्य* IF_SELINUX(, security_context_t scontext)),
		स्थिरांक शून्य *data
		IF_SELINUX(, security_context_t scontext))
{
	/* Create a working copy of the format string */
	अक्षर *format = xstrdup(masterformat);
	/* Add 2 to accommodate our conversion of the stat '%s' format string
	 * to the printf '%llu' one.  */
	अक्षर *dest = xmalloc(म_माप(format) + 2 + 1);
	अक्षर *b;

	b = format;
	पर्यंत (b) {
		/* Each iteration finds next %spec,
		 * prints preceding string and handles found %spec
		 */
		माप_प्रकार len;
		अक्षर *p = म_अक्षर(b, '%');
		जर (!p) {
			/* coreutils 6.3 always prints newline at the end */
			/*fputs(b, stdout);*/
			माला_दो(b);
			थांबा;
		}

		/* dest = "%<modifiers>" */
		len = 1 + म_अखोज(p + 1, "#-+.I 0123456789");
		स_नकल(dest, p, len);
		dest[len] = '\0';

		/* print preceding string */
		*p = '\0';
		fputs_stdout(b);

		p += len;
		b = p + 1;
		निवड (*p) {
		स्थिती '\0':
			b = रिक्त;
			/* fall through */
		स्थिती '%':
			bb_putchar('%');
			थांबा;
		पूर्वनिर्धारित:
			/* Completes "%<modifiers>" with specifier and printfs */
			print_func(dest, *p, filename, data IF_SELINUX(,scontext));
			थांबा;
		}
	}

	मुक्त(format);
	मुक्त(dest);
}
#endif  /* FEATURE_STAT_FORMAT */

#जर ENABLE_FEATURE_STAT_FILESYSTEM
/* Stat the file system and print what we find.  */
#जर !ENABLE_FEATURE_STAT_FORMAT
#define do_statfs(filename, format) do_statfs(filename)
#endif
स्थिर bool do_statfs(स्थिरांक अक्षर *filename, स्थिरांक अक्षर *format)
{
	संरचना statfs statfsbuf;
#जर !ENABLE_FEATURE_STAT_FORMAT
	स्थिरांक अक्षर *format;
#endif
#जर ENABLE_SELINUX
	security_context_t scontext = रिक्त;

	जर (option_mask32 & OPT_SELINUX) {
		जर ((option_mask32 & OPT_DEREFERENCE
		     ? lgetfilecon(filename, &scontext)
		     : getfilecon(filename, &scontext)
		    ) < 0
		) {
			bb_simple_perror_संदेश(filename);
			परत 0;
		}
	}
#endif
	जर (statfs(filename, &statfsbuf) != 0) {
		bb_perror_संदेश("can't read file system information for '%s'", filename);
		परत 0;
	}

#जर ENABLE_FEATURE_STAT_FORMAT
	जर (format == रिक्त) {
# जर !ENABLE_SELINUX
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
	म_लिखो(format,
	       filename,
	       get_f_fsid(&statfsbuf),
	       statfsbuf.f_namelen);

	जर (option_mask32 & OPT_TERSE)
		म_लिखो("%lx ", (अचिन्हित दीर्घ) statfsbuf.f_type);
	अन्यथा
		म_लिखो("Type: %s\n", human_fstype(statfsbuf.f_type));

# जर !ENABLE_SELINUX
	format = (option_mask32 & OPT_TERSE
		? "%lu %llu %llu %llu %llu %llu\n"
		: "Block size: %-10lu\n"
		  "Blocks: Total: %-10llu Free: %-10llu Available: %llu\n"
		  "Inodes: Total: %-10llu Free: %llu\n");
	म_लिखो(format,
	       (अचिन्हित दीर्घ) statfsbuf.f_bsize,
	       (अचिन्हित दीर्घ दीर्घ) statfsbuf.f_blocks,
	       (अचिन्हित दीर्घ दीर्घ) statfsbuf.f_bfree,
	       (अचिन्हित दीर्घ दीर्घ) statfsbuf.f_bavail,
	       (अचिन्हित दीर्घ दीर्घ) statfsbuf.f_files,
	       (अचिन्हित दीर्घ दीर्घ) statfsbuf.f_ffree);
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
	म_लिखो(format,
		(अचिन्हित दीर्घ) statfsbuf.f_bsize,
		(अचिन्हित दीर्घ दीर्घ) statfsbuf.f_blocks,
		(अचिन्हित दीर्घ दीर्घ) statfsbuf.f_bfree,
		(अचिन्हित दीर्घ दीर्घ) statfsbuf.f_bavail,
		(अचिन्हित दीर्घ दीर्घ) statfsbuf.f_files,
		(अचिन्हित दीर्घ दीर्घ) statfsbuf.f_ffree,
		scontext);

	जर (scontext)
		freecon(scontext);
# endif
#endif  /* FEATURE_STAT_FORMAT */
	परत 1;
}
#endif  /* FEATURE_STAT_FILESYSTEM */

/* stat the file and print what we find */
#जर !ENABLE_FEATURE_STAT_FORMAT
#define do_stat(filename, format) do_stat(filename)
#endif
स्थिर bool do_stat(स्थिरांक अक्षर *filename, स्थिरांक अक्षर *format)
{
	संरचना stat statbuf;
#जर ENABLE_SELINUX
	security_context_t scontext = रिक्त;

	जर (option_mask32 & OPT_SELINUX) {
		जर ((option_mask32 & OPT_DEREFERENCE
		     ? lgetfilecon(filename, &scontext)
		     : getfilecon(filename, &scontext)
		    ) < 0
		) {
			bb_simple_perror_संदेश(filename);
			परत 0;
		}
	}
#endif
	जर ((option_mask32 & OPT_DEREFERENCE ? stat : lstat) (filename, &statbuf) != 0) {
		bb_perror_संदेश("can't stat '%s'", filename);
		परत 0;
	}

#जर ENABLE_FEATURE_STAT_FORMAT
	जर (format == रिक्त) {
# जर !ENABLE_SELINUX
		जर (option_mask32 & OPT_TERSE) {
			format = "%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %o";
		} अन्यथा {
			जर (S_ISBLK(statbuf.st_mode) || S_ISCHR(statbuf.st_mode)) {
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
		जर (option_mask32 & OPT_TERSE) {
			format = (option_mask32 & OPT_SELINUX ?
				"%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %o %C\n"
				:
				"%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %o\n"
				);
		} अन्यथा {
			जर (S_ISBLK(statbuf.st_mode) || S_ISCHR(statbuf.st_mode)) {
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
	जर (option_mask32 & OPT_TERSE) {
		म_लिखो("%s %llu %llu %lx %lu %lu %llx %llu %lu %lx %lx %lu %lu %lu %lu"
		       IF_NOT_SELINUX("\n"),
		       filename,
		       (अचिन्हित दीर्घ दीर्घ) statbuf.st_size,
		       (अचिन्हित दीर्घ दीर्घ) statbuf.st_blocks,
		       (अचिन्हित दीर्घ) statbuf.st_mode,
		       (अचिन्हित दीर्घ) statbuf.st_uid,
		       (अचिन्हित दीर्घ) statbuf.st_gid,
		       (अचिन्हित दीर्घ दीर्घ) statbuf.st_dev,
		       (अचिन्हित दीर्घ दीर्घ) statbuf.st_ino,
		       (अचिन्हित दीर्घ) statbuf.st_nlink,
		       (अचिन्हित दीर्घ) major(statbuf.st_rdev),
		       (अचिन्हित दीर्घ) minor(statbuf.st_rdev),
		       (अचिन्हित दीर्घ) statbuf.st_atime,
		       (अचिन्हित दीर्घ) statbuf.st_mtime,
		       (अचिन्हित दीर्घ) statbuf.st_ctime,
		       (अचिन्हित दीर्घ) statbuf.st_blksize
		);
# जर ENABLE_SELINUX
		जर (option_mask32 & OPT_SELINUX)
			म_लिखो(" %s\n", scontext);
		अन्यथा
			bb_putchar('\n');
# endif
	} अन्यथा {
		अक्षर modestr[12];
		अक्षर *linkname = रिक्त;
		संरचना passwd *pw_ent;
		संरचना group *gw_ent;

		gw_ent = getgrgid(statbuf.st_gid);
		pw_ent = getpwuid(statbuf.st_uid);

		जर (S_ISLNK(statbuf.st_mode))
			linkname = xmalloc_readlink_or_warn(filename);
		जर (linkname) {
			म_लिखो("  File: '%s' -> '%s'\n", filename, linkname);
			मुक्त(linkname);
		} अन्यथा {
			म_लिखो("  File: '%s'\n", filename);
		}

		म_लिखो("  Size: %-10llu\tBlocks: %-10llu IO Block: %-6lu %s\n"
		       "Device: %llxh/%llud\tInode: %-10llu  Links: %-5lu",
		       (अचिन्हित दीर्घ दीर्घ) statbuf.st_size,
		       (अचिन्हित दीर्घ दीर्घ) statbuf.st_blocks,
		       (अचिन्हित दीर्घ) statbuf.st_blksize,
		       file_type(&statbuf),
		       (अचिन्हित दीर्घ दीर्घ) statbuf.st_dev,
		       (अचिन्हित दीर्घ दीर्घ) statbuf.st_dev,
		       (अचिन्हित दीर्घ दीर्घ) statbuf.st_ino,
		       (अचिन्हित दीर्घ) statbuf.st_nlink);
		जर (S_ISBLK(statbuf.st_mode) || S_ISCHR(statbuf.st_mode))
			म_लिखो(" Device type: %lx,%lx\n",
			       (अचिन्हित दीर्घ) major(statbuf.st_rdev),
			       (अचिन्हित दीर्घ) minor(statbuf.st_rdev));
		अन्यथा
			bb_putchar('\n');
		म_लिखो("Access: (%04lo/%10.10s)  Uid: (%5lu/%8s)   Gid: (%5lu/%8s)\n",
		       (अचिन्हित दीर्घ) (statbuf.st_mode & (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)),
		       bb_mode_string(modestr, statbuf.st_mode),
		       (अचिन्हित दीर्घ) statbuf.st_uid,
		       (pw_ent != रिक्त) ? pw_ent->pw_name : "UNKNOWN",
		       (अचिन्हित दीर्घ) statbuf.st_gid,
		       (gw_ent != रिक्त) ? gw_ent->gr_name : "UNKNOWN");
# जर ENABLE_SELINUX
		जर (option_mask32 & OPT_SELINUX)
			म_लिखो("   S_Context: %s\n", scontext);
# endif
		म_लिखो("Access: %s\n", human_time(&statbuf.st_atim));
		म_लिखो("Modify: %s\n", human_time(&statbuf.st_mtim));
		म_लिखो("Change: %s\n", human_time(&statbuf.st_ctim));
	}
#endif  /* FEATURE_STAT_FORMAT */
	परत 1;
}

पूर्णांक stat_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक stat_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	IF_FEATURE_STAT_FORMAT(अक्षर *format = रिक्त;)
	पूर्णांक i;
	पूर्णांक ok;
	statfunc_ptr statfunc = do_stat;
#जर ENABLE_FEATURE_STAT_FILESYSTEM || ENABLE_SELINUX
	अचिन्हित opts;

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
#जर ENABLE_FEATURE_STAT_FILESYSTEM
	जर (opts & OPT_FILESYS) /* -f */
		statfunc = do_statfs;
#endif
#जर ENABLE_SELINUX
	जर (opts & OPT_SELINUX) {
		selinux_or_die();
	}
#endif
	ok = 1;
	argv += optind;
	साठी (i = 0; argv[i]; ++i)
		ok &= statfunc(argv[i] IF_FEATURE_STAT_FORMAT(, format));

	परत (ok ? निकास_सफल : निकास_त्रुटि);
}
