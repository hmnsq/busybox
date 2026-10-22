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

#অন্তর্ভাৱ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

#যদি !ENABLE_USE_BB_PWD_GRP
#যদি defined(__UCLIBC__) && UCLIBC_VERSION < KERNEL_VERSION(0, 9, 30)
#error "Sorry, you need at least uClibc version 0.9.30 for id applet to build"
#endif
#endif

গণনা {
	PRINT_REAL      = (1 << 0),
	NAME_NOT_সংখ্যা = (1 << 1),
	JUST_USER       = (1 << 2),
	JUST_GROUP      = (1 << 3),
	JUST_ALL_GROUPS = (1 << 4),
#যদি ENABLE_SELINUX
	JUST_CONTEXT    = (1 << 5),
#endif
};

স্থির পূর্ণাঙ্ক print_common(unsigned id, ধ্রুৱ ৱর্ণ *name, ধ্রুৱ ৱর্ণ *prefix)
{
	যদি (prefix) {
		printf("%s", prefix);
	}
	যদি (!(option_mask32 & NAME_NOT_সংখ্যা) || !name) {
		printf("%u", id);
	}
	যদি (!option_mask32 || (option_mask32 & NAME_NOT_সংখ্যা)) {
		যদি (name) {
			printf(option_mask32 ? "%s" : "(%s)", name);
		} অন্যথা {
			/* Don't set error status flag in default mode */
			যদি (option_mask32) {
				যদি (ENABLE_DESKTOP)
					bb_error_বাৰ্তা("unknown ID %u", id);
				ঘূৰাই EXIT_FAILURE;
			}
		}
	}
	ঘূৰাই EXIT_SUCCESS;
}

স্থির পূর্ণাঙ্ক print_group(gid_t id, ধ্রুৱ ৱর্ণ *prefix)
{
	ঘূৰাই print_common(id, gid2group(id), prefix);
}

স্থির পূর্ণাঙ্ক print_user(uid_t id, ধ্রুৱ ৱর্ণ *prefix)
{
	ঘূৰাই print_common(id, uid2uname(id), prefix);
}

/* On error set *n < 0 and return >= 0
 * If *n is too small, update it and return < 0
 * (ok to trash groups[] in both cases)
 * Otherwise fill in groups[] and return >= 0
 */
স্থির পূর্ণাঙ্ক get_groups(ধ্রুৱ ৱর্ণ *username, gid_t rgid, gid_t *groups, পূর্ণাঙ্ক *n)
{
	পূর্ণাঙ্ক m;

	যদি (username) {
		/* If the user is a member of more than
		 * *n groups, then -1 is returned. Otherwise >= 0.
		 * (and no defined way of detecting errors?!) */
		m = getgrouplist(username, rgid, groups, n);
		/* I guess *n < 0 might indicate error. Anyway,
		 * malloc'ing -1 bytes won't be good, so: */
		যদি (*n < 0)
			ঘূৰাই 0;
		ঘূৰাই m;
	}

	*n = getgroups(*n, groups);
	যদি (*n >= 0)
		ঘূৰাই *n;
	/* Error */
	যদি (errno == EINVAL) /* *n is too small? */
		*n = getgroups(0, groups); /* get needed *n */
	/* if *n >= 0, return -1 (got new *n), else return 0 (error): */
	ঘূৰাই -(*n >= 0);
}

