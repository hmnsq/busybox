/* vi: set sw=4 ts=4: */
/*
 * Mini id implementation for busybox
 *
 * Copyright (C) 2000 by Randolph Chung <tausq@debian.org>
 * Copyright (C) 2008 by Tito Ragusa <farmatito@tiscali.it>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
/* Hacked by Tito Ragusa (C) 2004 to handle usernames of whatever
 * length and to be more similar to GNU id.
 * -Z option support: by Yuichi Nakamura <ynakam@hitachisoft.jp>
 * Added -G option Tito Ragusa (C) 2008 for SUSv3.
 */
//config:config ID
//config:	bool "id (7.1 kb)"
//config:	default y
//config:	help
//config:	id displays the current user and group ID names.
//config:
//config:config GROUPS
//config:	bool "groups (6.8 kb)"
//config:	default y
//config:	help
//config:	Print the group names associated with current user id.

//applet:IF_GROUPS(APPLET_NOEXEC(groups, id, BB_DIR_USR_BIN, BB_SUID_DROP, groups))
//applet:IF_ID(    APPLET_NOEXEC(id,     id, BB_DIR_USR_BIN, BB_SUID_DROP, id    ))

//kbuild:lib-$(CONFIG_GROUPS) += id.o
//kbuild:lib-$(CONFIG_ID)     += id.o

/* BB_AUDIT SUSv3 compliant. */

//usage:#define id_trivial_usage
//usage:       "[-ugGnr"IF_SELINUX("Z")"] [USER]"
//usage:#define id_full_usage "\n\n"
//usage:       "Print information about USER or the current user\n"
//usage:	IF_SELINUX(
//usage:     "\n	-Z	Security context"
//usage:	)
//usage:     "\n	-u	User ID"
//usage:     "\n	-g	Group ID"
//usage:     "\n	-G	Supplementary group IDs"
//usage:     "\n	-n	Print names instead of numbers"
//usage:     "\n	-r	Print real ID instead of effective ID"
//usage:
//usage:#define id_example_usage
//usage:       "$ id\n"
//usage:       "uid=1000(andersen) gid=1000(andersen)\n"

//usage:#define groups_trivial_usage
//usage:       "[USER]"
//usage:#define groups_full_usage "\n\n"
//usage:       "Print the groups USER is in"
//usage:
//usage:#define groups_example_usage
//usage:       "$ groups\n"
//usage:       "andersen lp dialout cdrom floppy\n"

#ಅನ್ತರ್ಭಾವ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

#ಆದರೆ !ENABLE_USE_BB_PWD_GRP
#ಆದರೆ defined(__UCLIBC__) && UCLIBC_ಆವೃತ್ತಿ < KERNEL_ಆವೃತ್ತಿ(0, 9, 30)
#ದೋಷ "Sorry, you need at least uClibc version 0.9.30 for id applet to build"
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ

ಎಣಿಕೆ {
	PRINT_REAL      = (1 << 0),
	NAME_NOT_ಸಂಖ್ಯೆ = (1 << 1),
	JUST_USER       = (1 << 2),
	JUST_ಗುಂಪು      = (1 << 3),
	JUST_ALL_GROUPS = (1 << 4),
#ಆದರೆ ENABLE_SELINUX
	JUST_CONTEXT    = (1 << 5),
#ಆದರೆ_ಅಂತ್ಯ
};

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ print_common(ಚಿಹ್ನೆರಹಿತ id, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ)
{
	ಆದರೆ (ಪೂರ್ವಪ್ರತ್ಯಯ) {
		ಬರೆ("%s", ಪೂರ್ವಪ್ರತ್ಯಯ);
	}
	ಆದರೆ (!(option_mask32 & NAME_NOT_ಸಂಖ್ಯೆ) || !name) {
		ಬರೆ("%u", id);
	}
	ಆದರೆ (!option_mask32 || (option_mask32 & NAME_NOT_ಸಂಖ್ಯೆ)) {
		ಆದರೆ (name) {
			ಬರೆ(option_mask32 ? "%s" : "(%s)", name);
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* Don't set error status flag in default mode */
			ಆದರೆ (option_mask32) {
				ಆದರೆ (ENABLE_DESKTOP)
					bb_error_ಸಂದೇಶ("unknown ID %u", id);
				ಹಿಂತಿರುಗಿ EXIT_FAILURE;
			}
		}
	}
	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ print_ಗುಂಪು(gid_t id, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ)
{
	ಹಿಂತಿರುಗಿ print_common(id, gid2group(id), ಪೂರ್ವಪ್ರತ್ಯಯ);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ print_user(uid_t id, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ)
{
	ಹಿಂತಿರುಗಿ print_common(id, uid2uname(id), ಪೂರ್ವಪ್ರತ್ಯಯ);
}

/* On error set *n < 0 and return >= 0
 * If *n is too small, update it and return < 0
 * (ok to trash groups[] in both cases)
 * Otherwise fill in groups[] and return >= 0
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ get_groups(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *username, gid_t rgid, gid_t *groups, ಪೂರ್ಣಾಂಕ *n)
{
	ಪೂರ್ಣಾಂಕ m;

	ಆದರೆ (username) {
		/* If the user is a member of more than
		 * *n groups, then -1 is returned. Otherwise >= 0.
		 * (and no defined way of detecting errors?!) */
		m = getgrouplist(username, rgid, groups, n);
		/* I guess *n < 0 might indicate error. Anyway,
		 * malloc'ing -1 bytes won't be good, so: */
		ಆದರೆ (*n < 0)
			ಹಿಂತಿರುಗಿ 0;
		ಹಿಂತಿರುಗಿ m;
	}

	*n = getgroups(*n, groups);
	ಆದರೆ (*n >= 0)
		ಹಿಂತಿರುಗಿ *n;
	/* Error */
	ಆದರೆ (errno == EINVAL) /* *n is too small? */
		*n = getgroups(0, groups); /* get needed *n */
	/* if *n >= 0, return -1 (got new *n), else return 0 (error): */
	ಹಿಂತಿರುಗಿ -(*n >= 0);
}

ಪೂರ್ಣಾಂಕ id_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ id_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	uid_t ruid;
	gid_t rgid;
	uid_t euid;
	gid_t egid;
	ಚಿಹ್ನೆರಹಿತ opt;
	ಪೂರ್ಣಾಂಕ i;
	ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2 = EXIT_SUCCESS;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *username;
