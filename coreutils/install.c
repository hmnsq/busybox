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

#അന്തര്ഭാവ "libbb.h"
#അന്തര്ഭാവ "libcoreutils/coreutils.h"

#എങ്കിൽ ENABLE_FEATURE_INSTALL_LONG_OPTIONS
സ്ഥിതം സ്ഥിരം അക്ഷരം install_longopts[] ALIGN1 =
	IF_FEATURE_VERBOSE(
	"verbose\0"             No_argument       "v"
	)
	"directory\0"           No_argument       "d"
	"preserve-timestamps\0" No_argument       "p"
	"strip\0"               No_argument       "s"
	"group\0"               Required_argument "g"
	"mode\0"                Required_argument "m"
	"owner\0"               Required_argument "o"
	"target-directory\0"	Required_argument "t"
/* autofs build insists of using -b --suffix=.orig */
/* TODO? (short option for --suffix is -S) */
# എങ്കിൽ ENABLE_SELINUX
	"context\0"             Required_argument "Z"
	"preserve_context\0"    No_argument       "\xff"
	"preserve-context\0"    No_argument       "\xff"
# endif
	;
# define GETOPT32 getopt32long
# define LONGOPTS install_longopts,
#അല്ലെങ്കിൽ
# define GETOPT32 getopt32
# define LONGOPTS
#endif


#എങ്കിൽ ENABLE_SELINUX
സ്ഥിതം ശൂന്യം setdefaultfilecon(സ്ഥിരം അക്ഷരം *path)
{
	ഘടന stat s;
	security_context_t scontext = ശൂന്യ;

	എങ്കിൽ (!is_selinux_enabled()) {
		തിരികെ;
	}
	എങ്കിൽ (lstat(path, &s) != 0) {
		തിരികെ;
	}

	എങ്കിൽ (matchpathcon(path, s.st_mode, &scontext) < 0) {
		പോകുക out;
	}
	എങ്കിൽ (strcmp(scontext, "<<none>>") == 0) {
		പോകുക out;
	}

	എങ്കിൽ (lsetfilecon(path, scontext) < 0) {
		എങ്കിൽ (errno != ENOTSUP) {
			bb_perror_സന്ദേശം("warning: can't change context"
					" of %s to %s", path, scontext);
		}
	}

 out:
	freecon(scontext);
}

#endif

