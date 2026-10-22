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

#అన్తర్భావ "libbb.h"
#అన్తర్భావ "libcoreutils/coreutils.h"

#ఒకవేళ ENABLE_FEATURE_INSTALL_LONG_OPTIONS
స్థిర స్థిరరాశి అక్షరం install_longopts[] ALIGN1 =
	IF_FEATURE_VERBOSE(
	"verbose\0"             No_వాదం       "v"
	)
	"directory\0"           No_వాదం       "d"
	"preserve-timestamps\0" No_వాదం       "p"
	"strip\0"               No_వాదం       "s"
	"group\0"               Required_వాదం "g"
	"mode\0"                Required_వాదం "m"
	"owner\0"               Required_వాదం "o"
	"target-directory\0"	Required_వాదం "t"
/* autofs build insists of using -b --suffix=.orig */
/* TODO? (short option for --suffix is -S) */
# ఒకవేళ ENABLE_SELINUX
	"context\0"             Required_వాదం "Z"
	"preserve_context\0"    No_వాదం       "\xff"
	"preserve-context\0"    No_వాదం       "\xff"
# endif
	;
# define GETOPT32 getopt32long
# define LONGOPTS install_longopts,
#లేకపోతే
# define GETOPT32 getopt32
# define LONGOPTS
#అయితే_ముగింపు


#ఒకవేళ ENABLE_SELINUX
స్థిర శూన్య setdefaultfilecon(స్థిరరాశి అక్షరం *మార్గం)
{
	నిర్మాణం stat s;
	security_context_t scontext = ఖాళీ;

	ఒకవేళ (!is_selinux_enabled()) {
		తిరిగి;
	}
	ఒకవేళ (lstat(మార్గం, &s) != 0) {
		తిరిగి;
	}

	ఒకవేళ (matchpathcon(మార్గం, s.st_బహుళకం, &scontext) < 0) {
		వెళ్ళు బహిర్గతం;
	}
	ఒకవేళ (strcmp(scontext, "<<none>>") == 0) {
		వెళ్ళు బహిర్గతం;
	}

	ఒకవేళ (lsetfilecon(మార్గం, scontext) < 0) {
		ఒకవేళ (errno != ENOTSUP) {
			bb_perror_సందేశం("warning: can't change context"
					" of %s to %s", మార్గం, scontext);
		}
	}

 బహిర్గతం:
	freecon(scontext);
}

#అయితే_ముగింపు

