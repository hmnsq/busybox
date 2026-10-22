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

#అన్తర్భావ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

#ఒకవేళ !ENABLE_USE_BB_PWD_GRP
#ఒకవేళ defined(__UCLIBC__) && UCLIBC_సంస్కరణ < KERNEL_సంస్కరణ(0, 9, 30)
#దోషం "Sorry, you need at least uClibc version 0.9.30 for id applet to build"
#అయితే_ముగింపు
#అయితే_ముగింపు

గణన {
	PRINT_REAL      = (1 << 0),
	NAME_NOT_సంఖ్య = (1 << 1),
	JUST_USER       = (1 << 2),
	JUST_సమూహం      = (1 << 3),
	JUST_ALL_GROUPS = (1 << 4),
#ఒకవేళ ENABLE_SELINUX
	JUST_CONTEXT    = (1 << 5),
#అయితే_ముగింపు
};

స్థిర పూర్ణాంకం print_common(గుర్తులేని id, స్థిరరాశి అక్షరం *name, స్థిరరాశి అక్షరం *ఉపసర్గ)
{
	ఒకవేళ (ఉపసర్గ) {
		రాయి("%s", ఉపసర్గ);
	}
	ఒకవేళ (!(option_mask32 & NAME_NOT_సంఖ్య) || !name) {
		రాయి("%u", id);
	}
	ఒకవేళ (!option_mask32 || (option_mask32 & NAME_NOT_సంఖ్య)) {
		ఒకవేళ (name) {
			రాయి(option_mask32 ? "%s" : "(%s)", name);
		} లేకపోతే {
			/* Don't set error status flag in default mode */
			ఒకవేళ (option_mask32) {
				ఒకవేళ (ENABLE_DESKTOP)
					bb_error_సందేశం("unknown ID %u", id);
				తిరిగి EXIT_FAILURE;
			}
		}
	}
	తిరిగి EXIT_SUCCESS;
}

స్థిర పూర్ణాంకం print_సమూహం(gid_t id, స్థిరరాశి అక్షరం *ఉపసర్గ)
{
	తిరిగి print_common(id, gid2group(id), ఉపసర్గ);
}

స్థిర పూర్ణాంకం print_user(uid_t id, స్థిరరాశి అక్షరం *ఉపసర్గ)
{
	తిరిగి print_common(id, uid2uname(id), ఉపసర్గ);
}

/* On error set *n < 0 and return >= 0
 * If *n is too small, update it and return < 0
 * (ok to trash groups[] in both cases)
 * Otherwise fill in groups[] and return >= 0
 */
స్థిర పూర్ణాంకం get_groups(స్థిరరాశి అక్షరం *username, gid_t rgid, gid_t *groups, పూర్ణాంకం *n)
{
	పూర్ణాంకం m;

	ఒకవేళ (username) {
		/* If the user is a member of more than
		 * *n groups, then -1 is returned. Otherwise >= 0.
		 * (and no defined way of detecting errors?!) */
		m = getgrouplist(username, rgid, groups, n);
		/* I guess *n < 0 might indicate error. Anyway,
		 * malloc'ing -1 bytes won't be good, so: */
		ఒకవేళ (*n < 0)
			తిరిగి 0;
		తిరిగి m;
	}

	*n = getgroups(*n, groups);
	ఒకవేళ (*n >= 0)
		తిరిగి *n;
	/* Error */
	ఒకవేళ (errno == EINVAL) /* *n is too small? */
		*n = getgroups(0, groups); /* get needed *n */
	/* if *n >= 0, return -1 (got new *n), else return 0 (error): */
	తిరిగి -(*n >= 0);
}

పూర్ణాంకం id_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం id_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	uid_t ruid;
	gid_t rgid;
	uid_t euid;
	gid_t egid;
	గుర్తులేని opt;
	పూర్ణాంకం i;
	పూర్ణాంకం స్థితి_2 = EXIT_SUCCESS;
	స్థిరరాశి అక్షరం *ఉపసర్గ;
	స్థిరరాశి అక్షరం *username;
#ఒకవేళ ENABLE_SELINUX
	security_context_t scontext = ఖాళీ;
