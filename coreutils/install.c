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

#شامل "libbb.h"
#شامل "libcoreutils/coreutils.h"

#اگر ENABLE_FEATURE_INSTALL_LONG_OPTIONS
ساکن ثابت حرف install_longopts[] ALIGN1 =
	IF_FEATURE_VERBOSE(
	"verbose\0"             No_دلیل       "v"
	)
	"directory\0"           No_دلیل       "d"
	"preserve-timestamps\0" No_دلیل       "p"
	"strip\0"               No_دلیل       "s"
	"group\0"               Required_دلیل "g"
	"mode\0"                Required_دلیل "m"
	"owner\0"               Required_دلیل "o"
	"target-directory\0"	Required_دلیل "t"
/* autofs build insists of using -b --suffix=.orig */
/* TODO? (short option for --suffix is -S) */
# اگر ENABLE_SELINUX
	"context\0"             Required_دلیل "Z"
	"preserve_context\0"    No_دلیل       "\xff"
	"preserve-context\0"    No_دلیل       "\xff"
# endif
	;
# define GETOPT32 getopt32long
# define LONGOPTS install_longopts,
#ورنہ
# define GETOPT32 getopt32
# define LONGOPTS
#اگر_ختم


#اگر ENABLE_SELINUX
ساکن خالی setdefaultfilecon(ثابت حرف *راستہ)
{
	ساخت stat s;
	security_context_t scontext = تہی;

	اگر (!is_selinux_enabled()) {
		واپس;
	}
	اگر (lstat(راستہ, &s) != 0) {
		واپس;
	}

	اگر (matchpathcon(راستہ, s.st_منوال, &scontext) < 0) {
		جاؤ برآمد;
	}
	اگر (strcmp(scontext, "<<none>>") == 0) {
		جاؤ برآمد;
	}

	اگر (lsetfilecon(راستہ, scontext) < 0) {
		اگر (errno != ENOTSUP) {
			bb_perror_پیغام("warning: can't change context"
					" of %s to %s", راستہ, scontext);
		}
	}

 برآمد:
	freecon(scontext);
}

#اگر_ختم

