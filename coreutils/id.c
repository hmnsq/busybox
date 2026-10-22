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

#ضمن "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

#إذا !ENABLE_USE_BB_PWD_GRP
#إذا defined(__UCLIBC__) && UCLIBC_VERSION < KERNEL_VERSION(0, 9, 30)
#error "Sorry, you need at least uClibc version 0.9.30 for id applet to build"
#endif
#endif

تعداد {
	بغ_REAL      = (1 << 0),
	NAME_NOT_لمب = (1 << 1),
	JUST_USER       = (1 << 2),
	JUST_GROUP      = (1 << 3),
	JUST_ALL_GROUPS = (1 << 4),
#إذا ENABLE_SELINUX
	JUST_CONTEXT    = (1 << 5),
#endif
};

ساكن صحيح بغ_common(unsigned id, ثابت حرف *name, ثابت حرف *prefix)
{
	إذا (prefix) {
		printf("%s", prefix);
	}
	إذا (!(option_mask32 & NAME_NOT_لمب) || !name) {
		printf("%u", id);
	}
	إذا (!option_mask32 || (option_mask32 & NAME_NOT_لمب)) {
		إذا (name) {
			printf(option_mask32 ? "%s" : "(%s)", name);
		} وإلا {
			/* Don't set error status flag in default mode */
			إذا (option_mask32) {
				إذا (ENABLE_DESKTOP)
					bb_error_سكو("unknown ID %u", id);
				أرجع EXIT_FAILURE;
			}
		}
	}
	أرجع EXIT_SUCCESS;
}

ساكن صحيح بغ_group(gid_t id, ثابت حرف *prefix)
{
	أرجع بغ_common(id, gid2group(id), prefix);
}

ساكن صحيح بغ_user(uid_t id, ثابت حرف *prefix)
{
	أرجع بغ_common(id, uid2uname(id), prefix);
}

/* On error set *n < 0 and return >= 0
 * If *n is too small, update it and return < 0
 * (ok to trash groups[] in both cases)
 * Otherwise fill in groups[] and return >= 0
 */
ساكن صحيح get_groups(ثابت حرف *username, gid_t rgid, gid_t *groups, صحيح *n)
{
	صحيح m;

	إذا (username) {
		/* If the user is a member of more than
		 * *n groups, then -1 is returned. Otherwise >= 0.
		 * (and no defined way of detecting errors?!) */
		m = getgrouplist(username, rgid, groups, n);
		/* I guess *n < 0 might indicate error. Anyway,
		 * malloc'ing -1 bytes won't be good, so: */
		إذا (*n < 0)
			أرجع 0;
		أرجع m;
	}

	*n = getgroups(*n, groups);
	إذا (*n >= 0)
		أرجع *n;
	/* Error */
	إذا (errno == EINVAL) /* *n is too small? */
		*n = getgroups(0, groups); /* get needed *n */
	/* if *n >= 0, return -1 (got new *n), else return 0 (error): */
	أرجع -(*n >= 0);
}

صحيح id_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح id_main(صحيح argc UNUSED_PARAM, حرف **argv)
{
	uid_t ruid;
	gid_t rgid;
	uid_t euid;
	gid_t egid;
	unsigned opt;
	صحيح i;
	صحيح status = EXIT_SUCCESS;
	ثابت حرف *prefix;
	ثابت حرف *username;
#إذا ENABLE_SELINUX
	security_context_t scontext = عدم;
#endif

	إذا (ENABLE_GROUPS && (!ENABLE_ID || applet_name[0] == 'g')) {
		/* TODO: coreutils groups prepend "USER : " prefix,
		 * and accept many usernames. Example:
		 * # groups root root
		 * root : root
		 * root : root
		 */
		opt = option_mask32 = getopt32(argv, "") | JUST_ALL_GROUPS | NAME_NOT_لمب;
	} وإلا {
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
	إذا (username) {
		بنية passwd *p = xgetpwnam(username);
		euid = ruid = p->pw_uid;
		egid = rgid = p->pw_gid;
	} وإلا {
		egid = getegid();
		rgid = getgid();
		euid = geteuid();
		ruid = getuid();
	}
	/* JUST_ALL_GROUPS ignores -r PRINT_REAL flag even if man page for */
	/* id says: print the real ID instead of the effective ID, with -ugG */
	/* in fact in this case egid is always printed if egid != rgid */
	إذا (!opt || (opt & JUST_ALL_GROUPS)) {
		gid_t *groups;
		صحيح n;

		إذا (!opt) {
			/* Default Mode */
			status |= بغ_user(ruid, "uid=");
			status |= بغ_group(rgid, " gid=");
			إذا (euid != ruid)
				status |= بغ_user(euid, " euid=");
			إذا (egid != rgid)
				status |= بغ_group(egid, " egid=");
		} وإلا {
			/* JUST_ALL_GROUPS */
			status |= بغ_group(rgid, عدم);
			إذا (egid != rgid)
				status |= بغ_group(egid, " ");
		}
		/* We are supplying largish buffer, trying
		 * to not run get_groups() twice. That might be slow
		 * ("user database in remote SQL server" case) */
		groups = xmalloc(64 * حجم(groups[0]));
		n = 64;
		إذا (get_groups(username, rgid, groups, &n) < 0) {
			/* Need bigger buffer after all */
			groups = xrealloc(groups, n * حجم(groups[0]));
			get_groups(username, rgid, groups, &n);
		}
		إذا (n > 0) {
			/* Print the list */
			prefix = " groups=";
			لكل (i = 0; i < n; i++) {
				إذا (opt && (groups[i] == rgid || groups[i] == egid))
					استمر;
				status |= بغ_group(groups[i], opt ? " " : prefix);
				prefix = ",";
			}
		} وإلا إذا (n < 0) { /* error in get_groups() */
			إذا (ENABLE_DESKTOP)
				bb_simple_error_سكو_and_die("can't get groups");
			أرجع EXIT_FAILURE;
		}
		إذا (ENABLE_FEATURE_CLEAN_UP)
			حرر(groups);
#إذا ENABLE_SELINUX
		إذا (is_selinux_enabled()) {
			إذا (getcon(&scontext) == 0)
				printf(" context=%s", scontext);
		}
#endif
	} وإلا إذا (opt & بغ_REAL) {
		euid = ruid;
		egid = rgid;
	}

	إذا (opt & JUST_USER)
		status |= بغ_user(euid, عدم);
	وإلا إذا (opt & JUST_GROUP)
		status |= بغ_group(egid, عدم);
#إذا ENABLE_SELINUX
	وإلا إذا (opt & JUST_CONTEXT) {
		selinux_or_die();
		إذا (username || getcon(&scontext)) {
			bb_error_سكو_and_die("can't get process context%s",
				username ? " for a different user" : "");
		}
		fputs_stdout(scontext);
	}
	/* freecon(NULL) seems to be harmless */
	إذا (ENABLE_FEATURE_CLEAN_UP)
		freecon(scontext);
#endif
	bb_putchar('\n');
	fflush_stdout_and_exit(status);
}
