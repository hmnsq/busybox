/* vi: set sw=4 ts=4: */
/*
 * Copyright (C) 2003 by Glenn McGrath
 * SELinux support: by Yuichi Nakamura <ynakam@hitachisoft.jp>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config INSTALL
//config:	bool "install (12 kb)"
//config:	default y
//config:	help
//config:	Copy files and set attributes.
//config:
//config:config FEATURE_INSTALL_LONG_OPTIONS
//config:	bool "Enable long options"
//config:	default y
//config:	depends on INSTALL && LONG_OPTS

//applet:IF_INSTALL(APPLET(install, BB_DIR_USR_BIN, BB_SUID_DROP))

//kbuild:lib-$(CONFIG_INSTALL) += install.o

/* -v, -b, -c are ignored */
//usage:#define install_trivial_usage
//usage:	"[-cdDsp] [-o USER] [-g GRP] [-m MODE] [-t DIR] [SOURCE]... DEST"
//usage:#define install_full_usage "\n\n"
//usage:       "Copy files and set attributes\n"
//usage:     "\n	-c	Just copy (default)"
//usage:     "\n	-d	Create directories"
//usage:     "\n	-D	Create leading target directories"
//usage:     "\n	-s	Strip symbol table"
//usage:     "\n	-p	Preserve date"
//usage:     "\n	-o USER	Set ownership"
//usage:     "\n	-g GRP	Set group ownership"
//usage:     "\n	-m MODE	Set permissions"
//usage:     "\n	-t DIR	Install to DIR"
//usage:	IF_SELINUX(
//usage:     "\n	-Z	Set security context"
//usage:	)

#ضمن "libbb.h"
#ضمن "libcoreutils/coreutils.h"

#إذا ENABLE_FEATURE_INSTALL_LONG_OPTIONS
ساكن ثابت حرف install_longopts[] ALIGN1 =
	IF_FEATURE_VERBOSE(
	"verbose\0"             No_دليل       "v"
	)
	"directory\0"           No_دليل       "d"
	"preserve-timestamps\0" No_دليل       "p"
	"strip\0"               No_دليل       "s"
	"group\0"               Required_دليل "g"
	"mode\0"                Required_دليل "m"
	"owner\0"               Required_دليل "o"
	"target-directory\0"	Required_دليل "t"
/* autofs build insists of using -b --suffix=.orig */
/* TODO? (short option for --suffix is -S) */
# إذا ENABLE_SELINUX
	"context\0"             Required_دليل "Z"
	"preserve_context\0"    No_دليل       "\xff"
	"preserve-context\0"    No_دليل       "\xff"
# endif
	;
# define GETOPT32 getopt32long
# define LONGOPTS install_longopts,
#وإلا
# define GETOPT32 getopt32
# define LONGOPTS
#جيڪڏهن_پورو


#إذا ENABLE_SELINUX
ساكن فراغ setdefaultfilecon(ثابت حرف *رستو)
{
	بنية stat s;
	security_context_t scontext = عدم;

	إذا (!is_selinux_enabled()) {
		أرجع;
	}
	إذا (lstat(رستو, &s) != 0) {
		أرجع;
	}

	إذا (matchpathcon(رستو, s.st_گهڻائي, &scontext) < 0) {
		اذهب ٻاهريون;
	}
	إذا (strcmp(scontext, "<<none>>") == 0) {
		اذهب ٻاهريون;
	}

	إذا (lsetfilecon(رستو, scontext) < 0) {
		إذا (errno != ENOTSUP) {
			bb_perror_نياپو("warning: can't change context"
					" of %s to %s", رستو, scontext);
		}
	}

 ٻاهريون:
	freecon(scontext);
}

#جيڪڏهن_پورو

