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

#شامل "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

#اگر !ENABLE_USE_BB_PWD_GRP
#اگر defined(__UCLIBC__) && UCLIBC_نسخہ < KERNEL_نسخہ(0, 9, 30)
#غلطی "Sorry, you need at least uClibc version 0.9.30 for id applet to build"
#اگر_ختم
#اگر_ختم

شمار {
	PRINT_REAL      = (1 << 0),
	NAME_NOT_نمبر = (1 << 1),
	JUST_USER       = (1 << 2),
	JUST_گروہ      = (1 << 3),
	JUST_ALL_GROUPS = (1 << 4),
#اگر ENABLE_SELINUX
	JUST_CONTEXT    = (1 << 5),
#اگر_ختم
};

ساکن صحیح print_common(غیرعلامتی id, ثابت حرف *name, ثابت حرف *سابقہ)
{
	اگر (سابقہ) {
		لکھو("%s", سابقہ);
	}
	اگر (!(option_mask32 & NAME_NOT_نمبر) || !name) {
		لکھو("%u", id);
	}
	اگر (!option_mask32 || (option_mask32 & NAME_NOT_نمبر)) {
		اگر (name) {
			لکھو(option_mask32 ? "%s" : "(%s)", name);
		} ورنہ {
			/* Don't set error status flag in default mode */
			اگر (option_mask32) {
				اگر (ENABLE_DESKTOP)
					bb_error_پیغام("unknown ID %u", id);
				واپس EXIT_FAILURE;
			}
		}
	}
	واپس EXIT_SUCCESS;
}

ساکن صحیح print_گروہ(gid_t id, ثابت حرف *سابقہ)
{
	واپس print_common(id, gid2group(id), سابقہ);
}

ساکن صحیح print_user(uid_t id, ثابت حرف *سابقہ)
{
	واپس print_common(id, uid2uname(id), سابقہ);
}

/* On error set *n < 0 and return >= 0
 * If *n is too small, update it and return < 0
 * (ok to trash groups[] in both cases)
 * Otherwise fill in groups[] and return >= 0
 */
ساکن صحیح get_groups(ثابت حرف *username, gid_t rgid, gid_t *groups, صحیح *n)
{
	صحیح m;

	اگر (username) {
		/* If the user is a member of more than
		 * *n groups, then -1 is returned. Otherwise >= 0.
		 * (and no defined way of detecting errors?!) */
		m = getgrouplist(username, rgid, groups, n);
		/* I guess *n < 0 might indicate error. Anyway,
		 * malloc'ing -1 bytes won't be good, so: */
		اگر (*n < 0)
			واپس 0;
		واپس m;
	}

	*n = getgroups(*n, groups);
	اگر (*n >= 0)
		واپس *n;
	/* Error */
	اگر (errno == EINVAL) /* *n is too small? */
		*n = getgroups(0, groups); /* get needed *n */
	/* if *n >= 0, return -1 (got new *n), else return 0 (error): */
	واپس -(*n >= 0);
}

صحیح id_main(صحیح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح id_main(صحیح argc غیر_مستعمل_PARAM, حرف **argv)
{
	uid_t ruid;
	gid_t rgid;
	uid_t euid;
	gid_t egid;
	غیرعلامتی opt;
	صحیح i;
	صحیح حالت_2 = EXIT_SUCCESS;
	ثابت حرف *سابقہ;
	ثابت حرف *username;
#اگر ENABLE_SELINUX
	security_context_t scontext = تہی;
#اگر_ختم

	اگر (ENABLE_GROUPS && (!ENABLE_ID || applet_name[0] == 'g')) {
		/* TODO: coreutils groups prepend "USER : " prefix,
		 * and accept many usernames. Example:
		 * # groups root root
		 * root : root
		 * root : root
		 */
		opt = option_mask32 = getopt32(argv, "") | JUST_ALL_GROUPS | NAME_NOT_نمبر;
	} ورنہ {
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
	اگر (username) {
		ساخت passwd *p = xgetpwnam(username);
		euid = ruid = p->pw_uid;
		egid = rgid = p->pw_gid;
	} ورنہ {
		egid = getegid();
		rgid = getgid();
		euid = geteuid();
		ruid = getuid();
	}
	/* JUST_ALL_GROUPS ignores -r PRINT_REAL flag even if man page for */
	/* id says: print the real ID instead of the effective ID, with -ugG */
	/* in fact in this case egid is always printed if egid != rgid */
	اگر (!opt || (opt & JUST_ALL_GROUPS)) {
		gid_t *groups;
		صحیح n;

		اگر (!opt) {
			/* Default Mode */
			حالت_2 |= print_user(ruid, "uid=");
			حالت_2 |= print_گروہ(rgid, " gid=");
			اگر (euid != ruid)
				حالت_2 |= print_user(euid, " euid=");
			اگر (egid != rgid)
				حالت_2 |= print_گروہ(egid, " egid=");
		} ورنہ {
			/* JUST_ALL_GROUPS */
			حالت_2 |= print_گروہ(rgid, تہی);
			اگر (egid != rgid)
				حالت_2 |= print_گروہ(egid, " ");
		}
		/* We are supplying largish buffer, trying
		 * to not run get_groups() twice. That might be slow
		 * ("user database in remote SQL server" case) */
		groups = xmalloc(64 * حجم(groups[0]));
		n = 64;
		اگر (get_groups(username, rgid, groups, &n) < 0) {
			/* Need bigger buffer after all */
			groups = xrealloc(groups, n * حجم(groups[0]));
			get_groups(username, rgid, groups, &n);
		}
		اگر (n > 0) {
			/* Print the list */
			سابقہ = " groups=";
			برائے (i = 0; i < n; i++) {
				اگر (opt && (groups[i] == rgid || groups[i] == egid))
					جاری;
				حالت_2 |= print_گروہ(groups[i], opt ? " " : سابقہ);
				سابقہ = ",";
			}
		} ورنہ اگر (n < 0) { /* error in get_groups() */
			اگر (ENABLE_DESKTOP)
				bb_simple_error_پیغام_and_die("can't get groups");
			واپس EXIT_FAILURE;
		}
		اگر (ENABLE_FEATURE_CLEAN_اوپر)
			آزاد(groups);
#اگر ENABLE_SELINUX
		اگر (is_selinux_enabled()) {
			اگر (getcon(&scontext) == 0)
				لکھو(" context=%s", scontext);
		}
#اگر_ختم
	} ورنہ اگر (opt & PRINT_REAL) {
		euid = ruid;
		egid = rgid;
	}

	اگر (opt & JUST_USER)
		حالت_2 |= print_user(euid, تہی);
	ورنہ اگر (opt & JUST_گروہ)
		حالت_2 |= print_گروہ(egid, تہی);
#اگر ENABLE_SELINUX
	ورنہ اگر (opt & JUST_CONTEXT) {
		selinux_or_die();
		اگر (username || getcon(&scontext)) {
			bb_error_پیغام_and_die("can't get process context%s",
				username ? " for a different user" : "");
		}
		fputs_stdout(scontext);
	}
	/* freecon(NULL) seems to be harmless */
	اگر (ENABLE_FEATURE_CLEAN_اوپر)
		freecon(scontext);
#اگر_ختم
	bb_putchar('\n');
	fflush_stdout_and_exit(حالت_2);
}
