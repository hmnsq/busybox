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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "libcoreutils/coreutils.h"

#ਜੇ ENABLE_FEATURE_INSTALL_LONG_OPTIONS
ਸਥਿਰ ਅਚਲ ਅੱਖਰ install_longopts[] ALIGN1 =
	IF_FEATURE_VERBOSE(
	"verbose\0"             No_ਦਲੀਲ       "v"
	)
	"directory\0"           No_ਦਲੀਲ       "d"
	"preserve-timestamps\0" No_ਦਲੀਲ       "p"
	"strip\0"               No_ਦਲੀਲ       "s"
	"group\0"               Required_ਦਲੀਲ "g"
	"mode\0"                Required_ਦਲੀਲ "m"
	"owner\0"               Required_ਦਲੀਲ "o"
	"target-directory\0"	Required_ਦਲੀਲ "t"
/* autofs build insists of using -b --suffix=.orig */
/* TODO? (short option for --suffix is -S) */
# ਜੇ ENABLE_SELINUX
	"context\0"             Required_ਦਲੀਲ "Z"
	"preserve_context\0"    No_ਦਲੀਲ       "\xff"
	"preserve-context\0"    No_ਦਲੀਲ       "\xff"
# endif
	;
# define GETOPT32 getopt32long
# define LONGOPTS install_longopts,
#ਨਹੀਂਤਾਂ
# define GETOPT32 getopt32
# define LONGOPTS
#ਜੇ_ਅੰਤ