పూర్ణాంకం install_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం install_main(పూర్ణాంకం argc, అక్షరం **argv)
{
	నిర్మాణం stat statbuf;
	బహుళకం_t బహుళకం;
	uid_t uid;
	gid_t gid;
	అక్షరం *arg, *చివరి;
	స్థిరరాశి అక్షరం *gid_str;
	స్థిరరాశి అక్షరం *uid_str;
	స్థిరరాశి అక్షరం *బహుళకం_str;
	పూర్ణాంకం mkdir_flags = FILEUTILS_RECUR;
	పూర్ణాంకం copy_flags = FILEUTILS_DEREFERENCE | FILEUTILS_FORCE;
	పూర్ణాంకం opts;
	పూర్ణాంకం ret = EXIT_SUCCESS;
	పూర్ణాంకం isdir;
#ఒకవేళ ENABLE_SELINUX
	security_context_t scontext;
	bool use_default_selinux_context = 1;
#అయితే_ముగింపు
	గణన {
		OPT_c             = 1 << 0,
		OPT_v             = 1 << 1,
		OPT_b             = 1 << 2,
		OPT_MKDIR_LEADING = 1 << 3,
		OPT_DIRECTORY     = 1 << 4,
		OPT_PRESERVE_సమయం = 1 << 5,
		OPT_STRIP         = 1 << 6,
		OPT_సమూహం         = 1 << 7,
		OPT_బహుళకం          = 1 << 8,
		OPT_OWNER         = 1 << 9,
		OPT_లక్ష్యం        = 1 << 10,
#ఒకవేళ ENABLE_SELINUX
		OPT_సమితి_SECURITY_CONTEXT = 1 << 11,
		OPT_PRESERVE_SECURITY_CONTEXT = 1 << 12,
#అయితే_ముగింపు
	};

	/* -c exists for backwards compatibility, it's needed */
	/* -b is ignored ("make a backup of each existing destination file") */
	opts = GETOPT32(argv, "^"
		"cvb" "Ddpsg:m:o:t:" IF_SELINUX("Z:")
		"\0"
		"t--d:d--t:s--d:d--s"
		IF_FEATURE_INSTALL_LONG_OPTIONS(IF_SELINUX(":Z--\xff:\xff--Z")),
		LONGOPTS
		&gid_str, &బహుళకం_str, &uid_str, &చివరి
		IF_SELINUX(, &scontext)
	);
	argc -= optind;
	argv += optind;

#ఒకవేళ ENABLE_SELINUX
	ఒకవేళ (opts & (OPT_PRESERVE_SECURITY_CONTEXT|OPT_సమితి_SECURITY_CONTEXT)) {
		selinux_or_die();
		use_default_selinux_context = 0;
		ఒకవేళ (opts & OPT_PRESERVE_SECURITY_CONTEXT) {
			copy_flags |= FILEUTILS_PRESERVE_SECURITY_CONTEXT;
		}
		ఒకవేళ (opts & OPT_సమితి_SECURITY_CONTEXT) {
			setfscreatecon_or_die(scontext);
			copy_flags |= FILEUTILS_సమితి_SECURITY_CONTEXT;
		}
	}
#అయితే_ముగింపు

	ఒకవేళ ((opts & OPT_v) && FILEUTILS_VERBOSE) {
		mkdir_flags |= FILEUTILS_VERBOSE;
		copy_flags |= FILEUTILS_VERBOSE;
	}

	/* preserve access and modification time, this is GNU behaviour,
	 * BSD only preserves modification time */
	ఒకవేళ (opts & OPT_PRESERVE_సమయం) {
		copy_flags |= FILEUTILS_PRESERVE_స్థితి;
	}
	బహుళకం = 0755; /* GNU coreutils 6.10 compat */
	ఒకవేళ (opts & OPT_బహుళకం)
		బహుళకం = bb_parse_బహుళకం(బహుళకం_str, బహుళకం);
	uid = (opts & OPT_OWNER) ? get_ug_id(uid_str, xuname2uid) : getuid();
	gid = (opts & OPT_సమూహం) ? get_ug_id(gid_str, xgroup2gid) : getgid();

	/* If -t DIR is in use, then isdir=true, last="DIR" */
	isdir = (opts & OPT_లక్ష్యం);
	ఒకవేళ (!(opts & (OPT_లక్ష్యం|OPT_DIRECTORY))) {
		/* Neither -t DIR nor -d is in use */
		argc--;
		చివరి = argv[argc];
		argv[argc] = ఖాళీ;
		/* coreutils install resolves link in this case, don't use lstat */
		isdir = stat(చివరి, &statbuf) < 0 ? 0 : S_ISDIR(statbuf.st_బహుళకం);
	}

	ఒకవేళ (argc < 1)
		bb_show_usage();

	అయితే ((arg = *argv++) != ఖాళీ) {
		అక్షరం *dest;

		ఒకవేళ (opts & OPT_DIRECTORY) {
			dest = arg;
			/* GNU coreutils 6.9 does not set uid:gid
			 * on intermediate created directories
			 * (only on last one) */
			ఒకవేళ (bb_make_directory(dest, 0755, mkdir_flags)) {
				ret = EXIT_FAILURE;
				వెళ్ళు తదుపరి;
			}
		} లేకపోతే {
			dest = చివరి;
			ఒకవేళ (opts & OPT_MKDIR_LEADING) {
				అక్షరం *ddir = xstrdup(dest);
				/*
				 * -D -t DIR1/DIR2/F3 FILE: create DIR1/DIR2/F3, copy FILE there
				 * -D FILE DIR1/DIR2/F3: create DIR1/DIR2, copy FILE there as F3
				 */
				bb_make_directory((opts & OPT_లక్ష్యం) ? ddir : dirname(ddir), 0755, mkdir_flags);
				/* errors are not checked. copy_file
				 * will fail if dir is not created.
				 */
				విడుదల(ddir);
			}
			ఒకవేళ (isdir)
				dest = concat_మార్గం_file(చివరి, bb_basename(arg));
			ఒకవేళ (copy_file(arg, dest, copy_flags) != 0) {
				/* copy is not made */
				ret = EXIT_FAILURE;
				వెళ్ళు తదుపరి;
			}
			ఒకవేళ (opts & OPT_STRIP) {
				అక్షరం *వాదనలు[4];
				వాదనలు[0] = (అక్షరం*)"strip";
				వాదనలు[1] = (అక్షరం*)"-p"; /* -p --preserve-dates */
				వాదనలు[2] = dest;
				వాదనలు[3] = ఖాళీ;
				ఒకవేళ (spawn_and_wait(వాదనలు)) {
					bb_simple_perror_సందేశం("strip");
					ret = EXIT_FAILURE;
				}
			}
		}

		/* Set the user and group id */
		/* (must be before chmod, or else chown may clear suid/gid bits) */
		ఒకవేళ ((opts & (OPT_OWNER|OPT_సమూహం))
		 && lchown(dest, uid, gid) == -1
		) {
			bb_perror_సందేశం("can't change %s of %s", "ownership", dest);
			ret = EXIT_FAILURE;
		}

		/* Set the file mode (always, not only with -m).
		 * GNU coreutils 6.10 is not affected by umask. */
		ఒకవేళ (chmod(dest, బహుళకం) == -1) {
			bb_perror_సందేశం("can't change %s of %s", "permissions", dest);
			ret = EXIT_FAILURE;
		}
#ఒకవేళ ENABLE_SELINUX
		ఒకవేళ (use_default_selinux_context)
			setdefaultfilecon(dest);
#అయితే_ముగింపు
 తదుపరి:
		ఒకవేళ (ENABLE_FEATURE_CLEAN_పైకి && isdir)
			విడుదల(dest);
	}

	తిరిగి ret;
}