صحيح install_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح install_main(صحيح argc, حرف **argv)
{
	بنية stat statbuf;
	گهڻائي_t گهڻائي;
	uid_t uid;
	gid_t gid;
	حرف *arg, *آخري;
	ثابت حرف *gid_str;
	ثابت حرف *uid_str;
	ثابت حرف *گهڻائي_str;
	صحيح mkdir_flags = FILEUTILS_RECUR;
	صحيح copy_flags = FILEUTILS_DEREFERENCE | FILEUTILS_FORCE;
	صحيح opts;
	صحيح ret = EXIT_SUCCESS;
	صحيح isdir;
#إذا ENABLE_SELINUX
	security_context_t scontext;
	bool use_default_selinux_context = 1;
#جيڪڏهن_پورو
	تعداد {
		OPT_c             = 1 << 0,
		OPT_v             = 1 << 1,
		OPT_b             = 1 << 2,
		OPT_MKDIR_LEADING = 1 << 3,
		OPT_DIRECTORY     = 1 << 4,
		OPT_PRESERVE_وقت = 1 << 5,
		OPT_STRIP         = 1 << 6,
		OPT_گروهه         = 1 << 7,
		OPT_گهڻائي          = 1 << 8,
		OPT_OWNER         = 1 << 9,
		OPT_هدف        = 1 << 10,
#إذا ENABLE_SELINUX
		OPT_سيٽ_SECURITY_CONTEXT = 1 << 11,
		OPT_PRESERVE_SECURITY_CONTEXT = 1 << 12,
#جيڪڏهن_پورو
	};

	/* -c exists for backwards compatibility, it's needed */
	/* -b is ignored ("make a backup of each existing destination file") */
	opts = GETOPT32(argv, "^"
		"cvb" "Ddpsg:m:o:t:" IF_SELINUX("Z:")
		"\0"
		"t--d:d--t:s--d:d--s"
		IF_FEATURE_INSTALL_LONG_OPTIONS(IF_SELINUX(":Z--\xff:\xff--Z")),
		LONGOPTS
		&gid_str, &گهڻائي_str, &uid_str, &آخري
		IF_SELINUX(, &scontext)
	);
	argc -= optind;
	argv += optind;

#إذا ENABLE_SELINUX
	إذا (opts & (OPT_PRESERVE_SECURITY_CONTEXT|OPT_سيٽ_SECURITY_CONTEXT)) {
		selinux_or_die();
		use_default_selinux_context = 0;
		إذا (opts & OPT_PRESERVE_SECURITY_CONTEXT) {
			copy_flags |= FILEUTILS_PRESERVE_SECURITY_CONTEXT;
		}
		إذا (opts & OPT_سيٽ_SECURITY_CONTEXT) {
			setfscreatecon_or_die(scontext);
			copy_flags |= FILEUTILS_سيٽ_SECURITY_CONTEXT;
		}
	}
#جيڪڏهن_پورو

	إذا ((opts & OPT_v) && FILEUTILS_VERBOSE) {
		mkdir_flags |= FILEUTILS_VERBOSE;
		copy_flags |= FILEUTILS_VERBOSE;
	}

	/* preserve access and modification time, this is GNU behaviour,
	 * BSD only preserves modification time */
	إذا (opts & OPT_PRESERVE_وقت) {
		copy_flags |= FILEUTILS_PRESERVE_حالت;
	}
	گهڻائي = 0755; /* GNU coreutils 6.10 compat */
	إذا (opts & OPT_گهڻائي)
		گهڻائي = bb_parse_گهڻائي(گهڻائي_str, گهڻائي);
	uid = (opts & OPT_OWNER) ? get_ug_id(uid_str, xuname2uid) : getuid();
	gid = (opts & OPT_گروهه) ? get_ug_id(gid_str, xgroup2gid) : getgid();

	/* If -t DIR is in use, then isdir=true, last="DIR" */
	isdir = (opts & OPT_هدف);
	إذا (!(opts & (OPT_هدف|OPT_DIRECTORY))) {
		/* Neither -t DIR nor -d is in use */
		argc--;
		آخري = argv[argc];
		argv[argc] = عدم;
		/* coreutils install resolves link in this case, don't use lstat */
		isdir = stat(آخري, &statbuf) < 0 ? 0 : S_ISDIR(statbuf.st_گهڻائي);
	}

	إذا (argc < 1)
		bb_show_usage();

	طالما ((arg = *argv++) != عدم) {
		حرف *dest;

		إذا (opts & OPT_DIRECTORY) {
			dest = arg;
			/* GNU coreutils 6.9 does not set uid:gid
			 * on intermediate created directories
			 * (only on last one) */
			إذا (bb_make_directory(dest, 0755, mkdir_flags)) {
				ret = EXIT_FAILURE;
				اذهب ايندڙ;
			}
		} وإلا {
			dest = آخري;
			إذا (opts & OPT_MKDIR_LEADING) {
				حرف *ddir = xstrdup(dest);
				/*
				 * -D -t DIR1/DIR2/F3 FILE: create DIR1/DIR2/F3, copy FILE there
				 * -D FILE DIR1/DIR2/F3: create DIR1/DIR2, copy FILE there as F3
				 */
				bb_make_directory((opts & OPT_هدف) ? ddir : dirname(ddir), 0755, mkdir_flags);
				/* errors are not checked. copy_file
				 * will fail if dir is not created.
				 */
				حرر(ddir);
			}
			إذا (isdir)
				dest = concat_رستو_file(آخري, bb_basename(arg));
			إذا (copy_file(arg, dest, copy_flags) != 0) {
				/* copy is not made */
				ret = EXIT_FAILURE;
				اذهب ايندڙ;
			}
			إذا (opts & OPT_STRIP) {
				حرف *دليلون[4];
				دليلون[0] = (حرف*)"strip";
				دليلون[1] = (حرف*)"-p"; /* -p --preserve-dates */
				دليلون[2] = dest;
				دليلون[3] = عدم;
				إذا (spawn_and_wait(دليلون)) {
					bb_simple_perror_نياپو("strip");
					ret = EXIT_FAILURE;
				}
			}
		}

		/* Set the user and group id */
		/* (must be before chmod, or else chown may clear suid/gid bits) */
		إذا ((opts & (OPT_OWNER|OPT_گروهه))
		 && lchown(dest, uid, gid) == -1
		) {
			bb_perror_نياپو("can't change %s of %s", "ownership", dest);
			ret = EXIT_FAILURE;
		}

		/* Set the file mode (always, not only with -m).
		 * GNU coreutils 6.10 is not affected by umask. */
		إذا (chmod(dest, گهڻائي) == -1) {
			bb_perror_نياپو("can't change %s of %s", "permissions", dest);
			ret = EXIT_FAILURE;
		}
#إذا ENABLE_SELINUX
		إذا (use_default_selinux_context)
			setdefaultfilecon(dest);
#جيڪڏهن_پورو
 ايندڙ:
		إذا (ENABLE_FEATURE_CLEAN_مٿي && isdir)
			حرر(dest);
	}

	أرجع ret;
}
