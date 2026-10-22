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

#ಅನ್ತರ್ಭಾವ "libbb.h"
#ಅನ್ತರ್ಭಾವ "libcoreutils/coreutils.h"

#ಆದರೆ ENABLE_FEATURE_INSTALL_LONG_OPTIONS
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ install_longopts[] ALIGN1 =
	IF_FEATURE_VERBOSE(
	"verbose\0"             No_ವಾದ       "v"
	)
	"directory\0"           No_ವಾದ       "d"
	"preserve-timestamps\0" No_ವಾದ       "p"
	"strip\0"               No_ವಾದ       "s"
	"group\0"               Required_ವಾದ "g"
	"mode\0"                Required_ವಾದ "m"
	"owner\0"               Required_ವಾದ "o"
	"target-directory\0"	Required_ವಾದ "t"
/* autofs build insists of using -b --suffix=.orig */
/* TODO? (short option for --suffix is -S) */
# ಆದರೆ ENABLE_SELINUX
	"context\0"             Required_ವಾದ "Z"
	"preserve_context\0"    No_ವಾದ       "\xff"
	"preserve-context\0"    No_ವಾದ       "\xff"
# endif
	;
# define GETOPT32 getopt32long
# define LONGOPTS install_longopts,
#ಇಲ್ಲದಿದ್ದರೆ
# define GETOPT32 getopt32
# define LONGOPTS
#ಆದರೆ_ಅಂತ್ಯ


#ಆದರೆ ENABLE_SELINUX
ಸ್ಥಿರ ಶೂನ್ಯ setdefaultfilecon(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ)
{
	ರಚನೆ stat s;
	security_context_t scontext = ಖಾಲಿ;

	ಆದರೆ (!is_selinux_enabled()) {
		ಹಿಂತಿರುಗಿ;
	}
	ಆದರೆ (lstat(ಮಾರ್ಗ, &s) != 0) {
		ಹಿಂತಿರುಗಿ;
	}

	ಆದರೆ (matchpathcon(ಮಾರ್ಗ, s.st_ಬಹುಲಕ, &scontext) < 0) {
		ಹೋಗು ಹೊರಹೋಗು;
	}
	ಆದರೆ (strcmp(scontext, "<<none>>") == 0) {
		ಹೋಗು ಹೊರಹೋಗು;
	}

	ಆದರೆ (lsetfilecon(ಮಾರ್ಗ, scontext) < 0) {
		ಆದರೆ (errno != ENOTSUP) {
			bb_perror_ಸಂದೇಶ("warning: can't change context"
					" of %s to %s", ಮಾರ್ಗ, scontext);
		}
	}

 ಹೊರಹೋಗು:
	freecon(scontext);
}

#ಆದರೆ_ಅಂತ್ಯ

ಪೂರ್ಣಾಂಕ install_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ install_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
	ರಚನೆ stat statbuf;
	ಬಹುಲಕ_t ಬಹುಲಕ;
	uid_t uid;
	gid_t gid;
	ಅಕ್ಷರ *arg, *ಕೊನೆಯ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *gid_str;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *uid_str;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ_str;
	ಪೂರ್ಣಾಂಕ mkdir_flags = FILEUTILS_RECUR;
	ಪೂರ್ಣಾಂಕ copy_flags = FILEUTILS_DEREFERENCE | FILEUTILS_FORCE;
	ಪೂರ್ಣಾಂಕ opts;
	ಪೂರ್ಣಾಂಕ ret = EXIT_SUCCESS;
	ಪೂರ್ಣಾಂಕ isdir;
#ಆದರೆ ENABLE_SELINUX
	security_context_t scontext;
	bool use_default_selinux_context = 1;
#ಆದರೆ_ಅಂತ್ಯ
	ಎಣಿಕೆ {
		OPT_c             = 1 << 0,
		OPT_v             = 1 << 1,
		OPT_b             = 1 << 2,
		OPT_MKDIR_LEADING = 1 << 3,
		OPT_DIRECTORY     = 1 << 4,
		OPT_PRESERVE_ಸಮಯ = 1 << 5,
		OPT_STRIP         = 1 << 6,
		OPT_ಗುಂಪು         = 1 << 7,
		OPT_ಬಹುಲಕ          = 1 << 8,
		OPT_OWNER         = 1 << 9,
		OPT_ಗುರಿ        = 1 << 10,
#ಆದರೆ ENABLE_SELINUX
		OPT_ಗಣ_SECURITY_CONTEXT = 1 << 11,
		OPT_PRESERVE_SECURITY_CONTEXT = 1 << 12,
#ಆದರೆ_ಅಂತ್ಯ
	};

	/* -c exists for backwards compatibility, it's needed */
	/* -b is ignored ("make a backup of each existing destination file") */
	opts = GETOPT32(argv, "^"
		"cvb" "Ddpsg:m:o:t:" IF_SELINUX("Z:")
		"\0"
		"t--d:d--t:s--d:d--s"
		IF_FEATURE_INSTALL_LONG_OPTIONS(IF_SELINUX(":Z--\xff:\xff--Z")),
		LONGOPTS
		&gid_str, &ಬಹುಲಕ_str, &uid_str, &ಕೊನೆಯ
		IF_SELINUX(, &scontext)
	);
	argc -= optind;
	argv += optind;