#అయితే_ముగింపు

	ఒకవేళ (ENABLE_GROUPS && (!ENABLE_ID || applet_name[0] == 'g')) {
		/* TODO: coreutils groups prepend "USER : " prefix,
		 * and accept many usernames. Example:
		 * # groups root root
		 * root : root
		 * root : root
		 */
		opt = option_mask32 = getopt32(argv, "") | JUST_ALL_GROUPS | NAME_NOT_సంఖ్య;
	} లేకపోతే {
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
	ఒకవేళ (username) {
		నిర్మాణం passwd *p = xgetpwnam(username);
		euid = ruid = p->pw_uid;
		egid = rgid = p->pw_gid;
	} లేకపోతే {
		egid = getegid();
		rgid = getgid();
		euid = geteuid();
		ruid = getuid();
	}
	/* JUST_ALL_GROUPS ignores -r PRINT_REAL flag even if man page for */
	/* id says: print the real ID instead of the effective ID, with -ugG */
	/* in fact in this case egid is always printed if egid != rgid */
	ఒకవేళ (!opt || (opt & JUST_ALL_GROUPS)) {
		gid_t *groups;
		పూర్ణాంకం n;

		ఒకవేళ (!opt) {
			/* Default Mode */
			స్థితి_2 |= print_user(ruid, "uid=");
			స్థితి_2 |= print_సమూహం(rgid, " gid=");
			ఒకవేళ (euid != ruid)
				స్థితి_2 |= print_user(euid, " euid=");
			ఒకవేళ (egid != rgid)
				స్థితి_2 |= print_సమూహం(egid, " egid=");
		} లేకపోతే {
			/* JUST_ALL_GROUPS */
			స్థితి_2 |= print_సమూహం(rgid, ఖాళీ);
			ఒకవేళ (egid != rgid)
				స్థితి_2 |= print_సమూహం(egid, " ");
		}
		/* We are supplying largish buffer, trying
		 * to not run get_groups() twice. That might be slow
		 * ("user database in remote SQL server" case) */
		groups = xmalloc(64 * పరిమాణం(groups[0]));
		n = 64;
		ఒకవేళ (get_groups(username, rgid, groups, &n) < 0) {
			/* Need bigger buffer after all */
			groups = xrealloc(groups, n * పరిమాణం(groups[0]));
			get_groups(username, rgid, groups, &n);
		}
		ఒకవేళ (n > 0) {
			/* Print the list */
			ఉపసర్గ = " groups=";
			కొరకు (i = 0; i < n; i++) {
				ఒకవేళ (opt && (groups[i] == rgid || groups[i] == egid))
					కొనసాగు;
				స్థితి_2 |= print_సమూహం(groups[i], opt ? " " : ఉపసర్గ);
				ఉపసర్గ = ",";
			}
		} లేకపోతే ఒకవేళ (n < 0) { /* error in get_groups() */
			ఒకవేళ (ENABLE_DESKTOP)
				bb_simple_error_సందేశం_and_die("can't get groups");
			తిరిగి EXIT_FAILURE;
		}
		ఒకవేళ (ENABLE_FEATURE_CLEAN_పైకి)
			విడుదల(groups);
#ఒకవేళ ENABLE_SELINUX
		ఒకవేళ (is_selinux_enabled()) {
			ఒకవేళ (getcon(&scontext) == 0)
				రాయి(" context=%s", scontext);
		}
#అయితే_ముగింపు
	} లేకపోతే ఒకవేళ (opt & PRINT_REAL) {
		euid = ruid;
		egid = rgid;
	}

	ఒకవేళ (opt & JUST_USER)
		స్థితి_2 |= print_user(euid, ఖాళీ);
	లేకపోతే ఒకవేళ (opt & JUST_సమూహం)
		స్థితి_2 |= print_సమూహం(egid, ఖాళీ);
#ఒకవేళ ENABLE_SELINUX
	లేకపోతే ఒకవేళ (opt & JUST_CONTEXT) {
		selinux_or_die();
		ఒకవేళ (username || getcon(&scontext)) {
			bb_error_సందేశం_and_die("can't get process context%s",
				username ? " for a different user" : "");
		}
		fputs_stdout(scontext);
	}
	/* freecon(NULL) seems to be harmless */
	ఒకవేళ (ENABLE_FEATURE_CLEAN_పైకి)
		freecon(scontext);
#అయితే_ముగింపు
	bb_putchar('\n');
	fflush_stdout_and_exit(స్థితి_2);
}