#ਜੇ ENABLE_SELINUX
ਸਥਿਰ ਖਾਲੀ setdefaultfilecon(ਅਚਲ ਅੱਖਰ *ਰਸਤਾ)
{
	ਬਣਤਰ stat s;
	security_context_t scontext = ਨਿਰਾ;

	ਜੇ (!is_selinux_enabled()) {
		ਵਾਪਸ;
	}
	ਜੇ (lstat(ਰਸਤਾ, &s) != 0) {
		ਵਾਪਸ;
	}

	ਜੇ (matchpathcon(ਰਸਤਾ, s.st_ਬਹੁਲਕ, &scontext) < 0) {
		ਜਾਓ ਬਾਹਰਲਾ;
	}
	ਜੇ (strcmp(scontext, "<<none>>") == 0) {
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	ਜੇ (lsetfilecon(ਰਸਤਾ, scontext) < 0) {
		ਜੇ (errno != ENOTSUP) {
			bb_perror_ਸੁਨੇਹਾ("warning: can't change context"
					" of %s to %s", ਰਸਤਾ, scontext);
		}
	}

 ਬਾਹਰਲਾ:
	freecon(scontext);
}

#ਜੇ_ਅੰਤ

ਪੂਰਨ install_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ install_main(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
	ਬਣਤਰ stat statbuf;
	ਬਹੁਲਕ_t ਬਹੁਲਕ;
	uid_t uid;
	gid_t gid;
	ਅੱਖਰ *arg, *ਆਖਰੀ;
	ਅਚਲ ਅੱਖਰ *gid_str;
	ਅਚਲ ਅੱਖਰ *uid_str;
	ਅਚਲ ਅੱਖਰ *ਬਹੁਲਕ_str;
	ਪੂਰਨ mkdir_flags = FILEUTILS_RECUR;
	ਪੂਰਨ copy_flags = FILEUTILS_DEREFERENCE | FILEUTILS_FORCE;
	ਪੂਰਨ opts;
	ਪੂਰਨ ret = EXIT_SUCCESS;
	ਪੂਰਨ isdir;
#ਜੇ ENABLE_SELINUX
	security_context_t scontext;
	bool use_default_selinux_context = 1;
#ਜੇ_ਅੰਤ
	ਗਿਣਤੀ {
		OPT_c             = 1 << 0,
		OPT_v             = 1 << 1,
		OPT_b             = 1 << 2,
		OPT_MKDIR_LEADING = 1 << 3,
		OPT_DIRECTORY     = 1 << 4,
		OPT_PRESERVE_ਸਮਾਂ = 1 << 5,
		OPT_STRIP         = 1 << 6,
		OPT_ਸਮੂਹ         = 1 << 7,
		OPT_ਬਹੁਲਕ          = 1 << 8,
		OPT_OWNER         = 1 << 9,
		OPT_ਨਿਸ਼ਾਨਾ        = 1 << 10,
#ਜੇ ENABLE_SELINUX
		OPT_ਸੈੱਟ_SECURITY_CONTEXT = 1 << 11,
		OPT_PRESERVE_SECURITY_CONTEXT = 1 << 12,
#ਜੇ_ਅੰਤ
	};

	/* -c exists for backwards compatibility, it's needed */
	/* -b is ignored ("make a backup of each existing destination file") */
	opts = GETOPT32(argv, "^"
		"cvb" "Ddpsg:m:o:t:" IF_SELINUX("Z:")
		"\0"
		"t--d:d--t:s--d:d--s"
		IF_FEATURE_INSTALL_LONG_OPTIONS(IF_SELINUX(":Z--\xff:\xff--Z")),
		LONGOPTS
		&gid_str, &ਬਹੁਲਕ_str, &uid_str, &ਆਖਰੀ
		IF_SELINUX(, &scontext)
	);
	argc -= optind;
	argv += optind;

#ਜੇ ENABLE_SELINUX
	ਜੇ (opts & (OPT_PRESERVE_SECURITY_CONTEXT|OPT_ਸੈੱਟ_SECURITY_CONTEXT)) {
		selinux_or_die();
		use_default_selinux_context = 0;
		ਜੇ (opts & OPT_PRESERVE_SECURITY_CONTEXT) {
			copy_flags |= FILEUTILS_PRESERVE_SECURITY_CONTEXT;
		}
		ਜੇ (opts & OPT_ਸੈੱਟ_SECURITY_CONTEXT) {
			setfscreatecon_or_die(scontext);
			copy_flags |= FILEUTILS_ਸੈੱਟ_SECURITY_CONTEXT;
		}
	}
#ਜੇ_ਅੰਤ

	ਜੇ ((opts & OPT_v) && FILEUTILS_VERBOSE) {
		mkdir_flags |= FILEUTILS_VERBOSE;
		copy_flags |= FILEUTILS_VERBOSE;
	}

	/* preserve access and modification time, this is GNU behaviour,
	 * BSD only preserves modification time */
	ਜੇ (opts & OPT_PRESERVE_ਸਮਾਂ) {
		copy_flags |= FILEUTILS_PRESERVE_ਸਥਿਤੀ;
	}
	ਬਹੁਲਕ = 0755; /* GNU coreutils 6.10 compat */
	ਜੇ (opts & OPT_ਬਹੁਲਕ)
		ਬਹੁਲਕ = bb_parse_ਬਹੁਲਕ(ਬਹੁਲਕ_str, ਬਹੁਲਕ);
	uid = (opts & OPT_OWNER) ? get_ug_id(uid_str, xuname2uid) : getuid();
	gid = (opts & OPT_ਸਮੂਹ) ? get_ug_id(gid_str, xgroup2gid) : getgid();

	/* If -t DIR is in use, then isdir=true, last="DIR" */
	isdir = (opts & OPT_ਨਿਸ਼ਾਨਾ);
	ਜੇ (!(opts & (OPT_ਨਿਸ਼ਾਨਾ|OPT_DIRECTORY))) {
		/* Neither -t DIR nor -d is in use */
		argc--;
		ਆਖਰੀ = argv[argc];
		argv[argc] = ਨਿਰਾ;
		/* coreutils install resolves link in this case, don't use lstat */
		isdir = stat(ਆਖਰੀ, &statbuf) < 0 ? 0 : S_ISDIR(statbuf.st_ਬਹੁਲਕ);
	}

	ਜੇ (argc < 1)
		bb_show_usage();

	ਜਦੋਂ ((arg = *argv++) != ਨਿਰਾ) {
		ਅੱਖਰ *dest;

		ਜੇ (opts & OPT_DIRECTORY) {
			dest = arg;
			/* GNU coreutils 6.9 does not set uid:gid
			 * on intermediate created directories
			 * (only on last one) */
			ਜੇ (bb_make_directory(dest, 0755, mkdir_flags)) {
				ret = EXIT_FAILURE;
				ਜਾਓ ਅਗਲਾ;
			}
		} ਨਹੀਂਤਾਂ {
			dest = ਆਖਰੀ;
			ਜੇ (opts & OPT_MKDIR_LEADING) {
				ਅੱਖਰ *ddir = xstrdup(dest);
				/*
				 * -D -t DIR1/DIR2/F3 FILE: create DIR1/DIR2/F3, copy FILE there
				 * -D FILE DIR1/DIR2/F3: create DIR1/DIR2, copy FILE there as F3
				 */
				bb_make_directory((opts & OPT_ਨਿਸ਼ਾਨਾ) ? ddir : dirname(ddir), 0755, mkdir_flags);
				/* errors are not checked. copy_file
				 * will fail if dir is not created.
				 */
				ਮੁਕਤ(ddir);
			}
			ਜੇ (isdir)
				dest = concat_ਰਸਤਾ_file(ਆਖਰੀ, bb_basename(arg));
			ਜੇ (copy_file(arg, dest, copy_flags) != 0) {
				/* copy is not made */
				ret = EXIT_FAILURE;
				ਜਾਓ ਅਗਲਾ;
			}
			ਜੇ (opts & OPT_STRIP) {
				ਅੱਖਰ *ਦਲੀਲਾਂ[4];
				ਦਲੀਲਾਂ[0] = (ਅੱਖਰ*)"strip";
				ਦਲੀਲਾਂ[1] = (ਅੱਖਰ*)"-p"; /* -p --preserve-dates */
				ਦਲੀਲਾਂ[2] = dest;
				ਦਲੀਲਾਂ[3] = ਨਿਰਾ;
				ਜੇ (spawn_and_wait(ਦਲੀਲਾਂ)) {
					bb_simple_perror_ਸੁਨੇਹਾ("strip");
					ret = EXIT_FAILURE;
				}
			}
		}

		/* Set the user and group id */
		/* (must be before chmod, or else chown may clear suid/gid bits) */
		ਜੇ ((opts & (OPT_OWNER|OPT_ਸਮੂਹ))
		 && lchown(dest, uid, gid) == -1
		) {
			bb_perror_ਸੁਨੇਹਾ("can't change %s of %s", "ownership", dest);
			ret = EXIT_FAILURE;
		}

		/* Set the file mode (always, not only with -m).
		 * GNU coreutils 6.10 is not affected by umask. */
		ਜੇ (chmod(dest, ਬਹੁਲਕ) == -1) {
			bb_perror_ਸੁਨੇਹਾ("can't change %s of %s", "permissions", dest);
			ret = EXIT_FAILURE;
		}
#ਜੇ ENABLE_SELINUX
		ਜੇ (use_default_selinux_context)
			setdefaultfilecon(dest);
#ਜੇ_ਅੰਤ
 ਅਗਲਾ:
		ਜੇ (ENABLE_FEATURE_CLEAN_ਉੱਪਰ && isdir)
			ਮੁਕਤ(dest);
	}

	ਵਾਪਸ ret;
}
