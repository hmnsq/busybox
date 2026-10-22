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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

#ਜੇ !ENABLE_USE_BB_PWD_GRP
#ਜੇ defined(__UCLIBC__) && UCLIBC_ਸੰਸਕਰਣ < KERNEL_ਸੰਸਕਰਣ(0, 9, 30)
#ਗਲਤੀ "Sorry, you need at least uClibc version 0.9.30 for id applet to build"
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ

ਗਿਣਤੀ {
	PRINT_REAL      = (1 << 0),
	NAME_NOT_ਸੰਖਿਆ = (1 << 1),
	JUST_USER       = (1 << 2),
	JUST_ਸਮੂਹ      = (1 << 3),
	JUST_ALL_GROUPS = (1 << 4),
#ਜੇ ENABLE_SELINUX
	JUST_CONTEXT    = (1 << 5),
#ਜੇ_ਅੰਤ
};

ਸਥਿਰ ਪੂਰਨ print_common(ਬਿਨਾਨਿਸ਼ਾਨੀ id, ਅਚਲ ਅੱਖਰ *name, ਅਚਲ ਅੱਖਰ *ਅਗੇਤਰ)
{
	ਜੇ (ਅਗੇਤਰ) {
		ਲਿਖੋ("%s", ਅਗੇਤਰ);
	}
	ਜੇ (!(option_mask32 & NAME_NOT_ਸੰਖਿਆ) || !name) {
		ਲਿਖੋ("%u", id);
	}
	ਜੇ (!option_mask32 || (option_mask32 & NAME_NOT_ਸੰਖਿਆ)) {
		ਜੇ (name) {
			ਲਿਖੋ(option_mask32 ? "%s" : "(%s)", name);
		} ਨਹੀਂਤਾਂ {
			/* Don't set error status flag in default mode */
			ਜੇ (option_mask32) {
				ਜੇ (ENABLE_DESKTOP)
					bb_error_ਸੁਨੇਹਾ("unknown ID %u", id);
				ਵਾਪਸ EXIT_FAILURE;
			}
		}
	}
	ਵਾਪਸ EXIT_SUCCESS;
}

ਸਥਿਰ ਪੂਰਨ print_ਸਮੂਹ(gid_t id, ਅਚਲ ਅੱਖਰ *ਅਗੇਤਰ)
{
	ਵਾਪਸ print_common(id, gid2group(id), ਅਗੇਤਰ);
}

ਸਥਿਰ ਪੂਰਨ print_user(uid_t id, ਅਚਲ ਅੱਖਰ *ਅਗੇਤਰ)
{
	ਵਾਪਸ print_common(id, uid2uname(id), ਅਗੇਤਰ);
}

/* On error set *n < 0 and return >= 0
 * If *n is too small, update it and return < 0
 * (ok to trash groups[] in both cases)
 * Otherwise fill in groups[] and return >= 0
 */
ਸਥਿਰ ਪੂਰਨ get_groups(ਅਚਲ ਅੱਖਰ *username, gid_t rgid, gid_t *groups, ਪੂਰਨ *n)
{
	ਪੂਰਨ m;

	ਜੇ (username) {
		/* If the user is a member of more than
		 * *n groups, then -1 is returned. Otherwise >= 0.
		 * (and no defined way of detecting errors?!) */
		m = getgrouplist(username, rgid, groups, n);
		/* I guess *n < 0 might indicate error. Anyway,
		 * malloc'ing -1 bytes won't be good, so: */
		ਜੇ (*n < 0)
			ਵਾਪਸ 0;
		ਵਾਪਸ m;
	}

	*n = getgroups(*n, groups);
	ਜੇ (*n >= 0)
		ਵਾਪਸ *n;
	/* Error */
	ਜੇ (errno == EINVAL) /* *n is too small? */
		*n = getgroups(0, groups); /* get needed *n */
	/* if *n >= 0, return -1 (got new *n), else return 0 (error): */
	ਵਾਪਸ -(*n >= 0);
}

ਪੂਰਨ id_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ id_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
	uid_t ruid;
	gid_t rgid;
	uid_t euid;
	gid_t egid;
	ਬਿਨਾਨਿਸ਼ਾਨੀ opt;
	ਪੂਰਨ i;
	ਪੂਰਨ ਸਥਿਤੀ = EXIT_SUCCESS;
	ਅਚਲ ਅੱਖਰ *ਅਗੇਤਰ;
	ਅਚਲ ਅੱਖਰ *username;
#ਜੇ ENABLE_SELINUX
	security_context_t scontext = ਨਿਰਾ;