صحیح install_main(صحیح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح install_main(صحیح argc, حرف **argv)
{
	ساخت stat statbuf;
	منوال_t منوال;
	uid_t uid;
	gid_t gid;
	حرف *arg, *آخری;
	ثابت حرف *gid_str;
	ثابت حرف *uid_str;
	ثابت حرف *منوال_str;
	صحیح mkdir_flags = FILEUTILS_RECUR;
	صحیح copy_flags = FILEUTILS_DEREFERENCE | FILEUTILS_FORCE;
	صحیح opts;
	صحیح ret = EXIT_SUCCESS;
	صحیح isdir;
#اگر ENABLE_SELINUX
	security_context_t scontext;
	bool use_default_selinux_context = 1;
#اگر_ختم
	شمار {
		OPT_c             = 1 << 0,
		OPT_v             = 1 << 1,
		OPT_b             = 1 << 2,
		OPT_MKDIR_LEADING = 1 << 3,
		OPT_DIRECTORY     = 1 << 4,
		OPT_PRESERVE_وقت = 1 << 5,
		OPT_STRIP         = 1 << 6,
		OPT_گروہ         = 1 << 7,
		OPT_منوال          = 1 << 8,
		OPT_OWNER         = 1 << 9,
		OPT_ہدف        = 1 << 10,
#اگر ENABLE_SELINUX
		OPT_سیٹ_SECURITY_CONTEXT = 1 << 11,
		OPT_PRESERVE_SECURITY_CONTEXT = 1 << 12,
#اگر_ختم
	};

	/* -c exists for backwards compatibility, it's needed */
	/* -b is ignored ("make a backup of each existing destination file") */
	opts = GETOPT32(argv, "^"
		"cvb" "Ddpsg:m:o:t:" IF_SELINUX("Z:")
		"\0"
		"t--d:d--t:s--d:d--s"
		IF_FEATURE_INSTALL_LONG_OPTIONS(IF_SELINUX(":Z--\xff:\xff--Z")),
		LONGOPTS
		&gid_str, &منوال_str, &uid_str, &آخری
		IF_SELINUX(, &scontext)
	);
	argc -= optind;
	argv += optind;

#اگر ENABLE_SELINUX
	اگر (opts & (OPT_PRESERVE_SECURITY_CONTEXT|OPT_سیٹ_SECURITY_CONTEXT)) {
		selinux_or_die();
		use_default_selinux_context = 0;
		اگر (opts & OPT_PRESERVE_SECURITY_CONTEXT) {
			copy_flags |= FILEUTILS_PRESERVE_SECURITY_CONTEXT;
		}
		اگر (opts & OPT_سیٹ_SECURITY_CONTEXT) {
			setfscreatecon_or_die(scontext);
			copy_flags |= FILEUTILS_سیٹ_SECURITY_CONTEXT;
		}
	}
#اگر_ختم

	اگر ((opts & OPT_v) && FILEUTILS_VERBOSE) {
		mkdir_flags |= FILEUTILS_VERBOSE;
		copy_flags |= FILEUTILS_VERBOSE;
	}

	/* preserve access and modification time, this is GNU behaviour,
	 * BSD only preserves modification time */
	اگر (opts & OPT_PRESERVE_وقت) {
		copy_flags |= FILEUTILS_PRESERVE_حالت;
	}
	منوال = 0755; /* GNU coreutils 6.10 compat */
	اگر (opts & OPT_منوال)
		منوال = bb_parse_منوال(منوال_str, منوال);
	uid = (opts & OPT_OWNER) ? get_ug_id(uid_str, xuname2uid) : getuid();
	gid = (opts & OPT_گروہ) ? get_ug_id(gid_str, xgroup2gid) : getgid();

	/* If -t DIR is in use, then isdir=true, last="DIR" */
	isdir = (opts & OPT_ہدف);
	اگر (!(opts & (OPT_ہدف|OPT_DIRECTORY))) {
		/* Neither -t DIR nor -d is in use */
		argc--;
		آخری = argv[argc];
		argv[argc] = تہی;
		/* coreutils install resolves link in this case, don't use lstat */
		isdir = stat(آخری, &statbuf) < 0 ? 0 : S_ISDIR(statbuf.st_منوال);
	}

	اگر (argc < 1)
		bb_show_usage();

	جب ((arg = *argv++) != تہی) {
		حرف *dest;

		اگر (opts & OPT_DIRECTORY) {
			dest = arg;
			/* GNU coreutils 6.9 does not set uid:gid
			 * on intermediate created directories
			 * (only on last one) */
			اگر (bb_make_directory(dest, 0755, mkdir_flags)) {
				ret = EXIT_FAILURE;
				جاؤ اگلا;
			}
		} ورنہ {
			dest = آخری;
			اگر (opts & OPT_MKDIR_LEADING) {
				حرف *ddir = xstrdup(dest);
				/*
				 * -D -t DIR1/DIR2/F3 FILE: create DIR1/DIR2/F3, copy FILE there
				 * -D FILE DIR1/DIR2/F3: create DIR1/DIR2, copy FILE there as F3
				 */
				bb_make_directory((opts & OPT_ہدف) ? ddir : dirname(ddir), 0755, mkdir_flags);
				/* errors are not checked. copy_file
				 * will fail if dir is not created.
				 */
				آزاد(ddir);
			}
			اگر (isdir)
				dest = concat_راستہ_file(آخری, bb_basename(arg));
			اگر (copy_file(arg, dest, copy_flags) != 0) {
				/* copy is not made */
				ret = EXIT_FAILURE;
				جاؤ اگلا;
			}
			اگر (opts & OPT_STRIP) {
				حرف *دلائل[4];
				دلائل[0] = (حرف*)"strip";
				دلائل[1] = (حرف*)"-p"; /* -p --preserve-dates */
				دلائل[2] = dest;
				دلائل[3] = تہی;
				اگر (spawn_and_wait(دلائل)) {
					bb_simple_perror_پیغام("strip");
					ret = EXIT_FAILURE;
				}
			}
		}

		/* Set the user and group id */
		/* (must be before chmod, or else chown may clear suid/gid bits) */
		اگر ((opts & (OPT_OWNER|OPT_گروہ))
		 && lchown(dest, uid, gid) == -1
		) {
			bb_perror_پیغام("can't change %s of %s", "ownership", dest);
			ret = EXIT_FAILURE;
		}

		/* Set the file mode (always, not only with -m).
		 * GNU coreutils 6.10 is not affected by umask. */
		اگر (chmod(dest, منوال) == -1) {
			bb_perror_پیغام("can't change %s of %s", "permissions", dest);
			ret = EXIT_FAILURE;
		}
#اگر ENABLE_SELINUX
		اگر (use_default_selinux_context)
			setdefaultfilecon(dest);
#اگر_ختم
 اگلا:
		اگر (ENABLE_FEATURE_CLEAN_اوپر && isdir)
			آزاد(dest);
	}

	واپس ret;
}