പൂർണ്ണസംഖ്യ install_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) MAIN_EXTERNALLY_VISIBLE;
പൂർണ്ണസംഖ്യ install_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv)
{
	ഘടന stat statbuf;
	mode_t mode;
	uid_t uid;
	gid_t gid;
	അക്ഷരം *arg, *last;
	സ്ഥിരം അക്ഷരം *gid_str;
	സ്ഥിരം അക്ഷരം *uid_str;
	സ്ഥിരം അക്ഷരം *mode_str;
	പൂർണ്ണസംഖ്യ mkdir_flags = FILEUTILS_RECUR;
	പൂർണ്ണസംഖ്യ copy_flags = FILEUTILS_DEREFERENCE | FILEUTILS_FORCE;
	പൂർണ്ണസംഖ്യ opts;
	പൂർണ്ണസംഖ്യ ret = EXIT_SUCCESS;
	പൂർണ്ണസംഖ്യ isdir;
#എങ്കിൽ ENABLE_SELINUX
	security_context_t scontext;
	bool use_default_selinux_context = 1;
#endif
	എണ്ണൽ {
		OPT_c             = 1 << 0,
		OPT_v             = 1 << 1,
		OPT_b             = 1 << 2,
		OPT_MKDIR_LEADING = 1 << 3,
		OPT_DIRECTORY     = 1 << 4,
		OPT_PRESERVE_TIME = 1 << 5,
		OPT_STRIP         = 1 << 6,
		OPT_GROUP         = 1 << 7,
		OPT_MODE          = 1 << 8,
		OPT_OWNER         = 1 << 9,
		OPT_TARGET        = 1 << 10,
#എങ്കിൽ ENABLE_SELINUX
		OPT_SET_SECURITY_CONTEXT = 1 << 11,
		OPT_PRESERVE_SECURITY_CONTEXT = 1 << 12,
#endif
	};

	/* -c exists for backwards compatibility, it's needed */
	/* -b is ignored ("make a backup of each existing destination file") */
	opts = GETOPT32(argv, "^"
		"cvb" "Ddpsg:m:o:t:" IF_SELINUX("Z:")
		"\0"
		"t--d:d--t:s--d:d--s"
		IF_FEATURE_INSTALL_LONG_OPTIONS(IF_SELINUX(":Z--\xff:\xff--Z")),
		LONGOPTS
		&gid_str, &mode_str, &uid_str, &last
		IF_SELINUX(, &scontext)
	);
	argc -= optind;
	argv += optind;

#എങ്കിൽ ENABLE_SELINUX
	എങ്കിൽ (opts & (OPT_PRESERVE_SECURITY_CONTEXT|OPT_SET_SECURITY_CONTEXT)) {
		selinux_or_die();
		use_default_selinux_context = 0;
		എങ്കിൽ (opts & OPT_PRESERVE_SECURITY_CONTEXT) {
			copy_flags |= FILEUTILS_PRESERVE_SECURITY_CONTEXT;
		}
		എങ്കിൽ (opts & OPT_SET_SECURITY_CONTEXT) {
			setfscreatecon_or_die(scontext);
			copy_flags |= FILEUTILS_SET_SECURITY_CONTEXT;
		}
	}
#endif

	എങ്കിൽ ((opts & OPT_v) && FILEUTILS_VERBOSE) {
		mkdir_flags |= FILEUTILS_VERBOSE;
		copy_flags |= FILEUTILS_VERBOSE;
	}

	/* preserve access and modification time, this is GNU behaviour,
	 * BSD only preserves modification time */
	എങ്കിൽ (opts & OPT_PRESERVE_TIME) {
		copy_flags |= FILEUTILS_PRESERVE_STATUS;
	}
	mode = 0755; /* GNU coreutils 6.10 compat */
	എങ്കിൽ (opts & OPT_MODE)
		mode = bb_parse_mode(mode_str, mode);
	uid = (opts & OPT_OWNER) ? get_ug_id(uid_str, xuname2uid) : getuid();
	gid = (opts & OPT_GROUP) ? get_ug_id(gid_str, xgroup2gid) : getgid();

	/* If -t DIR is in use, then isdir=true, last="DIR" */
	isdir = (opts & OPT_TARGET);
	എങ്കിൽ (!(opts & (OPT_TARGET|OPT_DIRECTORY))) {
		/* Neither -t DIR nor -d is in use */
		argc--;
		last = argv[argc];
		argv[argc] = ശൂന്യ;
		/* coreutils install resolves link in this case, don't use lstat */
		isdir = stat(last, &statbuf) < 0 ? 0 : S_ISDIR(statbuf.st_mode);
	}

	എങ്കിൽ (argc < 1)
		bb_show_usage();

	ആകുമ്പോൾ ((arg = *argv++) != ശൂന്യ) {
		അക്ഷരം *dest;

		എങ്കിൽ (opts & OPT_DIRECTORY) {
			dest = arg;
			/* GNU coreutils 6.9 does not set uid:gid
			 * on intermediate created directories
			 * (only on last one) */
			എങ്കിൽ (bb_make_directory(dest, 0755, mkdir_flags)) {
				ret = EXIT_FAILURE;
				പോകുക next;
			}
		} അല്ലെങ്കിൽ {
			dest = last;
			എങ്കിൽ (opts & OPT_MKDIR_LEADING) {
				അക്ഷരം *ddir = xstrdup(dest);
				/*
				 * -D -t DIR1/DIR2/F3 FILE: create DIR1/DIR2/F3, copy FILE there
				 * -D FILE DIR1/DIR2/F3: create DIR1/DIR2, copy FILE there as F3
				 */
				bb_make_directory((opts & OPT_TARGET) ? ddir : dirname(ddir), 0755, mkdir_flags);
				/* errors are not checked. copy_file
				 * will fail if dir is not created.
				 */
				സ്വതന്ത്രം(ddir);
			}
			എങ്കിൽ (isdir)
				dest = concat_path_file(last, bb_basename(arg));
			എങ്കിൽ (copy_file(arg, dest, copy_flags) != 0) {
				/* copy is not made */
				ret = EXIT_FAILURE;
				പോകുക next;
			}
			എങ്കിൽ (opts & OPT_STRIP) {
				അക്ഷരം *args[4];
				args[0] = (അക്ഷരം*)"strip";
				args[1] = (അക്ഷരം*)"-p"; /* -p --preserve-dates */
				args[2] = dest;
				args[3] = ശൂന്യ;
				എങ്കിൽ (spawn_and_wait(args)) {
					bb_simple_perror_സന്ദേശം("strip");
					ret = EXIT_FAILURE;
				}
			}
		}

		/* Set the user and group id */
		/* (must be before chmod, or else chown may clear suid/gid bits) */
		എങ്കിൽ ((opts & (OPT_OWNER|OPT_GROUP))
		 && lchown(dest, uid, gid) == -1
		) {
			bb_perror_സന്ദേശം("can't change %s of %s", "ownership", dest);
			ret = EXIT_FAILURE;
		}

		/* Set the file mode (always, not only with -m).
		 * GNU coreutils 6.10 is not affected by umask. */
		എങ്കിൽ (chmod(dest, mode) == -1) {
			bb_perror_സന്ദേശം("can't change %s of %s", "permissions", dest);
			ret = EXIT_FAILURE;
		}
#എങ്കിൽ ENABLE_SELINUX
		എങ്കിൽ (use_default_selinux_context)
			setdefaultfilecon(dest);
#endif
 next:
		എങ്കിൽ (ENABLE_FEATURE_CLEAN_UP && isdir)
			സ്വതന്ത്രം(dest);
	}

	തിരികെ ret;
}
