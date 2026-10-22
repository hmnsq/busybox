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

#अन्तर्भाव "libbb.h"
#अन्तर्भाव "libcoreutils/coreutils.h"

#यदि ENABLE_FEATURE_INSTALL_LONG_OPTIONS
स्थिर ध्रुव वर्ण install_longopts[] ALIGN1 =
	IF_FEATURE_VERBOSE(
	"verbose\0"             No_तर्कः       "v"
	)
	"directory\0"           No_तर्कः       "d"
	"preserve-timestamps\0" No_तर्कः       "p"
	"strip\0"               No_तर्कः       "s"
	"group\0"               Required_तर्कः "g"
	"mode\0"                Required_तर्कः "m"
	"owner\0"               Required_तर्कः "o"
	"target-directory\0"	Required_तर्कः "t"
/* autofs build insists of using -b --suffix=.orig */
/* TODO? (short option for --suffix is -S) */
# यदि ENABLE_SELINUX
	"context\0"             Required_तर्कः "Z"
	"preserve_context\0"    No_तर्कः       "\xff"
	"preserve-context\0"    No_तर्कः       "\xff"
# endif
	;
# define GETOPT32 getopt32long
# define LONGOPTS install_longopts,
#अन्यथा
# define GETOPT32 getopt32
# define LONGOPTS
#यदि_अन्तः


#यदि ENABLE_SELINUX
स्थिर शून्य setdefaultfilecon(ध्रुव वर्ण *मार्गः)
{
	संरचना stat s;
	security_context_t scontext = रिक्त;

	यदि (!is_selinux_enabled()) {
		प्रत्यागम;
	}
	यदि (lstat(मार्गः, &s) != 0) {
		प्रत्यागम;
	}

	यदि (matchpathcon(मार्गः, s.st_बहुलकम्, &scontext) < 0) {
		गच्छ बहिर्गमनम्;
	}
	यदि (वाक्यं_तुलय(scontext, "<<none>>") == 0) {
		गच्छ बहिर्गमनम्;
	}

	यदि (lsetfilecon(मार्गः, scontext) < 0) {
		यदि (errno != ENOTSUP) {
			bb_perror_सन्देशः("warning: can't change context"
					" of %s to %s", मार्गः, scontext);
		}
	}

 बहिर्गमनम्:
	freecon(scontext);
}

#यदि_अन्तः