#ಆದರೆ ENABLE_SELINUX
	ಆದರೆ (opts & (OPT_PRESERVE_SECURITY_CONTEXT|OPT_ಗಣ_SECURITY_CONTEXT)) {
		selinux_or_die();
		use_default_selinux_context = 0;
		ಆದರೆ (opts & OPT_PRESERVE_SECURITY_CONTEXT) {
			copy_flags |= FILEUTILS_PRESERVE_SECURITY_CONTEXT;
		}
		ಆದರೆ (opts & OPT_ಗಣ_SECURITY_CONTEXT) {
			setfscreatecon_or_die(scontext);
			copy_flags |= FILEUTILS_ಗಣ_SECURITY_CONTEXT;
		}
	}
#ಆದರೆ_ಅಂತ್ಯ

	ಆದರೆ ((opts & OPT_v) && FILEUTILS_VERBOSE) {
		mkdir_flags |= FILEUTILS_VERBOSE;
		copy_flags |= FILEUTILS_VERBOSE;
	}

	/* preserve access and modification time, this is GNU behaviour,
	 * BSD only preserves modification time */
	ಆದರೆ (opts & OPT_PRESERVE_ಸಮಯ) {
		copy_flags |= FILEUTILS_PRESERVE_ಸ್ಥಿತಿ;
	}
	ಬಹುಲಕ = 0755; /* GNU coreutils 6.10 compat */
	ಆದರೆ (opts & OPT_ಬಹುಲಕ)
		ಬಹುಲಕ = bb_parse_ಬಹುಲಕ(ಬಹುಲಕ_str, ಬಹುಲಕ);
	uid = (opts & OPT_OWNER) ? get_ug_id(uid_str, xuname2uid) : getuid();
	gid = (opts & OPT_ಗುಂಪು) ? get_ug_id(gid_str, xgroup2gid) : getgid();

	/* If -t DIR is in use, then isdir=true, last="DIR" */
	isdir = (opts & OPT_ಗುರಿ);
	ಆದರೆ (!(opts & (OPT_ಗುರಿ|OPT_DIRECTORY))) {
		/* Neither -t DIR nor -d is in use */
		argc--;
		ಕೊನೆಯ = argv[argc];
		argv[argc] = ಖಾಲಿ;
		/* coreutils install resolves link in this case, don't use lstat */
		isdir = stat(ಕೊನೆಯ, &statbuf) < 0 ? 0 : S_ISDIR(statbuf.st_ಬಹುಲಕ);
	}

	ಆದರೆ (argc < 1)
		bb_show_usage();

	ಆಗ ((arg = *argv++) != ಖಾಲಿ) {
		ಅಕ್ಷರ *dest;

		ಆದರೆ (opts & OPT_DIRECTORY) {
			dest = arg;
			/* GNU coreutils 6.9 does not set uid:gid
			 * on intermediate created directories
			 * (only on last one) */
			ಆದರೆ (bb_make_directory(dest, 0755, mkdir_flags)) {
				ret = EXIT_FAILURE;
				ಹೋಗು ಮುಂದಿನ;
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			dest = ಕೊನೆಯ;
			ಆದರೆ (opts & OPT_MKDIR_LEADING) {
				ಅಕ್ಷರ *ddir = xstrdup(dest);
				/*
				 * -D -t DIR1/DIR2/F3 FILE: create DIR1/DIR2/F3, copy FILE there
				 * -D FILE DIR1/DIR2/F3: create DIR1/DIR2, copy FILE there as F3
				 */
				bb_make_directory((opts & OPT_ಗುರಿ) ? ddir : dirname(ddir), 0755, mkdir_flags);
				/* errors are not checked. copy_file
				 * will fail if dir is not created.
				 */
				ಬಿಡುಗಡೆ(ddir);
			}
			ಆದರೆ (isdir)
				dest = concat_ಮಾರ್ಗ_file(ಕೊನೆಯ, bb_basename(arg));
			ಆದರೆ (copy_file(arg, dest, copy_flags) != 0) {
				/* copy is not made */
				ret = EXIT_FAILURE;
				ಹೋಗು ಮುಂದಿನ;
			}
			ಆದರೆ (opts & OPT_STRIP) {
				ಅಕ್ಷರ *ವಾದಗಳು[4];
				ವಾದಗಳು[0] = (ಅಕ್ಷರ*)"strip";
				ವಾದಗಳು[1] = (ಅಕ್ಷರ*)"-p"; /* -p --preserve-dates */
				ವಾದಗಳು[2] = dest;
				ವಾದಗಳು[3] = ಖಾಲಿ;
				ಆದರೆ (spawn_and_wait(ವಾದಗಳು)) {
					bb_simple_perror_ಸಂದೇಶ("strip");
					ret = EXIT_FAILURE;
				}
			}
		}

		/* Set the user and group id */
		/* (must be before chmod, or else chown may clear suid/gid bits) */
		ಆದರೆ ((opts & (OPT_OWNER|OPT_ಗುಂಪು))
		 && lchown(dest, uid, gid) == -1
		) {
			bb_perror_ಸಂದೇಶ("can't change %s of %s", "ownership", dest);
			ret = EXIT_FAILURE;
		}

		/* Set the file mode (always, not only with -m).
		 * GNU coreutils 6.10 is not affected by umask. */
		ಆದರೆ (chmod(dest, ಬಹುಲಕ) == -1) {
			bb_perror_ಸಂದೇಶ("can't change %s of %s", "permissions", dest);
			ret = EXIT_FAILURE;
		}
#ಆದರೆ ENABLE_SELINUX
		ಆದರೆ (use_default_selinux_context)
			setdefaultfilecon(dest);
#ಆದರೆ_ಅಂತ್ಯ
 ಮುಂದಿನ:
		ಆದರೆ (ENABLE_FEATURE_CLEAN_ಮೇಲೆ && isdir)
			ಬಿಡುಗಡೆ(dest);
	}

	ಹಿಂತಿರುಗಿ ret;
}