#ਜੇ_ਅੰਤ

	ਜੇ (ENABLE_GROUPS && (!ENABLE_ID || applet_name[0] == 'g')) {
		/* TODO: coreutils groups prepend "USER : " prefix,
		 * and accept many usernames. Example:
		 * # groups root root
		 * root : root
		 * root : root
		 */
		opt = option_mask32 = getopt32(argv, "") | JUST_ALL_GROUPS | NAME_NOT_ਸੰਖਿਆ;
	} ਨਹੀਂਤਾਂ {
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
	ਜੇ (username) {
		ਬਣਤਰ passwd *p = xgetpwnam(username);
		euid = ruid = p->pw_uid;
		egid = rgid = p->pw_gid;
	} ਨਹੀਂਤਾਂ {
		egid = getegid();
		rgid = getgid();
		euid = geteuid();
		ruid = getuid();
	}
	/* JUST_ALL_GROUPS ignores -r PRINT_REAL flag even if man page for */
	/* id says: print the real ID instead of the effective ID, with -ugG */
	/* in fact in this case egid is always printed if egid != rgid */
	ਜੇ (!opt || (opt & JUST_ALL_GROUPS)) {
		gid_t *groups;
		ਪੂਰਨ n;

		ਜੇ (!opt) {
			/* Default Mode */
			ਸਥਿਤੀ |= print_user(ruid, "uid=");
			ਸਥਿਤੀ |= print_ਸਮੂਹ(rgid, " gid=");
			ਜੇ (euid != ruid)
				ਸਥਿਤੀ |= print_user(euid, " euid=");
			ਜੇ (egid != rgid)
				ਸਥਿਤੀ |= print_ਸਮੂਹ(egid, " egid=");
		} ਨਹੀਂਤਾਂ {
			/* JUST_ALL_GROUPS */
			ਸਥਿਤੀ |= print_ਸਮੂਹ(rgid, ਨਿਰਾ);
			ਜੇ (egid != rgid)
				ਸਥਿਤੀ |= print_ਸਮੂਹ(egid, " ");
		}
		/* We are supplying largish buffer, trying
		 * to not run get_groups() twice. That might be slow
		 * ("user database in remote SQL server" case) */
		groups = xmalloc(64 * ਆਕਾਰ(groups[0]));
		n = 64;
		ਜੇ (get_groups(username, rgid, groups, &n) < 0) {
			/* Need bigger buffer after all */
			groups = xrealloc(groups, n * ਆਕਾਰ(groups[0]));
			get_groups(username, rgid, groups, &n);
		}
		ਜੇ (n > 0) {
			/* Print the list */
			ਅਗੇਤਰ = " groups=";
			ਲਈ (i = 0; i < n; i++) {
				ਜੇ (opt && (groups[i] == rgid || groups[i] == egid))
					ਜਾਰੀ;
				ਸਥਿਤੀ |= print_ਸਮੂਹ(groups[i], opt ? " " : ਅਗੇਤਰ);
				ਅਗੇਤਰ = ",";
			}
		} ਨਹੀਂਤਾਂ ਜੇ (n < 0) { /* error in get_groups() */
			ਜੇ (ENABLE_DESKTOP)
				bb_simple_error_ਸੁਨੇਹਾ_and_die("can't get groups");
			ਵਾਪਸ EXIT_FAILURE;
		}
		ਜੇ (ENABLE_FEATURE_CLEAN_ਉੱਪਰ)
			ਮੁਕਤ(groups);
#ਜੇ ENABLE_SELINUX
		ਜੇ (is_selinux_enabled()) {
			ਜੇ (getcon(&scontext) == 0)
				ਲਿਖੋ(" context=%s", scontext);
		}
#ਜੇ_ਅੰਤ
	} ਨਹੀਂਤਾਂ ਜੇ (opt & PRINT_REAL) {
		euid = ruid;
		egid = rgid;
	}

	ਜੇ (opt & JUST_USER)
		ਸਥਿਤੀ |= print_user(euid, ਨਿਰਾ);
	ਨਹੀਂਤਾਂ ਜੇ (opt & JUST_ਸਮੂਹ)
		ਸਥਿਤੀ |= print_ਸਮੂਹ(egid, ਨਿਰਾ);
#ਜੇ ENABLE_SELINUX
	ਨਹੀਂਤਾਂ ਜੇ (opt & JUST_CONTEXT) {
		selinux_or_die();
		ਜੇ (username || getcon(&scontext)) {
			bb_error_ਸੁਨੇਹਾ_and_die("can't get process context%s",
				username ? " for a different user" : "");
		}
		fputs_stdout(scontext);
	}
	/* freecon(NULL) seems to be harmless */
	ਜੇ (ENABLE_FEATURE_CLEAN_ਉੱਪਰ)
		freecon(scontext);
#ਜੇ_ਅੰਤ
	bb_putchar('\n');
	fflush_stdout_and_exit(ਸਥਿਤੀ);
}