पूर्णाङ्क install_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क install_main(पूर्णाङ्क argc, वर्ण **argv)
{
	संरचना stat statbuf;
	बहुलकम्_t बहुलकम्;
	uid_t uid;
	gid_t gid;
	वर्ण *arg, *चरमः;
	ध्रुव वर्ण *gid_str;
	ध्रुव वर्ण *uid_str;
	ध्रुव वर्ण *बहुलकम्_str;
	पूर्णाङ्क mkdir_flags = FILEUTILS_RECUR;
	पूर्णाङ्क copy_flags = FILEUTILS_DEREFERENCE | FILEUTILS_FORCE;
	पूर्णाङ्क opts;
	पूर्णाङ्क ret = EXIT_SUCCESS;
	पूर्णाङ्क isdir;
#यदि ENABLE_SELINUX
	security_context_t scontext;
	bool use_default_selinux_context = 1;
#यदि_अन्तः
	गणना {
		OPT_c             = 1 << 0,
		OPT_v             = 1 << 1,
		OPT_b             = 1 << 2,
		OPT_MKDIR_LEADING = 1 << 3,
		OPT_DIRECTORY     = 1 << 4,
		OPT_PRESERVE_TIME = 1 << 5,
		OPT_STRIP         = 1 << 6,
		OPT_गणः         = 1 << 7,
		OPT_बहुलकम्          = 1 << 8,
		OPT_OWNER         = 1 << 9,
		OPT_TARGET        = 1 << 10,
#यदि ENABLE_SELINUX
		OPT_समुच्चयः_SECURITY_CONTEXT = 1 << 11,
		OPT_PRESERVE_SECURITY_CONTEXT = 1 << 12,
#यदि_अन्तः
	};

	/* -c exists for backwards compatibility, it's needed */
	/* -b is ignored ("make a backup of each existing destination file") */
	opts = GETOPT32(argv, "^"
		"cvb" "Ddpsg:m:o:t:" IF_SELINUX("Z:")
		"\0"
		"t--d:d--t:s--d:d--s"
		IF_FEATURE_INSTALL_LONG_OPTIONS(IF_SELINUX(":Z--\xff:\xff--Z")),
		LONGOPTS
		&gid_str, &बहुलकम्_str, &uid_str, &चरमः
		IF_SELINUX(, &scontext)
	);
	argc -= optind;
	argv += optind;

#यदि ENABLE_SELINUX
	यदि (opts & (OPT_PRESERVE_SECURITY_CONTEXT|OPT_समुच्चयः_SECURITY_CONTEXT)) {
		selinux_or_die();
		use_default_selinux_context = 0;
		यदि (opts & OPT_PRESERVE_SECURITY_CONTEXT) {
			copy_flags |= FILEUTILS_PRESERVE_SECURITY_CONTEXT;
		}
		यदि (opts & OPT_समुच्चयः_SECURITY_CONTEXT) {
			setfscreatecon_or_die(scontext);
			copy_flags |= FILEUTILS_समुच्चयः_SECURITY_CONTEXT;
		}
	}
#यदि_अन्तः

	यदि ((opts & OPT_v) && FILEUTILS_VERBOSE) {
		mkdir_flags |= FILEUTILS_VERBOSE;
		copy_flags |= FILEUTILS_VERBOSE;
	}

	/* preserve access and modification time, this is GNU behaviour,
	 * BSD only preserves modification time */
	यदि (opts & OPT_PRESERVE_TIME) {
		copy_flags |= FILEUTILS_PRESERVE_स्थितिः;
	}
	बहुलकम् = 0755; /* GNU coreutils 6.10 compat */
	यदि (opts & OPT_बहुलकम्)
		बहुलकम् = bb_parse_बहुलकम्(बहुलकम्_str, बहुलकम्);
	uid = (opts & OPT_OWNER) ? get_ug_id(uid_str, xuname2uid) : getuid();
	gid = (opts & OPT_गणः) ? get_ug_id(gid_str, xgroup2gid) : getgid();

	/* If -t DIR is in use, then isdir=true, last="DIR" */
	isdir = (opts & OPT_TARGET);
	यदि (!(opts & (OPT_TARGET|OPT_DIRECTORY))) {
		/* Neither -t DIR nor -d is in use */
		argc--;
		चरमः = argv[argc];
		argv[argc] = रिक्त;
		/* coreutils install resolves link in this case, don't use lstat */
		isdir = stat(चरमः, &statbuf) < 0 ? 0 : S_ISDIR(statbuf.st_बहुलकम्);
	}

	यदि (argc < 1)
		bb_show_usage();

	यावत् ((arg = *argv++) != रिक्त) {
		वर्ण *dest;

		यदि (opts & OPT_DIRECTORY) {
			dest = arg;
			/* GNU coreutils 6.9 does not set uid:gid
			 * on intermediate created directories
			 * (only on last one) */
			यदि (bb_make_directory(dest, 0755, mkdir_flags)) {
				ret = EXIT_FAILURE;
				गच्छ अग्र;
			}
		} अन्यथा {
			dest = चरमः;
			यदि (opts & OPT_MKDIR_LEADING) {
				वर्ण *ddir = xstrdup(dest);
				/*
				 * -D -t DIR1/DIR2/F3 FILE: create DIR1/DIR2/F3, copy FILE there
				 * -D FILE DIR1/DIR2/F3: create DIR1/DIR2, copy FILE there as F3
				 */
				bb_make_directory((opts & OPT_TARGET) ? ddir : dirname(ddir), 0755, mkdir_flags);
				/* errors are not checked. copy_file
				 * will fail if dir is not created.
				 */
				मुक्त(ddir);
			}
			यदि (isdir)
				dest = concat_मार्गः_file(चरमः, bb_basename(arg));
			यदि (copy_file(arg, dest, copy_flags) != 0) {
				/* copy is not made */
				ret = EXIT_FAILURE;
				गच्छ अग्र;
			}
			यदि (opts & OPT_STRIP) {
				वर्ण *तर्काः[4];
				तर्काः[0] = (वर्ण*)"strip";
				तर्काः[1] = (वर्ण*)"-p"; /* -p --preserve-dates */
				तर्काः[2] = dest;
				तर्काः[3] = रिक्त;
				यदि (spawn_and_wait(तर्काः)) {
					bb_simple_perror_सन्देशः("strip");
					ret = EXIT_FAILURE;
				}
			}
		}

		/* Set the user and group id */
		/* (must be before chmod, or else chown may clear suid/gid bits) */
		यदि ((opts & (OPT_OWNER|OPT_गणः))
		 && lchown(dest, uid, gid) == -1
		) {
			bb_perror_सन्देशः("can't change %s of %s", "ownership", dest);
			ret = EXIT_FAILURE;
		}

		/* Set the file mode (always, not only with -m).
		 * GNU coreutils 6.10 is not affected by umask. */
		यदि (chmod(dest, बहुलकम्) == -1) {
			bb_perror_सन्देशः("can't change %s of %s", "permissions", dest);
			ret = EXIT_FAILURE;
		}
#यदि ENABLE_SELINUX
		यदि (use_default_selinux_context)
			setdefaultfilecon(dest);
#यदि_अन्तः
 अग्र:
		यदि (ENABLE_FEATURE_CLEAN_ऊर्ध्वम् && isdir)
			मुक्त(dest);
	}

	प्रत्यागम ret;
}