#ಆದರೆ ENABLE_SELINUX
	security_context_t scontext = ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ

	ಆದರೆ (ENABLE_GROUPS && (!ENABLE_ID || applet_name[0] == 'g')) {
		/* TODO: coreutils groups prepend "USER : " prefix,
		 * and accept many usernames. Example:
		 * # groups root root
		 * root : root
		 * root : root
		 */
		opt = option_mask32 = getopt32(argv, "") | JUST_ALL_GROUPS | NAME_NOT_ಸಂಖ್ಯೆ;
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Don't allow -n -r -nr -ug -rug -nug -rnug -uZ -gZ -GZ*/
		/* Don't allow more than one username */
		opt = getopt32(argv, "^"
			"rnugG" IF_SELINUX("Z")
			"\0"
			"?1:u--g:g--u:G--u:u--G:g--G:G--g:r?ugG:n?ugG"
			IF_SELINUX(":u--Z:Z--u:g--Z:Z--g:G--Z:Z--G")
		);
	}

	username = argv[optind];
	ಆದರೆ (username) {
		ರಚನೆ passwd *p = xgetpwnam(username);
		euid = ruid = p->pw_uid;
		egid = rgid = p->pw_gid;
	} ಇಲ್ಲದಿದ್ದರೆ {
		egid = getegid();
		rgid = getgid();
		euid = geteuid();
		ruid = getuid();
	}
	/* JUST_ALL_GROUPS ignores -r PRINT_REAL flag even if man page for */
	/* id says: print the real ID instead of the effective ID, with -ugG */
	/* in fact in this case egid is always printed if egid != rgid */
	ಆದರೆ (!opt || (opt & JUST_ALL_GROUPS)) {
		gid_t *groups;
		ಪೂರ್ಣಾಂಕ n;

		ಆದರೆ (!opt) {
			/* Default Mode */
			ಸ್ಥಿತಿ_2 |= print_user(ruid, "uid=");
			ಸ್ಥಿತಿ_2 |= print_ಗುಂಪು(rgid, " gid=");
			ಆದರೆ (euid != ruid)
				ಸ್ಥಿತಿ_2 |= print_user(euid, " euid=");
			ಆದರೆ (egid != rgid)
				ಸ್ಥಿತಿ_2 |= print_ಗುಂಪು(egid, " egid=");
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* JUST_ALL_GROUPS */
			ಸ್ಥಿತಿ_2 |= print_ಗುಂಪು(rgid, ಖಾಲಿ);
			ಆದರೆ (egid != rgid)
				ಸ್ಥಿತಿ_2 |= print_ಗುಂಪು(egid, " ");
		}
		/* We are supplying largish buffer, trying
		 * to not run get_groups() twice. That might be slow
		 * ("user database in remote SQL server" case) */
		groups = xmalloc(64 * ಗಾತ್ರ(groups[0]));
		n = 64;
		ಆದರೆ (get_groups(username, rgid, groups, &n) < 0) {
			/* Need bigger buffer after all */
			groups = xrealloc(groups, n * ಗಾತ್ರ(groups[0]));
			get_groups(username, rgid, groups, &n);
		}
		ಆದರೆ (n > 0) {
			/* Print the list */
			ಪೂರ್ವಪ್ರತ್ಯಯ = " groups=";
			ಗಾಗಿ (i = 0; i < n; i++) {
				ಆದರೆ (opt && (groups[i] == rgid || groups[i] == egid))
					ಮುಂದುವರಿ;
				ಸ್ಥಿತಿ_2 |= print_ಗುಂಪು(groups[i], opt ? " " : ಪೂರ್ವಪ್ರತ್ಯಯ);
				ಪೂರ್ವಪ್ರತ್ಯಯ = ",";
			}
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (n < 0) { /* error in get_groups() */
			ಆದರೆ (ENABLE_DESKTOP)
				bb_simple_error_ಸಂದೇಶ_and_die("can't get groups");
			ಹಿಂತಿರುಗಿ EXIT_FAILURE;
		}
		ಆದರೆ (ENABLE_FEATURE_CLEAN_ಮೇಲೆ)
			ಬಿಡುಗಡೆ(groups);
#ಆದರೆ ENABLE_SELINUX
		ಆದರೆ (is_selinux_enabled()) {
			ಆದರೆ (getcon(&scontext) == 0)
				ಬರೆ(" context=%s", scontext);
		}
#ಆದರೆ_ಅಂತ್ಯ
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (opt & PRINT_REAL) {
		euid = ruid;
		egid = rgid;
	}

	ಆದರೆ (opt & JUST_USER)
		ಸ್ಥಿತಿ_2 |= print_user(euid, ಖಾಲಿ);
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (opt & JUST_ಗುಂಪು)
		ಸ್ಥಿತಿ_2 |= print_ಗುಂಪು(egid, ಖಾಲಿ);
#ಆದರೆ ENABLE_SELINUX
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (opt & JUST_CONTEXT) {
		selinux_or_die();
		ಆದರೆ (username || getcon(&scontext)) {
			bb_error_ಸಂದೇಶ_and_die("can't get process context%s",
				username ? " for a different user" : "");
		}
		fputs_stdout(scontext);
	}
	/* freecon(NULL) seems to be harmless */
	ಆದರೆ (ENABLE_FEATURE_CLEAN_ಮೇಲೆ)
		freecon(scontext);
#ಆದರೆ_ಅಂತ್ಯ
	bb_putchar('\n');
	fflush_stdout_and_exit(ಸ್ಥಿತಿ_2);
}