পূর্ণাঙ্ক id_main(পূর্ণাঙ্ক argc, ৱর্ণ **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণাঙ্ক id_main(পূর্ণাঙ্ক argc UNUSED_PARAM, ৱর্ণ **argv)
{
	uid_t ruid;
	gid_t rgid;
	uid_t euid;
	gid_t egid;
	unsigned opt;
	পূর্ণাঙ্ক i;
	পূর্ণাঙ্ক status = EXIT_SUCCESS;
	ধ্রুৱ ৱর্ণ *prefix;
	ধ্রুৱ ৱর্ণ *username;
#যদি ENABLE_SELINUX
	security_context_t scontext = রিক্ত;
#endif

	যদি (ENABLE_GROUPS && (!ENABLE_ID || applet_name[0] == 'g')) {
		/* TODO: coreutils groups prepend "USER : " prefix,
		 * and accept many usernames. Example:
		 * # groups root root
		 * root : root
		 * root : root
		 */
		opt = option_mask32 = getopt32(argv, "") | JUST_ALL_GROUPS | NAME_NOT_সংখ্যা;
	} অন্যথা {
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
	যদি (username) {
		সংরচনা passwd *p = xgetpwnam(username);
		euid = ruid = p->pw_uid;
		egid = rgid = p->pw_gid;
	} অন্যথা {
		egid = getegid();
		rgid = getgid();
		euid = geteuid();
		ruid = getuid();
	}
	/* JUST_ALL_GROUPS ignores -r PRINT_REAL flag even if man page for */
	/* id says: print the real ID instead of the effective ID, with -ugG */
	/* in fact in this case egid is always printed if egid != rgid */
	যদি (!opt || (opt & JUST_ALL_GROUPS)) {
		gid_t *groups;
		পূর্ণাঙ্ক n;

		যদি (!opt) {
			/* Default Mode */
			status |= print_user(ruid, "uid=");
			status |= print_group(rgid, " gid=");
			যদি (euid != ruid)
				status |= print_user(euid, " euid=");
			যদি (egid != rgid)
				status |= print_group(egid, " egid=");
		} অন্যথা {
			/* JUST_ALL_GROUPS */
			status |= print_group(rgid, রিক্ত);
			যদি (egid != rgid)
				status |= print_group(egid, " ");
		}
		/* We are supplying largish buffer, trying
		 * to not run get_groups() twice. That might be slow
		 * ("user database in remote SQL server" case) */
		groups = xmalloc(64 * পরিমাণ(groups[0]));
		n = 64;
		যদি (get_groups(username, rgid, groups, &n) < 0) {
			/* Need bigger buffer after all */
			groups = xrealloc(groups, n * পরিমাণ(groups[0]));
			get_groups(username, rgid, groups, &n);
		}
		যদি (n > 0) {
			/* Print the list */
			prefix = " groups=";
			বাবে (i = 0; i < n; i++) {
				যদি (opt && (groups[i] == rgid || groups[i] == egid))
					অনুৱর্ত;
				status |= print_group(groups[i], opt ? " " : prefix);
				prefix = ",";
			}
		} অন্যথা যদি (n < 0) { /* error in get_groups() */
			যদি (ENABLE_DESKTOP)
				bb_simple_error_বাৰ্তা_and_die("can't get groups");
			ঘূৰাই EXIT_FAILURE;
		}
		যদি (ENABLE_FEATURE_CLEAN_UP)
			মুক্ত(groups);
#যদি ENABLE_SELINUX
		যদি (is_selinux_enabled()) {
			যদি (getcon(&scontext) == 0)
				printf(" context=%s", scontext);
		}
#endif
	} অন্যথা যদি (opt & PRINT_REAL) {
		euid = ruid;
		egid = rgid;
	}

	যদি (opt & JUST_USER)
		status |= print_user(euid, রিক্ত);
	অন্যথা যদি (opt & JUST_GROUP)
		status |= print_group(egid, রিক্ত);
#যদি ENABLE_SELINUX
	অন্যথা যদি (opt & JUST_CONTEXT) {
		selinux_or_die();
		যদি (username || getcon(&scontext)) {
			bb_error_বাৰ্তা_and_die("can't get process context%s",
				username ? " for a different user" : "");
		}
		fputs_stdout(scontext);
	}
	/* freecon(NULL) seems to be harmless */
	যদি (ENABLE_FEATURE_CLEAN_UP)
		freecon(scontext);
#endif
	bb_putchar('\n');
	fflush_stdout_and_exit(status);
}
